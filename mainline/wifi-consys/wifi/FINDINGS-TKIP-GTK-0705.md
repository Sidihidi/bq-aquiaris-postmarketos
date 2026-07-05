# WiFi: segunda red validada (WPA2/CCMP ✓) + sospecha GTK TKIP (2026-07-05)

## Lo validado en la red real `vodafoneA544` (router Vodafone + repetidor TP-Link)
- **BSS del router (`08:7e:64:ef:f5:90`, WPA2 puro, grupo CCMP)**: asociación + 4-way + DHCP
  (`192.168.0.109/24`) + **RX de broadcast OK** (la Pi resuelve el ARP del móvil y entra por
  **SSH nativo por WiFi** — `SSH-POR-WIFI-NATIVO-OK`). Canal 13 sin problema regulatorio.
- **BSS del repetidor (`18:a6:f7:c0:a6:8b`, WPA1+WPA2 mixto, grupo TKIP)**: asociación + 4-way +
  DHCP **OK**, unicast bidireccional **OK** (ping móvil→Pi 0% loss con pairwise CCMP), pero
  **RX con clave de GRUPO roto**: el móvil no ve los ARP broadcast ajenos → ARP del GW `FAILED`,
  Pi→móvil "No route to host". El ping al GW falla por eso.

## Config de rescate dejada en el móvil
- `vodafoneA544` fijada al **BSSID del router** (grupo CCMP) + `autoconnect yes` priority 10.
- `hola-test` autoconnect priority 5. SSH por WiFi = `root@192.168.0.109` (lease estable por MAC).
- El default route sigue por usb0 (Pi NAT) cuando está conectada; la ruta connected de wlan0
  permite el SSH LAN↔móvil.

## Diagnóstico del fallo TKIP (estado: 2 sospechosos, falta el test decisivo)
**Hecho**: la ruta de instalación del GTK es idéntica para CCMP y TKIP y el grupo CCMP funciona
→ si es el driver, es específico de TKIP (MICs Michael o config de cifrado por-clave en el FW).

**Descartado ya**:
- El swap de MICs del glue NO es sospechoso por sí mismo: nuestro `mtk_cfg80211_add_key` es copia
  1:1 del stock conn_soc (`gl_cfg80211.c` del downstream, en la Pi:
  `~/mainline/downstream/drivers/misc/mediatek/conn_soc/drv_wlan/mt_wifi/wlan/os/linux/`), que
  hace el mismo swap ciego 16-23↔24-31 para TODA clave de 32 bytes (igual que su `gl_wext.c`,
  "switch tx/rx MIC key for sta").
- `wlanoidSetAddKey` (core) mapea 32 bytes → `CIPHER_SUITE_TKIP` correctamente (leído en el árbol).

**Sospechoso A — driver/FW**: algo TKIP-específico en el core dieteado o en la config FW
(¿RSC de grupo no pasado? nuestro glue ignora `params->seq`, igual que el stock cfg80211…
el wext stock SÍ pasaba `rx_seq` → diferencia real stock-android vs nuestro flujo).
**Sospechoso B — el repetidor TP-Link**: hace MAC-translation (modo extender); su reenvío de
broadcast hacia clientes WiFi puede estar roto/peculiar con clientes "raros". El móvil SÍ resolvió
el ARP del propio repetidor (.42 REACHABLE) pero no el del GW a través de él.

**Test decisivo pendiente**: asociar el móvil a un AP con grupo TKIP SIN repetidor de por medio
(reconfigurar el AP de `hola-test` a WPA-mixto/TKIP si es controlable) y comprobar si la Pi/host
resuelve el ARP del móvil. Si falla ahí también → driver (probar: GTK sin swap de MICs; pasar
`params->seq` a `rKeyRSC`; mirar eventos MIC-failure con debug REQ/RSN en WARN).

## Cómo reproducir el estado
```
nmcli con modify vodafoneA544 802-11-wireless.bssid ""    # liberar BSSID = puede caer al repetidor
nmcli con modify vodafoneA544 802-11-wireless.bssid 08:7E:64:EF:F5:90   # fijar al router (CCMP)
```
