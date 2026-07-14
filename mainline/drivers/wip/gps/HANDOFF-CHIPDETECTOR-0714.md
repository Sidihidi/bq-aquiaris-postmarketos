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

**El frontier ES el read-back del DSP.** Hipótesis a investigar (orden):
1. **¿Hay un hilo LECTOR de /dev/stpgps que no arranca?** (En stock, un thread aparte lee el STP y
   despacha frames.) Revisar en el strace COMPLETO si algún tid hace `read(11)` que bloquea
   (`<unfinished>`), o si el lector nunca se crea. Si no se crea → ver por qué (¿otra prop? ¿otro gate en
   `linux_gps_init`/`FUN_00017048`/`FUN_00018458`, decompilables con el mismo método Ghidra).
2. **Power/arranque del DSP vía gpsdrv**: la secuencia `pwrctl` 0→1→2 (OFF/ON/RST) del driver
   `mt6582-gpsdrv.c` — ¿enciende el DSP de verdad? El Mac vio el DSP hablar 35 frames AAF0 con OTRO runner
   (Fase A), así que el HW responde; con mnld hay que confirmar que el power se aplica igual.
3. **Sky view**: interior = 0 satélites; aun sin fix el motor debería emitir `$GPGGA` vacío + `$GPGSV`
   si recibe medidas. Sin read-back (punto 1/2) no llega ni a eso.
4. **Ruta de salida**: cuando haya NMEA, mnld escribe en fd12→/dev/gps y fd13→socket(HAL); enganchar a
   gpsd/geoclue (`zzz-gps.start` ya existe).

## Estado en el móvil (Alpine/pmOS)
Shim v3 limpio en `/system/lib/libxlogshim.so` (v2 en `.v2`). Setup del Mac intacto: `gpsdrv` cargado,
calibración TCXO (`/data/nvram/APCFG/APRDEB/GPS`), `mnld`/`libmnlp_mt6582`/`libmnl.so` en /system,
config `/data/misc/mnl.prop` (`dev.dsp=/dev/stpgps pmtk.conn=serial`). Arranque validado:
`env GPS_SET_CHIPID=1 LD_LIBRARY_PATH=/system/lib LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld`
+ enviar `\x00\x03` al socket `hal2mnld`.

*Casa (Opus 4.8), 2026-07-14. Gate service.nvram_init RESUELTO por RE Ghidra; el DSP se abre; falta el
NMEA con datos (DSP measurements + cielo despejado).*
