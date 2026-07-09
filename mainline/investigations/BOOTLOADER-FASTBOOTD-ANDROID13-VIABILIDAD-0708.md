# Estudios de viabilidad: Bootloader custom · fastbootd · Android 13 (32-bit)

> **Fecha:** 2026-07-08. Tres investigaciones en paralelo sobre el krillin (MT6582).
> Cada una examina el hardware real, el código downstream y los proyectos existentes.

---

## Resumen ejecutivo

| Subsistema | Veredicto | Esfuerzo | Recomendación |
|---|---|---|---|
| **Bootloader custom** (U-Boot) | **CONDITIONAL GO** (modo "secondary") | 1-3 días | U-Boot como 2º stage empaquetado en boot.img; LK intacto. SD + cmdline custom. |
| **fastbootd** (userspace) | **NO-GO** (bajo valor) | 1-6 sem según enfoque | No invertir: `dd` over SSH + MD5 ya cubre el caso de uso mejor que fastbootd. |
| **Android 13 nativo 32-bit** | **NO-GO** (3 bloqueadores duros) | meses-años | RAM (1<2GB), armv7 deprecado, no existe LOS20 para krillin. Mantener pmOS. |

---

## 1. Bootloader custom — CONDITIONAL GO (modo "secondary")

### Arquitectura de boot del MT6582
```
BROM (mask ROM @0xFFFF0000) → Preloader (eMMC boot1) → LK Little Kernel (partición UBOOT) → Linux (boot.img)
```
- **BROM**: hardwired, no se toca. Carga el preloader.
- **Preloader** (proprietario MTK): init DRAM + clocks. **Regla de oro: NUNCA tocar.** Mientras esté intacto, mtkclient puede recuperar cualquier brick.
- **LK KitKat 1.5.2** (partición UBOOT, 384KB): fastboot + carga boot.img. **No verifica firma** en krillin (eFuse SBC no activado en variantes de consumo BQ).
- El LK de **Lollipop** sí verifica firma del kernel → bootloop. El bueno es el de KitKat.

### Proyectos existentes
| Repo | Estado | SD | Display | Modo |
|---|---|---|---|---|
| `mediatek-mainline/u-boot` (rama mt6582) | **Funciona** (feb 2022) | ✅ | ❌ (solo UART) | secondary + first |
| `cristianc/u-boot-mt65xx` (rama mt6582) | Arranca (sep 2025) | ❌ eMMC/SD rotos | — | — |
| U-Boot mainline upstream | Sin soporte MT65xx | — | — | — |
| coreboot | Sin soporte MT6582 | — | — | — |

### Enfoque recomendado: U-Boot modo "secondary bootloader"
```
BROM → Preloader (intacto) → LK KitKat (intacto) → U-Boot (en boot.img) → Linux mainline (desde SD)
```
- U-Boot se empaqueta como `boot.img` con cabecera MTK → el LK lo carga como "kernel"
- U-Boot carga Linux desde SD (`load mmc 1:1`) sin cabecera MTK
- **100% reversible**: re-flashear boot.img original restaura todo
- **No toca preloader ni LK** → riesgo de brickeo permanente BAJO
- Da: cmdline custom, boot.scr, carga desde SD nativa, sin cabecera MTK

### Qué ganaría el proyecto
- Boot de rescue/instalación desde SD sin tocar eMMC
- Kernel mainline puro sin cabecera MTK (elimina `mkimage`/`mkbootimg-osm0sis`)
- Cmdline completa controlada
- Inmunidad permanente al bootloop del LK de Lollipop

### Estimación
- **U-Boot secondary funcionando: 1-3 días** (build + empaquetar + test SD)
- U-Boot "first" (reemplazar LK) con display: 1-4 semanas (portear driver display MTK)
- No vale la pena reescribir el LK desde cero (3-6 meses, innecesario)

---

## 2. fastbootd — NO-GO (bajo valor)

### Tabla comparativa
| | fastboot del LK (hardware) | fastbootd (AOSP userspace) | Gadget kernel (f_fastboot.c) |
|---|---|---|---|
| Dónde corre | Bare-metal en el LK | Proceso en Linux (modo recovery) | Driver del kernel |
| Requiere Android | No | **Sí** (libs de AOSP) | No |
| `fastboot boot` (temporal) | **Sí** | No | No |
| En krillin | **Funciona hoy** (Power+Vol-) | No instalado | No existe en mainline |

### Hallazgos clave
- **fastbootd NO está empaquetado para Alpine/pmOS.** El paquete `android-tools` solo tiene el cliente host. fastbootd es un target de dispositivo de AOSP que depende de ~10 libs de Android.
- **`f_fastboot.c` NO existe en mainline Linux** (verificado: `find` devuelve vacío). Los maintainers decidieron que fastboot es de bootloader/userspace, no del kernel.
- **El kernel del krillin SÍ soporta configfs + FunctionFS** (CONFIG_USB_CONFIGFS, USB_F_FS, USB_F_FS habilitados). Un daemon fastboot sobre FunctionFS sería técnicamente posible de escribir.
- **Pero el caso de uso ya está cubierto**: `dd if=boot.img of=/dev/mmcblk0 seek=83968` por SSH con verificación MD5 es más robusto que fastbootd.

### Veredicto
**No invertir.** El coste/beneficio es desfavorable:
- fastbootd AOSP en Alpine: NO-GO (3-6 sem, libs asumen Android)
- Daemon minimal sobre FunctionFS: viable (1-2 sem) pero valor marginal
- `dd` over SSH + MD5 ya hace lo mismo mejor
- `fastboot boot` temporal (única capacidad que dd no replica) requiere el LK, que ya funciona

---

## 3. Android 13 (32-bit) nativo — NO-GO (3 bloqueadores duros)

### Corrección importante de la premisa
**No existe LineageOS 20 (Android 13) para el krillin.** La documentación de Halium confirma que el port más nuevo es **LineageOS 13 (Android 6.0)** sobre kernel 3.10.49. La premisa de que "hay ports de Android 13" era incorrecta. Nadie ha arrancado Android 7+ en este SoC.

### Tres bloqueadores duros

**1. RAM (1 GB < 2 GB mínimo)**
- Android 13 Go subió el mínimo a **2 GB** (desde 1 GB en A12)
- El krillin tiene 945 MB físicos (~700 MB usables tras reservas)
- Android 13 consume ~2 GB solo arrancar → `lmkd` entraría en bucle de kill
- **Bloqueador absoluto, no negociable**

**2. armv7 deprecado en Android moderno**
- La GSI arm32 pura (`treble_arm_b`) se deprecated en Android 12
- El truco `a64` (userspace 32-bit + kernel 64-bit) **no aplica**: el MT6582 es Cortex-A7, que NO ejecuta AArch64
- El target `aosp_arm` sigue en AOSP 13 pero es de emulador, no de dispositivo real
- **armv7 pura está abandonada en Android moderno**

**3. Kernel incompatible**
- Android 13 exige mínimo kernel 5.10 (GKI). El downstream del krillin es 3.10.
- El mainline 7.0.12 del proyecto NO es un ACK (Android Common Kernel) válido para boot nativo
- Para Waydroid/Halium sí sirve (es contenedor), pero no para boot Android nativo

### Opciones evaluadas
| Opción | RAM | Kernel | Apps 2026 | Drivers | Veredicto |
|---|---|---|---|---|---|
| Android 13 nativo | ❌ 1<2GB | ❌ ni 3.10 ni ACK | ❌ menguante | ❌ pierde todo | **NO-GO** |
| Waydroid (contenedor) | ❌ 1GB inusable | ✅ mainline OK | ⚠️ armv7 | ✅ preserva | **NO-GO práctico (RAM)** |
| Halium | ❌ | ❌ no hay H13 | ❌ | ❌ blobs KitKat | **NO-GO** |

### Ecosistema de apps en armv7 (2026)
- Pixel 7+ ya es 64-bit-only (2022). Apps cada vez más son arm64-only.
- Google Play exige 64-bit desde 2019. Apps nuevas pueden no tener `armeabi-v7a`.
- WhatsApp/Telegram siguen con builds 32-bit hoy, pero la tendencia es clara.
- **En Android 13 armv7 en 2026, una fracción creciente del Play Store no instalaría.**

### Veredicto
**NO-GO categórico.** Tres bloqueadores duros (RAM, armv7 deprecado, kernel). La decisión del ROADMAP (0706) de aparcar Android era correcta. El pmOS+Phosh que ya funciona es infinitamente mejor que un Android que no arrancaría.

Waydroid sería la única arquitectura viable en abstracto (preserva drivers, kernel mainline correcto), pero choca con la RAM. **Reevaluar solo si se cambia a un móvil con ≥2 GB.**

---

## Conclusión global

De los tres estudios, solo el **bootloader custom (U-Boot modo secondary)** es GO — y es barato (1-3 días) y de bajo riesgo. Daría al proyecto capacidades reales: carga desde SD nativa, cmdline completa, sin cabecera MTK. Los otros dos (fastbootd y Android 13) son NO-GO con bloqueadores técnicos duros.

*Investigación por agentes en paralelo. Co-autor: ZCode (glm-5.2).*
