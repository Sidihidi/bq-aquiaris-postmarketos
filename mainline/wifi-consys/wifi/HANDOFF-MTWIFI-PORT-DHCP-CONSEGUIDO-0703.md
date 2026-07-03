# 🏆 HANDOFF — PORT mt_wifi → 7.0.12: DHCP CONSEGUIDO, el WiFi FUNCIONA (Windows/.38, 2026-07-03)

## ★★★ MISIÓN CUMPLIDA: WPA2 + DHCP DE PUNTA A PUNTA ★★★
El port del driver stock MediaTek `mt_wifi` a Linux 7.0.12 (BQ Aquaris E4.5 krillin, pmOS mainline)
**conecta a WPA2-CCMP y saca IP por DHCP**. Log del HW (cold-boot #233, `.ko` md5 `e2eb1043`):
```
wpa: Associated with e6:92:82:f4:ce:44
wpa: WPA: Key negotiation completed with e6:92:82:f4:ce:44 [PTK=CCMP GTK=CCMP]   <- 4-WAY CERRADO
wpa: CTRL-EVENT-CONNECTED - Connection to e6:92:82:f4:ce:44 completed
udhcpc: broadcasting discover
udhcpc: broadcasting select for 10.181.211.252, server 10.181.211.204
udhcpc: lease of 10.181.211.252 obtained from 10.181.211.204, lease time 3599   <- LEASE
inet 10.181.211.252/24 brd 10.181.211.255 scope global wlan0                     <- IP REAL
ping 10.181.211.204: 3 packets transmitted, 3 received, 0% loss, avg 26ms        <- NAVEGA
```

## El bug de fondo (TX-encrypt del broadcast) — RESUELTO por el port
Toda la saga (RE del FW nds32, gates `[0x12e3]`/`[0x12f5]`, blind-pokes, FIX C, enc_status=6 en el
driver A) chocaba con que **el FW no cifraba el DHCP DISCOVER broadcast** → el AP lo tiraba → sin OFFER.
La causa era una **divergencia del command stream host↔FW** del driver A monolítico. **El port con el
core stock INTACTO emite el stream canónico** (`.connect`: SetInfrastructureMode→SetAuthMode→
SetEncryptionStatus[`ENUM_ENCRYPTION3_ENABLED`]→SetSsid; EAPOL por SECURITY_FRAME; flow-control TC4;
clasificación RX) → el broadcast se cifra bien → OFFER → lease. **Vía ganadora = el port, NO los
parches al driver A.**

## Las 5 fases (todas en el repo `mt_wifi_port/`)
1. **KAL headers + 6 shims + backend HIF PIO** (nuestro mt6582-hif.c como kalDev*).
2. (fusionada con 1) — `mtk_mtwifi.ko` enlaza.
3. **Probe real** (`gl_init.c`): platform_driver sobre nuestro consys → `wlanAdapterStart` STOCK →
   **M1** (FW arranca, wlan0). Requiere **chip frío** (VCN33 always-on impide reset en runtime → el
   único frío fiable es el del boot; driver A sin auto_bringup, o el port como único WiFi).
4. **`gl_cfg80211.c`** (scan/connect/keys, verbatim del stock + deltas 7.0.12) → **M2** (`iw scan`).
5. **TX de datos + BSSID** → **4-way + DHCP**:
   - `mtk_ndo_xmit`: portado `wlanHardStartXmit` (clasifica EAPOL→SECURITY_FRAME, encola, tx_thread).
   - `kalIndicateStatusAndComplete`: fix del BSSID nulo (caer al target BSS desc si `wlanoidQueryBssid`
     devuelve 00:00:00 → sin esto wpa derivaba la PTK con A2=00:00:00 y mandaba el M2 a nadie).

## Cómo reproducir el test (chip frío)
Kernel #233 (driver A bound sin auto-descargar el FW). En la Pi .38:
`make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2 CONFIG_MTK_MTWIFI=m M=drivers/net/wireless/mtk_mtwifi modules`
En el móvil: `echo 180f0000.wifi > /sys/bus/platform/drivers/mt6582-wifi/unbind` → copiar `.ko` a /tmp
(verificar md5) → `insmod /tmp/mtk.ko` → `rfkill unblock all` → `wpa_supplicant -i wlan0 -c
/etc/wpa_hola.conf -D nl80211` (**SIN `-f`**, este build imprime help) → `udhcpc -i wlan0 -q`.

## SIGUIENTE (consolidación)
- **Quitar la instrumentación DIAG**: portW/portR PRE/POST + WCIR dump (mt6582-hif.c), power-cycle del
  consys + `aucDebugModule=0x1F` (gl_init.c). Ya no hacen falta.
- **Hacer el port el WiFi por defecto** (driver A fuera / Kconfig): sería el único → chip siempre frío al
  boot → M1 nativo sin unbind, y wlan0 lista para NetworkManager/Phosh.
- **Estabilidad**: reconexión multi-ciclo, teardown/disconnect (el driver A tenía el crash de settle;
  el core stock hace su propio teardown — validar que no cuelga), roaming, suspend.
- **RF-cal**: el port hereda la cal del boot (VCN33 always-on); confirmar TX-power/rate finos.

*Sesión Windows (Fable 5), 2026-07-03. De "el DHCP no funciona" a navegar, vía el port completo del stock.*
