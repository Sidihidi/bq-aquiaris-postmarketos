# Handoff — continuar en casa (2026-06-24)

Estado al cierre de la sesión Mac. Tres frentes abiertos: **Phosh con plugins (build en WSL)**, **brillo por slider**, **WiFi (DHCP + crash de `iw scan`)**.

> ⚠️ El móvil quedó colgado al final de la sesión (ver §2 — `gsd-power` lo suspendió). **Power-cycle y arranca normal**; nada quedó persistente.

---

## 1. Compilar Phosh (con plugins) en WSL

**Por qué:** el Phosh 0.55 instalado se compiló **SIN plugins** → no hay toggles de WiFi/BT en la UI. Hay que recompilar con `-Dplugins=true`. La infra de build (pmbootstrap) la dejé montada y validada en la Mac (colima); en WSL es idéntico.

### Prerequisitos en WSL
- **WSL2** (no WSL1 — hace falta `binfmt` para cross-arch armhf).
- Ubuntu. Activa systemd en WSL (`/etc/wsl.conf` →`[boot]` / `systemd=true`, luego `wsl --shutdown`) para que `binfmt` funcione.
- `sudo apt update && sudo apt install -y git python3 python3-venv kpartx qemu-user-static binfmt-support`

### Montar pmbootstrap
```bash
git clone https://gitlab.postmarketos.org/postmarketOS/pmbootstrap.git
cd pmbootstrap
```
> OJO: `gitlab.com/postmarketOS` es un **stub viejo** que ya no sirve. Usa **`gitlab.postmarketos.org`**.

### init (casi todo por defecto)
```bash
yes "" | python3 pmbootstrap.py init
```
Pregunta work-path, pmaports-path, canal, device, UI, timezone, locale, hostname. Para **compilar el paquete** todo el default sirve (device=`qemu-amd64`; la arch real la forzamos al construir).

> **Caveat de canal:** el default es `systemd-edge`, pero tu móvil es **OpenRC**. Si lo vas a instalar, mejor elige canal **`edge`** (sin systemd) cuando el init lo pregunte, en vez de dejar el default. Para solo *compilar y mirar* da igual.

### Compilar phosh para armv7 (armhf)
```bash
python3 pmbootstrap.py build phosh --arch=armhf
```
El APKBUILD de pmaports **ya trae los plugins** (confirmado: tiene el trigger `/usr/lib/phosh/plugins/prefs`). El `.apk` queda en `~/.local/var/pmbootstrap/packages/<canal>/armhf/`.

### ⚠️ El problema de versión (decidir ANTES de instalar)
pmaports `edge` = phosh **9999 (git, lo último)**, mucho más nuevo que tu **0.55**. Instalar la edge arrastra deps nuevas (gtk4 / libadwaita / gmobile) que tu rootfs quizá no tiene → puede romper la UI. Tres caminos, de menos a más trabajo:

- **A — recomendado:** recompilar la **MISMA 0.55** con plugins. Es lo más limpio (los plugins solo estaban desactivados). Necesitas el APKBUILD de la 0.55 → en pmaports `git log`/`git checkout` a la fecha en que existía la 0.55, o ajusta `pkgver` en el APKBUILD.
- **B:** subir TODO el stack de la UI a edge (rebuild de phosh + deps). Más trabajo y más riesgo.
- **C — el más rápido si aún tienes el árbol:** si la 0.55 la compilaste desde fuente (no pmbootstrap), solo reañade `-Dplugins=true` a esa misma build de meson y recompila. No hace falta pmbootstrap para nada.

### Instalar en el móvil
```bash
# copiar el .apk al móvil, luego:
apk add --allow-untrusted /ruta/phosh-0.XX-rN.apk
```

---

## 2. Brillo por slider — diagnóstico y plan

### Qué descubrimos (definitivo, con pruebas)
- Moviste el slider muchas veces y **NO cambió NADA**: ni `/sys/class/backlight/backlight/brightness` (siguió en `8`), ni `/run/mt6582-bl-pct` (siguió en `80`), ni el shim recibió ningún `ON_SET` nuevo. → **El slider de Phosh no tiene backend.**
- El **shim** (`mt6582-bl-shim.py`, lanzado por `phosh-session.sh`) era el **enfoque equivocado**: Phosh ni le habla (0 GET, 0 SET). Y encima **squatea** el nombre D-Bus `org.gnome.SettingsDaemon.Power`.
- `gsd-power` **SÍ está instalado** (`/usr/libexec/gsd-power`, paquete `gnome-settings-daemon`) pero **no corría** — porque el shim le robaba el nombre.
- Maté el shim y arranqué gsd-power a mano → **tomó el nombre** (`:1.17`). **Ese es el backend que el slider de Phosh espera.** Los warnings (`SessionManager not provided`, `lid switch`) son inofensivos.

### ⚠️ Por qué se colgó el móvil
`gsd-power` **no gestiona solo el brillo**: también hace **power-management (idle / dim / suspend)**. Lo dejé corriendo ~10 min mientras trabajaba en la VM y, al quedar el móvil idle, **intentó SUSPENDERLO**. En mainline el **suspend del MT6582 está roto** → cuelgue → cae el USB (`No route to host`). Tras power-cycle, normal (gsd-power lo arranqué a mano, no persiste).

### El fix (para hacer en casa)
En `phosh-session.sh`, en vez del shim, lanzar **gsd-power con suspend/idle DESACTIVADO**:
```bash
# en la sesión, ANTES de exec phosh:
gsettings set org.gnome.settings-daemon.plugins.power sleep-inactive-ac-type 'nothing'
gsettings set org.gnome.settings-daemon.plugins.power sleep-inactive-battery-type 'nothing'
gsettings set org.gnome.settings-daemon.plugins.power idle-dim false
/usr/libexec/gsd-power &
```
Y ajustar `mt6582-backlight.py` para que lea **`/sys/class/backlight/backlight/brightness`** (donde gsd-power escribe) en vez de `/run/mt6582-bl-pct`.
Cadena final: **slider → gsd-power → /sys → daemon Python → PWM real**. (Quitar el shim del `phosh-session.sh`.)

### Cómo verificarlo en casa
1. Arranca, quita el shim, lanza gsd-power con los gsettings de arriba.
2. Abre el panel de Phosh y mueve el slider mientras miras:
```bash
watch -n1 'echo "sys=$(cat /sys/class/backlight/backlight/brightness) run=$(cat /run/mt6582-bl-pct)"; pgrep -f gsd-power'
```
3. Si `/sys` cambia al mover el slider → backend OK. Entonces el daemon Python (leyendo `/sys`) cierra el círculo y el brillo cambia de verdad.
4. **Comprueba que NO se suspende** tras unos minutos idle (los gsettings deben evitarlo).

---

## 3. WiFi — hipótesis: DHCP (sin datos en WPA2) y el crash de `iw scan` van de la mano

**Tu intuición es CORRECTA en el mecanismo:** el crash de `iw scan` y el WPA2-sin-datos **llegan al mismo cuelgue**. Pero hay un matiz honesto: arreglar el cuelgue **no arregla solo el DHCP** — el WPA2-sin-datos tiene además una causa a nivel FW. Análisis con evidencia (ficheros en `mainline/wifi-consys/wifi/`):

### La causa raíz unificada (el CUELGUE)
**El acceso PIO al HIF no tiene timeout, y el guard mira el registro EQUIVOCADO.**
- `mt6582-wifi.c:106` — `rd()` es un `readl` pelado, **sin `_poll_timeout`**.
- `mt6582-wifi.c:186-194` — `wifi_port_read_pio` drena N palabras de `WRDR0` en un bucle **sin chequear vida entre palabras**. Si el data-port se atasca a mitad de burst, **el `readl` cuelga el CPU para siempre** (el bus AHB no completa) → hard-lockup → WDT a 31s.
- `mt6582-wifi.c:117-128` — el guard `wifi_hif_alive` lee `WCIR` (chip-id). **`WCIR` sigue respondiendo `0x6582` aunque `WRDR0` esté colgado** → el guard NO detecta el cuelgue real (solo lo pone al entrar al callback, no entre lecturas del puerto).

Los dos síntomas son el MISMO cuelgue por dos disparadores:
- **`iw scan`:** `.scan` toca el bus (`wifi_hstcr` hace `rd/wr` *antes* de mover datos) mientras el rx_thread lee `WRDR0` con el `hif_lock` cogido → en el estado frágil post-WPA2 esa transacción no completa → cuelgue instantáneo.
- **WPA2 sin datos:** el FW nunca sube tramas DATA al puerto (se desconecta a ~30s con beacon-timeout `0x1b`), así que `rx_packets` se queda en 2.

**Matiz vs mi hipótesis inicial:** NO es que "PIO no aguante el bulk" — PIO mueve bulk perfectamente (el OPEN navega, y el firmware se DESCARGA por este mismo PIO, 160KB en chunks). El defecto es **`readl` sin timeout + guard en el registro equivocado**, no un problema de timing del bulk.

### Lo que YA está descartado (no perseguir de primeras)
- **enc_status 7 vs 6:** en el código el join manda `7` (KEY_ABSENT) y nunca lo sube a `6` tras instalar claves (`mt6582-wifi.c:943-950`; `.add_key` no re-emite `SET_BSS_INFO`). Es **literalmente cierto**. PERO **ya probaste 6 y 7 en HW y el DHCP falló idéntico** (`handoff-wifi-0624/HANDOFF-0624.md`) → **no es la causa**.

### La pista más fuerte para el DHCP (causa FW, aparte del cuelgue)
- **`set_default_key` nunca lo llama wpa_supplicant** → el FW no tiene TX-default-key → el **DHCP DISCOVER sale en claro** → el AP lo descarta. (~30% probable, explicación de TX.) `wifi_cfg_set_default_key` existe (`:1233-1250`) pero los logs muestran que no se invoca.
- Otras (~25%): secuencia de join incompleta vs downstream (falta `SET_BSS_RLM_PARAM 0x1d` standalone, `UPDATE_WMM 0x21`); o el `0x1b` (beacon-timeout) como killer real si el FW suelta el BSS.

### 🔧 El fix a probar PRIMERO (≈5 líneas, máxima señal)
**Acotar `wifi_port_read_pio` (`mt6582-wifi.c:186-194`)** para que un data-port colgado ABORTE en vez de colgar el CPU:
```c
for (i = 0; i < words; i++) {
    /* si el data-port se cuelga a mitad de burst, el readl cuelga el AHB.
     * Re-sondear WCIR (registro estatico, no toca el FIFO) cada palabra:
     * si el core murio, abortar en vez de colgar el CPU. */
    if ((rd(w->hif, MCR_WCIR) & WCIR_CHIP_ID) != WIFI_CHIP_ID_6582) {
        w->started = false;
        memset(&p[i], 0, (words - i) * 4);
        return;
    }
    p[i] = rd(w->hif, MCR_WRDR0);
}
```
(Igual en `wifi_port1_read_pio`, `:345-353`.) **Esto mata el brick-on-scan y te deja instrumentar el data-path SIN power-cycle.** No arregla el DHCP solo — pero **desbloquea poder depurarlo** sin reiniciar a cada crash.
**Caveat honesto:** si se cuelga TODO el slave AHB (no solo el data-port), hasta el `WCIR` colgaría y esto no salva — ese caso solo lo cierra PDMA+IRQ (el TODO de `0x11000180`). Pero la evidencia previa es que `WCIR` sigue vivo mientras `WRDR0` cuelga → este es justo el caso que cubre.

**+ diagnóstico en el mismo flash:** añade un `dev_info` en `wifi_rx_drain` (`:755`) logueando `l0/l1/pt` cada pasada → te dice si el OFFER del DHCP **llega al HIF** (FW entrega, driver falla) o **nunca llega** (FW). Es el dato que desempata el frente DHCP.

### Tests on-device (build+flash con `~/wifi-iter-w.sh` en la Pi)
1. **Scan desconectado:** boot limpio → `ip link set wlan0 up` → `iw dev wlan0 scan`. Debe devolver resultados (o `-ENODEV`) y el móvil **NO reiniciarse**.
2. **Scan conectado (el que crasheaba):** conecta WPA2 (deja que **wpa_supplicant** lo haga, NO uses `iw scan` para conectar), luego `iw dev wlan0 scan` + `dmesg | tail -40`. CONFIRMA (a) si NO hay hard-lockup → `boot_reason` queda en 1, no 4.
3. **Aislar el RX:** con el móvil ya sobrevivible, `udhcpc -i wlan0 -t 3` mientras miras `dmesg | grep -E "rx_drain|0x1b"`. Si ves tramas DATA llegando pero `rx_packets` sigue 2 → bug del driver (`wifi_rx_data:731`). Si NO llega ninguna DATA antes del `0x1b` → es FW (persigue `set_default_key` y la secuencia de join vs el downstream de la Pi).

> ⚠️ Lee `/sys/fs/pstore/console-ramoops-0` **ANTES** de reiniciar tras un crash — es la única ventana al cuelgue mudo.
