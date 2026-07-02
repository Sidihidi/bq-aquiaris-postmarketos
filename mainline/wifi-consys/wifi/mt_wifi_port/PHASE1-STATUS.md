# FASE 1 del PORT mt_wifi -> linux-7.0.12 — ESTADO (0702)

Generada por el workflow `w1fzd4bv0`/`w2fntslnd` (9 agentes) + integracion/compile-iterate manual.
Arbol de build en la Pi: `~/mainline/linux-7.0.12/drivers/net/wireless/mtk_mtwifi/`.

## PRODUCIDO E INTEGRADO (todo en la Pi + versionado aqui en `glue/`)
- **hif.h** (contrato HIF: firmas kalDev*, MCR_* offsets, GL_HIF_INFO reducido, glBusSetIrq).
- **mt6582-hif.c** (458L): backend PIO extraido de nuestro mt6582-wifi.c (driver-own/WHLPCR,
  HSTCR+erratum WHIER, guard WCIR por palabra, enhance-block 88B, ISR GIC_SPI 184).
- **gl_kal_prototypes.h**: contrato de todos los kal* que implementan los shims.
- **6 shims gl_kal**: gl_kal_mem.c (mem/pkt/QoS-classifier con deteccion EAPOL->SECURITY_FRAME) ·
  gl_kal_thread.c (tx_thread + GLUE_FLAG dispatch INT>OID>TXREQ>TIMEOUT>HALT) ·
  gl_kal_ioctl.c (kalIoctl con wait_for_completion_TIMEOUT por el wiphy_lock) ·
  gl_kal_timer.c (timer_setup/timer_delete, timer_container_of) ·
  gl_kal_indicate.c (cfg80211 moderno: connect_bss/disconnected/scan_done/inform_bss_frame_data) ·
  gl_kal_fw.c (request_firmware, sin set_fs).
- **13 edits a los headers KAL stock** (hdr_edits.json, 10 aplicados; 3 no-aplican = flags ya a 0 por la dieta):
  gl_typedef.h (#undef BIT/BITS por colision con linux/bits.h), gl_os.h (uaccess.h, guard wakelock/aee/kfifo),
  gl_kal.h (KAL_WAKE_LOCK no-op), config.h (defaults defensivos XLOG/AEE=0, CFG_CHIP_RESET_SUPPORT=0).
- **version.h** copiado de os/version.h del stock.
- **Makefile** (Kbuild: core dieted + glue + hif, includes, -Wno-format, -D de config STA-minima) + **Kconfig**.

## COMPILE-ITERATE (estado real, primer objeto = mt6582-hif.o):
- ✅ RESUELTO: `version.h` faltante (copiado de os/version.h).
- 🔴 EN CURSO (colisiones a reconciliar, trabajo normal de port Fase 1-2):
  1. `mt6582-wifi-reg.h` (copiado a glue/) COLISIONA con las definiciones propias del core stock:
     redefine MTK_WIFI_SIGNATURE / PHY_TYPE_SET_802_11BG / RATE_SET_ERP / HAL_MCR_RD... y da
     "expected identifier before numeric constant" (nombres que en el stock son valores de enum).
     **FIX**: NO copiar reg.h entero; hif.h ya lleva los MCR_* con #ifndef. Quitar el `#include
     "mt6582-wifi-reg.h"` de hif.h y de mt6582-hif.c, y anadir a hif.h SOLO los defines que
     mt6582-hif.c necesita y que no esten ya en el core (WHLPCR bits, HIF_TARGET_*).
  2. `hif.h` re-declara kalDevRegRead/Write/PortRead/Write que `gl_kal.h` YA declara (con BOOLEAN/
     UINT_32, no bool/u32) -> conflicting types. **FIX**: hif.h NO debe re-declararlas (usar las de
     gl_kal.h); solo aportar los MCR_* y glBus*. Alinear los tipos a BOOLEAN/UINT_16/UINT_32.
  3. `hif.h:52` typo (`dev_` / token roto) -> corregir la linea.

## SIGUIENTE (retomar el compile-iterate):
1. Arreglar las 3 colisiones de arriba -> `mt6582-hif.o` compila (hito: 1er objeto del port).
2. Compilar los 6 shims uno a uno (`make ... os/linux/glue/gl_kal_X.o`), reconciliando cada
   undefined/mismatch contra gl_kal_prototypes.h y los tipos del core.
3. Cuando la capa KAL compile toda -> **FIN FASE 1**. Luego Fase 2 (afinar HIF), Fase 3 (gl_init.c:
   probe + wlanAdapterStart), Fase 4 (gl_cfg80211.c: scan = M1), Fase 5 (connect+keys por SECURITY_FRAME).

## Regenerar el arbol de build (el core stock NO se versiona — 71k LOC, regenerable):
```
SRC=~/mainline/downstream/.../conn_soc/drv_wlan/mt_wifi/wlan
DST=~/mainline/linux-7.0.12/drivers/net/wireless/mtk_mtwifi
cp -r $SRC/{mgmt,nic,common,include} $DST/ ; mkdir -p $DST/os/linux/glue $DST/os/linux/hif
cp -r $SRC/os/linux/include $DST/os/linux/ ; cp $SRC/os/version.h $DST/os/linux/include/
# dieta STA-only: rm p2p_*/aaa_fsm/wapi/hs20/tdls*/wlan_p2p/wlan_bow ; flags CFG_* a 0
# aplicar hdr_edits.json ; copiar glue/* y Makefile/Kconfig de este repo
```
