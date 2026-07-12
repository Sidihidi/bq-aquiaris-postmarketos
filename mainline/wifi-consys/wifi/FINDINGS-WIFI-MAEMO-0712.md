# WiFi en Maemo — validado 0712 (paridad con pmOS)

> Con el firmware `WIFI_RAM_CODE` ya en `/lib/firmware/mediatek/mt6582/` (horneado previo, tarea #17),
> el WiFi de Maemo Leste **arranca y funciona igual que en pmOS**. Validación end-to-end abajo.

## Validado (Maemo Leste, Debian trixie, init OpenRC)
- **Firmware presente**: `/lib/firmware/mediatek/mt6582/WIFI_RAM_CODE` (207648 B). Antes daba
  "WIFI_RAM_CODE no mapeado / func_on(WIFI) fallo -5"; ahora carga.
- **Driver arranca**: dmesg `mtk_mtwifi ... wlan0 ARRIBA (FW arrancado, cfg80211 registrado)` →
  renombrada a `wlan1`. CONSYS completo: BT (hci0) + GPS (/dev/stpgps) + WiFi los tres ON.
- **Scan OK**: `iw dev wlan1 scan` encuentra redes reales (vodafoneA544, hola).
- **Asociación + DHCP + red local OK**: asoció a vodafoneA544, DHCP dio IP 192.168.0.102, y **ping al
  gateway (.42) y a .1 = 2/2** (unicast local perfecto, ARP resuelve).

## El hueco de internet = la limitación CONOCIDA del driver (TKIP GTK RX), NO un bug de Maemo
Al asociar a vodafoneA544 el cliente cogió el **BSSID del repetidor `18:a6:f7:c0:a6:8b` con
Group cipher = TKIP** (Pairwise CCMP TKIP). Síntoma exacto del hueco documentado
(`FINDINGS-TKIP-GTK-0705.md`): **unicast local OK, pero internet/DNS fallan** (broadcast/GTK roto).
- Hay un BSSID CCMP puro `08:7e:64:ef:f5:90` (el router bueno) pero a **−68 dBm** desde la posición
  actual → la asociación es marginal/inestable (asocia y cae). pmOS evita esto **fijando el BSSID
  CCMP** cuando está en rango (autoconnect prio 10); en Maemo el mismo truco funcionaría en rango.
- ⇒ WiFi en Maemo = **paridad con pmOS**. El internet vía el repetidor TKIP es el mismo límite de
  driver en ambos SO, no algo específico de Maemo. Fix real = arreglar RX con GTK TKIP (tarea grande
  documentada).

## Para uso diario en Maemo
La GUI de conectividad (icd2 / osso-connectivity-ui) puede gestionar el WiFi ahora que el firmware
carga; conectará al AP CCMP cuando esté en rango. (Las pruebas CLI de esta sesión se limpiaron; no se
dejó wpa_supplicant manual para no pelear con la GUI.)

## De paso, confirmado en Maemo esta sesión
- **Carga arreglada y PERSISTE** (daemon `fan5405-charge.py` + servicio OpenRC + rc.local; VBAT 4144 mV
  cargando) = resuelta la causa de los apagados.
- **Init = OpenRC** (no systemd); boot **sin servicios crashed/failed** (Maemo no sufre la cascada
  `root`/`localmount` que sí rompía pmOS). Reloj correcto (RTC compartido, puesto desde pmOS).

*Sesión principal (Fable 5), 2026-07-12.*
