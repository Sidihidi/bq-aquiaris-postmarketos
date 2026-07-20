# Módem H6d — protocolo FS crackeado con Ghidra (encoding de la respuesta) — 0720

> Decompilado el `ccci_fsd` con **Ghidra 12.1.2** (headless, JVM arm64 para el decompilador nativo).
> Cierra la última milla de [H6c]: el **encoding byte-a-byte** del request/response del proxy FS.
> Decompilaciones crudas (derivadas de binario propietario) en la Pi `~/modem-fsd/fsd-*.c`.

## Setup Ghidra (para reproducir)
`brew` trae Ghidra pero el decompilador nativo pide JVM **arm64** (si no, `os/mac_x86_64/decompile
does not exist`). Fix: `JAVA_HOME=/opt/homebrew/opt/openjdk@21/...`. Ghidra 12 usa **PyGhidra** (no Jython)
→ los postScripts van en **Java** (`.java GhidraScript`), no `.py`. El ELF carga en vaddr base `0x10000`
(dir Ghidra = dir r2 + 0x10000).

## Las 3 funciones núcleo (decompiladas)

### `FUN_0001d344` = Open handler (path + flags)
1. Convierte el path del MD (`"Z:\..."`) a path real bajo `/data/nvram/md` (via `FUN_00018da0` +
   tabla de drives + `sprintf`).
2. **Mapea los flags FS del MD → `open()` oflag** (directamente portable):
```
req_flags & 0x10000    -> O_RDWR|O_CREAT              (0x42)
req_flags & 0x20000    -> O_RDWR|O_CREAT|O_TRUNC      (0x242)
req_flags & 0x1000000  -> | O_LARGEFILE (|0x101000)
req_flags & 0x10000000 -> | 0x800
```
3. `open(real_path, oflag, 0660)`.

### `FUN_0002427c` = ensambladora de respuesta + SEND
- Buffer del índice `idx` en `base + stride*idx` (stride = `bufsize+0x14`).
- Cabecera de respuesta: `word[0]` = flags (solo low 16 bits: `*addr = ~(~(*addr<<0x10)>>0x10)`),
  `word[1]` = **count** de entradas de datos.
- Datos empaquetados como **`{u32 len; u8 data[len]}` × count**, cada uno alineado a 4
  (`(len+3)&~3`), a partir de `addr+8`.
- `local_30 = __n` (longitud total) → `ioctl(fd, 0x80044b02 = SEND, &{length, index})`.

### `FUN_000240a8` = constructor del mensaje CCCI de respuesta
Escribe los 5 words del mensaje (en `buf-0x14 .. buf-0x4`):
- **`word0` = resultado**: `req_word0 & 0x7fffffff` (éxito) vs `req_word0 | 0x80000000` (error) →
  **el bit 31 = flag de error/éxito** (param_5 = el código de resultado del file-op).
- `word1` (=data0) = `idx_buffer + 0x14` (MD-view del buffer, saltando la cabecera).
- `word4` (=reserved) = `idx` (el índice).
(= lo que el driver mete en el `FS_TX`: `data0/data1/channel=15/reserved`, ver `ccci_fs_send`.)

## Resumen del protocolo (cerrado, request→response)
```
REQUEST (buffer[idx], del MD):   [u32 fs_ops][u32 flags][u32 ...][path/params...]   (data payload @ +0x14)
  fs_ops = op (open/read/write/close/...), flags = FS_flags (ver mapeo arriba)
RESPONSE (el proxy escribe):     header{ word0: resultado (bit31=error) ; count } + {u32 len; data}×count
  SEND(fd, {length, idx}) -> el driver manda FS_TX{data0=buf+0x14, data1=length+4, ch=15, rsv=idx}
```
El file-op real es un `open/read/write/close` estándar sobre `/data/nvram/md/NVRAM/NVD_DATA/<LID>`
(ver [H6c] para la traza syscall). Éxito → bit31=0 + los datos leídos; error → bit31=1.

## Implementación del proxy (ahora determinada)
**Diseño = daemon userspace** (port de ccci_fsd), es lo natural:
1. Nuestro driver expone `/dev/ccci_fs` (ioctl GET_INDEX=0x4b01 / SEND=0x80044b02 + `mmap` de la region FS)
   replicando `ccci_fs_main.c` (ya lo tenemos de fuente).
2. El daemon: `GET_INDEX` → lee `buffer[idx]` (fs_ops+flags+path) → traduce path a `/data/nvram/md/...` +
   mapea flags → hace el file-op → escribe la respuesta (header{resultado,count} + {len;data}) → `SEND`.
3. Backing-store = la **NVRAM real** del MD (extraída) → calibración correcta.
Alternativa mínima (test HS2): el kernel-proxy H6 puede responder los primeros ops (mount "Z:\" + los
LIDs del boot) con éxito (bit31=0) y datos de los ficheros NVRAM, para ver si el MD llega a NORMAL_BOOT_ID.

## Estado
CERRADO: el encoding request/response (Ghidra) + el mapeo de flags + los file-ops + la traza syscall +
los paths NVRAM + la region FS. → **el proxy FS está completamente especificado**. Siguiente = implementar
(daemon userspace o kernel-proxy H6) + probar en HW. Decompilaciones en la Pi `~/modem-fsd/fsd-*.c`.

*Mac (Fable 5), 2026-07-20. Protocolo FS crackeado con Ghidra: encoding de respuesta (bit31=resultado,
{len;data}×count), mapeo de flags, path translation. El proxy FS queda especificado de punta a punta.*
