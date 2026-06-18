# M3a — "el CONSYS habla": BTIF + STP + WMT (guía de implementación para casa)

## ✅ FASE A RESUELVE EL BOOTLOOP (2026-06-18, `boot-btifA.img`) — EL BUS BTIF VIVE
Reescrito `mt6582-btif.c` a **Fase A pasiva**: ungate del clock PERI_BTIF + leer LSR/IIR,
**SIN `request_irq`, SIN `IER`, SIN TX**. Resultado en HW: **ARRANCA a Alpine/Phosh**, el BTIF
`probe returned 0`, sin abort/panic. dmesg:
```
faseA clock BTIF: PDN0_STA 0x00000000 -> 0x00000000 (bit20 gated: 0 -> 0)
faseA BTIF pasivo: irq=195 LSR=0x00000060 IIR=0x00000001 (bus vivo)
probe of 1100c000.btif returned 0 after 2869 usecs
```
- `bit20 gated: 0 -> 0` → **el clock PERI_BTIF YA estaba ON** (clk_ignore_unused/LK). El clock
  NO era la causa del bootloop.
- `LSR=0x60` = THRE|TEMT (TX holding+shift vacíos, valor SANO) → **el bus BTIF está listo.**
- → **CAUSA REAL DEL BOOTLOOP = la tormenta de IRQ**: la v1 hacía `request_irq` + `IER_RXFEN`
  con la ISR sin filtrar (lee RBR solo si LSR.DR, pero devuelve IRQ_HANDLED siempre); con la
  línea SPI en LEVEL_LOW asertada → re-dispara en bucle infinito. Sin registrar el IRQ, arranca.
- Clock BTIF (por si hiciera falta forzarlo): `MT_CG_PERI_BTIF`=bit20, ungate=`writel(1<<20,
  PERICFG 0x10003000 + PDN0_CLR 0x10)`, estado en PDN0_STA 0x18 (bit=1 → gated). Reset MCU
  CONSYS: en downstream está en `#if 0` (lo difiere al FW patch) → NO hace falta tocarlo.

### ✅ Fase C+D v2: EL TX DEL BTIF FUNCIONA (RX=0 al resync, esperado)
`boot-btifCD2.img`: `faseCD2: resync TX OK (LSR pre=0x60 post-init=0x60). RX=0 bytes`.
- **TX OK** (ya no hay timeout). Fixes vs v1: (1) el TX espera `LSR & (THRE|TEMT)` —cualquiera—
  igual que `_btif_is_tx_allow` del downstream (mi v1 esperaba solo THRE y daba timeout con
  LSR=0x40/TEMT=1); (2) init fiel a `hal_btif_hw_init`: FAKELCR normal + **HANDSHAKE on (0x6C bit0)**
  + TRI_LVL TX8/RX1 + DMA off/AUTORST, SIN habilitar IER (RX por polling, no IRQ).
- `LSR post-init=0x60` (THRE+TEMT) → el init deja el bus sano (v1 lo dejaba en 0x40).
- **RX=0 al resync** = esperado: `4×0x7F` solo sincroniza la máquina STP, no provoca EVT. Para
  que el CONSYS conteste hay que enviarle un **comando WMT** envuelto en **STP**.

### ▶ SIGUIENTE = Fase STP + WMT (que el CONSYS CONTESTE)
1. **STP framing** (de `conn_soc/common/core/stp_core.c`): header 4B `hdr[0]=0x80|seq<<3|ack`,
   type/length en hdr[1..2], `hdr[3]=checksum`; payload; **CRC16** (copiar `crc16_table[256]` tal
   cual; `crc=(crc>>8)^tbl[(crc^byte)&0xff]`, init 0). Antes del 1er paquete: resync `4×0x7F`.
2. **WMT QUERY_BAUD** = payload `{01 04 01 00 02}` envuelto en STP. Enviar, luego RX-poll y loguear
   el EVT crudo; comparar con `WMT_QUERY_BAUD_EVT`. Iterar el reparto type/len del header STP hasta
   que CRC/seq cuadren. **RX>0 con EVT coherente = M3a CONSEGUIDO.**
3. Si sigue RX=0: probar **WAK** (`set BTIF_WAK 0x64 bit0` para despertar ap_wakeup_consys) antes del TX.
4. Luego: `WMT_RESET` → GET chip-id/ver → descargar patch (`mtk_wcn_soc_patch_dwn`) + WIFI_RAM_CODE = M3.
5. IRQ (Fase E) al final, ISR con `IRQ_NONE` si `!(LSR&DR)`.

---

## ⛔ (HISTÓRICO) RESULTADO INICIAL: `boot-m3a.img` HACÍA BOOTLOOP en el logo BQ
`boot-m3a.img` (USER_NS + WiFi M1 + **BTIF v1**) **no pasaba del logo** (bootloop, cuelga MUY
temprano, antes de consola/switch_root). NO llegamos a leer ningún log. → resuelto arriba (Fase A).

### 🔧 RECUPERAR EL TELÉFONO (hacer ESTO primero)
Fastboot (Power 10s → Power+Vol↑, pantalla negra normal) y desde la Pi (`~/mainline/pkg`):
```
sudo fastboot flash boot boot-color1.img   # known-good: display+Phosh+lima+color
sudo fastboot reboot
```
(`boot-wifi1.img` también arranca bien y mantiene WiFi M1, si prefieres esa base.)

### 🐞 CAUSA MÁS PROBABLE = el driver BTIF (no USER_NS)
boot-wifi1 (M1 solo) arrancó OK; lo NUEVO en boot-m3a que cuelga es el **acceso al BTIF**. Sospechas
por orden de probabilidad:
1. **Tormenta de IRQ en SPI 50** (LEVEL_LOW): hago `request_irq` + habilito `IER_RXFEN`. Si el IRQ
   queda asertado y mi ISR no limpia la fuente real (solo leo RBR mientras LSR.DR), re-dispara en
   bucle → cuelga. (Mismo patrón que la tormenta del pwrap que ya nos pasó.)
2. **BTIF sin clock**: si el clock del BTIF (INFRACFG, `MT_CG_INFRA_BTIF`) está gated, `readl(BTIF_*)`
   da *external abort* → cuelga. `clk_ignore_unused` quizá NO lo cubre.
3. El **TX del resync** en probe se queda en el poll de LSR.THRE (acotado a 100ms, no debería colgar
   solo, pero suma).
4. Menos probable: **USER_NS** (es una feature de kernel, no suele romper el boot).

### 🧪 AISLAR EN CASA (orden recomendado)
1. Recuperar con boot-color1 (arriba).
2. Flashear **`boot-userns.img`** (ya está en `~/mainline/pkg`, = USER_NS + M1, **sin BTIF**):
   - **Si arranca** → USER_NS es inocente, **el BTIF es el culpable** (ir al paso 3). Y de paso
     **probar Epiphany** (con USER_NS el sandbox bwrap ya funciona): `WEBKIT_DISABLE_DMABUF_RENDERER=1
     epiphany` en la sesión Phosh.
   - **Si también hace bootloop** → USER_NS está implicado (raro): revisar deps del config.
3. **Arreglar el BTIF (driver pasivo primero):** reescribir `mt6582-btif.c` por fases, flasheando
   entre cada una:
   - **Fase A (pasivo):** solo `ioremap` + leer y loguear LSR/IIR (`dev_info`). SIN `request_irq`,
     SIN TX, SIN `IER`. ¿Arranca? ¿LSR es un valor sano (no 0xffffffff)? Si 0xffffffff o cuelga →
     es el **clock** (paso B). Si LSR sano → el bus está vivo.
   - **Fase B (clock):** ungate el clock del BTIF antes de leer (buscar `MT_CG_INFRA_BTIF` en
     `mt_clkmgr.c` downstream; es un bit de INFRACFG ~0x10001000). Soltar también el reset del MCU
     CONSYS (TOPRGU 0x10007000+0x18 bit12 key 0x88<<24).
   - **Fase C (TX polled):** init + `mt6582_btif_tx(resync)` (sin IRQ). Loguear LSR tras TX.
   - **Fase D (RX polled, sin IRQ):** tras TX, poll LSR.DR y leer RBR en bucle acotado (p.ej. 200ms).
     Loguear bytes. **Aquí ya se ve si el CONSYS responde** sin riesgo de tormenta de IRQ.
   - **Fase E (IRQ):** solo cuando todo lo anterior va, añadir `request_irq` + `IER_RXFEN`, con la
     ISR limpiando bien (y `IRQ_NONE` si no era para nosotros, para no colgar el GIC).


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
