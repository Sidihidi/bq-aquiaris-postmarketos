# Módem H6f — op-codes FS del MD + plan de implementación del proxy — 0720

> Camino A (proxy propio). Capturados los **op-codes reales** que el MD manda, con un poller de
> `/dev/ccci_fs` (mmap de la misma shared-mem) + strace correlados por timestamp. Cierra el dispatch.

## Método
`fsops_poll.c` (ARM estático, `~/modem-fsd/`): abre `/dev/ccci_fs`, `mmap` de los 5 buffers
(`sizeof(fs_stream_buffer_t)*5`), bucle que loguea cada transición de `fs_ops` (word0) por buffer, con
timestamp. Corrido a la vez que `strace` del `ccci_fsd` durante un toggle de airplane (genera ops) →
correlación op-code ↔ syscall. (El boot-poll vía wrapper es frágil: satura CPU / race con el device;
innecesario — ver abajo.)

## ★ Op-codes (request/response)
- **request**: `fs_ops` high16 = `0x0000`; **response**: high16 = `0xffff` (el daemon marca "hecho":
  `word0 = ~(~(op<<16)>>16)` = conserva low16, pone high16=0xffff — el `FUN_0002427c` de [H6d]).
- op real = low 16 bits. Params en los words siguientes del buffer (p0=+4, p1=+8, p2=+0xc, p3=+0x10).

| op (low16) | tipo | evidencia |
|---|---|---|
| **0x1001** | **OPEN** | p2=`003a005a`("Z:")+p3=`005c004e`("\N") = path UTF-16LE; p1=`0x36`=len path; p0=`2`=flags |
| **0x1004** | **WRITE** | p3 = nº bytes: `6, 2, 0x7d0(2000), 0x1ffe(8190)` — casan EXACTO con los `write(7,N)` del strace |
| 0x1009 | GETFILESIZE/STAT (prob) | entre OPEN y WRITE; usa `fstat` (no aparece como syscall filtrado) |
| 0x1005 | CLOSE/SEEK (prob) | tras los WRITEs; p3=4 |
| 0x101c | metadata (mount) | idx=1, p0=1 p1=4 p2=0 |
| READ | (por confirmar) | el airplane hace WRITEs; el boot hace READs. Enum probable: **0x1003** |

**Cómo se confirmarán los que falten SIN más captura en Lineage**: nuestro **kernel-proxy H6 ya loguea
`H6 FS REQ idx=%u op=%08x [..] path=[..]`** por cada request del MD (mt6582-spm-H1.c). Al correr el
bring-up en pmOS, el dmesg revela los op-codes exactos que el MD manda (READ + los 42 de mount) → se
completa el switch iterando.

## Plan de implementación (kernel-proxy H6, sobre `spm_md_hs2`)
Estado servidor FS mínimo (los ops del boot son OPEN/READ/CLOSE + los 42 metadata; el boot LEE la NVRAM):
1. **Tabla de handles**: `struct file *fs_h[16];` (idx → fichero abierto).
2. Por cada `FS_RX` (lch=14), leer `buffer[idx]` (fs_ops+params), dispatch por `fs_ops & 0xffff`:
   - **OPEN (0x1001)**: parsear path UTF-16 "Z:\..." → `/data/nvram/md/...` (backslash→slash), `filp_open`
     con oflag del mapeo de [H6d] (`0x10000`→O_RDWR|O_CREAT...), guardar handle, responder handle.
   - **READ**: `kernel_read(fs_h[h], buf, len)` → escribir `{u32 len; data}` en el buffer, responder.
   - **WRITE (0x1004)**: `kernel_write`.
   - **CLOSE**: `filp_close`.
   - **GETFILESIZE (0x1009)**: `vfs_getattr` → tamaño.
   - **metadata/mount (0x101c, GetDrive)**: responder éxito mínimo.
3. **Respuesta** (de [H6d]/[H6e]): word0 = `0xffff0000 | (op & 0xffff)` (o el `& 0x7fffffff` del msg CCCI),
   datos `{u32 len; data}` desde `buffer+8`; `SEND` = FS_TX{data0=`fs_phys - MD_AP_OFF + 0x4004*idx`,
   data1=`length+4`, ch=15, rsv=idx} (fix del stride ya aplicado: `0x4004`).
4. **Backing-store**: la NVRAM real extraída (`~/modem-fsd/nvram/md`, 55 LIDs) → copiar a
   `/data/nvram/md` en pmOS.

## Estado — servidor FS IMPLEMENTADO y COMPILANDO
- `spm_fs_serve()` + `spm_fs_path()` escritos en `mt6582-spm-H1.c`; el bucle de `spm_md_hs2` los llama.
- **COMPILA contra el kernel real** (`mt6582-spm.o` RC=0). Fix necesario: `strlcpy`→`strscpy` (strlcpy
  removido en kernels modernos). El árbol de la Pi (`drivers/soc/mediatek/mt6582-spm.c`) estaba MÁS VIEJO
  que el wip del repo (sin la región `fs` mapeada en spm_md_hs2) → actualizado con el wip (backup en
  `/tmp/tree-spm-backup.c` de la Pi). ⚠️ el árbol de la Pi NO es git → el repo wip es la fuente de verdad.
- Op-codes OPEN=0x1001 / WRITE=0x1004 confirmados; READ/CLOSE/GETSIZE/mount se completan del log del
  propio proxy (`H6 FS REQ op=%08x`) en pmOS.

### Pasos para el test HW (siguiente sesión, con el móvil en pmOS)
1. Copiar la NVRAM real (`~/modem-fsd/nvram/md`, 55 LIDs) → `/data/nvram/md` en pmOS.
2. Flashear el boot con este kernel (build-krillin/zImage) — OJO: la config tiene el firmware ROMv1 del
   GPS; para el modem da igual (el spm es built-in). 
3. `spm_md_load` → `spm_md_remap` → `spm_md_release` → `spm_md_hs2` (params del driver).
4. Leer el dmesg: `H6 FS REQ op=%08x` revela los op-codes que faltan (READ/mount) → completar el `switch`
   de `spm_fs_serve` → re-build → repetir hasta `NORMAL_BOOT_ID` (HS2 completo).

Herramientas y capturas: `~/modem-fsd/` (poller, strace, decompilaciones, nvram/). Móvil restaurado limpio
(LineageOS). Commits hasta `d2a8f4f`.

*Mac (Opus 4.8), 2026-07-20. Op-codes del MD capturados (poller /dev/ccci_fs): OPEN=0x1001, WRITE=0x1004
+ encoding response ffff. El proxy revela el resto en su propio log. Camino A desbloqueado.*
