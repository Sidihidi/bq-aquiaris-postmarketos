# M3a — "el CONSYS habla": BTIF + STP + WMT (guía de implementación para casa)

## 🎉🎉🎉🎉 ¡¡CONSEGUIDO 2026-06-19!! EL CONSYS RESPONDE (M3a LOGRADO) 🎉🎉🎉🎉
Con `mt6582-btif.c` en **modo DMA (TX+RX VFF)** enviamos GEN_HCR y **el CONSYS contestó 22 bytes**:
```
TX_VFF valid=0x0 rpt=0x1a wpt=0x1a BTIF_LSR=0x40   (el TX drenó, el BTIF transmitió)
RX=22: 80 40 10 00 02 08 0c 00 00 00 00 01 08 00 00 80   (¡respuesta STP/WMT válida!)
```
Header respuesta `80 40 10 00` = STP MAND, type=4 (WMT), len=16. **Canal BTIF↔CONSYS bidireccional OK.**

### 🔑 EL BUG QUE NOS BLOQUEABA (it.1 daba RX=0 + PC del MCU congelado):
**El TX-DMA no expulsa la cola parcial (<8 bytes) de un paquete sin un FLUSH explícito.** Nuestro
comando de 26 bytes (26 mod 8 = 2 bytes de cola) se quedaba atascado en el motor DMA → el MCU
recibía una trama INCOMPLETA y la ignoraba (por eso el PC no se movía). Hallado en `hal_dma_send_data`:
`if (0<VALID<8) _tx_dma_flush()`. El stock lo hace en su IRQ; nosotros (polled) a mano.
**Los 3 arreglos de la it.2 (`mt6582-btif.c` actual):**
1. **FLUSH** `TX_DMA_FLUSH @0x14 bit0` tras escribir WPT, en bucle mientras `0<VALID<8` hasta VALID==0.
2. **Formato WPT correcto**: offset en bits[15:0] (tope vff_size), **wrap en bit16** (`DMA_WPT_WRAP=0x10000`), no bit12.
3. **TX threshold = vff_size-7** (`DMA_TX_THRE`), STOP@0x10=0, EN antes de cada WPT.
RX: `valid = WPT-RPT` (wrap-aware), no el registro VALID (mask 0x1F raro).

### ▶ SIGUIENTE (con el canal ya abierto): parsear la respuesta hw_check (chip-id 0x6582, hw/fw ver)
→ secuencia completa: hw_check → **descarga del patch** (`mt6572_82_patch_e1`) → WIFI_RAM_CODE →
wlanProbe → **WiFi ON**. Y de ahí BT/GPS/FM casi gratis (mismo canal WMT). Ver §secuencia capturada abajo.

---

## 🏆🏆🏆 PLAN B LOGRADO (2026-06-19): CAPTURADA LA SECUENCIA REAL DE ANDROID
Vía **adb root sin auth** (default.prop ro.secure=0) en un stock con /data neutralizado + **busybox
estático** en el ramdisk (`adb shell`) + **/system montado a mano por loop+offset 0x5900000** (el
offset real = linear − 0x1400000, dicho por el propio kernel: `[Dumchar] android start=5900000`),
corrimos `wmt_loader` + `6620_launcher` + `echo 1 > /dev/wmtWifi` y capturamos el dmesg por
`adb pull /proc/kmsg`. **El WiFi arrancó del todo.** Captura completa en `captura/stock-consys-bringup.txt`.

### ✅ LO QUE NOS FALTABA (los 3 arreglos para nuestro driver M3a):
1. **BTIF en modo DMA, NO PIO.** El stock hace `_btif_tx_dma_setup` + `_btif_rx_dma_setup` y
   `btif_open: BTIF's Tx Mode:1, Rx Mode(1)` (1=DMA). **Nuestro M3a usaba PIO** → por eso RX=0 (la
   respuesta del CONSYS va por el VFF/DMA, no por el RBR). El TX **también** es DMA. → reimplementar
   el BTIF con VFF DMA en AMBAS direcciones (ya teníamos la receta RX-DMA en §3; aplicarla + TX-DMA).
2. **Handshake "abrir canal STP" ANTES de los comandos WMT** (lo que buscábamos): la secuencia es
   `mtk_wcn_stp_enable(0)` → `stp_ready(0)` → `stp_set_mode(PROTO=8)` → `stp_enable(1)` →
   hw_check → `stp_set_mode(PROTO=4)` → `stp_enable(1)` → `enable host STP-BTIF-FULL mode`.
   Sin esta secuencia de enable/mode el MCU no procesa (nuestro type=4 "no enganchaba").
3. **Modo STP: empieza simple y pasa a FULL.** `STP_SUPPORT_PROTOCOL=8` (¿MAND?) al inicio, luego
   `=4` (FULL/BTIF-FULL) tras el hw_check. El framing cambia con el modo.

### 📋 La secuencia EXACTA del bring-up (de la captura):
1. `func_ctrl OPID(3) type(3)` (func_on WiFi) → `consys_hw_pwr_on`: PMIC **VCN18+VCN28**, SPM
   `TOP1_PWR_CTRL=0xd` (= nuestro PWR_CON), `PWR_CONN_ACK=0x314e`, poll `chipId=0x6582` (retry).
2. `consys_hw_gpio_ctrl` (BGF IRQ registrado+disabled). Lee `WMT_SOC.cfg` (co_clock_flag=0).
3. **`mtk_wcn_btif_open` (owner CONSYS_STP)** → loopback OFF → `_btif_controller_setup` →
   **`_btif_tx_dma_setup` + `_btif_rx_dma_setup`** → `Tx Mode:1 Rx Mode:1` → `stp_btif_open OK` +
   `register rxcb to btif`.
4. **STP enable** (ver arriba) → `wmt_core_hw_check`: chip-id 0x6582, **hw_ver 0x8a01, fw_ver 0x8a00**,
   ic info `SOC_CONSYS.E2 patch_ext _e1`.
5. STP→FULL → **patch download** (2 patches, del launcher): `mt6572_82_patch_e1_1_hdr.bin`
   (addr cmd `00 00 0e f0`, frag(21,488)) y `..._e1_0_hdr.bin` (addr `00 00 06 00`, frag(41,896)).
   Cabecera patch: Built 20140904, HwVer 0x8a00, SwVer 0x8a00, PhVer 0x00fe, Platform 1432.
6. `stp_ready(1)` → **`[MT6582] HifAhbProbe()`** (el HIF AHB del WiFi) → `wlanNetCreate` (wiphy+netdev)
   → `wlanProbe: download firmware` → abre **`WIFI_RAM_CODE_MT6582`** → `nicVerifyChipID 0x6582` →
   `wlanAdapterStart: Ready bit asserted` → `download firmware status=0` → **WiFi func ON**.

### 🔧 DATOS PARA LA REESCRITURA BTIF-DMA (extraídos del downstream):
- **BTIF en modo DMA para TX y RX** (NO PIO): `hal_btif_tx_mode_ctrl(BTIF_MODE_DMA)` +
  `hal_btif_rx_mode_ctrl(BTIF_MODE_DMA)` (registro de modo en el BTIF core) + **new-handshake**
  `BTIF_HANDSHAKE(0x6C) bit0 = 1` (`btif_new_handshake_ctrl`). El stock loguea `Tx Mode:1 Rx Mode:1`.
- **APDMA VFF**: TX-DMA @**0x11000780** (IRQ SPI71), RX-DMA @**0x11000800** (IRQ SPI72). Programar
  ambos vFIFO (VFF_ADDR/LEN/THRE/WPT/RPT, INT_FLAG/EN, EN). Ring coherente. (Receta RX en §3, ya
  validada por loopback; falta el TX-DMA análogo.)
- **`mtk_wcn_stp_enable` es SOLO estado AP** (STP_SET_ENABLE en stp_core_ctx) — NO manda nada al MCU.
  Lo que hace responder al MCU es el **setup DMA completo del BTIF**, no el handshake suelto que
  probamos. → replicar `_btif_controller_setup`+`_btif_tx_dma_setup`+`_btif_rx_dma_setup` del stock.
- **Framing: MAND → FULLSET**. Empieza `btif_mand_mode` (header `80 (type<<4) len 00`, CRC 0000),
  cambia a `btif_fullset_mode` (con seq/ack/CRC) tras el hw_check (`STP_SUPPORT_PROTOCOL` 8→4).
- Plan probado-en-stock (boot-stockadb2.img): `losetup -o 93323264 loop0 mmcblk0; mount loop0 /system;
  /system/bin/wmt_loader; /system/bin/6620_launcher -p /system/etc/firmware/; echo 1 > /dev/wmtWifi`
  → comparar el dmesg con el nuestro paso a paso. **Es un port grande (BTIF-DMA + STP + WMT + patch)**.

### ▶ SIGUIENTE: reescribir mt6582-btif.c con BTIF-DMA (TX+RX VFF) + la secuencia STP-enable,
luego portar el patch download (mtk_wcn_soc_patch_dwn) y por último el HIF-AHB + el 802.11 (M4).
El blueprint completo está en la captura. **adb-en-stock queda como herramienta** para depurar/comparar
en vivo (boot-stockadb2.img + /system por loop 0x5900000).


## 🔬 SESIÓN 2026-06-19 (cont.): RX-DMA RESUELTO + el muro acotado a CONSYS→AP
Implementado el **RX por DMA/VFF** (`mt6582-btif.c`: rxdma_setup/drain/poll, APDMA-RX @0x11000800,
ring 8KB coherente, BTIF_DMA_EN.RX). Resultados en HW (boot-btifHCR/btifT0, logs legibles con
`log_buf_len=4M` + sin `initcall_debug`):
- ✅ **`LOOPBACK(DMA): RX=4 aa bb cc dd FUNCIONA`** → el RX por DMA del AP funciona perfecto. Era
  la pieza que la síntesis decía que faltaba. **RX-DMA RESUELTO.**
- ✅ **El MCU PROCESA** (con type=0): tras el resync el PC salta `0xe38→0x13e8c` y **cicla activo**
  por `0x13e7e/0x3952/0xf658/0x3980/0x394c` ~2s (k=0-8) — ejecuta handlers reales.
- ❌ **`WPT=0x4` NO avanza nunca** (quedó del loopback) → **el CONSYS no escribe ni 1 byte al BTIF.**
  El HW avanza WPT solo al recibir; no llega nada de la dirección CONSYS→AP.
- Al k=9 (~2s) el MCU entra en `exp_main 0x23d8` (excepciona, "jump from RST").
- **type=4** (WMT_TASK_INDX, correcto por `wmt_ctrl.c:345/348`): el MCU **NO reacciona** (PC=0xe38
  idle) — peor que type=0. El "fix type=4" de la síntesis estaba SIN probar en HW y empeora.
- Framing STP **validado exacto** vs `stp_core.c:899-902` MAND mode (`h0=0x80`, `h1=type<<4|len_hi`,
  `h2=len_lo`, `h3=0`, +CRC 00 00). Comando GEN_HCR validado vs `wmt_core_reg_rw_raw`.

**→ EL MURO REAL = la dirección CONSYS→AP del BTIF.** RX del AP ✓, TX del AP ✓, MCU procesa ✓.
Falta que el CONSYS TRANSMITA la respuesta. Dos frentes para casa (investigar, no disparar):
1. **Estabilidad del MCU (evitar exp_main):** el MCU excepciona ~2s tras procesar, quizá ANTES de
   poder responder. Probar: **MBIST** `CONN_MCU_CONFIG 0x18070110 |= (1<<18)` en `consys_activate_mcu`
   (la síntesis §1 lo señala, ACR leído=0x00300002 bit18=0); y/o la **secuencia de reset limpia**
   (assert CPU_SW_RST key → clocks/MBIST → deassert key) en vez de no-tocar-reset; y/o cargar el
   **patch** (mt6572_82_patch) en SRAM antes de esperar respuesta (el ROM puede requerirlo).
2. **Handshake RX CONSYS→AP del BTIF:** el loopback (AP→AP interno) funciona, pero recibir del CONSYS
   externo puede requerir más que limpiar TRI_LOOP_EN. Revisar en downstream `mtk_btif.c` el setup
   del **rx_irq_handler / la VFF + el new-handshake** para RX externo (¿crédito/flow-control/RTOCNT
   que el loopback no necesita?). Confirmar si el CONSYS realmente intenta TX (¿mide algún testpoint?).
- **type a usar:** type=0 hace que el MCU procese (mejor punto de partida que type=4). Mantener type=0
  hasta resolver CONSYS→AP; luego revisar si type=4 con el canal "abierto" es lo correcto.

### ❌ Watchdog DESCARTADO (2026-06-19) + ▶ PLAN B (capturar la secuencia real de Android)
**Cazado el watchdog: NO es un watchdog.** `CONSYS_WD_SYS_RST_REG` (0x10000018 bit9, key 0x88<<24)
existe pero el downstream NO lo usa. Y el `exp_main` **lo dispara nuestro comando**, no un temporizador:
con **type=4 el MCU NUNCA va a 0x23d8** (se queda idle 0xe38 todo el rato); con **type=0 procesa y
SÍ excepciona a ~2s**. → el `exp_main` es el firmware *assertando* al recibir un comando por el canal
**BT(0) equivocado**. El bloqueo real: **type=4 (canal WMT correcto) no engancha al MCU** → falta el
**handshake que "abre" el canal WMT**, que es FIRMWARE (no está en el source del kernel).

**→ PLAN B = capturar qué hace Android de verdad** (la única vía a ese handshake):
- **Mecanismo de Android**: userspace `wmt_loader` + `6620_launcher` (en stock `/system/bin/`, el
  daemon WMT genérico MTK) abren `/dev/stpwmt` → ioctl → el **kernel** (wmt_lib/wmt_ic_soc) hace el
  handshake STP/BTIF real. **`stp_dbg_pkt_log` registra CADA paquete TX/RX** con dirección.
- **Cómo capturar**: arrancar el **stock Android** (flashear `boot.img` + `system.img` de
  `~/Downloads/1.5.2_krillin`, en la Pi `~/wifi-fw/`; **reversible**: el rootfs pmOS está en p7,
  intacto; restaurar = flashear boot-color1.img). Activar el debug STP (proc/`wmt_dbg` o build eng) y
  leer `dmesg` con el log de paquetes = **la secuencia EXACTA + las respuestas del CONSYS**.
- **Refinamiento (idea del usuario)**: montar `system.img` rw en la Pi y meter un servicio init que en
  el **primer boot** active stp_dbg + vuelque el dmesg a una partición leíble → sin interacción manual.
  (system.img montado en Pi `/mnt/stocksys`; binarios en `/mnt/stocksys/bin/{wmt_loader,6620_launcher}`.)
- **Mutualizar**: el patrón BTIF/STP/WMT que saquemos sirve de plantilla para el resto de coprocesadores
  con firmware (el otro grande del MT6582 es el módem/MD, que es Halium-only).
- Pendiente menor: el ramdisk del boot.img stock está en formato MTK comprimido (no gzip+cpio directo;
  desempaquetar con el header MTK / lz4 si hace falta el init.rc exacto).


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

### ✅ Fase STP+WMT: framing PERFECTO, TX al FIFO OK — pero el enlace físico al CONSYS NO clockea
`boot-btifSTP.img`: el paquete STP se arma EXACTO →
`STP TX 11 bytes (hdr 87 00 05 8c, crc f1ec): 87 00 05 8c 01 04 01 00 02 ec f1`
(seq=0, ack=7, type=0/WMT, len=5, payload QUERY_BAUD `{01 04 01 00 02}`, CRC16 `0xf1ec` LE). El
header (`h0=0x80|seq<<3|ack`, `h1=type<<4|((len>>8)&0xf)`, `h2=len&0xff`, `h3=sum`), la `crc16_table`
y el `crc=(crc>>8)^tbl[(crc^b)&0xff]` están **validados** contra `stp_core.c`.
- **PERO TX timeout**: `tras resync LSR=0x20` (THRE=1 **TEMT=0**) → el shift register **no se vacía**:
  el BTIF mete bytes al FIFO pero **no los clockea** al CONSYS. Probado SIN WAK y SIN HANDSHAKE → igual.
- **CAUSA RAÍZ: el lado CONSYS del enlace no está activo.** El M1 (`mt6582-consys.c`) sólo hizo el
  **power del SPM** (`SPM_CONN_PWR_CON`: PWR_ON/RST_B/des-ISO/SRAM + TOPAXI de-protect) → basta para
  leer chip-id, pero NO arranca el subsistema interno. Faltan 3 pasos de la secuencia downstream
  `mtk_wcn_consys_hw_reg_ctrl` (estaban en `#if 0`, "los hace el FW patch" — pero sin patch hay que
  hacerlos a mano):

### ✅✅ ACTIVACIÓN DEL CONSYS FUNCIONA — EL ENLACE FÍSICO AP↔CONSYS TRANSMITE (`boot-btifACT.img`)
Implementado en `mt6582-consys.c` (`consys_activate_mcu`, tras el SPM power): **(1)** TOP_CLKCG_CLR
`writel(1<<26, ioremap(0x10000000)+0x84)`; **(2)** ungate CONNMCU `writel(1<<12, cs->infra+0x44)`
(INFRA_PDN_CLR, INFRACFG 0x10001000); **(3)** deassert MCU `writel((readl(0x10007018)&~(1<<12))|(0x88<<24),
…)`; msleep(30). Y `bool mt6582_consys_ready`(EXPORT) → el **btif hace EPROBE_DEFER** hasta que el
CONSYS está listo (resuelve el orden: btif 1.9s vs consys 3.5s). dmesg:
```
CONSYS MCU activado (TOP_CLKCG bit26 + CONNMCU + deassert reset)
CONSYS VIVO: chip-id=0x6582                  (la activación NO rompió nada)
probe of btif returned 517                   (DEFER, espera al consys) -> luego returned 0
STP: tras resync LSR=0x60                    ← ¡0x60! (antes 0x20) TEMT SE VACÍA = el shift clockea
STP TX 11 bytes (87 00 05 8c 01 04 01 00 02 ec f1)   x6 intentos, todos LSR=0x60
STP+WMT intento N: LSR=0x60 RX=0             (el paquete LLEGA al CONSYS pero NO contesta)
```
**→ MURO FÍSICO ROTO**: la activación del MCU hizo que el shift clockee (LSR 0x20→0x60); el
QUERY_BAUD se transmite entero y repetido al CONSYS. **Falta sólo que el CONSYS CONTESTE** (RX=0
tras 6 intentos + 30ms + reintentos). Es protocolo fino, no físico.

### ▶ SIGUIENTE = que el CONSYS CONTESTE (RX=0) — hipótesis ordenadas (investigar, no disparar)
1. **Habilitación/ruta del RX del BTIF**: confirmar que el RX del AP recibe (¿necesita el FIFO RX o
   un enable que el hw_init no pone?). Quizá falta `RX IER`/trigger aunque leamos por polling.
2. **WAK / handshake HW**: el CONSYS quizá no "ve" el frame hasta que el AP asierta `ap_wakeup_consys`
   (BTIF_WAK 0x64, pulso clr→delay→set) o se reactiva el **HANDSHAKE** (0x6C bit0) — ahora que el MCU
   corre, el handshake podría completarse (antes bloqueaba el TX porque el MCU estaba en reset).
3. **Delimiter STP**: `stp_core.c` envía un `stp_delimiter` (STP_DEL) antes del frame si
   `fgEnableDelimiter`. Revisar STP_DEL_SIZE/contenido y mandarlo.
4. **type/seq/ack o el ROM**: confirmar STP type=WMT(0); ¿el ROM del CONSYS escucha BTIF directo o
   necesita un trigger previo? Revisar `wmt_ic_soc.c mtk_wcn_soc_sw_init` (qué hace ANTES de QUERY_BAUD).
EVT esperado: `WMT_QUERY_BAUD_EVT_115200 = {02 04 06 00 00 02 00 C2 01 00}`. Cuando RX>0 coherente =
**M3a**. Luego WMT_RESET → GET chipid/ver → patch_dwn + WIFI_RAM_CODE = M3; IRQ (Fase E) al final.

### ✅✅✅✅ EL MCU RECIBE Y PROCESA NUESTROS COMANDOS WMT (2026-06-19 madrugada)
Instrumentando el PC del MCU (CONN_MCU_CONFIG 0x18070160) tras cada paso del btif:
- Con EMI+OSC el MCU corre en su **loop idle CPUPCR=0x0e38** esperando comandos (4s+ sin excepcionar).
- hw_init + loopback + resync: PC sigue en 0x0e38 (no le afectan).
- **al enviar el comando WMT el PC SALTA a 0x13e8c → 0x2eac → 0x13e7a** = el MCU EJECUTA el handler
  ~700ms. **RECIBE Y PROCESA lo que le mandamos.** Luego (k≈9) entra en exp_main (0x23d8)+coredump. RX=0.
- **Claves descubiertas**: (1) comando correcto p/BTIF = **WMT_QUERY_STP** `{01 04 01 00 04}` (el
  QUERY_BAUD `{..02}` es SOLO UART, `#if CFG_WMT_UART_HIF_USE`); (2) framing **BTIF_MAND_MODE** (header
  `80 00 LL 00`, sin seq/ack, **CRC=0000**), no FULLSET; (3) **HANDSHAKE (0x6C bit0) NECESARIO** para
  que el CONSYS reciba (sin él el PC no sale de 0x0e38 y no excepciona).
- **PENDIENTE (último tramo) = RX CONSYS→AP en handshake mode**: el MCU procesa y responde, pero el AP
  no captura la respuesta (RX=0) → el MCU excepciona. El RX downstream NO es PIO simple: handshake
  activo (IRQ BTIF + DMA/VFF + rx_irq_handler/rx_thread). Implementarlo = **M3a COMPLETO**. A UN paso:
  el canal AP→CONSYS funciona; falta CONSYS→AP.

### ✅✅✅ EL MCU DEL CONSYS EJECUTA — faltaban EMI + OSC 26M (2026-06-18 noche)
El M1 no hacía dos pasos que el downstream sí (`mtk_wcn_consys_hw_init`):
1. **EMI compartida**: reservar 1MB de RAM (`dma_alloc_coherent`) + decirle al CONSYS dónde está vía
   `CONSYS_EMI_MAPPING` (0x10001310 = `cs->infra+0x310`) = `(emi_phys>>20)|0x1000`. Sin su RAM el MCU
   no puede arrancar/trazar.
2. **Oscilador 26M**: `AP2CONN_OSC_EN` (0x10001f00 = `cs->infra+0xf00`) **bit10** estaba a **0** (26M del
   CONSYS apagado). Ponerlo a 1 tras el power.
Implementado en `mt6582-consys.c` (`consys_setup_emi` + OSC en `consys_activate_mcu`). **RESULTADO: el
MCU EJECUTA** — escribe un trace/coredump en la EMI que **REAPARECE tras `memset`** cada boot. En
`0xbb180400` (=gConEmiPhyBase+0x80400, PAGED_TRACE):
```
{asser_type=4}<2>exp_main: maybe jump from RST  EXP_MAIN_ENTRY_CNT=1
[DLMfull] dump_base=0xF0090400 ... PagedDumpSz=0xA5269 ... TotalTimeForDump
```
→ El MCU arranca, ejecuta y entra en su exception handler (`exp_main`, `asser_type=4 / jump from RST`),
hace un coredump y se queda ahí (CPUPCR=0x23d8 = exp_main). **De "el MCU no hace nada" a "ejecuta y deja
un coredump". La idea del system.img (→EMI) fue la clave.** Sigue RX=0 (en exp_main no atiende el BTIF).

### ▶▶▶ SIGUIENTE = evitar/entender el assert `jump from RST` (debugging del firmware CONSYS):
- Es del **firmware** del CONSYS (no del kernel-source). Hipótesis: (a) **watchdog del CONN_MCU** que
  resetea el MCU si el AP no completa un handshake/kick → localizar y deshabilitar/patear; (b) el ROM
  **espera el patch** y al no recibirlo excepciona (pero el patch va por BTIF y en exp_main no responde
  al QUERY_BAUD); (c) faltan pasos de init del MCU (`#if 0`: ROM_RAM_DELSEL 0x18070114, MCU_CFG_ACR
  0x18070110 bit18 MBIST, AFE). ACR leído=0x00300002 (MBIST off), DELSEL=0xaaa0aaab.
- **Recurso**: kernel Huawei `h30t00` (MT6582) en GitHub tiene `mtk_wcn_consys_hw.c` completo (con #if
  resueltos) — comparar el flujo. Leer el coredump completo (regiones DLM/B2/B3 en la EMI) para el PC/LR.

### ⛔ (SUPERADO la misma noche) Diagnóstico inicial: parecía que el MCU NO ejecutaba
Probado en HW (`boot-btifACT`): reintentos(6) + HANDSHAKE on + **loopback** (`RX del AP FUNCIONA`:
envié `aa bb cc dd`, recibí `aa bb cc dd` → el RX del AP es perfecto) + **WAK** (pulso clr→set). Todo RX=0.
Leído por `devmem` en el teléfono vivo:
- `CONSYS_CPUPCR (0x18070160) = 0x23d8` **FIJO** (5+ lecturas, 1s entre ellas) → **el MCU NO avanza**.
- `CONSYS_CPU_SW_RST (0x10007018) = 0x0` → reset bit12 **deasserted** ✓.
- `INFRA_PDN_STA (0x10001048) = 0x0` → clock CONNMCU (bit12) **ON** ✓.
- chip-id `0x6582` legible; cfg `0x18070000=0x8a01`, `0x18070004=0x8a00`.

→ El MCU tiene **clock + reset OK pero está clavado en 0x23d8**. El enlace BTIF funciona en AMBOS
sentidos (TX llega, RX del AP recibe en loopback); el problema NO es el BTIF sino que **el ROM del MCU
no arranca a ejecutar**. El WAK no lo despierta.

### ▶▶ SIGUIENTE = arrancar el MCU del CONSYS (bring-up, investigar — NO disparar):
1. **Pasos `#if 0` del downstream que faltan** (añadir a `consys_activate_mcu` en orden): `ROM_RAM_DELSEL`
   (0x18070114), `MCU_CFG_ACR` (0x18070110) bit18 MBIST, **AFE regs** (CONSYS_AFE_*), `PWRON_CONFG_EN`.
   Alguno puede ser prerequisito de que el ROM ejecute.
2. **Orden/secuencia de reset**: el downstream hace assert reset → clocks/power → ... → deassert. Probar
   un pulso de reset limpio: assert CPU_SW_RST (bit12=1 + key 0x88) ANTES de los clocks, luego deassert.
3. **Entender 0x23d8**: ¿vector de reset (MCU nunca arrancó) o bucle de espera del ROM? Buscar en
   downstream/firmware el dump del ROM o el CPUPCR esperado tras un power-on correcto.
4. Si el ROM espera el patch en SRAM antes de ejecutar: cargar `mt6572_82_patch` en la SRAM del CONSYS
   antes del deassert (camino más complejo, ya es parte de M3).

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
