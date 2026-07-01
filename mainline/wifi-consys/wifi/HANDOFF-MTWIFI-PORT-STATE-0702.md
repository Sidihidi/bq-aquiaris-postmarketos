# Estado del PORT del driver stock mt_wifi -> linux-7.0.12 (0702)

## DECISION (0702): portamos el driver stock (via de MAXIMA garantia de driver funcional)
Razon: replica el driver que DEMOSTRABLEMENTE funciona (LineageOS navega estable con el MISMO FW);
elimina TODAS las divergencias de golpe (no una hipotesis). Y es trabajo de escritorio: no depende
del movil fragil (que ahora crashea cada 1-2 connects por degradacion del FW). Plan arquitectonico
completo + veredictos de los 2 criticos: HANDOFF-MTWIFI-PORT-PLAN-0702.md.

Estrategia = HIBRIDO 3 capas:
- ABAJO (nuestro codigo, ya funciona): mt6582-hif.c extraido de mt6582-wifi.c (HIF PIO endurecido:
  wifi_hif_alive, HSTCR+erratum WHIER, guard WCIR por palabra, enhance-block 88B, driver/fw-own,
  disc_settle) implementando el contrato kalDev*/glBusSetIrq. Reemplaza ahb.c/ahb_pdma.c.
- MEDIO (reescrito para 7.0.12): gl_kal.c/gl_init.c/gl_cfg80211.c NUEVOS, preservando el tx_thread
  unico + bits GLUE_FLAG (la serializacion ES el command stream) y los cuerpos de los handlers verbatim.
- ARRIBA (stock INTACTO): mgmt/ + nic/ + common/. P2P/BoW/TDLS/WAPI/HS20/AAA fuera (-35k LOC).

## Divergencia de cifrado MAS FUERTE hallada (el objetivo real del port):
El stock entrega el EAPOL del 4-way por la COLA DE COMANDOS (COMMAND_TYPE_SECURITY_FRAME ->
wlanProcessSecurityFrame), NO por el data-path. Nuestro driver A lo manda por tx_queue (data). El FW
podria gatear el cifrado del TX al recibir el 4/4 EAPOL por la ruta de seguridad correcta. (El init-blob
SET_PHY_PARAM resulto ser calibracion RF -> debil para un bug de cifrado.) Priorizar llegar a Fase 5.

## FASE 0 COMPLETADA (arbol real en la Pi):
`~/mainline/linux-7.0.12/drivers/net/wireless/mtk_mtwifi/`
- core stock copiado de ~/mainline/downstream/.../conn_soc/drv_wlan/mt_wifi/wlan (mgmt/ nic/ common/ include/)
- DIETA STA-only aplicada: 34 .c / 84 .h, 71k LOC (borrados p2p_*/aaa_fsm/wapi/hs20/tdls/wlan_p2p/wlan_bow)
- include/config.h con CFG_ENABLE_WIFI_DIRECT/BT_OVER_WIFI/WAPI/HS20/TDLS = 0
- os/linux/glue/ y os/linux/hif/ creados (vacios, para el codigo NUEVO)
- Regenerable: copiar del downstream + re-aplicar la dieta (ver README-PORT.md en el arbol).

## SIGUIENTE = FASE 1 (contrato KAL en headers + shims que compilan, 1-1.5 sem):
Portar gl_typedef.h/gl_os.h/gl_kal.h/hif.h (guard-out wakelock/aee/xlog/kfifo, CONF_MTK_AHB_DMA=0,
GLUE_FLAG bits identicos; #undef BIT por colision con linux/bits.h; -Wno-format por UINT_32=unsigned long)
+ escribir gl_kal.c con los ~30 shims (lista en el PLAN). Empezar por: kalMem*/locks/timers (macros triviales)
-> tx_thread (dispatch INT>OID>TXREQ>TIMEOUT>HALT) -> kalIoctl+completions -> kalDev* (backend = nuestro HIF).
OJO 7.0.12 (criticos): from_timer->timer_container_of, del_timer->timer_delete; wiphy_lock cogido en los
cfg80211 ops -> kalIoctl DEBE tener timeout (este FW cuelga en SET_BSS_INFO); CFG_SDIO_RX_AGG multi-paquete
en port0 (nuestro HIF PIO debe soportar el burst-read del enhance-block); ndo_select_queue NO omitir (TC map).

## HITO M1 (fin de Fase 4): insmod -> FW download stock -> WLAN_READY -> GET_NIC_CAPABILITY ->
wlanLoadManufactureData -> 'iw scan' lista APs. Instrumentar wlanSendCommand {CID,seq,len} para diffear
el stream contra nuestro driver A (fwdump). M1 crypto (fin Fase 5) = connect WPA2 + 4-way por SECURITY_FRAME
+ ADD_REMOVE_KEY por el core stock + DISCOVER cifrado en sniff + LEASE = el fix del DHCP.

## Driver A (mt6582-wifi.ko #220) se conserva conmutable por Kconfig: plan B + generador del stream de referencia.
