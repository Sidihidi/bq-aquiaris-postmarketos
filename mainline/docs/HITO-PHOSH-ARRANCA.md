# HITO: Phosh arranca en el BQ Aquaris E4.5 (MT6582 "krillin") — 2026-06-17

**Phosh 0.55.0, compilado desde fuente, corriendo** sobre kernel mainline 7.0.12
en Alpine 3.24 (armhf), sobre `simpledrm` (`/dev/dri/card0`). Un teléfono de 2014
con la GUI de Librem 5 / PinePhone.

## Estado verificado (log de phosh)
- `phosh-main-Message: Phosh ready after 4.04s`
- `card0-Unknown-1: connected 540x960` (142mm x 254mm) — pantalla del BQ
- `Running compositor on wayland display 'wayland-0'` — phoc sobre simpledrm
- Táctil: `ft5336_touch` (driver userspace, inyecta por uinput) leído por libinput
- Procesos: `dbus-run-session phoc -E /usr/local/libexec/phosh` → `phoc` → `phosh`
- Warnings NO fatales (= servicios que aún faltan): NetworkManager, UPower,
  Bluetooth (bluez), org.gnome.Calls, org.gnome.SessionManager, Xwayland

## EL BLOQUEO CLAVE: schema `org.gnome.shell.keybindings`
gnome-shell NO está empaquetado en Alpine armhf, pero phosh hace
`g_settings_new("org.gnome.shell.keybindings")` y lee keys concretas. Sin el
schema, phosh **aborta con signal 6 (SIGABRT)** en `GLib-GIO-ERROR ... Settings
schema 'org.gnome.shell.keybindings' is not installed / does not contain a key`.

phosh 0.55 lee **EXACTAMENTE estas 8 keys** de ese schema (extraídas de los
`#define KEYBINDING_KEY_*` en `src/{screenshot-manager,home,top-panel,brightness-manager}.c`):

    screenshot, toggle-overview, toggle-application-view, toggle-message-tray,
    screen-brightness-up, screen-brightness-down,
    screen-brightness-up-monitor, screen-brightness-down-monitor

**Solución:** crear un schema mínimo con esas 8 keys (tipo `as`, default `[]`),
que phosh solo necesita poder LEER (no que tengan valor). Ver
`zz-phosh-shell.gschema.xml`. Instalar en `/usr/share/glib-2.0/schemas/` +
`glib-compile-schemas`.

> Pista de depuración: cada relanzamiento de phosh delata la SIGUIENTE key/schema
> que falta en el log. NO ir una a una — extraer todas del source de un golpe con
> `grep -hE '#define [A-Z_]*KEY[A-Z_]* +"' src/*.c`. (Intentar bajar el schema
> entero de gnome-shell gnome-48 NO sirve: trae enums de mutter que rompen la
> compilación, y de hecho NO contiene las keys `screen-brightness-*` — esas las
> espera phosh de un gnome-shell muy reciente que no tenemos.)

## Procedimiento de lanzamiento
1. **Servicios previos**: `udevd` (eudev, OBLIGATORIO para libinput/táctil),
   `seatd -g seat`.
2. **Usuario no-root `sxmo`** (uid 1000, grupos video/input/seat/audio); phosh/phoc
   NO corren como root.
3. `XDG_RUNTIME_DIR=/run/user/1000` (chown sxmo, chmod 700).
4. `launch_phosh.sh` exporta `WLR_RENDERER=pixman` (sin GPU), `LIBSEAT_BACKEND=seatd`,
   `XDG_DATA_DIRS=/usr/local/share:/usr/share`, y hace:
   `exec dbus-run-session phoc -E /usr/local/libexec/phosh`
   (phosh está en **libexec**, NO bin; usar ruta absoluta.)
5. Lanzar: `su - sxmo -c 'sh /tmp/launch_phosh.sh'`

## Dependencias (apk) además de phosh/phoc compilados
`feedbackd`, `gnome-settings-daemon` (provee `org.gnome.settings-daemon.plugins.power`),
`gnome-backgrounds`, `squeekboard`, `adwaita-icon-theme`, `mutter`, `eudev`, `seatd`.
Lista completa: **apk-installed.txt** (736 paquetes).

## Reproducir el estado (backup adjunto)
- `apk-installed.txt` — paquetes (`apk add ...` los que falten)
- `usrlocal.tar.gz` — binarios compilados (phosh/phoc/...): `tar xzf usrlocal.tar.gz -C /`
- `zz-phosh-shell.gschema.xml` — fix de schema → `/usr/share/glib-2.0/schemas/` + `glib-compile-schemas`
- `launch_phosh.sh` — lanzador

## Próximos drivers (cada warning = una pieza que falta)
- **UPower** → icono de batería (ya leemos batería% por sysfs; falta el daemon)
- **backlight** → brillo (phosh ya trae el slider; busca `/sys/class/backlight`)
- **NetworkManager** → WiFi/datos en quick-settings
- **power button** (PMIC pwrkey) → bloquear/apagar pantalla
- **suspend/PM** → ahorro de energía (s2idle)
