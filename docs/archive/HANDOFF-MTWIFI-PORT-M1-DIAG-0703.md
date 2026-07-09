# HANDOFF — Port mt_wifi: **M1 CONSEGUIDO** ✅ (Mac/.123, 2026-07-03)

## ★★★ M1 LOGRADO Y ESTABLE (0703 PM) ★★★
Con **chip frío fiable** (driver A bindea el nodo pero NO auto-descarga el FW — schedule del
auto_bringup deshabilitado, kernel #233), el port arranca el FW **de punta a punta, limpio**:
```
firmware-arrancado=0  (chip frio)
portW #1..#12 POST OK  → descarga WIFI_RAM_CODE completa (12 chunks, cada uno ACK'd por portR de 8B)
Waiting for Ready bit.. -> Ready bit asserted   (WLAN_READY=1, el FW ARRANCO)
wlanAdapterStart status=0x00000000 (SUCCESS)
nicTxReleaseResource: Release TC4 count 1, Free=4   (¡flow-control TX del core FUNCIONA!)
*** mtk_mtwifi: wlan0 ARRIBA (FW arrancado, cfg80211 registrado) ***
wlan0 UP (ndo_open rc=0)   — NO-CARRIER (scan = stub Fase 4)
```
- ✅ insmod rc=0, `mtk_mtwifi` cargado, **wlan0 registrado y UP**, tx_thread vivo, **CERO corrupción**
  (userspace intacto, sin Oops/BUG). wlanAdapterStart COMPLETA (download + WIFI_START + WLAN_READY +
  BASIC_CONFIG + GET_NIC_CAPABILITY + wlanLoadManufactureData, todo SUCCESS).
- La "corrupción de memoria" que se vio con el enfoque of_ids-off era **un artefacto de aquel setup**
  (chip mal calibrado/estado raro), NO un bug real. Con driver A bindeado-pero-idle (RF-cal del boot
  presente + chip frío) todo va limpio. Overruns de buffer ya estaban descartados (RFB/coalescing bien
  dimensionados, PIO sin DMA).
- **CLAVE del chip frío**: el port NECESITA que nadie haya descargado el FW antes (VCN33 always-on impide
  enfriarlo en runtime). En el producto final el port sera el UNICO driver WiFi (driver A fuera) -> chip
  siempre frío al boot -> M1 nativo. Para tests con driver A presente: deshabilitar su auto_bringup
  (`schedule_delayed_work(&w->auto_bringup...)` en mt6582-wifi.c:2175) o su binding.
- **SIGUIENTE = Fase 4**: `gl_cfg80211.c` real (scan+connect+keys) para sustituir el `mtk_cfg80211_ops`
  __weak stub → `iw dev wlan0 scan` lista APs (M2), luego connect+4-way por SECURITY_FRAME = DHCP (Fase 5).
- Instrumentación DIAG (portW/portR PRE/POST + WCIR dump) en `mt6582-hif.c`: QUITAR ya (M1 validado).

---

# (histórico) Fase 3/M1: probe ARRANCA en profundidad, cuelga en la descarga por CHIP CALIENTE

> Continúa `HANDOFF-MTWIFI-PORT-FASE3-PROBE-0702.md`. Sesión Mac (Fable 5) con el móvil en la .123.

## TL;DR — avance ENORME, causa raíz aislada
Con el probe de Fase 3 de casa (`gl_init.c`, power-cycle de chip-frío) + instrumentación mía en `mt6582-hif.c`,
el probe corre **mucho** más lejos que el `0xc0000001` de chip-caliente:
```
power-cycle del consys WiFi (func_off+VCN33 off -> frio)
gl_init: HIF/MCU/PDMA mapeados ... IRQ 212 registrado
DIAG: FW mapeado OK (207648 bytes) -> wlanAdapterStart...
wlanAdapterStart(): Acquiring LP-OWN 0
nicVerifyChipID: Chip ID: 0x6582        <- BACKEND HIF LEE EL CHIP (kalDevRegRead OK)
wlanAdapterStart(): fgValidHead == TRUE
nicTxAcquireResource: Acquire TC=0 aucFreeBufferCount=7
DIAG HIF-cfg pre-DL: WHCR=0x0 WHIER=0xffffff0f WHLPCR=0x100 WHISR=0x13
DIAG portW #1 port=0x28(WTDR0) len=2072 tgt=0 PRE   <- CUELGA AQUÍ (writel WTDR0), sin POST -> WDT
```

## Validado (de-riesga el port de golpe)
- ✅ **Backend HIF `mt6582-hif.c` conduce el chip**: `kalDevRegRead` lee Chip ID `0x6582`.
- ✅ **Power-cycle de chip-frío del probe se ejecuta**; `wlanAdapterStart` corre en profundidad (LP-OWN →
  verify chip → nicInitializeAdapter → entra en la descarga del FW).
- ✅ **Config del HIF pre-descarga == driver A**: WHCR=0 (MAX_HIF_RX_LEN=0, RX_ENHANCE=0, W_INT_CLR=0),
  WHIER=0xffffff0f, WHLPCR=driver-own. NO es divergencia de config.
- ✅ **Primitivas byte-idénticas al driver A** (que descarga el FW sin fallar): `hif_hstcr`≡`wifi_hstcr`,
  `kalDevPortWrite`≡`wifi_port_write_pio`. NO es la primitiva.

## CAUSA RAÍZ del cuelgue: chip CALIENTE (no frío de verdad)
El WARN del pstore lo confirma:
```
WARNING _regulator_disable: unbalanced disables for vcn33_wifi
```
**`ldo_vcn33_wifi` es `regulator-always-on`** (fix de la flakiness WLAN_READY, [[project-wifi-wlan-ready-flakiness-fix]])
→ el `regulator_disable(VCN33)` del power-cycle **falla** → el chip **conserva la alimentación y el FW que el
driver A cargó al boot**. `wlanAdapterStart` intenta re-descargar el FW sobre uno medio-vivo → el `writel` a
WTDR0 no completa en el AHB → WDT. WHISR=0x13 (RX0_DONE + TX_DONE pendientes) = residuo del FW vivo.
Este confounder está en TODAS las tentativas por `insmod` (el driver A siempre arranca el FW al boot).

Descartado en el proceso: la ISR disparándose a mitad del burst (probé enmascarar el IRQ del HIF durante
`wlanAdapterStart` — commit incluido, correcto vs driver A pero NO era el bloqueo de M1).

## RESET RUNTIME BLOQUEADO (probado 0703 PM) — el reset fiable es a nivel BOOT
Instrumenté el power-cycle del probe + un dump de WCIR pre-descarga. Resultado en HW:
```
func_off(WIFI)=0   vcn33(false)=-5
pre-DL: WCIR=0x00306582 (chip=0x6582 WLAN_READY=1) WHCR=0 WHIER=0xffffff0f WHLPCR=0x100 WHISR=0x13
```
**`func_off(WIFI)` devuelve OK pero WLAN_READY SIGUE A 1** → el WMT apaga la *función* WIFI pero
**VCN33 `regulator-always-on` mantiene el MAC alimentado → el FW sigue vivo → la re-descarga cuelga.**
Cortar VCN33 para enfriar de verdad está bloqueado: (a) `regulator_disable` falla (-EIO, always-on),
(b) cortar VCN33 pierde la cal-RF que NO es re-emitible en runtime (lección driver A) → WLAN_READY nunca.
**=> El reset de CONSYS en RUNTIME es físicamente inviable. El único chip frío+calibrado fiable es el del
ARRANQUE** (bring_up_chip hace la RF-cal con VCN33 on, ANTES de cualquier descarga de FW).

## SIGUIENTE (revisado) = chip frío por BOOT + depurar la corrupción de memoria
Ruta fiable: que driver A **NO descargue el FW al boot** (deshabilitar su auto-bringup, o su binding) →
el port descarga a un chip frío+calibrado (PROBADO: la escritura WTDR0 NO cuelga en ese caso). El bloqueo
real que queda entonces es la **corrupción de memoria** tras el inicio de la descarga (segfaults, ramoops
vaciado) → hace falta captura a prueba de corrupción (dmesg -w en vivo, YA usado) + depurar el overrun
(candidatos: RX-aggregation del core stock sobre nuestro HIF PIO, o un buffer del path de descarga).
GOTCHA: al deshabilitar driver A el móvil arranca sin WiFi; si el SD-boot crashea cae a boot INTERNO
(driver A on) → reflashear SD tras tests.

## (obsoleto) Opción A (cold-boot, la definitiva)
Kernel con **driver A deshabilitado** → chip genuinamente frío al boot (nadie carga el FW) → nodo libre →
`insmod` port → `wlanAdapterStart` descarga a chip frío → **debería completar = M1**.
- Deshabilitar driver A: `CONFIG_MT6582_WIFI=n` (o vaciar su `of_device_id`) → rebuild zImage → flash → boot.
- Riesgo: sin driver A de fallback si el port cuelga (reversible: reflashear el kernel con driver A).
- Si con chip frío AÚN cuelga el WTDR0 → entonces sí es el backend HIF en el path de descarga (comparar el
  stream de comandos INIT del stock vs el del driver A con la instrumentación DIAG portW/portR ya puesta).

## Instrumentación DIAG dejada en `mt6582-hif.c` (quitar tras M1)
`kalDevPortWrite/Read`: prints PRE/POST (primeras 12) + dump WHCR/WHIER/WHLPCR/WHISR en la 1a escritura.
El "PRE sin POST" localiza el cuelgue al `writel`/`readl` exacto.

## Receta (Pi .123, móvil en .123)
Build: `make O=build-krillin ... CONFIG_MTK_MTWIFI=m M=drivers/net/wireless/mtk_mtwifi modules`.
Test: `unbind` driver A → `insmod /tmp/mtk_mtwifi.ko` → leer `/var/log/pstore/` (guardian instalado, salva
el crash tras el auto-reboot del WDT). El móvil auto-recupera del WDT (~2min).

*Sesión Mac (Fable 5), 2026-07-03. M1 a un cold-boot de distancia.*
