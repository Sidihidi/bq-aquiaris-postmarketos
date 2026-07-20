# Módem H6b — RE del daemon `ccci_fsd` (respuesta FS ground-truth) — 0720

> Para implementar la respuesta CORRECTA del proxy FS (H6) hay que saber, por cada `fs_op`, qué escribe
> `ccci_fsd` (userspace) de vuelta en el buffer. Como no hay source, **extraído el binario de LineageOS**
> y hecho RE. Este doc = lo hallado + lo que queda (el formato byte-a-byte por op, mejor en Ghidra).

## Extracción (sin rebootar a Lineage)
Desde la pmOS viva, montado el `system` de LineageOS (ext4 @ sector 205824, método de H5) y sacados:
- **`~/modem-fsd/ccci_fsd`** (108500 B, ELF ARM 32-bit EABI5, **stripped**, dynlink) ← el daemon FS
- **`~/modem-fsd/ccci_mdinit`** (42720 B) ← el init del MD (boot del modem)
- **`~/modem-fsd/libnvram.so`** (50368 B) ← la lib de NVRAM que usa el daemon
(Binarios PROPIETARIOS → en la Pi, NO en el repo. Re-extraíbles del `/mnt/lin/bin/` de Lineage.)

## Lo confirmado por RE (r2)

### El daemon hace FILE-OPS REALES (no es un stub)
Enlaza `libc, libcutils, libnvram, liblog`. Imports: `open/read/write/close/lseek/creat/opendir/
readdir/closedir/fstat/mmap/ioctl` + `NVM_RestoreFromBinRegion_OneFile` + `acquire/release_wake_lock`.
Espera a `service.nvram_init` = "Ready" (`property_get`) antes de servir. Handlers (por los logs):
**O**(pen) **R**(ead) **W**(rite) **D**(elete) **X**(delete) **M**(ove) + `FS_CCCI_GetDrive` + OTP
(`/dev/otp`: OTPLock/OTPWrite/OTPQueryLength — NVRAM segura).

### ★ Mapeo de la unidad "Z:\" (dónde vive la NVRAM del MD)
```
"Z:\"        -> /data/nvram/md          (EFS/NVRAM del modem: calibración RF, IMEI, config)
  (md2 var.) -> /data/nvram/md2  +  /protect_f/md2  +  /protect_s/md2
```
`FS_CCCI_GetDrive` valida `type == FS_DRIVE_I_SYSTEM`. → **el backing-store del proxy = `/data/nvram/md`**
(la NVRAM real del módem, ya extraída por casa; imprescindible para calibración RF = señal).

### Ciclo del daemon (main loop)
1. `ioctl(fd, CCCI_FS_IOCTL_GET_INDEX)` (= `_IO('K',1)` = **0x00004b01**) → bloquea hasta que el MD manda
   (el driver mete el `index` en un fifo desde `ccci_fs_callback`); devuelve `index`.
2. `mmap` de la shared-mem FS → lee `fs_buffers[index]` = `{u32 fs_ops; u8 buf[16384]}`.
3. **switch(fs_ops)** → handler (O/R/W/D/M/GetDrive) → file-op real bajo `/data/nvram/md`.
4. Escribe el RESULTADO en `fs_buffers[index].buf` (formato por-op).
5. `ioctl(fd, CCCI_FS_IOCTL_SEND, &{length, index})` (= `_IOR('K',2,u32)` = **0x80044b02**) →
   el driver manda `FS_TX`: `data0 = MD-view del buffer+16388*idx`, `data1 = length+4`, `rsv = idx`.

Petición capturada (H5, idx 0): `fs_ops=0x100e`, params `0x02, 0x08`, path `"Z:\"` (UTF-16LE).

## ⚠️ LO QUE QUEDA — el formato byte-a-byte de la respuesta por op
El `fs_ops` (0x100e = ?) y el layout EXACTO del resultado que MOLY espera por cada op (Open→handle,
Read→{code,len,data}, etc.) están en el código del daemon. El RE en r2 es lento (stripped, 51 fcns,
mucho logging por md_id). **Recomendado: Ghidra (que casa ya usa)** sobre `~/modem-fsd/ccci_fsd`:
- Localizar el main loop (xref a "Main: FS_CCCI_GetDrive" @0x188d8 / "buffer index" @0x17ef8).
- Sacar la tabla `fs_ops -> handler` (mapear 0x100e y los siguientes que el MD usa en el boot).
- Por cada handler: el layout del resultado que escribe en el buffer + el `length` del SEND.

## Alternativa (más rápida que RE completo): captura en vivo
En LineageOS, activar el log FS del driver stock (`ccci_ch_filter_store`, comando `-c`):
`-c=0 0x20111111 0x0 0x0 0x1 0x1 0x0` (fs_tx=1, fs_rx=1) al nodo de filtro CCCI → loguea
`fs_callback:`/`fs_send:` (los mensajes). Y volcar `fs_buffers[idx]` (phys de la region FS del stock)
antes/después de cada op → la pareja petición→respuesta real. Con 3-4 ops del boot basta para el mount.

## Plan
1. [hecho] Extraído ccci_fsd/mdinit/libnvram + protocolo + mapeo "Z:\"→/data/nvram/md.
2. [siguiente] Ghidra del ccci_fsd (op-table + formato de respuesta) — o captura en vivo (3-4 ops).
3. Implementar en el proxy H6 (kernel) la respuesta real por op + backing-store `/data/nvram/md` (NVRAM
   real → calibración). Objetivo: MD pasa el mount → NORMAL_BOOT_ID (HS2 completo).

*Mac (Fable 5), 2026-07-20. Daemon ccci_fsd extraído + protocolo/paths mapeados. El formato de respuesta
por op = RE en Ghidra (binario en la Pi `~/modem-fsd/`) o captura en vivo. Es el sub-proyecto "port ccci_fsd".*
