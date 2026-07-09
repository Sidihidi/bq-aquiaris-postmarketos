# HANDOFF — PORT mt_wifi → 7.0.12: Fase 3 (probe real) CODE-COMPLETE, pendiente test HW (Windows/.38, 2026-07-02)

> Continúa `PARA-CASA-PORT-SIGUIENTE-FASE3.md` y `HANDOFF-MTWIFI-PORT-FASE1-KO-0702.md`.
> Reparto: **Casa/.38 = Fase 3 (probe)** ← este handoff. **Mac/.123 = Fase 4 (gl_cfg80211.c)**.

## TL;DR
✅ **`gl_init.c` reescrito como el PROBE REAL** (756 L, era el stub mínimo de Fase 1.5). **`mtk_mtwifi.ko`
COMPILA Y ENLAZA** con el probe (1.90 MB, símbolos `mtk_wlanProbe`/`mtk_wlanRemove`/`wlanNetCreate`
presentes; externos del consys del driver A sin resolver = se enlazan en el kernel, OK).
⏳ **NO probado en HW todavía**: el móvil se cayó del USB (`usb 1-1: USB disconnect`) estando idle, ANTES
de correr el primer `insmod`. Requiere power-cycle del usuario para el primer tiro.

## Qué hace el probe (`mt_wifi_port/glue/gl_init.c`)
**Insight de diseño:** en el port de core-stock, TODO el bring-up de registros del HIF (chip-id, WHCR/
WHIER/WHLPCR, descarga de `WIFI_RAM_CODE`, WIFI_START, poll WLAN_READY, BASIC_CONFIG/NIC_CAPABILITY) lo
hace **`wlanAdapterStart()` DENTRO del core**, vía las macros `HAL_*` → `kalDev*` de `mt6582-hif.c`. Por eso
el probe **NO toca registros** (a diferencia del `wifi_bringup()` monolítico del driver A): solo enciende el
consys, puebla `rHifInfo` con los ioremaps, y delega en `wlanAdapterStart`.

- `platform_driver` con `of_match` = `"mediatek,mt6582-wifi"` (misma compatible que el driver A → hay que
  **unbind el driver A** del nodo `180f0000.wifi` antes del `insmod`, o el port no captura el device).
- **`mtk_wlanProbe`**: EPROBE_DEFER(`!mt6582_consys_ready`) → `func_on(WIFI)` → VCN33 always-on →
  `wlanNetCreate` (wiphy+netdev) → `glSetHifInfo` (ioremap HIF/MCU/PDMA) → `glBusSetIrq` (GIC_SPI 184) →
  **bloque FW-download STOCK**: `kalFirmwareImageMapping` → **`wlanAdapterStart(prAdapter, prRegInfo,
  prFwBuffer, u4FwSize)`** → `kalFirmwareImageUnmapping` → `kthread_run(tx_thread)` → MAC por
  `kalIoctl(wlanoidQueryCurrentAddr)` + `eth_hw_addr_set` → `wlanNetRegister` → `g_u4HaltFlag=0`.
- `mtk_wlanRemove` (void, 7.0.12): unwind escalonado + `wlanAdapterStop` + VCN33 off.
- **Dependencia Fase 4:** `mtk_cfg80211_ops` es un **stub `__weak`** local (`.scan/.connect/... → -EOPNOTSUPP`)
  para que enlace y `wlan0` se registre. Cuando el Mac suba `gl_cfg80211.c` con el `mtk_cfg80211_ops`
  no-weak, el enlazador lo prefiere solo. M1 (FW arranca) NO depende de Fase 4.

## Instrumentación de MÁXIMOS DATOS (para no gastar ciclos a ciegas) — QUITAR tras diagnosticar
Dos añadidos DIAGNÓSTICOS en este `gl_init.c` que colapsan el árbol de prueba-error en 1 tiro:
1. **DBGLOG del core activado** (`module_init`: `aucDebugModule[*]=0x1F`) → `wlanAdapterStart` imprime el
   **sub-paso EXACTO** que falla: `nicInitializeAdapter failed` (chip-id/driver-own = backend HIF roto),
   `Firmware download failed` (TX del backend), o pasa → `wlan0` aparece = **M1**.
2. **Power-cycle del consys en el probe** (`func_off(WIFI)` + VCN33 off + `msleep(120)` + `func_on`) →
   fuerza **chip FRÍO**. Razón: el driver A ya arrancó el FW al boot (chip caliente) y `wlanAdapterStart`
   no re-inicia un FW vivo. Distingue: lee `WCIR=0x6582`+driver-own+descarga → backend HIF OK, era el chip
   caliente; falla temprano → bug en `mt6582-hif.c`.
   ⚠️ El flanco VCN33 off→on pierde la cal-RF (scan/TX no irán finos), pero para el TEST del bring-up del FW
   da igual. **Es DIAGNÓSTICO: quitar ambos (DBGLOG 0x1F + power-cycle) una vez validado el path.**
   `mt6582_consys_func_off` está EXPORTADO (`mt6582-btif.c:525`), confirmado.

## Correcciones de compilación aplicadas (Fase 3)
- `gl_init.c:445` comentario `HAL_*/kalDev*` rompía el parser (`*/` prematuro) → reescrito `HAL_x/kalDevX`.
- `glLoadNvram` usado antes de definir (static) → forward-declaration añadida arriba.

## Test listo (en cuanto el móvil vuelva) — 1 comando
`.ko` en la Pi .38: `~/mainline/linux-7.0.12/drivers/net/wireless/mtk_mtwifi/mtk_mtwifi.ko` (md5
`7a8b96844ce0be8743144275d33339b8`). Build:
`make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2 CONFIG_MTK_MTWIFI=m M=drivers/net/wireless/mtk_mtwifi modules`
Test: `echo 180f0000.wifi > /sys/bus/platform/drivers/mt6582-wifi/unbind` → `insmod /tmp/mtk_mtwifi.ko`
→ `dmesg | grep -iE "mtwifi|wlanAdapter|nicInit|Firmware|WLAN_READY|DIAG"`. Envolver en `timeout` (puede
colgar el FW). pstore se salva solo a `/var/log/pstore/`.

## Gotchas pendientes (de los críticos, para Fase 5)
- `HAL_READ_TX_RELEASED_COUNT`/`RX_LENGTH` redefinidos en hif.h Y hal.h → reconciliar o el RX0_DONE no
  se re-arma.
- RX-agg: capar `CFG_SDIO_MAX_RX_AGG_NUM=1` o endurecer el guard WCIR del `kalDevPortRead`.
- `kalIoctl` con timeout (verificar `wait_for_completion_timeout`) — este FW cuelga en SET_BSS_INFO.

## Estado HW
Móvil #230 (driver A). El test anterior dejó el móvil idle y **se cayó del USB solo** (no llegó a correr
el insmod). Power-cycle pendiente. El driver A sigue siendo el WiFi funcional-a-medias del sistema.

*Sesión Windows (Fable 5), 2026-07-02. Probe portado del stock wlanNetCreate/Probe (gl_init.c:2913 stock)
sobre el consys del driver A.*
