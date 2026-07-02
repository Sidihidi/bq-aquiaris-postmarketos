# SPDX-License-Identifier: GPL-2.0
#
# drivers/net/wireless/mtk_mtwifi/Makefile
#
# Kbuild del modulo 'mtk_mtwifi' — port del driver full-MAC MediaTek MT6582 'mt_wifi'
# (Android 3.10-era) a linux-7.0.12 (postmarketOS / BQ krillin, armv7).
#
# Arquitectura HIBRIDA:
#   [arriba] core stock INTACTO (mgmt/ nic/ common/) dieted STA-only
#   [medio]  glue NUEVO para 7.0.12 (os/linux/glue/gl_kal_*.o + gl_init.o + gl_cfg80211.o)
#   [abajo]  backend HIF PIO probado (os/linux/hif/mt6582-hif.o)
#
# Este Makefile es un Kbuild de modulo externo/in-tree: se selecciona con
#   CONFIG_MTK_MTWIFI=m  (ver Kconfig en este mismo directorio).
#
# Build in-tree:      make M=drivers/net/wireless/mtk_mtwifi modules
# Build out-of-tree:  make -C <kdir> M=$(PWD) modules

obj-$(CONFIG_MTK_MTWIFI) += mtk_mtwifi.o

# ---------------------------------------------------------------------------
#  Raiz del arbol del modulo (soporta build in-tree e out-of-tree).
# ---------------------------------------------------------------------------
MTWIFI_ROOT := $(src)

# ===========================================================================
#  (1) CORE STOCK — dieted STA-only.  Rutas relativas al directorio del modulo.
# ===========================================================================

# --- common/ ---------------------------------------------------------------
#   wlan_bow.c  (BOW) y wlan_p2p.c (P2P) EXCLUIDOS.
mtk_mtwifi-y += \
	common/dump.o \
	common/wlan_lib.o \
	common/wlan_oid.o

# --- mgmt/ -----------------------------------------------------------------
#   EXCLUIDOS (STA-only): p2p_assoc p2p_bss p2p_fsm p2p_func p2p_ie p2p_rlm
#   p2p_rlm_obss p2p_scan p2p_state (P2P), tdls tdls_com (CFG_SUPPORT_TDLS=0).
#   wapi/hs20/wnm quedan comentados (features off); descomentar si el core
#   dieted arroja undefined-reference a sus simbolos en el primer make.
mtk_mtwifi-y += \
	mgmt/ais_fsm.o \
	mgmt/assoc.o \
	mgmt/auth.o \
	mgmt/bss.o \
	mgmt/cnm.o \
	mgmt/cnm_mem.o \
	mgmt/cnm_timer.o \
	mgmt/hem_mbox.o \
	mgmt/mib.o \
	mgmt/privacy.o \
	mgmt/rate.o \
	mgmt/rlm.o \
	mgmt/rlm_domain.o \
	mgmt/rlm_obss.o \
	mgmt/rlm_protection.o \
	mgmt/roaming_fsm.o \
	mgmt/rsn.o \
	mgmt/saa_fsm.o \
	mgmt/scan.o \
	mgmt/scan_fsm.o \
	mgmt/sec_fsm.o \
	mgmt/swcr.o
# mtk_mtwifi-y += mgmt/wapi.o    # CFG_SUPPORT_WAPI (off)
# mtk_mtwifi-y += mgmt/hs20.o    # CFG_SUPPORT_HOTSPOT_2_0=0
# mtk_mtwifi-y += mgmt/wnm.o     # CFG_SUPPORT_802_11V/WNM (off)

# --- nic/ ------------------------------------------------------------------
#   p2p_nic.c EXCLUIDO (P2P).
mtk_mtwifi-y += \
	nic/cmd_buf.o \
	nic/nic.o \
	nic/nic_cmd_event.o \
	nic/nic_pwr_mgt.o \
	nic/nic_rx.o \
	nic/nic_tx.o \
	nic/que_mgt.o

# ===========================================================================
#  (2) GLUE NUEVO — reemplaza gl_kal.c / gl_init.c / gl_cfg80211.c stock.
#      Los 6 shims KAL + gl_init + gl_cfg80211 portados a 7.0.12.
# ===========================================================================
mtk_mtwifi-y += \
	os/linux/glue/gl_kal_mem.o \
	os/linux/glue/gl_kal_thread.o \
	os/linux/glue/gl_kal_ioctl.o \
	os/linux/glue/gl_kal_timer.o \
	os/linux/glue/gl_kal_indicate.o \
	os/linux/glue/gl_kal_fw.o

# Los siguientes glue stock NO se compilan en la Fase 1/STA-only:
#   os/linux/gl_bow.o gl_p2p.o gl_p2p_cfg80211.o gl_p2p_init.o gl_p2p_kal.o (P2P/BOW)
#   os/linux/gl_rst.o          (CFG_CHIP_RESET_SUPPORT=0)
#   os/linux/gl_wext.o gl_wext_priv.o gl_proc.o  (wext legacy; cfg80211-only)
#   os/linux/platform.o        (reemplazado por el probe AHB de gl_init.c)

# ===========================================================================
#  (3) BACKEND HIF — PIO probado (mt6582-hif.c). Reemplaza hif/ahb/*.c.
# ===========================================================================
mtk_mtwifi-y += \
	os/linux/hif/mt6582-hif.o

# hif/ahb/ahb.o arm.o mt6582/ahb_pdma.o  -> EXCLUIDOS (los sustituye mt6582-hif.o)

# ===========================================================================
#  INCLUDES
# ===========================================================================
ccflags-y += \
	-I$(MTWIFI_ROOT)/include \
	-I$(MTWIFI_ROOT)/include/nic \
	-I$(MTWIFI_ROOT)/include/mgmt \
	-I$(MTWIFI_ROOT)/os/linux/include \
	-I$(MTWIFI_ROOT)/os/linux/glue \
	-I$(MTWIFI_ROOT)/os/linux/hif

# ===========================================================================
#  WARNINGS
#    -Wno-format        : UINT_32 == unsigned long en arm32 -> %d/%u vs long
#    -Wno-unused        : el core stock deja muchos static/local sin usar tras el diet
#    -Wno-implicit-fallthrough / -Wno-misleading-indentation : ruido del core 3.10
# ===========================================================================
ccflags-y += \
	-Wno-format \
	-Wno-unused \
	-Wno-implicit-fallthrough \
	-Wno-misleading-indentation \
	-Wno-declaration-after-statement

# ===========================================================================
#  DEFINES de config STA-minima.
#  config.h YA fija estos valores; se re-pasan por -D como red de seguridad y
#  para desactivar codigo condicional sin editar config.h.
# ===========================================================================
ccflags-y += \
	-DLINUX \
	-DCFG_SUPPORT_AAA=0 \
	-DCFG_ENABLE_WIFI_DIRECT=0 \
	-DCFG_ENABLE_WIFI_DIRECT_CFG_80211=0 \
	-DCFG_ENABLE_BT_OVER_WIFI=0 \
	-DCFG_SUPPORT_TDLS=0 \
	-DCFG_SUPPORT_HOTSPOT_2_0=0 \
	-DCFG_SUPPORT_WAPI=0 \
	-DCFG_SUPPORT_802_11W=0 \
	-DCFG_CHIP_RESET_SUPPORT=0 \
	-DCFG_SUPPORT_AGPS_ASSIST=0 \
	-DCFG_ENABLE_FW_DOWNLOAD=1 \
	-DMTK_WCN_HIF_SDIO=0 \
	-DCFG_BUILT_IN_DRIVER=0
ccflags-y += -Wno-error=date-time
mtk_mtwifi-y += os/linux/glue/gl_init.o
