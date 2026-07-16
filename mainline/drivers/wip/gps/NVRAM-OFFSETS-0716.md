# NVRAM / PROTECT / SECCFG — offsets calibrados + estado (0716)

Contexto: el usuario borró el NVRAM por accidente en un flasheo con SP Flash Tool (perdió IMEI/serial).
Objetivo: restaurar el NVRAM de fábrica. Bonus: el crystal-trim RF vive ahí → podría ayudar al GPS.

## Offsets VERIFICADOS en pmOS (`/dev/mmcblk0`)
**Fórmula confirmada: `offset_pmOS = physical_start_addr(scatter) + 0xB80000`** (igual que el `wo` de BROM).
Calibrado con la firma `AND_SECCFG_v`, que aparece exactamente en 0x2880000 (= physical 0x1d00000 + 0xB80000). ✓

| Partición | physical (scatter) | offset pmOS mmcblk0 | sector (bs=512) | size |
|---|---|---|---|---|
| PRO_INFO  | 0x100000  | 0xC80000  | 25600 | 0x300000 (3MB) |
| **NVRAM** | 0x400000  | **0xF80000**  | **31232** | 0x500000 (5MB) |
| **PROTECT_F** | 0x900000 | **0x1480000** | **42496** | 0xA00000 (10MB) |
| **PROTECT_S** | 0x1300000 | **0x1E80000** | **62464** | 0xA00000 (10MB) |
| **SECCFG** | 0x1d00000 | **0x2880000** | 82944 | (firma AND_SECCFG_v ✓) |

⚠️ Leer NVRAM en 0x400000 crudo (sin el +0xB80000) da ceros — ese fue el error inicial. Y **SP Flash Tool
restaurando NVRAM borra el SECCFG a ceros → bootloop "logo→apaga"** (brick del 0713). Si se restaura, hacerlo
por `dd`/`wo` con estos offsets y **NO tocar el seccfg** (o restaurarlo golden UNLOCK después).

## ★ HALLAZGO CLAVE: el golden NO sirve para restaurar el NVRAM
Comparado el contenido (bytes NO-cero) del móvil actual vs el backup golden (`~/golden/`):

| | NVRAM (5MB) | PROTECT_F (10MB) | PROTECT_S (10MB) |
|---|---|---|---|
| **Móvil actual** | **3.037 B** (≈vacío) | 1.052.372 B (~1MB, con datos) | 11.554 B |
| **Golden backup** | 694 B (≈vacío) | 12.743 B (≈vacío) | 12.297 B |

→ **El golden NVRAM está tan vacío como el del móvil** (694 vs 3037 bytes de 5MB) — el backup golden se tomó
**DESPUÉS** del borrado, o nunca capturó el NVRAM de fábrica. **Restaurar el golden NO recuperaría el IMEI**
(no está ahí). Además el golden PROTECT_F (12KB) es MUCHO menor que el del móvil (1MB) → **flashear el golden
BORRARÍA datos que el móvil aún tiene** (regresión). **NO restaurar el NVRAM/protect desde el golden.**

El firmware stock 1.5.2 de red de seguridad tampoco trae nvram/proinfo/protect (ver [[reference-mtkclient-krillin]]).

## Conclusión / opciones para el IMEI
- El IMEI de fábrica **no está en ningún backup que tengamos** → recuperarlo requiere: (a) el NVRAM original
  (si el usuario lo tiene guardado de otra sesión/otro backup con los IMEIs), o (b) reescribir IMEI con
  herramienta MTK (SN Writer / maui META) — proceso aparte, arriesgado. **Por ahora: NO tocar** (el móvil
  arranca y funciona; restaurar mal = brick del seccfg otra vez).
- **PROTECT_F del móvil tiene 1MB de datos reales** — conviene hacerle un backup ANTES de cualquier
  restauración: `dd if=/dev/mmcblk0 bs=512 skip=42496 count=20480 of=/backup/protect_f-actual.img`.

## Relación con el GPS (crystal-trim RF)
El crystal-trim que afina el TCXO del CONSYS vive en el NVRAM **WIFI** runtime (`/data/nvram/APCFG/APRDEB/WIFI`,
byte 0x6D), NO en la partición NVRAM cruda. En pmOS está VACÍO. Se puede sacar de LineageOS (que lo regeneró):
`adb pull /data/nvram/APCFG/APRDEB/WIFI` → aplicar el `crystal_triming_set`. (Candidato GPS #1; dudoso para el
ruido 0xCA que es de amplitud, no frecuencia, pero barato de probar.)

*Mac (Fable), 2026-07-16. Offsets NVRAM/protect/seccfg calibrados y verificados en pmOS. Golden NO sirve para
restaurar (está vacío). NO restaurar sin una fuente real del NVRAM de fábrica + backup previo del protect_f.*
