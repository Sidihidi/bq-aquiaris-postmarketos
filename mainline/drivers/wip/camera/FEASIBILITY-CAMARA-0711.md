# Cámara del krillin (MT6582) — feasibility 0711 (REFRAME del NO-GO)

> Investigación fresca (agente, read-only sobre downstream en la Pi + árbol mainline + repo).
> **VEREDICTO: CONDITIONAL-GO para una cámara RAW degradada (sin 3A); NO-GO para cámara de
> producto (calidad Android).** El NO-GO previo acertó para "cámara usable" pero **omitió que el
> sensor real del krillin YA tiene driver V4L2 en mainline** → hay un "hito 0" casi gratis.

## 1. Sensores exactos (evidencia)
Defconfig downstream: `CONFIG_CUSTOM_KERNEL_IMGSENSOR="ov5648_mipi_raw ov8865_mipi_raw t4k04_mipi_raw ov12830_mipi_raw"` (superset; el driver sondea por I2C cuál está presente).
- **Trasera (MAIN, 8 MP)**: **OmniVision OV8865**, MIPI 4-lane, I2C-1 @ **0x10** (WRITE_ID 0x20), chip-ID 0x300B/0x300C.
- **Frontal (SUB, 5 MP)**: **OmniVision OV5648**, 1-2 lane, I2C-1 @ **0x36** (WRITE_ID 0x6c), ID 0x300A/0x300B.
- T4K04 / OV12830 (13 MP) = en defconfig pero de dispositivos hermanos, no casan con los 8 MP grabados.
- Sin EEPROM cam_cal (GT24C32A/BRCC064 = not set) → calibración en OTP del sensor (una dependencia de blob menos). Bus I2C-1, controlador MT6582 @ 0x11008000.

## 2. Arquitectura de captura
Sensor (I2C+MCLK) → MIPI CSI-2 D-PHY → **SENINF** → **ISP (TG/pipeline)** → DMA IMGO → M4U/IOMMU → RAM.
Bases (de `mach-mt6582/camera_isp.c`, 6771 LOC): IMGSYS/ISP 0x15000000, SENINF 0x15008000, MIPI-RX
cfg 0x1500C000 / analog 0x10010000, PLL 0x10000000. **Hay ISP hardware** (no SW-ISP). MCLK del
sensor lo gatea el ISP (`ISP_MCLK1_EN()`).

## 3. Kernel-vs-blob (la clave)
**El kernel es un passthrough MMIO tonto**: ioctls `ISP_READ/WRITE_REGISTER`, `ISP_WAIT_IRQ`,
`ISP_mmap()` mapea el MMIO ISP/SENINF/PLL a userspace. **0 `request_firmware`** (no hay blob de FW).
La config del D-PHY (SENINF) y TODA la inteligencia (setup, demosaico, 3A AWB/AE/AF, lens-shading)
viven en la **HAL cerrada de Android** (`seninf_drv.cpp`, `cam.device.mt6582.so`) — **ausente de la Pi**.
⇒ **Contraste con el WiFi**: el WiFi ganó porque el driver stock era autocontenido (FW+pila→netdev).
Aquí portar `camera_isp.c` da un relé de registros que NO genera imagen. El playbook WiFi NO aplica igual.

## 4. Soporte mainline
- **Sensores YA en mainline**: `drivers/media/i2c/ov8865.c` y `ov5648.c` (subdevs V4L2 completos,
  CSI-2, RAW10, autor Kocialkowski). Necesitan MCLK (xvclk) + supplies (dvdd/avdd/dovdd) + GPIOs
  reset/pwdn + endpoint CSI-2. (T4K04/OV12830 NO tienen driver.)
- **SoC = hueco total**: `drivers/media/platform/mediatek/` solo tiene jpeg/vcodec/mdp/vpu; NO hay
  isp/seninf/camsys para MT6582 (el SENINF mainline es MT8183+ ISP6/7, mapa de regs distinto → no
  reutilizable). DT del krillin: falta i2c1 (0x11008000); mt6582.dtsi usa fixed-clocks, sin M4U/SMI.
  Sí está mt6323-regulator (las rails de cámara son wireables por DT).

## 5. Vías (esfuerzo/riesgo)
| Vía | Produce | Bloqueantes | Esfuerzo | Veredicto |
|---|---|---|---|---|
| **(a) Sensor directo V4L2 (RAW)** | Bayer crudo, sin 3A, demosaico SW | i2c1(trivial)+MCLK(medio)+**D-PHY SENINF**(duro)+**ISP-TG/IMGO/M4U**(duro) | 2-4 meses | **CONDITIONAL-GO** |
| (b) Portar camera_isp.c kernel | nada usable (relé de regs) | inútil sin HAL | 3-4 sem | NO-GO (trampa) |
| (c) libhybris + HAL blobs | cámara "completa" teórica | HAL enorme, muy acoplada a Android; peor que libmnl del GPS | 6-12+ meses | NO-GO/moonshot |
| **(d) Record & Replay** (RE, acelera (a)) | traza del HAL stock → replay de un modo fijo | trazar en dual-boot | dentro de (a) | **HABILITADOR clave** |

(d) = el método "ground-truth dual-boot" que resolvió FM y audio: trazar en el ROM stock la secuencia
exacta de `ISP_WRITE_REGISTER`/pokes para un modo fijo (p.ej. 1280x960 RAW) y reproducirla desde un
driver mainline mínimo. Reduce "1700 registros a ciegas" a captura/replay acotado.

## 6. Veredicto + HITO 0
- **Cámara de producto (3A): NO-GO** (inteligencia en HAL cerrada ausente, demasiado acoplada a Android).
- **Cámara RAW degradada: CONDITIONAL-GO** por (a)+(d), alcanzable porque los sensores ya están en
  mainline y el camino RAW es record-and-replay, no reimplementar la HAL. Riesgo alto pero acotado.
- **HITO 0 (días, casi gratis, read-only)**: confirmar el sensor fitted.
  1. **Cero-riesgo**: en el dual-boot Android stock, `dmesg | grep -iE "kd_sensorlist|OV8865|OV5648|CHECK_SENSOR_ID"` → el driver stock ya imprime el ID.
  2. **Mainline**: añadir `i2c1@0x11008000` + rails mt6323 + GPIO reset al DT, gatear MCLK, `i2cdetect -y 1` → ¿0x10 (OV8865) / 0x36 (OV5648)? Leer chip-ID. Demuestra rail+I2C+MCLK (80% del bring-up).
- **HITO 1 (stretch, semanas-meses)**: subdev OV8865 mainline + RX SENINF por replay + TG/IMGO DMA con M4U → un frame Bayer → demosaico SW → PNG.

## Ficheros clave (Pi)
Downstream: `~/mainline/downstream/arch/arm/mach-mt6582/camera_isp.c`, `.../krillin/camera/camera/kd_camera_hw.c`
(power seq), `~/mainline/downstream/drivers/misc/mediatek/imgsensor/src/mt6582/{ov8865_mipi_raw,ov5648_mipi_raw,kd_sensorlist.c}`.
Mainline: `~/mainline/linux-7.0.12/drivers/media/i2c/{ov8865.c,ov5648.c}`, DT `.../mt6582{.dtsi,-bq-krillin.dts}`.

## Incertidumbre honesta
Que el fitted sea OV8865+OV5648 es inferencia fuerte (resolución + IDs + lanes), NO lectura del móvil
→ el Hito 0 opción 1 la cierra en minutos. Si algún lote montara T4K04/OV12830, esos no tienen driver
mainline (habría que escribir el subdev, ~1-2K LOC c/u).

*Investigación 2026-07-11.*
