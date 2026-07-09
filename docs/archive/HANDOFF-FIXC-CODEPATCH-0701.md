# HANDOFF — FIX C (patch de código del FW) para el TX-encrypt del broadcast (Windows/Opus, 2026-07-01 noche)

Cierra el hilo del blind-poke con un RE profundo (workflow `w340q25jx`, 7 agentes, **2 verificadores adversariales
que refutaron el diagnóstico inicial** y afinaron el cuadro). Bytes **verificados 1:1** contra `fw_f0020000.bin`.

## Qué se probó y qué pasó (HW, #219, visto en la consola-pantalla)
El **pre-key poke DISPARÓ** (escribió `[0x12e3]=1`/`[0x12f5]=1` ANTES de cada add_key) — el `wifi_runtime_reg_write`
(ACCESS_REG SET) **SÍ escribe con la conexión activa** aunque el read dé `deadbeef`. **PERO SIGUIÓ CRASHEANDO**
~0.6s tras CONNECTED. → poner el gate a pelo NO es el fix.

## Causa raíz del crash (afinada por los verificadores; el "overrun SW" del diagnóstico inicial fue REFUTADO)
El path SW-CCMP (`f0034bd8`) exige fragmentación (`ucFragNum>=2`) y el DISCOVER no fragmenta → NO es esa la causa.
**La causa real:** con el gate a 1, `FUN_f002a4cc` (@0xf002a4cc, L7242) devuelve 1 para broadcast → los builders TX
(`f00351f4` L15069-71, `f0035fc4` L15981-86) ponen el bit **ENCRYPT 0x4000** en el descriptor → el **motor HW-crypto
intenta cifrar la trama de grupo contra una entrada WTBL de grupo SIN cifrador** → fallo del motor (~0.6s = primera
trama de grupo tras el add_key). Sin poke, `f002a4cc` devuelve 0 → esa ruta no corre → la conexión sobrevivía.

## Por qué la WTBL de grupo no tiene cifrador (el bloqueo real, verificado byte a byte)
`FUN_f004bb2c` (@0xf004bb2c) pone `local_40=1` (bit tx-encrypt WTBL) SOLO si `param_1+0x10==0x41 && [0xf7c]==0 &&
[0x12e3]!=0`. La **PTK** entra como `0x41` (con el poke, cumple → cifrador OK). La **GTK entra como `0x42`** (L30965)
→ el test `==0x41` FALLA → `local_40=0` para el grupo. Y el DHCP DISCOVER es **broadcast = GTK**. Por eso el NOP
simple de `f004bd24` (que solo quita el sub-check `[0x12e3]`) arregla la PTK pero **NO el DHCP**.

Stream confirmado en el binario (base 0xf0020000):
```
f004bd0c=44500041 (ADDI r5,#0x41)   f004bd10=d90d (branch del test ==0x41 -> f004bd2a, salta el store)
f004bd1e=cd06 (bnez38 guard [0xf7c]) f004bd20=000012e3 (LBI gate [0x12e3])
f004bd24=c003 (beqz38 gate -> f004bd2a)  f004bd26=8401 (movi55 r0,#1)  f004bd28=ae38 (store local_40=1)
```
nds32 BE: nop16=`0x9200`, movi55 r0,#1=`0x8401`, r0,#0=`0x8400`.

## EL FIX combinado (código + gate) — bytes exactos
**1) Parche de CÓDIGO por `fwpoke`, EN IDLE ANTES de conectar** (para que `f004bb2c` no esté ya cacheada al correr):
   - `@0xf004bd10 = 0x9200` — NOP del branch `==0x41` → la GTK (0x42) también alcanza el store de `local_40`.
   - `@0xf004bd24 = 0x9200` — NOP del check `[0x12e3]` → `local_40=1` sin depender del flag.
   - (opcional) `@0xf004bd1e = 0x9200` — NOP del guard `[0xf7c]`.
   - Si `fwpoke` es word-only: `@0xf004bd24 = 0x92008401` (preserva el `movi55 0x8401` de los 2 bytes bajos);
     idem cuidar `f004bd10`/`f004bd1e` (leer la word, cambiar solo el halfword, reescribir).
   → **la WTBL de grupo (y la PTK) reciben cifrador HW.**
**2) MANTENER el pre-key poke** de `[0x12e3]`/`[0x12f5]=1` (ya en el driver, `g_poke_gates=1`) → `f002a4cc` devuelve 1
   → el broadcast pide cifrar. Ahora, con la WTBL ya con cifrador, **no hay mismatch → no crash → el DISCOVER sale
   cifrado → DHCP**.
**3) VALIDAR (obligatorio, los verificadores insisten):** `fwdump` del bit tx-encrypt de la WTBL de grupo (índice
   uVar4=2, vía `_DAT_00014e4c`) TRAS el add_key GTK → debe estar a 1; + captura aérea del DHCP DISCOVER cifrado.

## Riesgos / cautelas (confianza baja-media)
- Confirmar con desensamblador nds32 que `f004bd10` (0xd90d) es EXACTAMENTE el branch del `==0x41` y su destino
  `f004bd2a` (tras el store), y que NOPearlo no rompe la ruta WEP (0x11 va por otra rama, L31007 → debería ser seguro).
- Dar cifrador HW a la WTBL de grupo con el contexto PN (`0x12e4`/`0x12ec`) — que el add_key GTK real (`f004b86c`,
  L30833-43) SÍ rellena — coherente: confirmar que add_key GTK corre ANTES del primer TX de grupo.
- I-cache nds32: pokear el código EN IDLE (antes de que la función corra) o invalidar la I-cache tras el poke.
- Descartadas: (b) forzar `f002a4cc->return 1` (compartida por >=4 sitios, global, riesgoso); (c) forzar el bit
  0x4000 en el builder (re-cifra EAPOL/mgmt en claro → rompe el 4-way).
- Alternativa a investigar: `[0x197c]` (L7245, gate para `+0xd==1`, ¿multicast vs broadcast?) podría cifrar el
  DHCP sin tocar `[0x12e3]`.

## Estado / herramientas listas
Kernel **#219** en el móvil (blind-poke + pre-key poke + `poke_gates` + `fwpoke` + `fwdump`; commit `0db468d`).
**Modo debug-consola activo** (`/etc/local.d/zzzz-phosh.start` → `.debugoff`; logs del kernel en pantalla; revertir
renombrando). El connect es MUY intermitente (el FW se degrada por intento) → **power-cycle EN FRÍO + 1ª conexión de
boot fresco**. Full RE del workflow: task `w340q25jx`.

*Co-autor: Claude Opus 4.8 (sesión Windows). Sobre `HANDOFF-FWDUMP-GATES-0701.md`.*
