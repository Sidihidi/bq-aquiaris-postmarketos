# Módem H6 — Proxy FS (EFS/NVRAM) — spec del protocolo + plumbing (0720)

> Continuación de [H5-CCCI-SERVICE-FS-NVRAM-0719.md]. El MD arranca MOLY hasta pedir el mount de `Z:\`
> (su EFS/NVRAM, canal `CCCI_FS_RX`=14) y bloquea HS2 esperando la respuesta del AP. Aquí: el protocolo
> FS entero (RE del `dual_ccci` stock) + el plumbing en kernel para responderle. **Falta la ground-truth
> del CONTENIDO de la respuesta (snoop de LineageOS).**

## Fuente RE
`~/Downloads/kernel-3.18/drivers/misc/mediatek/dual_ccci/` (el CCCI del MT6582):
`ccci_fs_main.c`, `include/ccci_fs.h`, `include/ccci_ch.h`, `ccci_hw.c`, `ccci_logical.c`.

## El protocolo FS (byte a byte)

**Canales lógicos** (`ccci_ch.h`): `CCCI_CONTROL_RX=0, CCCI_CONTROL_TX=1, CCCI_FS_RX=14, CCCI_FS_TX=15`.

**Shared-mem FS** (`ccci_fs.h`): 5 buffers de
```c
struct fs_stream_buffer_t { u32 fs_ops; u8 buffer[16384]; };   /* = 16388B = 0x4004 */
CCCI_FS_SMEM_SIZE = 16388 * 5 = 0x14014   /* == el FileShareMemSize del fix del data-abort */
```
En nuestro driver la región FS va en `FileShareMemBase = MD_SMEM_PHYS+0xE000 = 0xB960E000` (phys),
MD-view `0x4160E000` (= el `d0` capturado en H5). Buffer `idx` en `0xB960E000 + idx*0x14014`.

**RX (MD→AP)** — `ccci_fs_callback`: el MD manda un mensaje CCIF por `CCCI_FS_RX(14)` con
`reserved = idx` (el índice del buffer). La petición (fs_op + params + path) está en `fs_buffers[idx]`.

**Petición** (capturada en H5, buffer idx 0):
```
+0x00: fs_ops = 0x0000100e     +0x04: 0x00000002     +0x08: 0x00000008
+0x0c: "Z:\" (5a 00 3a 00 5c 00 00 00, UTF-16LE)  + flags 04 03
```
(`fs_ops=0x100e` = la operación; el resto = params/longitud de path + el path del EFS.)

**TX (AP→MD)** — `ccci_fs_send` (el proxy responde tras hacer el file-op):
```c
msg.data0    = fs_buffers_phys - md2ap_off + (16388 * idx);   /* MD-view del buffer idx */
msg.data1    = length + 4;                                    /* long. de la respuesta + 4 */
msg.channel  = CCCI_FS_TX(15);
msg.reserved = idx;
ccci_message_send(...)   /* -> __ccif_v1_write_phy_ch_data */
```

**TX físico** (`__ccif_v1_write_phy_ch_data`, ccci_hw.c): canal físico = `m_tx_idx` round-robin
(`&= 7`, 8 canales); si `BUSY & (1<<ch)` ocupado → retry; si libre → `BUSY|=(1<<ch)`, escribe
`TXCHDATA[ch] = {data0,data1,channel,reserved}` (@0x100+ch*16), dispara `TCHNUM=ch` (@0x0c).

## Plumbing implementado (mt6582-spm-H1.c, spm_md_hs2, bucle de servicio)
`ioremap` de la región FS (`0xB960E000, 0x15000`). En el bucle, si `lch==14`:
1. **Vuelca la petición** (`H6 FS REQ idx=.. op=.. path=..`) — así vemos qué pide el MD EN NUESTRO HW.
2. Escribe `result=0` en `fs_buffers[idx][0]` — **HIPÓTESIS** (ver ground-truth abajo).
3. Envía `FS_TX(15)`: busca canal físico TX libre (BUSY bits 0-7), ocupa, escribe
   `{data0=MD-view buffer, data1=4, channel=15, reserved=idx}`, dispara `TCHNUM`.
4. ACKea el canal RX (libera el TX del MD). Compila RC=0 en la Pi.

## ⚠️ LO QUE FALTA — el CONTENIDO de la respuesta (ground-truth)
El kernel `ccci_fs_main.c` es solo un PIPE: no interpreta `fs_ops`. La semántica (qué resultado espera
MOLY para cada op: handle, código, datos, y a qué offset) vive en **`ccci_fsd` (userspace)** — que NO
tenemos en fuente. Por eso el `result=0` del plumbing es una hipótesis. **La ground-truth se saca de
LineageOS** (mismo método que cerró el WiFi/GPS): el driver stock tiene flags de debug que loguean cada
mensaje FS, y podemos volcar el buffer.

### SNOOP a ejecutar en LineageOS (arranque del módem con log FS)
```sh
adb root
# activar el log de FS (rx y tx) del dual_ccci:
adb shell "echo 1 > /sys/class/ccci_node/... "   # o el debugfs/param equivalente (fs_rx_debug_enable/fs_tx_debug_enable)
# provocar el boot del MD y capturar:
adb shell dmesg -c > /dev/null; <arrancar/reset del MD>; adb shell dmesg | grep -iE "fs_callback|fs_send|ccci.*fs"
# volcar el contenido del buffer FS antes/después de cada respuesta (via /dev/ccci_fs mmap o devmem
# a la phys de fs_buffers): capturar la pareja PETICIÓN -> RESPUESTA de cada fs_op del boot.
```
Con la secuencia real `op -> (buffer_in, buffer_out)` de cada paso del mount de `Z:\`, se implementan
las respuestas de verdad. Alternativa: backing-store con la **NVRAM real** del modem (ya extraída) para
que las lecturas devuelvan calibración RF correcta (imprescindible para señal).

## Estado
Plumbing FS listo y compilando (kernel de test = `mt6582-spm-H1.c` de este commit). Pendiente: (1) probar
en HW → ver la reacción del MD a la respuesta-hipótesis + volcar sus peticiones; (2) snoop de LineageOS
para el contenido real; (3) implementar las respuestas por `fs_op`. El árbol de la Pi se dejó en la
versión H5 (no se pisó el build de casa con el H6 experimental).

*Mac (Fable 5), 2026-07-20. Protocolo FS mapeado byte a byte + plumbing en kernel. Falta la ground-truth
del contenido (snoop Lineage).*
