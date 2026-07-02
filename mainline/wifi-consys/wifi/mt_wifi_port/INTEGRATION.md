# mtk_mtwifi — Integracion y orden de build (Fase 1: que la capa KAL compile)

Port del driver full-MAC MediaTek **MT6582 'mt_wifi'** (Android 3.10-era) a
**linux-7.0.12** (postmarketOS / BQ krillin, armv7). Arquitectura **HIBRIDA**:

```
  [arriba] core stock INTACTO (mgmt/ nic/ common/) dieted STA-only
  [medio]  glue NUEVO 7.0.12 (os/linux/glue/gl_kal_*.o + gl_init.o + gl_cfg80211.o)
  [abajo]  backend HIF PIO probado (os/linux/hif/mt6582-hif.o)
```

## Disposicion del arbol (montado en la Pi)

```
drivers/net/wireless/mtk_mtwifi/
  Makefile              <- este
  Kconfig               <- este
  INTEGRATION.md        <- este
  common/   mgmt/   nic/            (core stock dieted, INTACTO)
  include/  include/nic/  include/mgmt/    (headers stock)
  os/linux/include/                 (gl_kal.h, gl_os.h, gl_typedef.h ... stock)
  os/linux/glue/                    (NUEVO: gl_kal_prototypes.h, hif.h,
                                     gl_kal_{mem,thread,ioctl,timer,indicate,fw}.c,
                                     gl_init.c, gl_cfg80211.c)
  os/linux/hif/                     (NUEVO: mt6582-hif.c)
  os/linux/hif/  <mt6582-wifi-reg.h>  (defines de registro, junto al .c o en glue/)
```

## Orden de compilacion recomendado (iterativo)

La Fase 1 solo exige que **la capa KAL compile** (headers portados + los shims
`gl_kal_*.o` + `mt6582-hif.o`). El link del core entero es Fase 3-4. Recomendado:

1. **Solo el HIF**: comentar en el Makefile todo salvo
   `os/linux/hif/mt6582-hif.o`. Debe compilar contra `hif.h` +
   `mt6582-wifi-reg.h` + `precomp.h`. Resuelve primero el include de registro
   (colision `BIT()`/`BITS()` de `gl_typedef.h` vs `<linux/bits.h>`: `#undef
   BIT`/`#undef BITS` ANTES de `gl_typedef.h`).
2. **Los 6 shims KAL**: anadir `os/linux/glue/gl_kal_*.o`. Compilan contra
   `gl_kal_prototypes.h` + `precomp.h`. Aqui salen los primeros
   *implicit-declaration* si la cadena de includes del core no arrastra algun
   tipo (`P_QUE_ENTRY_T`, `ENUM_CHNL_EXT_T`, `P_RF_CHANNEL_INFO_T`).
3. **gl_init.o + gl_cfg80211.o** (aun por escribir): enganchan el probe AHB,
   `glSetHifInfo` (ioremap de `WIFI_HIF_PHYS`/`WIFI_MCU_PHYS`, `PDev`/`Dev`),
   `glBusSetIrq`, `init_completion(rPendComp)`, `timer_setup`, y las cfg80211 ops.
4. **Core stock dieted** (`common/ mgmt/ nic/`): descomentar por bloques. Iterar
   con `nm -u mtk_mtwifi.o | sort -u` sobre los *undefined* hasta cerrar.

Comando de build:
```
make -C <kdir-7.0.12> M=$(pwd) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules
```

## Simbolos que probablemente falten en el PRIMER `make` (para iterar)

### (a) Definidos en TU excluidos por el diet STA-only
Si el core dieted aun los referencia, apareceran como *undefined reference*.
Accion: o bien re-anadir el `.o` (descomentar en el Makefile), o guard-out el
caller con el `#if CFG_*` correspondiente.

- **P2P** (`kalP2P*`, `p2pFsm*`, `wlanoidSetP2p*`, `scanP2p*`, `bssCreateP2p*`):
  de `mgmt/p2p_*.c`, `nic/p2p_nic.c`, `os/linux/gl_p2p*.c`. Deberian estar todos
  bajo `CFG_ENABLE_WIFI_DIRECT` en el core; si alguno se cuela, guard-out.
- **BOW** (`bowRun*`, `kalGetBow*`, `kalSetBow*`, `kalIndicateBOWEvent`,
  `wlanoidSetBow*`): de `common/wlan_bow.c`, `os/linux/gl_bow.c`.
- **TDLS** (`Tdlsex*`, `tdls*`): de `mgmt/tdls*.c` (`CFG_SUPPORT_TDLS=0`).
- **RESET** (`kalIsResetting`, `glResetTrigger`, `wlanRst*`): de
  `os/linux/gl_rst.c` (`CFG_CHIP_RESET_SUPPORT=0`). El contrato de los shims ya
  compila-fuera `kalIsResetting`.
- **WAPI/HS20/WNM** (`wapiParse*`, `hs20*`, `wnm*`): de `mgmt/{wapi,hs20,wnm}.c`;
  descomentar en el Makefile si hacen falta.
- **AGPS** (`kalIndicateAgpsNotify`): ya declarado en `gl_kal.h` bajo
  `CFG_SUPPORT_AGPS_ASSIST=0`.

### (b) A implementar en el glue NUEVO (los provee Fase 1-2, no el core)
Apareceran undefined hasta que existan `gl_init.c`/`gl_cfg80211.c`:

- `glSetHifInfo` / `glClearHifInfo` / `glBusInit` / `glBusRelease` (ioremap +
  `dma_alloc`; pueblan `rHifInfo`: `HifRegBaseAddr`, `McuRegBaseAddr`, `Dev`,
  `PDev`). **Sin esto `mt6582-hif.c` referencia `rHifInfo` vacio.**
- `glStartTxThread` / `glStopTxThread` (los usa `gl_init`; NO estan en
  `gl_kal_prototypes.h` -> declararlos en `gl_init.h` o al final del contrato).
- `glHifIrqMask` / `glHifIrqUnmask` (los EXPORTA `mt6582-hif.c`; los llama
  `gl_cfg80211` en `.disconnect`/`.connect` para la ventana `disc_settle`).
- `wlanAdapterStart` / `wlanAdapterStop` / `wlanProbe` / `wlanRemove` /
  `wlanIST` / `wlanReleasePendingOid` (del core; visibles via `precomp.h`).

### (c) Colisiones de macro / redeclaracion conocidas
- `BIT()` / `BITS()` de `gl_typedef.h` vs `<linux/bits.h>` -> `#undef` antes.
- `kalSetTimer` **declarado DOS VECES** en `gl_kal.h` (forma `OS_SYSTIME` y
  forma `UINT_32`) -> unificar a `(P_GLUE_INFO_T, UINT_32) -> BOOLEAN`.
- `kalGetTimeTick` / `kalGetBootTime` siguen siendo macros de `gl_kal.h`; las
  definiciones-funcion de los shims van bajo `#ifndef` (dejar como macro).
- `ALIGN_4` (gl_kal_fw.c) con guard `#ifndef` vs `config.h`.
- `GLUE_FLAG_INT_BIT`: el `gl_os.h` stock lo pone en **1**; el fallback del
  `hif.h` del contrato lo pone en **2**. **ALINEAR** (el ISR y el tx_thread
  DEBEN coincidir) — usar el valor de `gl_os.h` y quitar el `#ifndef` fallback.

## Deltas de API 3.10 -> 7.0.12 aplicados en los bodies (recordatorio)

- timers: `timer_setup` + `timer_container_of` + `timer_delete_sync`.
- `cfg80211_scan_done(req, &(struct cfg80211_scan_info){.aborted=b})`.
- `cfg80211_disconnected(dev, reason, ie, len, locally_generated, gfp)`.
- `cfg80211_roamed(dev, &(struct cfg80211_roam_info){...}, gfp)`.
- `cfg80211_inform_bss_frame_data(...&(struct cfg80211_inform_bss){.signal}...)`.
- `cfg80211_rx_mgmt(wdev, freq, sig_dbm, buf, len, flags)`.
- `.update_mgmt_frame_registrations` (no `mgmt_frame_register`).
- add/get/del_key + set_default_(mgmt_)key: param extra `int link_id` (ignorar).
- `mgmt_tx(wiphy, wdev, struct cfg80211_mgmt_tx_params*, u64* cookie)`.
- `get_station`: `sinfo->filled = BIT_ULL(NL80211_STA_INFO_*)`.
- bandas `NL80211_BAND_2GHZ/5GHZ`; reg custom via
  `wiphy_apply_custom_regulatory` + `REGULATORY_CUSTOM_REG`.
- RX `netif_rx` (no `netif_rx_ni`); `dev_alloc_skb` + `eth_type_trans`.
- firmware: `request_firmware("mediatek/mt6582/WIFI_RAM_CODE", dev)` +
  `release_firmware`; NADA de `set_fs`/`get_fs`; `kernel_read` para ficheros.
- `wdev->sme_state` ELIMINADO -> estado en `GLUE_INFO` (`eParamMediaStateIndicated`).
- MEDIA_CONNECT -> `cfg80211_connect_bss`/`cfg80211_connect_result`;
  MEDIA_DISCONNECT -> `cfg80211_disconnected`.
- **wiphy_lock (>=5.12)**: las cfg80211 ops corren con el wiphy mutex cogido ->
  `kalIoctl` DEBE llevar `wait_for_completion_timeout(~2-3s)`, porque ESTE FW
  cuelga en `SET_BSS_INFO`; sin timeout el cuelgue retiene el wiphy_lock para
  siempre (rfkill/ip/wpa_supplicant muertos).

## Firmware runtime

Colocar el blob stock en:
```
/lib/firmware/mediatek/mt6582/WIFI_RAM_CODE
```

## TODOs agregados de TODOS los shims (Fase 2-4)

### gl_kal_prototypes.h
- Verificar que `P_QUE_ENTRY_T` (link.h/queue.h) y
  `ENUM_CHNL_EXT_T`/`ENUM_BAND_T`/`P_RF_CHANNEL_INFO_T` (nic/mac.h, wlan_def.h)
  esten visibles via `gl_kal.h`; anadir includes `nic/` que falten.
- `kalSetTimer` duplicado en `gl_kal.h` -> unificar a `(P_GLUE_INFO_T,UINT_32)->BOOLEAN`.
- `kalTimeoutHandler` mantiene `unsigned long arg`; el .c usa
  `timer_setup(&tickfn, trampolin, 0)` + `timer_container_of`.
- `kalIoctl` con `wait_for_completion_timeout(~2-3s)` (wiphy_lock).

### hif.h / mt6582-hif.c
- Confirmar `#undef BIT`/`#undef BITS` (gl_typedef) ANTES de `hif.h`.
- `GLUE_INFO_T` real debe declarar `GL_HIF_INFO_T rHifInfo;` y `ADAPTER_T *prAdapter;`.
- `hif_hstcr` usa `MCR_HSTCR`/`HSTCR_*` de `mt6582-wifi-reg.h`; confirmar que no
  colisiona con un `mtreg.h` stock que redefina `MCR_HSTCR`.
- `glBusSetIrq` necesita `h->PDev` seteado en `glSetHifInfo`.
- Confirmar que `gl_init` llama `glBusSetIrq(ndev, NULL, prGlueInfo)`.
- `GLUE_FLAG_INT_BIT`/`GLUE_FLAG_HALT` fallback defensivo; alinear con `gl_os.h`.
- Falta `glSetHifInfo`/`glClearHifInfo`/`glBusInit`/`glBusRelease` (van en gl_init).
- El bloque enhance de 88B (`Port==MCR_WHISR`, `len=88`) requiere que
  `nicRxEnhance` pida `len=sizeof(ENHANCE_MODE_DATA_STRUCT_T)=88`.

### gl_kal_thread.c
- Verificar que `GLUE_INFO_T` declara: `main_thread`, `prDevHandler`, `waitq`,
  `u4Flag`, `rTxQueue`, `rCmdQueue`, `OidEntry`, `rPendComp`, `rHaltComp`,
  `rSpinLock[]`, `i4TxPendingFrameNum`, `i4TxPendingSecurityFrameNum`,
  `u4OidCompleteFlag`, `u4OsMgmtFrameFilter`, `prAdapter`.
- `GLUE_FLAG_INT_BIT` en `gl_os.h` es **1** (no 2 del fallback hif.h): alinear.
- `ktime_get_boot_ns()` existe en 7.0.12; si `kalGetBootTime` ya es macro, borrar.
- `glStartTxThread`/`glStopTxThread` no estan en el contrato -> declarar en `gl_init.h`.
- `wlanReleasePendingOid(prAdapter, 0)`: confirmar firma.
- Confirmar que `wlanIST` (no `wlanISR`) es el punto de entrada del IST.

### gl_kal_ioctl.c
- Verificar campos de `GL_IO_REQ_T`
  (`prAdapter,pfnOidHandler,pvInfoBuf,u4InfoBufLen,pu4QryInfoLen,fgRead,fgWaitResp,rStatus,u4Flag,fgIsP2pOid`).
- Confirmar campos de `GLUE_INFO_T` (`OidEntry,rPendComp,rPendStatus,u4OidCompleteFlag,ioctl_sem,waitq,u4Flag,rCmdQueue,prAdapter`).
- `reinit_completion` existe en 7.0.12; `rPendComp` con `init_completion()` en el probe.
- `kalIsResetting` no declarado (`CFG_CHIP_RESET_SUPPORT=0`) -> bloque fuera.
- Cadena de includes de `wlanoidClearTimeoutCheck`/`cmdBufFreeCmdInfo`/`QUEUE_*`/`GLUE_SPIN_LOCK_*` via precomp.h — validar Fase 3-4.
- `DBGLOG` doble-parentesis (macro variadica stock).

### gl_kal_timer.c
- `kalSetTimer` duplicado en `gl_kal.h` -> unificar.
- `prTimerHandler` de `kalOsTimerInitialize` se ignora (trampolin fijo despierta
  el tx_thread -> `cnmTimerDoTimeOutCheck`); confirmar que el bucle del tx_thread
  trata `GLUE_FLAG_TIMEOUT_BIT`.
- `kalGetTimeTick` sigue macro; definicion-funcion bajo `#ifndef`.
- `timer_container_of`/`timer_setup`/`timer_delete_sync` requieren `<linux/timer.h>` 7.0.12.

### gl_kal_indicate.c
- Verificar `WLAN_STATUS_UNSPECIFIED_FAILURE` vs `REASON_CODE_UNSPECIFIED`
  (status IEEE vs reason) para `cfg80211_connect_result`.
- `cfg80211_connect_bss` arg `timeout` (`NL80211_TIMEOUT_UNSPECIFIED`): confirmar firma 7.0.12.
- `struct cfg80211_inform_bss.scan_width` podria estar eliminado en 7.0.12 -> borrar si no compila.
- `kalGetAisChannel` asume 5GHz para canal>14 (port es 2.4GHz-only); `NL80211_BAND_5GHZ` debe existir aunque no se registre.
- `kalGetBootTime()` (gl_kal_thread.c) para el timestamp del beacon.
- `SPIN_LOCK_NET_DEV`/`GLUE_*_SPIN_LOCK`/`RCPI_TO_dBm`/`MAC2STR`/`DBGLOG`/
  `wlanQueryInformation`/`scanReportBss2Cfg80211`/`nicChannelNum2Freq`/
  `wlanReturnPacket` via precomp.h.

### gl_kal_fw.c
- `glSetHifInfo` debe fijar `rHifInfo.Dev = &pdev->dev` ANTES del primer
  `wlanAdapterStart` (si no, `kalFirmwareOpen` -> FAILURE por `Dev` NULL).
- Blob en `/lib/firmware/mediatek/mt6582/WIFI_RAM_CODE`.
- `kalFirmwareOpen/Close/Load/Size` son `static`; promover si otro TU los usa.
- `DBGLOG`/`ASSERT`/`kalMemCopy` via precomp.h; `ALIGN_4` con guard `#ifndef`.
- Opcion Fase 2: devolver `prFw->data` directo (sin `vmalloc`) para ahorrar RAM.

### gl_kal_mem.c
- `kalUpdateRxCSUMOffloadParam` bajo `CFG_TCP_IP_CHKSUM_OFFLOAD` (=0 por defecto).
- `eth_hw_addr_set()` requiere kernel >=5.15; si <5.15 usar `dev_addr_set()`.
- `kalIsResetting` con guard (`CFG_CHIP_RESET_SUPPORT=0`); borrar si el core no lo referencia.
- `kalProcessRxPacket` asume `pucPacketStart` en el area lineal del skb (valido con PIO; revisar con PDMA Fase 3).
- `COPY_MAC_ADDR`/`GLUE_DEC_REF_CNT`/`NETWORK_TYPE_AIS_INDEX` via precomp.h.
