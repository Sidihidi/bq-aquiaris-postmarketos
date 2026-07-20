# Módem H6g — TEST HW del proxy FS en pmOS: EL PROXY RESPONDE AL MD REAL — 0720

> Primer test en hardware del servidor FS (`spm_fs_serve`). **Resultado: el proxy funciona end-to-end —
> el MD manda su petición real de mount y nosotros respondemos.** Falta afinar la respuesta del GetDrive.

## Setup del test
- Kernel #26 con el servidor FS, empaquetado con el **dtb bueno** (`extract-300/dtb-300.dtb` del
  `boot-gsync-lna.img`; el dtb del árbol restaurado de la Pi es VIEJO → bootloop, mismo gotcha que el GPS #301).
  Imagen: `~/mainline/pkg/boot-modem-fsproxy2.img`.
- **NVRAM real** copiada a `/data/nvram/md` en pmOS (55 LIDs, de `~/modem-fsd/nvram/`).
- Trigger: `echo 1 > /sys/module/mt6582_spm/parameters/{spm_md_load,spm_md_remap,spm_md_release,spm_md_hs2}`.

## Resultado (dmesg) — ★ EL PROXY RESPONDE
```
H3 load/remap/release OK
H3 v2: *** HANDSHAKE! @t=100ms *** (HS1 confirmado)
H4 HS2: runtime+tag+msg enviados. Bucle de servicio CCCI 8s...
CCCI RX ch1: d0=4160e000 id=00000020 lch=0000000e rsv=00000000     <- lch=14 = FS_RX !
H6 FS REQ idx=0 op=0000100e [00000002 00000008] path=[003a005a 0000005c 00000004]  <- "Z:\"
H6 FS RESP idx=0 -> FS_TX ch0 data0=4160e000                        <- RESPONDIMOS
CCCI RX ch2: d0=ffffffff id=00000004 lch=00000000 rsv=45584350      <- control ("EXCP"?)
CCCI RX ch3: d0=ffffffff id=00000006 lch=00000000 rsv=45524543      <- control ("EREC"?)
H4 HS2: sin boot-ready tras 8s.
```

## Análisis
- **La tubería del proxy FS funciona 100%**: HS1 → runtime → el MD pide su EFS por `FS_RX(14)` →
  parseamos la petición (`H6 FS REQ`) → respondemos por `FS_TX(15)`. Todo el camino que diseñamos, vivo.
- **★ El op de mount es `0x100e` (GetDrive), NO `0x1001`** (ese era runtime). Request: `p0=2` (type),
  `p1=8` (serial), path "Z:\". Mi `spm_fs_serve` cae en el `default` (éxito mínimo, length 0) → la
  respuesta GetDrive es INSUFICIENTE.
- Tras responder, el MD manda 2 mensajes de control (`rsv=0x45584350`="EXCP", `0x45524543`="EREC") y
  para → **la init del FS del MD falló** (respuesta GetDrive mal) → no llega a HS2.

## Lo que queda (la última milla, muy acotada)
La **respuesta correcta del GetDrive (`0x100e`)**: GetDrive devuelve el drive validado (type==
FS_DRIVE_I_SYSTEM). Hay que darle el resultado que espera (nº de drive / handle) en el payload, no
length 0. Cómo obtenerlo:
1. **Ghidra del handler GetDrive** (el string xref no resolvió; buscar la fcn por el switch de fs_ops =
   caso 0x100e, o por la constante FS_DRIVE_I_SYSTEM en un cmp), O
2. **poll en Lineage del 0x100e** (request `0000100e` + response `ffff100e`+params) durante el boot.
Luego: añadir `case 0x100e` a `spm_fs_serve` con la respuesta correcta → rebuild → reflash → retest.
El resto de ops (READ de los 55 LIDs) ya están implementados (OPEN=0x1001, READ, etc.) y se validarán
cuando el MD pase el mount.

## Estado
★★ **Hito grande: el proxy FS responde al MD real en hardware.** De "protocolo crackeado" a "servidor
FS vivo respondiendo al módem". Falta 1 cosa acotada: la respuesta del GetDrive `0x100e`. Móvil en pmOS
#26 (arranca bien; el proxy solo se dispara a mano). Artefactos: `~/modem-fsd/` + `~/mainline/pkg/
boot-modem-fsproxy2.img`. NVRAM en `/data/nvram/md` del móvil.

*Mac (Opus 4.8), 2026-07-20. TEST HW: el proxy FS responde al MD real (FS_REQ op=0x100e -> FS_RESP).
Falta la respuesta GetDrive. La tubería completa funciona en hardware.*
