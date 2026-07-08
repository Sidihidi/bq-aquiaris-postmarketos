# 2026-07-08 · Maemo Leste port (en progreso)

## TL;DR
Port de Maemo Leste al krillin. Device package y board script listos.
Rootfs build **bloqueado por dependencias** entre Devuan Excalibur y paquetes Maemo Leste.
El camino correcto es arm-sdk (maneja el pinning) — pendiente de debuggear.

## Lo que SÍ está hecho y funciona

### Device package `leste-config-krillin` ✅
En `~/leste-config/leste-config-krillin/` y en el repo (`mainline/maemo-leste/`):
- `hildon-desktop.env.leste`: COGL_RENDERER=egl_xlib, COGL_DRIVER=gles2
- `70-krillin.ini.leste`: MCE con sensores del krillin
- `50-iio-sensors.rules.leste`: mount matrix LSM330 + TMD2772
- `99-krillin.conf.leste`: Xorg modesetting + Glamor

### Board script `krillin.sh` para arm-sdk ✅
En `~/arm-sdk/boards/krillin.sh` y en el repo. Reusa kernel prebuilt.

### Kernel modules compilados ✅
`modules.order` existe. `modules_install` listo.

## Lo que FALLA y cómo arreglarlo

### Problema: dependencias rotas al instalar hildon-meta-core
```
hildon-meta-core : Depends: mce but it is not going to be installed
                   Depends: hildon-base but it is not going to be installed
                   Depends: upower but it is not going to be installed
cron-daemon-common : Depends: systemd but it is not installable
```

**Causa:** el rootfs se construye con debootstrap de Devuan excalibur, pero los paquetes
de Maemo Leste exigen versiones específicas que entran en conflicto con lo que debootstrap
instala. El arm-sdk maneja esto con pinning de apt y scripts post-build.

### Solución: usar arm-sdk correctamente
El arm-sdk tiene toda la lógica para resolver estas dependencias. El problema fue:
1. La función `build_kernel_armhf` del krillin.sh necesita ajustes (crear dirs, modules)
2. La función `image_pack_dist` (que hace el debootstrap con el blend correcto) no se invocó

**Pasos para continuar:**
1. Arreglar `krillin.sh` para que `build_kernel_armhf` cree `$strapdir/boot/dtbs` antes del cp
2. Hacer que `build_kernel_armhf` NO lance error si modules_install falla temporalmente
3. Ejecutar arm-sdk interactivo: `zsh -f; cd arm-sdk; source sdk; load devuan krillin maemo; build_kernel_armhf; image_pack_dist`
4. El `blend=maemo` instala los paquetes correctos con el pinning adecuado

### Alternativa: imagen precompilada
No hay imagen armhf genérica reciente en https://maedevu.maemo.org/images/ 
(la última sunxi es vieja). Las imágenes de pinephone son arm64.

## Cómo seguir (para la próxima sesión)

### Opción A: arreglar arm-sdk (recomendado)
```sh
cd ~/arm-sdk
# Arreglar krillin.sh: crear boot/dtbs antes del cp de zImage
# Ejecutar interactivo:
zsh -f
source sdk
load devuan krillin maemo   # el blend "maemo" es clave
build_kernel_armhf
image_pack_dist             # crea la imagen SD
```

### Opción B: debootstrap manual con pinning
1. Hacer debootstrap de excalibur
2. Instalar `leste-config-common` PRIMERO (antes que hildon-meta-core)
3. Pinning apt: `Package: * / Pin: release a=excalibur / Pin-Priority: 1001`
4. Instalar `hildon-meta-core` con `--fix-broken`

### Opción C: chroot en el móvil
Hacer todo el build dentro del móvil (armhf nativo, sin QEMU):
1. Crear mmcblk1p3 en la SD
2. debootstrap directo en el móvil
3. apt install hildon-meta-core (red nativa, sin problemas de chroot)

## Archivos en el repo (`mainline/maemo-leste/`)
- `krillin-board.sh` — board script del arm-sdk
- `hildon-desktop.env.leste`, `70-krillin.ini.leste`, `50-iio-sensors.rules.leste`, `99-krillin.conf.leste`

## Archivos en la Pi
- `~/leste-config/leste-config-krillin/` — device package completo
- `~/arm-sdk/boards/krillin.sh` — board script (registrado en sdk)
- `~/arm-sdk/` — arm-sdk con submódulos inicializados
- `~/maemo-rootfs/` — rootfs base Devuan excalibur (331MB, SIN hildon aún)

*Co-autor: ZCode (glm-5.2).*
