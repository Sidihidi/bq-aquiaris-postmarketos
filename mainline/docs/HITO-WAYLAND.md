# HITO — Wayland (weston) sobre simpledrm — FUNCIONA y SUPERFLUIDO (2026-06-17)

Un compositor **Wayland completo (weston 14) corre fluido y táctil** sobre el kernel
mainline 7.0.12 del BQ Aquaris E4.5, usando **simpledrm + renderer Pixman (software)**.
Confirmado en hardware por el usuario: "funciona perfecto y superfluido".

**Implicación clave:** si la composición Wayland por software ya va fluida a qHD, el
ancho de banda de memoria del MT6582 es suficiente → **Phosh por Pixman es viable sin
GPU**. lima (Mali-400) sería un extra, no un requisito.

## Lo que se vio
- `weston` + `weston-desktop-shell` + `weston-terminal` + `weston-keyboard` activos.
- Output DRM **540x960@60** (shadow framebuffer), color sRGB.
- libinput asoció **event0 = keypad (ID_INPUT_KEY)** y **event1 = FT5336 (ID_INPUT_TOUCHSCREEN)**.
- **simpledrm soporta ATOMIC modesetting** (no solo legacy) → wlroots/phoc/Phosh OK.

## Receta reproducible (Alpine en el teléfono)
1. **Kernel**: `CONFIG_DRM_SIMPLEDRM=y`, `CONFIG_FB_SIMPLE` off, `mt6582-dispfix` → `obj-y`
   (mantiene el fix del OVL). Da `/dev/dri/card0` + `/dev/fb0` (DRM_FBDEV_EMULATION).
2. `apk add weston weston-backend-drm weston-shell-desktop weston-terminal seatd eudev libinput mesa-dri-gallium`
3. **eudev es OBLIGATORIO** (libinput lo necesita; sin él weston aborta con "no input devices"):
   `/sbin/udevd --daemon; udevadm trigger; udevadm settle`. ← **Esto también arregla el
   táctil en X11** (la razón por la que no controlaba la GUI jwm era la ausencia de udev).
4. `seatd` (daemon) + `LIBSEAT_BACKEND=seatd`.
5. `XDG_RUNTIME_DIR=/run/wl` (mkdir, chmod 700).
6. **Matar X11/jwm antes** (libera `card0`).
7. `weston --backend=drm-backend.so --renderer=pixman --idle-time=0`.

## Siguiente: PHOSH
- `phoc` (compositor wlroots de Phosh) + `phosh` + `squeekboard` (teclado) con
  `WLR_RENDERER=pixman` y `WLR_DRM_NO_ATOMIC` no necesario (simpledrm soporta atomic).
- **zram + swap** (1 GB es el mínimo para Phosh).
- Persistencia: servicio OpenRC que arranque udevd + seatd + el compositor al boot
  (más el daemon del táctil y el power-cycle VGP1 que ya tenemos).
- Escala HiDPI 540×960 → `scale 1.5`.
