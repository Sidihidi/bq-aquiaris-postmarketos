# PARA CASA — FM Radio: estado y siguiente paso (audio)

> Handoff de la sesión Mac (2026-07-08). El FM **sintoniza**; falta el **audio** (ruteo del codec).
> Detalle técnico completo: `docs/bitacora/2026-07-08-fm-audio.md`.

## ✅ Hecho (driver funcional, committeado)
El port stock del FM (MT6627) **enciende y sintoniza** en mainline. Verificado en HW (#255):
`/dev/fm` ✅ · `GETCHIPID=0x6627` ✅ · **POWERUP rc=0** ✅ · **TUNE rc=0** ✅ · SETVOL/GETVOL/MUTE rc=0 ✅.
Dos fixes (ya en el repo):
1. **`fm_glue.c:123`** — `mtk_wcn_wmt_chipid_query()` devuelve **`0x6582`** (SoC id, no `0x6627`): el dispatch
   de `fm_open` (`core/fm_main.c:231`) enlaza `MT6627fm_low_ops` con `0x6572|0x6582|0x6592|...`. Con `0x6627`
   no matcheaba → link muerto. **En el árbol de build de la Pi ya está aplicado.**
2. **Firmware del patch** — `/etc/firmware/mt6627/mt6627_fm_v1_{patch,coeff}.bin` (el chip usa ROM v1).
   Versionados en `mainline/wifi-consys/fm/firmware/`. Extraídos de `~/wifi-fw/system.img`
   (`simg2img` + mount `/mnt/sysimg/etc/firmware/mt6627/`). **DESPLEGARLOS en un flash limpio** o POWERUP
   da `FM_EPATCH`.

Test: `mainline/wifi-consys/fm/fmtest.c` → `arm-linux-gnueabihf-gcc -static -o fmtest fmtest.c` →
`/tmp/fmtest 1041` (freq=MHz×10; band UE, space 100k). Deja el FM encendido (se apaga al `close`).

## ❌ Falta: AUDIO (silencio con auriculares) — es RUTEO, no el driver FM
Probado con auriculares + FM sonando 30 s (fd abierto, vol 15, unmute) → **nada por altavoz ni auriculares**.
El driver FM está bien; el bloqueo es el **path de audio**:
- Nuestro `mtk_wcn_cmb_stub_audio_ctrl` (en `fm_glue.c`) es **no-op** (`return 0`). En el stock, ese llama al
  **`aif_ctrl_cb` que registra el DRIVER DE AUDIO** (`mtk_wcn_cmb_stub_alps.c:161` → `wmt_lib_set_aif`) y hace
  el **pin-mux + config del codec** para conectar el audio del FM a la salida. En el port ese callback **no
  existe** → el audio del FM nunca se enruta → silencio.
- Además el chip arranca con **`aud_cfg.aud_path = FM_AUD_I2S`** (`mt6627/pub/mt6627_fm_config.c:117`) =
  digital por I2S al AFE, no analógico.

### Qué implementar (terreno de audio = vuestro, como el playback normal)
Decidir el path del krillin y cablearlo:
- **Opción A (analógico, probablemente lo más simple)**: el FM saca audio analógico → entrada FM/line-in del
  **codec MT6323** → mezclar a HP/altavoz. Poner `aud_path` a analógico (config del FM o ioctl `I2S_SETTING`=33)
  + activar el mixer **FM→HP** del MT6323 (registros, análogo a lo que ya hacéis en `mt6582-afe-pcm.c` para el
  playback). Comprobar en el downstream cómo cablea el krillin el audio del FM (analógico vs I2S).
- **Opción B (I2S)**: FM → I2S → AFE del MT6582 → codec. Nuevo DAI de captura/loopback en el AFE. Más trabajo.
- Punto de enganche común: implementar el `aif_ctrl_cb` (hoy no-op) para que al POWERUP configure el
  pin-mux/codec del FM. Es la pieza exacta que falta.

## Cómo verificar cuando esté el ruteo
Reproducir sonido manteniendo el fd abierto (si no, el FM se apaga al cerrar): compilar `fmtest.c` variante
que haga POWERUP+SETVOL+MUTE(0)+TUNE y luego `sleep(30)` sin `close`, con auriculares (=antena). Sintonizar
una emisora local real. **RSSI=0 vía `GETRSSI` puede ser un valor cacheado** (no fiar; el juez es el oído).

## Reproducibilidad en flash limpio
1. Kernel con `fm_glue.c` (chipid=0x6582) — ya en el árbol de build.
2. Desplegar `mainline/wifi-consys/fm/firmware/*.bin` → `/etc/firmware/mt6627/` en el móvil.
3. (Cuando haya audio) el ruteo del codec + arranque del FM en la GUI/`fmtool`.

*Sesión Mac (Fable 5), 2026-07-08. FM sintoniza; audio = ruteo codec pendiente (vuestro).*
