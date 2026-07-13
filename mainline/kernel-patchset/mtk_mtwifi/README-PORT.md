# mt_wifi_port -> linux-7.0.12 (BQ krillin) — ARBOL DEL PORT
Core stock INTACTO (mgmt/ nic/ common/ include/) copiado de:
  ~/mainline/downstream/.../conn_soc/drv_wlan/mt_wifi/wlan
Dieta STA-only aplicada (P2P/BoW/TDLS/WAPI/HS20/AAA borrados; flags a 0 en include/config.h).
GLUE NUEVO (a escribir, kernel 7.0.12) va en:
  os/linux/glue/   -> gl_kal.c gl_init.c gl_cfg80211.c gl_hook_api.c (shims KAL + cfg80211 moderno)
  os/linux/hif/    -> mt6582-hif.c (extraido de nuestro mt6582-wifi.c: kalDev*, glBusSetIrq, enhance-block)
Plan completo: HANDOFF-MTWIFI-PORT-PLAN-0702.md
