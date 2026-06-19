# WIFI-ROADMAP.md — plan por fases del driver WiFi MT6582 CONSYS

Plan realista y priorizado para `mt6582-wifi.c` (cfg80211 full-MAC sobre el HIF AHB @0x180F0000).
Acompaña a `WIFI-DESIGN.md`. Cada fase: objetivo, ficheros del downstream a estudiar, lo difícil, y un
estimado en "sesiones de kernel" (1 sesión ≈ compilar en la otra máquina + flashear + leer dmesg + iterar).

**Hito mental:** la línea que parte el proyecto en dos es **Fase 1** (¿el HIF responde un evento del
firmware tras la descarga?). Si llegamos a un `EVENT_ID_NIC_CAPABILITY` válido, el 70% del riesgo
desaparece y el resto es "rellenar comandos". El otro punto duro es la **integración con el btif**
(Fase 0) por la concurrencia del WMT.

---

## Fase 0 — Engancharse: probe + func_on(WIFI) + descarga de firmware
**Objetivo:** que el módulo cargue, espere al CONSYS, pida `func_on(WIFI)` por WMT, mapee el HIF, gane
driver-own, lea `WCIR == 0x6582`, y **descargue `WIFI_RAM_CODE` entero sin error de CRC**, terminando con
`INIT_CMD_ID_WIFI_START` y `WCIR.WLAN_READY == 1`. **Sin cfg80211 todavía** — todo disparado por debugfs,
como el btif.

**Sub-pasos:**
1. **Ampliar el btif**: hacer `func_on()` exportable y *thread-safe* → `mt6582_consys_func_on(u8 type)`
   (`EXPORT_SYMBOL_GPL`). Serializar con el RX-thread del BT (el WMT comparte el mismo BTIF/rxbuf). Ver
   "Riesgo A".
2. **probe** del `mt6582-wifi`: `-EPROBE_DEFER` mientras `!mt6582_consys_ready`; luego
   `mt6582_consys_func_on(WIFI=3)`.
3. `ioremap(HIF_DRV_BASE=0x180F0000, 0x5c)` y `ioremap(AP_DMA_HIF_BASE=0x11000180, 0x54)`.
4. **Driver-own**: `writel(WHLPCR_FW_OWN_REQ_CLR, base+WHLPCR)`; poll `readl(WHLPCR)&IS_DRIVER_OWN`.
5. Leer `WCIR`; `(val & 0xFFFF) == 0x6582`? log chip-id/rev/ready.
6. `request_firmware("WIFI_RAM_CODE...", dev)`; inspeccionar firma `'MTKW'`; parsear secciones.
7. **Descargar** cada sección en chunks ≤2048B con `INIT_CMD_ID_DOWNLOAD_BUF` por `WTDR0` (PIO basta).
   Con ACK: esperar `INIT_EVENT_ID_CMD_RESULT.ucStatus==0` leyendo `WRDR0`.
8. `INIT_CMD_ID_WIFI_START`; poll `WCIR.WLAN_READY`.

**Ficheros del downstream a estudiar:**
- `os/linux/hif/ahb/ahb.c` (`glSetHifInfo` 677-756 = el orden de mapeos; `kalDevPortRead/Write`).
- `include/nic/mtreg.h` (WCIR/WHLPCR/WHCR/WTDR0/WRDR0 bits).
- `common/wlan_lib.c:1219-1817` (`wlanAdapterStart` = la receta exacta de orden), `:3140-3236`
  (`wlanImageSectionDownload`), `:1318-1396` (parseo de cabecera + bucle de secciones).
- `include/nic_init_cmd_event.h` (todos los structs init), `include/wlan_lib.h:568-583` (cabecera FW).
- `os/linux/gl_kal.c:1123-1150` (`kalFirmwareImageMapping` → en mainline = `request_firmware`).

**Lo difícil:**
- **Riesgo A (el grande): concurrencia BTIF/WMT.** El `func_on(WIFI)` viaja por el mismo BTIF que ya está
  ocupado con el RX-thread del BT. Si mandamos un `wmt_cmd` síncrono mientras el kthread del BT drena
  `rxbuf`, nos pisamos el evento de respuesta. Solución: un mutex "wmt_busy" + que el RX-thread del btif,
  cuando vea un frame de canal WMT(4), lo entregue a un `completion` en vez de descartarlo. ~30-40
  líneas en el btif. **Hacer esto bien ANTES de tocar el HIF.**
- **Riesgo B: ¿deja `func_on(WIFI)` el HIF accesible?** Puede que falte algún clock/power extra del HIFSYS
  (`HIF_DRV_BASE` está en `0x180Fxxxx`, dentro del CONSYS). Si `WCIR` lee `0x0`/`0xFFFFFFFF` tras
  func_on, hay que revisar si el downstream toca algún registro de HIFSYS además del WMT (buscar en
  `ahb.c`/`platform`/`mt_clkmgr` referencias a `0x180F`, HIF clock, `MT_CG_*HIF*`). Plan B: dejar el HIF
  en PIO y depurar con lecturas de `WCIR`.
- **Riesgo C: formato del blob.** Hay que confirmar si el `WIFI_RAM_CODE` que tienes trae cabecera
  `'MTKW'` (seccionado) o es plano. Volcar los primeros 16 bytes. Si es plano, una sola sección a la
  dirección por defecto (buscar la constante en `wlan_lib.c`).

**Entregable de fase:** dmesg con `WCIR=0x...6582`, `*** firmware descargado N secciones ***`,
`WLAN_READY=1`. (Equivalente al "CANAL OK" del btif, pero para el HIF.)

**Estimado: 3–5 sesiones.** (1 para la ampliación btif + concurrencia, 1-2 para el HIF/driver-own/WCIR,
1-2 para clavar la descarga de firmware y el ACK.)

---

## Fase 1 — cmd/event básico + scan pasivo
**Objetivo:** mandar el primer comando "de verdad" y recibir su evento; luego un **scan pasivo** y ver
beacons. Aquí aparece `wlan0` (wiphy + netdev mínimos) para que `iw`/`wpa_supplicant` puedan hablar.

**Sub-pasos:**
1. Implementar `wifi_send_cmd(CID, set/query, payload, len)` (arma `WIFI_CMD_T`+cabecera HIF, PKT_TYPE=CMD,
   `WTDR0`) y un **RX poller** que lee `WRDR0` cuando `WHISR.RX0_DONE`, parsea `HIF_RX_HEADER_T`,
   y despacha por `u2PacketType&3` → EVENT por `ucEID`.
2. `CMD_ID_GET_NIC_CAPABILITY` (query) → `EVENT_ID_NIC_CAPABILITY` (trae MAC permanente, bandas).
   **Este es el "hello world" del firmware.**
3. `CMD_ID_BASIC_CONFIG` (set MAC) + `CMD_ID_SET_DOMAIN_INFO` (canales 1-13).
4. `wiphy_new` + 14 canales 2.4G + cifrados + `wiphy_register`; `alloc_netdev`/`register_netdev` para
   `wlan0` con su `wireless_dev` (iftype STATION).
5. `.scan` op: `CMD_ID_SCAN_REQ_V2` (pasivo, todos los canales). Recibir tramas **MGMT** (beacons) →
   `cfg80211_inform_bss_frame()`. Al `EVENT_ID_SCAN_DONE` → `cfg80211_scan_done()`.

**Ficheros del downstream a estudiar:**
- `nic/nic_tx.c` (`nicTxCmd`/`nicTxInitCmd`: cómo rellena la cabecera y escribe el puerto).
- `nic/nic_rx.c` (`nicRxProcessRFBs` 2401-2436 = el switch de PKT_TYPE; `nicRxProcessEventPacket`).
- `include/nic_cmd_event.h` (structs `CMD_SCAN_REQ_V2`, `EVENT_SCAN_DONE`, `EVENT_NIC_CAPABILITY`;
  **confirmar los valores numéricos de CID/EID aquí**).
- `os/linux/gl_cfg80211.c` (`mtk_cfg80211_scan` 856-918), `gl_init.c:737-897` (bandas/canales/cifrados).
- `mgmt/scan.c` / `mgmt/cnm_scan.c` (cómo arma la lista de canales y los IE de probe).

**Lo difícil:**
- **El RX poller vs IRQ.** En Fase 1 polling de `WHISR` (un kthread o un timer). Cuidado con leer la
  longitud de `WRPLR` y no pasarse. El "enhance mode" simplifica (una lectura trae todo) pero añade el
  struct `ENHANCE_MODE_DATA_STRUCT_T`; empezar **sin** enhance (RX_ENHANCE_MODE_EN=0), un paquete por vez.
- **Casar SeqNum** del evento con el comando (para comandos query síncronos).
- **Inyectar beacons a cfg80211**: el RSSI viene en `HIF_RX_HEADER_T.ucRcpi`, el canal en
  `ucHwChannelNum`. Hay que construir el `cfg80211_inform_bss_frame` con `mgmt` empezando en el cuerpo
  de la trama 802.11 (saltar la cabecera HIF).

**Entregable:** `iw wlan0 scan` devuelve APs reales. (Esto ya es demostrable y motivador.)

**Estimado: 4–6 sesiones.** (RX dispatcher + NIC_CAPABILITY: 2; wiphy/netdev: 1; scan + inform_bss: 2-3.)

---

## Fase 2 — connect / auth / claves (asociarse a un AP WPA2)
**Objetivo:** `wpa_supplicant` se asocia a un AP WPA2-PSK y completa el 4-way handshake (que lo hace el
firmware), quedando "associated".

**Sub-pasos:**
1. `.connect`: mapear cripto de `cfg80211_connect_params` → auth mode / enc status; mandar
   `CMD_ID_INFRASTRUCTURE` (op-mode STA) + `CMD_ID_SET_BSS_INFO` (SSID/BSSID/auth/enc) + activar STA
   record (`CMD_ID_BSS_ACTIVATE_CTRL`/`CMD_ID_UPDATE_STA_RECORD`).
2. `.add_key`/`.del_key`/`.set_default_key`: `CMD_ID_ADD_REMOVE_KEY` con `CMD_802_11_KEY` (CCMP/TKIP/WEP).
   Ojo al swap de los bytes 16-23 de la clave de 32B que hace el downstream (`gl_cfg80211.c:add_key`).
3. Manejar `EVENT_ID_CONNECTION_STATUS`/`EVENT_ID_ASSOC_INFO` → `cfg80211_connect_result()` /
   `cfg80211_roamed()` / `cfg80211_disconnected()`.
4. `.disconnect` → `wlanoidSetDisassociate` equiv.

**Ficheros del downstream a estudiar (la coreografía importa):**
- `mgmt/ais_fsm.c` (`aisFsmRunEventJoinRequest`, `aisFsmStateInit_JOIN`: el ORDEN exacto de comandos al
  conectar — qué va antes de qué). **Documento de oro para esta fase.**
- `mgmt/assoc.c`, `mgmt/sec_fsm.c`, `mgmt/rsn.c` (cómo deriva auth/enc; qué espera el FW).
- `common/wlan_oid.c` (`wlanoidSetSsid`, `wlanoidSetBssid`, `wlanoidSetAddKey`,
  `wlanoidSetInfrastructureMode`, `wlanoidSetEncryptionStatus`).
- `os/linux/gl_cfg80211.c:934-1328` (`mtk_cfg80211_connect` completo) y `:185-265` (`add_key`).
- `include/nic_cmd_event.h` (`CMD_SET_BSS_INFO`, `CMD_802_11_KEY`, eventos de conexión).

**Lo difícil:**
- **La coreografía de comandos.** Si falta un STA-record o un `CH_PRIVILEGE`, el FW ignora el join sin
  decir por qué. Iterar leyendo `ais_fsm.c` y comparando con dmesg de eventos.
- **El 4-way handshake**: lo hace el firmware, pero hay que entregarle bien las claves PTK/GTK vía
  `ADD_KEY` en el momento correcto (tras el handshake EAPOL que `wpa_supplicant` corre por `wlan0`). El
  orden EAPOL↔ADD_KEY es el clásico punto de fallo.
- **Control de flujo TX** mínimo: los EAPOL son datos; necesitan que el data-path TX (Fase 3) funcione al
  menos para tramas sueltas. → solapar el principio de Fase 3 con Fase 2.

**Entregable:** `wpa_cli status` = `wpa_state=COMPLETED`, `iw wlan0 link` muestra el AP.

**Estimado: 5–8 sesiones.** (Es la fase de mayor incertidumbre por la coreografía y el handshake.)

---

## Fase 3 — data path netdev (navegar)
**Objetivo:** `wlan0` pasa tráfico IP real: DHCP, ping, throughput decente. Aquí entra el **PDMA** y
(opcionalmente) la **IRQ** para rendimiento.

**Sub-pasos:**
1. `ndo_start_xmit`: skb → prepend `HIF_TX_HEADER_T` (PKT_TYPE=DATA, StaRecIdx del AP, csum offload
   flags) → `kalDevPortWrite(WTDR0)` **con PDMA**. Respetar tokens TX (`WTSR0`/`EVENT_ID_TX_DONE`).
2. RX data: en el dispatcher, PKT_TYPE=DATA → quitar cabecera HIF → `eth_type_trans` → `netif_rx`.
3. **PDMA real** para TX y RX (no PIO): `HifPdmaConfig/Start`, poll `INT_FLAG`, `dma_map_single`.
4. **IRQ** `WF_HIF_IRQ_ID` (level-low) en vez de polling: ISR lee `WHISR`, agenda un tasklet/NAPI que
   drena RX. Configurar `WHIER_DEFAULT` y `WHCR` (int-clear).
5. (Opcional) **enhance mode** RX para agrupar varias tramas por transacción → throughput.
6. Control de flujo TX por TC (token bucket) — portar lo mínimo de `nic/nic_tx.c` (`nicTxAcquireResource`).

**Ficheros del downstream a estudiar:**
- `os/linux/hif/ahb/ahb_pdma.c` (config/start/poll/ack del PDMA — el data-path físico).
- `os/linux/hif/ahb/ahb.c:1197-1470` (read DMA), `:1487+` (write DMA), `HifAhbISR`, `HifAhbDmaEnhanceModeConf`.
- `nic/nic_tx.c` (recursos TX por TC, `nicTxAdjustTcQuotas`), `nic/nic_rx.c` (`nicRxProcessDataPacket`,
  reordering BA si hace falta — probablemente diferible).
- `include/nic/mtreg.h` (WHISR/WHIER/WHCR/WRPLR/WTSR para IRQ y flujo).

**Lo difícil:**
- **PDMA + coherencia de cache** (el A7 del MT6582 no es coherente para DMA; ya lo sabemos por el i2c).
  `dma_map_single` con la dirección correcta, `dma_wmb`, y el `Dst/Src` = dirección física del puerto.
- **IRQ del HIFSYS**: encontrar el número de IRQ correcto para mainline (el downstream usa
  `WF_HIF_IRQ_ID`; en DT será un `interrupts = <...>` del nodo `mediatek,WIFI`). Mientras tanto, polling
  funciona (lento pero válido).
- **Throughput/estabilidad** bajo carga: control de flujo TX, no desbordar `WASR` (overflow de colas).

**Entregable:** `udhcpc -i wlan0` coge IP, `ping` y `wget` funcionan. **WiFi usable en Phosh.**

**Estimado: 5–8 sesiones.** (TX/RX básico por PIO: 2; PDMA: 2; IRQ+flujo+enhance: 2-4.)

---

## Resumen de esfuerzo y dependencias

| Fase | Objetivo | Estimado | Bloqueante principal |
|---|---|---|---|
| 0 | func_on(WIFI) + FW download + WLAN_READY | 3–5 ses. | concurrencia BTIF/WMT (Riesgo A); ¿HIF accesible? (B) |
| 1 | cmd/event + NIC_CAPABILITY + scan pasivo | 4–6 ses. | RX dispatcher; inyectar beacons a cfg80211 |
| 2 | connect/auth/keys (WPA2) | 5–8 ses. | coreografía de comandos (ais_fsm); EAPOL↔ADD_KEY |
| 3 | data path netdev + PDMA + IRQ | 5–8 ses. | coherencia DMA; IRQ del HIFSYS; flujo TX |

**Total: ~17–27 sesiones** para "WiFi navegando". El primer hito *demostrable* (scan) cae al final de
Fase 1 (~7–11 sesiones). El proyecto **no tiene muro conocido**: el data-path es DMA+registros hardware
(no nuestra capa STP frágil), el firmware ya existe y ya lo descargamos parcialmente en concepto, y el
gemelo MT6572 comparte el mismo HIF (mismas bases, mismo `ahb.c`).

### Orden recomendado de ataque dentro de Fase 0 (para reducir riesgo pronto)
1. Primero la **ampliación del btif** (`mt6582_consys_func_on` thread-safe) y verificar por debugfs que
   `func_on(WIFI)` devuelve el evento OK. Esto valida el Riesgo A **sin tocar el HIF**.
2. Luego `ioremap(0x180F0000)` + driver-own + leer `WCIR`. Si lee `0x6582`, el Riesgo B está resuelto.
3. Solo entonces, la descarga de firmware.

> Si en el paso 2 `WCIR` no responde tras `func_on(WIFI)`, **parar y mapear el HIFSYS** (clocks/power
> extra) antes de seguir — es el único punto donde el proyecto podría necesitar RE adicional de registros
> no cubiertos por el WMT.
