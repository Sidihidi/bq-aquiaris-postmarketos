# HANDOFF — Port mt_wifi Fase 3/M1: probe ARRANCA en profundidad, cuelga en la descarga por CHIP CALIENTE (Mac/.123, 2026-07-03)

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

## SIGUIENTE = Opción A (cold-boot, la definitiva)
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
