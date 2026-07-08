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

## SIGUIENTE PASO (el que cierra el FM)
**RE de la HAL para la secuencia ANALÓGICA line-in→HP del MT6323** (playbook del codec, ya probado):
- Símbolos de entrada: `AudioFMController::SetFmEnable(bool,bool,bool,bool,bool)` @0x73968 (1580 B),
  `AudioFMResourceManager::SetFmDirectConnection(bool)` @0x522d4 (652 B — hace llamadas por vtable, hay
  que seguirlas), `AudioFMController::ChangeDevice(uint)` @0x734e8. HAL en `~/audio-hal/` de la Pi.
- Objetivo: la lista ORDENADA de writes `SetAnalogReg(reg,val,mask)` (@0x34ea4) del path FM-analog
  (esperable: AUDTOP_CONx para line-in PGA + mux HP←line-in + quizá algo en el lado connsys).
- Al tenerla: añadirla como variante en `mt6582_codec_dl_on()` o un kcontrol `FM Analog Route`, y el
  fmradio + kcontrol lo enchufan. Alternativa de validación: dual-boot al ROM STOCK de BQ (tiene app
  FM Radio) + `/proc/audio` sonando = ground truth de los AUDTOP con FM activo (como se hizo con el HP).

## Cómo reproducir el estado de prueba
`fmradio 1023 &` (fd abierto = FM on) + `amixer -c 0 cset name='FM Radio Route' 1` (path digital,
no suena por lo de arriba). Limpieza: kill fmradio + cset 0.
