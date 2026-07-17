# Módem H3 — HS2: receta COMPLETA del handshake (runtime data + MD_INIT) — 0717

> **Todo lo necesario para cerrar HS2, extraído del source stock** (bq-src, byte-idéntico
> al que corre en LineageOS 13 en este chip). **Nota metodológica clave**: los campos de
> DIRECCIÓN del runtime dependen del LAYOUT de memoria (nuestro carveout @0xb8000000), NO
> del de Lineage. Un volcado de la SMEM de Lineage daría SUS direcciones (su carveout) =
> **incorrectas para nosotros**. La derivación del source (calcular sobre NUESTRO layout)
> es la verdad correcta; el source es idéntico en ambos, solo cambian las bases de memoria.

## Flujo del handshake (quién hace qué)

```
MD (MOLY)                          AP (nuestro driver)
  |-- HS1: msg por CCIF RX -------->|  (canal CCCI_CONTROL_RX=0)
  |                                 |  ccci_send_run_time_data():
  |                                 |   1. set_md_runtime() -> escribe modem_runtime_t en SMEM
  |                                 |   2. config_misc_info() -> escribe misc_info en SMEM
  |                                 |   3. envia CCIF TX: {0xFFFFFFFF, 0, 1, 0x5555FFFF}
  |<-- MD_INIT_START_BOOT ----------|  (canal CCCI_CONTROL_TX=1)
  |-- lee runtime, se configura     |
  |-- HS2: NORMAL_BOOT_ID por CCIF ->|  -> boot stage 2 = M1 COMPLETO
```

Ya teníamos HS1 pendiente (el MD no hablaba); si con el fix del power/CCIF el MD llega a
HS1, este doc cierra el HS2. (Si HS1 sigue sin llegar, es problema del arranque del MD,
no de aquí — ver H3-ARRANQUE.)

## 1. modem_runtime_t (280 bytes = 70 ints, orden EXACTO)

```c
typedef struct {          // TODO int (4B), little-endian
    int Prefix;           // 0x46494343  "CCIF"
    int Platform_L;       // 0x3536544D  "MT65"  (de CCCI_PLATFORM="MT6582E1")
    int Platform_H;       // 0x31453238  "82E1"
    int DriverVersion;    // 0x20121001  (CCCI1_DRIVER_VER)
    int BootChannel;      // 0  (CCCI_CONTROL_RX)
    int BootingStartID;   // 0  (NORMAL_BOOT_ID; mdlog_flag=0 sin /data/mdl/mdl_config)
    int BootAttributes;   // 0
    int BootReadyID;      // 0
    int MdlogShareMemBase, MdlogShareMemSize;      // 0,0 si no se usa
    int PcmShareMemBase,   PcmShareMemSize;         // 0,0
    int UartPortNum;                                // 0 (sin puertos UART en bring-up)
    int UartShareMemBase[8], UartShareMemSize[8];   // 0
    int FileShareMemBase,  FileShareMemSize;        // 0,0 (FS opcional)
    int RpcShareMemBase,   RpcShareMemSize;         // 0,0
    int PmicShareMemBase,  PmicShareMemSize;        // 0,0
    int ExceShareMemBase,  ExceShareMemSize;        // base MD-side, 0x800 (2KB)
    int SysShareMemBase,   SysShareMemSize;         // 0,0
    int IPCShareMemBase,   IPCShareMemSize;         // 0,0
    int MDULNetShareMemBase, MDULNetShareMemSize;   // 0,0
    int MDDLNetShareMemBase, MDDLNetShareMemSize;   // 0,0
    int NetPortNum;                                 // 0
    int NetULCtrlShareMemBase[4], NetULCtrlShareMemSize[4];  // 0
    int NetDLCtrlShareMemBase[4], NetDLCtrlShareMemSize[4];  // 0
    int MDExExpInfoBase,   MDExExpInfoSize;         // base MD-side, size CCCI_MD_EX_EXP_INFO_SMEM_SIZE
    int IPCMDIlmShareMemBase, IPCMDIlmShareMemSize; // 0,0
    int MiscInfoBase,      MiscInfoSize;            // base MD-side, 0x400 (1KB)
    int CheckSum;          // 0 (NUNCA se calcula, queda del memset)
    int Postfix;           // 0x46494343  "CCIF"
} modem_runtime_t;
```

**Mínimo viable para HS2**: casi todo 0. Las sub-regiones que el MD SÍ necesita
localizadas: **Exce** (excepción, 2KB), **MDExExpInfo**, **MiscInfo** (1KB). El resto
(mdlog/pcm/uart/net/fs/rpc/pmic/ipc) puede ir a 0 en bring-up (validar iterando: si el MD
se queja de una región, darle esa).

## 2. Layout del SMEM (ccci_settings.c: ccci_alloc_smem) — sub-regiones SECUENCIALES

Desde la base física de la SMEM (`md_resv_smem_addr[MD_SYS1]`, NUESTRA elección — H2 la
puso en el carveout), en este orden, cada una `round`-eada:
```
+0x000  ccci_md_runtime_data   size = sizeof(modem_runtime_t) = 280
+...    ccci_exp_smem          size = MD_EX_LOG_SIZE          = 0x800 (2KB)
+...    ccci_md_ex_exp_info    size = CCCI_MD_EX_EXP_INFO_SMEM_SIZE
+...    ccci_misc_info         size = CCCI_MISC_INFO_SMEM_SIZE = 0x400 (1KB)
+...    (pcm, mdlog, rpc, fs, ...)   // opcionales en bring-up
```
El runtime data va en el OFFSET 0 de la SMEM. `config_misc_info` escribe en
`ccci_misc_info_base_virt`.

## 3. Conversión de direcciones AP → MD (la fórmula)

```c
md_2_ap_phy_addr_offset_fixed = (smem_base_before_map & 0xFE000000) - 0x40000000;
// cada ShareMemBase del runtime = <sub_region_phys_AP> - md_2_ap_phy_addr_offset_fixed
```
Y el remap BANK4 (H2, ya hecho): el MD ve la SMEM en 0x40000000.
`set_ap_smem_remap(0x40000000, smem_base)` + `set_md_smem_remap(...)`.

## 4. config_misc_info (ccci_platform.c:196) — escribe en misc_info_base

```c
misc_info_t misc:  // memset 0
  prefix = postfix = 0x46494343 ("CCIF")   // str[0] de snprintf("CCIF")
  index = 0; next = 0;
  feature_0_val[0] = get_md_mem_start_addr(MD_SYS1);  // = md_resv_mem_addr (0xb8000000)
  support_mask |= (FEATURE_SUPPORT << MISC_DMA_ADDR);
  support_mask |= (FEATURE_NOT_SUPPORT << (MISC_32K_LESS*2));  // 32k-less OFF
```
(Sacar `misc_info_t`, `FEATURE_SUPPORT/NOT_SUPPORT`, `MISC_DMA_ADDR`, `MISC_32K_LESS` del
header cuando se implemente.)

## 5. Envío por CCIF (ccci_hw.c) — el mensaje MD_INIT_START_BOOT

`ccci_msg_t msg = {magic, id, channel, reserved}`:
```
magic    = 0xFFFFFFFF
id       = MD_INIT_START_BOOT = 0x00000000
channel  = CCCI_CONTROL_TX    = 1
reserved = MD_INIT_CHK_ID      = 0x5555FFFF
```
Secuencia HW (AP_CCIF @0x1020A000):
```c
busy = read(CCIF_BUSY);                 // +0x04
ch = tx_idx; if (busy & (1<<ch)) retry; // buscar canal libre
write(CCIF_BUSY, 1<<ch);                // ocupar canal
tx = CCIF_TXCHDATA + ch*16;             // +0x100, cada slot 16B (4 ints)
write(tx+0,  msg.magic);
write(tx+4,  msg.id);
write(tx+8,  msg.channel);
write(tx+12, msg.reserved);
write(CCIF_TCHNUM, ch);                 // +0x0C  -> dispara IRQ al MD
```
`CCIF_STD_V1_MAX_CH_NUM` canales (sacar valor; típico 8). tx_idx incrementa y wrapea.

## 6. RX del handshake (para detectar HS1 y HS2)

IRQ del CCIF (GIC_SPI 100). En el handler:
```c
rchnum = read(CCIF_RCHNUM);   // +0x10, bitmap de canales con datos
por cada ch con dato:
  rx = CCIF_RXCHDATA + ch*16;  // +0x180
  msg = {rx[0], rx[1], rx[2], rx[3]};
  write(CCIF_ACK, 1<<ch);      // +0x14, ack
```
- **HS1** = el MD manda un msg por el canal RX de control → dispara el paso 1-5.
- **HS2** = el MD manda `NORMAL_BOOT_ID`/boot-ready → stage 2 = M1 OK.

## Constantes (todas confirmadas del source)
| símbolo | valor |
|---|---|
| Prefix/Postfix "CCIF" | 0x46494343 |
| CCCI_PLATFORM | "MT6582E1" → L=0x3536544D H=0x31453238 |
| CCCI1_DRIVER_VER | 0x20121001 |
| NORMAL_BOOT_ID | 0 |
| MD_INIT_START_BOOT | 0x00000000 |
| MD_INIT_CHK_ID | 0x5555FFFF |
| CCCI_CONTROL_RX / TX | 0 / 1 |
| MD_EX_LOG_SIZE | 0x800 (platform cfg) |
| CCCI_MISC_INFO_SMEM_SIZE | 0x400 |
| CCCI_UART_PORT_NUM / NET_PORT_NUM | 8 / 4 |
| CCIF regs (base 0x1020A000) | CON+0 BUSY+4 START+8 TCHNUM+C RCHNUM+10 ACK+14 TXCHDATA+100 RXCHDATA+180 |
| CheckSum | 0 (no se calcula) |

## Pendiente al implementar
- Sacar `misc_info_t`, `ccif_msg_t`, `CCIF_STD_V1_MAX_CH_NUM`, `CCCI_MD_EX_EXP_INFO_SMEM_SIZE`,
  y los enums FEATURE_*/MISC_* (headers del downstream).
- Escribir el runtime al offset 0 de la SMEM antes de enviar el TX.
- Handler IRQ del CCIF que detecte HS1 (dispara la respuesta) y HS2 (marca M1 OK).
- **Requisito previo**: que el MD llegue a HS1 (arranque real del MD — H3-ARRANQUE). Si no
  hay HS1, esto no se ejercita.

*0717, sesión Windows (Fable 5). Fuente: bq-src dual_ccci (ccci_md_main.c set_md_runtime/
ccci_send_run_time_data, ccci_settings.c smem layout, ccci_hw.c CCIF TX, ccci_platform.c
config_misc_info). Ground truth = LineageOS corre este mismo código en este chip.*
