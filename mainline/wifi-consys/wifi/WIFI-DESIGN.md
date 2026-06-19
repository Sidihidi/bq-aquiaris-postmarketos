# WIFI-DESIGN.md — Driver mainline WiFi para MT6582 CONSYS (krillin)

Diseño del driver cfg80211 full-MAC para el WiFi del combo CONSYS del MT6582, partiendo del
downstream `downstream-wlan/wlan/` y de nuestra base que ya funciona (`our-drivers/mt6582-btif.c`,
`mt6582-consys.c`).

---

## 0. LA DECISIÓN ARQUITECTÓNICA (la pregunta clave del encargo)

> **¿Los datos/comandos WiFi de alta tasa van por el MISMO enlace BTIF/STP que usamos para BT y GPS,
> o por un mecanismo distinto?**

### Respuesta: **POR UN MECANISMO DISTINTO. El WiFi NO usa BTIF/STP para nada de su tráfico.**

El WiFi del MT6582 tiene su **propio bloque HIF mapeado en memoria sobre el bus AHB del SoC**, con un
juego de registros estilo SDIO ("MCR" = MAC Control Registers) y un **canal PDMA dedicado** para mover
los datos. Es un periférico completamente separado del BTIF.

**Evidencia dura del downstream:**

| Qué | Valor | Fichero:línea |
|---|---|---|
| Base de registros HIF del WiFi (AHB slave) | `HIF_DRV_BASE = 0x180F0000`, len `0x5c` | `os/linux/hif/ahb/include/hif.h:201-202` |
| Se mapea con `ioremap()` directo | `HifInfo->HifRegBaseAddr = ioremap(HIF_DRV_BASE, HIF_DRV_LENGTH)` | `os/linux/hif/ahb/ahb.c:696` |
| Canal PDMA dedicado al HIF del WiFi | `AP_DMA_HIF_BASE = 0x11000180` (no-OF), len `0x54` | `os/linux/hif/ahb/include/hif_pdma.h:62-65,81` |
| El PDMA del WiFi se mapea aparte | `HifInfo->DmaRegBaseAddr = ioremap(AP_DMA_HIF_BASE, AP_DMA_HIF_0_LENGTH)` | `os/linux/hif/ahb/mt6582/ahb_pdma.c:219` |
| El nombre del módulo es "AHB slave HIF" | `#define HIF_MOD_NAME "AHB_SLAVE_HIF"` | `os/linux/hif/ahb/include/hif.h:199` |
| El driver de plataforma se llama `mt-wifi`, compatible `mediatek,WIFI`, IRQ propia | `MtkPltmAhbDriver` | `os/linux/hif/ahb/ahb.c:376-399` |
| `ahb.c` es literalmente `sdio.c` portado a AHB | *"Port sdio.c to ahb.c on MT6572/MT6582"* | `os/linux/hif/ahb/ahb.c:17-19` (changelog) |

**Comparación con NUESTRO BTIF** (`our-drivers/mt6582-btif.c`):

| | BTIF (BT/GPS, lo que ya tenemos) | HIF WiFi (lo nuevo) |
|---|---|---|
| Base de registros | `BTIF_PHYS = 0x1100C000` | `HIF_DRV_BASE = 0x180F0000` |
| Canal(es) DMA | APDMA TX `0x11000780` / RX `0x11000800` (VFF) | PDMA HIF `0x11000180` (1 canal bidi config) |
| Protocolo de transporte | STP framing (`80 (type<<4|len_hi) len_lo 00` + payload + `00 00`) | Registros MCR + puertos de datos `WTDR0/WRDR0` |
| Multiplexado | canales STP: BT=0 FM=1 GPS=2 WIFI=3 WMT=4 | sin multiplexar: es el bloque WiFi entero |
| IRQ | (lo hacemos por polling de VFF_WPT) | `WF_HIF_IRQ_ID`, status en `WHISR` |

**Conclusión operativa:** el canal STP=3 ("WIFI") del WMT **NO transporta datos WiFi**. El subsistema
WMT/STP del CONSYS sólo se usa para el **bring-up del chip** (descargar el patch del MCU, encender la
radio con `func_on`) y para **coordinar el reset** (el driver WiFi se registra en WMT sólo como callback
de reset: `mtk_wcn_wmt_msgcb_reg(WMTDRV_TYPE_WIFI=3, glResetCallback)`, `os/linux/gl_rst.c:210`).
Una vez la radio WiFi está encendida, **todo el tráfico cmd/event/data va por el HIF en `0x180F0000`**.

### Qué implica esto para nuestro diseño

1. **El "canal STP 3" del encargo es un espejismo para los datos.** Lo seguimos necesitando para una
   sola cosa: pedirle al WMT que **encienda la radio WiFi** (`func_on(WIFI)`), exactamente como el btif
   ya hace `func_on(BT)` y `func_on(GPS)`. A partir de ahí, el driver WiFi habla directo con `0x180F0000`.

2. **El firmware `WIFI_RAM_CODE_MT6582` NO se descarga por STP.** El encargo asume que "ya lo cargamos en
   el bring-up", pero eso es incorrecto para este firmware. El **patch del MCU**
   (`mt6572_82_patch_e1_x`) sí va por STP/WMT y eso ya lo hace el btif. Pero `WIFI_RAM_CODE` es el
   **firmware del MAC del WiFi** y se descarga por el **puerto de datos del HIF** como una secuencia de
   comandos `INIT_CMD_ID_DOWNLOAD_BUF` (ver §3). Esto lo hace el driver WiFi, no el btif. (Confirmado:
   `wlanImageSectionDownload()` arma un `INIT_HIF_TX_HEADER_T` y lo manda con `nicTxInitCmd` por el
   puerto HIF — `common/wlan_lib.c:3140-3236`. No hay ni una llamada a STP en esa ruta.)

3. **Nuestro driver WiFi es un `platform_driver` independiente** que necesita:
   - una **dependencia de orden** sobre el bring-up del CONSYS (igual que el btif espera
     `mt6582_consys_ready`),
   - una **forma de pedir `func_on(WIFI)` al btif** (exportar una función del btif; ver §6),
   - y luego es **autónomo**: `ioremap(0x180F0000)` + `ioremap(0x11000180)` + `request_irq(WF_HIF_IRQ)`
     y maneja su propia máquina HIF.

> **Esto es buenísimo para nosotros**: el data-path NO depende de nuestra capa STP frágil (polling de
> VFF, sin IRQ). El WiFi tiene DMA e IRQ hardware propios y un protocolo de puertos limpio. El riesgo se
> concentra en (a) que `func_on(WIFI)` deje el HIF accesible y (b) clavar la secuencia de FW-own +
> descarga de firmware + WIFI_START.

---

## 1. El modelo del hardware HIF (registros MCR @ 0x180F0000)

El bloque HIF expone los mismos registros que una tarjeta SDIO MT6620, pero pinchados al AHB. Mapa
(`include/nic/mtreg.h:84-146`), todos offsets de 32 bits desde `0x180F0000`:

```
0x00  WCIR    Chip ID + ready.   bit21 WLAN_READY, bit20 POR_INDICATOR, bits16-19 rev, bits0-15 chipid(=0x6582)
0x04  WHLPCR  Low-power / own.   bit0 INT_EN_SET, bit1 INT_EN_CLR, bit8 FW_OWN_REQ_SET(/IS_DRIVER_OWN al leer), bit9 FW_OWN_REQ_CLR
0x08  WSDIOCSR  (SDIO re-init; no usado en AHB)
0x0C  WHCR    HIF control.       bit16 RX_ENHANCE_MODE_EN, bits4-7 MAX_HIF_RX_LEN_NUM, bit2 MAILBOX_RD_CLR, bit1 INT_CLR_CTRL
0x10  WHISR   Int status (R).    bit0 TX_DONE, bit1 RX0_DONE, bit2 RX1_DONE, bit3 ABNORMAL, bit4 FW_OWN_BACK, bits8-31 D2H_SW_INT
0x14  WHIER   Int enable.        WHIER_DEFAULT = RX0|RX1|TX_DONE|ABNORMAL|D2H_SW_INT
0x18  WASR    Abnormal status.   over/under-flow de las colas TX/RX
0x1C  WSICR   SW int a device.   bits16-31 H2D_SW_INT_SET
0x20  WTSR0 / 0x24 WTSR1   TX status (cuántos buffers TX libres por TC)
0x28  WTDR0   *** PUERTO DE DATOS TX 0 *** (aquí se escriben cmd y data)
0x2C  WTDR1   *** PUERTO DE DATOS TX 1 ***
0x30  WRDR0   *** PUERTO DE DATOS RX 0 *** (de aquí se leen event/data)
0x34  WRDR1   *** PUERTO DE DATOS RX 1 ***
0x38  H2DSM0R / 0x3C H2DSM1R   Host->Device mailbox
0x40  D2HRM0R / 0x44 D2HRM1R / 0x48 D2HRM2R   Device->Host mailbox (error codes, etc.)
0x50  WRPLR   RX packet length.  bits0-15 RX0_LEN, bits16-31 RX1_LEN
0x58  HSTCR   HSIF transaction count (para el modo DMA)
```

### Modelo de propiedad ("FW own" / "Driver own")
El chip arranca en **FW-own** (CONSYS dormido respecto al WiFi). Para tocar los registros de datos hay
que estar en **driver-own**:
- **Pedir driver-own:** `writel(WHLPCR_FW_OWN_REQ_CLR /*bit9*/, base+WHLPCR)`, luego *poll* hasta que
  `readl(base+WHLPCR) & WHLPCR_IS_DRIVER_OWN /*bit8*/` sea 1.
- **Devolver FW-own (para dormir):** `writel(WHLPCR_FW_OWN_REQ_SET /*bit8*/, base+WHLPCR)`.
- `nicpmSetDriverOwn()` / `nicpmSetFWOwn()` en `nic/nic_pwr_mgt.c` hacen esto.

### Interrupción
`WF_HIF_IRQ_ID` (level-low) → ISR lee `WHISR` (lectura limpia el flag si `WHCR.INT_CLR_CTRL`), mira
RX0/RX1_DONE, lee `WRPLR` para la longitud, y hace `kalDevPortRead(WRDR0, len)`. En enhance-mode
(`WHCR.RX_ENHANCE_MODE_EN`) una sola lectura del puerto trae una `ENHANCE_MODE_DATA_STRUCT_T`
(`mtreg.h:149-174`) con WHISR + contadores TX + longitudes RX + mailboxes de golpe.

**Para Phase 0/1 lo haremos por POLLING** (más simple, igual que hicimos en el btif): poll de `WHISR`.
La IRQ se añade en Phase 3 cuando importe el rendimiento.

---

## 2. El data-path real: puertos WTDR0/WRDR0 con PDMA

### TX (host → chip)
`kalDevPortWrite(GlueInfo, Port=WTDR0, Size, Buf)` — `os/linux/hif/ahb/ahb.c:1487+`:
- En **PIO**: bucle `writel(*buf++, base+WTDR0)` de `Size/4` palabras.
- En **DMA** (default, `fgDmaEnable=TRUE`): configura el canal PDMA `0x11000180` con
  `Src = dma_map_single(Buf)`, `Dst = HIF_DRV_BASE + WTDR0` (¡dirección física del puerto!),
  `Count = Size`, dir TX; `DmaStart`; *poll* `AP_DMA_HIF_0_INT_FLAG`; `DmaAckIntr`; `DmaStop`;
  `dma_unmap_single`. Ver `HifPdmaConfig`/`HifPdmaStart` en `ahb_pdma.c:261-332`.

### RX (chip → host)
`kalDevPortRead(GlueInfo, Port=WRDR0, Size, Buf)` — `os/linux/hif/ahb/ahb.c:1197+`:
- Idéntico pero dir RX: `Src = HIF_DRV_BASE + WRDR0`, `Dst = dma_map_single(Buf, DMA_FROM_DEVICE)`.

### Registros del canal PDMA (`hif_pdma.h:68-113`), offsets desde `0x11000180`:
```
0x00 INT_FLAG   bit0 FLAG_0 (RW1C-ish: se limpia escribiendo 0)
0x04 INT_EN     bit0 INTEN_FLAG_0
0x08 EN         bit0 EN (arranca; queda 1 mientras transfiere)
0x0C RST        bit0 WARM_RST, bit1 HARD_RST
0x10 STOP       bit0 STOP, bit1 PAUSE
0x14 FLUSH      bit0
0x18 CON        bits16-17 BURST_LEN (usar HIF_PDMA_BURST_4_4=3), bit1 FIX_EN, bit0 DIR (0=TX,1=RX)
0x1C SRC_ADDR   (físico)
0x20 DST_ADDR   (físico)
0x24 LEN        bits0-19 (cuenta de bytes)
0x38 INT_BUF_SIZE
0x50 DEBUG_STATUS
```
> Ojo: este es **otro** canal del mismo AP-DMA del que cuelgan los del btif. El btif usa
> `0x11000780/0x11000800`; el WiFi usa `0x11000180`. **No colisionan.** El clock del AP-DMA
> (`MT_CG_PERI_AP_DMA`) ya debería estar on (el btif lo usa). En mainline lo pediremos por `clk` o lo
> dejaremos on como hace consys.

> Para la EMI: `HifPdmaInit` llama `emi_mpu_set_region_protection(gConEmiPhyBase, +SZ_1M/2, ...)`
> (`ahb_pdma.c:230-239`). Es la **misma región EMI** que `mt6582-consys.c` ya reserva (1MB,
> `consys_setup_emi`). En mainline NO tenemos `emi_mpu` driver; lo más probable es que **no haga falta**
> tocar el MPU (LK/preloader ya abre la región, y nosotros corremos sin TEE). Se deja como TODO de
> verificación: si el PDMA da fault de EMI, habrá que abrir la región. La memoria de los buffers TX/RX
> del WiFi es **DRAM normal del AP** (`dma_alloc_coherent`/`dma_map_single`), no la EMI compartida.

---

## 3. Descarga del firmware `WIFI_RAM_CODE` (por el HIF, no por STP)

### Fichero y formato
- Nombre: `CFG_FW_FILENAME = "WIFI_RAM_CODE"` (`include/config.h:919-922`). El downstream además le pega
  el sufijo del chip: `glGetChipInfo()` para chip-id `0x6582`/`0x6572` devuelve **"SOC"**
  (`ahb.c:800-808`), así que el fichero efectivo es **`WIFI_RAM_CODE_SOC`** (o el plano `WIFI_RAM_CODE`
  como fallback, `gl_kal.c:858,873`). **→ En mainline lo pondremos en `/lib/firmware/` y lo cargaremos
  con `request_firmware("mediatek/mt6582/WIFI_RAM_CODE", dev)`** (nombre a fijar; lo importante es el
  contenido, que es el blob que ya tienes de 160KB).
- Cabecera (descarga "dividida en secciones"), `include/wlan_lib.h:568-583`:
```c
#define MTK_WIFI_SIGNATURE  BUILD_SIGN('M','T','K','W')   /* = 'MTKW' = 0x57 4B 54 4D LE */

typedef struct _FWDL_SECTION_INFO_T {     /* una entrada por sección */
    UINT_32 u4Offset;       /* offset del dato dentro del fichero */
    UINT_32 u4Reserved;
    UINT_32 u4Length;       /* bytes de la sección */
    UINT_32 u4DestAddr;     /* dirección destino en la RAM del chip */
} FWDL_SECTION_INFO_T;

typedef struct _FIRMWARE_DIVIDED_DOWNLOAD_T {
    UINT_32 u4Signature;    /* == MTK_WIFI_SIGNATURE */
    UINT_32 u4CRC;          /* CRC32 del resto del fichero (desde offset 8) */
    UINT_32 u4NumOfEntries; /* nº de secciones */
    UINT_32 u4Reserved;
    FWDL_SECTION_INFO_T arSection[ /*u4NumOfEntries*/ ];
} FIRMWARE_DIVIDED_DOWNLOAD_T;
```
- Si la firma no casa (o el firmware no trae cabecera), el downstream lo trata como **una sola sección**
  a una dirección por defecto. La lógica de validación está en `common/wlan_lib.c:1318-1396`. **Hay que
  inspeccionar el blob real**: leer los primeros 4 bytes; si son `4D 54 4B 57` → formato seccionado.

### Protocolo de descarga (comando INIT por el puerto HIF)
Cada chunk (≤ `CMD_PKT_SIZE_FOR_IMAGE = 2048`B, `hif_tx.h:102`) se envía como un comando init.
`wlanImageSectionDownload()` (`wlan_lib.c:3140-3236`) arma este buffer y lo escribe en `WTDR0`:

```c
/* nic_init_cmd_event.h:98-114 */
struct INIT_HIF_TX_HEADER_T {            /* 4 bytes de cabecera HIF */
    u16 u2TxByteCount;                   /* tamaño total del paquete */
    u8  ucEtherTypeOffset;               /* 0 */
    u8  ucCSflags;                       /* 0 */
    struct INIT_WIFI_CMD_T {             /* 4 bytes de cabecera de comando init */
        u8  ucCID;                       /* = INIT_CMD_ID_DOWNLOAD_BUF (1) */
        u8  ucSeqNum;                    /* secuencia incremental */
        u16 u2Reserved;
        u8  aucBuffer[];
    } rInitWifiCmd;
};
/* nic_init_cmd_event.h:108-114, va dentro de aucBuffer */
struct INIT_CMD_DOWNLOAD_BUF {
    u32 u4Address;      /* = arSection[i].u4DestAddr (+ offset del chunk) */
    u32 u4Length;       /* tamaño del chunk */
    u32 u4CRC32;        /* wlanCRC32(chunk) */
    u32 u4DataMode;     /* bit31 ACK_OPTION, bit30 NO_CRC, bit0 ENCRYPTION (lo dejamos a 0 salvo ACK) */
    u8  aucBuffer[];    /* los bytes del chunk */
};
```
Tras cada chunk (si `ACK_OPTION`), se espera un evento init de resultado
(`wlanImageSectionDownloadStatus`, lee `WRDR0`):
```c
/* nic_init_cmd_event.h:129-147 */
struct INIT_WIFI_EVENT_T { u16 u2RxByteCount; u8 ucEID; u8 ucSeqNum; u8 aucBuffer[]; };
struct INIT_EVENT_CMD_RESULT { u8 ucStatus; u8 aucReserved[3]; };  /* EID = INIT_EVENT_ID_CMD_RESULT(1) */
/* ucStatus: 0 OK, 1 invalid param, 2 CRC, 3 decrypt, 4 unknown */
```

### Arranque del firmware
Tras descargar todas las secciones: comando **`INIT_CMD_ID_WIFI_START` (2)** con payload
`INIT_CMD_WIFI_START { u32 u4Override; u32 u4Address; }` (`nic_init_cmd_event.h:116-119`). `u4Override=1`
+ `u4Address` = dirección de arranque (XIP) si se quiere forzar; normalmente `0`. Lo manda
`wlanConfigWifiFunc()` (`wlan_lib.c`). Después se *poll*ea `WCIR.WLAN_READY (bit21)` hasta 1
(`wlanAdapterStart`, `wlan_lib.c:~1587-1612`).

### Resumen de la secuencia de arranque (lo que replica `wlanAdapterStart`)
1. `func_on(WIFI)` vía WMT/btif → enciende la radio, el HIF en `0x180F0000` queda vivo.
2. `ioremap(HIF_DRV_BASE)` + `ioremap(AP_DMA_HIF_BASE)`.
3. Pedir **driver-own** (`WHLPCR` bit9, poll bit8).
4. Leer `WCIR`, comprobar chipid bits0-15 == `0x6582`.
5. `nicInitializeAdapter` (config de `WHCR`, colas TX, etc. — para nosotros: lo mínimo).
6. **Descargar `WIFI_RAM_CODE`** sección a sección con `INIT_CMD_ID_DOWNLOAD_BUF`.
7. `INIT_CMD_ID_WIFI_START`.
8. Poll `WCIR.WLAN_READY`.
9. `CMD_ID_BASIC_CONFIG` (fija MAC), `CMD_ID_GET_NIC_CAPABILITY` (lee capacidades/MAC permanente).
10. Listo: registrar `wiphy` + `netdev wlan0`.

---

## 4. Protocolo runtime cmd/event (sobre los mismos puertos WTDR0/WRDR0)

Una vez el firmware corre, el tráfico de control son comandos `WIFI_CMD_T` (TX) y eventos `WIFI_EVENT_T`
(RX), con cabecera HIF de 16/12 bytes delante. **Tipos de paquete** discriminan cmd vs data vs mgmt.

### Cabecera TX de datos: `HIF_TX_HEADER_T` (16B, `include/nic/hif_tx.h:158-171`)
```c
struct HIF_TX_HEADER_T {
    u16 u2TxByteCount_UserPriority;  /* DW0: bits0-11 byte count, bits12-15 UP */
    u8  ucEtherTypeOffset;           /* offset del ethertype en la trama */
    u8  ucResource_PktType_CSflags;  /* bit0 IP_CSUM, bit1 TCP_CSUM, bits2-5 RESOURCE(TC),
                                        bits6-7 PKT_TYPE (0=DATA,1=CMD,2=LOOPBACK,3=MGMT) */
    u8  ucWlanHeaderLength;          /* bits0-5 */
    u8  ucPktFormtId_Flags;          /* bits0-2 FORMAT_ID, bits4-5 NETWORK_TYPE, bit6 1X_FRAME, bit7 802.11 */
    u16 u2LLH; u16 u2SeqNo;          /* sólo BOW (BT-over-WiFi); 0 para nosotros */
    u8  ucStaRecIdx;                 /* índice del STA-record destino */
    u8  ucForwardingType_SessionID_Reserved;
    u8  ucPacketSeqNo;               /* para TX-DONE awareness */
    u8  ucAck_BIP_BasicRate;         /* bit0 NEED_ACK, bit1 BIP, bit2 BASIC_RATE */
    u8  aucReserved[2];
};   /* PKT_TYPE en bits6-7 del byte 3: DATA=0, CMD=1, MGMT=3 (ENUM_HIF_TX_PKT_TYPE_T, hif_tx.h:173-179) */
```

### Cabecera de comando: `WIFI_CMD_T` (8B, comparte los 4 primeros con la cabecera HIF)
(`include/nic_cmd_event.h`, ~línea 850)
```c
struct WIFI_CMD_T {
    u16 u2TxByteCount_UserPriority;  /* idem */
    u8  ucEtherTypeOffset;
    u8  ucResource_PktType_CSflags;  /* PKT_TYPE = CMD(1) en bits6-7 */
    u8  ucCID;                       /* Command ID (ver tabla) */
    u8  ucSetQuery;                  /* 1 = SET, 0 = QUERY */
    u8  ucSeqNum;                    /* secuencia, para casar el EVENT_ID_CMD_RESULT */
    u8  aucReserved2;
    u8  aucBuffer[];                 /* payload del comando */
};
```

### Cabecera RX: `HIF_RX_HEADER_T` (12B, `include/nic/hif_rx.h:131-141`) y `WIFI_EVENT_T` (8B)
```c
struct HIF_RX_HEADER_T {
    u16 u2PacketLen;                 /* longitud total */
    u16 u2PacketType;                /* bits0-1 = PKT_TYPE (DATA=0,EVENT=1,LOOPBACK=2,MGMT=3) */
    u8  ucHerderLenOffset;
    u8  uc80211_Reorder_PAL_TCL;     /* bit0 802.11 fmt, bit1 reorder, bits4-7 network idx */
    u16 u2SeqNoTid;                  /* bits0-11 SN, bits12-14 TID */
    u8  ucStaRecIdx;
    u8  ucRcpi;                      /* RSSI */
    u8  ucHwChannelNum;              /* canal (≤14 → 2.4G) */
    u8  ucReserved;
};
struct WIFI_EVENT_T {                /* cuando PKT_TYPE==EVENT, el payload empieza así */
    u16 u2PacketLen; u16 u2PacketType;
    u8  ucEID;                       /* Event ID */
    u8  ucSeqNum;                    /* casa con el ucSeqNum del comando */
    u8  aucReserved2[2];
    u8  aucBuffer[];
};
```

### Discriminación RX (clave para el RX handler)
Al leer un paquete de `WRDR0`, mirar `HIF_RX_HEADER_T.u2PacketType & 0x3`
(`HIF_RX_HDR_PACKET_TYPE_MASK`, `hif_rx.h:91`), enrutado en `nic/nic_rx.c` (`nicRxProcessRFBs`):
- `0 = DATA` → `nicRxProcessDataPacket` → quitar cabecera, `netif_rx(skb)`.
- `1 = EVENT` → `nicRxProcessEventPacket` → despachar por `ucEID`.
- `3 = MGMT` → `nicRxProcessMgmtPacket` → a cfg80211 (probe/auth/assoc/action).

### Opcodes (los que nos importan)
> **Importante:** estos valores vienen de `include/nic_cmd_event.h` (enums `ENUM_CMD_ID_T` /
> `ENUM_EVENT_ID_T`). Algunos miembros del enum son condicionales (`#if CFG_...`), así que los valores
> numéricos exactos **hay que confirmarlos** contra ese fichero al implementar. Aun así, la **forma de
> uso** (qué comando para qué) es la que vale:

| Operación | CMD_ID (CID) | Payload struct | Respuesta |
|---|---|---|---|
| Config básica / MAC | `CMD_ID_BASIC_CONFIG` | `CMD_BASIC_CONFIG {rMyMacAddr[6]; ucNative80211; rCsumOffload}` | `EVENT_ID_BASIC_CONFIG` |
| Capacidades NIC (lee MAC perm.) | `CMD_ID_GET_NIC_CAPABILITY` (query) | — | `EVENT_ID_NIC_CAPABILITY` |
| **Scan** | `CMD_ID_SCAN_REQ_V2` (o `_SCAN_REQ`) | `CMD_SCAN_REQ_V2 {ucSeqNum; ucNetworkType; ucScanType(0 act/1 pas); arSSID[4]; u2ChannelDwellTime; ucChannelListNum; arChannelList[32]; u2IELen; aucIE[]}` | `EVENT_ID_SCAN_DONE` |
| Cancelar scan | `CMD_ID_SCAN_CANCEL` | `CMD_SCAN_CANCEL {ucSeqNum; ...}` | `EVENT_ID_SCAN_DONE` |
| Modo infraestructura | `CMD_ID_INFRASTRUCTURE` | (set op-mode STA) | `EVENT_ID_CMD_RESULT` |
| **Set BSS (conectar)** | `CMD_ID_SET_BSS_INFO` | `CMD_SET_BSS_INFO {ucNetTypeIndex; ucConnectionState; ucCurrentOPMode; aucSSID[32]; aucBSSID[6]; ucAuthMode; ucEncStatus; aucOwnMac[6]; ...}` | (estado de conexión) |
| Activar/STA record | `CMD_ID_BSS_ACTIVATE_CTRL`, `CMD_ID_UPDATE_STA_RECORD` | structs STA | `EVENT_ID_ACTIVATE_STA_REC` |
| **Añadir/quitar clave** | `CMD_ID_ADD_REMOVE_KEY` | `CMD_802_11_KEY {ucAddRemove; ucKeyType; aucPeerAddr[6]; ucAlgorithmId; ucKeyId; ucKeyLen; aucKeyMaterial[32]; aucKeyRsc[16]}` | `EVENT_ID_CMD_RESULT` |
| Clave por defecto | `CMD_ID_DEFAULT_KEY_ID` | idx | `EVENT_ID_CMD_RESULT` |
| Estado link / RSSI | `CMD_ID_GET_LINK_QUALITY` (query) | — | `EVENT_ID_LINK_QUALITY` |
| Dominio regulatorio | `CMD_ID_SET_DOMAIN_INFO` | lista de canales | — |
| Power save | `CMD_ID_POWER_SAVE_MODE` | modo | — |

Eventos clave a manejar en RX: `EVENT_ID_CMD_RESULT(1)`, `EVENT_ID_NIC_CAPABILITY(2)`,
`EVENT_ID_SCAN_DONE`, `EVENT_ID_CONNECTION_STATUS`/`EVENT_ID_ASSOC_INFO`, `EVENT_ID_LINK_QUALITY`,
`EVENT_ID_TX_DONE`. (Para los resultados de scan, el firmware **no** manda cada BSS como evento; manda
las **tramas beacon/probe-resp como paquetes MGMT** que cfg80211 inyecta con
`cfg80211_inform_bss_frame`, y al final un `EVENT_ID_SCAN_DONE`.)

---

## 5. La capa cfg80211 full-MAC (qué ops implementar)

El downstream registra un `struct cfg80211_ops mtk_wlan_ops` (`os/linux/gl_init.c:858-897`). La ruta de
cada op baja a un `wlanoidSetXxx` (en `common/wlan_oid.c`) que arma el comando y lo encola con
`wlanSendCommand`/`nicTxCmd`. Para nuestro driver mainline **reescribimos las ops directamente sobre
nuestro armador de comandos** (no portamos la maquinaria OID/glue gigante; ver §7).

### Set mínimo para "conecta a un AP WPA2 y navega" (orden de implementación)
| cfg80211 op | función downstream | baja a | comando FW |
|---|---|---|---|
| `.scan` | `mtk_cfg80211_scan` | `wlanoidSetBssidListScanExt` | `CMD_ID_SCAN_REQ_V2` → resultados como MGMT + `SCAN_DONE` |
| `.connect` | `mtk_cfg80211_connect` | `wlanoidSetInfrastructureMode` + `wlanoidSetEncryptionStatus` + `wlanoidSetSsid`/`SetBssid` | `CMD_ID_INFRASTRUCTURE`, `CMD_ID_SET_BSS_INFO`, claves |
| `.disconnect` | `mtk_cfg80211_disconnect` | `wlanoidSetDisassociate` | (deauth en FW) |
| `.add_key` | `mtk_cfg80211_add_key` | `wlanoidSetAddKey` | `CMD_ID_ADD_REMOVE_KEY` |
| `.del_key` | `mtk_cfg80211_del_key` | `wlanoidSetRemoveKey` | `CMD_ID_ADD_REMOVE_KEY` (remove) |
| `.set_default_key` | `mtk_cfg80211_set_default_key` | `wlanoidSetDefaultKey` | `CMD_ID_DEFAULT_KEY_ID` |
| `.get_station` | `mtk_cfg80211_get_station` | `wlanoidQueryStaStatistics`/link quality | `CMD_ID_GET_LINK_QUALITY` |
| `.change_virtual_intf` | `mtk_cfg80211_change_iface` | set op-mode | `CMD_ID_INFRASTRUCTURE` |

Ops adicionales que el downstream trae pero **diferimos**: `.join_ibss/.leave_ibss`, `.set_pmksa/.del_pmksa/.flush_pmksa`,
`.remain_on_channel`, `.mgmt_tx`, `.assoc`, TDLS, P2P. (P2P es un netdev entero aparte: lo ignoramos.)

### Registro del wiphy (`gl_init.c:~2259-2300`)
- `wiphy_new(&our_cfg80211_ops, sizeof(priv))`.
- Banda 2.4GHz: 14 canales (`CHAN2G(1..14)`, 2412–2484), sin 5GHz (el MT6582 es solo 2.4G).
- `cipher_suites[]`: `WEP40, WEP104, TKIP, CCMP, AES_CMAC` (`gl_init.c:847-856`).
- `interface_modes = BIT(NL80211_IFTYPE_STATION)` (+ ADHOC opcional).
- `max_scan_ssids = 1`, `max_scan_ie_len = 512`, `signal_type = CFG80211_SIGNAL_TYPE_MBM`.
- `flags`: regulatorio custom + `SUPPORTS_FW_ROAM` (opcional).
- `wiphy_register()`, luego crear el `net_device wlan0` (`alloc_netdev` + `register_netdev`) con un
  `struct wireless_dev` enganchado.

### El netdev (data path arriba)
- `ndo_start_xmit`: skb Ethernet → prepend `HIF_TX_HEADER_T` (PKT_TYPE=DATA, StaRecIdx del AP) →
  `kalDevPortWrite(WTDR0)`.
- RX: el RX handler, al ver PKT_TYPE=DATA, quita la cabecera, hace `eth_type_trans`, `netif_rx`.
- `EVENT_ID_TX_DONE` / `WTSR0` para el control de flujo de recursos TX (token por TC). En Phase 3.

---

## 6. Qué REUSAR de `mt6582-btif.c`/`mt6582-consys.c` vs qué PORTAR

### Reusar tal cual (del CONSYS, ya hecho)
- **`mt6582_consys_ready`** (`mt6582-consys.c:146`, ya exportado): el probe del WiFi hace
  `if (!mt6582_consys_ready) return -EPROBE_DEFER;` igual que el btif. **Sin cambios.**
- **La EMI compartida + power SPM/MTCMOS + 26M + activación del MCU**: todo eso ya lo deja listo
  `mt6582-consys.c`. El WiFi no lo toca.

### Reusar del btif **vía una pequeña ampliación** (hay que tocar `mt6582-btif.c`)
El único servicio que el WiFi necesita del btif es **encender la radio WiFi por WMT**: el equivalente a
`func_on(BT)` / `func_on(GPS)` que ya hace el btif, pero con `type = WIFI`. Hoy `func_on()` es `static`.

**Plan: exportar una API mínima desde el btif** (lo añado como TODO en el btif, y el scaffold del WiFi la
llama):
```c
/* en mt6582-btif.c, nuevo: */
int mt6582_consys_func_on(u8 type);   /* type: 0=BT,1=FM,2=GPS,3=WIFI,... usa el wmt_cmd interno */
EXPORT_SYMBOL_GPL(mt6582_consys_func_on);
/* (opcional, para reset/cleanup) */
int mt6582_consys_func_off(u8 type);
```
Implementación: es exactamente el cuerpo de `func_on()` actual (arma
`{0x01,0x06,0x02,0x00,type,0x01}`, `stp_send(WMT)`, espera el evt `{0x02,0x06,...,0}`), pero tomando un
lock para no pisar el RX-thread del BT. **Coste: ~15 líneas + un mutex que ya existe (`tx_lock`)**.

> **Importante:** el btif ya hace `func_on(GPS)` dentro de `bring_up_chip()` de forma síncrona *antes* de
> arrancar el kthread RX. Para el WiFi NO podemos hacer eso en el bring-up del btif (el WiFi se carga
> después y por otro driver). Por eso necesitamos la versión exportada y **thread-safe** (el kthread RX
> del BT estará corriendo). El `wmt_cmd` síncrono compite con el RX-thread por `rxbuf`; la API exportada
> debe **serializar con el RX-thread** (un mutex de "WMT en curso" que el RX-thread respete, o mover el
> demux de WMT al propio RX-thread y exponer un completion). Lo dejo señalado como el punto delicado de
> la integración btif (ver ROADMAP Fase 0).

### Lo que NO se reusa del btif (el WiFi tiene lo suyo)
- El framing STP, `btif_tx_raw`, `vff_setup`, las colas VFF: **nada de esto aplica al WiFi**. El WiFi usa
  registros MCR + PDMA propio.
- El patch del MCU (`mt6572_82_patch_e1_x`): ya lo hace el btif en el bring-up. El WiFi **no** lo repite.

### Lo que se PORTA del downstream (lo nuevo, en `mt6582-wifi.c`)
- La capa HIF: `kalDevPortRead/Write` (PIO primero, PDMA después), driver-own handshake, poll de WHISR.
  → de `os/linux/hif/ahb/ahb.c` + `ahb_pdma.c` + `mtreg.h`.
- La descarga de firmware: `wlanImageSectionDownload` + parser de `FIRMWARE_DIVIDED_DOWNLOAD_T`.
  → de `common/wlan_lib.c` + `nic_init_cmd_event.h`.
- El armador de comandos y el RX-dispatcher: versión mínima de `nicTxCmd`/`nicRxProcessRFBs`.
  → de `nic/nic_tx.c`, `nic/nic_rx.c`, `nic/cmd_buf.c` + `nic_cmd_event.h`.
- La capa cfg80211: las ~8 ops del §5. → de `os/linux/gl_cfg80211.c` (reescritas, no portadas literal).

---

## 7. Estrategia: NO portar los 176k líneas

El downstream mete una FSM de management completa (`mgmt/ais_fsm.c`, `assoc.c`, `auth.c`, `sec_fsm.c`,
`rsn.c`, scan FSM, CNM, roaming, P2P, BOW, TDLS, HS20…). **El firmware full-MAC ya hace casi todo eso**:
la asociación/auth 802.11, el handshake de claves a nivel MAC, el roaming. El host solo manda comandos de
alto nivel (`SET_BSS_INFO`, `ADD_KEY`, `SCAN_REQ`) y recibe estado. Por tanto:

- **Implementamos un "thin full-MAC"**: cfg80211_ops → armador de comandos → HIF. Sin FSM propia.
- La FSM del downstream (`ais_fsm`) la usamos **como documentación** de qué comandos manda en qué orden al
  conectar (mirar `aisFsmRunEventJoinRequest` → `SET_BSS_INFO` + `ACTIVATE_STA` + `ADD_KEY`), no la
  portamos.
- Riesgo: algunos firmwares MTK de esta época esperan una coreografía exacta (p.ej. `CH_PRIVILEGE` antes
  de mandar tramas, o `STA_REC` antes de `ADD_KEY`). Eso se descubre en Fase 2 leyendo `ais_fsm.c` y con
  pruebas. Por eso el roadmap separa "scan" (fácil, sin STA-record) de "connect" (la coreografía).

---

## 8. Mapa de ficheros del downstream → de dónde sacar cada cosa

| Necesito… | Mirar en el downstream |
|---|---|
| Bases/offsets de registros HIF + PDMA | `os/linux/hif/ahb/include/hif.h`, `hif_pdma.h`, `include/nic/mtreg.h` |
| Driver-own / FW-own / poll WCIR | `nic/nic_pwr_mgt.c` (`nicpmSetDriverOwn`), `mtreg.h` (WHLPCR/WCIR) |
| Lectura/escritura de puertos (PIO+DMA) | `os/linux/hif/ahb/ahb.c:1197-1470` (read), `:1487+` (write), `ahb_pdma.c` |
| Descarga de firmware + parser cabecera | `common/wlan_lib.c:1219-1817` (`wlanAdapterStart`), `:2990-3436` (download), `include/wlan_lib.h:568-583`, `nic_init_cmd_event.h` |
| Nombre/carga del fichero de firmware | `include/config.h:919-922`, `os/linux/gl_kal.c:1123-1150` (`kalFirmwareImageMapping` con `request_firmware`) |
| Structs cmd/event + opcodes | `include/nic_cmd_event.h`, `include/nic/cmd_buf.h`, `include/nic/hif_tx.h`, `hif_rx.h` |
| Armar y mandar un comando | `nic/cmd_buf.c`, `nic/nic_tx.c` (`nicTxCmd`, `nicTxInitCmd`) |
| Despachar RX (cmd/event/data/mgmt) | `nic/nic_rx.c` (`nicRxProcessRFBs`, `nicRxProcessEventPacket`, `nicRxProcessDataPacket`) |
| cfg80211 ops + wiphy + bandas/cifrados | `os/linux/gl_cfg80211.c`, `os/linux/gl_init.c:737-897, 2259-2300` |
| Qué comandos manda al conectar (coreografía) | `mgmt/ais_fsm.c`, `mgmt/assoc.c`, `mgmt/sec_fsm.c`, `common/wlan_oid.c` |
| Coordinación de reset con WMT (opcional) | `os/linux/gl_rst.c` |

---

## 9. Resumen de una línea (para arriba del todo)

**El data-path WiFi NO va por BTIF/STP**: el WiFi del MT6582 es un periférico AHB con registros estilo-SDIO
en **`0x180F0000`** y un canal **PDMA propio en `0x11000180`**; STP/WMT solo sirve para `func_on(WIFI)` y
reset. El firmware `WIFI_RAM_CODE` se descarga por el **puerto de datos del HIF** (no por STP) como
comandos `INIT_CMD_ID_DOWNLOAD_BUF`. Nuestro driver es un `platform_driver` autónomo que reusa
`mt6582_consys_ready` y una nueva `mt6582_consys_func_on(WIFI)` exportada del btif, y por lo demás vive
sobre `ioremap(0x180F0000)`.
