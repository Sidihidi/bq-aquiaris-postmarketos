# Fase B — codec analógico MT6323: mapeado 90%, bloqueado en 1 valor del HAL (2026-07-06)

## Objetivo
Producir el primer SONIDO audible. La Fase A.2 (motor PCM DL1 del AFE del SoC) funciona y está
verificada. Falta encender la **etapa analógica del MT6323** (por pwrap) para que el audio digital
que ya fluye salga por altavoz/auriculares.

## Arquitectura confirmada (MT6582 + MT6323 "clásico")
- El **DAC vive en el ADDA del SoC** (bloque AFE @0x11220000; nuestro driver ya configura
  `AFE_ADDA_DL_SRC2`, `AFE_I2S_CON1`, `AFE_ADDA_UL_DL_CON0`). El MT6323 NO hace el DAC principal
  (su driver kernel NUNCA toca el banco PMICDIG `AFE_UL_DL_CON0`/`AFUNC_AUD_CON` @0x4000).
- El **MT6323** hace la etapa analógica de salida: bias del DAC (`AUDTOP_CON5`), driver de
  auriculares (`AUDTOP_CON6`), amplificador de altavoz (`SPK_CON0..12`), y **gate de la interfaz
  de audio** (`ABB_AFE_CON0` @0x4000, bits 0-1). Todo por **pwrap** (`/usr/local/bin/pwrap_poke`,
  verificado que alcanza 0x0700, 0x0052 y el banco 0x4000).

## Secuencia de ENABLE reconstruida (invirtiendo el DISABLE de AudDrv_Kernel.c:1299-1308)
Probado en HW por pwrap mientras el AFE reproduce (ninguna combinación sonó aún):
```
# power_init (AudDrv_Kernel.c:281-288, MT6323 confirmado)
clksq_en_aud(1)  = TOP_CKPDN0(0x0102) bit0=1
AUDTOP_CON5(0x070A) = 0x1114     # DAC bias 50%
AUDTOP_CON6(0x070C) = 0x37A2 -> 0x37E2   # enable depop MUX de HP drivers
clksq_en_aud(0)
# gates que estaban CERRADOS (leídos a 0 en baseline):
ABB_AFE_CON0(0x4000) = 0x0003    # enable interfaz de audio (baseline 0x0000)  <-- CONFIRMADO por el disable
AUDTOP_CON0(0x0700) |= 0x1000    # 1.4v common mode (baseline 0x6010 -> 0x7010) <-- CONFIRMADO por el disable
# amp de altavoz (mt_soc_codec_speaker_63xx.c, SPK_CON @0x0052 = MT6323):
SPK_CON7(0x0060) = 0x4531 ; SPK_CON0(0x0052)=0x3008 ; SPK_CON2(0x0056)=0x04A4
SPK_CON0=0x3009 (spk on) ; wait 10ms ; SPK_CON0=0x3001 (trim off)
#   -> SPK_CON1 bit15 quedó 0 = "spk trim FAIL" (posible: la ruta del krillin es amp externo GPIO118)
```

## EL BLOQUEO: falta el valor exacto de `AUDTOP_CON4`
El disable hace `AUDTOP_CON4(0x0708) = 0x0000  // Disable audio bias and L-DAC`. El valor de
**ENABLE** (bias + L/R DAC) NO está en NINGUNA fuente disponible:
- El kernel del krillin (`AudDrv_Kernel.c`) solo tiene `power_init` (no toca CON4 en enable) y el
  disable; el enable lo hacía el **HAL de Android en userspace** (`AudioAnalogControl`, binario).
- `upmu_hw.h` no tiene bit-fields de `AUDTOP_CON4` (solo la dirección).
- Los logs de LineageOS (`~/android-cap/lineage/kmsg*.log`) SÍ loguean cada `Ana_Set_Reg`
  (offset/value/mask) y capturaron una sesión de audio (`AUD_SET_ANA_CLOCK`, `SET_SPEAKER_ON`),
  PERO la ventana capturada solo tiene writes repetitivos a 0x10a/0x10c=0x100 — NO la init del
  codec (AUDTOP/SPK).

Baseline leído (codec apagado): `AUDTOP_CON0=0x6010 CON2=0x00c0 CON4=0x0000 CON5=0x1100 CON6=0x1c32
CON7=0x0000  SPK_CON0=0x0000 SPK_CON2=0x0014  ABB_AFE_CON0=0x0000 TOP_CKPDN0=0x0003`.

Sospecha añadida: quizás también falte encender el **ADDA del SoC** del todo
(`AFE_ADDA_TOP_CON0` @0x11220120, que nuestro driver NO escribe) — verificar si el DAC del SoC
está realmente produciendo analógico.

## Cómo desbloquear (por fiabilidad)
1. **Volcado de registros desde LineageOS EN VIVO (definitivo)**: el AudDrv downstream tiene un dump
   de `AUDTOP_CON0..9` (`AudDrv_Kernel.c:596-605`). Arrancar LineageOS (dual-boot; `boot.img` en
   `~/android-cap/`), reproducir un sonido, leer el dump (adb/sysfs) → valores EXACTOS de TODOS los
   registros de audio de golpe. Requiere el desvío de arrancar LineageOS + adb.
2. **RE del HAL**: montar la partición system de LineageOS del móvil desde pmOS (read-only),
   extraer `libaudio.primary.mt6582.so`, desensamblar (Ghidra, como el FW WiFi) → la secuencia de
   `AUDTOP_CON4`/routing. Sin arrancar LineageOS pero es RE de un binario ARM.
3. **Fuerza bruta empírica**: probar valores de `AUDTOP_CON4` de oído. Lento e incierto (16 bits).

## ✅ GROUND TRUTH capturado de LineageOS + HAL extraído (0706 noche)
Dual-boot a LineageOS (`fastboot`/`dd` del boot en `mmcblk0 seek=83968`; adb autorizado en la Pi) +
reproducir sonido + `cat /proc/audio` (dump del AudDrv chardev) = **estado EXACTO del codec sonando**
(`lineage-audio-registers-PLAYING-0706.txt`). El valor que faltaba: **AUDTOP_CON4=0x7c**. Repliqué
por pwrap+devmem TODOS los registros (SoC AFE + PMIC) a los valores exactos de LineageOS —
**y aún NO suena**. Diagnóstico: el estado FINAL coincide pero el encendido analógico necesita la
SECUENCIA ORDENADA con ESPERAS (depop, asentamiento del bias), que un poke plano no reproduce.

**La secuencia analógica vive SOLO en el HAL de Android** (no en el kernel — el driver del krillin es
el chardev `sound/mt6582/`, que ya porteamos como la Fase A.2 digital; la parte analógica la hacía el
HAL vía ioctl `SET_ANAAFE_REG`, y el kernel NO loguea esas escrituras). **EXTRAÍDO el binario del HAL**:
`audio.primary.mt6582.so` (720KB, ELF ARM 32-bit, en `~/audio-hal/` de la Pi + scratchpad local),
con la clase `AudioAnalogControl` y la escritora `AudioAnalogReg::SetAnalogReg(reg,val,mask)` @0x34ea4.
**RE en curso** (agente, estilo firmware WiFi con Ghidra/objdump): extraer la secuencia ordenada de
`AnalogOpen`/`AnalogOpenForAddSPK`@0x36d5c/`SetFrequency` con sus delays → esa es la secuencia a
implementar 1:1 en el codec driver. Salida esperada: lista ordenada de SetAnalogReg + msleep.

**SIGUIENTE (cuando el RE dé la secuencia)**: implementar la secuencia 1:1 (ordenada + delays) — como
script pwrap para validar (¿suena?) y luego en el codec driver ASoC (DAPM que llama a la secuencia).
Deltas SoC AFE ya conocidos (a meter en el driver AFE): `AFE_I2S_CON=0x8000000d`, `AFE_DAC_CON0=0x13043`
(no 0x3), `AFE_ADDA_NEWIF_CFG0=0x3f87200`/`CFG1=0x3117180`, `AFE_CONN2=0x410040`, `ABB_AFE_NEWIF0=0x7330`
(freq ADDA=7, no 9). El resto (AUDTOP/ABB/SPK) = PMIC, por la secuencia del HAL.

## Herramientas dejadas
- `/tmp/audio-codec-test.sh` (en el móvil): poke de power_init + SPK.
- `/tmp/tone30.wav`: tono 440Hz 30s estéreo para pruebas.
- Reproducir en bucle: `while [ ! -e /tmp/stopaudio ]; do aplay -D hw:0,0 /tmp/tone30.wav; done &`
- Codec ASoC de referencia del MT6323 (SPK): `~/mainline/downstream/sound/soc/mediatek/mt_soc_audio_v2/mt_soc_codec_speaker_63xx.c`. El v3 y el codec_63xx de v2 son MT6331 (no aplican salvo estructura).
