# Módem H4 — review del HS2 del Mac (por qué "el MD no avanza") — 0717 (sesión Windows)

> Revisión del `spm_md_hs2` (commit 55a7166) contra el source stock. **El mecanismo está
> BIEN** (offsets del runtime, TAG en CCIF+0x140, MD_AP_OFF=0x78000000, TX del CCIF, poll)
> — todo verificado 1:1 contra bq-src. Hay **un bug de valor claro** + 2 sospechas.

## 🔴 BUG confirmado: `Platform_H` incorrecto (en runtime[2] Y en el tag)

El Mac escribe `0x535f3238` (= "82_S"), que sale de **"MT6582_S00"** — pero ése es el
string del HEADER del img (`ccci_get_platform_ver` / check_md_header), NO el del runtime.

El runtime usa **`CCCI_PLATFORM = "MT6582E1"`** (`ccci_platform_cfg.h:16`,
`platform_set_runtime_data`: `snprintf(str,"%s",CCCI_PLATFORM); Platform_L=*(int*)str;
Platform_H=*(int*)&str[4]`). Con "MT6582E1":
```
Platform_L = "MT65" = 0x3536544D   ✅ (el Mac lo tiene bien)
Platform_H = "82E1" = 0x31453238   ❌ el Mac puso 0x535f3238 ("82_S")
```
**Fix (2 sitios):**
```c
writel(0x31453238, smem + 2 * 4);       /* runtime Platform_H "82E1" */
writel(0x31453238, ccif + 0x140 + 8);   /* tag platform_H "82E1"     */
```
El MD valida el platform del runtime; "82_S" ≠ "82E1" → lo más probable es que lo rechace.

## 🟡 Sospecha 2: faltan regiones de tamaño no-cero (el MD puede exigirlas)

`set_md_runtime` puebla cada ShareMem cuyo `smem_table->..._size != 0`. Con "share-mems a 0"
faltan al menos las de tamaño constante no-cero. La más probable de exigir:
- **Exce** (excepción): runtime **campos 35 (Base) / 36 (Size)**, size = `MD_EX_LOG_SIZE = 0x800`.
- **MDExExpInfo**: campos **62 / 63**, size = `CCCI_MD_EX_EXP_INFO_SMEM_SIZE`.

Colocarlas en el SMEM (tras el runtime, como MiscInfo) y apuntar `Base = phys - MD_AP_OFF`.
Layout del stock (ccci_settings.c, secuencial desde la base del SMEM):
`runtime(280) → Exce(0x800) → MDExExpInfo → Misc(0x400) → ...`.
(Nota: como el Mac apunta MiscInfoBase a donde escribe, el offset absoluto da igual; solo
importa que Base coincida con dónde se escribe cada bloque.)

## 🟡 Sospecha 3: `support_mask` de misc_info a 0

`config_misc_info` del stock pone:
```c
support_mask |= (FEATURE_SUPPORT << MISC_DMA_ADDR);          // DMA addr soportado
support_mask |= (FEATURE_NOT_SUPPORT << (MISC_32K_LESS*2));  // 32k-less OFF
```
El Mac lo dejó a 0. Sacar los enums `FEATURE_*`/`MISC_*` del header y poner el mismo valor.
`feature_0_val[0] = MD_MEM_PHYS` ✅ (el Mac lo tiene bien).

## ✅ Lo que está BIEN (verificado contra source, NO tocar)
- TAG en `CCIF + 0x140` (= `CCIF_STD_V1_RUN_TIME_DATA_OFFSET`, "need confirm" en el .h pero
  es el valor real). 7 ints (`modem_runtime_info_tag_t`): prefix, platform_L/H, driver_version,
  runtime_data_base, runtime_data_size, postfix. ✅
- `MD_AP_OFF = 0x78000000` = `(smem_base & 0xFE000000) - 0x40000000` con smem@0xb8xxxxxx. ✅
- runtime_data_base/size del tag (`MD_SMEM_PHYS - MD_AP_OFF`, `280`). ✅
- Prefix/Postfix "CCIF" (0x46494343), DriverVersion 0x20121001, BootChannel 0, BootingStartID 0. ✅
- Mensaje CCIF TX: `{0xFFFFFFFF, 0(MD_INIT_START_BOOT), 1(CCCI_CONTROL_TX), 0x5555FFFF(CHK_ID)}`
  a TXCHDATA ch0 + TCHNUM=0. ✅ (BUSY, orden data0..3, trigger — todo 1:1).
- ACK del HS1 (RCHNUM bit0 → CCIF_ACK=1). ✅

## Orden de prueba sugerido
1. Solo el fix de **Platform_H** (2 líneas) — cambio mínimo, alta probabilidad.
2. Si no, añadir **Exce (35/36)** + **support_mask**.
3. Si sigue: capturar el runtime real de Lineage (bloqueado hoy: sin /dev/mem, dump DBG
   gateado por `ccci_msg_mask` y el ring-buffer rota rápido por el spam de freqhopping;
   necesitaría reset del MD con debug on — nodo `-l=0 0x20111111 0xFFFFFFFF` de
   `register_filter_func`, pendiente de localizar su path en Android).

Receta completa del runtime en [H3-HS2-RECETA-0717.md](H3-HS2-RECETA-0717.md).

*Review 0717, sesión Windows (Fable 5), contra bq-src dual_ccci. Mecanismo del Mac validado;
el bug de Platform_H es el candidato #1.*
