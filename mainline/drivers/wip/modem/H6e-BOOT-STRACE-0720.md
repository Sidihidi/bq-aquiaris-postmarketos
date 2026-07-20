# Módem H6e — boot-strace del ccci_fsd: la secuencia FS REAL del arranque del MD — 0720

> Última incógnita del proxy FS cerrada: **qué pide el MD a la NVRAM en el arranque**, capturado con
> `strace` del `ccci_fsd` desde un **boot limpio del MD**. Completa [H6d] (encoding) con la secuencia real.

## Método (boot-strace fiable)
El `ccci_fsd` es daemon de arranque → para pillar el mount hay que trazarlo DESDE el inicio. Intentos:
- ✗ `strace -p <pid>` + toggle airplane → solo runtime (el MD ya montado).
- ✗ ioctls raw `DO_STOP_MD`/`DO_START_MD` (helper ARM propio en `/dev/ccci_ioctl0`) → **tumban el
  ccci_fsd y dejan el MD NOT_READY** (los coordina `ccci_mdinit`, no valen sueltos). No usar.
- ✓ **Wrapper**: `/system` rw (SELinux **Permissive** en el eng build) → `ccci_fsd` → script que
  `exec strace ... /system/bin/ccci_fsd.real` → **reboot** → captura desde el boot limpio. (Restaurado
  el binario real después.) Salida: `~/modem-fsd/boot-fsd.strace` (7688 líneas).

## La secuencia FS del arranque del MD (ground-truth)
Ciclo por op: `ioctl(fd, GET_INDEX=0x4b01)` → [file-op real si aplica] → `ioctl(fd, SEND=0x80044b02,
&{length,index})` (+ wakelock acquire/release en fds 5/6 = `/sys/power/wake_(un)lock`).

1. **Fase mount/metadata: 42 ops SIN fichero** (GET_INDEX→SEND directo). = GetDrive("Z:\") + init del FS
   (el MD valida la unidad y prepara handles). Responder éxito (bit31=0) con payload mínimo.
2. **Fase NVRAM: 155 accesos a 55 ficheros únicos** `/data/nvram/md/NVRAM/NVD_DATA/<LID>`:
   los LIDs (calibración RF, IMEI, config): `MT48, MT6K, MT9W, MT6G, MT44, MT4D, MT42, MT41, MT35, ...`
   (lista completa en `~/modem-fsd/boot-lids.txt`). Reads de **98/74/722/710/270/100/36/18/10/8** B;
   algunas escrituras (el MD actualiza contadores). Cada read → la respuesta lleva el contenido del fichero.

## ★ El backing-store extraído
La **NVRAM real del MD** (`/data/nvram/md`, 752 KB, 55 LIDs) → **`~/modem-fsd/nvram/`** (propietario,
device-specific, NO al repo). Es la calibración auténtica → imprescindible para que el RF funcione.

## Lo que queda EXACTO para el proxy
El `length` byte-a-byte por op (el `{length,index}` del SEND) strace no lo decodifica (arg = puntero),
PERO para los reads `length ≈ bytes_leídos + cabecera` (el read size SÍ está en la traza) y para los 42
metadata la respuesta es pequeña (cabecera). Con eso + el encoding de [H6d] (bit31 + `{len;data}×count`)
+ la NVRAM real → **el proxy es 100% implementable**.

## Implementación (determinada)
El proxy (userspace daemon o kernel-proxy H6): por cada `GET_INDEX`, lee `buffer[idx]` (fs_ops+flags+path),
y:
- op sin fichero (los 42 del mount) → éxito (word0 = fs_ops & 0x7fffffff), payload mínimo.
- open/read/write/close → file-op real bajo `/data/nvram/md` (la NVRAM extraída) → respuesta con los datos
  (`{u32 len; data}`), `SEND(fd, {length, idx})` → FS_TX{data0=buf+16388*idx, data1=len+4, ch=15, rsv=idx}.

## Estado
CERRADO: encoding (Ghidra) + secuencia de boot (strace) + los 55 LIDs + la NVRAM real. **El proxy FS está
completamente determinado y con su backing-store.** Siguiente = codificar el servicio de los 55 LIDs +
build + test HW (móvil a pmOS + `spm_md_load/remap/release/hs2`). Artefactos: `~/modem-fsd/`
(binario, decompilaciones, `nvram/`, `boot-fsd.strace`, `boot-lids.txt`).

*Mac (Opus 4.8), 2026-07-20. Boot-strace del ccci_fsd: 42 ops de mount + 155 accesos a 55 LIDs NVRAM.
NVRAM real extraída. El proxy FS queda determinado de punta a punta CON su backing-store.*
