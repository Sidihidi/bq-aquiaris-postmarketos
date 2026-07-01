# HANDOFF — TX-encrypt: el GATE del cifrado LOCALIZADO + cómo continuar (2026-06-30, sesión Windows/Opus 4.8)

**UNIFICA el hilo TX-encrypt con el RE del Mac** (`HANDOFF-KEYTYPE-NDS32-0630.md`, `HANDOFF-CASA-0630.md`).
Convergimos independientemente: nds32 + Ghidra 12.1.2 + las MISMAS funciones (`f002a4cc`, `f004bb2c`,
`f004be5c`, `f002a480/554`) + el MISMO `fwdump` (`wifi_runtime_reg_read`, `CMD_ID_ACCESS_REG=0xc2`, puerto 1).
Este RE (workflow de 10 agentes, 3/3 verificadores de acuerdo, confianza **media**) encontró el **gate exacto**
y el **fix**, y **reconcilia** el hilo "TX-encrypt" con el "data-path RX / 0 TX-DONE" del Mac: son el MISMO bug.

---

## TL;DR — qué hacer al sentarte
1. **CAUSA RAÍZ:** el FW gatea el data-path (cifrado TX + port control 802.1X) en flags software del **BSS
   record**: **`[0x12e3]`** (privacy/port; unicast + broadcast) y **`[0x12f5]`** (grupo/GTK; el broadcast).
   Con nuestro `SET_BSS_INFO enc_status=CCMP_KEY_ABSENT` quedan a **0** → `f002a4cc` devuelve false → los
   builders TX no ponen el bit Protected `0x4000` → los datos no salen (0 TX-DONE de datos) → sin OFFER.
2. **El re-envío de `SET_BSS_INFO(0x16)` NO cuelga por `enc_status`** — cuelga por `f004aac8` (re-init en
   caliente del MAC: beacon/TSF + busy-wait `0x6032ff14`). Ese es el misterio de los 2 días.
3. **FIX:** encender los flags por la **RUTA DE COMANDOS DE CLAVE** (no el `0x16`). **Plan B pragmático** =
   re-instalar la **GTK** → dispara `f0034610` → `[0x12f5]=1` → el **DISCOVER broadcast se cifra → DHCP**.
4. **Herramienta lista:** el `fwdump` **puerto-1 (0xc2)** LEE la data RAM `0x020axxxx` SIN colgar (demostrado:
   volqué `0x0209f800`+3160 palabras, incluido el BSS a `0x020a1000`). **Desbloquea el PASO 2 del `CASA`.**

---

## 1. El mecanismo (por DIRECCIÓN de función — válido para cualquier decompilado)
- **Decisor por-trama `FUN_f002a4cc` (`0xf002a4cc`, privacy.c)** = el "port control" que el Mac ya identificó.
  Decide si el data frame se cifra/pasa. Devuelve:
  - **BROADCAST/GTK** (el DHCP DISCOVER): directamente **`[0x12e3]`**.
  - **UNICAST**: `STA+0x289` (solo se pone a 1 para **TKIP** eStaType `0x11`, NO para **CCMP** `0x41`); el
    fallback `[0x12e3]` exige `BSS+0x10 < 3` = FALSO estando asociado → **`return false`**.
- Los **builders TX** `FUN_f00351f4` (`0xf00351f4`) y `FUN_f0035fc4` (`0xf0035fc4`) ponen el bit **Protected
  `0x4000`** SOLO si `f002a4cc != 0`. Con f002a4cc==0 → frame sin cifrar / descartado por el puerto.
- El **bit tx-encrypt de la WTBL** (`local_40` en `FUN_f004bb2c` @ `0xf004bb2c`, nic_privacy.c; commit HAL
  `_DAT_00014e4c`) se programa SOLO si se cumplen 3 condiciones: `algorithm==0x41 (CCMP) && [0xf7c]==0 &&
  [0x12e3]!=0`. Con `[0x12e3]==0` la PTK entra en la WTBL **sin cifrado**.
- **`[0x12e3]` tiene 3 LECTURAS y 0 ESCRITURAS** por símbolo en todo el FW → lo pone una **memcpy en bloque**
  del payload de **`CMD_ID_SET_BSS_INFO=0x16`** (handler `f002222c`, que Ghidra no decompila → capstone no
  soporta nds32). El campo **`enc_status`** aterriza en el BSS record. `ENABLED` → !=0; `KEY_ABSENT` → 0.
- El gate de **grupo `[0x12f5]`/`[0x1aa1]`** SÍ tiene setter explícito: **`FUN_f0034610`** (@ `0xf0034610`,
  `base[0x12f5]=1; base[0x1aa1]=1`), invocado por el path de instalación de la **GTK**.

## 2. Reconciliación con el Mac (el "0 TX-DONE de datos" ES este gate)
El Mac observó: handshake OK, PTK+GTK CCMP instaladas, pero **0 data frames RX + 0 TX-DONE de datos** +
tcpdump 0 paquetes. Explicación: con `[0x12e3]==0`, `f002a4cc` (su "port control") devuelve false → el puerto
controlado **descarta/no-cifra** los data frames antes del TX-DONE → el AP nunca ve el DISCOVER → 0 OFFER →
0 data RX. **Mismo bug, dos síntomas.** No es un problema de RX aparte: es el gate de cifrado/puerto en TX.

## 3. Por qué cuelga el re-envío de SET_BSS_INFO(0x16) (el misterio resuelto)
El handler `0x16`, al detectar cambio de estado del BSS **estando asociado**, ejecuta **`FUN_f004aac8`**
(`0xf004aac8`): re-install del BSS (`_DAT_00014950`, assert nic.c:0x710), reprog WTBL (`FUN_f004bed0(7)`),
**reprograma beacon/TSF con un timer 5000** (`_DAT_00014c98`) y hace un **busy-wait sobre `0x6032ff14`**
(`FUN_f004be5c`). El re-arme del beacon/TSF con la conexión viva → **watchdog**. **NO es el campo de cifrado.**

## 4. EL FIX + Plan B
Encender los flags + el bit WTBL por la **ruta de CLAVE** (tocan la WTBL vía punteros HAL `_DAT_00014e4c/e54/
e78` SIN pasar por `f004aac8`/beacon → no cuelga; son escrituras puntuales idempotentes):
- **🥇 Plan B (lo que desbloquea el DHCP):** el DISCOVER es **broadcast** → cifrado con la GTK, gateado por
  `[0x12f5]`. **Re-instalar la GTK** dispara `f0034610` → `[0x12f5]=1` → broadcast cifrado → **OFFER → DHCP**.
  Sin tocar `[0x12e3]` ni la PTK ni el `0x16` colgante.
- **Completo (unicast):** además, re-instalar la **PTK** (cmd `0x08` o `0x17`) **con `[0x12e3]` ya != 0** para
  que `f004bb2c` ponga `local_40=1`. ORDEN OBLIGATORIO: gate a 1 → reinstalar PTK.
- **Plan B-3 (último recurso):** patch binario del FW = NOP del check de `[0x12e3]` en `f004bb2c` para que
  ponga `local_40=1` incondicionalmente en CCMP, o forzar `base[0x37b]=1` en el install-key.

## 5. CÓMO CONTINUAR — pasos concretos (fwdump puerto-1)
**PASO A — validar el diagnóstico (leer los flags):**
- Conectar WPA2 a "hola" (ver receta del `CASA`). Dentro de los ~30s (antes del `BSS_BEACON_TIMEOUT 0x1b`):
- `echo "020a0068 1" > fwdump_cfg; cat fwdump` → el puntero al BSS array (en idle dio **`0x020a1000`**;
  converge con el PASO 2 del Mac).
- Leer el bloque de estado del BSS **`0x020a1340 .. 0x020a1390`** (BSS[0]=`0x020a1000`):
  `echo "020a1340 20" > fwdump_cfg; cat fwdump`. Ahí están: **`+0x35c`** (`0x020a135c`, key-installed, el flag
  del Mac), **`+0x37b`** (`0x020a137b`, candidato a `[0x12e3]`), **`+0x38d`** (`0x020a138d`, candidato a
  `[0x12f5]`). (El offset exacto de `[0x12e3]` está entre `+0x347` y `+0x37b` — identificarlo por cuál está a
  **0** tras el connect y **flip** con el fix.)
- **CONFIRMAR:** con `enc_status=KEY_ABSENT`, `[0x12e3]` y `[0x12f5]` deben estar a **0** = el bug probado.

**PASO B — probar el Plan B (GTK, sin tocar el 0x16):**
- Tras el 4-way, **re-emitir el `ADD_REMOVE_KEY` de la GTK** (grupo). Re-leer `[0x12f5]` → si pasa a **!=0**,
  el broadcast se cifra → `udhcpc -i wlan0 -t 6` → **¿llega la OFFER?**. Si sí: DHCP resuelto (broadcast).

**PASO C — unicast (si hace falta):** re-instalar la PTK con `[0x12e3]!=0` → `f004bb2c` pone `local_40=1`.

## 6. Coordinación / notas
- **fwdump:** usar la versión **puerto-1 (`CMD_ID_ACCESS_REG=0xc2`)**. CONFIRMADA leyendo `0x020axxxx` sin
  colgar (en idle). ⚠️ Con conexión activa, tener el power-cycle a mano (el Mac vio cuelgues con la versión
  port-0). Si el kernel flasheado aún cuelga en `0x020axxxx`, reflashear el driver port-1.
- **Dirección del flag:** el puntero base `0x020a0098` está a 0 en idle → **se resuelve en runtime** (al crear
  el BSS). Por eso hay que leerlo CONECTADO. El BSS array vive en `0x020a1000` (confirmado por `0x020a0068`).
- **Artefactos (workspace Windows `C:\Users\jferr\Desktop\pmos-krillin\`):** `fw_decompiled.c` (880KB, 648
  funcs, Ghidra 12.1.2 nds32 headless), `fw_functions.txt`, `fw_key_functions.txt`, proyecto `ghidra_proj/`,
  scripts `ghidra_scripts/` (`Sweep.java` = detector de arch; `AddBlocks.java`+`ExportAll.java` = extracción).
  Las **direcciones** de función coinciden con el `fw-analysis/` del Mac; las líneas NO (decompilados distintos).
- **Confianza media:** el único eslabón sin prueba dura es que `enc_status → BSS+0x37b` (`f002222c` no
  decompilable). El PASO A lo cierra empíricamente.

*Co-autor: Claude Opus 4.8 (sesión Windows). Convergente con la sesión del Mac (glm-5.2 / opencode).*
