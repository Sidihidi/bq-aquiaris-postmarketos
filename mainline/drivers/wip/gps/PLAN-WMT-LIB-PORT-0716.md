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

## ★★ FASE 1 EJECUTADA (0716): CAPTURADO el command stream canónico en LineageOS

Método: `adb root` → `/proc/driver/wmt_dbg`: `echo "9 4"` (WMT log LOUD) + `echo "a 4"` (STP log 4)
+ `echo "6 0"` (**whole chip reset** → re-corre el sw_init COMPLETO) + drenaje `dmesg -c` en bucle.
98MB de captura (`wmt-reset.log`). ⚠️ El log del kernel MTK entrelaza líneas de 2 CPUs (usar `grep -a`,
y filtro POSITIVO — el spam de bluedroid contamina las mismas líneas físicas).

### La secuencia REAL del stock (observada, no del código):
1. Rails ON (hwPowerOn powerId 8, 24) + CONSYS **HW RST** + BTIF open + STP reset
2. **PATCH_ADDRESS_CMD** → EVT(8B) — `01 08 10 00 01 01 00 01 3c 02 09 02 00 00 00 00 ff ff ff ff`
3. **PATCH_P_ADDRESS_CMD con addr[12..15] = `00 00 0e f0`** → EVT(8B)
4. **UN SOLO PATCH: 81 frags** (= `ROMv1_patch_1_0_hdr.bin`, 80812B exactos) — ¡NO hay segundo patch!
5. **WMT_RESET** (`01 07 01 00 04`) → EVT
6. **PALDO ON**: BT (VCN33_BT@3300 + HW-mode + VCN28_bt) y WIFI (t=612.547/612.550)
7. **RF_CALIBRATION** (`01 14 01 00 01`) → EVT — tarda **372ms**
8. **PALDO OFF**: BT y WIFI (612.925/612.935)
9. **coex_wmt** (ant_mode=1, como el nuestro)
10. **configure FM comm**: `01 05 02 00 02 02` → EVT `02 05 02 00 00 02` (init_table_5_1)

**Lo que el stock NO hace** (refutado del análisis estático del GPL): NO manda init_table_1_2/4/5
(NADA de SET_STP/QUERY_STP/FULL-mode: 0 ocurrencias en 98MB), NO lee hw_ver por WMT (lo lee por MMIO),
NO GEN_HCR, NO osc_type (co_clock=0), NO merge_pcm.

`WMT_SOC.cfg` del stock confirma nuestra config: `coex_wmt_ant_mode=1, wmt_gps_lna_pin=0,
wmt_gps_lna_enable=0, co_clock_flag=0`.

GPS-on (captura 1): GPS_SYNC vía ic_pin_ctrl (cmd WMT familia 0x08, EVT 8B) + LNA host GPIO + func_on
`01 06 02 00 02 01` → ok. **★DE-SENSE REFUTADO DEFINITIVO**: hasta en el stock muere en
`send_command_to_daemon: invalid native process pid` (no hay daemon) — y el GPS del stock VE 13 sats.

### ★ EXPLICACIÓN DEL CUELGUE ROMv1 (#298):
Cargamos DOS patches con el de 80KB en `06 00` = **dirección equivocada** (machaca otra región del
chip) + un `1_1` que el stock ni descarga. **FIX: UN solo patch `ROMv1_patch_1_0_hdr.bin` → `00 00 0e f0`.**
(La pareja "e1_1→0ef0 + e1_0→0600" era del flujo antiguo del mt6572_82; para ROMv1 NO aplica.)

### FASE 2 (build #301): cambios en `bring_up_chip` (mt6582-btif.c)
- `patch_dwn(b, "ROMv1_patch_1_0_hdr.bin", {0x00,0x00,0x0e,0xf0})` — ÚNICO patch + WMT_RESET.
- Mantener GEN_HCR (check de vida, inocuo), rfcal (VCN33 ya always-on en pmOS ≈ PALDO on), coex.
- Añadir FM comm (`01 05 02 00 02 02`/EVT `02 05 02 00 00 02`) tras coex (fidelidad).
- Mantener GSYNC + LNA + func_on como están (ya coinciden con el stock).
- HW-mode de VCN33 (0x416[5]/0x418[14]): NO en iter-1 (es power-mgmt post-cal; VCN33 ya on).
- Firmware built-in: ROMv1 ya horneado en #298 (verificar que sigue en firmware/ + EXTRA_FIRMWARE).

### FASE 3: flash + test
Desde Lineage: `adb reboot bootloader` → `fastboot flash boot <img#301>`. En pmOS: dmesg (81 frags,
bring-up OK, WiFi/BT siguen vivos) → `/dev/stpgps` → ¿frames con datos reales (no 0xCA)? → gpspipe sats.

## FASE 3 — iteraciones (0716 tarde)

**#301 (build completo, árbol restaurado): BOOTLOOP.** Causa: el árbol restaurado esta mañana tiene el
**DTS VIEJO** — le faltan el mt6323 interrupt-controller+RTC (fix #230), el carveout del módem, y tiene
**consys/btif/wifi en `status=disabled`**. El empaquetado era idéntico (mismo cmdline/initrd que el
bueno). **Fix #301c: zImage nuevo + dtb BUENO extraído del boot-gsync-lna.img** (el dtb va pegado al
final del zImage del boot.img; localizarlo por el magic `d00dfeed` con rfind). → **ARRANCA.**
⚠️ El DTS del árbol de la Pi sigue viejo — pendiente restaurarlo del dtb bueno (decompilar) o del árbol
de casa.

**#301c: la descarga del ROMv1 muere a mitad (FRAG timeout) → chip mudo.** ADDR y PADDR pasan bien;
un FRAG deja de recibir ACK a ~750ms del inicio y el chip queda mudo (GEN_HCR timeout en reintentos).
**CAUSA (medida en la captura canónica): el ROM del chip PAUSA los ACKs ~430-500ms periódicamente**
durante la descarga de 80KB (flow-control interno: ráfagas de ~8 frags + pausa — visible en los
timestamps de los RX dumps: 609.6→610.07→610.57→611.05→611.56→612.0). Nuestro `wmt_cmd` esperaba
**400ms** → timeout dentro de una pausa → abandono a mitad → chip desincronizado. El mt6572_82 nunca
lo sufrió (tandas de 21+41 frags con RESET entre medias, pausas cortas). El stock tarda 3.5s en los
81 frags (~43ms/frag de media CON las pausas). **Fix #301d: timeout de wmt_cmd 400→2000ms** + log del
seq del frag que falla.

**#301d (timeout 2000ms): la descarga llega al FINAL — "fallo en frag 81/81".** 80 frags ACKeados;
el ÚLTIMO frag (el que dispara la validación del patch completo en el ROM) nunca recibe ACK y el chip
queda catatónico (GEN_HCR mudo, `resync_drops=0` = ni basura). **#301e (12s en el último frag +
pacing 4ms/frag): IGUAL — frag 81/81.** No es timing.

**Descartado también**: contenido (md5 `9adccfae` idéntico repo=Pi=árbol=**blob dentro del vmlinux**),
formato del frag (header `01 01 len flag` byte-idéntico al stock, verificado en la fuente:
`WMT_PATCH_CMD` wmt_ic_soc.c:103), addresses (plantillas ADDR/PADDR byte-idénticas), ring DMA
(8KB, wrapea siempre — no es boundary).

**Hipótesis #301f (en test): el estado PRE-descarga del chip.** El stock hace **HW-RST del CONSYS
(MTCMOS+rails off→on) JUSTO antes de descargar** — el patch es el PRIMER tráfico WMT que ve el ROM
tras el reset (0.4s después). Nosotros descargábamos ~10s tras el power-on y tras VARIOS GEN_HCR
(probe t=2.3 + bring-up). El ROM acepta los frags (protocolo OK) pero la validación final del patch
falla → estado interno sucio. Fix: `mt6582_consys_hw_rst()` en mt6582-consys.c (apagado ordenado:
TOPAXI prot → SRAM_PDN → ISO → ~RST_B → CLK_DIS → ~PWR_ON → rails off; reencendido = orden del
probe) + el bring-up del btif lo llama y descarga DIRECTO (sin GEN_HCR previo).

*Mac (Fable), 2026-07-16. Iteraciones: #301 bootloop (DTS viejo) → #301c arranca pero frag-timeout →
#301d/e frag 81/81 (validación) → #301f = HW-RST pre-descarga (réplica fiel del stock).*

---
## RESULTADO #301f (0716, sesión casa Fable) — FALLÓ: el HW-RST NO arregla el frag 81/81
`#301f` construido, flasheado y arrancado. **El HW-RST funciona** (`HW-RST OK: chip-id=0x6582, ROM
corriendo`), pero **la descarga del ROMv1 SIGUE muriendo en `frag 81/81`** (`wmt_cmd[FRAG]: TIMEOUT` →
`patch ROMv1_patch_1_0_hdr.bin: fallo en frag 81/81`), el chip queda catatónico, `func_on(WIFI) fallo (-5)`,
bring-up abandonada tras 40 intentos. **La hipótesis del estado pre-descarga (HW-RST) queda REFUTADA.**

⇒ El chip acepta los 81 frags (con HW-RST limpio y timeout 12s en el último) pero la **validación final del
patch en el ROM falla igual**. Descartado ya: contenido (md5 idéntico), formato/addr (byte-idénticos),
timing (12s+pacing), estado pre-descarga (HW-RST). **Único candidato que queda del análisis §2: el modo STP
(`init_table_4` SET_STP `DF 0E 68 01` + host **BTIF_FULL_MODE**)** — el stock negocia STP FULL antes del
patch; nuestro btif descarga en modo simple. Hipótesis: el ROM valida/recibe el patch de 80KB de forma
distinta en FULL (con CRC+seq por paquete) vs simple → en simple un byte se corrompe en 80KB → checksum del
patch falla en el frag 81. **PERO implementar FULL_MODE = reescribir la capa STP entera (proyecto grande) y
resultado incierto** (aunque cargue el ROMv1, no está probado que arregle el 0xCA del GPS).

**Estado del móvil**: ROTO con `#301f` (CONSYS caído, WiFi/BT/GPS abajo; SSH sigue por USB). Imagen funcional
para restaurar: `~/mainline/pkg/boot-gsync-lna.img` (#300-era, mt6572_82+GSYNC+COEX+LNA, WiFi/BT/GPS-radio OK,
GPS 0xCA) o `~/boot-GOOD-backup.img` (Jul14).

*Casa (Fable), 2026-07-16. #301f REFUTADO (HW-RST no arregla frag 81/81). Queda solo FULL_MODE STP (grande e
incierto). Móvil roto → restaurar #300 recomendado antes de decidir.*
