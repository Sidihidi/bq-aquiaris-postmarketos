# HANDOFF — Análisis del GATE del cifrado (decompilado) + procedimiento fwdump REFINADO (Mac, 2026-07-01)

Complementa `HANDOFF-TXENCRYPT-GATE-0630.md` (sesión Windows). Verifiqué su RE contra el decompilado del Mac
(`mainline/wifi-consys/fw-analysis/fw_decompiled.c`) **y** el de Windows (`fw-analysis/fw_all_labeled.c`,
`caller_decomp.c`, `callers.txt`): **coinciden byte a byte**. Aquí quedan el mecanismo confirmado, las
direcciones PINCHADAS y el procedimiento fwdump corregido para la Fase 0.

## Mecanismo confirmado (2 gates en una struct runtime del FW)
- **`[0x12e3]`** — puerto/privacidad. Lo lee **`f002a4cc`** (port control): BROADCAST (STA=0, frame[0xd]==0) →
  devuelve `[0x12e3]`; UNICAST → `STA+0x289` (solo TKIP) o fallback `[0x12e3]`. Y **`f004bb2c`** (arm TX enc)
  pone el bit tx-encrypt de la WTBL **solo si `algorithm==0x41 (CCMP) && [0xf7c]==0 && [0x12e3]!=0`**
  (`fw_all_labeled.c` ~L227-230 / `f004bb2c`). → con `[0x12e3]==0` la PTK entra en la WTBL **SIN cifrado**.
- **`[0x12f5]`** — cifrado de grupo/broadcast. Setter = **`f0034610(enable,which)`**: `f0034610(1,0)` →
  `[0x12f5]=1`; `f0034610(1,1)` → `[0x1aa1]=1`. Lo LEEN los TX builders de `txm.c` (`fw_all_labeled.c`
  ~L13990/14133) para decidir si el broadcast se cifra con la GTK.
- Ambos flags cuelgan de una **base runtime** (valor ≈ `0x020a0064`, puntero guardado en `0x020a0098`; el
  BSS/STA array vive en `0x020a1000` vía el puntero de `0x020a0068`). base+`0x12e3` ≈ `0x020a1347` =
  BSS(`0x020a1000`)+0x347 → **CONVERGE con el "+0x347" del handoff de casa**.

## Lo que el decompilado NO cierra (→ empírico, Fase 0)
- `f0034610` se invoca por **COMPUTED_CALL** desde `f0026470` (fn de "BSS state==3 setup"; ver `callers.txt`).
  Ghidra no resuelve el destino del call → **ambiguo** si el trigger real es el `ADD_REMOVE_KEY(GTK)` o el
  `SET_BSS_INFO`/JoinComplete. **El fwdump lo resuelve** (leer `[0x12f5]` antes/después de cada uno).
- El handler de `SET_BSS_INFO` (`f002222c`) que escribe `[0x12e3]` por memcpy de bloque NO es decompilable
  (capstone sin soporte nds32) → por eso `[0x12e3]` tiene "3 lecturas, 0 escrituras" por símbolo.

## Fix refinado (respecto al Plan B de casa)
Hacen falta **LOS DOS** flags a 1: `[0x12e3]` (puerto unicast + fallback broadcast) **Y** `[0x12f5]` (cifrado
broadcast). El `enc_status=ENABLED` (probado→refutado en la tabla de hipótesis) probablemente ponía `[0x12e3]`
pero **NO `[0x12f5]`** → el DISCOVER broadcast seguía sin cifrar → sin OFFER. **Plan B ataca `[0x12f5]` vía la
GTK.** Recordatorio: el re-envío de `SET_BSS_INFO(0x16)` cuelga por `f004aac8` (re-init beacon/TSF en caliente),
NO por el `enc_status`.

## Procedimiento fwdump — Fase 0 (CORREGIDO)
Conectar WPA2 a "hola" (receta del `HANDOFF-CASA-0630.md`) y en la ventana de ~30s (antes del beacon-timeout
`0x1b`):
1. **Validar fwdump**: `echo "f0020000 4" > fwdump_cfg; cat fwdump` → valores VARIABLES por dirección = OK.
   Si salen todos iguales → la lectura no va; reflashear el driver (versión que lee bien `0x020axxxx`).
2. **Resolver la base**: leer 1 palabra de `0x020a0098`, `0x020a0064` y `0x020a0068` → identificar el puntero
   base de los gates (esperado un valor `0x020axxxx`).
3. **Leer los flags**: con base B → `B+0x12e3` y `B+0x12f5` (son bytes). Si B≈`0x020a0064` → leer `0x020a1347`
   y `0x020a1359`. (⚠️ leer `0x020axxxx` puede colgar → power-cycle a mano.)
4. **Confirmar el bug**: ambos deben estar a **0** tras el connect KEY_ABSENT.

## Fase 1 (fix, tras confirmar)
- Re-emitir `ADD_REMOVE_KEY(GTK)` tras el 4-way → re-leer `[0x12f5]`. ¿Flipa a !=0? → `udhcpc -i wlan0 -t 6` →
  ¿llega la OFFER? Si sí: **DHCP resuelto (broadcast)**.
- Si `[0x12f5]` NO flipa con la GTK → el trigger es el otro path (SET_BSS_INFO); atacar `[0x12e3]` primero.

## Artefactos
- `fw-analysis/` (raíz): decompilado Windows — `fw_all_labeled.c`, `caller_decomp.c`, `callers.txt`,
  `fw_key_decompiled.c`, `fw_f0020000.txt` (disasm), `fw_f0020000.bin`.
- `mainline/wifi-consys/fw-analysis/`: decompilado Mac — `fw_decompiled.c`, `fw_key_functions.txt`.
- Las DIRECCIONES de función coinciden entre ambos decompilados; las líneas no.

*Sesión Mac (Opus 4.8), 2026-07-01. Verifica y refina `HANDOFF-TXENCRYPT-GATE-0630.md`.*
