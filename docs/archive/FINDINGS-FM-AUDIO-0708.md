# FM audio — RESUELTO (2026-07-08, sesión casa)

## TL;DR — FUNCIONA 🎉
**La radio FM SUENA en mainline** por los auriculares. La causa del silencio eran DOS bugs nuestros,
no el hardware:

1. **Lado chip**: `fmradio` llamaba a `FM_IOCTL_RESTORE_SEARCH` (46) para "encender el I2S" —
   pero `mt6627_restore_search()` empieza con **`mt6627_RampDown()` = PARA el DSP de audio**.
   El chip quedaba sintonizado (RSSI real) pero con la salida I2S emitiendo **DC constante**
   (muestras `0xff0c` fijas en el AWB). Fix: NO llamar a ese ioctl; con POWERUP+TUNE+VOL+UNMUTE
   el DSP queda sonando (`FM_MAIN_CTRL 0x63 = 0x0001/0x0011`, como el stock).
2. **Lado SoC**: la ruta del kernel #256 usaba GAIN1 vía `AFE_CONN_GAIN1_IN (0x448)` — registro
   que **NO existe en mt6582** (tabla de otro chip) → el I2S-in nunca llegaba al DAC. Fix:
   **conexión directa `CONN1 bit16` (I00→O03) + `CONN2 bit1` (I01→O04)**, que sí latchea y suena.

Uso final: `fmradio 1023 &` + `amixer -c 0 cset name='FM Radio Route' 1` (auriculares = antena).

## El veredicto "FM analógico" del 0708-mediodía estaba EQUIVOCADO
El ground-truth con LineageOS (dual-boot, dumps en `groundtruth-0708/`) lo refutó:
- `AUDTOP_CON3 = 0x0` con FM sonando → **ningún line-in analógico activo** en el codec.
- `AFE_I2S_CON = 0x8000000d` (2º I2S in) **idéntico** a lo que ya escribíamos.
- ASRC con **tracking vivo** (`CON15/18/19` moviéndose ~0xA00000) = reloj I2S real entrando.
- El path del stock es: **FM chip → I2S digital → ASRC → AWB (captura a DRAM, `CONN2 b16/b22` =
  I00/I01→O05/O06) → loop por SOFTWARE (AudioFlinger) → DL1 (SRAM 0x11221000) → DAC → HP**.
  Nosotros lo mejoramos: conexión directa I00/I01→O03/O04, sin memif ni userspace.
- La evidencia "analógica" (flags FM_DIGITAL ausentes, MRGIF comentado, I2s_Setting sin linkar)
  era del **transporte por pines/MRGIF externos**, que no aplica: el MT6627 es on-die y su I2S
  entra al AFE por pad interno sin pinmux. Lección: esos gates de build no describían este path.

## Ground truth (dumps en `groundtruth-0708/`)
- `base.txt` / `fm-on.txt`: `/proc/audio` de LineageOS sin/con FM sonando (SoC + codec PMIC).
  Delta clave codec: `ABB_AFE_CON0=1, CON3=0x255, CON4=0x272, CON10=1, CON11=0x303`;
  `AUDTOP_CON0 0x6012→0x7012, CON4=0x7c, CON5=0x4414, CON6=0xf5ba` (= nuestro HP + bit12 CON0).
- `fmchip-pmos.txt` / `fmchip-lineage.txt`: los 256 regs del chip FM (ioctl `FM_IOCTL_RW_REG`,
  herramienta `fmdump.c`). Diferencia decisiva: `0x63 FM_MAIN_CTRL` = 0x0000 (nuestro, DSP parado
  por el RampDown) vs 0x0011 (stock, TUNE+RDS_MASK activos). El resto: freq (98.4 vs 102.3),
  monitores RSSI y RDS. `0x9b = 0x000b` EN AMBOS → el write 0xF9AB del restore_search ni se
  retiene ni hace falta.
- Tabla de interconexiones REAL: `mt_soc_audio_v1/mt_soc_afe_connection.c` (MT6583, misma
  generación): `mConnectionReg`/`mConnectionbits` decodifican CONN0=0x20...CONN4=0x30.

## Detalles del fix en el kernel (`mt6582-afe-pcm.c`, ruta `FM Radio Route`)
- ASRC: constantes que ya teníamos eran CORRECTAS (14/16/17/20 idénticas al stock); se añade
  `CON13=0x11`, `CON21=0x1800` y el enable pasa a **solo bit0** (bit6 no latchea en mt6582).
- Conexiones: `CONN1 |= BIT(16)`, `CONN2 |= BIT(1)`. Nada de GAIN1.
- El resto (clocks, ADDA 44.1k, I2S_CON1=0x909, DAC_CON1 [15:12]=9, codec HP) ya estaba bien.

## GOTCHAs aprendidos (siguen valiendo)
- `0x448` (CONN del GAIN1) NO existe en mt6582; `ASRC_CON0 bit6` no latchea.
- El dd al eMMC **desde Android** "verifica" md5 pero NO persiste (page cache + write-protect
  de la región boot al arrancar Android) → para volver a pmOS: **fastboot flash boot**.
  Desde pmOS el dd a seek=83968 sí funciona.
- La app FM de LineageOS: desbloquear pantalla (540x960), botón play en ~(468,888); el chip
  se enciende AL DAR PLAY, no al abrir la app.
- `devmem ADDR` lee ("ADDR = VAL"), `devmem ADDR VAL` escribe (sintaxis propia, sin width).

## Cómo reproducir
```sh
fmradio 1023 &                                  # chip on + tune (fd abierto = FM viva)
amixer -c 0 cset name='FM Radio Route' 1        # path SoC + codec HP
# parar: amixer -c 0 cset name='FM Radio Route' 0; pkill -x fmradio
```
Pendiente de pulido (menor): control de volumen FM (SETVOL/escala), stop limpio al cerrar
sesión, y quizá un applet/script `fm` que haga las dos cosas.
