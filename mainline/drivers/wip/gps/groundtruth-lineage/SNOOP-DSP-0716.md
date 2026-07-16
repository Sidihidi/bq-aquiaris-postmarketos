# Snoop del stock GPS (LineageOS) + candidato DE-SENSE — 0716

Boot a LineageOS (GPS a 13 sats en GPSTest). `strace` del `mnld` stock (`/system/xbin/strace` existe).

## fds del mnld stock
- **fd 13 = `/dev/stpgps`** — frames `AAF0` que mnld MANDA al DSP (query/report; formato IGUAL que pmOS).
- **fd 14 = `/dev/gps`** — NMEA (con posición real: `$GPGGA,...,3735.9898,N,00058.7826,W,...`).
- fd 8 = pwrctl (escribe "0"). fd 17 = fichero (bloques 1024B con "15012801").
- El DSP del stock RESPONDE con datos reales; el de pmOS con ruido `0xCA` — **con el MISMO libmnlp**
  → confirma que el delta NO es mnld, es la init RF del CONSYS (kernel).

## REFUTADOS con el NVRAM del stock (adb pull)
- **crystal-trim**: `/data/nvram/APCFG/APRDEB/WIFI` byte 0x6D = **0x00** (todo el contexto 0x68-0x77 = ceros).
  El stock NO aplica trim y ve 13 sats → **crystal-trim NO es el delta.**
- **GPS NVRAM**: `/data/nvram/APCFG/APRDEB/GPS` = IDÉNTICO a pmOS (`80 ba 8c 01`=TCXO 26MHz, `aa 58`).
- Ficheros guardados: `nvram/WIFI` (514B), `nvram/GPS` (54B).

## ★ CANDIDATO NUEVO Y FUERTE: DE-SENSE (coexistencia WiFi/BT ↔ GPS)
En Lineage con GPS a 13 sats, **el WiFi está ON** (wlan0 up, wifi_on=1, bt_on=1). El stock, en
`wmt_func_gps_on` (source GPL), tras `func_on(GPS)` **si BT_ON||WIFI_ON manda un DE-SENSE**:
```c
if (BT_ON || WIFI_ON) { ctrlPa1=1; ctrlPa2=0; wmt_core_ctrl(WMT_CTRL_BGW_DESENSE_CTRL,...); }
```
Comando WMT (wmt_core.c:1991): `WMT_BGW_DESENSE_CMD = 01 0e 0f 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00`
(EVT `02 0e 01 00 00`). ⚠️ Pero NO es directo: `wmt_ctrl_bgw_desense_ctrl` hace `wmt_dev_send_cmd_to_daemon(1)`
→ el daemon userspace lee el PERFIL de de-sense y llama de vuelta con el payload real (bytes 5+). El payload
por defecto de la declaración es `02 00 00...`.

**Hipótesis**: en pmOS el WiFi arranca solo (auto-connect) y NUNCA mandamos el de-sense → el WiFi/BT ahogan
la señal GPS débil (-130dBm) → el correlador ve `0xCA` = ruido de AMPLITUD (encaja con el diagnóstico).
Casa probó "bajar wlan0" (insuficiente: no apaga el RF del CONSYS, solo la interfaz).

## ⏭️ Test/fix (requiere volver a pmOS #300 — por `fastboot flash boot` desde Lineage)
1. **TEST barato (valida la hipótesis, sin build)**: módulo que llame `mt6582_consys_func_off(WMTDRV_TYPE_WIFI)`
   (+BT) — exportado por el btif — para apagar el RF del WiFi/BT, y leer `/dev/stpgps`: ¿sigue `0xCA` o mejora?
   Si mejora → interferencia CONFIRMADA.
2. **FIX**: (a) el de-sense (complejo: daemon+payload), o (b) más simple — **no encender el WiFi mientras el GPS
   busca fix** (gate en el bring-up / servicio), o mandar el de-sense básico (`01 0e 0f 00 02...`) desde el btif.

*Mac (Fable), 2026-07-16. Snoop del stock: mnld idéntico (delta no es mnld). Crystal-trim y GPS-NVRAM
REFUTADOS. Candidato fuerte: DE-SENSE (interferencia WiFi/BT↔GPS). Test = func_off WiFi en pmOS.*

---
## Test del de-sense en pmOS #300 (0716 tarde) — NO concluyente + hipótesis debilitada
Módulo `funcoff.ko` (`mt6582_consys_func_off(type)`, exportado): `off=3`(WiFi)=**0 OK** la 1ª vez, pero
`off=0`(BT)=**-5 (EIO)** justo después → **apagar el WiFi por func_off DESESTABILIZA el chip** (el WMT deja de
responder; el motor abre `/dev/stpgps` pero el DSP no emite). No viable como test. `func_off` es más brusco que
el de-sense del stock (que NO apaga el WiFi, solo ajusta la coexistencia).

**Pega de fondo que debilita la hipótesis de interferencia**: en pmOS `wlan0` está **DOWN** (idle, no
transmitiendo) y el GPS igual da `0xCA`. Si fuera interferencia por transmisión del WiFi, con el WiFi idle el
GPS debería ir mejor. (El de-sense se manda si BT_ON||WIFI_ON; el BT sí está on en pmOS.)

### Estado: candidatos baratos AGOTADOS
Descartados: GPIOs/rails/LNA (idénticos), crystal-trim (0x00 en ambos), GPS-NVRAM (idéntico), mnld (mismo
binario/mismos comandos), patch mt6572_82 (WiFi/BT OK), interferencia-por-transmisión (WiFi idle). Lo que
queda son las vías CARAS: (a) portar el **`wmt_lib` completo** del stock para poder cargar el ROMv1 (el patch
correcto del CONSYS_6582) — proyecto grande; (b) **de-sense real** (kernel→daemon→payload) — complejo y dudoso
(WiFi idle). El **TEST LIMPIO de interferencia** que falta: recompilar SIN el mtwifi (WiFi nunca hace func_on)
→ GPS con el RF del WiFi OFF desde el boot, sin desestabilizar → si mejora, interferencia confirmada.

⚠️ El chip quedó DEGRADADO tras el `func_off` (WMT no responde) → necesita power-cycle para recuperar WiFi/BT/GPS.

*Mac (Fable), 2026-07-16 tarde. Test func_off no viable (desestabiliza). Interferencia-por-transmisión
debilitada (WiFi idle). Candidatos baratos agotados; quedan las vías caras (wmt_lib completo / de-sense).*
