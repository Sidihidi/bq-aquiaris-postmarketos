# FM audio — diagnóstico completo (2026-07-08, sesión casa)

## TL;DR — VEREDICTO
**El FM del krillin es ANALÓGICO, no I2S.** El chip FM (CONNSYS on-die) saca el audio por un
**camino analógico** hacia el line-in del codec MT6323; NO por I2S al AFE. Todo el path digital
construido esta sesión (2º I2S in → ASRC → GAIN1 → DAC, kernel #256) es correcto pero **no está
cableado en este hardware** → silencio. **Lo que falta = la secuencia line-in→HP del codec MT6323**,
que vive en la HAL (`audio.primary.mt6582.so`) — el MISMO tipo de RE que desbloqueó los auriculares.

## Evidencia del veredicto (0708)
1. El **AudDrv del mt6582** (el driver real del krillin) tiene los regs **MRGIF COMENTADOS** (no hay
   merge-interface en mt6582) y llama a los estados de audio como "**Analog FM**" (AIF_0/1) vs
   "Digital FM" (AIF_2/3).
2. El pinmux I2S del combo en `wmt_plat_alps.c` está **gateado por `FM_DIGITAL_INPUT/OUTPUT`** — y el
   krillin **no define esos flags** ni tiene pines `GPIO_COMBO_I2S_*` en su cust (`dct/cust_gpio_usage.h`).
3. La HAL tiene `AudioFMResourceManager::SetFmDirectConnection(bool)` = el modo "direct" analógico.
4. **`mt6627_I2s_Setting` NO EXISTE en el kernel compilado** (nm vacío; la asignación `ops->bi.i2s_set`
   está dentro de un `#if` falso) → el ioctl `FM_IOCTL_I2S_SETTING` (33) es código muerto (devuelve
   ~ELINK). Coherente: esta plataforma nunca usó FM digital.
5. Con TODO el path digital del SoC armado y verificado por registro + codec sonando (tono de prueba
   audible), el FM = silencio → no llegan datos al AFE.

## Lo que SÍ quedó funcionando/hecho
- **FM chip**: POWERUP + TUNE (102.3, RSSI=-60 real con auriculares-antena) + vol/unmute, por `/dev/fm`.
- **`fmradio.c`** (repo + `/usr/local/bin/fmradio` en el móvil): powerup+tune+unmute+**RESTORE_SEARCH**
  (ioctl 46 → `mt6627_restore_search()` = FM reg `0x9B=0xF9AB` "I2S Tx mode" + host `0x80101054=0x3f35`;
  rc=0) y mantiene el fd. Nota: esos writes son la salida DIGITAL (inexistente aquí); para el analógico
  puede que no hagan falta (o que el DAC analógico del FM esté siempre on — a confirmar en el RE).
- **Kernel #256**: kcontrol **`FM Radio Route`** en `mt6582-afe-pcm.c` que monta el path digital completo
  (2º I2S in `AFE_I2S_CON=0x8000000d` + ASRC 32k→44.1k CON13-20 + GAIN1 + conexiones + DAC + codec).
  Inofensivo; queda como referencia/por si otro device de la familia sí cablea I2S.
  GOTCHA hallado: **el interconnect `0x448` NO existe en mt6582** (la tabla de `mt_soc_afe_connection.c`
  v3 es de otro chip); las conexiones directas al DAC van por CONN1 b16 (I00→O03) / CONN2 b1 (I01→O04)
  — esos SÍ latchean. El bit6 de `ASRC_CON0` tampoco latchea en mt6582.

## RE de la HAL — AVANCE (0708 tarde): estructura mapeada, valores parametrizados
La escritora es `AudioAnalogReg::SetAnalogReg(this, reg, val, mask)` — args: **r1=reg, r2=val, r3=mask**
(NO r0/r1/r2; r0=this). Se llama SIEMPRE por PLT `0x2f00c` (0 llamadas directas). Funciones clave:
- **`AudioMachineDevice::AnalogOpen@0x3723c`** (1928B) = el path HP que ya conocíamos (AUDTOP CON4=0x708,
  CON5=0x70a, CON6=0x70c, CON7=0x70e + usleep de bias). Ramifica por DEVICE_TYPE (r5: cmp 18, 11).
- **`AudioMachineDevice::AnalogSetMux@0x37074`** (456B) = **EL MUX DE ENTRADA ANALÓGICA** (line-in/FM).
  DECODIFICADO (ramas por device r1 ∈ {16,17} y mux r2 ∈ {8,10,16}):
    - `AUDTOP_CON3 (0x706)` bit8 / bit9 = enable L / R del input (val 0x100/0x200, o 0 para clear).
    - `AUDTOP_CON1 (0x702)` bits[7:4] = selección PGA/mux (val 0x20 dev16, 0x40 dev17).
    - `AUDTOP_CON0 (0x700)` bits[3:0] = mux (val 3 ó 4).
- `AudioMTKVolumeController::SetLineInPlaybackGain@0x4d614` = ganancia line-in→HP (FM = line-in PLAYBACK).
- `AudioPlatformDevice::AnalogOpen` = el ABB_AFE del PMIC (0x4024/0x4002/0x4028/...).

**BLOQUEO del RE estático**: `AudioFMController::SetFmDirectConnection@0x731dc` usa **DEVICE_TYPE 3 y 5**
y despacha por **vtable** (`blx ip` sobre punteros del pool) → no se ve qué AnalogSetMux/AnalogOpen device
concreto llama sin resolver las vtables (lento). Probé en HW las conjeturas del mux (CON3=0x300 L+R,
CON1[7:4]=2/4, CON0[3:0]=3/4) con FM on + HP on → **SILENCIO** (no son los valores/no es solo el mux).

## SIGUIENTE PASO — GROUND TRUTH (la vía fiable, como se clavó el HP)
**Dual-boot al ROM STOCK de BQ + volcar `/proc/audio` con la radio SONANDO.** El AudDrv chardev dumpea
TODOS los registros del codec (AUDTOP CON0-9 + ABB_AFE + SoC AFE). Comparar ese dump (FM activo) contra el
dump del HP-solo → el **delta = los registros exactos del path FM-analog** (line-in mux + lo que enrute HP
←line-in), sin adivinar. Infra dual-boot ya validada (`~/android-cap/`, adb autorizado en la Pi). Ojo: usar
el ROM de **BQ** (trae app FM Radio); si LineageOS no la trae, instalar una app FM o usar la de BQ.
Con los valores exactos → añadir un kcontrol `FM Analog Route` en `mt6582-afe-pcm.c` (variante de
`mt6582_codec_dl_on` con el mux line-in) que el `fmradio` + amixer enchufan. AUDTOP en el PMIC = por
`regmap`/pwrap (como el HP).

## Cómo reproducir el estado de prueba
`fmradio 1023 &` (fd abierto = FM on) + `amixer -c 0 cset name='FM Radio Route' 1` (path digital,
no suena por lo de arriba). Limpieza: kill fmradio + cset 0.
