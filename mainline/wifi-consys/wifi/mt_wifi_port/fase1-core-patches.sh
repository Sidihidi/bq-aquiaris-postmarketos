#!/bin/bash
# fase1-core-patches.sh — re-aplica los parches de Fase 1 al CORE STOCK (que NO se
# versiona) tras REGENERAR el arbol dieted. Simplemente extrae los 7 ficheros del core
# ya parcheados (core-patched-fase1.tar.gz) sobre el arbol.
#
# Orden completo para regenerar el arbol del port desde cero:
#   1) copiar mgmt/ nic/ common/ include/ os/linux/include del downstream
#   2) aplicar la dieta STA-only (rm p2p_*/aaa_fsm/wapi/hs20/tdls*/wlan_p2p/wlan_bow)
#   3) aplicar hdr_edits.json
#   4) ESTE script: tar xzf core-patched-fase1.tar.gz  (config.h debug.h wlan_lib.c
#      wlan_oid.c rsn.c scan.c assoc.c ya parcheados: HOTSPOT=0, stubs WMT/MCU en
#      debug.h, guards CFG_ENABLE_WIFI_DIRECT en rsn/scan/assoc, guards
#      CFG_SUPPORT_TX_POWER_LIMIT en wlanLoadManufactureData, <linux/stddef.h>, ...)
#   5) copiar glue/ hif/ Makefile Kconfig de este repo al arbol
#   6) make ... CONFIG_MTK_MTWIFI=m M=drivers/net/wireless/mtk_mtwifi modules
#
# Uso: ./fase1-core-patches.sh [ruta-al-arbol-mtk_mtwifi]
set -e
T=${1:-~/mainline/linux-7.0.12/drivers/net/wireless/mtk_mtwifi}
HERE="$(cd "$(dirname "$0")" && pwd)"
echo ">> extrayendo core parcheado en $T"
tar xzf "$HERE/core-patched-fase1.tar.gz" -C "$T"
echo ">> OK. Ahora copiar glue/ hif/ Makefile del repo y compilar."
