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
