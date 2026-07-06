# 🏆 HITO — AUDIO del BQ krillin FUNCIONA EN MAINLINE (auriculares + altavoz) (2026-07-06)

Por primera vez, el BQ Aquaris E4.5 "krillin" (MediaTek MT6582) **reproduce sonido en el kernel
mainline** (linux-7.0.12, postmarketOS): **auriculares Y altavoz**, validado en hardware.

## El camino (de "no suena" a sonido completo)
1. **Fase A.2** (commit 16ec3a6): motor PCM DL1 del AFE del SoC — el audio digital fluye (aplay sin
   XRUN, IRQ 1×/periodo). Sin sonido audible (falta el codec analógico).
2. **Ground truth de LineageOS**: dual-boot + `cat /proc/audio` sonando → estado EXACTO del codec.
   Repliqué TODOS los registros por pwrap/devmem → **seguía sin sonar**. Diagnóstico: el estado
   final coincide, pero el encendido analógico necesita la **SECUENCIA ORDENADA con ESPERAS**.
3. **RE del HAL** (`audio.primary.mt6582.so`, ARM, con objdump — estilo firmware WiFi): extraída la
   secuencia exacta de `AudioAnalogControl::AnalogOpen`/`AudioMachineDevice::AnalogOpen` (dev 2/3 =
   auriculares). **La pieza que faltaba: un `usleep(10000)` = 10 ms de asentamiento del bias entre
   `AUDTOP_CON4=0x7c` y `AUDTOP_CON6=0xf5ba`** + el reloj del AFE del PMIC (`0x10c`) + el ORDEN.
4. **Secuencia 1:1 → SONIDO POR AURICULARES** ✅.
5. **Amp externo del altavoz** (GPIO118): el RE confirmó que el altavoz del krillin = ruta de
   auriculares + amp externo (no el class-D interno del MT6323, que queda a `SPK_CON0=0x200`). Tren
   de pulsos de `Sound_Speaker_Turnon` (yusu_android_speaker.c) → **SONIDO POR ALTAVOZ** ✅.

## Arquitectura (confirmada)
- **DAC digital**: AFE del SoC @0x11220000 (nuestro driver, Fase A.2) → ADDA SRC + NEWIF serial al PMIC.
- **Codec analógico**: PMIC MT6323 por pwrap — bias del DAC + drivers de auriculares (`AUDTOP_CON*`)
  + interfaz de audio (`ABB_AFE_CON*` @0x4000). La secuencia de encendido (con delays) la hacía el
  HAL de Android; la reconstruimos por RE.
- **Altavoz**: driver de auriculares del codec → amp externo habilitado por **GPIO118** (tren de
  pulsos 1‑0‑1‑0‑1 + 40 ms warm-up, termina HIGH).

## Las secuencias que funcionan (en `codec-sequence/`)
- **`codec-enable.sh`**: el encendido del codec. Clave = ORDEN + `sleep 0.02` de bias-settle entre
  `AUDTOP_CON4(0x708)=0x7c` y `AUDTOP_CON6(0x70c)=0xf5ba`. Incluye el reloj AFE del PMIC
  (`0x10c |= 0x100`), `ABB_AFE` (NEWIF `0x4024=0x7330` freq ADDA=7, CON0=1, CON1/3/4/10/11), y los
  deltas del SoC AFE por devmem (`AFE_I2S_CON=0x8000000d`, `AFE_DAC_CON0=0x13043`,
  `AFE_ADDA_NEWIF_CFG0=0x3f87200/CFG1=0x3117180`, `AFE_CONN2=0x410040`).
- **`spk-amp-on.py`**: amp externo GPIO118 (DOUT reg7 bit6=0x40 en 0x10005000; SET 0x474 / RST 0x478;
  GPIO118 ya viene dir=out + modo GPIO del boot). Timing de µs por mmap /dev/mem.

## Registros de referencia (estado sonando, de LineageOS — `lineage-audio-registers-PLAYING-0706.txt`)
AUDTOP_CON4=0x7c CON5=0x4414 CON6=0xf5ba CON0=0x7012; ABB_AFE_CON0=0x1 CON1=0x9 CON3=0x253 CON4=0x274
CON10=0x1 CON11=0x303 NEWIF_CFG0=0x7330; SoC AFE_DAC_CON0=0x13043 I2S_CON=0x8000000d
ADDA_NEWIF_CFG0=0x3f87200.

## ✅✅ FORMALIZADO EN EL DRIVER (0706, kernel #241) — aplay suena SOLO, sin scripts
`mt6582-afe-pcm.c` ahora hace TODO en `.prepare`/`.close`:
- Obtiene el **regmap del MT6323** vía el phandle DT `mediatek,pmic = <&pmic>` (el mt6323 es hijo del
  MFD mt6397; `dev_get_drvdata(mt6323_dev)` = `mt6397_chip`, con `chip->regmap` del pwrap,
  `max_register=0xffff` → alcanza AUDTOP @0x700 y ABB_AFE @0x4000). EPROBE_DEFER si no está listo.
- `mt6582_codec_dl_on()`: la secuencia analógica EXACTA por `regmap_write`/`regmap_update_bits` con el
  **`usleep_range(10000,12000)`** de bias-settle (la clave). `mt6582_codec_dl_off()` al cerrar.
- `mt6582_spk_amp()`: amp externo GPIO118 por `ioremap(0x10005000)` — tren de pulsos con `udelay(2)`
  + `msleep(40)`. On en prepare, off en close.
- Deltas del SoC AFE (I2S_CON=0x8000000d, NEWIF0/1, DAC_CON0 bits 6/12/16, DAC_CON1, CONN2) en prepare.

**Validado en HW**: con `aplay` solo (sin scripts), el driver deja AUDTOP_CON4=0x7c, CON6=0xf5ba,
ABB_CON0=0x1, GPIO118=ON, I2S_CON=0x8000000d → suena por auriculares Y altavoz. Confirmado por el
usuario. Los scripts de `codec-sequence/` quedan como referencia histórica.

## PENDIENTE (no bloqueante)
- **Audio en la GUI (Phosh/PulseAudio)**: `aplay` (ALSA directo) suena; Livi/mpv por GStreamer
  probablemente también (fallback a ALSA). Pero PulseAudio/callaudiod **descartan la card**:
  `Card 'alsa_card.platform-11220000.audio-controller' lacks speaker and/or earpiece port, skipping`.
  Causa: la card usa codec **dummy** → sin puertos con nombre (Speaker/Headphones/Earpiece) que
  necesita el mapeo de PulseAudio/UCM. **Fix (userspace, aparte del driver)**: un **perfil UCM**
  (`/usr/share/alsa/ucm2/...` o ucm) que describa el path de playback + los puertos, O un codec ASoC
  de verdad con DAPM widgets (Speaker/Headphones/Earpiece routes). Con eso la GUI/YouTube suenan.
- Jack detection (rutar HP vs SPK según auriculares enchufados) — ahora enciende ambas rutas a la vez.
- Captura (mic/VUL) = Fase D. TX-power/rate finos.
- Limpieza pendiente: organizar `~/home/cpcd` de la Pi (audio-hal, builds, logs dispersos).

*El audio del krillin, de imposible en mainline a auriculares+altavoz sonando. Vía: Fase A.2 (AFE)
+ ground truth de LineageOS + RE del HAL de Android (usleep de bias-settle) + amp GPIO118.*
