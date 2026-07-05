# Estabilización del boot + botón + backlight (2026-07-05) — RESUELTO

Síntomas reportados (intermitentes): (1) a veces la GUI no llegaba a arrancar, (2) a veces se perdía
la red/SSH ("No route to host") hasta hacer power-cycle; (3) el toque corto no apagaba el LCD y no
bloqueaba/desbloqueaba bien. Validado con varios reboots limpios: boot ~35s, `crng init done` ~7-8s,
nada en `crashed`, WiFi+GUI+usb0 arriba, botón OK.

## Causas raíz del BOOT (todas reales, se solapaban)

### 1. Entropía: `crng init done` tardaba ~230s (LA transversal)
Sin RTC, el móvil arranca sin fuente de entropía. `seedrng` (OpenRC, runlevel boot) carga y acredita
`/var/lib/seedrng/seed.credit`… pero la cadena de semillas **se rompe con cualquier apagado sucio**:
si un boot arranca sin semilla acreditable, en el re-guardado del arranque el crng aún no está
inicializado → guarda semilla *no acreditable* → **todos** los boots siguientes tardan ~230s en
`crng init done` y todo lo que usa `getrandom()` se bloquea minutos: **sshd, ssh-keygen, los nonces
WPA de wpa_supplicant** (= SSH que no entra y WiFi que no asocia justo tras el boot). Solo un apagado
limpio reparaba la cadena.

**Fix**: [etc/local.d/01-rng-resave.start](etc/local.d/01-rng-resave.start) — en background,
`head -c1 /dev/random` bloquea exactamente hasta crng-init y entonces re-guarda una semilla ya
acreditable con `/usr/libexec/rc/sbin/seedrng` (ojo: el binario NO está en PATH, el applet busybox
no existe). Autosuficiente aunque el apagado sea siempre sucio.

### 2. usb0 (red de rescate) era un `ifconfig` one-shot
Si fallaba ese instante, el móvil quedaba inaccesible para siempre.
**Fix**: [etc/local.d/usb0.start](etc/local.d/usb0.start) — reintenta hasta poner link+IP y vigila
10 min re-poniéndola si se pierde. En background.

### 3. sshd: dos mecanismos peleándose → `[crashed]`
El arranque nativo de OpenRC fallaba a veces (entropía, ver #1) y un retry artesanal
(`zz-sshd.start`) levantaba un sshd manual; a partir de ahí el nativo SIEMPRE chocaba (puerto 22
ocupado) y el estado quedaba `crashed` (sin supervisión real).
**Fix**: sshd bajo **supervise-daemon** (re-arranca solo si muere), y `zz-sshd.start` RETIRADO.
En `/etc/conf.d/sshd`:
```
supervisor=supervise-daemon
command_args="-D"
supervise_daemon_args="--respawn-delay 3"
```

### 4. `aa-sshd-keys.start` era un NO-OP
Las `$vars` se evaporaron en un deploy antiguo (heredoc sin quotear): quedó `[ -e "" ] || continue`,
que salta siempre → la restauración de host-keys nunca hizo nada.
**Fix**: [etc/local.d/aa-sshd-keys.start](etc/local.d/aa-sshd-keys.start) con las variables de verdad.
(Lección para deploys: transferir ficheros por base64/tar, nunca por heredoc expandible.)

### 5. Phosh sin supervisión: `exec` one-shot
Si phoc moría al arrancar (carrera DRM/seatd), no había GUI y nadie reintentaba. Y en `phosh-session.sh`
había otro `exec`: si **phosh** moría dejaba a **phoc vivo** (sesión zombi sin GUI, visto 0705).
**Fix**: [usr/local/bin/launch_phosh.sh](usr/local/bin/launch_phosh.sh) — bucle supervisor: fallo de
arranque (<60s de vida) reintenta hasta 5 veces; sesión larga que muere se relanza siempre.
[usr/local/bin/phosh-session.sh](usr/local/bin/phosh-session.sh) — sin `exec`: al salir phosh
`kill $PPID` (phoc) → cae dbus-run-session → launch_phosh relanza la sesión entera.

### 6. La cadena local.d corría EN SERIE con esperas largas
`local.d` ejecuta por orden alfabético y secuencial; esperas de 8-15s (`zy-usb-route`, `zzw-powerkey`,
`zzz-gps`) retrasaban/arriesgaban lo posterior (Phosh). Un cuelgue de cualquiera = móvil a medias.
**Fix**: esos pasan su trabajo a background. (`touch-power` y los pokes pwrap de `zz-consys-bt` se
quedan en serie A PROPÓSITO: el pwrap no tiene lock y no conviene solaparlos.)

## Botón de encendido (mt6582-powerkey v3.1) — ver commits e27a1a0 / 094d0de
- **EVIOCGRAB** = dueño único del evdev: phosh/phoc/gsd-power ya no ven KEY_POWER (antes 3 actores se
  pisaban; phosh apagaba el dpms y NUNCA lo re-encendía —su inhibit de logind da AccessDenied en la
  sesión aislada—; gsd-power intentaba SUSPEND en cada toque).
- Toque corto: OFF = flag `/run/mt6582-screen-off` + lock; ON = **despertar directo a la sesión**
  (medido: no existe "despertar al lockscreen" desde fuera de phosh; sin PIN es equivalente).
- Menú (toque largo): powermenu v2 espera el desbloqueo antes de mostrar los botones (phosh los
  desactiva bloqueado). Log de pulsaciones en `/tmp/powerkey.log`.

## Backlight: el LCD se quedaba encendido con `duty=0` (LA queja visible del usuario)
El daemon `mt6582-backlight.py` apagaba poniendo **solo** `PWM_DUTY=0` (reg +0xa0). En este IP eso
**no corta la luz** con el bloque PWM habilitado (bit EN `0x10000` en reg +0x00): el usuario seguía
viendo el LCD iluminado. **Fix**: al existir el flag, además de `duty=0` se **DESHABILITA el PWM**
(`EN &= ~0x10000`), y se re-habilita al quitar el flag. Verificado a nivel registro: con flag
`EN=0x00000000`, sin flag `EN=0x00010000`.

## Incidente aparte: `apk add wlopm` corrompió libs de core → udev segfault
Instalar `wlopm` arrastró una transacción apk grande (upgrade de libblkid, eudev-dev, musl-fts…) y
dejó **musl/kmod-libs mal escritas** → `udevd`/`udevadm` **segfault en cualquier invocación** (incl.
`--version`), mientras `lsblk`/`findmnt` (mismas util-linux libs pero sin libkmod) funcionaban. Sin
udev-trigger, NM marcó `wlan0 unmanaged` y la sesión GUI se degradó. **Fix**: `apk fix musl libeconf
kmod-libs libblkid libmount libuuid libsmartcols eudev` → udevadm vuelve (v251) → reboot limpio.
Lección: en este móvil, `apk add` de algo trivial puede tocar libs de core; tras un `apk add`
verificar `udevadm --version` y `rc-status | grep crashed`.

## Cómo verificar un boot sano
```
dmesg | grep "random: crng"        # debe ser < 10s
rc-status | grep -E "crashed"      # vacío
pgrep -x phoc; pgrep -x phosh      # ambos vivos (phosh se busca por /proc/*/comm, no pgrep -x)
rc-service udev status             # started
nmcli -t dev status | grep wlan0   # connected
ip -4 addr show usb0 | grep inet   # 172.16.42.1
```
