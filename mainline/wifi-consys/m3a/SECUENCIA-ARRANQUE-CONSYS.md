# Secuencia de arranque del CONSYS MT6582 — síntesis de la investigación multi-agente (2026-06-19)

> Documento maestro para retomar el WiFi M3a. Resultado de 4 agentes que analizaron en paralelo:
> nuestro código + el downstream MTK (Pi `~/mainline/downstream`) + drivers de otros kernels Android.
> Estado de partida: **el MCU del CONSYS ejecuta, recibe y procesa nuestros comandos WMT** (el PC
> salta de su loop idle 0x0e38 a 0x13e8c y ejecuta ~700ms), pero **excepciona al responder** y el AP
> no capta la respuesta (RX=0). Ver [[HITO-WIFI-M3A.md]] para el recorrido completo.

## ⚡ TL;DR — plan de acción para mañana (en orden de probar)

1. **★★★ FIX DE 1 BYTE — el canal del header STP está mal.** En `BTIF_MAND_MODE`, el header es
   `h[0]=0x80`, **`h[1]=(type<<4)|((len>>8)&0x0f)`**, `h[2]=len&0xff`, `h[3]=0x00`, + CRC `00 00`.
   El **type del canal WMT = `WMT_TASK_INDX = 4`** (no 0). Para `len=5`, el header correcto es
   **`80 40 05 00`**, NO `80 00 05 00`. Nuestro `h[1]=0x00` etiqueta el frame como **canal 0 = BT**,
   así que el MCU lo parsea/enruta por el canal equivocado → responde mal y excepciona. En
   `mt6582_btif_stp_send`: cambiar `pkt[1] = (type << 4) | ...` y llamar con `type = 4` (o dejar el
   shift y pasar STP_TYPE_WMT=4). **Esto es lo primero, es trivial, y explica perfectamente "procesa
   pero excepciona al responder".**

2. **★★ El primer comando debería ser un register-read de chip-id, no QUERY_STP.** El downstream, en
   `wmt_core_hw_check` (ANTES de `sw_init`), envía como PRIMERA transacción BTIF un STP register-read de
   **GEN_HCR = 0x80000008** (20 bytes de payload): `01 08 10 00 | 02 01 00 01 | 08 00 00 80 | 00 00 00
   00 | FF FF 00 00` (op=2 lectura, addr LE=0x80000008, mask=0x0000FFFF), y espera el `RD_EVT` de 16
   bytes (`02 08 04 00 …`). `QUERY_STP {01 04 01 00 04}` es de `sw_init`, una fase POSTERIOR. Para el
   primer "ping" usar el register-read (con el header `80 40 14 00 …`).

3. **★★ El RX real es DMA/VFF, no PIO** (último tramo grande). Ver §3. El PIO simple (poll `LSR.DR`)
   NO capta la respuesta del CONSYS en modo handshake; el RX FIFO PIO es de solo **8 bytes** y el dato
   va por el path DMA. Si tras (1)+(2) el MCU deja de excepcionar pero RX sigue 0 → implementar RX-DMA.

4. **★ Mientras tanto, si se prueba PIO**: drenar por **IIR (0x08) bit2** con lecturas de **BYTE**
   (`readb`) de RBR (0x00), NO por `LSR.DR` (0x14 bit0). Y asegurar `BTIF_DMA_EN.RX (0x4C bit0)=0`.

## 1. La REFERENCIA correcta: `conn_soc` (CONSYS integrado), NO Huawei

El driver de Huawei h30t00 es para un **combo externo (MT6620/6628)** por UART/SDIO → tiene el reset
del MCU en `#if 0` porque no lo necesita (igual que nosotros lo dejamos). **No sirve de referencia.**
La buena es `drivers/misc/mediatek/connectivity/conn_soc/common/mt6752|mt6580/mtk_wcn_consys_hw.c`
(MT6752/6580 = gemelos del 6582), que SÍ ejecuta fuera de `#if 0`:

Secuencia ACTIVA de power-on del CONSYS integrado (orden exacto):
1. PMIC VCN18/VCN28 on + `udelay(150)`
2. **ASSERT `CPU_SW_RST`**: `AP_RGU(0x10007000)+0x18 |= (1<<12) | (0x88<<24)` (key)
3. `TOP_CLKCG_CLR`: `TOPCKGEN(0x10000000)+0x84 = (1<<26)`
4. **`SPM PWRON_CONFG_EN`**: `SPM(0x10006000)+0x00 = 0x0b160001` ← **ya lo hacemos** (es nuestro SPM_KEY)
5. `conn_power_on()` (MTCMOS) + `udelay(10)`
6. `enable_clock(MT_CG_INFRA_CONNMCU)` (INFRA bit) ← ya lo hacemos
7. poll chip-id == 0x6582
8. **`MCU_CFG_ACR` MBIST**: `CONN_MCU_CONFIG(0x18070000)+0x110 |= (1<<18)` ← **NO lo hacemos** (leído 0x00300002, bit18=0)
9. (AFE regs: siguen en `#if 0` — los hace el FW patch, NO hace falta)
10. **DEASSERT `CPU_SW_RST`**: `0x10007018 = (val & ~(1<<12)) | (0x88<<24)` + `msleep(5)`

**Lo que YA hacemos bien**: PMIC VCN, SPM power (incl. 0x0b160001), MTCMOS (PWR_RST_B bit0 = soltar el
reset del *dominio*), TOP_CLKCG bit26, CONNMCU clock, EMI_MAPPING (0x10001310 = (phys>>20)|0x1000),
AP2CONN_OSC_EN bit10 (0x10001f00), clock PERI_BTIF.

**Lo que NO hacemos del flujo integrado**: el **MBIST** (paso 8) y la **secuencia assert→…→deassert
del CPU_SW_RST con la key** (pasos 2 y 10). OJO: cuando probamos SOLO el deassert (sin el assert
previo), el MCU se clavaba en 0x23d8. La diferencia es que el downstream hace **assert ANTES** (con
clocks/MBIST en medio) y **deassert al final** — un pulso de reset limpio, no un deassert suelto.
**PERO**: actualmente el MCU YA ejecuta (0x0e38) sin tocar el reset, así que esto es secundario a (1).
El "jump from RST" probablemente NO es por el reset del MCU sino por el **canal STP equivocado** (1):
el MCU recibe un frame etiquetado como BT, su handler de BT salta a una ruta inválida → "jump from RST".

## 2. Framing STP en MAND mode (header correcto) + el flujo de envío/espera

- **Header (4B)** en `BTIF_MAND_MODE` (`stp_core.c:896`): `h[0]=0x80`; **`h[1]=(type<<4)|((len>>8)&0x0f)`**
  con **type=4 (WMT)**; `h[2]=len&0xff`; `h[3]=0x00` (sin checksum). Luego payload, luego **CRC `00 00`**
  (no se calcula en MAND). → para QUERY_STP(len=5): `80 40 05 00 01 04 01 00 04 00 00`.
- **Prerequisitos de estado** (lado AP, `wmt_ctrl_tx_ex`): STP abierto + STP enable + `flush_rx_queue`
  antes del TX. Modo STP = MAND al inicio; cambia a FULL **después** del patch (con `osal_sleep_ms(10)`).
- **Espera del EVT** (`wmt_ctrl_rx`): `wait_event_interruptible_timeout(..., 2000ms)`, despertado por el
  callback de RX cuando el parser STP mete un frame en el ring del canal. **Timeout = 2 segundos.**
- **El CRC del EVT NO se valida en MAND** (el parser consume los 2 bytes sin comparar). Solo exige:
  inicio `0x80`, nibble de tipo ≤ max_task, length < 2000.

## 3. El RX (CONSYS→AP) — por qué RX=0 y la receta DMA/VFF

**El RX downstream es DMA/VFF SIEMPRE** (`ENABLE_BTIF_RX_DMA` definido; el PIO de RX está tras
`MTK_BTIF_MARK_UNUSED_API`, que está definido → ni se compila). El RX FIFO PIO es de **8 bytes**.
- **Detección de datos = IRQ GIC SPI 72** (canal RX-DMA), NO polling de LSR. "Cuánto/dónde" =
  `RX_DMA_VFF_VALID_SIZE (0x1100083C)` + `WPT(HW, 0x...82C) − RPT(SW, 0x...830)`. Backup: thread RT cada 360ms.
- **HANDSHAKE (0x6C bit0)** = entrega sincronizada BTIF↔APDMA (análogo al `dma_sync=1` del i2c
  APDMA, ver [[reference_mt6582_apdma_i2c]]). Pasivo desde el AP (set-once) pero **obliga al path DMA**.
  Por eso con HANDSHAKE=1 el PIO no capta (el dato va al vFIFO, no al RBR) y con HANDSHAKE=0 el CONSYS
  no recibe (el firmware asume el new-handshake).
- **Receta RX-DMA mínima** (canal RX en `0x11000800`, IRQ SPI 72):
  1. `dma_alloc_coherent(dev, 8192, &phys, GFP_KERNEL)` (ring 8KB, align 8). Coherente → sin caché rancia.
  2. Programar: RST (`+0x0C` bit1), `VFF_ADDR(+0x1C)=phys`, `VFF_LEN(+0x24)=0x2000`, `VFF_WPT(+0x2C)=0`,
     `VFF_THRE(+0x28)=0x1800` (6144), `INT_FLAG(+0x00) |= 0x3` (W1C), `INT_EN(+0x04)=0x3`.
  3. Arrancar: `RX_DMA_EN(0x11000808) |= 1`; `BTIF_DMA_EN(0x1100C04C) |= bit0 (RX→DMA)`; mantener
     `HANDSHAKE(0x6C bit0)=1` y `AUTORST(0x4C bit2)=1`.
  4. ISR (SPI 72, level-low): mask INT → W1C `INT_FLAG|=0x3` → leer VALID_SIZE/WPT/RPT → copiar `[RPT,WPT)`
     del ring (con wrap) → **escribir RPT de vuelta (el crédito)** → repetir hasta vaciar → unmask.
  5. Para una primera prueba SIN IRQ: **pollear** `RX_DMA_VFF_VALID_SIZE (0x1100083C)` y leer del ring +
     escribir RPT (evita el wiring de la IRQ, pero sigue necesitando el ring DMA — NO sirve leer RBR/LSR).
- DT: el nodo APDMA-BTIF-RX necesita `interrupts = <GIC_SPI 72 IRQ_TYPE_LEVEL_LOW>`. El clock AP_DMA es
  dummy 13MHz (el LK lo deja on, igual que el i2c).

## 4. Orden de prueba recomendado (mañana)

1. **(1) header canal WMT** (`h[1]=0x40|...`) — solo cambiar el byte. Re-medir el PC del MCU: ¿deja de
   excepcionar (no entra en 0x23d8) y/o aparece algo en el RX?  ← lo más probable que desbloquee.
2. Si el MCU deja de excepcionar pero RX=0 → **(3) implementar RX-DMA/VFF** (la "Fase E").
3. Si sigue excepcionando → probar **(2) el register-read de GEN_HCR** como primer comando, y **MBIST**
   (`0x18070110 |= (1<<18)`) en `consys_activate_mcu`.
4. Opcional, si nada: la secuencia de reset limpia (assert key → clocks/MBIST → deassert key).

## Apéndice — registros clave (todos confirmados en fuente)
- CONSYS base 0x18070000: chip-id +0x08, **MCU_CFG_ACR +0x110 (MBIST bit18)**, ROM_RAM_DELSEL +0x114, **CPUPCR +0x160** (PC del MCU; idle=0x0e38, handler=0x13e8c, exp_main=0x23d8).
- AP_RGU 0x10007000: **CPU_SW_RST +0x18** (bit12, key 0x88<<24).
- TOPCKGEN 0x10000000: TOP_CLKCG_CLR +0x84 (bit26), **AP2CONN_OSC_EN +0x1f00 (bit10)**, EMI_MAPPING +0x1310 (=+0x310 desde INFRACFG 0x10001000).
- SPM 0x10006000: PWRON_CONFG_EN/POWERON_CFG +0x00 (=0x0b160001), CONN_PWR_CON +0x280, PWR_STATUS +0x60c/+0x610.
- BTIF 0x1100C000: RBR/THR 0x00, IER 0x04, **IIR 0x08 (RX=bit2 0x04, RX_TIMEOUT 0x44)**, LSR 0x14, **DMA_EN 0x4C (RX bit0, TX bit1, AUTORST bit2)**, RTOCNT 0x54, TRI_LVL 0x60, WAK 0x64, **HANDSHAKE 0x6C (bit0)**.
- APDMA BTIF-RX 0x11000800: INT_FLAG +0x00, INT_EN +0x04, EN +0x08, RST +0x0C, VFF_ADDR +0x1C, VFF_LEN +0x24, WPT +0x2C, RPT +0x30, THRE +0x28, VALID_SIZE +0x3C. IRQ = GIC SPI 72. (TX-DMA = 0x11000780, IRQ SPI 71.)
- STP: `WMT_TASK_INDX=4`. EVT QUERY_STP = `02 04 06 00 00 04 11 00 00 00`. RD_EVT chip-id = `02 08 04 00 …` (16B). Timeout RX = 2000ms.
