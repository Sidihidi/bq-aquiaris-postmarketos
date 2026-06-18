# M3a — "el CONSYS habla": BTIF + STP + WMT (guía de implementación para casa)

Objetivo: que el AP envíe un comando WMT al CONSYS por BTIF y reciba la respuesta.
Eso prueba el canal de control AP↔CONSYS (como M1 probó el power). Es el cimiento de
**las 4 radios** (WiFi/BT/GPS/FM) — ver [[HITO-WIFI-CONSYS]].

Estado: **M1 (power, chip-id=0x6582) ✓ · M2 (firmware en /lib/firmware) ✓**. Aquí va M3a.

## Capas (de abajo a arriba)
```
WMT (handshake + descarga FW)   <- comandos {type,opcode,len,payload}
  STP (Serial Transport Proto)  <- trama 4B header + payload + CRC16, resync 4x0x7F
    BTIF (transporte serie PIO)  <- mt6582-btif.c (HECHO: init + tx/rx por bytes)
      CONSYS (encendido en M1)    <- 0x18070000, chip-id 0x6582
```

## 1. BTIF (transporte) — `mt6582-btif.c` LISTO
- Base física **0x1100C000**, IRQ **GIC_SPI 50** (DMA TX/RX = SPI 71/72, no usados en PIO).
- Es un 16550 extendido. Regs: RBR/THR 0x00, IER 0x04, IIR/FIFOCTRL 0x08, FAKELCR 0x0C,
  **LSR 0x14** (DR=1<<0, THRE=1<<5, TEMT=1<<6), DMA_EN 0x4C, TRI_LVL 0x60, WAK 0x64.
- PIO: TX = poll LSR.THRE → write THR; RX = IRQ (IER.RXFEN) → read RBR mientras LSR.DR.
- El driver inicia BTIF, manda un resync (4×0x7F) y loguea los bytes RX. **Sin baud** (enlace
  interno de tasa fija). DT: nodo `btif@1100c000` (ver `consys-dt-btif-snippet.dts`).
- **PENDIENTE/INCIERTO del transporte:**
  - **Clock del BTIF**: en INFRACFG; con `clk_ignore_unused` el LK puede dejarlo on. Si BTIF no
    responde, ungate el clock del BTIF (buscar el bit en mt_clkmgr downstream: `MT_CG_INFRA_BTIF`).
  - **Reset del MCU CONSYS**: para que el ROM corra y escuche STP, soltar el reset:
    `TOPRGU 0x10007000 + 0x18`, bit **12**, key **0x88<<24** (CONSYS_CPU_SW_RST). En M1 NO se hizo
    (downstream lo difería al "FW patch"). **Probar a soltarlo ANTES de hablar por BTIF.**

## 2. STP (framing) — POR IMPLEMENTAR encima del BTIF
- `MTKSTP_HEADER_SIZE = 4`. Header (de downstream `stp_core.c` ~824):
  ```
  hdr[0] = 0x80 | (seq << 3) | ack    // bit7=1 marca STP; seq/ack de la secuencia
  hdr[1] = (type & 0x0f) | (length_hi << 4)?   // TYPE = canal: WMT=? confirmar en stp_core
  hdr[2] = length_low                 // longitud del payload
  hdr[3] = (hdr[0]+hdr[1]+hdr[2]) & 0xff   // checksum del header
  ```
  ⚠️ El reparto exacto de TYPE/LENGTH en hdr[1..2] hay que confirmarlo leyendo `stp_send_data`
  completo en `stp_core.c` (la versión que vimos era un ACK con length=0). El canal/type para WMT
  es el índice de función (WMT=0 en la tabla STP de funciones).
- Tras el payload va el **CRC16** (2 bytes). Algoritmo (stp_core.c):
  `crc = (crc>>8) ^ crc16_table[(crc ^ byte) & 0xff]`, init 0 → **copiar `crc16_table[256]` tal cual
  de `stp_core.c`** (es CRC16 reflejado, no reinventar).
- **resync** = 4 bytes 0x7F (ya lo manda el driver) — sincroniza la máquina STP del CONSYS.

## 3. WMT (comandos) — POR IMPLEMENTAR encima de STP
Formato del paquete WMT (payload que mete STP): `[type][opcode][len_lo][len_hi][payload...]`
con type: 0x01=CMD, 0x02=EVT. Bytes reales (de `wmt_ic_soc.c`):
```
WMT_QUERY_BAUD_CMD  = {01 04 01 00 02}          ->  primer comando del init
WMT_RESET_CMD       = {01 07 01 00 04}  EVT {02 07 01 00 00}
WMT_PATCH_CMD       = {01 01 00 00 00}  EVT {02 01 01 00 00}   (cabecera de cada fragmento de patch)
WMT_WAKEUP_*_GATE_CMD = {01 03 01 00 04/05}
WMT_GET_SOC_ADIE_CHIPID_CMD = (leer chip-id por WMT)
```
**Secuencia de arranque** (de `mtk_wcn_soc_sw_init` + tabla `INIT_CMD`):
`(resync) → QUERY_BAUD → [SET_BAUD] → WMT_RESET → GET chip-id/hw_ver/fw_ver → descargar patch →
descargar WIFI_RAM_CODE → "ready"`.
- Para M3a-mínimo basta: **resync → enviar QUERY_BAUD (envuelto en STP) → leer EVT**. Si vuelve
  algo coherente con `WMT_QUERY_BAUD_EVT_*`, **el CONSYS habla** = M3a conseguido.

## 4. Descarga de firmware (ya en /lib/firmware, M2)
- Patch `mt6572_82_patch_e1_0/1_hdr.bin`: cabecera `WMT_PATCH` (28B: ucDateTime[16] + ucPLat[4]
  ("6582") + u2HwVer + u2SwVer + u4PatchVer) + payload. Se trocea y se manda con `WMT_PATCH_CMD`
  por fragmentos (ver `mtk_wcn_soc_patch_dwn`).
- Luego `WIFI_RAM_CODE_MT6582`. `request_firmware("mt6572_82_patch_e1_0_hdr.bin")` etc. los lee de
  /lib/firmware (ya están). `WMT_SOC.cfg`: `co_clock_flag=0` (CONSYS usa su clock, VCN28 modo HW).

## 5. Plan de prueba (en casa)
1. Compilar con `mt6582-btif.o` + nodo DT btif. Flashear. `dmesg|grep btif`:
   - Ver `BTIF init OK, resync enviado; RX=N bytes: ...`. **¿RX>0?** → el CONSYS contesta algo
     (¡buena señal!). ¿RX=0? → revisar (a) reset del MCU, (b) clock BTIF, (c) que el resync sea lo
     que espera, (d) que el CONSYS siga encendido (M1).
2. Implementar STP (copiar crc16_table) + enviar QUERY_BAUD; loguear el EVT crudo. Comparar con
   `WMT_QUERY_BAUD_EVT_*`. Iterar el header STP hasta que el CRC/seq cuadren.
3. Con QUERY_BAUD+RESET respondiendo → portar `mtk_wcn_soc_patch_dwn` (descarga de patch) y
   WIFI_RAM_CODE. Eso es ya M3 completo.

## 6. El premio (por qué merece la pena)
Con el WMT vivo, `mtk_wcn_wmt_func_on(tipo)` enciende cada radio:
- **BT** (lo más fácil): exponer `/dev/stpbt` (HCI) → **BlueZ** estándar. Sin driver propio.
- **GPS**: `/dev/stpgps` → NMEA → gpsd.
- **FM**: control simple.
- **WiFi**: el más duro (802.11/cfg80211, 133K líneas — M4).

## Archivos
- `mt6582-btif.c` (este dir): transporte BTIF PIO — LISTO para compilar.
- `consys-dt-btif-snippet.dts`: nodo DT del BTIF.
- Referencia downstream (Pi `~/mainline/downstream/drivers/misc/mediatek/`):
  `btif/mt6582/inc/hal_btif.h` (regs), `conn_soc/common/core/stp_core.c` (STP+crc16_table),
  `conn_soc/common/core/wmt_ic_soc.c` (secuencia+comandos), `conn_soc/common/core/include/wmt_core.h`
  (struct WMT_PATCH).
