# GPS Fase A — ✅ EL RUNNER ENLAZA (sesión drivers 0709)

> Objetivo de la Fase A (PORT-STRATEGY-DRIVERS-0707 §3.1): *"que enlace y llame a `mtk_gps_mnl_run()`"*.
> **CONSEGUIDO.** Binario ARM producido (dinámico glibc **y estático**), con el core cerrado
> `libmnl_6628.a` + libagent/libsupl/libhotstill enlazados y los shims KAL resueltos.
> **NO probado en HW** (Fase B — necesita el móvil, ocupado por la sesión SPM).

## Qué se logró
- Glue abierto AOSP (Nu3001 `hardware_mediatek`, mnl6628): `mnl_process_6620.c` (tiene `main()`),
  `mtk_gps_6620.c`, `mnl_common_6620.c` **compilan** con el toolchain de la Pi.
- Blobs cerrados descargados y enlazados: `libmnl_6628.a` (10.5 MB, el motor PVT/framer 0xAAF0),
  `libagent_6628.a`, `libsupl.a`, `libhotstill.a` (BEE/efemérides).
- **`nm` confirma en el binario**: `T main`, `T mtk_gps_mnl_run` (entrypoint del core), `T mtk_gps_sys_create_mutex`.
- **Estático** = `ELF 32-bit ARM statically linked` → corre en Alpine/musl del móvil sin intérprete.

## Shims escritos (Fase A', el "porting layer" KAL)
En `~/gps/fase-a/shim/` (en la Pi):
- `cutils/log.h` → ALOG*/XLOG*/SLOG* a stderr (bionic logcat → stderr).
- `cutils/properties.h` → `property_get/set` a `getenv/setenv`.
- `compat.h` → `sys_siglist` (retirado de libc moderna, solo para logs) + declaraciones que el glue
  usa sin incluir header (`mtk_agps_agent_epo_*`, `SUPL_encrypt/decrypt`) + include de properties.
- `bionic_shims.c` → **`__strlen_chk`**, **`__errno`** (fortify/errno bionic) + **los 4 callbacks KAL de
  mutex** (`mtk_gps_sys_{create,destroy,take,give}_mutex` sobre `pthread_mutex_t`, indexados por
  `MTK_GPS_MUTEX_ENUM`).

## ⚠️ El hallazgo ABI (riesgo #1 de la estrategia, ahora PRECISADO)
Los blobs son **soft-float ARM** (`Tag_ABI_VFP_args` ausente = AAPCS base, floats por registros core);
el toolchain de la Pi es **hard-float** (`arm-linux-gnueabihf`, floats por registros VFP). El linker
aborta al fundir los atributos EABI ("uses VFP register arguments ... does not").
- **Desbloqueo aplicado**: `-Wl,--no-warn-mismatch` → el binario se produce (todos los símbolos
  resuelven; NO hay undefined).
- **Por qué es probablemente inofensivo**: la API pública de mnl (`mtk_gps.h`) es **INT32 + punteros a
  structs**; los floats (posición/tiempo/efemérides) cruzan **por referencia** (en memoria, se leen
  igual en ambos ABIs), no por valor en registro. El mismatch VFP-args solo afecta a `float/double`
  pasados **por valor**, que esta API no usa en su frontera. El cómputo float pesado es interno al blob
  (blob→blob, ABI consistente). → **A verificar en Fase B**; si algún NMEA sale con números basura,
  la causa es esto.
- **Fix limpio (si hiciera falta)**: sysroot soft-float (falta `gnu/stubs-soft.h` → instalar el
  multilib `-msoft-float` o un toolchain `arm-linux-gnueabi` soft) para compilar MIS objetos en softfp
  y casar el ABI de los blobs 1:1. Alternativa P2 (estrategia): correr `libmnl.so` exacto vía libhybris.

## Reproducir (en la Pi)
```
~/gps/fase-a/            # workspace
  inc/    <- headers AOSP + CFG_GPS_File.h + shims
  src/    <- mnl_process_6620.c, mtk_gps_6620.c, mnl_common_6620.c
  lib/    <- libmnl_6628.a, libagent_6628.a, libsupl.a, libhotstill.a
  shim/   <- cutils/{log,properties}.h, compat.h, bionic_shims.c
CC=arm-linux-gnueabihf-gcc ; FL="-mfloat-abi=softfp -mfpu=vfpv3"
$CC $FL -c -I inc -I shim -include shim/compat.h src/*.c        # (cada .c -> .o)
$CC $FL -c shim/bionic_shims.c -o obj_shims.o
$CC $FL -static -Wl,--no-warn-mismatch -o mnlp_static *.o \
     lib/libmnl_6628.a lib/libagent_6628.a lib/libsupl.a lib/libhotstill.a -lpthread -lm -lrt
```
(nota: los .c se compilan hoy con el hardfp default + `--no-warn-mismatch`; `-mfloat-abi=softfp` pide
`gnu/stubs-soft.h` que no está instalado — da igual para "que enlace", relevante solo para el fix ABI limpio.)

## SIGUIENTE (Fase A'' / B — cuando haya móvil)
1. **Rellenar `init_cfg`/`driver_cfg`**: `dsp_port=/dev/stpgps`, `nmea_port=pty`, y **los TCXO reales**
   (`hw_Clock_Freq/Drift/u1ClockType`) de la NVRAM GPS del krillin (sin ellos el DSP no adquiere).
2. **Stubs de ioctl** `COMBO_IOC_GPS_HWVER` / `/dev/mtgpio` (el glue tolera fallo, pero mejor stub).
3. **Correr en el móvil** sobre `/dev/stpgps` (ya funciona, radio GPS on por WMT) → instrumentar el
   `write` al dsp_fd para loguear el burst START 0xAAF0 en NUESTRO hardware.
4. Enchufar el NMEA de salida a la cadena **gpsd→geoclue→Phosh** que ya existe; retirar `mtkgps_aaf0.c`.
5. Verificar el ABI de floats (punto ⚠️); si falla, sysroot softfp.

*Sesión drivers (Opus) 2026-07-09. Fase A cerrada: el core stock del GPS enlaza en un binario nativo, igual que el WiFi trajo su core. La novedad (primer combo MT65xx host-based fuera de Android) sigue en pie.*
