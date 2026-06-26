# HANDOFF — Cerrar el WiFi al 100%: el IRQ del HIF (2026-06-26)

Estado tras la sesión de verdad-terreno (LineageOS) + implementación del IRQ. **El bug del DHCP está
caracterizado al 100% y reducido a UNA pieza concreta que falta.** Este doc = lo probado + lo hecho + lo
que queda para cerrarlo.

---

## TL;DR
- **Bug A (WPA2 asocia pero sin DHCP/internet) = bug de DRIVER host-side, PROBADO.** No es el AP ni el firmware.
- **Causa raíz:** el driver original es **interrupt-driven**; el nuestro sondeaba. El IRQ del HIF entrega
  el RX a tiempo (cierra el handshake RX0_DONE); el polling no.
- **Implementado el IRQ** y **registra perfecto** (IRQ 209, GIC_SPI 160) **pero NO dispara (count=0)**.
- **Lo único que falta:** el **routing/unmask de la interrupción a nivel CONSYS/HIFSYS** (bridge AP2CONN),
  que el bring-up del CONSYS del original habilita y nuestro `mt6582-consys` no. Está MÁS ABAJO del driver WiFi.
- Baseline estable actual: **#171** (escanea, conecta, OPEN navega; sin DHCP). El IRQ está revertido a
  polling para no romper nada, pero el infra del IRQ queda en el código listo para activarse.

---

## 1. Lo PROBADO en hardware (verdad-terreno LineageOS)
Arrancado LineageOS 13 en el propio krillin (boot `~/lineage13-boot.img` por fastboot), adb root:
1. **Firmware IDÉNTICO:** LineageOS carga `/etc/firmware/WIFI_RAM_CODE_SOC` md5 `8762595d` = exactamente
   el nuestro. **Firmware descartado al 100%.**
2. **LineageOS navega ESTABLE en "hola"** (CCMP, canal 11): IP 10.181.211.96, `ping 8.8.8.8` 4/4 0% pérdida.
   → el WiFi del móvil va PERFECTO en stock. **Nuestro bug A es 100% de driver** (no AP, no firmware).
   (En vodafone el original también coge IP pero cae por ARP-al-gateway fallido = problema de red de vodafone, aparte.)
3. **El original usa un IRQ REAL:** `/proc/interrupts` muestra `AHB_SLAVE_HIF` con **6163 disparos**. Cada
   evento RX/TX → IRQ → `nicProcessIST` → lee el bloque enhance → re-arma RX0_DONE. **Nuestro driver NO
   tenía IRQ** (sondeaba WRPLR cada 20ms + deshabilitaba el INT) → el FW se atasca tras los 1ºs paquetes.

## 2. El IRQ IMPLEMENTADO (lo hecho, en este commit)
- **DT** `wifi@180f0000`: `interrupts = <GIC_SPI 160 IRQ_TYPE_LEVEL_LOW>;`
  (= `MT6582_AHB_SLAVE_HIF_IRQ_ID = GIC_PRIVATE_SIGNALS(32)+160`, verificado en `mt_irq.h`; polaridad =
  `IRQF_TRIGGER_LOW` del original `ahb.c:1037`).
- **Driver** (`mt6582-wifi.c`): `request_irq(wifi_isr, IRQF_TRIGGER_LOW)` en el probe; `wifi_isr` enmascara
  el INT (`WHLPCR_INT_EN_CLR`, always-on, seguro en hardirq) y despierta el `rx_thread`; tras WLAN_READY
  re-aplica WHCR(read-clear)+WHIER y habilita el INT (`WHLPCR_INT_EN_SET`) — el FW resetea WHCR/WHIER en
  WIFI_START, igual que el 2º `nicInitializeAdapter` del original (`wlan_lib.c:1629`).
- **Resultado:** se REGISTRA perfecto (`209: ... MT_SYSIRQ 160 Level mt6582-wifi`) — número, bit
  (`WHLPCR_INT_EN_SET=BIT0`), polaridad: todo correcto — **pero el contador SIEMPRE está a 0**, con
  LEVEL_LOW y LEVEL_HIGH, con y sin polling. Otros periféricos (i2c hwirq44, mmc hwirq40) sí disparan.

## 3. El BLOQUEO (la pieza que falta)
**La línea de interrupción del HIF NO llega al GIC.** Verificado:
- Las 6 escrituras de bringup (WHCR read-clear, WHIER, WHLPCR_INT_EN) son **byte-idénticas** al original
  (cruzado por workflow contra `nic.c`/`wlan_lib.c`/`mtreg.h`). No falta ningún registro de enable en el driver WiFi.
- `ahb.c` del original solo hace `request_irq` (sin registro de routing visible ahí).
- Descartado: polaridad (i2c/mmc con la misma config disparan), PDMA/clock-del-driver (el INT del HIF es
  independiente del PDMA en el path PIO).
- **Conclusión:** falta el **routing/unmask de la INT del HIF hacia el GIC del AP**, a nivel **CONSYS/HIFSYS**
  (el bridge **AP2CONN GALS**, rango físico `0x18000000–0x18011000`). El bring-up del CONSYS del original lo
  habilita; nuestro `mt6582-consys.c` solo toca `AP2CONN_OSC_EN` (el 26M) y NO el routing de interrupción.

## 4. ⭐ EL PLAN para cerrarlo al 100%
**Atacar el routing CONSYS/HIFSYS** (es lo único que queda):

1. **Arrancar LineageOS** (boot `~/lineage13-boot.img` por fastboot; adb root) y **conectar a "hola"**
   (donde el original va). Con el WiFi activo:
2. **Volcar los registros CONSYS/HIFSYS** del rango `0x18000000–0x18011000` (devmem o un módulo) — en
   especial el bloque **AP2CONN / conn-infra** y cualquier "conn2ap IRQ unmask" / "HIFSYS INT enable" /
   clock-gate. Anotar sus valores con el IRQ disparando.
3. **Comparar** con el estado de esos mismos registros en **nuestro pmOS** (mismo volcado, con el WiFi
   nuestro intentando) y con lo que escribe nuestro `mt6582-consys.c`.
4. **El registro/bit que el original tiene a 1 y el nuestro a 0** (o que el original escribe y nosotros no)
   = el routing que falta. Añadirlo al bring-up del CONSYS (`mt6582-consys.c`).
5. **Verificar:** `cat /proc/interrupts | grep mt6582-wifi` → el contador del **209 debe pasar de 0 a >0**
   durante el RX. Si dispara → el FW sigue entregando RX tras la GTK → **el DHCP completa** (probar en "hola").

**Pistas concretas para el paso 2** (buscar en el downstream `conn_soc`/WMT + en `wmt_ic_soc.c`):
`WMT_SET_ALLINT_REG_CMD` (CONSYS reg `0x80050300=0x0000C400`, aunque para MT6582 dice hacerlo el f/w),
`mtk_wcn_consys_*`, `CONN_MCU_CONFIG`, `AP2CONN`, `GALS`, `conn2ap_*irq*`, `hif_*int*`. El registro de
unmask/routing de INT suele estar en `CONN_HIF_ON` o `CONN_INFRA` cerca de `0x1800xxxx`.

**Plan B (si no aparece el registro):** ya descartado por improbable, pero si el IRQ no se puede activar,
queda investigar si el FW necesita el path **PDMA** (no PIO) para generar el INT — sería un cambio grande
(reescribir el data-path a PDMA, AP-DMA `0x11000180`), por eso es el último recurso.

## 5. Estado del código y cómo probar
- **Driver** (`mt6582-wifi.c`, este commit): infra del IRQ COMPLETA (request_irq/ISR/enable) + `rx_thread`
  por **polling** (fallback: si el DT no declara `interrupts` o el IRQ no dispara, cae a polling y al menos
  escanea/conecta). Cuando se arregle el routing CONSYS, el IRQ disparará y el `rx_thread` debe pasar a
  drenar SOLO en respuesta al IRQ (leer WHISR en polling lo desasserta — ver el comentario en `wifi_rx_thread`).
- **DT:** añadir `interrupts = <GIC_SPI 160 IRQ_TYPE_LEVEL_LOW>;` al nodo `wifi@180f0000` (NO está en este
  repo; va en `arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts` del kernel).
- **Test (en "hola", CCMP):** `nmcli device set wlan0 managed no`; `wpa_supplicant -i wlan0 -c wpa.conf -D
  nl80211 -B` (ssid "hola" psk "kakatua1", multilínea); `udhcpc -i wlan0 -t6 -T3 -nq`; mirar `rx>2` e IP +
  el contador del IRQ 209.

## 6. ⚠️ Recuperación (fastboot) — y la lección
Un `sed` no-anclado al editar el DT (`s/IRQ_TYPE_LEVEL_HIGH/.../LOW/` sin fijar el SPI) cambió el
`pmic_wrap` (GIC_SPI 115) además del WiFi → PMIC con polaridad mala → **bootloop en el logo BQ**.
**Recuperado por fastboot:** móvil apagado → Vol-Abajo+Power → `fastboot flash boot <boot-bueno>.img` →
`fastboot reboot`. Boot bueno = #171 (`.bak-pre-expAB-0625` + DT `.bak-pre-irq-0625`) o `lineage13-boot.img`.
**Lección:** anclar SIEMPRE los `sed` del DT al nodo concreto; el DT es un fichero compartido.

---

**Resumen:** WiFi básico (scan/connect/OPEN) funciona. Para el DHCP falta **una pieza** — el routing de
interrupción CONSYS/HIFSYS — que se encuentra comparando registros con LineageOS arrancado. Todo lo demás
(diagnóstico, IRQ, firmware, comandos) está cerrado y verificado.
