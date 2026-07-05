# Estabilización del boot (2026-07-05) — RESUELTO

Síntomas reportados (intermitentes): (1) a veces la GUI no llegaba a arrancar, (2) a veces se perdía
la red/SSH ("No route to host") hasta hacer power-cycle. Validado con 2 reboots limpios tras los fixes:
boot completo en ~35s, `crng init done` a los ~7s, nada en `crashed`.

## Causas raíz encontradas (todas reales, se solapaban)

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
Si phoc moría al arrancar (carrera DRM/seatd), no había GUI y nadie reintentaba.
**Fix**: [usr/local/bin/launch_phosh.sh](usr/local/bin/launch_phosh.sh) — bucle supervisor: fallo de
arranque (<60s de vida) reintenta hasta 5 veces; sesión larga que muere se relanza siempre (la GUI
vuelve sola). Log en `/tmp/phosh-boot.log`.

### 6. La cadena local.d corría EN SERIE con esperas largas
`local.d` ejecuta por orden alfabético y secuencial: `zy-usb-route` (hasta 15s), `zzw-powerkey`
(sleep 8), `zzz-gps` (hasta 6s) retrasaban/arriesgaban todo lo posterior (Phosh). Un cuelgue de
cualquiera = móvil a medias (los DOS síntomas a la vez).
**Fix**: todos esos pasan su trabajo a background. (`touch-power` y los pokes pwrap de
`zz-consys-bt` se quedan en serie A PROPÓSITO: el pwrap no tiene lock y no conviene solaparlos.)

## Qué NO se tocó
- `zzzz-phosh.start` (la espera de `/dev/dri/card1`+seatd de hasta 90s es necesaria y ya lanzaba al
  final igualmente).
- `00-pstore-save.start`, `zzv-battery`, `zzy-charge`, `zzzzz-bt-agent` (ya eran correctos/async).

## Cómo verificar un boot sano
```
dmesg | grep "random: crng"        # debe ser < 10s
rc-status | grep -E "crashed"      # vacío
pgrep -x phoc                      # corriendo
ip -4 addr show usb0 | grep inet   # 172.16.42.1
```
