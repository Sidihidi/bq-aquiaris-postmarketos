# 2026-07-07 · frente wifi — ¡WPA2+DHCP+datos FUNCIONAN! (el bug no existía)

## TL;DR
**El port del stock `mt_wifi` (`mtk_mtwifi`) funciona completamente.** WPA2 handshake ✅,
cifrado L2 ✅, DHCP ✅, datos a internet ✅, **HTTPS real** ✅ (wget de google.com = 81 KB).

El "DHCP falla / 0 data RX / FW no cifra" que dominó 6 semanas de handoffs (0624→0702)
**era un malentendido de rutas de red**, no un bug del crypto-path del firmware. El driver
estaba bien; solo no se había probado forzando el tráfico por wlan0.

## Hecho (probado en HW, 2026-07-07 ~16:00 UTC)
- **WiFi WPA2 a `vodafoneA544`** — handshake COMPLETED, asociado, señal −65 dBm.
- **DHCP** — `udhcpc` obtiene lease `192.168.0.104/24` del servidor `192.168.0.42`.
- **Datos a internet forzando ruta por wlan0:**
  `ping 8.8.8.8` 0% loss (~48 ms) · `ping 1.1.1.1` 10/10, 0% loss (~33 ms, jitter bajo) ·
  `nslookup google.com 192.168.0.1` resuelve · **`wget https://www.google.com` = 81 KB OK**.
- **Fix #1 — stats del netdev (commit en este push):** el driver acumulaba rx/tx en
  `prGlueInfo->rNetDevStats` pero `mtk_netdev_ops` no tenía `.ndo_get_stats` → el kernel
  caía a `dev->stats` (vacío) → `/sys/class/net/wlan0/statistics/*` siempre 0 (parecía
  "sin tráfico"). Añadido `mtk_ndo_get_stats` + unificado `tx_dropped`.
  `mainline/wifi-consys/wifi/mt_wifi_port/glue/gl_init.c`. **Probado post-flash:**
  tras wget+pings, deltas +96 KB rx / +6 KB tx / +82 pkts / +67 pkts. ✅
- **Fix #2 — config de red persistente (en el móvil, no en el repo):** el AP entrega
  gateway `192.168.0.42` (él mismo, que **no enruta** a internet) y DNS `8.8.8.8`.
  Añadido a la conexión NM `vodafoneA544`:
  `+ipv4.routes "0.0.0.0/0 192.168.0.1 50"` + `ipv4.dns 192.168.0.1` + `ignore-auto-dns yes`.
  Persiste en `/etc/NetworkManager/system-connections/vodafoneA544.nmconnection` (sobrevive reboot).

## Por qué parecía roto (las 3 causas del falso diagnóstico)
1. **Dos rutas default**: el móvil tenía `default via 172.16.42.2 dev usb0` (Pi, métrica
   preferida) **y** `default via 192.168.0.42 dev wlan0`. Los `ping`/tests se iban por USB →
   parecían "funcionar" sin probar WiFi de verdad.
2. **El DHCP entrega gateway malo** (`.42` = el propio AP, no enruta). DHCP/ARP llegan al
   AP pero el tráfico a internet muere ahí → síntoma idéntico a "FW no cifra el data-path".
   El router real es `192.168.0.1`.
3. **`rx_bytes`/`tx_bytes = 0`** (bug cosmético del fix #1) → se leyó como "0 paquetes RX"
   cuando en realidad había tráfico.

## Implicación: el "plan de port de 6-9 semanas" ya está entregado
Las 8 hipótesis de crypto-path "refutadas" + todo el esfuerzo del port se basaban en un
malentendido de rutas. El port **de todas formas era el camino correcto** (y AHORA es el que
corre en HW con datos cifrados reales) — solo que el éxito ya estaba ahí cuando se flasheó.
**El WiFi — el problema central del proyecto durante 6 semanas — ESTÁ RESUELTO.**

## Cómo seguir (con WiFi cerrado, roadmap `PORT-STRATEGY-DRIVERS-0707.md` se limpia)
1. **Ola 1 quick-wins** (1-2 días c/u, userspace, riesgo cero): fluidez botón power,
   auto-brillo ALS, STP resync-RX. Rematan la sensación de "móvil terminado".
2. **Ola 2 ingeniería con retorno** (semanas): thermal CPU, accdet jack, **GPS** (la pieza
   grande realista, CONDITIONAL-GO 3-6 sem — primer combo MTK host-based en mainline+musl).
3. Investigación (moonshot): módem 2G/3G, cámara, FM (NO-GO/moonshot confirmado).

## Entorno / recetas
- Pi `.38` (`cpcd@192.168.0.38`) → móvil `ssh root@172.16.42.1`
  (`sudo ip addr replace 172.16.42.2/24 dev usb0`). Build+flash: `bash ~/wifi-iter-w.sh`.
- Boot de hoy: `boot-ndogetstats-0707.img` (sector 83968, md5 `b39eae54…`).
- Config NM persistente: editar con `nmcli connection modify vodafoneA544 …` (ver fix #2).
- Validar WiFi en una sesión nueva: forzar ruta `ip route replace default via 192.168.0.1
  dev wlan0 metric 50` + `wget -O /tmp/v.html https://www.google.com` → debe bajar ~81 KB.
