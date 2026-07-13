# Módem M1 — H0 COMPLETADO: extracción e inventario del firmware del MD (2026-07-13)

> Primer hito del `PLAN-M1-ARRANQUE-MD-0712.md`. **La imagen del módem está extraída, identificada
> y checksummeada en la Pi de build.** Sin tocar el móvil (la fuente fue el `/system` stock).

## Dónde está
- **Pi build (192.168.0.123): `~/modem-h0/`**
  - `modem_1_wg_n.img` — **5.100.832 bytes** — md5 `7fdeceadb7c15e3ac824f5ef92383f4f`
  - `catcher_filter_1_wg_n.bin` — 1.472 bytes — md5 `6751661877ec16be4aa085ab6e609af0`
- ⚠️ Los blobs NO van al repo (firmware propietario, repo público). Referencia = este doc + md5.

## Fuente y método
- El **eMMC del móvil ya no era fuente**: las particiones del `blkdevparts` (ebr1/protect/sec_ro/
  android/cache/usrdata) leen CEROS en sus arranques — la instalación Android del eMMC está
  vaciada/sin firmas. (El eMMC en bruto sí tiene datos dispersos, pero sin tabla legible.)
- La fuente buena: **`~/wifi-fw/system.img`** en la Pi (el `/system` stock completo, 838MB sparse,
  el mismo del que salió `WIFI_RAM_CODE`). Método: `simg2img` (magic `3a ff 26 ed` = Android sparse)
  → mount loop ro → `/etc/firmware/` → copiar → borrar el raw.
  (Para liberar disco se comprimieron `~/log_escritura.txt` y `~/android-cap/logs/logcat-live.log`
  → `.gz`, conservados.)

## Identificación (confirma las asunciones del plan)
- Nombre real: **`modem_1_wg_n.img`** — exactamente la variante prevista (`1_wg_n` = MD1,
  WCDMA+GSM, normal). **Un solo blob** (sin DSP aparte) ✅ como decía el plan.
- Strings internos: **"MAUI Project"**, build **`CKT82_WE_KK_HSPA`** (CKT82 = plataforma MT6582 del
  fabricante, KK = KitKat, HSPA = 3G). Firmware MOLY/MAUI legítimo del krillin.
- **El binario NO empieza con GFH**: arranca con la tabla de vectores ARM (`18 f0 9f e5` = `ldr pc,
  [pc,#0x18]` repetido) = entry directo. La metadata (check header que parsea `check_md_header`)
  va **al final**: los últimos ~32 bytes contienen estructura, p.ej. `0x004dd380` (= tamaño−416)
  y `0x01200000`. → Para H3: replicar el parse del tail del downstream, no asumir GFH en la cabeza.
- El resto de `/etc/firmware/` stock (inventario): `catcher_filter_1_wg_n.bin` (log filter, copiado),
  `S_ANDRO_SFL.ini`, `WIFI_RAM_CODE*` (ya conocidos), `mt6572_82_patch_e1_{0,1}_hdr.bin` (patches
  WMT/consys, no del MD), `mt6627/` (FM), `WMT_SOC.cfg`.

## Siguiente: H1 — dominio MTCMOS `SYS_MD1` en `mt6582-spm.c`
Según el plan (secuencia ya transcrita allí): PWR_ON→PWR_ON_S→wait `SPM_PWR_STATUS` bit0→
clk/iso/rst→SRAM→TOPAXI prot. Medible sin CCCI: `SPM_PWR_STATUS (0x1000660c)` bit0 = 1.
NOTA: coordinar con casa — `mt6582-spm.c` está caliente (soak M3 resuelto el 0713, commits aún
sin subir); hacer H1 SOBRE esa versión, no sobre la del repo.

*Sesión Mac (Fable 5), 2026-07-13.*
