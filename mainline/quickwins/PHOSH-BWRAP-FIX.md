# Fix: Phosh no arranca — `bwrap` setuid rompe glycin (2026-06-18)

## Síntoma
Al bootear, **Phosh no carga** (pantalla en negro / vuelve a tty). El display y la GPU están
PERFECTOS (card1=mediatek-drm, lima, phoc arranca, `Output DSI-1 added`). phoc lanza phosh y
**phosh muere con SIGABRT** en `/tmp/phosh-boot.log`:
```
Failed to load .../Adwaita/scalable/status/image-missing.svg: Loader process exited early
Command: "bwrap" "--unshare-all" ... "/usr/libexec/glycin-loaders/2+/glycin-svg"
Bail out! Gtk:ERROR ...
phoc-server: Session terminated: Child process killed by signal 6 (SIGABRT)
```

## Causa raíz
GTK4 carga imágenes con **glycin**, que aísla cada loader (svg/jxl/…) en un sandbox **`bwrap`**.
El `bwrap` de Alpine estaba instalado **setuid root** (`-rwsr-xr-x`), pero esa build de bubblewrap
**no soporta el modo setuid** → al ejecutarse setuid se niega:
```
bwrap: setuid use of bubblewrap is not supported in this build
```
El loader muere, glycin no carga el SVG, GTK hace `assert` y **aborta phosh entero**. NADA que ver
con el kernel/display/WiFi. (User namespaces SÍ están disponibles: `/proc/sys/user/max_user_namespaces`=7607.)

## Fix
Quitar el bit setuid de `bwrap` → usa **user namespaces** no privilegiados (lo correcto para esa build):
```sh
chmod u-s /usr/bin/bwrap
```
Verificación: `su - sxmo -c 'bwrap --unshare-all --dev /dev /bin/true'` ya NO da el error de setuid;
phosh + squeekboard arrancan.

**Persistente**: añadida la línea `chmod u-s /usr/bin/bwrap 2>/dev/null` al principio de
`/etc/local.d/zzzz-phosh.start` (se reaplica en cada boot, por si un `apk upgrade bubblewrap`
restaura el setuid). Ver `zzzz-phosh.start` en este directorio.

## Secundario (cosmético, NO crashea)
El fondo de pantalla por defecto es un **JXL** y `glycin-jxl` no lo carga (falta `libjxl`):
`Failed to load background image ... build libjxl from source`. Phosh funciona igual; para quitar el
warning, poner un fondo PNG/JPG o `apk add libjxl` si está disponible.
