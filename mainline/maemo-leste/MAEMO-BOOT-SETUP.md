# Maemo Leste — Configuracion de Boot para krillin

## Como arrancar Maemo en el krillin

### Requisitos:
1. SD card con 3 particiones:
   - p1: pmOS (ext4, LABEL=pmos)
   - p2: SHARED (vfat)
   - p3: Maemo (ext4, LABEL=maemo, UUID=1b8a35d9-1276-4d73-a8d7-ac5ab010ddc7)

2. eMMC sector 83968: boot-menupick-dsifix.img

3. Kernel 7.0.12 con:
   - DRM MediaTek (mtk_drm_drv, mtk_dsi, mtk_disp_ovl/rdma/color)
   - Panel HX8389 (panel-himax-hx8389.c)
   - GPU lima (Mali-400)
   - DSI fix: mtk_crtc.c atomic_disable usa msleep en vez de vblank wait
   - WiFi/BT desactivados en DTS (para logs limpios)

### Instalacion de Maemo en la SD:


### Acceso SSH por USB:
- Telefono: 172.16.42.1
- Pi (gateway): 172.16.42.2
- NAT en la Pi: ejecutar pi-nat-usb.sh

### Problema conocido:
- hildon-desktop fuerza rotacion right (landscape) que el DSI no soporta
- Workaround: krillin-rotate-fix fuerza xrandr --rotate normal tras boot
- La GUI se ve en portrait (540x960) en vez de landscape (960x540)
