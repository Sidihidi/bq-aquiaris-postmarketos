# FASE 3 del PORT — PROBE REAL: mtk_mtwifi.ko con probe COMPILA+ENLAZA (0702, sesion .38)

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
