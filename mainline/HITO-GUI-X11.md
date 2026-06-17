# HITO — GUI X11 táctil en mainline (2026-06-17) ✅

**Escritorio gráfico X11 con pantalla táctil funcionando en el BQ Aquaris E4.5
(MT6582) sobre Linux mainline 7.0.12 + Alpine 3.24.** Verificado en hardware:
arrancó, **fluido en 2D**, y el táctil responde al dedo dentro de la GUI.

## Stack
- **Xorg** con driver **fbdev** sobre `/dev/fb0` (simplefb 540×960, 16bpp RGB565).
- **Táctil**: `xf86-input-evdev` sobre `/dev/input/event0` (el daemon `ft5336_touch`).
  Xorg log: `Using input driver 'evdev' for 'touch'` · `Vendor 0x5a Product 0x14` (FT5336/Truly).
- **WM**: `jwm` (matchbox-window-manager NO está en Alpine 3.24 → jwm/fluxbox/icewm).
- **Teclado en pantalla**: `matchbox-keyboard`.
- Dispositivos **explícitos** en `xorg.conf` (`AutoAddDevices off`) → no necesita udev/eudev.

## Cómo arrancarlo (en el teléfono, desde SSH)
`apk add xorg-server xf86-video-fbdev xf86-input-evdev xinit xterm jwm matchbox-keyboard ttf-dejavu xsetroot`
Config en repo: `mainline/rootfs/x11/{xorg.conf,xinitrc,start-x.sh}`. Arrancar:
`/root/start-x.sh` (lanza `X :0 vt1 -keeptty -config /etc/X11/xorg.conf` + jwm + teclado).

## Notas
- (EE) inofensivos: módulo `kbd` ausente (sin teclado físico) y GLX/swrast (sin OpenGL;
  2D va perfecto). Para apps GL haría falta `mesa-dri-gallium`/swrast.
- Render por software (pixman) — aún así fluido para jwm/teclado/terminal.
- **Pendiente**: hacer X persistente al boot (servicio local.d), y calibrar ejes del
  táctil si hiciera falta (matriz en xorg.conf).

## ⚠️ BLOQUEANTE OPERATIVO confirmado 2x: SIN CARGA en mainline
El teléfono **se quedó sin batería dos veces a mitad de sesión**: en mainline NO hay
driver de carga (charger MT6323), así que el USB da datos pero NO carga → drena y muere.
**Mitigación actual: cargar el teléfono APAGADO** (el LK/preloader carga con USB/cargador).
**Próximo driver imprescindible: `carga-usb`** (ver ROADMAP-DRIVERS.md) — sube a máxima
prioridad real porque limita toda sesión de trabajo.
