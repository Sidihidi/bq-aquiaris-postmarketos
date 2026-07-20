# Op-codes FS del MD (poller /dev/ccci_fs + strace correlados) — 0720

request: high16 = 0x0000 ; response: high16 = 0xffff (el daemon marca "hecho": `~(~(op<<16)>>16)`)
op = low 16 bits. Params en p0/p1/p2/p3 (words tras fs_ops en el buffer).

| op | tipo | evidencia |
|----|------|-----------|
| 0x1001 | **OPEN**  | p2=003a005a("Z:")+p3=005c004e("\N") = path UTF-16; p1=0x36=len path, p0=2=flags |
| 0x1004 | **WRITE** | p3 = nº bytes (6,2,0x7d0=2000,0x1ffe=8190) casa EXACTO con write(7,N) del strace |
| 0x1009 | GETFILESIZE/STAT | entre open y write; usa fstat (no sale como syscall en el filtro) |
| 0x1005 | CLOSE (prob) | tras los writes, antes del close(7) del strace |
| 0x101c | ? (idx=1, metadata) | p0=1 p1=4 p2=0 |
| READ   | (pendiente) | el airplane hace WRITES; el boot hace READS -> boot-poll |

Nota: el boot hace 42 ops de metadata (mount/GetDrive) + reads. Op-codes del boot = boot-poll throttled.
