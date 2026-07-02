# HANDOFF — WiFi mainline MT6582: estado, causas raíz y PLAN DE ACTUACIÓN (2026-06-25)

Sesión larga (~19 ciclos build+flash+reboot) sobre `mt6582-wifi.c`. Honesto: **NO está resuelto
end-to-end** (sin DHCP → sin internet), pero **se ha cerrado mucho el círculo** y se han arreglado
varios hard-lockups. Driver resultante: **kernel #171** (en este repo, `mt6582-wifi.c`).

> Cada conclusión de abajo está respaldada por evidencia en HW (migas de log + `ramoops` + logs
> persistentes en `/root`), no por suposición.

---

## TL;DR — estado actual

| Subsistema | Estado |
|---|---|
| WiFi en la GUI (escanear/listar redes) | ✅ funciona |
| Asociación WPA2 (AUTH/ASSOC/4-way handshake) | ✅ `wpa_state=COMPLETED`, PTK+GTK instaladas |
| TX cifrado (los DHCP DISCOVER SALEN) | ✅ |
| **RX de datos (OFFER del DHCP, beacons)** | ❌ **el FW deja de RECIBIR ~1s tras conectar** — PRIORIDAD |
| Cuelgue del `.disconnect` | ✅ arreglado |
| Cuelgue post-disconnect (idle, +5s) | ✅ arreglado |
| Reconectar en el MISMO boot | ❌ cuelga |
| Activación de la conexión por NM (GUI connect) | ❌ no completa (manual sí) |

---

## Lo que se RESOLVIÓ esta sesión (con evidencia)

### 1. WiFi en la GUI — NO hacía falta recompilar Phosh
El handoff anterior (`HANDOFF-CASA-0624.md`) decía "recompilar Phosh con `-Dplugins` en WSL". **Era un
diagnóstico equivocado** (los toggles WiFi/BT de Phosh son del *core*/libnm, no de `-Dplugins`, que son
widgets de la pantalla de bloqueo). Causas reales, dos:
- Faltaba el paquete **`networkmanager-wifi`** → `apk add networkmanager-wifi` (necesita DNS:
  `printf 'nameserver 8.8.8.8\n' > /etc/resolv.conf`).
- `wlan0` estaba **unmanaged** → `/etc/NetworkManager/conf.d/20-wlan0-managed.conf`:
  ```
  [device-wlan0-managed]
  match-device=interface-name:wlan0
  managed=1
  ```
Tras eso NM escanea y lista redes → Phosh las muestra. (NM auto-activa su `wpa_supplicant -u` por D-Bus.)

### 2. Causa raíz de los hard-lockups del HIF: propiedad FW_OWN / driver-own (WHLPCR)
El FW del WiFi puede **TOMAR la propiedad del HIF** (power-save / transiciones). Leer/escribir un
registro NORMAL (WCIR/WHIER/WRDR0/WTDR0/WRPLR) mientras el FW es dueño **NO completa en el bus AHB →
hard-lockup mudo → WDT a 31s**. El guard viejo leía WCIR (también colgaba). **Fix:** `wifi_hif_alive()`
reimplementado para asegurar driver-own vía **WHLPCR** (registro *always-on*: leerlo/escribirlo NUNCA
cuelga), e invocado al inicio de las 4 funciones de puerto. (= `ACQUIRE_POWER_CONTROL_FROM_PM` +
`nicpmSetDriverOwn` del downstream, `ahb.c:1222`, `nic_pwr_mgt.c:313`.)

### 3. Causa raíz del cuelgue del DISCONNECT (PROBADA con migas DIAG por-comando)
El comando **`SET_BSS_INFO(conn_state=DISCONNECTED)` CUELGA el FW** desde su lado (arrastra el bus
AHB/CONSYS), **incluso con el struct bien formado** (`saved_bi`). Demostrado: la miga "DIAG disc B"
(justo tras mandar el SET_BSS_INFO) fue la última antes del cuelgue. **Fix:** NO mandar ese comando;
desconectar con `UPDATE_STA_RECORD(STATE_1)` + `BSS_ACTIVATE(active=0)` (comandos que el connect ya usa
sin colgar). → el `.disconnect` ya **sobrevive**.

### 4. Cuelgue post-disconnect a +5s
El `rx_thread` sondeaba el data-port (WRPLR) SIEMPRE, también en IDLE. Tras desconectar, el FW deja el
data-port en estado "cuelga al leer" (WHLPCR sano = el caso PDMA del audit). **Fix:** el `rx_thread`
solo drena cuando de verdad espera RX (scan activo / connecting / connected / TX pendiente). En idle no
toca el HIF. → **disconnect + 30s idle: sobrevive (probado)**.

---

## Los 3 BUGS QUE QUEDAN

### A) 🔴 DHCP — el RX se degrada ~1s tras conectar  (EMPEZAR POR AQUÍ)
**Síntoma, probado con flujo MANUAL `wpa_supplicant`+`udhcpc` (sin NM, sin crash):**
```
wpa_state=COMPLETED   pairwise=CCMP  group=CCMP
udhcpc: broadcasting discover ×8  →  "failed to get a DHCP lease"
tx=8  rx=2     <- los DISCOVER salen (TX cifrado OK), pero solo llega el EAPOL inicial.
               <- ni la OFFER del DHCP ni los beacons -> EVENT_ID_BSS_BEACON_TIMEOUT (0x1b) a +30s.
```
- Pasa con **AMBOS** APs probados: vodafoneA544 (grupo TKIP) **y** "hola" (grupo CCMP confirmado), y en
  **canal fijo** → **NO es el AP**, ni el cifrado de grupo, ni NM.
- En el build **#162** (baseline, antes de mis cambios) el DHCP-WPA2 SÍ funcionó (lease real
  `10.181.211.99`, la conexión aguantó 31 min).
- **Dos hipótesis:** (a) **regresión** de los fixes #163→171; (b) la **flakiness de RX pre-existente**
  que ya documentaban los handoffs (scan a 0 beacons, WLAN_READY flaky). La bisección lo decide.

### B) 🟡 Reconnect-crash (2ª conexión del MISMO boot)
Tras un disconnect (que ya sobrevive), una **2ª conexión** cuelga — el teardown deja el data-port del FW
degradado. **Path:** re-init del FW en el `.disconnect` (re-bringup del MAC WiFi, ojo que el CONSYS lo
comparte el BT), o el data-path **PDMA+IRQ** (el "fix real" del audit, fase grande).

### C) 🟡 NM no completa la activación de la conexión
`nmcli connection up` se queda esperando aunque la conexión esté asociada y con `ipv4.method=disabled`;
el `wpa_supplicant` **MANUAL** sí llega a COMPLETED en 2 s. Es un problema de integración NM↔driver
(¿el driver no señala bien el resultado de conexión a la instancia de wpa_supplicant de NM?).

---

## ⭐ PLAN DE ACTUACIÓN — BISECCIÓN del DHCP

Objetivo: decidir si los fixes #163→171 rompieron el RX, o si el RX siempre fue flaky.

**Paso 0 — ¿el baseline #162 coge IP?**
1. En la Pi: `cp ~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-wifi.c.bak-pre-gate-0624 \`
   `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-wifi.c`  (= #162, antes de TODOS mis cambios).
2. `bash ~/wifi-iter-w.sh` (build+flash+reboot).
3. **Test DHCP manual** (receta abajo), en la 1ª conexión del boot.
   - **Coge IP** → mis fixes rompieron el RX → **Paso 1**.
   - **NO coge IP** → el RX es flaky de base → sección "Si el RX es flaky".

**Paso 1 — bisectar (si #162 funciona).** Re-aplicar los backups en ORDEN cronológico y testear DHCP en
cada build. El primero que falle = el fix culpable:
```
bak-pre-gate-0624 (=#162) -> +gate(0624b) -> +driver-own(0624c) -> +settle(0624d)
   -> +teardown(0624f) -> +noSBI/disconnect(0624g) -> +idledrain(0624h) -> #171
```
Cada `*.bak-pre-X` = el estado ANTES de aplicar X. (P.ej. para probar "con gate pero sin driver-own",
flashear `bak-pre-own-0624c`.)
**Sospechoso nº1 del RX:** el **guard driver-own (0624c)** — añade una lectura de WHLPCR ANTES de cada
lectura de puerto (`wifi_port_read_pio`), lo que podría alterar el timing/estado del RX del FW. Nº2: el
**idle-drain (0624h)**.

**Paso 2 — arreglar el RX** manteniendo los fixes de crash; re-verificar DHCP + estabilidad.

**Si el RX es flaky de base** (Paso 0 falla): no es regresión. Mirar `reference_mt6582_wifi_hif` /
`reference_mt6582_boot_stability`; el RX del FW (beacons + datos) es intermitente. Posibles causas: la
descarga de FW / cal RF no determinista, o el RX_FILTER / enc_status. El "fix real" es **PDMA+IRQ**
(sacar el RX del sondeo PIO).

---

## Recetas de test

### DHCP MANUAL (el test FIABLE — sin NM, sin reconnect-crash)
```sh
nmcli device set wlan0 managed no; sleep 2
pkill -f 'wpa_supplicant.*wlan0'; pkill -f 'udhcpc.*wlan0'; ip link set wlan0 up
printf 'ctrl_interface=/var/run/wpa_supplicant\nnetwork={\n ssid="hola"\n psk="kakatua1"\n key_mgmt=WPA-PSK\n scan_ssid=1\n}\n' > /tmp/wpa.conf
wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -B
wpa_cli -i wlan0 status            # esperar wpa_state=COMPLETED (2 s)
udhcpc -i wlan0 -t 6 -T 2 -nq      # ¿lease?
ip -4 addr show wlan0              # ¿IP?
cat /sys/class/net/wlan0/statistics/{rx,tx}_packets   # rx>2 = la OFFER llegó
```
> ⚠️ **El bloque `network={...}` DEBE ir multilínea** (un campo por línea) o wpa_supplicant lo rechaza.
> ⚠️ Logs de test: escribir a **`/root/...`** y `sync` (NO a `/tmp` — es tmpfs, se pierde al reboot).
> Tras el test: `nmcli device set wlan0 managed yes; rc-service networkmanager restart` para la GUI.

### Leer un hard-lockup (cuelgue mudo → WDT)
`tail -40 /sys/fs/pstore/console-ramoops-0`  (la ÚLTIMA línea = dónde colgó). `boot_reason=4` en
`/proc/cmdline` = reset por WDT.

---

## Acceso / infraestructura
- **Pi** (host de build): `cpcd@192.168.0.38` (sudo passwordless). Árbol kernel `~/mainline/linux-7.0.12`.
  Build SOLO del WiFi: `bash ~/wifi-iter-w.sh` (sube el .c, compila `zImage`, `dd` a **sector 83968**,
  reboot). Downstream de referencia: `~/mainline/downstream/.../combo/drv_wlan/mt6628/wlan`.
- **Móvil** (USB, desde la Pi): `root@172.16.42.1`. Si "No route to host":
  `sudo ip addr replace 172.16.42.2/24 dev usb0` en la Pi. **Boot flaky** (~1/4 se cuelga → power-cycle
  físico; síntoma: el gadget USB no enumera). `boot_reason=4` casi siempre tras los tests (WDT del crash).
- **Backups** del driver en la Pi (cadena de bisección):
  `mt6582-wifi.c.bak-pre-{gate-0624, own-0624c, settle-0624d, teardown-0624f, noSBI-0624g, idledrain-0624h}`.
- **APs de prueba:** `hola`/`kakatua1` (CCMP, **salta de canal**), `vodafoneA544` (WPA/WPA2 mixto,
  TKIP-grupo, canal fijo). Ambos fallan el DHCP igual → el AP NO es la causa.

## Nota sobre el driver #171 (este commit)
Lleva **logs de diagnóstico** (`DIAG disc A/B/C`, `DIAG rx_drain post-disc`, el log del wake driver-own)
que sirvieron para localizar las causas; se pueden quitar una vez cerrado el RX. Los **fixes
funcionales**: `wifi_hif_alive` (WHLPCR/driver-own), gate de scan (no scan-while-connected), ventana
settle post-disconnect, teardown del disconnect SIN `SET_BSS_INFO`, y el idle-drain del `rx_thread`.
