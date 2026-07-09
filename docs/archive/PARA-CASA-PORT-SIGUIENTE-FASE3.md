# PARA CASA — Port mt_wifi: dónde estamos y por dónde seguir (Fase 3)

> Escrito por la sesión Mac tras cerrar la Fase 1. Léelo antes de retomar el port.
> Estado técnico completo: `HANDOFF-MTWIFI-PORT-FASE1-KO-0702.md`. Commit `f677c67` (en `main`).

## DÓNDE ESTAMOS (hito conseguido)
✅ **`mtk_mtwifi.ko` COMPILA y ENLAZA** contra Linux 7.0.12 (1.87 MB, 40 objetos: 34 core dieted +
6 shims KAL + backend HIF + `gl_init.c` mínimo). **Pero carga INERTE: aún NO hace probe.**
- Build (Pi Mac .123): `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2 CONFIG_MTK_MTWIFI=m M=drivers/net/wireless/mtk_mtwifi modules`
- Versionado en el repo: `mt_wifi_port/` (glue/ hif/ Makefile Kconfig + `core-patched-fase1.tar.gz` + `fase1-core-patches.sh`).

## PRIMERO: montar el árbol en TU Pi (.38) desde el repo
El **core stock NO se versiona** (71k LOC, regenerable). Para tener el árbol buildable en .38:
1. `git pull` (trae `mt_wifi_port/` actualizado).
2. Regenerar el core: copiar `mgmt/ nic/ common/ include/ os/linux/include` del downstream → dieta
   STA-only (`rm p2p_*/aaa_fsm/wapi/hs20/tdls*/wlan_p2p/wlan_bow`) → aplicar `hdr_edits.json`.
3. **`bash mt_wifi_port/fase1-core-patches.sh <ruta-al-arbol>`** ← extrae los 7 ficheros del core ya
   parcheados (config.h HOTSPOT=0, stubs WMT/MCU en debug.h, guards P2P/TX-power, etc.). **SIN esto no
   compila.**
4. Copiar `mt_wifi_port/{glue,hif}/* Makefile Kconfig` al árbol → `make ... modules`. Debe dar el `.ko`.

## SIGUIENTE = FASE 3: el probe real (que el módulo HAGA algo)
Reescribir `os/linux/glue/gl_init.c` (ahora es un stub mínimo que solo enlaza) como el **probe real**.
**Reutiliza la lógica ya probada de nuestro `mt6582-wifi.c` (driver A)** — el probe/consys/VCN33 YA
funciona ahí, solo hay que trasplantarlo:
1. `platform_driver` que casa el nodo DT del WiFi (HIF@0x180F0000). En `.probe`:
   - EPROBE_DEFER sobre `mt6582_consys_ready`, `func_on(WIFI)` vía btif, **VCN33 regulator-always-on**
     (lección [[project-wifi-wlan-ready-flakiness-fix]]: NO crear flanco off→on o se pierde la RF-cal).
   - Auto-bringup con retry (WLAN_READY es flaky ~3/4; recuperable con `echo 1 > debugfs/bringup`).
2. Llamar al **`wlanAdapterStart` STOCK** (no el nuestro): INT off durante download, FW download por
   `request_firmware('mediatek/mt6582/WIFI_RAM_CODE')`, WIFI_START, poll WLAN_READY, re-init WHCR/WHIER
   post-START, BASIC_CONFIG (MAC polled), GET_NIC_CAPABILITY, `wlanLoadManufactureData` (con NVRAM stub),
   nicInitMGMT, enable INT.
3. `kthread` para el `tx_thread` (el dispatch INT>OID>TXREQ>TIMEOUT>HALT ya está en `gl_kal_thread.c`).
4. Registrar wiphy/netdev con **`eth_hw_addr_set(dev, MAC-del-FW)`** (lección: el netdev MAC DEBE = FW MAC
   o el AP tira los data frames).
5. Poner `g_u4HaltFlag = 0` cuando el probe termina OK (ahora está a 1 = halted).

**HITO M1 (fin Fase 4, ~1 sem):** `insmod mtk_mtwifi.ko` → wlanAdapterStart completa → `wlanQueryPermanentAddress`
da la MAC real → `iw dev wlan0 scan` **lista APs**. Instrumentar `wlanSendCommand {CID,seq,len}` para
**diffear el stream de init contra el driver A** (fwdump) — es el primer trozo del stream que el driver A
NUNCA mandó.

## GOTCHAS que dejaron los críticos (no los pases por alto)
- **`kalIoctl` SIN timeout + wiphy_lock** (era blocking): `gl_kal_ioctl.c` YA usa
  `wait_for_completion_TIMEOUT` — verifícalo, porque este FW cuelga documentadamente en SET_BSS_INFO y sin
  timeout dejas el wiphy_lock cogido para siempre (todo nl80211 en D-state).
- **`HAL_READ_TX_RELEASED_COUNT` / `HAL_READ_RX_LENGTH` redefinidos** (warning ahora): los define
  TANTO nuestro `hif.h` COMO el stock `nic/hal.h`. Reconciliar antes de Fase 5 — si los valores difieren,
  el RX0_DONE no se re-arma (el bug flaky que ya sufrimos). Nuestro `hif.h` NO debería redefinir lo que
  `hal.h` ya da; alinear.
- **RX aggregation** (`CFG_SDIO_RX_AGG`): el core hará burst-reads multi-paquete del port0
  (`nicRxSDIOAggReceiveRFBs`) — un patrón que el driver A nunca ejercitó. O capar `CFG_SDIO_MAX_RX_AGG_NUM`
  a 1 (replica el driver A) o endurecer el guard WCIR del `kalDevPortRead` para no abortar a mitad de burst.
- **`kalDevPortRead(MCR_WHISR)`** debe devolver el `ENHANCE_MODE_DATA_STRUCT_T` de 88B que `nicProcessIST`
  espera (poner un `BUILD_BUG_ON(sizeof==88)` + offsets).

## FASE 5 = el objetivo real (DHCP)
connect WPA2 + 4-way **por SECURITY_FRAME** (cola de comandos, NO data-path) + ADD_REMOVE_KEY por el core
stock → DISCOVER cifrado + LEASE. **Herencia del experimento driver A (ya de-riesgado en HW):** la ruta
EAPOL por TC4/puerto-1 funciona a nivel HIF, pero el driver A no la sostiene por falta de **flow-control
de TC4** (`nicTxAcquireResource` + `TxReleaseCount` del enhance-block) y de la **clasificación RX del M3**
(267 B) — **el core stock trae ambas de fábrica**, así que Fase 5 las hereda resueltas. (El terminador +4
del TX NO era el bug; se quitó.)

## División de trabajo sugerida
- **Casa (.38):** Fase 3 (probe) — es donde más código nuevo hay y tú tienes el móvil a mano para insmod+scan.
- **Mac (.123):** puedo ir con Fase 4 (gl_cfg80211.c: scan/connect, cuerpos verbatim del stock) en paralelo,
  o reconciliar los HAL_* / RX-agg. Coordinamos por GitHub y NO tocamos los mismos ficheros a la vez.

*Sesión Mac (Fable 5), 2026-07-02.*
