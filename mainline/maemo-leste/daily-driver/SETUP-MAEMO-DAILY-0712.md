# Maemo Leste como daily-driver en el krillin — setup 0712

> El rootfs Maemo Leste del dual-boot venía **pelado** (solo terminal + gestor de apps). Esta sesión
> lo convirtió en un móvil usable. Mismo kernel/dtb que pmOS (los drivers de kernel cargan igual);
> todo lo de aquí es **userspace de Maemo** (Debian/hildon/mce), distinto del de pmOS (Alpine/phosh).
> ⚠️ CUIDADO CON LA RAM: 1GB. Meter muchos daemons + apt install + abrir apps a la vez = **thrash/cuelgue**
> (paso una vez). Escalonar, y vigilar `free -m`/`load`. ZRAM (512M) ayuda pero no salva de un pico grande.

## ✅ Lo que quedó FUNCIONANDO
| Cosa | Cómo |
|---|---|
| **Shell completo** | `apt install hildon-meta` (482+ pkgs): teclado (`hildon-im-fkb`), barra de estado (`hildon-status-menu`), reloj (`clockd`/`clock-ui`), bloqueo (`osso-applet-devicelock`), applets de ajustes (display/idioma/perfiles), calculadora, contactos, email (modest), calendario (qalendar), framework multimedia (mafw). Reiniciar sesión tras instalar. |
| **Apps en el menú** | los `.desktop` van en `/usr/share/applications/hildon/`; el menú los toma vía `/etc/xdg/menus/hildon.menu` (`<Include><All/>`). |
| **Texto traducido (no placeholders)** | RAÍZ: `/etc/osso-af-init/af-defines.sh` exportaba `LANG/LC_MESSAGES/LC_TIME=en_GB` pero solo esta generado el locale **`en_GB.UTF-8`** → `setlocale` falla → gettext devuelve la clave cruda (`wdgt_va_12h`, `calc_ap_calculator`). FIX: `sed -i -E 's/=en_GB([^.]\|$)/=en_GB.UTF-8\1/g' af-defines.sh` + reiniciar la sesión X (`rc-service xorg stop; ... start`; el `restart` es no-op). Las traducciones YA estan en `/usr/share/locale/en_GB/LC_MESSAGES/*.mo`. |
| **Batería (icono + %)** | Maemo no trae power_supply → **puente igual que pmOS**: `test_power.ko` (crea batería falsa) + `battery-upower` (python, alimenta los params con el VBAT real del hwmon `mt6323_auxadc`) + `upowerd`. Necesita **polkitd** vivo (upowerd lo exige; su activacion D-Bus falla → arrancar polkitd a mano). `charge-status` reescrito a **python i2c directo** (Maemo no tiene i2c-tools). Applet `status-area-applet-battery`. |
| **Carga** | `fan5405-charge.py` (i2c directo, kick del watchdog cada 10s) — ver `mainline/userspace/usr/local/sbin/`. |
| **Brillo (slider)** | mce en Leste NO aplica el nivel del slider a nuestro backlight (max=10) y su `display_status` es poco fiable. Puente `mt6582-brightness-bridge` (python): guiado por **tklock** (fiable) — `locked`→backlight 0, `unlocked`→nivel del slider (`gconf /system/osso/dsm/display/display_brightness` 1..5 → 2..max). Verificado mecanicamente (5→10, 2→4). **Nota abierta**: puede que el applet Display de la GUI escriba en otra key; confirmar qué escribe. |
| **Botón: bloqueo** | `mce.ini` `PowerKeyShortAction=menu` → cambiado a **`tklock`** (pulsación corta = bloquear/apagar). Necesita **reiniciar mce** (no solo SIGHUP) para aplicar. |
| **WiFi (backend)** | `icd2` + `libicd-network-wpasupplicant` + connui (`connui-conndlgs-wlan`, `connui-iapsettings-wlan`, `hildon-connectivity-wlan`) — TODO instalado. `rc-update add icd2 default`. El HW escanea (4 redes). `osso-wlan`/`conbtdialogs` NO existen como paquete. |
| **Bluetooth** | `apt install bluez bluez-obexd pulseaudio-module-bluetooth` (A2DP) + **`blueman`** (GUI GTK: emparejar/A2DP/enviar archivos; Maemo NO tiene applet BT nativo). `rc-update add bluetooth default`. hci0 UP/powered. Launcher copiado a `hildon/`. |
| **Internet por USB** | NAT desde la Pi (`.38`, usb0=172.16.42.2) → móvil `ip route add default via 172.16.42.2` + `nameserver 1.1.1.1`. Se pierde al reiniciar sesión/icd2 → re-aplicar. En rc.local. |
| tklock GOTCHA | Bloqueado (`tklock=locked`) = mce **desactiva el táctil** (a propósito). Si "el táctil no va", comprobar `get_tklock_mode` y `req_tklock_mode_change string:"unlocked"`. |

## ❌ Pendiente de confirmar en HW (el usuario reportó que fallan desde la GUI)
- **Brillo desde el applet Display**: el slider de la GUI no cambia (el puente sí mapea gconf→backlight; falta ver qué key escribe el applet).
- **WiFi desde la GUI**: escanear/conectar por connui (backend listo; falta validar el flujo GUI).
- **Bluetooth GUI**: usar blueman para emparejar (recién instalado, sin validar).
- **Bloqueo apaga pantalla**: tras el fix del PowerKey + reiniciar mce (sin validar).
- Todo esto = **iteración lenta a ciegas** (necesita los ojos del usuario). Vigilar RAM al probar.

## Persistencia (rc.local + OpenRC)
`/etc/rc.local`: polkitd, `modprobe test_power`, battery-upower, upowerd, brightness-bridge, `hciconfig hci0 up`, ruta+DNS del NAT. Servicios OpenRC `default`: bluetooth, icd2. Scripts en `bin/` de esta carpeta.

## LECCIÓN
No apilar daemons + instalar + abrir apps a la vez en 1GB. Escalonar y medir. El pulido daily-driver de
Maemo es **mucho** userspace y lento de validar; no confundir con el porteo de drivers (objetivo real).

*Sesión principal (Fable 5), 2026-07-12.*
