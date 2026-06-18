# WiFi MT6582 (CONSYS) — ALCANCE, MAPA HW y ROADMAP (2026-06-18)

Siguiente driver del roadmap tras display+GPU. **Es el más duro**: no hay driver mainline
para este combo. Este doc fija el mapa de hardware (investigado del downstream 3.10) y el plan.

## 🏆 M1 LOGRADO (2026-06-18, kernel boot-wifi1.img) — EL CONSYS RESPONDE
```
mt6582-consys 18070000.consys: CONSYS VIVO: chip-id=0x6582 (PWR_CON=0xd)
probe of 18070000.consys returned 0 after 87495 usecs
```
El hardware WiFi (CONSYS) **enciende y es direccionable**: lee `chip-id=0x6582`. Flujo: primer
probe `-517` (EPROBE_DEFER, reguladores no listos) → re-probe a 3.5s → OK. `PWR_CON=0xd` =
PWR_ON|PWR_ON_S|PWR_RST_B, sin ISO/CLK_DIS/SRAM_PDN = estado correcto. **La secuencia MTCMOS
portada del MFG funcionó a la primera.** Display/lima/Phosh siguen OK (no rompió nada).
Equivale a "el power del GPU funciona": cimiento probado. Siguiente: **M2 (firmware)**.

## QUÉ ES (arquitectura)
El BQ E4.5 usa `CONFIG_MTK_COMBO_CHIP_CONSYS_6582` = la **conectividad integrada en el SoC**
(WiFi + BT + GPS + FM), NO un chip SDIO externo. Piezas downstream:
- `drivers/misc/mediatek/conn_soc/common/{core,mt6582}` = stack **WMT** (Wireless Mgmt Task) +
  glue de plataforma (`mtk_wcn_consys_hw.c`: power/clk/reset/chip-id).
- `conn_soc/drv_wlan/mt_wifi/wlan` = driver **802.11** (cfg80211/wext).
- **HIF = AHB** (`wlan/os/linux/hif/ahb/ahb.c`) → el MAC WiFi es **memory-mapped** en el bus
  interno (como el GPU/display), con base de registros + IRQ. (Bueno: es un `platform_device`.)
- Necesita **firmware** que el WMT descarga al CONSYS: `WIFI_RAM_CODE`, `WIFI_RAM_CODE_E6`,
  `mt66xx_patch_hdr.bin`, `WMT.cfg`, `WMT_SOC.cfg`. **El teléfono NO los tiene** en /lib/firmware
  → extraer del stock system.img (M2).

## MAPA DE HARDWARE (físico MT6582) — investigado
Downstream usa virtual 0xF0xxxxxx/0xF8xxxxxx; el físico:
| Bloque | Físico | Uso |
|---|---|---|
| SPM | `0x10006000` | power MTCMOS del CONSYS (**ya mapeado en mt6582-mfg-power.c**) |
| INFRACFG_AO | `0x10001000` | protección bus TOPAXI (0x220/0x228) |
| TOPCKGEN | `0x10000000` | clock-gate CONSYS (0x84 bit26) — opcional |
| TOPRGU (AP_RGU) | `0x10007000` | reset MCU CONSYS (0x18 bit12, key 0x88<<24) — para FW |
| CONN_MCU_CONFIG | `0x18070000` | **CHIP_ID @+0x08** (=0x6582), ACR @0x110, DELSEL @0x114 |
| CONN_TOP_CR | `0x180B0000` | AFE/PLL WiFi (@+0x2000) |
| CONN_HIF_CR | `0x180F0000` | HIF AHB del WiFi |

IRQ (convención SPI = nº tras GIC_PRIVATE_SIGNALS, igual que GPU/display):
`CONN_WDT`=**SPI 163**, `WF_HIF`=**SPI 164** (LEVEL_LOW, el del WiFi), `BTIF_WAKEUP`=SPI 165.

Reguladores PMIC (MT6323, ya en el driver mainline — faltan nodos DT):
`ldo_vcn18` 1.8V (digital), `ldo_vcn28` 2.8V (analog), `ldo_vcn33_wifi` 3.3V (RF), `ldo_vcn33_bt`.

## SECUENCIA DE ENCENDIDO (exacta, de `spm_mtcmos_ctrl_connsys(ON)` + `hw_reg_ctrl`)
**Es la plantilla MFG con otra dirección** (0x280 vs 0x214, ack mask 1<<1 vs 1<<4):
1. PMIC: encender VCN_1V8 + VCN28 (+ VCN33_WIFI para RF).
2. SPM unlock: `0x0b160001 -> SPM+0x000`.
3. `CONN_PWR_CON(SPM+0x280) |= PWR_ON(1<<2); |= PWR_ON_S(1<<3)`.
4. esperar `PWR_STATUS(0x60c)&(1<<1)` && `PWR_STATUS_S(0x610)&(1<<1)`.
5. `&= ~PWR_CLK_DIS(1<<4); &= ~PWR_ISO(1<<1); |= PWR_RST_B(1<<0); &= ~SRAM_PDN(1<<8)`.
6. `TOPAXI_PROT_EN(INFRA+0x220) &= ~0x104`; esperar `PROT_STA1(0x228)&0x104 == 0`.
7. ungate clock CONNMCU (`enable_clock(MT_CG_INFRA_CONNMCU)`; bit por confirmar en INFRACFG).
8. poll `CHIP_ID(0x18070008) == 0x6582` (10 reintentos, 20ms).
(Reset del MCU + AFE CR + DELSEL → en M3, los hace el FW patch.)

## ROADMAP (milestones)
- **M1 — Bring-up + chip-id [✅ LOGRADO 2026-06-18]**: `code/mt6582-consys.c` (platform_driver) +
  DT integrado. Enciende CONSYS (reguladores+MTCMOS) y lee `chip-id=0x6582`. HW probado.
- **M2 — Firmware [✅ LOGRADO 2026-06-18]**: extraídos del stock `system.img` (sparse →
  `simg2img` → mount ro → `/etc/firmware/`) y copiados al **`/lib/firmware/` del teléfono**:
  `WIFI_RAM_CODE_MT6582` (160KB, el de nuestro chip), `WIFI_RAM_CODE`/`_E6`/`_MT6628` (variantes),
  `mt6572_82_patch_e1_0_hdr.bin` + `e1_1` (patch WMT/conn MT6572/82 rev E1), `WMT_SOC.cfg`.
  **`WMT_SOC.cfg`**: `coex_wmt_ant_mode=1`, `co_clock_flag=0` (← param `co_clock_en` del power-on:
  0 = CONSYS usa su propio clock, VCN28 en modo HW), gps_lna off. build.prop confirma
  `mediatek.wlan.chip=CONSYS_MT6582`, `wifi.interface=wlan0`. Blobs propietarios MTK → NO se
  commitean al repo público; copia en Pi `~/wifi-fw/extracted/` + `system.img` guardado para
  re-extraer. (El formato del patch `_hdr.bin`: cabecera + payload, a parsear en M3.)
- **M3 — WMT + descarga FW**: portar el handshake WMT (BTIF/serial interno o EMI), reservar EMI,
  soltar reset del MCU, descargar firmware, esperar el "ready". El más incierto.
- **M4 — 802.11 / cfg80211**: el camino largo — o forward-port de `mt_wifi/wlan` (3.10→7.0.12,
  cfg80211 cambió mucho) o driver nuevo. Exponer `wlanX`.
- **M5 — Integración**: nl80211/wpa_supplicant, NetworkManager en Phosh, escaneo+asociación.

**Honestidad**: M1 es alcanzable ya y reutiliza trabajo probado. M3–M4 son un esfuerzo grande
(varias sesiones, éxito no garantizado por ser un combo propietario sin soporte mainline).

## CÓMO PROBAR M1
1. Copiar `code/mt6582-consys.c` → `drivers/soc/mediatek/` en la Pi; `obj-y += mt6582-consys.o`
   en su Makefile (junto a mt6582-mfg-power.o).
2. Aplicar `code/consys-dt-snippet.dts` al DT (reguladores VCN bajo el MT6323 + nodo consys).
3. Compilar zImage+dtbs, empaquetar boot-wifi1.img, flashear (ver DISP-DRM-CONTINUACION.md).
4. dmesg/`/dev/kmsg`: buscar `mt6582-consys ... CONSYS VIVO: chip-id=0x6582`.
   Si "no responde": revisar reguladores (¿VCN on?), o añadir el ungate del clock CONNMCU.

## Recuperación
WiFi no afecta al boot ni al display/GPU. Si el poke colgara (improbable), recuperar con
`boot-color1.img` (el kernel actual bueno, display+Phosh+lima OK).
