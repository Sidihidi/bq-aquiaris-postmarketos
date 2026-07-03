# HANDOFF — PORT mt_wifi: M2 ✅ + Fase 5 el connect ASOCIA, el 4-way se atasca por el TX-drop stub (Windows/.38, 2026-07-03)

> Continúa `HANDOFF-MTWIFI-PORT-M1-DIAG-0703.md`. Fase 4 (gl_cfg80211.c) commit `27943c0`.

## 🎉 M2 CONSEGUIDO EN HW — el port ESCANEA
Con chip frío (kernel #233, driver A sin auto-descargar el FW), unbind driver A → `insmod mtk_mtwifi.ko`
(Fase 4, md5 `c2c5a6e...`) → **M1** (portW #1..#12 OK → Ready bit → `wlanAdapterStart SUCCESS` → wlan0 ARRIBA)
→ **`iw dev wlan0 scan` LISTA APs REALES** (`hola` -30dBm, MIWIFI_2G, Telecartagena132, vodafoneA544 ×2),
**SIN CRASH**. El handler `mtk_cfg80211_scan` va de punta a punta (scan→FW→gl_kal_indicate→cfg80211→iw).
**Fase 4 VALIDADA.**

## Fase 5 (connect+DHCP): el connect ASOCIA, el 4-way se ATASCA — causa raíz IDENTIFICADA
`wpa_supplicant -i wlan0 -c /etc/wpa_hola.conf -D nl80211` (¡SIN `-f`! este build imprime el help y sale
con `-f`) → el port conecta:
```
wpa: SCANNING -> ASSOCIATING -> 4WAY_HANDSHAKE (se queda AQUI, nunca COMPLETED)
dmesg: wlanoidSetSsid -> aisFsmRunEventJoinComplete  (el connect del core dispara)
wpa: CTRL-EVENT-ASSOC-REJECT status_code=1 (x2) -> "Associated with 00:00:00:00:00:00"
udhcpc: broadcasting discover x6 -> no lease  (falla porque NO hay conexion, no por TX-encrypt)
```
- ✅ **El command stream del connect FUNCIONA**: `mtk_cfg80211_connect` → `wlanoidSetSsid` → `aisFsmRunEventJoinComplete`.
- ❌ **El 4-way NO completa**: los EAPOL de wpa_supplicant **no se transmiten**.

### CAUSA RAÍZ: el `ndo_start_xmit` del port es un STUB QUE TIRA LOS PAQUETES
`mt_wifi_port/glue/gl_init.c` `mtk_ndo_xmit` (líneas ~127-133):
```c
static netdev_tx_t mtk_ndo_xmit(struct sk_buff *skb, struct net_device *ndev) {
    dev_kfree_skb(skb); ndev->stats.tx_dropped++; return NETDEV_TX_OK;  /* TODO FASE 4: drop silencioso */
}
```
→ TODO el TX de datos (incluidos los **EAPOL M2/M4** del 4-way) se DROPEA → el handshake se cuelga → sin
claves → sin conexión → udhcpc falla. **NO es el TX-encrypt: es que no hay TX de datos.**

## EL FIX (bien acotado, ~144 L)
**Portar el `wlanHardStartXmit` del stock** (`os/linux/gl_init.c:1682-1826`) al `mtk_ndo_xmit` del port,
con los ajustes de skb de 7.0.12. Ese xmit: aloca `MSDU_INFO`, **clasifica el frame** (detecta EAPOL →
`COMMAND_TYPE_SECURITY_FRAME`), lo **encola** y despierta el **tx_thread**. La infra ya está en el port:
- `kalQoSFrameClassifierAndPacketInfo` → `gl_kal_mem.c` (clasificador EAPOL, YA portado).
- `tx_thread` (dispatch INT>OID>TXREQ>TIMEOUT) → `gl_kal_thread.c` (YA portado).
- `rTxQueue` inicializada en el probe (`gl_init.c` QUEUE_INITIALIZE).
- El path SECURITY_FRAME (`wlanProcessSecurityFrame` → puerto 1/TC4) + flow-control TC4 + clasificación
  RX del M3 → **el core stock los trae de fábrica** (la herencia que de-riesga Fase 5).

Además revisar el **RX de datos**: que el skb del AP (EAPOL M1/M3, luego DHCP OFFER) suba a la pila
(`kalRxIndicatePkts`/`netif_rx` — ya en `gl_kal_mem.c`/indicate). Si el TX se cablea y el 4-way COMPLETA,
el siguiente hito es **udhcpc → LEASE = el objetivo (DHCP)**; ahí es donde el command stream del core
(enc_status=ENABLED del `.connect` ya portado) debe cifrar el broadcast bien (lo que el driver A no lograba).

## Notas de test
- **Chip frío requerido** (driver A sin FW al boot; mi móvil ya en #233). unbind driver A → insmod.
- `wpa_supplicant` de este rootfs: **NO usar `-f`** (imprime help y sale); redirigir stdout (`>log 2>&1`).
- `/etc/wpa_hola.conf` (ctrl_interface + hola/kakatua1 CCMP) ya está en el móvil.
- `rfkill unblock all` + parar NetworkManager antes del test.
- El .ko lleva aún el DIAG portW/portR + power-cycle + DBGLOG 0x1F → **quitar tras cerrar Fase 5**.

## Estado
Fase 4 cerrada y validada (M2). Fase 5 al 80%: connect OK, falta cablear el TX de datos (el drop stub) →
4-way → DHCP. El .ko de Fase 4 en la Pi .38 y en el móvil (`/tmp/mtk_mtwifi.ko`).

*Sesión Windows (Fable 5), 2026-07-03. M2 en HW; Fase 5 a un `wlanHardStartXmit` de distancia.*
