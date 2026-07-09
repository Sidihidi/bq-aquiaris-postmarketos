# Verificación de los drivers de la sesión paralela — 2026-07-09 (sesión principal)

> Contraste 1:1 contra el downstream en la Pi (registros, fórmulas, umbrales, secuencias, DT).
> Anexo prometido en `ESTADO-PROYECTO-0709.md` §carril-paralela.

## Resumen
| Driver | Veredicto | Bloqueantes |
|---|---|---|
| mmc3516x (magnetómetro) | **LISTO-PARA-HW** | 0 |
| mt6582-thermal | **LISTO-PARA-HW** (reservas menores) | 0 |
| mt6323-accdet | **NECESITA-FIXES** (jack OK; los BOTONES no funcionarían) | 2-3 |

## 1. mmc3516x — LISTO-PARA-HW
Registros/comandos/timings 1:1 con el downstream (`mmc3516x.h:35-50`, RESET_INTV=150 ✓); secuencia
degauss = read-path stock (`mmc3516x.c:475-506`); TM+poll DS ✓; conversión `(raw-32768)*1000/2048`
con Z invertida ≡ `65536-raw` del stock ✓; escala IIO correcta (mGauss + scale 0.001).
- Menores (cosméticos): sin degauss en resume; regcache_sync no-op.
- **Mount-matrix**: el cust del krillin usa `direction=1` → empezar por `[[0,-1,0],[1,0,0],[0,0,1]]`
  (NO la matriz del accel).

## 2. mt6582-thermal — LISTO-PARA-HW
Efuse V1 bit a bit = `thermal_cal_prepare()` (GE=buf1[31:22], OE=buf1[21:12], VTS/DEGC/SLOPE/SIGN ✓;
index7/8 → 0x10206100/4 → celda DT `calib@100` len 0xc ✓). Fórmula término a término =
`raw_to_temperature_MCU1` (gain, x_roomt, 15/18, 165±slope; el plegado de signo del parser mainline
reproduce ambas ramas) ✓. Init HW = `thermal_reset_and_initial()` (ch11 immediate, VALIDMASK 0x2C,
TS_CON0[7:6]=00 buffer-on, PNPMUXADDR, MONCTL0=0x3) ✓. Bases confirmadas: AUXADC 0x11001000,
THERMAL 0x1100B000, APMIXED 0x10209000 (los comentarios del mt_thermal.h ds son morralla).
- Reservas menores para la validación HW: (1) mainline no hace el reset PERI bit16 del bloque
  (riesgo bajo); (2) clock "therm" fijo asume gate PERI abierto (vigilar que la 1ª lectura no
  cuelgue el bus); (3) confirmar label `&auxadc_clk` en el DTS; (4) nuance efuse ID==0 && slope≠0
  (solo fuses raros).

## 3. mt6323-accdet — NECESITA-FIXES (botones)
Correcto: todos los offsets (`reg_accdet.h`), cust krillin exacto ({0x900,0x400,1,0x3f0,0x800,0x800,0x20},
press-debounce 0x400), umbrales botones MD<90/UP<240/DW<500 mV, semántica AB, protocolo de clear del
IRQ, ADC ch8 (CON22 0x76E, ADC7 0x722, mV=raw·1800/32768), IRQ MFD 18, binding DT.

**Fixes necesarios (~15 líneas en total):**
1. **BLOQUEANTE — auxadc switch**: antes de leer la tensión de tecla escribir `ACCDET_RSV=0x5A20`
   (bit5 conecta accdet→AUXADC; downstream `accdet_auxadc_switch(1)`, accdet.c:159-176,638) y
   restaurar 0x5A10 después. Sin esto TODO botón lee <90mV = PLAYPAUSE.
2. **BLOQUEANTE — PWM boost**: durante la medida, `PWM_THRESH=pwm_width` (duty 100%) y restaurar
   0x400 después (accdet.c:639-640/824-825). Sin esto la tensión va troceada al 44% = lecturas aleatorias.
3. **PROBABLE BLOQUEANTE — IDLE_EN**: en power_on, `SWCTRL=0x77` (añadir `0x07<<4`; downstream 378-381
   con MULTI_KEY) — mantiene micbias/comparador en idle; sin ello el botón puede no disparar.
4. Menor: eliminar el write `0x0400` bit14 (ANALDO_CON0) — es EXCLUSIVO de ACCDET_PIN_SWAP (mux
   FSA8049) que el krillin no monta.
5. Menor: `RG_VBUF_EN` (0x758 bit4) antes del read del ADC (el stock lo activa para ch<9).
6. Menor: en plug-out dejar `RSV=0x1A10` (consumo).

Jack-detect y mic/no-mic probablemente funcionen TAL CUAL; los 3 botones necesitan 1-3.

*Verificación por agente (contraste downstream), revisada por la sesión principal. 2026-07-09.*
