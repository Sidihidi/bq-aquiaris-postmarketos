# Módem H4 — HS2 probado en HW con el fix Platform_H (0717, sesión Windows)

> **HS1 CONFIRMADO EN HW (kernel #18)** + aplicado el fix Platform_H que la captura de Lineage
> validó. **HS2 sigue sin cerrar**: el MD recibe (limpia RCHNUM) pero no avanza → rechaza el
> runtime. TX/tag/checksum descartados. Sospechoso restante = consistencia BANK4 ↔ runtime_data_base.

## ✅ HS1 confirmado end-to-end (dmesg #18)
```
MD power-on OK: PWR_STATUS=0x3f5f CON=0xd bit0=1
H3 load: modem.img 5172580 B -> 0xb8000000; w0=0xe59ff018 (vector MOLY ✓)
H3 v3 remap: BANK0(fw 0xb8000000)=514f4d39/59575553  BANK4(smem 0xb9600000) MD=43413f39/4b494745
H3 v2: CCIF init -> CON=0x1 (ARB)
H3 v2: *** HANDSHAKE! @t=100ms RCHNUM=0x00000001 ***      <- HS1 ✓ (el BANK0 era la pieza)
```
El BANK0 ROM-remap (`0x514f4d39` para des=0xB8000000) es correcto por la fórmula stock — mi
cálculo a mano previo (0x53514f39) estaba mal: los slots INVALID de BANK0 son 7-13, y con
`INVALID_ADDR=0x3E000000` el resultado es 0x514f4d39. Lineage usaba `0x53514f3d` porque su
`INVALID_ADDR` = `round_up(get_max_DRAM_size(),O)` = **0x40000000** (no el fijo 0x3E000000) y su
des=0xBC. La diferencia en los slots INVALID no bloquea (ambos son direcciones inválidas fuera del
carveout); el byte del des real (0x39 vs 0x3d) sí es el que importa y es correcto.

## ✅ Fix aplicado: Platform_H "82E1" (ground-truth Lineage)
`0x535f3238` ("82_S", del header del img) → **`0x31453238`** ("82E1", del runtime real). 2 sitios
(runtime[2] y tag+0x8). Capturado de Lineage: `Platform_L=MT65 / Platform_H=82E1`. Necesario pero
**no suficiente**.

## ❌ HS2 aún falla (dmesg #18, con Platform_H arreglado)
```
H4 HS2: pre RCHNUM=0x1 (HS1 presente)
H4 HS2: runtime+tag+msg enviados. Sondeando 5s...
H4 HS2: sin respuesta nueva tras 5s. RCHNUM=0x0 BUSY=0x0
```
El MD limpió RCHNUM (nuestro ACK del HS1 llegó) pero no emitió BootReadyID → leyó el runtime y no
lo aceptó, o no lo leyó.

## Descartado (verificado 1:1 contra bq-src)
- **CheckSum**: NUNCA se asigna en el path CCCI (queda 0 del memset). No lo valida el MD. ✅ ok a 0.
- **Formato del mensaje TX**: `__ccif_v1_write_phy_ch_data` escribe `data[0],data[1],channel,reserved`
  a `TXCHDATA[ch]` (base+0x100+ch*16), BUSY=1<<ch antes, trigger `TCHNUM=ch`. Nuestro ch=0:
  BUSY=0x1, {0xFFFFFFFF, 0, 1, MD_INIT_CHK_ID} a 0x100..0x10c, TCHNUM=0. **1:1 correcto.**
- **Offset del tag**: `__ccif_v1_write_runtime_data` escribe a `m_reg_base + 0x140`
  (=CCIF_STD_V1_RUN_TIME_DATA_OFFSET). Nuestro CCIF+0x140. **Correcto** (el "need confirm" del .h
  se confirma = 0x140).
- `platform_set_runtime_data` solo pone Platform_L/H + DriverVersion (ya cubierto).

## 🔴 Sospechoso #1 restante: consistencia BANK4 ↔ runtime_data_base (dónde LEE el MD)
Escribimos el runtime a AP `0xB9600000` (=`ioremap(MD_SMEM_PHYS)`). El tag dice
`runtime_data_base = MD_SMEM_PHYS − MD_AP_OFF = 0xB9600000 − 0x78000000 = 0x41600000` (vista-MD).
El MD lee su `0x41600000` y debe caer en AP `0xB9600000`. Pero el BANK4 mapea el banco de 16/32MB
del MD 0x40000000 con `des` = SMEM (no la base 32M-align del carveout) → **la traducción de
0x41600000 puede NO dar 0xB9600000** (posible off-by-16MB por la granularidad del banco). En el
stock BANK4 mapea con `smem_base_before_map` y `md_2_ap_offset` usa `&0xFE000000` (32M-align):
hay que verificar que MD 0x41600000 → 0xB9600000 EXACTO con nuestros valores, o el MD lee basura.
**Test siguiente**: escribir el runtime en la dirección AP que corresponde a la vista-MD real del
banco (o ajustar `des` del BANK4 a la base 32M-align 0xB8000000 en vez de 0xB9600000, de modo que
MD 0x40000000→0xB8000000 y MD 0x41600000→0xB9600000). Confirmar leyendo de vuelta lo escrito por
el MD, o instrumentar el read del MD.

## 🟡 Sospechoso #2: regiones ShareMem de tamaño no-cero (Exce/MDExExpInfo) + support_mask
Si el #1 se descarta, añadir Exce (runtime campos 35/36, size 0x800) y `support_mask` del
misc_info (FEATURE_SUPPORT<<MISC_DMA_ADDR). Ver [H4-HS2-REVIEW-0717.md](H4-HS2-REVIEW-0717.md).

## Estado
HS1 = **HECHO y confirmado en HW** (primer MT6582 con el MD arrancando en mainline). HS2 = iteración
en curso (mecanismo correcto; falta que el MD acepte el runtime). Driver: `mt6582-spm-H1.c` (snapshot
con el fix). Kernel de test #18 (`boot-modem-hs2.img`). Backup `mt6582-spm.c.pre-platformH-0717`.

*2026-07-17, sesión Windows (Fable 5). HS1 validado; Platform_H de la captura Lineage; HS2 pendiente
del sospechoso BANK4/runtime_data_base.*
