# 2026-07-08 · Maemo Leste port (en progreso)

## TL;DR
Port de **Maemo Leste** (Hildon Desktop sobre Devuan) al krillin. El 90% del trabajo
(kernel + drivers) ya está hecho del proyecto pmOS. Resto: device package + rootfs +
validar Xorg/lima. **Estado: device package creado, arm-sdk configurado, rootfs build
en curso (compilando modules).**

## Lo que se ha hecho

### 1. Device package `leste-config-krillin` ✅
Copy-adapt de PinePhone (lima/GLES2) + mz617 (sin módem). Ficheros:
- `hildon-desktop.env.leste`: `COGL_RENDERER=egl_xlib`, `COGL_DRIVER=gles2` (lima)
- `70-krillin.ini.leste`: MCE con `iio-accelerometer;iio-als;evdevvibrator;iio-proximity` (sensores del krillin)
- `50-iio-sensors.rules.leste`: mount matrix LSM330 (`0,1,0;1,0,0;0,0,-1`), proximity TMD2772
- `99-krillin.conf.leste`: Xorg driver `modesetting` + Glamor (lima GLES2)

### 2. Board script `krillin.sh` para arm-sdk ✅
- Reusa el kernel mainline 7.0.12 **prebuilt** (no recompila)
- Copia zImage + dtb + modules del build existente
- Copia leste-config-krillin al rootfs
- Registrado en `arm-sdk/sdk` board_map

### 3. arm-sdk configurado ✅
- Clonado `arm-sdk` + `leste-config` en la Pi
- Submódulos inicializados (libdevuansdk)
- `zsh` + `debootstrap` instalados (Debian trixie en la Pi)

### 4. Rootfs build (en curso)
- `make modules` corriendo (necesario para `modules_install` en el rootfs)
- Una vez listo: `zsh sdk; load devuan krillin; build_kernel_armhf; image_pack_dist`

## Cómo seguir (para la próxima sesión)

### Si el build de modules terminó:
```sh
# Verificar:
ls ~/mainline/linux-7.0.12/build-krillin/modules.order
# Si existe, relanzar el build de Maemo Leste:
cd ~/arm-sdk && nohup bash ~/build_maemo.sh > /tmp/maemo-build.log 2>&1 &
# Monitorear:
tail -f /tmp/maemo-build.log
```

### Si el build del rootfs completa:
El output es una imagen SD en `~/arm-sdk/dist/`. Flashear a una SD aparte o a `mmcblk1p3`.

### Punto crítico: validar Xorg + lima
El krillin hoy valida lima bajo **Wayland/Phosh**. Maemo Leste lo necesita bajo **Xorg/Cogl**.
Antes de montar el rootfs completo, test mínimo:
```sh
# En el rootfs de Maemo (chroot o SD):
COGL_RENDERER=egl_xlib COGL_DRIVER=gles2 cogl-info
# Si muestra info de EGL/GLES2 → lima acelera bajo Xorg ✅
# Si falla → caer a software rendering (usable pero lento)
```

## Archivos en el repo (`mainline/maemo-leste/`)
- `krillin-board.sh` — board script del arm-sdk
- `hildon-desktop.env.leste` — COGL config
- `70-krillin.ini.leste` — MCE config (sensores)
- `50-iio-sensors.rules.leste` — udev sensores (mount matrix)
- `99-krillin.conf.leste` — Xorg config (modesetting + glamor)

## Arquitectura del krillin bajo Maemo Leste
```
Kernel mainline 7.0.12 (ya funciona: DRM+lima, touch, sensores, audio, WiFi, BT)
        ↓
Devuan armhf rootfs (sysvinit/elogind, sin systemd)
        ↓
Xorg + modesetting + Glamor (lima GLES2)
        ↓
Cogl + Clutter (egl_xlib driver)
        ↓
Hildon Desktop (matchbox WM, hildon-home, launcher)
        ↓
MCE (iio-accelerometer, evdevvibrator, iio-als, iio-proximity, LEDs)
```

*Co-autor: ZCode (glm-5.2).*
