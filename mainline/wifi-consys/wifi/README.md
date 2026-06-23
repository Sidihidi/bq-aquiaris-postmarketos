# WiFi cfg80211 (CONSYS MT6582)

Driver WiFi **full-MAC / cfg80211** del combo CONSYS del MT6582. Decisión
arquitectónica clave: el WiFi **NO** va por el enlace BTIF/STP (eso es BT y GPS,
ver `../m3a/`); tiene su **propio bloque HIF estilo-SDIO** en el bus AHB
(`0x180F0000`, registros "MCR": WCIR/WHLPCR/WHISR/…) y un canal PDMA propio en
`0x11000180`. STP/WMT sólo enciende la radio (`func_on(WIFI)`) y coordina el reset;
el firmware `WIFI_RAM_CODE` se descarga por el puerto de datos del HIF (WTDR0).

**Estado: ⚠️ Fase 2.** Scan ✅ (escanea 14 redes reales; la MAC arranca con la
RF-cal). `.connect` en progreso: la coreografía completa (UPDATE_STA_RECORD + RLM)
está implementada y dispara la asociación, pero **falta CH_PRIVILEGE** (el FW no
asocia sin que se le conceda el canal) y **WPA2** (Fase 3, EAPOL + `.add_key`).
Depende de que `../m3a/` tenga el CONSYS vivo (probe hace `EPROBE_DEFER` hasta
`mt6582_consys_ready`).

## Ficheros

- **`mt6582-wifi.c`** — el driver: probe → `func_on(WIFI)` → ioremap HIF+PDMA →
  descarga de firmware → `WIFI_START` → cmd/event, scan y connect (cfg80211 ops +
  netdev). Disparado por debugfs, igual que el bring-up del BTIF.
- **`mt6582-wifi-reg.h`** — registros MCR del HIF, mapa del PDMA y structs de
  descarga de firmware, extraídos del downstream (`mtreg.h`, `hif.h`, `hif_pdma.h`).

## Documentación detallada

- **`WIFI-DESIGN.md`** — diseño completo: la decisión HIF-vs-STP (§0), el mapa de
  registros, la descarga de firmware y el modelo cmd/event.
- **`WIFI-ROADMAP.md`** — fases del driver (0: scaffold → 1: cmd/event → 2: scan +
  connect → 3: WPA2/netdev) y estado de cada una.
- **`FASE2-CONNECT.md`** — detalle de la Fase 2 `.connect`: la coreografía
  UPDATE_STA_RECORD, qué dispara la asociación y lo que falta (CH_PRIVILEGE).
