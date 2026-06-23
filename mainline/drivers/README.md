# Drivers del kernel mainline (MT6582 / krillin)

Conjunto consolidado de los drivers y *fixups* que el port mainline (Linux 7.0.12)
añade/parchea para el BQ Aquaris E4.5 (MT6582). Todos están **portados del
downstream 3.10** (sin scpsys/CCF/pwrap completos en mainline para este SoC, se
hace *poke* directo a los bloques). Esta carpeta es la **ubicación canónica** de
estos `.c`/`.h`; copias sueltas más antiguas viven en `../drivers-video/`,
`../drivers-musb/`, `../tools/` y `../disp-drm/code/` — usa las de aquí.

Para el detalle de cada uno, lee la cabecera del propio `.c` (todas documentan la
secuencia y el porqué) y la doc de hito correspondiente en `../`.

## Ficheros

| Fichero | Qué hace / para qué | Estado |
|---|---|---|
| **`mt6582-consys.c`** | Enciende el subsistema de conectividad (CONSYS): reguladores VCN + MTCMOS por SPM, `VCN33_ON_CTRL_WIFI` en el PMIC, y lee el chip-id (`0x6582`) para probar que el HW responde. Base de WiFi/BT/GPS. | ✅ |
| **`mt6582-btif.c`** | BTIF-DMA + bring-up STP/WMT del CONSYS + Bluetooth HCI (`hci0`). Descarga del patch, `func_on` de las radios, registra `hci0` para BlueZ/Phosh. Disparo por debugfs. Doc: `../wifi-consys/m3a/`. | ✅ |
| **`mt6582-wifi.c`** + **`mt6582-wifi-reg.h`** | WiFi full-MAC/cfg80211 por el HIF estilo-SDIO propio (`0x180F0000`), no por STP. Driver + registros MCR/PDMA/firmware. Doc: `../wifi-consys/wifi/`. | ⚠️ Fase 2 (scan ✅, connect en progreso: falta CH_PRIVILEGE + WPA2) |
| **`mt6582-dispfix.c`** | Reprograma el OVL del display a RGB565 (el LK lo deja en 32bpp → tinte amarillo) desde un `late_initcall`. Hack provisional mientras no hay DRM. Sustituido por el driver DRM real (`../disp-drm/`). | ✅ (legacy/simplefb) |
| **`pwm-mtk-disp.c`** | Driver PWM de display de MediaTek (mainline upstream) para el backlight vía DISP-PWM/BLS. | ✅ |
| **`mt6582-mfg-power.c`** | Enciende el power-domain del GPU (MFG) por SPM MTCMOS **antes** de que lima probe (`subsys_initcall`). Sin él lima/Mali-400 no arranca. | ✅ |
| **`mt6582-musb.c`** | Glue MUSB (gadget/peripheral) del MT6582: clocks por *poke*, secuencia de power-on del PHY USB2 inline. Objetivo: red/SSH por USB. PIO only. | ✅ |
| **`mt6582-pmic-fixup.c`** | Enciende la alimentación del táctil (MT6323 VGP1) por la interfaz pwrap WACS2 desde un `late_initcall`, mientras pwrap/mt6323 no están en el DT. | ✅ |

## Documentación relacionada (en `../`)

- CONSYS/BTIF/BT, WiFi, GPS → `../wifi-consys/{m3a,wifi,gps}/` (cada uno con su README + `.md`).
- Display DRM (sustituto de `dispfix`) → `../disp-drm/`.
- Hitos de táctil/I2C, display, etc. → ficheros `HITO-*.md` en `../`.
