# Investigación a fondo: Cámara · Módem · FM Radio (MT6582)

> **Fecha:** 2026-07-07. Tres investigaciones en paralelo (workflow de agentes sobre el código downstream real).
> Filosofía: la misma que ganó el WiFi — "imposible/moonshot" suele ser falso al portear el driver oficial de MediaTek.
> **Resultado: el plan previo (PORT-STRATEGY-DRIVERS-0707.md) estaba EQUIVOCADO en 2 de los 3 subsistemas.**

---

## Veredictos consolidados (con código real, no suposiciones)

| Subsistema | Plan previo | Investigación real | ¿Playbook WiFi aplica? | M1 realista |
|---|---|---|---|---|
| **Cámara** | MOONSHOT | **NO-GO confirmado** | ❌ NO (kernel tonto + HAL cerrada) | Sensor en I2C: ~1-2 sem; imagen: 6-12 meses |
| **Módem M1** (arrancar MD) | MOONSHOT | **✅ GO** | ✅ SÍ (1:1 con el WiFi) | MD_BOOT_STAGE_2: ~3-5 sem |
| **Módem M3** (llamadas) | MOONSHOT | MOONSHOT (confirmado) | n/a | mtk-rild + oFono: meses |
| **FM Radio** | NO-GO (29K LOC) | **CONDITIONAL GO** (~12K LOC) | ✅ SÍ (reusa CONSYS que ya funciona) | `/dev/fm` + tune: ~1-2 sem |

---

## 1. CÁMARA — NO-GO confirmado (con pruebas de código)

**El plan previo acertó en el veredicto, pero por la razón equivocada.** No es "complejidad": es que la arquitectura es **la antítesis del WiFi**.

### Por qué el playbook WiFi NO aplica
- El WiFi funcionó porque su driver stock era **autocontenido** (FW blob + pila 802.11 completa → netdev estándar).
- La cámara es lo **opuesto**: el kernel es un **passthrough MMIO tonto** (`ISP_READ_REGISTER`/`ISP_WRITE_REGISTER` ioctls + `mmap` del espacio MMIO completo a userspace). **Toda la inteligencia** (3A, demosaico, tuning) vive en la **HAL cerrada de Android** (`cam.device.mt6582.so`, XMLs de tuning) que **no existe en mainline**.
- **0 `request_firmware`** en todo el stack → no hay FW blob; todo es registro/DMA controlado por la HAL.

### Lo único reutilizable
- **Sensores OV8865/OV5648 SÍ están en mainline** (`drivers/media/i2c/ov8865.c` 81KB, `ov5648.c` 68KB) — subdevs V4L2 completos.
- **Secuencia de power del krillin** (`kd_camera_hw.c`): VCAM_A 2.8V, VCAM_D 1.5V, CMRST/CMPDN GPIO — mapeable 1:1 a DT.

### Ruta alternativa (NO playbook WiFi)
M1 = sensor visible en I2C/V4L2 (`v4l2-ctl --all` reporta OV8865) — ~1-2 sem, no produce imagen.
Imagen RAW (pass1 IMGO): 3-6 meses escribiendo driver V4L2 SENINF+CSI+ISP desde cero contra ~1.700 registros sin documentar.
Imagen procesada con 3A: **NO ALCANZABLE** sin reimplementar la HAL.

### Conclusión cámara
**Aparcar.** El "imposible" aquí es real y demostrado en código, no una sobreestimación. La diferencia con el WiFi es estructural: no hay "core autocontenido" que portear.

---

## 2. MÓDEM — M1 GO, M3 MOONSHOT (cambio radical del plan)

**El plan previo marcaba TODO el módem como moonshot. Esto es INCORRECTO para M1.**

### Por qué el playbook WiFi SÍ aplica al M1
El arranque del MD (M1) es **kernel-autocontenido**, exactamente el patrón del WiFi:

| WiFi (lo que funcionó) | Módem M1 (aplica igual) |
|---|---|
| `probe` → `wlanAdapterStart` → FW command stream | `module_init` → `echo 0 > /sys/.../boot` → `boot_md()` → MD_INIT_START_BOOT → espera NORMAL_BOOT_ID |
| `request_firmware("WIFI_RAM_CODE")` | `request_firmware("modem.img")` |
| Driver HIF nuestro (ioremap + IRQ) | Glue: ioremap CCIF (0xF020A000) + IRQ 132 + SPM MTCMOS + boot-slave keys |
| `wpa_supplicant` solo para servicio | `mtk-rild` solo para servicio (M3, no M1) |

**`mtk-rild` es al módem lo que `wpa_supplicant` es al WiFi**: necesario para la capa de servicio (M3), NO para el bring-up (M1).

### Riesgo #1 del plan refutado: el SPM
El plan decía "power/clock del MD ausente en mainline, tipo SPM = riesgo #1". **FALSO para MT6582:**
- `spm_mtcmos_ctrl_mdsys1` es **~50 líneas de escrituras de registro MTCMOS** (`SPM_MD_PWR_CON` con `PWR_ON`/`PWR_RST_B`/`~PWR_ISO`, polling de `SPM_PWR_STATUS`).
- **No hay microcódigo SPM, no hay state-machine en firmware.** `spm_write` = `writel` bare-metal. Mapeable con `ioremap(SPM_BASE=0xF0006000)`.
- Es el equivalente exacto del power-sequence del WiFi en el HIF que ya hicieron.

### Secure-boot: DESCARTADO
```
# CONFIG_MTK_SECURITY_SW_SUPPORT is not set   (krillin_defconfig)
# CONFIG_MTK_SEC_MODEM_NVRAM_ANTI_CLONE is not set
```
→ La verificación de firma del MOLY está compilada FUERA. `load_std_firmware` (copia raw). **No hay fusibles secure-boot.**

### LOC reales
- `dual_ccci/` (core): **~15.8K LOC** (ccci_md_main 2558, cci_hw 493, ccci_logical 847, ccmni 2042, etc.)
- `mt6582/src/` (SoC glue): ~3.4K LOC
- Dependencia `mach/`: **baja** (6 ficheros con includes, los críticos como cci_hw.c tienen 0)

### Estado actual en el móvil
MD **totalmente muerto** (no hay `/proc/ccci*`, no hay IRQs CCIF registradas, no hay `modem.img`). Lienzo en blanco.

### Estimación
- **M1 (MD arranca → NORMAL_BOOT_ID → STAGE_2): ~3-5 semanas** (alguien que ya hizo el WiFi)
- **M2 (datos IP via ccmni): +1-2 sem** (si el MOLY coopera con PDP context)
- **M3 (llamadas/SMS): MOONSHOT** (mtk-rild binario + oFono rilmodem, meses)

---

## 3. FM RADIO — CONDITIONAL GO (cambio radical del plan)

**El plan previo decía NO-GO por "29K LOC y utilidad cero". Ambas cosas refutadas.**

### LOC real: 12K (no 29K)
El build krillin del FM es **12.304 LOC**, no 29K. Y no hay HAL cerrado (el gran miedo no se materializa).

### Por qué el playbook aplica
El FM **reutiliza literalmente el CONSYS que ya funciona**:
- **Powerup**: una sola llamada `mtk_wcn_wmt_func_on(WMTDRV_TYPE_FM)` (= `mt6582_consys_func_on(1)` que ya está exportado y probado para BT/GPS/WiFi).
- **Transporte**: canal STP `FM_TASK_INDX=1` (BT=0, GPS=2, WiFi=3, WMT=4 — el btif ya enruta todos excepto FM).
- **No carga FW propio**: el combo CONSYS ya lo descarga el btif en boot.
- **Kernel 100% autocontenido**: 53 ioctls MTK implementadas en kernel (`/dev/fm` chardev, no V4L2). Sin HAL cerrado.

### Lo que falta
1. **Shim `stp_exp.h`/`wmt_exp.h`** (~150-200 LOC): traducir las 8 APIs externas a `mt6582_consys_func_on` + nuevo canal STP-1.
2. **Extender `btif_rx_thread`** (~80-120 LOC): añadir rama `type==1` (FM) siguiendo el patrón GPS existente.
3. **Build del `mtk_fm_drv` stock** (12K LOC copiados): fricción de Kbuild 7.0 (similar a lo ya hecho para WiFi/BT).

### Estimación
**~1-2 semanas.** El FM es **más fácil que el WiFi** porque no tiene bloque físico propio — todo va por el STP que el btif ya domina. Es el último 10% de funcionalidad del CONSYS por un coste marginal.

### Utilidad (honesta)
Baja en términos absolutos (casi nadie escucha FM en 2026), pero el coste es proporcionalmente bajo. Cierra el círculo "CONSYS 4 radios". No candidato a upstream (ioctls MTK privadas).

---

## Orden de ataque recomendado (con la nueva info)

| Prioridad | Subsistema | Esfuerzo | Valor | Notas |
|---|---|---|---|---|
| **1** | **FM Radio** | ~1-2 sem | Cierra CONSYS, barato | Reusa todo lo hecho; el más fácil de los tres |
| **2** | **Módem M1** | ~3-5 sem | Revive el MD (base para investigar M2/M3) | Playbook WiFi 1:1; valor real incluso sin llamadas |
| **—** | GPS (ya planeado) | 3-6 sem | Novedad real, único subsistema "de teléfono" alcanzable | Sigue el plan de PORT-STRATEGY §3.1 |
| **Aparcar** | Cámara | 6-12 meses | NO-GO demostrado en código | Diferencia estructural con WiFi |
| **Aparcar** | Módem M3 | meses | Moonshot confirmado | mtk-rild + oFono |

**Recomendación:** el FM y el M1 del módem son los dos "GO" que el plan previo no vio. El FM sale casi gratis (1-2 sem) y es el warm-up perfecto antes de atacar el GPS (misma infra CONSYS). El M1 del módem es el proyecto ambicioso pero alcanzable que cambia la narrativa del "móvil sin llamadas".

*Co-autor: investigación por agentes en paralelo sobre código downstream real. Bitacora de sesión ZCode.*
