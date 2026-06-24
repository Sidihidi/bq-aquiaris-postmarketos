# Handoff WiFi — 2026-06-24 (sesión Claude)

> Subido por la sesión de Claude del 24-jun-2024. **Lee esto antes de tocar el driver.**
> Contiene: estado real verificado en HW, hipótesis del fallo del DHCP + crash del scan,
> qué se probó, qué NO funciona, y por dónde seguir. El driver editado está en esta misma carpeta.

---

## TL;DR — dónde estamos

- **WPA2 asocia + 4-way handshake + claves PTK/GTK instaladas**: ✅ FUNCIONA (`wpa_state=COMPLETED`,
  `key_mgmt=WPA2-PSK`, `pairwise/group=CCMP`, `Key negotiation completed [PTK=CCMP GTK=CCMP]`).
- **Estabilidad (sin reinicios cada 3min)**: ✅ ARREGLADO con `wifi_hif_alive()` guard en rx_thread
  + en los 6 callbacks cfg80211 + rx_thread pausado a 200ms cuando idle. El móvil aguanta >10min
  sin actividad WiFi agresiva.
- **DHCP por WiFi (WPA2)**: ❌ SIGUE FALLANDO. `rx_packets=2` (solo los 2 EAPOL del handshake,
  **0 data frames recibidos**). `tx_packets` sube normal (TX funciona). El AP no responde al
  DISCOVER o el FW no entrega la OFFER al host.
- **`iw dev wlan0 scan` estando conectado**: ❌ CRASHEA el móvil (hard-lockup mudo, `boot_reason=4`
  = WDT reset, sin panic en pstore). A veces deja el móvil inaccesible (necesita power-cycle físico).
- **`enc_status` en el join**: cambiado a `CCMP_KEY_ABSENT(7)` (lo que hace el downstream
  `nic.c:2000`). **No arregló el DHCP ni el scan** — la hipótesis del `enc_status=6` era FALSA
  (ver abajo). El cambio a 7 es correcto por fidelidad al downstream y se mantiene, pero no era
  la causa.

**Conclusión:** el WPA2 de control-plane (assoc + handshake + claves) está RESUELTO. Lo que
falla es el **data-path de RX de datos cifrados** y el **scan estando conectado**. Ambos parecen
tener una **causa común** (el motor de RX del FW + PIO polling sin timeout del driver).

---

## Estado del driver en esta carpeta

`mt6582-wifi.c` y `mt6582-wifi-reg.h` son la versión **desplegada en la Pi** (md5 sincronizado
repo↔Pi, zImage compilado a las 13:10 del 24-jun, flasheado y probado en HW). Contiene:

1. **`wifi_hif_alive()` guard** (línea 117) — comprueba `WCIR` (chip-id) antes de tocar el HIF.
   Si el FW muere del todo (WCIR≠0x6582), aborta el acceso y marca `started=false`. Llamado en:
   - rx_thread (l.793)
   - los 6 callbacks cfg80211: `.scan`(825), `.connect`(1094 vía wifi_wait_grant), `.disconnect`(1158),
     `.add_key`(1187), `.del_key`(1220), `.set_default_key`(1242)
2. **rx_thread pausado a 200ms** cuando idle (l.808-811) — menos tráfico PIO al HIF.
3. **`.del_key` implementado** (l.1211) — `CMD_802_11_KEY add_remove=0`.
4. **`.set_default_key` implementado** (l.1231) — `CMD_ID_DEFAULT_KEY_ID(0x09)`, struct
   `cmd_default_key_id {u8 net_type_idx, key_id, rsv[2]}` añadido al reg.h.
5. **`.add_key` corregido**: `tx_key=pairwise?1:0` (solo PTK es TX-key), GTK→`peer_addr=ff:ff:ff`
   cuando `mac_addr=NULL` (group key), `net_type=AIS`.
6. **`enc_status=CCMP_KEY_ABSENT(7)`** en el join (l.945) — revertido a lo que hace el downstream.
7. **EAPOL**: flag `HIF_TX_FLAG_1X_FRAME` en `wifi_tx_data` (l.695) para que el FW los emita sin cifrar.

> **Nota:** esta versión es la que el móvil está corriendo AHORA. No la pierdas — tiene los fixes
> de estabilidad que funcionan. Si recompilas, usa esta como base.

---

## 🔴 HIPÓTESIS del fallo (lo que falla y por qué)

### A) DHCP sin OFFER — el data-path RX de datos cifrados no entrega

**Síntoma medido en HW:**
- Tras `ASSOC-RESP status=0` + `JoinComplete: data-path ON` + 4-way handshake COMPLETED.
- `rx_packets=2` (los 2 EAPOL del handshake) y **NO suben más** en minutos.
- `tx_packets=5-7` (DHCP DISCOVER + EAPOL TX salen — el TX funciona).
- `wpa_state=COMPLETED` (sigue asociado L2).
- ARPing a la gateway: 0 respuestas.
- Eventos FW post-connect: `0x13` (ACTIVATE_STA_REC, normal) y a los ~31s `0x1b` (BSS_BEACON_TIMEOUT).
  **NO hay `0x19` (BSS_ABSENCE)** — el FW no reporta estar off-channel.

**Lo descartado:**
- `enc_status` en el join: probado con `6 (ENABLED)` y `7 (KEY_ABSENT)` → DHCP falla igual en ambos.
  **No es la causa.** (El downstream usa 7; lo dejo en 7 por fidelidad.)
- Claves mal instaladas: el `.add_key` manda `algorithm_id=CIPHER_CCMP(4)` (verificado contra
  `privacy.h:123 CIPHER_SUITE_CCMP=4`), `net_type=AIS`, GTK con `peer=ff:ff:ff`, RSC copiado.
  El struct `CMD_802_11_KEY` coincide byte a byte con el downstream. Las claves llegan al FW.
- MAC del netdev: ya usa `w->mac` (la del FW), no random — el TX sale con la MAC asociada.
- Filtro RX: `CMD_ID_SET_RX_FILTER 0x0B` (DIRECTED|MULTICAST|BROADCAST) se manda en el join.
- STA a STATE_3: se promueve en el join (`sta_state=STA_STATE_3=2`) — abre Class 1,2,3.

**Hipótesis principal — el FW descarta el RX de datos y no lo entrega al host:**
El FW full-MAC recibe la OFFER cifrada con la GTK, la descifra, pero **no la inyecta en el bus HIF**
hacia el host. Posibles causas:
1. **El motor de RX del FW no está activado para datos.** El downstream manda más comandos en el
   join que nuestro driver no envía (ver "lo que falta investigar" abajo). Candidatos:
   `CMD_ID_SET_BSS_RLM_PARAM(0x1d)` como **comando separado** (nosotros lo llevamos embebido en
   SET_BSS_INFO — quizá el FW lo necesita aparte), `CMD_ID_UPDATE_WMM_PARMS(0x21)`,
   `CMD_ID_INDICATE_PM_BSS_CREATED(0x19)` (no solo CONNECTED).
2. **El `need_resp=1` del UPDATE_STA_RECORD** activa `EVENT_ACTIVATE_STA_REC(0x13)` (lo vemos) pero
   quizá falta confirmar/ack ese evento al FW para que abra el RX-data.
3. **El FW está en power-save a pesar del CAM.** El `0x1b` a los 30s sugiere que el FW duerme
   entre beacons. Quizá el `CMD_ID_POWER_SAVE_MODE` no se aplica bien o falta
   `CMD_ID_INDICATE_PM_BSS_CONNECTED` con los parámetros correctos (beacon_interval/DTIM/AID).
   Revisar `nicPmIndicateBssConnected` del downstream.

**Hipótesis alternativa — el TX del DISCOVER sale mal cifrado y el AP lo descarta:**
Menos probable (tx_packets sube y el EAPOL TX funcionó para el handshake), pero posible: el FW
cifrar el DHCP DISCOVER con la PTK, pero si `set_default_key` no se llama (wpa_supplicant **NO**
lo llama en nuestros logs — buscar por qué), el FW no sabe qué clave es TX-default y lo manda
en claro → el AP WPA2 lo descarta. **Acción:** forzar `set_default_key` manualmente tras el
handshake y ver si el DHCP empieza a funcionar.

### B) `iw dev wlan0 scan` crashea estando conectado

**Síntoma medido en HW:**
- Scan en boot limpio (sin conectar) funciona (ve "hola" y asocia).
- Scan **estando conectado WPA2** → crash **instantáneo** (sin log, `boot_reason=4` WDT).
- A veces el móvil queda inaccesible (WDT no resetea o gadget USB no reenumera → power-cycle físico).
- El pstore del boot anterior muestra el log cortado en un `0x1b` o sin nada tras el connect.

**Hipótesis principal — el rx_thread se cuelga en un `readl` y retiene el `hif_lock`:**
1. Tras el connect WPA2, el FW entra en estado frágil (el `0x1b` a los 30s lo confirma: pierde
   beacons, posiblemente el motor de RX está atascado).
2. El rx_thread hace `wifi_rx_drain()` → `rd(w->hif, MCR_WRPLR)` (l.757) y se **cuelga** en la
   transacción del bus AHB (el FW no responde el data-port, pero `WCIR` sigue dando 0x6582).
3. El `wifi_hif_alive()` guard **NO lo pilla** porque lee `WCIR` (estático) y ese registro sigue
   respondiendo aunque el data-port (`WRDR0/WRPLR`) esté colgado. **Este es el agujero del guard.**
4. El rx_thread retiene `hif_lock` colgado. Tu `iw scan` hace `mutex_lock(hif_lock)` (l.824) y se
   bloquea esperando para siempre → hard-lockup del CPU → WDT reset a los 31s.

**Por qué el scan en boot limpio funciona:** el FW está fresco, el data-port responde, el
rx_thread no se cuelga, el mutex está libre.

**Hipótesis secundaria — el scan en sí coloca al FW off-channel y eso lo atasca:**
El `CMD_SCAN_REQ_V2` manda al FW a escuchar otros canales. Estando conectado, el FW tiene que
pausar el BSS actual (BSS_ABSENCE 0x19), escanear, y volver. Si esa máquina de estados del FW
falla (canal home mal fijado, o el CH_ABORT del join no se hizo bien), el FW se queda off-channel
permanente → pierde beacons → 0x1b → se atasca. El downstream maneja esto con
`aisFsmReleaseCh` + `CMD_CH_PRIVILEGE action=ABORT` que nosotros sí mandamos, pero quizá falta
re- grant del canal home tras el scan.

---

## 🔬 Lo que se probó en esta sesión (cronología)

1. Audité el driver en la Pi + repo. Encontré que los fixes de estabilidad (guard + rx_thread
   pausa) ya estaban aplicados por una sesión previa, pero el kernel flasheado NO los llevaba
   (zImage viejo). `make clean` + rebuild + flash → móvil estable (>10min sin reinicios).
2. WPA2 connect a "hola"/"kakatua1": handshake COMPLETED en 3s. ✅
3. DHCP: `udhcpc` 3x DISCOVER sin OFFER. `rx=2 tx=5`. ❌
4. Cambio `enc_status 6→7` (hipótesis del descarte de beacons). Rebuild + flash. WPA2 OK pero
   DHCP sigue `rx=2`. **Hipótesis FALSA** — no era enc_status.
5. `iw dev wlan0 scan` estando conectado WPA2: **crash instantáneo**, móvil inaccesible
   (necesitó power-cycle físico). El crash del scan **NO se arregló** con enc_status=7.

---

## ▶️ Por dónde seguir (prioridad)

### 🔴 1. Arreglar el crash del scan PRIMERO (te impide debuggear)
Sin esto, cada prueba de data-path que requiera scan recae en un crash. Dos caminos:
- **(A) Workaround rápido — timeout en el rx_thread:** en `wifi_rx_drain`, si `rd(WRPLR)` tarda
  más de N iteraciones en devolver algo coherente (o si el valor es sospechoso), marcar
  `started=false` y dejar de acceder al HIF. En ARM los `readl` MMIO no tienen timeout portable,
  pero se puede usar `readx_poll_timeout_atomic()` con un timeout corto, o rodear el rx_thread
  con un watchdog que detecte si una iteración dura >200ms (signo de bus hang) y mate el driver.
- **(B) Fix estructural — PDMA + IRQ (el TODO del driver, l.152):** migrar el RX de PIO polling
  a PDMA (`0x11000180`) + IRQ del HIFSYS (`WHISR`). El DMA tiene timeout y no bloquea el CPU si
  el FW se atasca. Es la solución real. Es trabajo de medio día pero mata el crash de raíz y
  elimina el polling de 20ms (consume CPU). **Recomendado.**

### 🟠 2. DHCP — comparar la coreografía del join con el downstream comando a comando
El downstream `nic.c:2000` y `aisFsmRunEventJoinComplete` mandan una secuencia de comandos tras
el ASSOC. Nuestro `wifi_send_join()` manda: SET_BSS_INFO, UPDATE_STA_RECORD, POWER_SAVE_MODE,
SET_RX_FILTER, INDICATE_PM_BSS_CONNECTED, CH_PRIVILEGE(ABORT). **Falta comprobar si el downstream
manda más.** Pasos:
- Leer `aisFsmRunEventJoinComplete` en `mgmt/ais_fsm.c` del downstream
  (`~/mainline/downstream/.../drv_wlan/mt6628/wlan/mgmt/ais_fsm.c`) y listar TODOS los CMD_ID que
  manda tras el assoc, en orden. Comparar con `wifi_send_join()`.
- Candidatos a añadir: `CMD_ID_SET_BSS_RLM_PARAM(0x1d)` separado, `CMD_ID_UPDATE_WMM_PARMS(0x21)`,
  `CMD_ID_INDICATE_PM_BSS_CREATED(0x19)`.
- Verificar que `EVENT_ACTIVATE_STA_REC(0x13)` (llegan a los 0.03s post-connect) no requiera un
  ack/comando del host para abrir el RX-data.

### 🟠 3. DHCP — forzar `set_default_key` manualmente
wpa_supplicant en nuestros logs **NO llama** `set_default_key` (buscar por qué — quizá falta
declarar `wiphy->max_num_keys` o un flag). Probar: tras el handshake, lanzar a mano
`iw dev wlan0 set key 0` (o equivalent nl80211) y ver si el DHCP empieza a funcionar. Si sí,
el problema era que el FW no sabía qué clave era TX-default.

### 🟡 4. Capturar el RX con más instrumentación
Añadir a `wifi_rx_drain()` un log de `rx_packets` y del `packet_type` de cada frame recibido
(ahora solo loguea mgmt/events). Así veremos si la OFFER llega al HIF y se descarta en el driver,
o si no llega ni al HIF (FW no la inyecta). Punto clave para aislar FW vs driver.

---

## 🛠️ Cómo reproducir el test WPA2 (sin crashear)

**NO hagas `iw dev wlan0 scan` estando conectado — crashea.** Flujo seguro:
```sh
pkill wpa_supplicant; pkill udhcpc; ip link set wlan0 up; ip addr flush dev wlan0
printf 'ctrl_interface=/var/run/wpa_supplicant\nnetwork={\n  ssid="hola"\n  psk="kakatua1"\n  key_mgmt=WPA-PSK\n  proto=RSN\n  pairwise=CCMP\n  group=CCMP\n  scan_ssid=1\n}\n' > /tmp/wpa.conf
# wpa_supplicant hace su propio scan interno (scan_ssid=1) — NO iw scan manual
wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -dd -t > /tmp/wpa.log 2>&1 &
# esperar COMPLETED (3-5s), luego DHCP rápido (ventana de 30s antes del 0x1b)
wpa_cli -i wlan0 status | grep wpa_state   # debe dar COMPLETED
udhcpc -i wlan0 -t 3 -T 2 -n               # 3 DISCOVER, falla sin OFFER
cat /sys/class/net/wlan0/statistics/rx_packets   # =2 (solo EAPOL)
```
Red: SSID **"hola"**, WPA2-PSK, pass **"kakatua1"**, BSSID `e6:92:82:f4:ce:44`, ch=1.

## 🛠️ Entorno
- **Pi de build**: `ssh cpcd@192.168.0.123` (sudo NOPASSWD). Kernel `~/mainline/linux-7.0.12`,
  build `O=build-krillin`. Driver en `drivers/soc/mediatek/mt6582-wifi.c`.
- **Móvil**: `ssh root@172.16.42.1` (USB gadget, la Pi es `172.16.42.2/24` en `usb0`).
  `sudo ip addr replace 172.16.42.2/24 dev usb0` para reconectar.
- **Build + flash**: `~/wifi-iter-w.sh` en la Pi (build zImage + empaqueta + dd sector 83968 +
  reboot). O manual: `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage
  dtbs` → empaquetar en `~/mainline/pkg` → `dd` sector 83968 con verify md5.
- **Downstream de referencia**: `~/mainline/downstream/drivers/misc/mediatek/combo/drv_wlan/mt6628/wlan/`.
- **pstore (último crash)**: `/sys/fs/pstore/console-ramoops-0` en el móvil. **Léelo antes de
  reiniciar de nuevo** o se sobreescribe.

## ⚠️ Reglas de oro (no cambiar)
- `fastboot flash boot` NUNCA `fastboot boot`. mtkclient: `wo` NUNCA `wf`. No flashear preloader.
- No restaurar LK Lollipop (bueno = KitKat 1.5.2).
- Si el móvil se queda inaccesible tras un crash del scan: **power-cycle físico** (Power ~10s o
  quitar batería). El WDT a veces no resetea cuando el bus AHB está colgado del todo.
- No martillear SSH anidado Pi→móvil (satura el sshd/musb). Usar el wrapper base64 (ver
  `CONTINUAR-AQUI-0624.md`).

---

## 📂 Ficheros en esta carpeta
- `mt6582-wifi.c` — driver editado (versión desplegada + probada en HW el 24-jun).
- `mt6582-wifi-reg.h` — regs/structs (con `cmd_default_key_id` añadido).
- `HANDOFF-0624.md` — este documento.

*Co-autor: Claude (glm-5.2).*
