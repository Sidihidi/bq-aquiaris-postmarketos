# FASE 3 del PORT — ★ M1 CONSEGUIDO ★ (0703, sesion Mac/.123)

## M1 = insmod -> FW ARRANCA -> wlan0 registrado ✅ (2026-07-03)
Con CHIP FRIO (driver A bindea el nodo pero SIN auto-descargar el FW — auto_bringup deshabilitado,
kernel #233): `insmod mtk_mtwifi.ko` rc=0 →
- descarga WIFI_RAM_CODE completa (12 chunks por WTDR0, cada uno ACK'd por WRDR0)
- `Ready bit asserted` (WLAN_READY=1) → `wlanAdapterStart status=0x0 SUCCESS`
- BASIC_CONFIG + GET_NIC_CAPABILITY + wlanLoadManufactureData OK (init-stream stock completo)
- **wlan0 registrado y UP** (ndo_open rc=0; NO-CARRIER porque .scan = stub Fase 4)
- tx_thread vivo + flow-control TX del core funcionando (`nicTxReleaseResource: TC4 Free=4`)
- **CERO corrupcion** (la vista con of_ids-off era artefacto de aquel setup)
REQUISITO: chip frio. VCN33 always-on impide enfriar en runtime (func_off(WIFI) deja WLAN_READY=1).
En el producto final el port es el UNICO driver WiFi → chip frio nativo al boot. Para test con driver A
presente: deshabilitar su auto_bringup (mt6582-wifi.c:2175) o su binding.
SIGUIENTE = FASE 4: gl_cfg80211.c real (scan/connect/keys verbatim) → `iw scan` lista APs (M2).
Detalle: HANDOFF-MTWIFI-PORT-M1-DIAG-0703.md

---

# (historico) FASE 3 — PROBE REAL: mtk_mtwifi.ko con probe COMPILA+ENLAZA (0702, sesion .38)

## HITO
`gl_init.c` reescrito de stub minimo -> **PROBE REAL** (733L). `mtk_mtwifi.ko` = 1.91MB (crecio de
1.87), enlaza sin referencias indefinidas; todos los externos resuelven (consys EXPORT_SYMBOL_GPL en
vmlinux; register_netdev/request_firmware/wiphy_register = core). En el .ko: mtk_wlanProbe (0x944),
mtk_wlanRemove, wlanAdapterStart, compatible "mediatek,mt6582-wifi", firmware alias WIFI_RAM_CODE.

## Diseno (insight clave)
El probe NO toca registros del HIF: TODO el bring-up (chip-id, WHCR/WHIER, descarga WIFI_RAM_CODE por
WTDR0, WIFI_START, poll WLAN_READY, BASIC_CONFIG/NIC_CAPABILITY/SET_DOMAIN) lo hace `wlanAdapterStart`
del CORE via HAL_*/kalDev* -> mt6582-hif.c. El probe solo: enciende consys, puebla rHifInfo con los
ioremaps, y delega. Flujo (portado de wlanProbe stock, dieted STA-only):
EPROBE_DEFER(!mt6582_consys_ready) -> func_on(WIFI=3) -> vcn33(true) always-on -> wlanNetCreate
(wiphy_new + alloc_netdev "wlan%d" + bands[NL80211_BAND_2GHZ] + ciphers CCMP/TKIP) -> glSetHifInfo
(ioremap HIF+MCU+PDMA) -> waitq + QUEUE_INITIALIZE -> glBusSetIrq (de mt6582-hif.c) -> FW-download
STOCK (glLoadNvram stub + kalFirmwareImageMapping -> wlanAdapterStart -> Unmapping) -> kthread_run
(tx_thread) -> MAC por kalIoctl(wlanoidQueryCurrentAddr) + eth_hw_addr_set -> wlanNetRegister
(wiphy_register + apply_custom_regulatory + register_netdev) -> u4ReadyFlag=1 -> g_u4HaltFlag=0.

## Deltas 7.0.12 aplicados: eth_hw_addr_set (dev_addr const), NL80211_BAND_2GHZ, .remove->void,
wiphy_new/alloc_netdev/register_netdev, wiphy_apply_custom_regulatory.

## TODO / riesgos
- mtk_cfg80211_ops = STUB local __weak (.scan/.connect/.add_key -> -EOPNOTSUPP) hasta Fase 4
  (gl_cfg80211.c del Mac). Cuando exista no-weak con el mismo nombre, el linker lo prefiere.
- .ndo_start_xmit = drop silencioso (TODO Fase 4: encolar en rTxQueue).
- GLUE_FLAG_INT_BIT: alinear gl_os.h (1) vs fallback hif.h (2) antes del test de IRQ.
- HAL_READ_TX_RELEASED_COUNT/RX_LENGTH: reconciliar hif.h vs hal.h antes de Fase 5 (RX0_DONE re-arm).

## SIGUIENTE = TEST HW del probe (M1):
1. COLD-BOOT del movil (esta degradado, crashea cada 1-2 connects -> hardware fresco).
2. Desvincular el driver A del nodo: `echo 180f0000.wifi > /sys/bus/platform/drivers/mt6582-wifi/unbind`
   (ambos usan compatible "mediatek,mt6582-wifi"; el built-in bindea primero al boot).
3. Copiar mtk_mtwifi.ko al movil + `insmod` -> mirar dmesg/consola:
   - probe corre? func_on OK? wlanAdapterStart completa (FW download + WLAN_READY)? wlan0 aparece?
   - Instrumentar wlanSendCommand {CID,seq,len} para diffear el init-stream vs driver A (fwdump).
4. Si wlan0 aparece: `iw dev wlan0 scan` (con el mtk_cfg80211_ops stub NO escanea aun -> necesita Fase 4).
   El primer objetivo M1 real es que wlanAdapterStart COMPLETE (FW arranca por el path stock).
RIESGO: es un full-MAC nuevo haciendo func_on+FW-download en HW degradado -> puede colgar; mirar consola
+ el pstore-save del Mac (/var/log/pstore/) captura el crash.

## TEST HW DEL PROBE (0702 tarde, sesion .38) — EL PROBE ARRANCA
Corrido en HW (#230): insmod inerte OK (rc=0, no Unknown symbol, sobrevive). Fix: `.driver.name`
de "mt6582-wifi" (colisiona con driver A en sysfs) -> "mtk_mtwifi" (el match va por .compatible).
Tras `unbind` del driver A + `bind` del port, **mtk_wlanProbe CORRE end-to-end**:
- ioremap HIF/MCU/PDMA en rHifInfo OK; IRQ 212 (GIC_SPI 184) registrado OK.
- **FW mapeado OK (207648 bytes)** (request_firmware "mediatek/mt6582/WIFI_RAM_CODE" funciona).
- **`wlanAdapterStart` LLAMADO** -> status=**0xc0000001** (WLAN_STATUS_FAILURE) -> probe -5. Movil SOBREVIVE
  (fallo con gracia, sin cuelgue en el probe).
- WARNING no-fatal en `wiphy_new_nm` (net/wireless/core.c:456) por el mtk_cfg80211_ops stub (Fase 4).

**Causa del 0xc0000001 (hipotesis fuerte):** el auto-bringup del driver A YA arranco el FW al boot (chip
CALIENTE, WLAN_READY set); `wlanAdapterStart` intenta re-descargar el FW sobre uno corriendo -> falla
(no re-inicia un FW vivo). Necesita **CHIP FRIO**.

**Los CRASHES vistos NO son del probe** (que falla -5 con gracia) sino del CHURN del consys (rmmod +
unbind/bind repetidos sobre un consys medio-vivo) -> WDT. Un solo intento limpio sobrevive.

## SIGUIENTE = test con CHIP FRIO (para que wlanAdapterStart complete):
Opcion A (definitiva): kernel con driver A deshabilitado (Kconfig off / mt6582_wifi_of_ids vacio) ->
rebuild zImage + flash -> nodo libre + chip frio al boot -> insmod port -> wlanAdapterStart deberia
COMPLETAR (FW download + WLAN_READY) = hito M1. Riesgo: sin driver A de fallback si el port cuelga.
Opcion B (rapida, fragil): en boot fresco, unbind A + bind port en la ventana ~19-29s ANTES del
auto-bringup de A (chip aun frio). Opcion C: anadir al probe un power-cycle del consys (func_off+VCN33
off+func_on) o un WIFI reset antes de wlanAdapterStart, para forzar chip frio aunque A lo haya calentado.
Si con chip frio wlanAdapterStart AUN falla -> es un bug del backend HIF (kalDevPortWrite/RegRead no
conduce bien el chip) -> depurar mt6582-hif.c. Instrumentacion DIAG ya en el probe (FW size + status).
