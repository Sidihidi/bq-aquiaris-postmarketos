# GPS Vía 1 (mnld) — ★ GATE RESUELTO: mnld ABRE /dev/stpgps y corre el motor en pmOS (0714 casa)

> Continuación de `RECETA-BIONIC-VIA1-0714.md` (sesión Mac). **RESUELTO el gate final** con Ghidra
> local (Windows) sobre el binario `mnld`. El stack GPS stock ahora corre en pmOS, ABRE el DSP
> (`/dev/stpgps`) y el motor genera eventos NMEA. Reproducible (2/2 runs, sin crash).

## ★ RESUMEN EJECUTIVO (lo conseguido)
`mnld` + `libmnl.so` stock, bajo el shim bionic (`libxlogshim.so`), en pmOS (Alpine/musl), ahora:
`main → chip_detector → epoll → START → mtk_gps_sys_init OK → linux_gps_init → **ABRE /dev/stpgps** →
mtk_gps_sys_nmea_output_to_app (motor produciendo NMEA)`. Verificado: `stpgps_pid` vivo, `SIGSEGV=0`,
`nmea_output_calls=10` por run. **La Vía 1 llega al DSP.**

## El gate final (RE con Ghidra, ghidra-mnld/)
Objdump fallaba (Thumb-2 stripped) → **Ghidra headless local** (`ghidra_12.1.2`, Java) sobre `mnld`
(72KB, md5 `300daa4c…`, idéntico al del móvil). Scripts `FindGate.java`/`GetRefs.java`, salidas
`*-decomp.txt`/`*-refs.txt`. Hallazgo (`FUN_000140fc` = `mtk_gps_sys_init`):
```c
do {                                        // bucle 20x, 500ms cada uno
    property_get("service.nvram_init", buf, 0);
    if (strcmp(buf, "Ready") == 0) break;   // ← rompe SÓLO si la prop == "Ready"
    usleep(500000);
} while (++i != 0x14);
if (i == 0x14) return -1;                    // agotado → ERROR → el motor NUNCA abre el DSP
// éxito → NVM_GetFileDesc(LID 0x1b) [NO-fatal] → return 0
```
La KEY del `property_get` = **`service.nvram_init`**, el valor esperado = **`"Ready"`** (strings en
`refs.txt` @0x1b854/@0x1b867). En Android lo pone el `nvram_daemon`; en pmOS no existe → el gate no pasa
nunca. `FUN_00014ef4` (el que abre `/dev/stpgps` + `read_NVRAM`) está gated tras ese éxito.

## El FIX (shim v3, `gps-bionic-shim-v2.c`, VALIDADO)
Inyectar en el área de properties de bionic: `service.nvram_init="Ready"` (+ los chipid del v2). Detalles
finos que costaron (todos resueltos):
1. **Lazy-init, no constructor**: el `__attribute__((constructor))` del build `-nostdlib` NO corría
   fiable en el linker bionic (y si corría, era ANTES de `environ` → `getenv`=NULL → envenenaba el flag).
   → la inyección va en la 1a llamada real a `__xlog_buf_printf`.
2. **CAS atómico** (`__sync_bool_compare_and_swap`): mnld es multihilo; 2 hilos llamando a la vez hacían
   doble `area_init`/`prop_add` → SIGSEGV. Sólo 1 hilo inyecta.
3. **`unlink("/dev/__properties__")` antes de `area_init`**: bionic mapea ese backing READ-ONLY para
   lectores; si una run previa lo dejó, el `area_init` de la nueva run lo mapea r/o → `prop_add` SIGSEGV
   en `área+4`. Borrarlo lo hace fresco/escribible → auto-reparable en cada arranque.
4. dlsym de `__system_property_{area_init,add,get}` (el v1 petaba al relocar por versión @@LIBC).
Compilar/uso: cabecera del `.c`. Markers `[shim] service.nvram_init="Ready"` confirman la inyección.

## ⏭️ Lo que falta: NMEA CON DATOS — frontier = READ-BACK del DSP (0714)
El motor dispara `mtk_gps_sys_nmea_output_to_app` (10x/run) pero **no salen sentencias `$G...` con datos**.
Diagnóstico DEFINITIVO del tráfico al DSP (strace read/write/ioctl, fd 11 = /dev/stpgps, hilo daemon):
- ✅ mnld **ENVÍA el protocolo AAF0 correcto**: `write(11, AAF0-burst, 116)` + frames t=0x05
  `write(11, "\xaa\xf0\x06\x00\x05\xfe\x04\x00\x0d\x01\xaa\x0f", 12)`. (El burst 116B coincide con el
  arranque stock.)
- ❌ **mnld RE-ENVÍA el burst en BUCLE y NUNCA hace `read(11)` del daemon** → el DSP no se lee → el
  handshake no completa → el motor emite nmea_output pero VACÍO (sin medidas).
- Los `ioctl(11)` termios (TCSETS B115200…) fallan **ENOTTY** (`pmtk.conn=serial` trata el STP como UART;
  también en Android = no-fatal).

### DIAGNÓSTICO COMPLETO (0714, Ghidra mnld + dmesg + strace + fuente del driver)
Cadena de init tras el gate (decompilada, `FUN_000184b0`): `mtk_gps_sys_init` OK → `FUN_00017048`
(=linux_gps_init: `open(/dev/stpgps)` + ioctls STP cmd 7/8 + `mtk_gps_sys_function_register` +
**`mtk_gps_mnl_run()` que DEBE devolver 0x12**) → `FUN_00018458` (sigaction SIGTERM) → `FUN_00017e64`.
El motor `mtk_gps_mnl_run` SÍ corre (dispara `mtk_gps_sys_nmea_output_to_app` 10×) y su reader
(tid separado) queda **BLOQUEADO en `read(/dev/stpgps)`** esperando datos.

**El DSP está ENCENDIDO** (dmesg boot: `func_on[GPS]: *** RADIO ENCENDIDO ***` + `CHIP LISTO + BT/GPS ON`).
**El driver del char device es correcto** (`drivers/soc/mediatek/mt6582-btif.c`, `gps_fops`):
`gps_write`→`stp_send(STP_TYPE_GPS)` (el burst sale), `gps_read`←`gps_fifo` (bloquea si vacío), y el hilo
RX demuxea bien: `if (type==STP_TYPE_GPS) { kfifo_in(gps_fifo); wake_up(gps_wq); }` (línea ~412).
`gps_fops` **NO tiene `.unlocked_ioctl`** → los ioctls STP de mnld (cmd 7/8, versión de chip) dan ENOTTY,
pero eso es NO-fatal (mnld sigue).

**⇒ El blocker exacto: mnld ESCRIBE el burst AAF0 al DSP (por STP) pero el DSP NO RESPONDE** → `gps_fifo`
nunca se llena → el reader se bloquea → NMEA vacío. El DSP está ON y el driver RX es correcto, así que el
DSP simplemente no ACKea el burst de mnld.

### ★★ DESCUBRIMIENTO (0714): el pipeline COMPLETO funcionó en STOCK (referencia `mnld-live.strace`)
Las capturas del Mac `~/gps/mnld-{init,live}.strace` (20 jun, entorno STOCK Android: paths
`/data/agps_supl/`, `/dev/nvram`, `/dev/__properties__`) muestran mnld **produciendo NMEA REAL con
satélites**: `read(13, "\xaa\xf0…", 512)=307` (el DSP RESPONDIENDO frames AAF0) → `write(14,
"$GPGGA,235958,3736.6702,N,00058.4496,W…$GPGSV,3,1,10,21,69,171,,05,69,288,…")` = **10 satélites en
vista** (posición de España, la del usuario). ⇒ **mnld + DSP + libmnl FUNCIONAN cuando el driver relaya
bien los frames AAF0.** El problema es una diferencia pmOS↔stock, NO del stack de userspace.

**Diagnóstico definitivo del origen**: el init stock lee de la MISMA NVRAM (`/data/nvram/APCFG/APRDEB/GPS`)
el MISMO `dev.dsp=/dev/stpgps` + la MISMA cal TCXO; es el MISMO binario `mnld`. La ÚNICA diferencia es
**el driver del kernel detrás de `/dev/stpgps`**: stock = connsys STP-GPS; pmOS = `mt6582-btif.c` (built-in).
El DSP respondía en stock; con btif calla. El pwrctl del gpsdrv (`mt6582-gpsdrv.c`, módulo) es NO-OP
(`mt6582_gps_power` = stub, echo-registers) → descartado. El power real (btif `func_on(GPS)`) está OK.

### ⏭️ Frontier restante: ¿por qué el DSP no responde vía `mt6582-btif.c`?
El driver btif SE VE correcto: `gps_write→stp_send(STP_TYPE_GPS)` (framing STP igual que BT/FM que SÍ
funcionan; `gps_tx[1024]` no trunca; `stp_send` OK), RX demux `STP_TYPE_GPS→kfifo_in(gps_fifo)+wake`
(línea 412). `func_on(GPS)` da OK. Pero el DSP no manda frames GPS de vuelta. Test DECISIVO pendiente:
**probe en el kernel** — añadir `dev_info` en `gps_write` (TX GPS) y en la rama `STP_TYPE_GPS` del hilo RX
(RX GPS) de `mt6582-btif.c`, recompilar (built-in → rebuild+reflash menupick) y correr mnld:
- Si aparece "GPS RX<-" en dmesg → el DSP SÍ responde → el bug está en mnld/reader (o el gps_fifo).
- Si sólo "GPS TX->" → el DSP calla vía btif → comparar el framing/canal STP-GPS con el stock connsys
  (fuente downstream perdida; reconstruir de la doc MTK o del binario stock). Hipótesis: el DSP GPS
  necesita un paso de init/patch por STP que el func_on stock hacía y btif no, o el canal STP difiere.

**Original (menos probable):**
El Mac vio el DSP hablar **35 frames AAF0 bidireccionales** con el runner **Fase A** → el HW+driver PUEDEN
entregar frames GPS. Algo difiere entre mnld y Fase A. Investigar (orden):
1. **Comparar el TX de mnld vs Fase A** (dominio del Mac — coordinar, NO en paralelo): ¿Fase A enviaba un
   init/secuencia distinta? ¿mnld re-envía el burst cada ~4 frames (visto en strace: burst→3×t=05→burst) y
   eso RESETEA el DSP antes de que responda? Probar: mandar el burst UNA vez (sin el re-envío de mnld) y
   leer /dev/stpgps con timeout — ¿responde? (capturar el burst 116B completo con `strace -s 300` de una
   run larga, luego replay directo).
2. **La cal TCXO en el burst**: el burst 116B lleva los TCXO de NVRAM (`/data/nvram/APCFG/APRDEB/GPS`);
   si están mal, el DSP podría ignorarlo. Verificar que la NVRAM GPS es la real del móvil.
3. **Sky view**: aun así el DSP debería ACKear el init (reportar 0 satélites) sin necesidad de cielo; que
   no ACKee = init/protocolo, no falta de satélites.
4. **Ruta de salida** (cuando haya frames): mnld → fd12=/dev/gps + fd13=socket(HAL) → gpsd/geoclue
   (`zzz-gps.start` ya existe).

## Estado en el móvil (Alpine/pmOS)
Shim v3 limpio en `/system/lib/libxlogshim.so` (v2 en `.v2`). Setup del Mac intacto: `gpsdrv` cargado,
calibración TCXO (`/data/nvram/APCFG/APRDEB/GPS`), `mnld`/`libmnlp_mt6582`/`libmnl.so` en /system,
config `/data/misc/mnl.prop` (`dev.dsp=/dev/stpgps pmtk.conn=serial`). Arranque validado:
`env GPS_SET_CHIPID=1 LD_LIBRARY_PATH=/system/lib LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld`
+ enviar `\x00\x03` al socket `hal2mnld`.

*Casa (Opus 4.8), 2026-07-14. Gate service.nvram_init RESUELTO por RE Ghidra; el DSP se abre; falta el
NMEA con datos (DSP measurements + cielo despejado).*
