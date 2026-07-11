# Maemo Leste en BQ Aquaris E4.5 (krillin) — Estado 2026-07-11

## LOGRO: GUI de Maemo Leste VISIBLE en el krillin

### Funcionando:
- Kernel 7.0.12 mainline + DRM MediaTek (mtk_drm, mtk_dsi, mtk_disp_ovl/rdma/color)
- DSI + panel HX8389 (540x960, sin timeouts despues de fix del CRTC vblank wait)
- GPU Mali-400 (driver lima) + AIGLX + EGL (Mesa 25.0.7)
- Xorg con modesetting + card1 (mediatek-drm)
- hildon-desktop + hildon-home + hildon-status-menu + mce + dsme + sapwood
- OpenRC (NO sysvinit) gestionando servicios
- SSH (openssh-server) + internet por USB (NAT Pi)
- Teclado virtual (hildon-input-method)
- Terminal (osso-xterm), Control Panel, App Manager, File Manager (hildon-fm1)
- Swap 512MB + zram (evita OOM)
- 745+ paquetes instalados (hildon-meta-core)

### Servicios OpenRC creados:
- krillin-usb: SSH + red USB + DNS + gateway al boot
- krillin-norotate: desactiva rotacion de hildon-desktop via gconf
- krillin-rotate-fix: fuerza xrandr --rotate normal tras hildon-desktop

### Problema pendiente:
- hildon-desktop fuerza rotacion right (landscape 960x540) al arrancar
- El DSI no soporta rotacion dinamica (mtk_dsi_stop -> vblank wait timeout)
- Workaround: xrandr --rotate normal (portrait 540x960) tras boot
- Fix real: parchear mtk_crtc.c (atomic_disable) para no esperar vblank

### Configuracion clave:
- Rootfs en SD mmcblk1p3 (ext4, UUID=1b8a35d9-...)
- OpenRC runlevels: sysinit (krillin-usb) + default (ssh, hildon-desktop, etc)
- fstab con UUID (inmune al bug mtk-sd mmcblk swap)
- Boot via menupick (boot-menupick-dsifix.img en sector 83968 eMMC)
