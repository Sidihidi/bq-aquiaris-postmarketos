# Módem H3 — arrancar el MD: scaffolding VALIDADO, handshake pendiente — 0714

> Primer intento de H3 (arrancar el MD ejecutando MOLY). **El andamiaje funciona y soltar el MD es
> SUPERVIVIBLE (no crashea el AP); falta el handshake CCCI** — la iteración profunda que el plan
> ([PLAN-M1](PLAN-M1-ARRANQUE-MD-0712.md)) predecía (~1-2 sem). Código granular en `mt6582-spm-H1.c`.

## Lo que FUNCIONA (validado en HW, menupick23, granular por sysfs)
1. **`spm_md_load`** ✅: `request_firmware("modem.img")` (5172580 B, en `/lib/firmware/`) → `ioremap` del
   carveout 0xb8000000 → `memcpy_toio`. Verificado: **`w0=0xe59ff018`** = el vector ARM de MOLY (`ldr
   pc,[pc,#24]`) → el firmware quedó bien colocado al inicio del carveout.
2. **`spm_md_remap`** ✅: fórmula BANK-remap 1:1 del downstream, `des=0xb8000000`. Valores calculados y
   escritos: `AP0=0x5f5d5bb9 AP1=0x67656361 MD0=0x43413f39 MD1=0x4b494745` (MD0 coincide con el cálculo a
   mano → **fórmula correcta**). Persisten tras el release (re-dump los confirma).
3. **`spm_md_release`** ✅ (¡el paso arriesgado!): WDT del MD off (`0x20050000`=0x2200) + boot-slave keys
   (Key `0x2019379C`=0x3567C766, Vector `0x20190000`=0x0, En `0x20195488`=0xA3B66175) → **el AP SOBREVIVE**
   (release-rc=0, uptime sigue subiendo, sin lockup del bus). Esto era el mayor riesgo de H3 y se pasó.

## Lo que NO funciona aún: el handshake CCCI
Tras el release, el CCIF sigue en **CON=BUSY=TCHNUM=RCHNUM=0** → **el MD no levanta ningún canal**
(no llega `MD_INIT_START_BOOT`). (El `RX0=0xb0661584` de la 1ª lectura era dato residual del buffer, no
un mensaje: CON/RCHNUM nunca cambiaron.) El MD no está ejecutando MOLY hasta el punto de hablar por CCIF.

## Sospechosos del handshake (siguiente iteración)
- **NO es la EMI-MPU**: `emi_mpu_set_region_protection` va bajo `#ifdef ENABLE_EMI_PROTECTION`; en bring-up
  se compila OFF (memoria ABIERTA = el MD accede libre) → no ponerla es lo correcto, no el bloqueante.
- **Candidatos reales** (pendiente de RE + probar):
  1. **CCIF init**: quizá el CCIF necesita configurarse (CON a modo arbitración) antes de que el MD lo use.
  2. **Run-time data / shared-mem geometry**: el MD puede esperar el bloque de run-time data escrito antes
     de pasar de su arranque interno al handshake.
  3. **Layout de memoria del MD**: cómo el vector de reset del MD (arranca en su addr 0x0 = SRAM/ROM
     interna) alcanza el firmware en DRAM 0xb8000000. El BANK-remap mapea bank4 (vista 0x40000000 del MD),
     pero el reset vector y el jump inicial a DRAM pueden necesitar otra pieza (¿el boot ROM del MD lee el
     GFH? ¿falta un registro que apunte el firmware?).
  4. Revisar la secuencia completa `md_env_setup_before_boot` → `set_ccci_runtime` → `let_md_go` del
     downstream (algo entre el remap y el release que omitimos).

## Estado y valor
**Andamiaje de H3 en su sitio**: carga + remap + release ejecutan y son seguros. El MD arranque completo
(handshake) es la iteración profunda restante — territorio de "primer MT65xx con el MD en mainline".
Firmware (H0), MTCMOS (H1), CCIF/BANK accesibles (H2a), carveout (H2b) todos ✅. Fuente:
`mt6582-spm-H1.c` (H1+H2+H3). Triggers: `/sys/module/mt6582_spm/parameters/spm_md_{load,remap,release,dump}`.

*H3 1er intento 2026-07-14. Scaffolding OK; handshake CCCI = siguiente iteración.*
