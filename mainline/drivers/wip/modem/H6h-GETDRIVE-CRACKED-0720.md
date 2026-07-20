# Módem H6h — ★ GetDrive (0x100e) CRACKED + funcionando: el MD avanza el mount (0720)

> Cierra el bloqueo de [H6g] (GetDrive daba length 0 → EXCP). **Capturé el ground-truth REAL del
> GetDrive del `ccci_fsd` de Lineage en boot FRÍO, lo implementé, y el MD AVANZA** al siguiente op del
> mount. Método nuevo de captura (wrapper+poller) que da lo que el strace no puede: el contenido del buffer.

## El método (novedad): captura del buffer FS en boot frío
El GetDrive es evento de boot-frío del MD (~2.5s), antes de adb (~16s), y el strace del `ccci_fsd`
muestra syscalls, NO el contenido del buffer mmap. Solución:
1. **Poller pasivo** (`tools-fsresp.c`, static ARM): mmap READ de `/dev/ccci_fs`, detecta cuando un
   buffer pasa a respuesta (op high16=0xffff) y vuelca 24 words. Coexiste con `ccci_fsd`.
2. **Wrapper de boot** (`tools-ccci_fsd-wrap.sh`): renombra `ccci_fsd`→`.real`, y el nuevo `ccci_fsd`
   lanza el poller + `exec ccci_fsd.real`. Init lo dispara al boot → captura el mount frío.
   ⚠️ GOTCHAs: el reboot resetea SELinux a **enforcing** → `fsresp` debe ir en `/system/bin` (no
   `/data/local/tmp`, que el dominio ccci_fsd no puede ejecutar) y el output a `/data/nvram/` (escribible
   por el dominio radio). Con eso, capturado limpio.

## Ground-truth capturado (respuestas del ccci_fsd de Lineage, boot frío)
```
REQ  op=0000100e p0=2 p1=8 path="Z:\"
RESP op=ffff100e | 02 04 00 54 [resto 0]         <- GetDrive: +4=2 +8=4 +0xc=0 +0x10=0x54, len=16
REQ  op=00001010 ...
RESP op=ffff1010 | 02 36 [path "Z:\NVRAM\..."]   <- devuelve path+attr (len 0x36=54)
RESP op=ffff1005 | 02 36 [path] ... 04 0100 14    <- (CLOSE/stat)
RESP op=ffff1002 | 02 04 01 04 24 04 01 [path]    <- 
RESP op=ffff1005 | 03 36 [path "Z:\NVRAM\NVD_DATA\MT.." LIDs] ... 0100 14   <- reads de los 55 LIDs
```
(El `gd.out` COMPLETO con los 42 ops está en Lineage `/data/nvram/gd.out` — p7 es F2FS/cifrado, no
montable desde pmOS; recuperable con `adb pull` en un boot de Lineage.)

## Implementado + TEST HW (kernel #27, pmOS, NVRAM real en /data/nvram/md)
`case 0x100e` en `spm_fs_serve` (mt6582-spm-H1.c): escribe +4=2 +8=4 +0xc=0 +0x10=0x54, length=16.
```
H3 HANDSHAKE OK (HS1)
H6 FS REQ idx=0 op=0000100e path=Z:\  -> H6 FS GetDrive Z:\ -> ground-truth [02 04 00 54] len=16
H6 FS RESP -> FS_TX
CCCI RX ch2: op=00001010 path=[Z:\F...]   ★ EL MD AVANZA al SIGUIENTE op del mount
CCCI RX ch3/ch4: EXCP/EREC                 (peta en 0x1010: el default sigue insuficiente)
```
**★ Confirmado: GetDrive correcto → el MD pasa del 1er op del mount al 2º (0x1010).** El pipeline FS
avanza op a op. Antes petaba EN GetDrive; ahora pasa GetDrive y pide 0x1010.

## Lo que queda: implementar los ops restantes del mount (0x1010, 0x1005, 0x1002)
Cada op del mount necesita su respuesta (depende del path/args). Vías:
- Ghidra de cada handler en `ccci_fsd` (el Mac ya tiene la infra), O
- el `gd.out` completo (recuperar de Lineage) = las respuestas byte-a-byte de los 42 ops.
Mejor: como `ccci_fsd` hace file-ops REALES sobre `/data/nvram/md`, implementar 0x1010/1005/1002 como
las operaciones de fichero correspondientes (GetAttr/Close/Open) sobre el backing, no respuestas fijas.

## Estado
★ GetDrive cracked y funcionando en HW; el MD avanza el mount. Fix en el `mt6582-spm-H1.c` compartido
(`case 0x100e`, aditivo). Tools de captura en el repo. Móvil en pmOS #27. NVRAM real intacta en
`/data/nvram/md`. Siguiente = ops 0x1010/1005/1002 del mount (Mac tiene la RE; el gd.out completo está
en Lineage).

*2026-07-20, sesión Windows (Fable 5). GetDrive (0x100e) cracked via boot-wrapper+poller + implementado;
el MD avanza el mount FS. Falta el resto de ops del mount.*
