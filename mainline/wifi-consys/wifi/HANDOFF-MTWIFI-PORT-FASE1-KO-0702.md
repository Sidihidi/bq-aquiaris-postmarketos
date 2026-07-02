# HANDOFF — PORT mt_wifi → 7.0.12: Fase 1 CERRADA, `mtk_mtwifi.ko` ENLAZA (Mac, 2026-07-02)

## TL;DR
🎉 **El driver stock `mt_wifi` portado COMPILA y ENLAZA como módulo cargable** contra Linux 7.0.12
(armv7/krillin). `mtk_mtwifi.ko` = **1.87 MB**, 40 objetos (34 core dieted + 6 shims KAL + HIF + gl_init).
De "no compilaba nada" a `.ko` en una sesión. Falta el **probe real (Fase 3)** para que haga algo.

- Árbol de build: `cpcd@192.168.0.123:~/mainline/linux-7.0.12/drivers/net/wireless/mtk_mtwifi/`
- Build: `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2 CONFIG_MTK_MTWIFI=m M=drivers/net/wireless/mtk_mtwifi modules`
- Versionado en el repo: `mt_wifi_port/` (glue/ hif/ Makefile Kconfig + **core-patched-fase1.tar.gz** + **fase1-core-patches.sh**).

## Cómo se llegó (frontera de errores resuelta, en orden)
1. **hif.h / mt6582-wifi-reg.h** (3 colisiones): reg.h re-split (93 símbolos MCR_* fuera → los da el stock
   `mtreg.h` vía precomp.h:260); hif.h dejó de re-declarar `kalDev*` (los da gl_kal.h); typo `dev_*/`.
2. **Alineado de tipos** en mt6582-hif.c: firmas `kalDev*` a `BOOL/UINT_32/PUINT_8` (no bool/u32).
3. **gl_kal_prototypes.h / hif.h**: bug de comentario (`kalGetBow*/` cerraba el `/* */` antes de tiempo).
4. **Deltas cfg80211 7.0.12** (gl_kal_indicate.c): `scan_width` eliminado de `cfg80211_inform_bss`;
   `cfg80211_roam_info` ahora MLO → `rRoamInfo.links[0].bss`.
5. **gl_kal_thread.c**: `ktime_get_boot_ns` → `ktime_get_boottime_ns`.
6. **Restos de P2P en el core dieted** (guards `#if CFG_ENABLE_WIFI_DIRECT`): rsn.c (prP2pSpecificBssInfo),
   scan.c (2× kalP2PIndicateBssInfo), assoc.c (entrada WSC de la tabla). wlan_oid.c: `CFG_SUPPORT_HOTSPOT_OPTIMIZATION=0`.
7. **Calibración RF no necesaria** (wlan_lib.c): items 7-10 de `wlanLoadManufactureData`
   (EDGE_TXPWR/5G/RSSI_COMPENSATE/BAND_SUPPORT) envueltos en `#if CFG_SUPPORT_TX_POWER_LIMIT` (no def → fuera).
8. **debug.h**: stubs no-op de los símbolos WMT/MCU-debug dieted (`ASSERT_BREAK`, `MCU_REG_READL`,
   `CONN_MCU_CPUPCR`, `WMTDRV_TYPE_WIFI`, `mtk_wcn_wmt_assert`, `glDumpConnSysCpuInfo`) → evita `#if 0`
   dispersos y cascada de variables sin usar.
9. **Otros**: wlan_oid.c `<stddef.h>`→`<linux/stddef.h>`; wlan_lib.c `fgTdlsIsSup` (TDLS dieted);
   Makefile `-Wno-error=date-time` (swcr `__DATE__`); quitar de la obj-list `aaa_fsm.o` (borrado) y
   `gl_cfg80211.o` (Fase 4).
10. **HifIsFwOwn duplicado**: se quita el nuestro (mt6582-hif.c/hif.h); lo provee el stock `nic/nic.c`
    (firma `P_ADAPTER_T`); nuestro guard interno es `hif_alive()`.
11. **`kalSecurityFrameSendComplete` duplicado** (gl_kal_mem.c + gl_kal_thread.c) → se deja solo en mem.c.
12. **gl_init.c NUEVO (mínimo, Fase 1.5)**: globales del ciclo (`g_halt_sem`, `g_u4HaltFlag`,
    `fgIsResetting`, `aucDebugModule`) + 12 shims KAL que faltaban (kalAllocateIOBuffer, kalRandomNumber,
    kalReadyOnChannel, kalUpdateRSSI, kalGetConfiguration, ...) + 3 stubs aaa_fsm (AP-mode) +
    `module_init`/`MODULE_LICENSE`. **El módulo carga INERTE** (sin probe todavía).

## SIGUIENTE — Fase 3 (probe real) para que el módulo HAGA algo
`gl_init.c` (sustituir el mínimo) = platform_driver sobre NUESTRO consys:
EPROBE_DEFER en `mt6582_consys_ready`, `func_on(WIFI)` vía btif, VCN33 always-on → `wlanAdapterStart`
STOCK (FW download por `request_firmware`, WIFI_START, WLAN_READY, GET_NIC_CAPABILITY) → registrar
wiphy/netdev con la MAC del FW. Instrumentar `wlanSendCommand {CID,seq,len}` para diffear el stream
contra el driver A (fwdump). **HITO M1** = `insmod` → FW arranca → `iw dev wlan0 scan` lista APs.
Luego Fase 4 (gl_cfg80211.c: scan/connect) y **Fase 5** = connect WPA2 + 4-way **por SECURITY_FRAME**
+ ADD_REMOVE_KEY por el core stock → **DISCOVER cifrado + LEASE = el fix del DHCP** (el objetivo).

### Herencia del experimento del driver A (NO perder en Fase 5)
La ruta EAPOL por **TC4/puerto-1** funciona a nivel HIF (probado en HW: M2 sale sin colgar), pero
el 2º EAPOL cuelga por **falta de flow-control de TC4** (`nicTxAcquireResource` + `TxReleaseCount` del
bloque enhance) y el **M3 (267 B) no se entrega** (clasificación RX). El core stock trae ambas cosas
de fábrica → Fase 5 las hereda resueltas. (El terminador +4 del TX NO era el bug; se quitó.)

## Reproducir el árbol desde cero
Ver `mt_wifi_port/fase1-core-patches.sh` (extrae `core-patched-fase1.tar.gz` sobre el core regenerado).
Orden: copiar downstream → dieta STA-only → hdr_edits.json → `fase1-core-patches.sh` → copiar glue/hif/Makefile → make.
**OJO: si alguien REGENERA el core sin re-aplicar estos parches, el build se rompe.**

*Sesión Mac (Fable 5), 2026-07-02. Co-autor de los parches: workflow de 3 agentes (reg.h split + type-align + core guards).*
