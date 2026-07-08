# 2026-07-08 · Menú multiboot visual (menupick) — FUNCIONA

## TL;DR
Menú de selección de SO **en la pantalla del móvil**, navegable con Vol+/- y Power.
**FUNCIONA en HW.** Sin U-Boot (no tiene pantalla). Usa el kernel mainline + initramfs
con un programa C que dibuja en el framebuffer DRM y lee los botones por evdev.

## Qué se hizo

### menupick.c (~200 LOC + font5x7.h)
Programa C estático que:
- Dibuja un menú en `/dev/fb0` (540×960 32bpp, DRM ya cargado por el kernel)
- Lee `/dev/input/event0` (mt6779-keypad: Vol+ = KEY_VOLUMEUP=115, Vol- = KEY_VOLUMEDOWN=114)
- Lee `/dev/input/event1` (mtk-pmic-keys: Power = KEY_POWER=116)
- Navega con Vol+/-, selecciona con Power
- Auto-boot tras 10s de inactividad
- Devuelve por stdout el índice del SO elegido (0, 1, ...)
- Compilado con `arm-linux-gnueabihf-gcc -static -O2` (491KB)

### init-menupick.sh (init del initramfs)
- Monta proc/sys/dev
- Ejecuta `menupick` (espera a que fb0 + event0 aparezcan)
- Según la elección: `switch_root` a pmOS (mmcblk1p1) o `kexec` a Android
- Fallback de emergencia: dropbear por USB (igual que el initrd-sd original)

### initramfs-menupick (2.8MB)
- Basado en initrd-sd (busybox + dropbear)
- Añadido: menupick + kexec + libs dinámicas (musl, libzstd, liblzma, libz)
- Empaquetado como boot.img con cabecera MTK (abootimg)

### Android boot.img
- `lineage13-boot.img` (4.7MB) copiado a `/boot/android-boot.img` en la SD del móvil
- El init-menupick lo lee y hace `kexec` del kernel 3.10

## Estado
- ✅ Menú funciona: visible en pantalla, navegable con botones
- ✅ pmOS arranca correctamente desde el menú
- ✅ boot-menupick2.img flasheado (con kexec para Android)
- 🟡 Android via kexec: pendiente de probar (boot.img copiado, initramfs listo)
- 🟡 Maemo Leste: pendiente de portear (entrada reservada en el init script)

## Cómo probar
1. Encender el móvil → aparece el menú (azul oscuro, dos entradas)
2. Vol+/- para navegar, Power para seleccionar
3. Auto-boot pmOS a los 10s si no se selecciona nada
4. Para Android: seleccionar "Android" → kexec del lineage13-boot.img

## Archivos en el repo (`mainline/bootmenu/`)
- `menupick.c` — el programa del menú
- `font5x7.h` — fuente bitmap 5x7
- `init-menupick.sh` — init del initramfs

## Compilar desde cero
```sh
# En la Pi (cross-compile armhf):
arm-linux-gnueabihf-gcc -static -O2 -o menupick menupick.c
# Construir initramfs (ver init-menupick.sh para instrucciones)
# Empaquetar como boot.img: ver ~/mainline/pkg (abootimg + mtk_hdr.py)
```

*Co-autor: ZCode (glm-5.2).*
