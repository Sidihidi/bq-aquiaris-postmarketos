# FM audio — HALLAZGO: es I2S INTERNO (PAD_CONN), no analógico (sesión drivers 0709)

> Continuación de `PARA-CASA-FM-SIGUIENTE.md` ("el FM sintoniza; falta el audio").
> **RE del downstream completado: la ruta del audio FM del krillin queda determinada.**

## 🎯 El hallazgo que cambia el plan
`mt6627_fm_config.c:117-121` configura **INCONDICIONALMENTE**:
```c
cfg->aud_cfg.aud_path = FM_AUD_I2S;       /* digital, NO analógico */
cfg->aud_cfg.i2s_info.status = FM_I2S_OFF;
cfg->aud_cfg.i2s_info.mode = FM_I2S_MASTER;   /* FM = master */
cfg->aud_cfg.i2s_info.rate = FM_I2S_32K;      /* 32 kHz */
cfg->aud_cfg.i2s_pad = FM_I2S_PAD_CONN;       /* ★ pad INTERNO CONSYS→AFE (on-die) */
```
- **La Opción A (analógico line-in del MT6323) NO es la ruta del krillin** — por eso los auriculares
  callan: el FM nunca saca audio analógico en esta config.
- `FM_I2S_PAD_CONN` = el I2S del FM va **por dentro del SoC** del CONSYS al AFE (no hay pines externos
  → **no hace falta pin-mux del AP**: en `wmt_plat_audio_ctrl` los estados AIF solo tocan GPIOs para
  pads EXTERNOS via `PIN_I2S_GRP`; con PAD_CONN el `aif_ctrl_cb` puede quedarse esencialmente no-op
  en el lado pines — el trabajo real está en el AFE).

## La cadena AFE que falta (de `mt_soc_audio_v2/mt_soc_pcm_fm_i2s.c` — START exacto)
```
FM (I2S master 32k, pad interno CONNSYS)
  → 2nd I2S IN del AFE  con  mI2S_IN_PAD_SEL = false  ("I2S_IN_FROM_CONNSYS")   [Set2ndI2SIn + Set2ndI2SInEnable]
  → interconexión:  I00→O13 , I01→O14      (entrada al ASRC)
  → ASRC 32000 → 44100   [SetI2SASRCConfig(true, 44100) + SetI2SASRCEnable(true)]
  → interconexión:  I10→O03 , I11→O04      (salida ASRC → I2S DAC out)
  → SetI2SDacOut(rate, false, 16 bits) + SetI2SDacEnable(true)   (ya lo hace vuestro DL1)
  → EnableAfe(true)
  → codec analógico MT6323 (la secuencia HAL que YA tenéis del MILESTONE playback)
```
El STOP hace el inverso (disconnect I00→O13, I01→O14, I10→O03, I11→O04; ASRC off; 2nd-I2S-in off; DAC off si nadie más lo usa).

## Receta de REGISTROS exacta (extraída de `mt_soc_afe_control.c` v2)
```c
/* 1) 2nd I2S IN desde CONNSYS (Set2ndI2SIn + Set2ndI2SInEnable):
 *    AFE_I2S_CON = (INV_LRCK<<5)|(FMT<<3)|(SLAVE<<2)|(WLEN<<1)|(PAD_SEL<<28)|BIT(31 phase-fix)
 *    FM: PAD_SEL=0 (CONNSYS), SLAVE=1 (el FM es master), FMT=1 (I2S), WLEN=0 (16b)
 *    => AFE_I2S_CON = 0x8000000C  (mask 0xFFFFFFFE), luego bit0=1 para enable
 *    ★ = 0x8000000D — ¡EXACTAMENTE el delta que ya apuntasteis en FASE-B ("AFE_I2S_CON=0x8000000d")!
 *      (o sea: LineageOS tenía el 2nd-I2S-in del FM configurado; vuestro valor ya es el correcto)
 */

/* 2) ASRC 32k->44.1k (SetI2SASRCConfig(true, 44100)): */
AFE_CONN4    &= ~BIT(30);
AFE_ASRC_CON13 &= ~BIT(16);          /* 0 = stereo */
AFE_ASRC_CON14 = 0xDC8000;           /* rama 32000->44100 */
AFE_ASRC_CON15 = 0xA00000;
AFE_ASRC_CON17 = 0x1FBD;
AFE_ASRC_CON16 = 0x00075987;         /* calibración */
/* 3) ASRC enable (SetI2SASRCEnable(true)): */
AFE_ASRC_CON0 |= BIT(6) | BIT(0);

/* 4) Conexiones (SetConnection; tabla reg/bit por par en mt_soc_afe_connection.c):
 *    I00->O13, I01->O14   (2nd-I2S-in -> entrada ASRC)
 *    I10->O03, I11->O04   (salida ASRC -> I2S DAC out)
 */

/* 5) DAC out + EnableAfe: lo que ya hace vuestro DL1 (SetI2SDacOut 44.1k 16b + enable). */
```

## Qué implementar (contra vuestro `mt6582-afe-pcm.c`)
Vuestro port del AFE ya escribe `AFE_CONN*`/I2S/DAC (FASE-B deltas). Falta añadir un camino "FM I2S":
1. **2nd I2S IN**: registro `AFE_I2S_CON` (el "2nd I2S" del v2; en vuestro `mt6582-afe-regs.h`), con
   `PAD_SEL=0` (CONNSYS), formato I2S, 16 bits, **slave del lado AFE** (el FM es master), 32 kHz.
2. **Conexiones**: bits de `AFE_CONN*` para I00→O13, I01→O14, I10→O03, I11→O04 (la tabla de
   interconexión del v2 — `SetConnection()` en `mt_soc_afe_control.c`/`AudDrv_Common_func.c`
   del downstream da el registro/bit exacto de cada par).
3. **ASRC**: bloque `AFE_ASRC_CON0..` (config 32000→44100 + enable) — funciones `SetI2SASRCConfig`/
   `SetI2SASRCEnable` del mismo fichero.
4. Exponer como kcontrol ALSA ("FM I2S Switch") o un simple echo por debugfs para probar.

## Cómo probar (cuando se implemente el punto de arriba)
1. `fmtest` variante sin `close()` (POWERUP+TUNE+SETVOL+MUTE(0) + sleep) a una emisora real.
2. Activar la ruta AFE FM (kcontrol/poke) + la secuencia del codec analógico (la del playback normal).
3. El juez es el oído (RSSI cacheado no fiable, dicho en el handoff FM).
4. Si hay silencio: verificar en el lado FM que el I2S esté ON — ioctl `FM_IOCTL_I2S_SETTING` (33)
   con `FM_I2S_ON` (el default arranca `status=FM_I2S_OFF`; puede que el POWERUP del port no lo
   encienda — mirar `mt6627_fm_lib.c:552` que solo hace el setup I2S si `aud_path==I2S`, y ver si
   el port llega a esa rama).

## Ficheros fuente (downstream, en la Pi)
- `sound/soc/mediatek/mt_soc_audio_v2/mt_soc_pcm_fm_i2s.c` — el flujo completo (líneas ~241-327).
- `sound/soc/mediatek/mt_soc_audio_v2/mt_soc_afe_control.c` — cuerpos de Set2ndI2SIn/ASRC/SetConnection.
- `fmradio/mt6627/pub/mt6627_fm_config.c` (config I2S) + `mt6627_fm_lib.c` (setup I2S del lado FM).

*Sesión drivers (Opus) 2026-07-09. El lado FM (driver) ya está; esto es 100% terreno del AFE.*
