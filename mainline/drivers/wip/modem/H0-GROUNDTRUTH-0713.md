# Módem H0 — extracción + inventario de ground-truth del stock (LineageOS 13) — 0713

> Ejecutado con el móvil arrancado en **LineageOS 13 (Android 6.0.1)** stock, adb root
> (`ro.secure=0`), SELinux **Permissive**. Todo read-only salvo un binario devmem temporal que se
> borró al terminar. Deliverable H0 del [PLAN-M1](PLAN-M1-ARRANQUE-MD-0712.md): **firmware MOLY
> extraído + inventario CONNSYS + arranque del MD confirmado en stock.**

## Firmware del módem (MOLY) — EXTRAÍDO
- **Fichero** (no una partición cruda): `/system/etc/firmware/modem.img` → el ccci lo carga con
  `load_std_firmware` (copia RAW + `check_md_header`), que es EXACTAMENTE el patrón `request_firmware`
  del plan H3. No hay partición `modem`/`md1img` en `/proc/dumchar_info` (se carga del fichero).
- **Tamaño**: 5 172 580 B. **md5**: `0414422bffd155fab369ef50b159dfb8`.
- **Identidad del build**: `MOLY.WR8.W1449.MD.WG.MP.V1` (2015/02/11 17:50) · `CKT6582_W_L_HSPA`
  = MT6582 **WCDMA/HSPA + GSM** (WG), mass-production V1. Coincide con el `modem_1_wg_n.img` que
  asumía el plan.
- **Formato**: empieza por tabla de vectores ARM (`18f0 9fe5` = `ldr pc,[pc,#24]` ×8) — imagen
  cargable directa (el GFH/`check_md_header` lo parsea el loader; no hay magic textual en offset 0).
- **Ubicación en la Pi**: `~/mainline/downstream/stock-firmware-0713/modem.img` (NO se sube al repo:
  blob propietario de 5MB). Para H3: copiar a `/lib/firmware/` y `request_firmware("modem.img")`.

## Estado del RIL/módem en stock (confirma que el MD arranca)
- `gsm.version.baseband = MOLY.WR8.W1449.MD.WG.MP.V1` · `ril.current.share_modem=2` (dual-SIM).
- `init.svc.ril-daemon-mtk = running`, `mtk gemini ril 1.0`. CCCI vivo: `CCCIReadThread` forkeado,
  RPC `MD1_SIM{1,2}_HOT_PLUG_EINT` OK, `/sys/bus/platform/devices/ccci-helper.0` presente.
  ⇒ **el MD arranca y hace handshake en el stock** (objetivo final de M1).

## Inventario CONNSYS (`/system/etc/firmware/`) — para WiFi/BT/FM
| Fichero | Tamaño | Uso |
|---|---|---|
| `WIFI_RAM_CODE_SOC` | 207 648 | WiFi RAM code — **mismo tamaño que nuestro `WIFI_RAM_CODE`** (paridad) |
| `ROMv1_patch_1_0_hdr.bin` | 80 812 | patch WMT/BT (CONNSYS) — OJO: nombre distinto al `mt6572_82_patch` que horneamos por error |
| `ROMv1_patch_1_1_hdr.bin` | 21 488 | patch WMT/BT |
| `WMT_SOC.cfg` | 80 | coex: `coex_wmt_ant_mode=1`, `wmt_gps_lna_pin/enable=0`, `co_clock_flag=0` |
| `catcher_filter_1_wg_n.bin` | 1 472 | filtro catcher del módem |
| `mt6627/mt6627_fm_v1_{coeff,patch}.bin` | 1092/1028 | FM (v1 tiene datos; v2-v5 vacíos) |
Copiados a la Pi: `modem.img`, `WMT_SOC.cfg`, `catcher_filter_1_wg_n.bin` (los demás son inventario).

## No capturable (y por qué no bloquea)
- **Handshake CCCI en vivo** (`MD_INIT_START_BOOT`→`NORMAL_BOOT_ID`): ocurre a ~4-6s y ya scrolleó
  fuera del ring del dmesg; logcat -b radio no lo tenía. Documentado en el downstream (plan §1/§5).
- **Registros SPM del MD con módem ON** (objetivo H1): **`/dev/mem` no existe** en el kernel de
  LineageOS (sin CONFIG_DEVMEM) → no se pudo leer MMIO. El criterio de éxito de H1 (`SPM_PWR_STATUS`
  bit0=1) se verifica igualmente en NUESTRO kernel tras el power-on.

## Nota de cámara (Hito 0, colateral — RESUELTO)
Al boot el stock listaba `ov12830mipiraw` como default de sondeo, pero dio `No Sensor Found` (sensor sin
alimentar). **Abriendo la app de cámara → dmesg muestra `[OV8865Raw]` streaming activo (1964 hits)**:
la trasera es **OV8865** (8MP), **confirmando la inferencia** de `wip/camera/FEASIBILITY-CAMARA-0711.md`.
OV8865 tiene driver mainline (`drivers/media/i2c/ov8865.c`) → vía RAW viable. Cámara = última prioridad.

## NVRAM / NV del módem (extraído — material para M3, no H1)
Set de NV extraído a la Pi (`~/mainline/downstream/stock-nvram-0713/`, NO en repo = datos del dispositivo):
- `nvram.img` (5MB, raw, md5 `8cbdb4fe`) — partición nvram (mmcblk0 @0x400000).
- `protect_f.img` / `protect_s.img` (10MB c/u, **ext4**, md5 `95a7a10b` / `c5519ab8`) — NV protegido (mirror).
- `/data/nvram/md/NVRAM/NVD_IMEI/MP0B_001` (120B) = IMEI **CIFRADO** (MTK ofusca; patrón de 12B repetido,
  no BCD en claro → no cross-check con el IMEI plaintext, que está en memoria local privada, NO aquí).
Para M3 (registro en red) hará falta esta NV **intacta** (calibración RF + IMEI). Para H1/H2 no se usa.

## Siguiente (H1)
Con el firmware y la identidad confirmados, H1 = añadir el dominio MTCMOS del MD a `mt6582-spm.c`
(secuencia en el plan §1) **con config LIMPIO (sin EXTRA_FIRMWARE)** y el `ioremap` PEREZOSO
(nada del código del MD debe tocar el arranque). El intento previo quedó en la Pi como
`mt6582-spm.c.h1-attempt1`.

*H0 ejecutado 2026-07-13 desde LineageOS stock (adb root).*
