# GPS — CONTINUAR: el problema de ADQUISICIÓN de RF (handoff para el Mac, 0715)

> **TL;DR**: toda la tubería del GPS está montada y funciona (DSP→mnld→gpsd→geoclue→apps, ver
> `HANDOFF-CHIPDETECTOR-0714.md` + `INTEGRACION-GPS-0714.md`). **PERO el DSP no adquiere ni un satélite**
> — `$GPGSV,1,1,0` = 0 satélites EN VISTA, incluso pegado a una ventana donde LineageOS pilla 10. Es un
> **fallo de init de RF del GPS en el kernel**, y para arreglarlo hace falta la **secuencia de encendido
> RF del GPS del stock**, que NO tenemos (el downstream de la Pi es solo firmware, sin source).

## Qué FUNCIONA (no re-hacer)
- **Gate `service.nvram_init=Ready`** resuelto (RE Ghidra) → mnld corre en pmOS, abre `/dev/stpgps`.
- **Shim v3** (`gps-bionic-shim-v2.c`, en `/system/lib/libxlogshim.so`): inyecta chipid + service.nvram_init,
  lazy-init + CAS atómico + unlink(/dev/__properties__). Robusto.
- **DSP vivo**: leer `/dev/stpgps` directo da frames 0xAAF0 (tipo 0x28/0x29) continuos y CAMBIANTES (busca).
- **mnld lee+procesa los frames** (probado con contadores de kernel en `mt6582-btif.c`: gps_rx≈gps_rd_data,
  gps_rd_eintr=0) → emite NMEA válida.
- **Integración**: `etc/local.d/zzz-gps.start` monta gpsdrv+mnld+socat-PTY+gpsd+gps-share.sock AUTO al boot;
  geoclue (network-nmea, modem-gps=false). Apps: `xgps`, `Where am I?`, `cgps` en el menú de Phosh.
- **EPO** (`/data/misc/EPO.DAT`+`EPOHAL.DAT`, de `http://epodownload.mediatek.com/EPO.DAT`, 276KB) — mnld lo
  ABRE (`openat`), pero NO cambia nada (lógico: sin RF no hay satélites que asistir).

## El PROBLEMA (lo único que queda): 0 satélites en vista = RF no recibe
`$GPGSV,1,1,0` siempre. gpsd `"mode":1` (no fix). xgps: todo N/A, cielo vacío. **El DSP busca pero el
payload de sus frames es ruido `0xCA`** → no le llega señal de antena al correlador.

### DESCARTADO (no perder tiempo aquí)
- DSP muerto ❌ (está vivo y buscando). TCXO ❌ (NVRAM 26MHz = `80 ba 8c 01`, correcto). Gate/pipeline ❌
  (funcionan). WiFi/coexistencia ❌ (bajar wlan0 no cambió nada). EPO/asistencia ❌ (sin satélites en vista,
  no hay a qué asistir). Reloj ❌ (sistema en 2026, OK).

### LA CAUSA (hipótesis fuerte)
El driver de pmOS `drivers/soc/mediatek/mt6582-btif.c` hace **SOLO `func_on(GPS)`** (WMT cmd
`01 06 02 00 02 01`) — **ninguna init de RF/reloj/LNA específica del GPS**. En stock, tras func_on(GPS) hay
más: probablemente **co-clock del GPS**, **calibración RF del GPS** (el driver hace un RF-CAL WMT `01 14 01
00 01` pero es genérico/para WiFi), y/o **power del LNA de la antena** (regulador/GPIO). Sin esa secuencia,
el frontend RF del GPS no recibe → 0 satélites.

## ⏭️ CÓMO SEGUIR (dos vías, ambas necesitan la referencia del stock)
1. **Recuperar la fuente GPL del kernel de BQ Aquaris E4.5 (MT6582)** — BQ la publicó. Buscar el driver
   **`mtk_wcn_combo`** / **`wmt_lib.c`** / **`wmt_ic_soc.c`** / **`stp_core`** / el `gps_drv`/`mt3326_gps`
   downstream, y ver QUÉ hace con el GPS DESPUÉS de `func_on(GPS)`:
   - ¿Envía un comando WMT extra de "GPS RF enable" / "set GPS param" / co-clock?
   - ¿Toca un regulador/GPIO para el LNA de la antena GPS? (buscar en el DTS stock la antena/LNA del GPS).
   - ¿Una calibración RF específica del GPS (adie/RF register writes)?
   Portar eso a `mt6582-btif.c` (añadir tras `func_on(b, STP_TYPE_GPS, "GPS")` en `__bringup`, ~línea 387).
2. **Capturar el arranque del GPS desde LineageOS/Android** (dual-boot, método ground-truth del proyecto):
   bootear stock, y con el GPS encendido capturar el tráfico WMT al combo (los comandos STP_TYPE_WMT que
   manda el kernel stock) + dmesg. Comparar con lo que manda pmOS (solo func_on). El delta = la init que falta.

Nota: el `RF-CAL` que hace el driver (`stp_send(STP_TYPE_WMT, {01,14,01,00,01})`) es EL sospechoso a variar
primero — probar otros parámetros / un segundo cal enfocado a GPS. Y el "co_clock" del combo (buscar
`CO_CLOCK`/`osc`/`26M` en el connsys stock).

## Estado del móvil (Alpine v3.24 armhf, kernel bueno `d7fe5484`)
- Cadena GPS auto-arranca al boot; para reiniciar a mano: `pkill -9 -f /xbin/mnld; pkill -f socat; pkill -f
  sbin/gpsd; /etc/local.d/zzz-gps.start`.
- Probar adquisición: `gpspipe -w | grep -E 'mode|nSat'` (o `cgps`/xgps en la pantalla). Con cielo:
  `$GPGSV` debería mostrar satélites (>0) si la RF se arregla.
- Debug del DSP directo: `pkill -9 -f mnld; dd if=/dev/stpgps bs=64 count=6 | od -An -tx1` (frames 0xAAF0).
- gpsdrv `.ko` persistido en `/lib/modules/7.0.12/`. Repo pmOS `@pmos` añadido (ver `mainline/APK-REPOS-0714.md`;
  ojo: es edge, `akms`/`libcamera` útiles). Kernel-debug con contadores: fuente en
  `mt6582-btif.c.DEBUG-counters` (restaurado el kernel bueno; los contadores NO están en el kernel actual).
- ⚠️ NVRAM GPS real del móvil en `/data/nvram/APCFG/APRDEB/GPS` (54B, TCXO). No borrar.

*Casa (Opus 4.8), 2026-07-15. Tubería GPS hecha; falta la init RF del GPS (necesita referencia del stock).*
