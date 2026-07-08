# Plan: U-Boot multiboot + Maemo Leste para el krillin

> **Fecha:** 2026-07-08. Dos investigaciones en paralelo. Plan de implementación concreto.

---

## Resumen ejecutivo

| Componente | Veredicto | Esfuerzo | Clave |
|---|---|---|---|
| **U-Boot secondary multiboot** | ✅ GO | 1-3 días | Port existe y funciona (SD+UART). `extlinux.conf` multiboot sin recompilar. |
| **Maemo Leste port** | ✅ GO | 2-4 semanas | 90% del trabajo (kernel+drivers) ya hecho. Device package copy-adapt PinePhone+mz617. |
| **Combinación** | ✅ multiboot real | — | U-Boot (boot.img fijo) → extlinux.conf → pmOS o Maemo Leste. Sin reflashear. |

---

## Parte 1: U-Boot multiboot (1-3 días)

### Arquitectura resultante
```
BROM → Preloader (intacto) → LK KitKat (intacto) → U-Boot (boot.img, UNA vez) → extlinux.conf → SO elegido
```

### Fases

#### Fase 0: Build del U-Boot (~1h)
```sh
# En la Pi (compila nativo armhf, no cross-compile)
sudo apk add bison flex dtc python3 py3-setuptools py3-elftools swig openssl-dev
git clone -b mt6582 https://github.com/mediatek-mainline/u-boot.git ~/u-boot-mt6582
cd ~/u-boot-mt6582
make mt6582_prestigio_pmt5008_3g_defconfig
make -j2   # -j2 como siempre
# Resultado: u-boot.bin
```

#### Fase 1: Test por UART (sin flashear) (~1h)
Usar el script del README del repo (`uartboot-mt6582-uboot.script` con ckerlermit):
- Cargar U-Boot por `loadb 0x81e00000` + `go` vía UART
- Verificar que detecta la SD: `mmc dev`, `ls mmc 1:1`
- **No tocar el eMMC** hasta confirmar que funciona

#### Fase 2: Empaquetar como boot.img MTK (~30min)
```sh
# initrd dummy (el LK exige ramdisk en el boot.img)
dd if=/dev/urandom of=ramdisk bs=2048 count=9
# Cabecera MTK (como KERNEL)
python3 mtk_hdr.py KERNEL u-boot.bin u-boot.bin.mtk
python3 mtk_hdr.py ROOTFS ramdisk ramdisk.img.mtk
# Empaquetar (offsets del krillin, ya conocidos)
abootimg --create u-boot-mt6582.img -f bootimg-sd.cfg -k u-boot.bin.mtk -r ramdisk.img.mtk
# Flashear (una sola vez):
fastboot flash boot u-boot-mt6582.img
# o dd al sector 83968
```

#### Fase 3: SD multiboot con extlinux.conf (~1h)
Layout de la SD:
```
mmcblk1p1  ~256MB  ext4   /extlinux/extlinux.conf + /zImage + /dtb + /uInitrd-*
mmcblk1p2  ~3-4GB  ext4   rootfs postmarketOS
mmcblk1p3  ~3-4GB  ext4   rootfs Maemo Leste
mmcblk1p4  resto   ext4   datos compartidos
```

`/extlinux/extlinux.conf`:
```
menu title Krillin multiboot
timeout 100
default pmos

label pmos
    menu label postmarketOS (Alpine + Phosh)
    kernel /zImage
    initrd /uInitrd-pmos
    fdt /mt6582-bq-krillin.dtb
    append root=/dev/mmcblk1p2 rw console=ttyS0,921600n8 clk_ignore_unused ignore_loglevel

label maemo
    menu label Maemo Leste (Hildon)
    kernel /zImage
    initrd /uInitrd-maemo
    fdt /mt6582-bq-krillin.dtb
    append root=/dev/mmcblk1p3 rw console=ttyS0,921600n8 clk_ignore_unused ignore_loglevel
```

**Cambiar de SO = editar `default` + reiniciar. Sin reflashear.**

#### Fase 4 (opcional): Menú en pantalla con initramfs menupick
U-Boot no tiene display (sin driver DRM para MT6582). Para selección *en el teléfono sin PC*:
- U-Boot arranca siempre el initramfs `menupick` del proyecto (ya existe, lee Vol- por evdev, dibuja por DRM nativa del kernel)
- El initramfs presenta el menú y hace `switch_root` al SO elegido
- **Combina lo mejor**: U-Boot da cmdline limpia + SD nativa; el initramfs da selección visual en el teléfono

### Cmdline del kernel
El kernel mainline 7.0.12 **no depende de la cmdline del LK**. Funciona con `bootargs` del DTB. Cmdline mínima desde U-Boot:
```
console=ttyS0,921600n8 clk_ignore_unused ignore_loglevel panic=15 root=/dev/mmcblk1pX rw
```
- `clk_ignore_unused` es **crítico** (evita que el kernel apague clocks que el LK dejó encendidos)
- `lcm=`, `boot_reason=`, `androidboot.*` del LK → omitibles (el mainline no los usa)

### Riesgos
| Riesgo | Mitigación |
|---|---|
| U-Boot no arranca | Backup del boot.img actual; fastboot/mtkclient recuperan siempre (LK+preloader intactos) |
| Sin display en U-Boot | Usar `default`+`timeout` (auto-boot) o initramfs menupick |
| Brick del preloader | **Regla de oro**: modo secondary NO toca preloader. Riesgo casi nulo. |

---

## Parte 2: Port de Maemo Leste (2-4 semanas)

### Por qué el krillin es un candidato inusualmente bueno
**El 90% del trabajo difícil ya está hecho.** El kernel mainline 7.0.12 tiene:
- ✅ DRM + lima (Mali-400) — mismo path que PinePhone y turbox-twister
- ✅ Touch edt-ft5x06 (mismo driver que PinePhone)
- ✅ Sensores IIO (LSM330 accel/gyro + TMD2772 ALS/prox) — MCE los usa
- ✅ Audio AFE MT6323 (suena) — PulseAudio + UCM2
- ✅ WiFi WPA2 + BT + FM
- ✅ Suspend s2idle + autosuspend
- ✅ LED pattern trigger + regulator-haptic

**No hay nada que reimplementar en el kernel para Maemo Leste.**

### Arquitectura de Maemo Leste
- **Base:** Devuan (Debian sin systemd, sysvinit/elogind)
- **Gráficos:** **X11/Xorg** (NO Wayland) — driver `modesetting` + Glamor/GLES
- **UI:** Hildon Desktop (matchbox WM, hildon-home, launcher)
- **GPU:** Cogl + GLES2 vía EGL/X11 (`COGL_RENDERER=egl_xlib`, `COGL_DRIVER=gles2`)
- **Middleware:** MCE (sensores/LEDs/vibrador), Telepathy (IM/llamadas), oFono (telefonía)
- **Audio:** PulseAudio + ALSA UCM2

### Funciona sin módem — y el krillin encaja
Maemo Leste soporta el modo "tablet/PDA sin módem" (ej: Allwinner A23 tablet, mz617 Xyboard). El krillin sería exactamente eso: todo funciona excepto llamadas celulares. Llamadas SIP vía sphone/Telepathy sí funcionarían.

### Fases del port

#### Fase A: Device package `leste-config-krillin` (~1 semana)
Copy-adapt de PinePhone (lima+GLES2) + mz617 (sin módem):

| Fichero | Origen | Qué adaptar |
|---|---|---|
| `hildon-desktop.env.leste` | pinephone | `COGL_RENDERER=egl_xlib`, `COGL_DRIVER=gles2` (idéntico) |
| `mce.ini.d/70-krillin.ini.leste` | pinephone | `ModulesDevice=iio-accelerometer;iio-als;evdevvibrator;iio-proximity` |
| `50-iio-sensors.rules.leste` | pinephone | Mount matrix LSM330 (`[[0,1,0],[1,0,0],[0,0,-1]]`, ya calibrada) |
| `99-krillin-xorg.conf.leste` | NUEVO | Driver `modesetting` + Glamor (validar lima bajo Xorg) |
| `alsa/ucm2/MT6582/*.conf.leste` | pinephone | Routing AFE MT6323 (auriculares+altavoz ya suenan) |
| `73-krillin-leds-color.rules.leste` | existente | Reglas LED color ya hechas |

#### Fase B: Rootfs Devuan armhf + hildon-meta-core (~1 semana)
```sh
# Usar arm-sdk de Maemo Leste
git clone https://github.com/maemo-leste/arm-sdk.git
cd arm-sdk
# Crear boards/krillin.sh basado en turbox-twister.sh (Cortex-A7+Mali-400)
# apuntar gitkernel a nuestro kernel 7.0.12 (o reusar zImage/dtb/modules ya compilados)
# debootstrap Devuan armhf + hildon-meta-core + leste-config-krillin
# Resultado: imagen SD con Maemo Leste
```

#### Fase C: Validar Xorg + lima + Cogl/GLES2 (~1 semana, **punto crítico**)
El krillin hoy valida lima bajo **Wayland/Phosh**. Maemo Leste lo necesita bajo **Xorg/Cogl**.
- El driver lima del kernel es el mismo
- Probado en PinePhone (arm64) y turbox-twister (armhf Cortex-A7 Mali-400)
- Test mínimo antes de montar rootfs completo: `Xorg + cogl-info` con `COGL_RENDERER=egl_xlib`
- Si no acelera: caer a software rendering (usable pero lento en A7)

#### Fase D: Integración multiboot (~3 días)
- Maemo Leste en `mmcblk1p3`
- Entrada en `extlinux.conf`
- Cmdline con `root=/dev/mmcblk1p3`

### Estimación total: 2-4 semanas
- 90% del trabajo (kernel+drivers) ya hecho
- Resto: device package (copy-adapt), rootfs (debootstrap), validar Xorg/lima
- **Riesgo principal:** GLES2 bajo Xorg (hoy solo probado bajo Wayland)

### Recursos
- **arm-sdk:** https://github.com/maemo-leste/arm-sdk — `boards/turbox-twister.sh` (mejor plantilla: Cortex-A7+Mali-400)
- **leste-config:** https://github.com/maemo-leste/leste-config — `leste-config-pinephone/` (lima ref), `leste-config-mz617/` (sin módem ref)
- **Imágenes:** https://maedevu.maemo.org/images/ — builds precompilados por device
- **Wiki:** https://leste.maemo.org/ — Getting Started, Downloads, PinePhone, A23 tablet

---

## Orden de ataque recomendado

1. **U-Boot secondary + extlinux.conf** (1-3 días) — el bootloader multiboot base
2. **pmOS en una partición de la SD** (ya funciona, solo reposicionar)
3. **Maemo Leste device package + rootfs** (2-4 sem) — el segundo SO del menú
4. **Validar Xorg+lima** (dentro de la Fase C de Maemo) — el punto crítico

El resultado: **un móvil con menú de arranque que elige entre postmarketOS (Phosh) y Maemo Leste (Hildon), sin reflashear nunca.**

*Co-autor: ZCode (glm-5.2). Investigación por agentes en paralelo.*
