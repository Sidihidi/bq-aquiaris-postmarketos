# Port del wmt_lib — análisis del bring-up del CONSYS vs stock (0716)

Contexto: el usuario eligió "entrar al port del wmt_lib" tras agotar los candidatos baratos del GPS RF
(ruido 0xCA = el correlador no engancha; WiFi/BT funcionan, GPS 0 sats). Objetivo: cargar el bring-up
CORRECTO del stock para que el DSP del GPS adquiera.

Fuente RE: `github.com/bq/aquaris-E4.5` → `mediatek/kernel/drivers/conn_soc/common/core/{wmt_ic_soc,wmt_func,wmt_ctrl,wmt_core}.c`
y `mediatek/platform/mt6582/kernel/drivers/wmt/{mtk_wcn_consys_hw,wmt_plat_alps}.c`.

## 1. El bring-up COMPLETO del stock (`mtk_wcn_soc_sw_init`, wmt_ic_soc.c:647)

Orden exacto (BTIF hif):
1. `init_table_1_2` — QUERY_STP default (`01 04 01 00 04`) → EVT default `...11 00 00 00`  *(TEST-ONLY)*
2. `init_table_4` — **SET_STP** (`01 04 05 00 03 DF 0E 68 01`) → pone el chip STP en modo `DF 0E 68 01`
3. host STP → `WMT_STP_CONF_MODE = MTKSTP_BTIF_FULL_MODE` (bit2) + `WMT_STP_CONF_EN=1`
4. `init_table_5` — QUERY_STP (`01 04 01 00 04`) → EVT `...DF 0E 68 01`  *(TEST-ONLY, confirma el set)*
5. `patch_info_prepare` + **multi-patch**: para cada patch → PATCH_ADDRESS_CMD + PATCH_P_ADDRESS_CMD
   (address en [12..15]) + N frags + `init_table_3` (WMT_RESET `01 07 01 00 04`) tras CADA patch
6. **RF-cal**: BT_PALDO ON + WIFI_PALDO ON → `calibration_table` (START_RF_CALIBRATION `01 14 01 00 01`)
   → BT_PALDO OFF + WIFI_PALDO OFF
7. `wmt_stp_init_coex()` (COEX)
8. `crystal_triming_set()` (byte 0x6D del NVRAM WIFI = 0x00 → no-op, ya verificado)
9. co_clock: `gCoClockEn = WMT_CO_CLOCK_DIS` por defecto → **rama osc_type SALTADA** en el krillin
10. `merge_pcm_table` (I2S/DAI = audio, NO GPS)

`wmt_func_gps_on` (wmt_func.c:500), config krillin (co_clock OFF, host-LNA, sin BT/WiFi):
- co_clock branch (VCN28/GPS_PALDO) → **SALTADA** (co_clock off)
- `gps_pre_on`: GPS_SYNC reg (chip `0x80050078` bits[30:28]) + GPS_LNA (GPIO47 high)
- `gps_ctrl(FUNC_ON)`: `01 06 02 00 02 01`
- de-sense: **solo si BT_ON||WIFI_ON** → SALTADO en pmOS (WiFi off)

## 2. Diff contra NUESTRO btif (`mainline/drivers/done/bt/mt6582-btif.c`, `bring_up_chip`)

Nuestro bring-up: `GEN_HCR → patch(e1_1,addr 0e f0) → RESET → patch(e1_0,addr 06 00) → RESET → rfcal → coex → gsync → func_on`.

| Paso stock | ¿lo hacemos? | nota |
|---|---|---|
| GEN_HCR | ✅ | |
| init_table_1_2 (query stp) | ❌ | test-only |
| **init_table_4 (SET_STP `DF 0E 68 01`)** | ❌ | **candidato: config STP FULL del chip** |
| **host STP BTIF_FULL_MODE** | ❌ | usamos modo simple |
| init_table_5 (query stp) | ❌ | test-only |
| PATCH_ADDRESS + P_ADDRESS + frags | ✅ | addresses OK (ver §3) |
| RESET tras cada patch | ✅ | RESET-1, RESET-2 |
| **BT/WIFI PALDO ON alrededor de rfcal** | ❌ | **candidato: entorno de la RF-cal** |
| calibration_table (rfcal) | ✅ | `01 14 01 00 01` byte-idéntico |
| coex | ✅ | |
| crystal_trim | n/a | byte=0 |
| co_clock/osc_type | n/a | co_clock DIS |
| merge_pcm | ❌ | audio, no GPS |
| GPS_SYNC + LNA + func_on | ✅ | **coincide con `wmt_func_gps_on`** |

**Conclusión: nuestro bring-up del GPS ya coincide con el stock salvo 3 huecos** (SET_STP/FULL, PALDO-gating de la cal, merge_pcm[audio]). El encendido GPS (GSYNC+LNA+func_on) es correcto.

## 3. ★ El patch: ROMv1 es el version-correcto; las addresses YA son correctas

Headers de los patches ROMv1 (28B = fecha[16] + "ALPS"[4] + hwVer[2] + swVer[2] + patchVer[4]):

| Patch | tamaño | frags | hwVer | swVer | últimos 2B header |
|---|---|---|---|---|---|
| ROMv1_patch_1_0 | 80812 | 81 | **0x008a** | 0x008a | `06 00` |
| ROMv1_patch_1_1 | 21488 | 21 | **0x008a** | 0x008a | `0e f0` |

Nuestras addresses hardcodeadas: `a_e1_0 = 00 00 06 00`, `a_e1_1 = 00 00 0e f0`.
→ **COINCIDEN byte a byte con los últimos 2B de los headers ROMv1** → la address de descarga NO era el
problema del cuelgue del ROMv1. (Las addresses son fijas de la familia mt6582, no del variant.)

**Ground-truth: el stock (LineageOS) en ESTE chip carga ROMv1.** La selección del stock
(`mtk_wcn_soc_find_wmt_ic_info(hw_ver)`, wmt_ic_soc.c:1266) empareja el patch con el `hw_ver` del chip
(GEN_HVR). → **ROMv1 es el patch version-correcto**; el `mt6572_82` es lo que pmOS usó por defecto
(tolerado para WiFi/BT, pero el DSP del GPS falla). WiFi/BT funcionando NO prueba que el DSP del GPS
esté bien: el patch parchea bugs de la ROM; si no es el correcto, WiFi/BT (menos sensibles) van igual
pero el GPS (más acoplado a la ROM/patch) da ruido.

## 4. Por qué colgó el ROMv1 antes (indistinguible desde el código)

El ROMv1 se descargó (81 frags OK) pero el chip quedó MUDO tras el RESET. Con las addresses ya correctas
(§3), las causas posibles NO se distinguen desde la fuente:
- (a) ROMv1 exige la negociación STP (`init_table_4` SET_STP + FULL) que el mt6572_82 tolera sin ella;
- (b) falta el PALDO-gating de la cal;
- (c) el orden/estado exacto del multi-patch;
- (d) un paso del `sw_init` que ROMv1 requiere y mt6572_82 no.

## 5. PLAN — vía empírica (como se cerró el WiFi WPA2)

La saga WiFi se cerró con **"el command stream canónico del core stock"**. Mismo enfoque aquí:

### Fase 1 (definitiva, necesita el móvil en LineageOS): CAPTURAR el bring-up canónico
Habilitar el logging WMT del driver stock en LineageOS y capturar TODA la secuencia de comandos WMT del
bring-up del CONSYS (de GEN_HCR a func_on), incluyendo el patch download real (addresses, orden) y las
init_table_1_2/4/5. El driver stock tiene `WMT_INFO_FUNC`/`WMT_DBG_FUNC` por todas partes.
- Ruta: `echo` al sysfs/proc de debug del wmt (`/proc/driver/wmt_dbg` o `wmt_aee`/`stpdbg`) → subir el
  nivel de log → `logcat`/`dmesg` durante el arranque del CONSYS (o forzar re-bring-up).
- Alternativa: strace del launcher (`6620_launcher`/`wmt_loader`) para ver los ioctl `SET_PATCH_INFO`
  (confirma addresses) + `stpbtif`/tracing del BTIF para los bytes crudos.
- Salida: la lista ordenada de comandos WMT (bytes) que el stock manda → nuestra "verdad".

### Fase 2 (código): replicar el stream exacto en `bring_up_chip`
Con el stream capturado, añadir a nuestro btif lo que falte, en el orden exacto. Candidatos ya mapeados:
- **init_table_4 (SET_STP `01 04 05 00 03 DF 0E 68 01`)** + host FULL mode (bit2). Requiere revisar si
  nuestra capa STP (mt6582-btif.c) hace CRC/secuencia; si no, es el cambio más grande.
- **PALDO-gating de la cal**: antes de `rfcal`, VCN33_BT (MT6323 `0x0416` bit5=1, +hwPowerOn 3.3V) y
  VCN33_WIFI (`0x0418` bit14=1) a **modo-HW**; después a modo-SW+off. (Regs vía el MFD mt6323.)
- Cargar **ROMv1** (ya horneable built-in; addresses `0e f0`/`06 00` correctas).

### Fase 3 (test): ROMv1 + stream correcto → leer `/dev/stpgps` → ¿frames reales (no 0xCA)? → sats.

## Estado del móvil
Degradado tras el test func_off (WMT no responde) → **necesita power-cycle**. Para la Fase 1 hay que
volver a LineageOS (`fastboot flash boot ~/images/lineage13-boot.img` desde fastboot).

*Mac (Opus), 2026-07-16. Bring-up del CONSYS mapeado byte a byte vs stock. Nuestro GPS bring-up ya
coincide salvo 3 huecos; ROMv1 es el patch correcto y sus addresses ya las tenemos. Siguiente =
capturar el command stream canónico en LineageOS (vía empírica, como el WiFi).*
