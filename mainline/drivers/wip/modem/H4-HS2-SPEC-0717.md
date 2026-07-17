# Módem H4 — HS2 (= M1 completo): SPEC DE IMPLEMENTACIÓN reverse-engineered (0717)

> Tras HS1 (el MD arranca y señaliza, `RCHNUM=0x1`, ver H3v3), falta HS2: el AP
> responde con la **runtime data** + un mensaje de control por CCIF, y el MD avanza
> a stage 2 = **M1 COMPLETO** (`/sys/kernel/ccci/boot = md1:2`). Aquí está TODO el
> mecanismo RE'd del stock, listo para codificar.

## Flujo HS2 (stock `ccci_send_run_time_data`, ccci_md_main.c)

1. El MD manda HS1 por CCIF (canal lógico CCCI_CONTROL_RX=0): mensaje id=MD_INIT_START_BOOT.
   → nuestro poll ya lo ve como `RCHNUM` bit 0.
2. AP construye `modem_runtime_t` en el SMEM (region runtime_data).
3. AP escribe `config_misc_info` (magic "CCIF" + dirección de mem del MD) en la region misc_info.
4. AP escribe el TAG (`modem_runtime_info_tag_t`, 28B) en la SRAM del CCIF @ offset **0x140**.
5. AP manda por CCIF (canal físico rotativo, empieza 0) el mensaje de arranque:
   `{magic=0xFFFFFFFF, id=MD_INIT_START_BOOT(0x0), channel=CCCI_CONTROL_TX(1), reserved=MD_INIT_CHK_ID(0x5555FFFF)}`
6. El MD lee el TAG (0x140) → apunta al runtime struct en SMEM → lo valida y configura →
   manda BootReadyID por CCIF → **stage 2 = HS2 = M1 completo**.

## Constantes (confirmadas del source)

| símbolo | valor | fuente |
|---|---|---|
| MD_INIT_START_BOOT | 0x00000000 | ccci_md.h:57 |
| MD_INIT_CHK_ID | 0x5555FFFF | ccci_md.h:58 |
| NORMAL_BOOT_ID | 0 | ccci_md.h:25 |
| CCCI_CONTROL_RX / TX | 0 / 1 | ccci_ch.h:25-26 |
| Prefix/Postfix | 0x46494343 ("CCIF") | set_md_runtime |
| Platform_L/H | "MT6582_S00" (primeros 8B) | platform_set_runtime_data / CCCI_PLATFORM |
| CheckSum | **NO se valida** (solo un print) — dejar 0 | ccci_md_main.c:850 |
| md_2_ap_phy_addr_offset_fixed | **0x78000000** = (smem_base & 0xFE000000) - 0x40000000 | ccci_settings.c:220 |

## CCIF hardware (base AP_CCIF = 0x1020A000, offsets ccci_platform_cfg.h)

- TX de un mensaje (`__ccif_v1_write_phy_ch_data`, ccci_hw.c:104): ch = tx_idx (0..7, rota);
  si `CCIF_BUSY & (1<<ch)` ocupado → esperar; si no: `CCIF_BUSY=(1<<ch)`; escribir el msg de
  4 words a `TXCHDATA(0x100) + ch*16` (data0,data1,channel,reserved); `CCIF_TCHNUM=ch` (dispara).
- TAG runtime: escribir a `AP_CCIF_base + 0x140` (CCIF_STD_V1_RUN_TIME_DATA_OFFSET), max 192B.
- CCIF_STD_V1_MAX_CH_NUM = 8.

## SMEM layout (base = 0xb9600000, secuencial; ccci_settings.c:290+)

Orden de sub-regiones (cada `base_phy += size`):
1. **runtime_data** @ 0xb9600000, size = sizeof(modem_runtime_t) (~50 int = 200B aprox)
2. exp_smem: MD_EX_LOG_SIZE = **2048** (platform_cfg, no el 512 del otro árbol)
3. md_ex_exp_info: CCCI_MD_EX_EXP_INFO_SMEM_SIZE (buscar; ~query)
4. misc_info: CCCI_MISC_INFO_SMEM_SIZE = **1024**
5. (round_up 0x1000) pcm, mdlog, rpc, fs, uart[8], net... (tamaños = structs anidados)

**⚠️ Los tamaños de pcm/rpc/fs/tty/net son `sizeof()` de structs anidados** (RPC_BUF,
fs_stream_buffer_t, shared_mem_tty_t) → computarlos a mano es propenso a error.

## Direcciones en el runtime struct (MD-view)

Cada `runtime->XxxBase = smem_table->ccci_xxx_base_phy - 0x78000000`. Ej: runtime_data @
0xb9600000 → MD-view 0x41600000; misc_info → su phys - 0x78000000. (El MD accede al SMEM por
su ventana BANK4 en 0x40000000 = 0xb8000000 phys; 0xb9600000-0x78000000=0x41600000 ✓.)

## modem_runtime_t (ccci_md.h, todos `int`=4B, en orden):

Prefix, Platform_L, Platform_H, DriverVersion, BootChannel, BootingStartID, BootAttributes,
BootReadyID, MdlogShareMemBase/Size, PcmShareMemBase/Size, UartPortNum, UartShareMemBase[8],
UartShareMemSize[8], FileShareMemBase/Size, RpcShareMemBase/Size, PmicShareMemBase/Size,
ExceShareMemBase/Size, SysShareMemBase/Size, IPCShareMemBase/Size, MDULNetShareMemBase/Size,
MDDLNetShareMemBase/Size, NetPortNum, NetULCtrlShareMemBase[4], NetULCtrlShareMemSize[4],
NetDLCtrlShareMemBase[4], NetDLCtrlShareMemSize[4], MDExExpInfoBase/Size, IPCMDIlmShareMemBase/Size,
MiscInfoBase/Size, CheckSum, Postfix.

## ⭐ VÍA RECOMENDADA: capturar el runtime EXACTO de LineageOS

`ccci_dump_runtime_data` (ccci_md_main.c:776) **loguea TODOS los campos del runtime struct**
(Prefix, Platform, todas las Base/Size, ...) justo antes de mandarlo. En vez de computar el
SMEM layout a mano (structs anidados), **capturar ese dump en Lineage** durante el arranque del
MD da los valores exactos a replicar 1:1. Reto: el dump es 1 vez al boot (rotó del dmesg) →
hace falta forzar un reset del MD en Lineage (el nodo `/sys/kernel/ccci/boot` tiene store=NULL;
lo hace `ccci_mdinit` por ioctl en `/dev/ccci_*`; explorar: toggle airplane mode / kill+restart
rild / un ioctl CCCI_IOC de reset). Con el dump capturado, HS2 es replay directo.

## Implementación (spm_md_hs2, disparado tras ver RCHNUM=1 en spm_md_release)

1. Leer el mensaje HS1 del RX (confirmar id=MD_INIT_START_BOOT); ACK el canal (`CCIF_ACK=1<<ch`).
2. ioremap SMEM (0xb9600000) + AP_CCIF (0x1020A000).
3. Construir el runtime struct (memset 0 + campos de arriba) en SMEM@0; misc_info en su offset.
4. Escribir el tag (prefix, platform, driver_ver, runtime_data_base=0x41600000,
   runtime_data_size=sizeof(runtime), postfix) a CCIF+0x140.
5. Enviar el msg de arranque (0xFFFFFFFF, 0, 1, 0x5555FFFF) a TXCHDATA+0*16 + TCHNUM=0.
6. Poll RCHNUM por la respuesta del MD (BootReadyID) → HS2 OK.

## 🔶 1er intento HS2 IMPLEMENTADO Y PROBADO (0717, kernel #17) — mecanismo OK, MD no avanza aún

Implementado `spm_md_hs2` (trigger sysfs): ACK del HS1 + runtime struct minimo (share-mems a 0
salvo MiscInfo) en SMEM@0 + misc_info@0x400 + tag en CCIF@0x140 + mensaje de arranque
(0xFFFFFFFF/0/1/0x5555FFFF) por CCIF TX ch0. **Secuencia: reboot → poweron → load → remap →
release(HS1, RCHNUM=1) → hs2.** Resultado:
```
H4 HS2: pre RCHNUM=0x1 (HS1 visto)  -> ACK canal 0
H4 HS2: runtime+tag+msg enviados. Sondeando 5s...
H4 HS2: sin respuesta nueva tras 5s. RCHNUM=0x0
```
**El mecanismo funciona** (ve el HS1, lo ACKea → RCHNUM vuelve a 0, manda el mensaje sin error),
pero **el MD no responde** con BootReadyID. Causa casi segura: el **runtime struct minimo (casi
todo a 0) no valida** en el MD — necesita los valores reales de las sub-regiones share-mem.
También posible: (a) offset RX mal (RX0 lee 0xffffffff siempre — revisar dónde cae el mensaje del
MD), (b) falta generar el IRQ al MD / timing, (c) el MD espera regiones share-mem concretas != 0.

**⇒ SIGUIENTE (lo que de-riska): capturar el runtime EXACTO de Lineage** (`ccci_dump_runtime_data`
loguea los 50 campos) y replicarlo 1:1, en vez de computar el SMEM layout a mano. Reto = forzar
reset del MD en Lineage para que re-loguee (explorar toggle airplane / kill rild / ioctl CCCI).
Alternativa: computar los `sizeof()` de las sub-regiones (structs anidados) y rellenar todas las
Base/Size del runtime. Los offsets/mecanismo (tag@0x140, TX ch0, MD_AP_OFF=0x78000000) están YA
validados por esta prueba — solo falta el CONTENIDO correcto del runtime.

## Estado
- HS1 ✅ (H3v3). HS2 = mecanismo implementado y validado (envía/ACKea); falta el runtime correcto.
- Constantes finales: CCCI1_DRIVER_VER=0x20121001, Platform="MT6582_S00" (L=0x3536544d H=0x535f3238),
  misc_info: prefix+feature_0_val[0]=md_mem_start(0xb8000000). get_md_mem_start_addr=md_resv_mem_addr.
- Kernel test = #16 boot-md-test.img (tiene load/remap-BANK0/CCIF-init/release).
- Fuentes stock: `ccci_md_main.c` (set_md_runtime:1391, ccci_send_run_time_data, dump:776),
  `ccci_hw.c` (write_phy_ch_data:104, write_runtime_data:206), `ccci_settings.c` (smem layout:247),
  `ccci_md.h`/`ccci_ch.h`/`ccci_platform_cfg.h` (structs/constantes).
