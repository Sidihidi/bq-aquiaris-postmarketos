# Módem H6c — Snoop EN VIVO del ccci_fsd en LineageOS (protocolo FS a nivel syscall) — 0720

> Captura del `ccci_fsd` con `strace` durante actividad del MD (airplane toggle) en LineageOS. Da el
> protocolo FS a nivel de syscall EXACTO + la región FS física + la lista de ficheros NVRAM. Complementa
> el RE del binario [H6b] y la spec [H6]. Capturas crudas en la Pi `~/modem-fsd/*.strace`.

## Setup del snoop
Lineage arrancado (dd del boot desde pmOS). `adb root`. `ccci_fsd` PID 144, `strace` en `/system/xbin`.
```
strace -f -tt -s 256 -e trace=open,openat,read,write,lseek,ioctl,close -p 144 -o fsd.strace
+ toggle airplane_mode (settings put global airplane_mode_on 1/0)  # fuerza recarga NVRAM del MD
```

## ★ La región FS (de `/proc/144/maps`)
```
b6ced000-b6d02000 rw-s bd6c0000 /dev/ccci_fs      ← VIRT daemon 0xb6ced000, PHYS Lineage 0xbd6c0000, size 0x15000
```
= 5 buffers × 16388B (`fs_stream_buffer_t`), confirma el `FileShareMemSize=0x14014`. `/dev/mem` en Lineage
está bloqueado (STRICT_DEVMEM) y `/proc/pid/mem` necesita ptrace → el contenido byte-a-byte del buffer
NO se pudo volcar con dd (para eso: Ghidra del daemon, o un helper mmap de `/dev/ccci_fs`).

## ★ El protocolo FS a nivel de syscall (CICLO por op)
```
ioctl(4, _IO('K',1))              = index      # GET_INDEX: bloquea hasta que el MD manda (fd4=/dev/ccci_fs)
write(5, "ccci_fsd", 8)                         # acquire_wake_lock (fd5=/sys/power/wake_lock)
openat("/data/nvram/md/NVRAM/NVD_DATA/MT0X_000", O_RDWR|O_CREAT|O_LARGEFILE, 0660) = 7   # el file-op
write(7, <buf+0x14>, N)   /  read(7, ...)       # datos (WRITE: 2000/8190/2 B; el payload va en buffer+0x10)
ioctl(4, _IOR('K',2,u32), &{length,index})      # SEND: la respuesta (data1=length+4)
write(6, "ccci_fsd", 8)                         # release_wake_lock (fd6=/sys/power/wake_unlock)
close(7)
```
- **fd 4** = `/dev/ccci_fs` (GET_INDEX / SEND).  **fd 5/6** = wake_lock/unlock (nombre "ccci_fsd").
  **fd 7** = el fichero NVRAM (por-op, `O_RDWR|O_CREAT` → si falta, se crea vacío).
- El daemon abre `O_RDWR|O_CREAT`, hace el op (read/write con offset via lseek), responde el `length`.
- **El payload de datos empieza en buffer+0x14** (= `fs_ops`[4B] + 16B de cabecera de params). O sea el
  request/response = `[u32 fs_ops][16B params: handle/offset/length...][datos]`.

## ★ Ficheros NVRAM que toca el MD (backing-store del proxy)
`/data/nvram/md/NVRAM/NVD_DATA/<LID>_<NNN>` — capturados en el toggle (RF-cal):
```
MT0X_000    UM06_003    MT1A_008      (≈16 ops en un toggle)
```
Son los LIDs de NVRAM del modem (calibración RF, config). El directorio raíz `"Z:\"` → `/data/nvram/md`
(de [H6b]). El boot completo del MD lee/crea más LIDs (no capturados aquí: el toggle no re-monta "Z:\";
para el set completo del boot → strace desde un MD-reset o phone-reboot).

## Lo que ESTO resuelve y lo que queda
RESUELTO: el protocolo syscall (cómo el daemon sirve cada op), la región FS física, el layout
`[fs_ops][params 16B][data@+0x14]`, los paths NVRAM, el ciclo GET_INDEX→file-op→SEND con wakelocks.
QUEDA: el **layout byte-a-byte de los 16B de params por fs_op** (handle/offset/length/mode) + la
respuesta del op de mount `"Z:\"` (fs_ops=0x100e). → **Ghidra del `ccci_fsd`** (`~/modem-fsd/`, en la Pi):
la función que parsea `buffer[]` por op da los offsets exactos. Con eso + este snoop, el proxy (userspace
port de ccci_fsd, o el kernel H6) queda determinado.

## Recomendación de diseño (revisada tras el snoop)
El proxy es **naturalmente un daemon userspace** (ccci_fsd LO ES): nuestro driver expone `/dev/ccci_fs`
(GET_INDEX/SEND/mmap, replicando `ccci_fs_main.c`) y un daemon hace los file-ops sobre `/data/nvram/md`.
Más limpio que hacer file-I/O en el kernel (el H6 kernel-proxy sirve para el mount mínimo/test).
Backing-store = la **NVRAM real del MD** (ya extraída) → calibración correcta = señal.

*Mac (Fable 5), 2026-07-20. Snoop en vivo del ccci_fsd: protocolo syscall + región FS (0xbd6c0000) +
paths NVRAM capturados. Falta el layout de params por op (Ghidra). Capturas en la Pi `~/modem-fsd/`.*
