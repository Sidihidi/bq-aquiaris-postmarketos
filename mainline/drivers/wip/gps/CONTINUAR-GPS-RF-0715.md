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

---
## 0715 tarde (Mac): fuente GPL de BQ recuperada + init RF portada + DIAGNÓSTICO del ruido 0xCA
Clonado `github.com/bq/aquaris-E4.5` (rama aquaris-E4.5) → tenemos el SOURCE del combo/wmt (antes solo fw).
**Portada al kernel (#297) TODA la init RF del GPS que el stock hace**, verificada byte a byte contra el source:
- **RF-CAL**: nuestro `01 14 01 00 01` es **byte-idéntico** al `calibration_table` del stock (WMT_CORE_START_RF_CALIBRATION). ✓ ya estaba bien.
- **COEX** (`WMT_COEX_SETTING_CONFIG` 01 10 .. ant_mode=1, del WMT_SOC.cfg krillin): AÑADIDO → `*** COEX OK ***` (chip lo acepta).
- **GSYNC** (reg chip `0x80050078` bits[30:28]=1, `mtk_wcn_soc_gps_sync_ctrl`): AÑADIDO → `*** GSYNC GPS OK ***` (chip lo acepta).
- **func_on(GPS)** ✓ + **LNA GPIO47 high** (userspace) ✓.
- crystal_trim: se salta (NVRAM WIFI vacío); co_clock: no aplica (co_clock_flag=0).

### DIAGNÓSTICO (leído /dev/stpgps directo): el DSP recibe **RUIDO 0xCA**
Frames crudos del DSP (tipos 0xFE28/29/42) con el payload **LLENO de bytes `0xCA`** = el correlador **no
engancha NINGUNA señal**. NO es frecuencia (el search Doppler ±5-10kHz cubriría el offset del TCXO, y el WiFi
va) → **es AMPLITUD: a la antena/mixer no le llega señal RF útil.** Reguladores RF TODOS on (vcn28/vcn18/VTCXO).
Móvil personal al lado = 18 satélites (hay señal ambiente + hw OK, LineageOS pilla 10).

### → El muro: el LNA / path RF de la antena (o algo del firmware que el source no muestra)
El ruido 0xCA con RF alimentado apunta a que **el LNA de la antena no amplifica**: o **GPIO47 NO es el pin
correcto**, o el LNA necesita **alimentación además del enable** (un regulador/bias que no vemos en el source),
o el path de antena del combo no está ruteado pese al COEX. El source del kernel NO tiene más init RF del GPS
que la ya portada → lo que falta está en el **HAL/firmware** o es un **detalle de HW del krillin** invisible en
el source genérico.

### ⏭️ ÚNICO camino definitivo: GROUND-TRUTH desde LineageOS
Arrancar LineageOS (GPS funciona) y con el GPS encendido + viendo satélites, capturar:
1. **Todos los GPIOs** (`cat /sys/kernel/debug/gpio`) → comparar con pmOS: **qué GPIO(s) mueve el stock para
   el GPS que nosotros no** (candidato: el LNA real; ver si GPIO47 está high en stock o es OTRO pin).
2. **Reguladores** (`/sys/class/regulator` o el PMIC) → qué LDO extra enciende para el GPS.
3. El tráfico WMT/STP al combo (si se puede snoopear el btif).
El delta GPIO/regulador entre stock-con-fix y pmOS-sin-fix = EXACTAMENTE lo que falta. (Alternativa parcial:
montar la data de Android para el NVRAM — no montó, cifrada/F2FS.)

*Mac (Fable), 0715 tarde. Init RF del kernel 100% portada del source GPL; el DSP sigue en ruido 0xCA =
problema de LNA/path-RF de antena no visible en el source. Necesita ground-truth de GPIOs/reguladores del
LineageOS con GPS activo.*

---
## 0715 noche: DELTA REAL = el patch del CONSYS (ROMv1 vs mt6572_82), pero con complicación
**Ground-truth de firmware (LineageOS)**: el stock carga **`ROMv1_patch_1_0/1_1`** (80812/21488 B) +
`WIFI_RAM_CODE_SOC`. pmOS (btif) cargaba **`mt6572_82_patch_e1_0/e1_1`** (40924/20516 B) = OTRA variante,
la mitad de tamaño. El krillin es `CONSYS_6582` → usa ROMv1. WIFI_RAM_CODE_SOC = IDÉNTICO (md5 8762595d).
**El único delta de firmware es el patch del CONSYS.**

**El firmware estaba BUILT-IN en el vmlinux** (`.gen.o` en `firmware_loader/builtin/`, de un build viejo con
CONFIG_EXTRA_FIRMWARE), por eso reemplazarlo en `/lib/firmware` (disco) NO valía — `request_firmware` carga el
built-in antes que el disco (el primer `-2` = built-in aún no registrado, luego carga los 40KB = 41 frags).
**Verificado**: PATCH_HDR=28, FRAG_SIZE=1000 → 40924B=41 frags, 80812B=81 frags.

**FIX aplicado (#298)**: horneado el ROMv1 como built-in (poner ROMv1 renombrado en `firmware/` + reactivar
CONFIG_EXTRA_FIRMWARE + rm los `.gen` viejos + rebuild). **Confirmado: cargó 81 frags = ROMv1** ✓.

**PERO el resultado rompió el CONSYS**: `openat(/dev/stpgps)=EIO`, y el chip se COLGÓ (`wmt_cmd[GEN_HCR]:
TIMEOUT`, -110, en todos los re-bring-ups; WiFi también `func_on -5`, abandonado tras 40 intentos). La causa
inmediata es el **doble bring-up NO serializado** (WiFi+GPS a la vez cuelgan el chip — problema conocido
[[project-boot-flakiness-consys-race]]), y **el ROMv1 es MUCHO más frágil a él** que el mt6572_82 (que lo
toleraba). → No se pudo probar el GPS con ROMv1 limpiamente; el chip se colgó antes.

### ⏭️ Siguiente (para un test LIMPIO del ROMv1)
1. **Serializar el bring-up** (mutex entre el btif-GPS y el mtwifi, o deshabilitar el WiFi temporalmente) para
   que el ROMv1 no se cuelgue por el doble bring-up → luego probar `/dev/stpgps` + GSV con cielo.
2. Si el ROMv1 sigue dando EIO aun serializado → nuestro **bring-up simplificado no es compatible con el
   ROMv1**; el stock usa el bring-up completo del `wmt_lib` (init_table_1_2/4/5 + patch + calibration + coex +
   crystal_trim) — habría que portarlo entero (proyecto grande).
3. **Reversión disponible**: el mt6572_82 (40KB) en `~/gps-groundtruth/fw-pmos/` (md5 6ea6be78) para volver al
   estado funcional (WiFi/BT OK, GPS ruidoso) recompilando el built-in.

*Mac (Fable), 0715 noche. El delta de firmware CONFIRMADO (ROMv1 vs mt6572_82) y aplicado, pero el ROMv1
cuelga el chip por el doble bring-up no serializado. Falta serializar + re-test, o portar el wmt_lib completo.*

---
## 0715 noche-2: ROMv1 REFUTADO como vía (cuelga el chip) — el patch NO es el delta del GPS
Probado el ROMv1 con re-sync tras el patch (#299): **el chip queda MUDO tras cargar el ROMv1** — 30 intentos
de GEN_HCR de re-sync, TODOS timeout. No es que tarde en re-arrancar: **el ROMv1 no arranca con nuestro
bring-up simplificado.** El stock lo carga con el `wmt_lib` COMPLETO (init_table_1_2/4/5 = query/set/query STP,
multi-patch con metadata, calibration, coex, crystal_trim) — portarlo entero es un proyecto grande, y no está
claro que arregle el GPS.

**Reinterpretación clave**: con el `mt6572_82` (40KB) el **WiFi funciona PERFECTO** (WPA2+DHCP+HTTPS, cerrado),
BT OK, GPS enciende radio + el DSP habla (aunque ruido 0xCA). Si el mt6572_82 fuese "el patch equivocado del
chip", el WiFi no iría. → **el mt6572_82 es válido para nuestro bring-up; el patch NO es el delta del GPS.**
El ruido 0xCA es otra cosa.

**REVERTIDO a #300** (mt6572_82 built-in, GSYNC+COEX sí, RESYNC quitado) = estado funcional (WiFi/BT + GPS
radio-on, GPS ruidoso). `/lib/firmware` restaurado también.

### Candidatos que QUEDAN (los otros 2 del ground-truth, sin tocar aún)
1. **NVRAM de calibración RF de fábrica**: el WIFI NVRAM (`/data/nvram/APCFG/APRDEB/WIFI`, byte 0x6D =
   crystal trim) está VACÍO en pmOS; el stock lo tiene (en Lineage `/data/nvram/...` o protect_f/s). Sin el
   trim de fábrica, el TCXO del CONSYS no se afina — candidato para el GPS (aunque el WiFi lo tolere).
   **Sacar el WIFI NVRAM de LineageOS (adb pull) + portar `crystal_triming_set` al btif.**
2. **Comandos del HAL/mnld del stock al DSP** (`/dev/stpgps` write burst al iniciar sesión): snoopear en
   LineageOS qué escribe mnld al arrancar vs pmOS. Usamos el MISMO libmnlp, pero el HAL `gps.default.so`
   (ausente en pmOS) puede mandar comandos de config RF/referencia que activan la adquisición.

*Mac (Fable), 0715 noche-2. El patch del CONSYS REFUTADO como delta del GPS (ROMv1 cuelga; mt6572_82 válido).
Quedan: NVRAM crystal-trim de fábrica, y los comandos del HAL al DSP. Estado funcional restaurado (#300).*
