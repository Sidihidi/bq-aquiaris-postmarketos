<!-- Generado por workflow de 5 agentes (w37dcsxna, 2026-07-07): GPS/módem/pulido investigados a fondo;
sección cámara sintetizada (el agente dedicado falló, pero el NO-GO es sólido y coincide con módem/FM). -->

# Estrategia de porteo — GPS · Cámara · Módem · Pulido (MT6582)

> Fuente de verdad complementaria a `ROADMAP.md` (raíz). Documento de arquitectura para decidir el orden de ataque de las funciones pendientes tras cerrar WiFi (WPA2+DHCP) y audio en mainline. Filosofía transversal — la misma que ganó el WiFi: **la joya cerrada de MTK (command-stream / motor GNSS / baseband) no se reimplementa; se CORRE intacta bajo una capa baja nuestra + shims KAL.**

---

## 1. Resumen ejecutivo

| Subsistema | Veredicto | Esfuerzo | Siguiente paso concreto |
|---|---|---|---|
| **GPS** — combo host-based (0xAAF0 → libmnl → NMEA → gpsd/geoclue/Phosh) | **CONDITIONAL-GO** | 3-6 sem (1er hito 1-2 sem) | **Fase A**: runner nativo mínimo = glue abierto recortado + shims KAL, **linkando `libmnl_6628.a`** con toolchain armv7-musl; `dsp_port=/dev/stpgps`, `nmea_port=pty` |
| **Cámara** (ISP/sensor MTK + HAL cerrada) | **NO-GO** | — | Documentar como **límite conocido** (junto al módem). No abordar |
| **Módem 2G/3G** — CCCI/dual_ccci + firmware MOLY | **MOONSHOT** | 4-8+ meses, prob. baja | **No entrar** salvo aceptar apuesta de investigación. Si se acepta: **solo spike M1** (Fase 0+1, 4-7 sem) con puerta de aborto dura |
| **Pulido (8)** Fluidez botón power | **GO** | 1-2 días | Daemon persistente en la sesión (gdbus + EVIOCGRAB ya abiertos), sin fork de `su` |
| **Pulido (1)** Auto-brillo ALS | **GO** | 1-2 días | Mini-daemon `ClaimLight`→curva lux→duty→`/run` (backlight actual). **NO** gsd-power |
| **Pulido (5)** STP core resync/CRC | **GO condicional** | 1-2 días | Integrar el parche **resync-RX ya escrito** en `mt6582-btif.c`. No portar stp_core entero sin corrupción observada |
| **Pulido (4)** Thermal CPU | **CONDITIONAL-GO** | 1-2 sem | Extender `auxadc_thermal.c` con `mtk_thermal_data` MT6582 + nvmem efuse en DTS; fallback `thermal-generic-adc` |
| **Pulido (3)** Accdet jack | **CONDITIONAL-GO** | 1 sem | Portar el core de `accdet.c` (MT6323) → extcon/ALSA-jack. Requiere RE de umbrales ADC |
| **Pulido (7)** Magnetómetro MMC3516x | **GO (baja prioridad)** | 2-4 días | Driver IIO ~300 LOC modelado en `mmc35240.c`. Sin consumidor en Phosh |
| **Pulido (6)** BTCVSD (audio BT SCO) | **DEFER** | 1-2 sem | Reevaluar solo si aparece caso de uso (VoIP + headset BT) |
| **Pulido (2)** FM Radio | **NO-GO** | 4-6 sem | Aparcar indefinidamente (base mainline nula, utilidad ~cero) |

---

## 2. Orden de ataque recomendado

Criterio: **impacto UX ÷ esfuerzo × probabilidad de éxito**. Tres olas.

### Ola 1 — Quick wins de una tarde (hacer YA)
Suben la sensación de "teléfono terminado" con riesgo cero y sin tocar kernel.
1. **Fluidez botón power** (1-2 d) — máximo impacto diario, 100% userspace.
2. **Auto-brillo ALS** (1-2 d) — HW ya listo (tsl2772 + iio-sensor-proxy), solo falta el lazo.
3. **STP resync-RX** (1-2 d) — integrar parche existente, robustez BT/GPS barata.

### Ola 2 — Ingeniería con valor real (semanas, riesgo acotado)
4. **Thermal CPU** (1-2 sem) — seguridad/longevidad; único del lote con riesgo técnico (calibración efuse). Camino limpio = extender `mtk-thermal`; fallback grueso disponible.
5. **Accdet jack** (1 sem) — cierra el audio (auto-routing altavoz/auricular + botones inline). Requiere RE de umbrales.
6. **GPS — Fase A/B** (3-6 sem) — **la pieza grande que SÍ vale la pena**. Novedad real (ningún MT65xx en mainline+musl tiene el GPS combo). Playbook WiFi aplica casi perfecto, con la diferencia de que el "core stock" es un blob de **userspace** (link estático + shims KAL) en vez de un driver de kernel.

### Ola 3 — Opcional / completitud
7. **Magnetómetro** (2-4 d) — driver factible pero **sin consumidor en Phosh**; solo por completitud IIO.
8. **BTCVSD** (1-2 sem) — **DEFER**: utilidad baja sin módem.

### Lo que es investigación pura (no confundir con roadmap de producto)
- **Módem** — **MOONSHOT**. Hay ruta concreta y precedente (UT/Halium en este mismo móvil) pero mainline-nativo tiene prob. ~10-15% y coste de meses. Entrar solo como apuesta explícita, y solo el spike M1 con puerta de aborto.
- **FM Radio** y **Cámara** — **NO-GO**. Coste alto/enorme, base mainline nula, utilidad casi cero. Aparcar.

> **Regla de oro**: las Olas 1 completan la percepción de "móvil terminado" en dos tardes. La Ola 2 es donde está el trabajo serio y con retorno. La Ola 3 y el módem/cámara/FM son opcionales o investigación — no bloquean declarar las *phone-features* como completas.

---

## 3. Detalle por subsistema

### 3.1 GPS — combo MT6582 (6628-class), host-based · **CONDITIONAL-GO**

**Arquitectura (breve).** El GPS del krillin es **HOST-BASED (MSB)**, no un chip NMEA autónomo. El DSP del combo SOLO adquiere/trackea y emite **medidas crudas** (pseudorangos) en frames binarios `0xAAF0` sobre `/dev/stpgps` (STP). El **cálculo de la posición (PVT)** — filtro de Kalman (`NK_Predict_State`/`NK_Correct_State`), efemérides/almanaque, geodesia — lo hace **`libmnl` en la CPU del host**. Flujo stock: `mnld` abre `/dev/stpgps`, hace un ioctl tolerante `COMBO_IOC_GPS_HWVER` y llama a `mtk_gps_mnl_run(init_cfg, driver_cfg)`; a partir de ahí libmnl es dueña del fd y construye **todo** el 0xAAF0 (incluido el frame START t=0x05 que nos falta, generado dinámicamente con estado/aiding). Devuelve NMEA por un `nmea_port` y/o por callback `mtk_gps_sys_nmea_output_to_app()`. El `init_cfg` lleva parámetros **TCXO del móvil** (`hw_Clock_Freq/Drift/u1ClockType`) de la NVRAM GPS.

**Respuesta directa a las dos preguntas del usuario:**
- **¿Replicar el driver oficial evita LineageOS?** → **SÍ, y es justo el punto.** Correr libmnl nativamente en el móvil sobre nuestro `/dev/stpgps` **elimina la dependencia del dual-boot**. En la **Fase B** instrumentamos el `write` al `dsp_fd` para loguear el burst START/aiding **en NUESTRO hardware**, lo que mata de raíz el bloqueo del `strace` en LineageOS. Ya no necesitamos capturar el START en Android: libmnl lo genera él mismo con los TCXO reales.
- **¿El source del protocolo 0xAAF0 está disponible?** → **NO.** El **framer 0xAAF0 y el motor PVT viven 100% en el blob cerrado** (`libmnl_6628.a` / nuestro `libmnl.so`). Está **confirmado por lectura**: el glue AOSP abierto (`mtk_gps_6620.c`, `data_coder.c`) **no construye ni una** inmediata de frame — `data_coder.c` es solo la serialización del socket HAL↔mnld. Corolario estratégico: **RE del blob y captura del START son INSUFICIENTES como endgame** — aunque tengas el START exacto, sin libmnl no hay fix porque el PVT lo calcula la librería. El START capturado se guarda **solo como referencia de depuración**.

**Dónde vive el stack stock.**
- **Core cerrado (la joya)**: nuestro `libmnl.so` exacto en la Pi (`~/gps/gps-grab/libmnl.so`, 1.78 MB, armv7 bionic, stripped) **O** el estático de AOSP Nu3001 `gps/combo_mt66xx/mnl/libmnlp/mnl6628/lib/libmnl_6628.a` (10.5 MB, sin strip) + `libagent_6628.a` + `libmnlp/libs/{libsupl.a,libhotstill.a}`.
- **Glue ABIERTO (source completo)**: Nu3001 `mnl_process_6620.c`, `mtk_gps_6620.c`, `mnl/src/mnl_common_6620.c` + headers `mtk_gps.h`, `mtk_gps_driver_wrapper.h`; lbule `mnld.c`/`mnld_uti.c`/`mtk_gps.c`/`data_coder.c`.
- **Transporte** `/dev/stpgps` (STP) — **ya funciona** (enciende el radio vía WMT func_on[GPS]).

**Estado mainline.** Nada usable. `drivers/gnss/` existe pero solo para GNSS serie NMEA autónomo (u-blox/SiRF/MT3339), **no** para el combo host-based con motor libmnl. No hay driver ni framework destino: la solución es **100% userspace trayendo la pila stock a mainline**, igual que el WiFi trajo el core stock.

**Plan de porteo por fases (playbook WiFi, adaptado a blob userspace):**
- **Fase A (barata, días)** — Runner nativo mínimo: compilar el glue abierto recortado (sin AGPS/SUPL/EPO) con toolchain **armv7-musl** de pmOS y **linkar `libmnl_6628.a` (+libagent/libsupl/libhotstill)**. Rellenar `init_cfg`/`driver_cfg`: `dsp_port=/dev/stpgps`, `nmea_port=pty`. **Objetivo: que enlace y llame a `mtk_gps_mnl_run()`.**
- **Fase A' (shims KAL, en paralelo)** — Implementar los ~6 callbacks del porting (`mtk_gps_sys_create_mutex`/`destroy_mutex`, `_nmea_output_to_app`, `_gps_mnl_callback`, `_pmtk_cmd_cb`, `_agps_disaptcher_callback`) + shims libc **bionic→musl** acotados (`__errno`, `__*_chk`, `__stack_chk_*`) + **mini-shim pthread con layout bionic** para las llamadas directas (estilo libhybris). La mayoría del locking ya va por `mtk_gps_sys_create_mutex` = **nuestro**. Stubs tolerantes de `COMBO_IOC_GPS_HWVER` y `/dev/mtgpio`.
- **Fase B (en HW)** — Correr en el móvil sobre `/dev/stpgps` con los **TCXO reales** (NVRAM). Instrumentar el `write` al `dsp_fd` para loguear el START 0xAAF0 en nuestro hardware. Meta: el DSP adquiere y libmnl emite NMEA.
- **Fase C (integración)** — Enchufar el NMEA a la cadena **gpsd→geoclue→Phosh que YA existe**. Validar fix real en exterior con cielo despejado. **Retirar `mtkgps_aaf0.c`** (queda superado: no podía calcular el PVT).
- **Fallback P2** — Si el `.a` de AOSP no casa de versión o el ABI pthread es intratable: correr **nuestro `libmnl.so` exacto vía libhybris** (bionic aporta el ABI correcto). Es un mini-proyecto de infra (libhybris en armv7+musl es nicho) pero usa el core exacto del chip.

**Dependencias.** Core cerrado (`libmnl.so` propio o `libmnl_6628.a` + libagent/libsupl/libhotstill) · glue abierto AOSP · `/dev/stpgps` (ya) · toolchain armv7-musl + lib de shims KAL · **parámetros TCXO de NVRAM/gps.conf stock** · cadena gpsd→geoclue→Phosh (ya) · (solo P2) libhybris armv7+musl.

**Riesgos (honestos).**
1. **ABI pthread bionic↔musl** (riesgo técnico #1): si libmnl embebe `pthread_mutex_t/cond_t` con tamaño bionic y llama a nuestro musl → corrupción de memoria. Mitigado por (a) el core abstrae la mayoría del locking vía callback nuestro y (b) mini-shim pthread con layout bionic.
2. **Versión del core**: `libmnl_6628.a` AOSP vs DSP-ROM del krillin. Bajo riesgo (el 0xAAF0 lo fija la ROM del DSP, estable en la generación); si no casa → P2 con nuestro `.so` exacto.
3. **TCXO/reloj**: si `hw_Clock_Freq/Drift/u1ClockType` van mal, el DSP no adquiere. Extraer de NVRAM/gps.conf stock.
4. **ioctls COMBO_IOC_* sobre stpgps mainline**: el código tolera fallo, pero quizá añadir stubs de ioctl en `gps.c`.
5. **~120 símbolos libc/libm/libgcc** (fortify/errno/legacy) — triviales pero hay que cubrirlos todos (presentes en musl).
6. **Cold start**: sin AGPS/EPO el primer fix autónomo puede tardar minutos y necesita hora válida + cielo despejado. Aceptable para validar.

**Recomendación.** **GO por la vía PORTAR/CORRER la pila stock**, no por adivinar/capturar el START. Orden: descartar como endgame la RE del blob y la captura del START (guardarla solo como debug) → Fase A (runner + link) → Fase A' (shims) → Fase B (HW + TCXO reales + log del START) → Fase C (NMEA → gpsd/geoclue/Phosh, retirar `mtkgps_aaf0.c`) → P2 solo si el `.a` no casa. **Precedente**: en ports downstream MT65xx, `mnld`+`libmnl.so` stock corre bajo Android y da fix — prueba de que "correr el core intacto" funciona; la novedad es hacerlo sobre **mainline + musl + STP propio** (equivalente a lo logrado con el WiFi).

---

### 3.2 Cámara — **NO-GO**

**Arquitectura (breve).** ISP MTK + sensor + pipeline propietario (3A, tuning) gestionado por una **HAL cerrada** de Android, sin equivalente mainline para esta generación.

**Estado mainline / stock.** Sin framework destino viable ni source utilizable para el pipeline completo. La cámara se documenta como **límite conocido** del port, en la misma categoría que el módem.

**Recomendación.** **NO-GO.** No abordar. Registrar en la doc como una de las dos limitaciones aceptadas (cámara + llamadas de módem) de un BQ de 2014 que ya hace WiFi + audio + suspend + GUI + vídeo + sensores en mainline.

---

### 3.3 Módem 2G/3G (CCCI/dual_ccci + MOLY) — **MOONSHOT**

**Arquitectura (breve).** El módem vive en un **core separado (MD/baseband)** dentro del SoC, distinto de los 4× Cortex-A7 (AP). **No es AT por UART**: es un co-procesador que corre la baseband propietaria **MOLY**. Transporte AP↔MD = **CCCI sobre CCIF v1** (mailbox HW) + shared-mem de 2 MB + carveout de RAM para la imagen del MD (~10-22 MB). El krillin usa **dual_ccci** (variante CCIF/shared-mem de KitKat 3.4), no eccci/CLDMA (eso es LTE). Boot del MD: `ccci_power_on_md()` → `ccci_load_firmware(LOAD_ALL_IMG)` copia MOLY → **handshake de 2 fases** (`MD_INIT_START_BOOT` eco → HS1; `MD_BOOT_READY` → HS2 = arriba). Userspace: **RIL propietario `mtk-rild`** sobre chardevs (`ccci_tty/ipc/rpc`); datos por netdev `ccmni`. **No es AT/QMI/MBIM.**

**Dónde vive el stack stock.** Downstream en la Pi: `dual_ccci/` (~12.4k LOC C: `ccci_md_main.c` boot/handshake, `ccci_hw.c` capa CCIF, `ccmni*.c` datos IP) + `dual_ccci/mt6582/` (~3.4k LOC: layout mem, carga imagen, RPC SECRO/masp). **Total port ~15.8k LOC.** Es **source** (KitKat 3.4/AOSP). Blobs: firmware **MOLY** (partición modem, firmado) + **NVRAM/SECRO** (IMEI/cal RF) + binario `mtk-rild`.

**Estado mainline.** **CERO.** No hay CCCI/dual_ccci ni eccci en mainline, ni ningún MT65xx con llamadas, ni intento conocido de port. El único WWAN MTK mainline es **t7xx = 5G PCIe** (arquitectura distinta, no aplica). **ModemManager NO tiene plugin MTK/RIL**; el único destino de un RIL propietario es **oFono rilmodem** (vía libhybris/Halium).

**Plan de porteo por fases.**
- **Fase 0 (recon, ~1 sem)** — DT con reserved-memory (carveout MD + 2 MB CCCI, alineado 2M), mapear `AP_CCIF_BASE`/IRQ del MD, localizar power-on/reset del MD y estado de secure-boot (fusibles).
- **Fase 1 = M1 (core intacto + shims, 3-6 sem)** — portar `dual_ccci/mt6582` a 7.0.12 lo más intacto posible. Shims: reserved-memory/CMA, MPU EMI, clocks/power-domain del MD, IRQ/GIC, drift API chardev/proc 3.4→7.0, stubs de aee y masp/SECRO. **Meta M1: el MD arranca + handshake CCCI de 2 fases completo (`MD_BOOT_STAGE_2`) + eco de canal de control.** SIN RIL, SIN llamadas.
- **Fase 2 (canales + datos, 2-4 sem)** — chardevs lógicos + netdev `ccmni`.
- **Fase 3 (puente RIL, 2-4 meses, prob. baja)** — el infierno: `mtk-rild` bajo **libhybris** contra los chardevs CCCI (modelo Halium) + cambiar telefonía de ModemManager a **oFono rilmodem** (GNOME Calls/Chatty apuntan a oFono).

**Dependencias.** DT reserved-memory · secuencia power/clk del MD (**ausente en mainline, tipo SPM**) · MPU EMI · blob MOLY + NVRAM/SECRO · estado de fusibles secure-boot · libhybris + `mtk-rild` (Fase 3) · oFono rilmodem · GNOME Calls/Chatty.

**Riesgos.** Secure-boot fusado bloquea MOLY propio · **power/clk del MD sin SPM mainline = riesgo #1 de M1** · **el playbook WiFi NO encaja del todo** (la "capa baja ya probada" no existe aquí: CCIF, power/reset, carveout+MPU y RPC/SECRO se construyen de cero — se pierde la mayor ventaja del WiFi) · integración es el killer (no hay framework WWAN mainline; el único destino arrastra Halium) · CVEs conocidas del CCCI · NVRAM/IMEI/cal RF por dispositivo + cuestiones regulatorias.

**Recomendación.** **NO entrar** salvo aceptar explícitamente que es una apuesta de investigación. Si se acepta, **solo el spike M1 acotado** (Fase 0+1, 4-7 sem) con **puerta de decisión dura**: si el MD no arranca limpio en ~6 sem (bloqueo por power/clk o secure-boot), **ABORTAR** — no tocar el RIL. **Alternativa pragmática si se quieren llamadas de verdad**: no es mainline, es **Halium/Ubuntu Touch** (ahí ya funcionó en este mismo móvil). Estimación total realista de mainline-nativo: 4-8+ meses, prob. acumulada ~10-15%.

---

### 3.4 Drivers de pulido — **CONDITIONAL-GO (agregado)**

Los 8 caen en **3 clases** y por eso el playbook-WiFi no aplica igual a todos:
- **Clase A (100% userspace, no tocan kernel)**: (8) Fluidez botón, (1) Auto-brillo. HW ya resuelto — no hay stock que portar.
- **Clase B (driver mainline existe → DT + datos + config)**: (4) Thermal, (7) Magnetómetro, (6) BTCVSD.
- **Clase C (port de source downstream estilo-WiFi)**: (3) Accdet, (5) STP core, (2) FM.

Todo es **source GPL** del downstream MTK 3.10 en `cpcd@192.168.0.123:~/mainline/downstream/drivers/misc/mediatek/` (ningún blob nuevo salvo el FW del BT ya cargado).

#### (8) Fluidez botón power — **GO** (1-2 d)
- **Problema**: el daemon `mt6582-powerkey` hace `su`+gdbus nuevos por acción (latencia 1-2 s). HW resuelto (mtk-pmic-keys, event1, probado #230).
- **Plan (1 fase, userspace)**: reescribir como **daemon persistente en la sesión** que ya tenga gdbus/D-Bus y EVIOCGRAB abiertos; recibe el evento por evdev y ejecuta Lock/blank **sin fork**. Debounce fino. Latencia ~0.
- **Riesgo**: ninguno relevante.

#### (1) Auto-brillo ALS — **GO** (1-2 d)
- **HW resuelto**: TMD2772 @0x39 por IIO `tsl2772`, **iio-sensor-proxy ya reporta lux** (D-Bus SensorProxy AmbientLight). Backlight = `mt6582-backlight.py` (PWM real por /sys). **Falta SOLO el lazo lux→duty.**
- **Plan (1 fase)**: mini-daemon que hace `ClaimLight` sobre `net.hadess.SensorProxy`, lee `LightLevel`, aplica curva lux→duty con histéresis + techo/suelo + suavizado, y escribe por el **mismo canal que el slider** (`/run`).
- **Riesgo**: **NO usar gsd-power** — resucita la regresión de auto-suspensión ya diagnosticada. Aislar solo el lazo ALS→backlight.

#### (5) STP core resync/CRC — **GO condicional** (1-2 d)
- **Es NUESTRA capa baja** (`mt6582-btif.c` simplificado, sin CRC). El FW ignora CRC TX → basta CRC-check RX + resync.
- **Plan**: **Fase 1** integrar el **parche resync-RX ya escrito** (recuperación de trama por búsqueda de preámbulo). **Fase 2** (solo si se observa corrupción BT/GPS) portar el validador/rearmado de `stp_core.c`. **No portar stp_core entero sin necesidad** (trabajo especulativo).

#### (4) Thermal CPU — **CONDITIONAL-GO** (1-2 sem)
- **Arquitectura**: sensor on-chip = controlador térmico que **muxea AUXADC canal 11** por AHB (TEMPADCMUX/EN/VALID/VOLT) + **calibración efuse** (g_o_slope/PTPOD). **Misma IP** que `drivers/thermal/mediatek/auxadc_thermal.c` mainline. `mt6577_auxadc` ya probea mt6582.
- **Stock**: `thermal/mt6582/mtk_ts_cpu.c` + coolers.
- **Plan**: **Fase 1** extraer offsets del banco térmico MT6582 + constantes ADC + fórmula slope/intercept + origen del efuse. **Fase 2** añadir `struct mtk_thermal_data` MT6582 a `auxadc_thermal.c` + compatible + nvmem-cell en DTS + thermal-zones con trips y cpufreq-cooling. **Fase 3 (fallback)** `thermal-generic-adc` para throttle grueso.
- **Riesgo**: que la IP de 2013 no calce byte-a-byte con el driver más nuevo → RE de registros TEMP*/efuse. Mitigación: fallback grueso. **Es el único del lote con riesgo real**; merece su propia iteración.
- **Precedente**: `mtk-thermal` soporta MT2701 (2014, muy cercano, misma arquitectura AHB→AUXADC+efuse).

#### (3) Accdet jack — **CONDITIONAL-GO** (1 sem)
- **Arquitectura**: bloque ACCDET del PMIC **MT6323** (comparador ADC + micbias + EINT interno + debounce), IOCTL-driven. **No hay driver mainline** para MT6323 accdet.
- **Stock**: `accdet/mt6582/accdet.c` (1613 LOC) + `accdet_drv.c` + customs.
- **Plan (playbook completo)**: Capa1 reusar EINT/AUXADC del MT6323 (ya es interrupt-controller en DT). Capa2 portar el core (secuencia comparador + micbias + debounce + **tabla de umbrales ADC de botones**) lo más intacto posible. Capa3 shims KAL→regmap del pwrap/mt6397. Capa4 exponer jack por **extcon → ALSA jack** (auto-routing altavoz/auricular) + KEY_MEDIA/VOLUP/DOWN inline. **Alternativa barata**: polling userspace del AUXADC del jack sin driver.
- **Riesgo**: sin la tabla de umbrales exacta los botones inline se detectan mal (riesgo medio de RE de constantes).

#### (7) Magnetómetro MMC3516x — **GO técnico, BAJA prioridad** (2-4 d)
- **Arquitectura**: MMC3516x @0x30, I2C simple (SET/RESET coil + measure X/Y/Z 16-bit). `mmc35240` mainline es MEMSIC 0x30 pero cubre otros parts (mapa de registros distinto).
- **Plan (1 fase)**: driver IIO ~300 LOC modelado en `mmc35240.c`, tomando el mapa de `mmc3516x.h` del downstream. DT i2c node.
- **Riesgo/valor**: **sin consumidor en Phosh** (la brújula no lo usa) + requiere calibración hard-iron → retorno casi nulo. Solo por completitud IIO.

#### (6) BTCVSD (audio BT SCO) — **DEFER** (1-2 sem)
- **Arquitectura**: códec CVSD que DMAea PCM SCO a/desde SRAM del BT (bank2 @0x18080000, IRQ 218, máscara INFRA_MISC bit11). `sound/soc/mediatek/common/mtk-btcvsd.c` mainline **existe** (misma arquitectura, genérico SRAM+IRQ).
- **Plan**: Fase 1 DT node (reg SRAM + IRQ + máscara). Fase 2 verificar routing SCO al path CVSD del AP. Fase 3 integrar con AFE mainline como DAI BT-SCO + UCM.
- **Riesgo/valor**: aunque el driver funcione, **sin routing SCO del FW no hay audio**; y **sin módem la utilidad se limita a headset BT en VoIP**. Alto esfuerzo / valor bajo → **DEFER**.

#### (2) FM Radio — **NO-GO** (4-6 sem)
- Función WMT tipo=1 por STP + interfaz V4L2, ~93 ficheros / ~29K LOC efectivos, **cero base mainline**, utilidad casi nula. **Aparcar indefinidamente** (junto a cámara/módem).

---

## 4. Nota final honesta — qué cierra de verdad "es un teléfono" y qué es investigación pura

**Lo que un BQ Aquaris E4.5 (2014) ya hace en MAINLINE hoy** es enorme y poco común: WiFi WPA2+DHCP, audio (auriculares+altavoz), vibración, LEDs, botón power, RTC, brillo por slider, GUI Phosh, vídeo y sensores. Eso ya es un dispositivo utilizable.

**Cierran de verdad la sensación de "es un teléfono terminado" (hacer):**
- **Ola 1 (dos tardes)**: fluidez botón + auto-brillo + STP resync. Cero riesgo, máximo impacto percibido.
- **Ola 2 (semanas, con retorno real)**: thermal (seguridad/longevidad), accdet (cierra el audio con auto-routing de jack) y — la pieza grande de verdad — **GPS**. El GPS es el único subsistema "de teléfono" pendiente que es **alcanzable** (CONDITIONAL-GO, 3-6 sem) y **novedad genuina** (primer combo MTK host-based en mainline+musl). Con GPS + geolocalización en Phosh, el móvil pasa de "utilizable" a "completo para uso diario sin SIM".

**Es investigación pura, no roadmap de producto (no confundir):**
- **Módem** — **MOONSHOT** honesto: hay ruta y precedente (UT/Halium en este mismo móvil), pero **mainline-nativo tiene prob. ~10-15% y coste de meses**, y el playbook WiFi **no** aplica (no existe la capa baja probada; hay que construir CCIF/power/carveout/RPC de cero) y la integración obliga a abandonar mainline-nativo (Halium/libhybris/oFono). Entrar **solo** como apuesta explícita, **solo** el spike M1, con **puerta de aborto dura a las ~6 semanas**. Si lo que se quiere son llamadas reales, el camino sensato **no es mainline**: es Ubuntu Touch/Halium, donde ya funcionaron.
- **Cámara** y **FM Radio** — **NO-GO**. Coste alto/enorme, base mainline nula o inexistente, utilidad casi cero.

**La línea honesta**: las llamadas de módem y la cámara son los **dos límites conocidos aceptados** de este port, exactamente como se documentó. No son fracasos: son decisiones de coste-oportunidad sobre un móvil de 2014 que ya funciona para casi todo. El mejor uso de la energía es **cerrar GPS** (la última función de teléfono realista), rematar los quick-wins de pulido, hacer thermal por seguridad, y **declarar las *phone-features* completas documentando módem y cámara como límites** — un resultado ya excepcional para MT6582 en mainline.