# GPS Fase B — receta del runtime bionic (Vía 1: mnld stock) — 0714

> Compañera de `FASE-B-PLAN-0714.md`. **Aterrizada sobre los binarios reales** (`readelf` + straces del
> `mnld` stock en `~/mainline/downstream/stock-gps-0713/` y `~/gps/mnld-{init,live}.strace` de la Pi `.123`).
> Correr el `mnld` stock **como proceso 100% bionic** (interpreter `/system/bin/linker`): habla con nuestro
> kernel solo por syscalls + `/dev` → **sin libhybris**.

## Deps reales (readelf)
- `mnld` (ARM EABI5, interp `/system/bin/linker`) NEEDED: `libc libcutils libm libmnl libnvram libstdc++`.
- `libmnl.so` NEEDED: `libc libstdc++ libm` (todas bionic — muy contenido).
- En runtime `mnld` hace `dlopen` de la cadena NVRAM: **`libnvram` + `libcustom_nvram` + `libnvram_platform` + `libnvram_sec`**.

## Prefijo bionic (montar en pmOS `/system/`, que es el root que el linker hardcodea)
```
/system/bin/linker            <- stock-gps-0713/linker
/system/lib/                  <- TODAS las .so bionic:
    libc.so libm.so libstdc++.so libcutils.so libdl.so liblog.so libmnl.so
    libnvram.so  libcustom_nvram.so  libnvram_platform.so  libnvram_sec.so
/system/xbin/mnld             <- stock-gps-0713/mnld
/system/etc/gps.conf          <- stock-gps-0713/gps.conf
/system/usr/share/zoneinfo/tzdata   <- (mnld lo abre; opcional, tolera fallo)
```
(pmOS corre desde la SD → `/system` está libre; no colisiona con el Android de la eMMC.)

## ⚠️ Los DOS huecos concretos a rellenar (bloquean el fix)
1. **Faltan 3 libs NVRAM** en la extracción H0: `libcustom_nvram.so`, `libnvram_platform.so`,
   `libnvram_sec.so` (solo está `libnvram.so`). Sin ellas la lectura de calibración falla.
   → **Extraer de LineageOS** (el móvil arranca Lineage ahora): `adb root && adb pull /system/lib/libcustom_nvram.so /system/lib/libnvram_platform.so /system/lib/libnvram_sec.so`.
2. **La calibración GPS de la NVRAM** (los TCXO — SIN ellos el DSP NO adquiere). El strace lo confirma:
   `mnld` lee el **fichero** `/data/nvram/APCFG/APRDEB/GPS` (registro con `NVRAM_VER_INFO`/`GPS`/TCXO;
   campos `gps_tcxo_hz/ppb/type` de `CFG_GPS_File.h`).
   → **Extraer de LineageOS**: `adb pull /data/nvram/APCFG/APRDEB/GPS` (+ `/data/nvram/APCFG/APRDCL/FILE_VER`)
   y colocarlo en pmOS en la MISMA ruta `/data/nvram/APCFG/APRDEB/GPS`. (Reusar el "set NV" que el H0 del
   módem ya extrajo si incluye el árbol APCFG.)

## Dev nodes que toca mnld (de los straces)
- `/dev/nvram` (char device NVRAM de MTK). **Riesgo**: no existe en mainline. Probar si con los FICHEROS
  `/data/nvram/APCFG/...` presentes `libnvram` lee sin el chardev (probable: los reads reales salen del
  fichero, fd=12). Si lo EXIGE, hay que portar el `nvram` chardev o hacer un shim.
- `/dev/pmsg0`, `/dev/__properties__` (log/props de Android): mnld **tolera fallo** (`= -1` en el strace) → ignorar.
- `/dev/stpgps` (nuestro, ya funciona por WMT) + `/dev/gps`: se abren en la fase LIVE (no en init). Verificar
  en la sesión; `/dev/gps` puede necesitar crearse (mknod/pty) si mnld no lo crea.

## Arrancar + instrumentar
```bash
# en pmOS, con el prefijo /system montado y los 2 huecos rellenados:
mkdir -p /data/nvram/APCFG/APRDEB /data/nvram/APCFG/APRDCL   # + los ficheros GPS/FILE_VER
strace -f -s 512 -e trace=openat,read,write,ioctl /system/xbin/mnld 2>/tmp/mnld.strace &
# ver el arranque del DSP:
grep -aE '/dev/stpgps|write\(.*\\xaa\\xf0' /tmp/mnld.strace   # ¿sale el START burst 0xAAF0 en NUESTRO HW?
```
Éxito parcial escalonado: (a) mnld lee la calibración sin ENOENT → (b) abre `/dev/stpgps` y **escribe el
START 0xAAF0** → (c) el DSP **responde** (reads con datos en /dev/stpgps) → (d) mnld emite **NMEA**
(`$GPGGA/$GPGSA/$GPRMC`) por su socket/pty.

## Consumo del NMEA (cadena ya existente)
`mnld` emite NMEA por socket (`mtk_gps_sys_nmea_output_to_app`). Puentear a un **pty** → `gpsd -N -n <pty>`
→ geoclue → Phosh. Retirar el viejo `mtkgps_aaf0.c`.

## Orden de ataque (mínimo esfuerzo → fix)
0. Restaurar pmOS (GANADOR del `.38`) — el móvil está en Lineage.
1. Extraer por adb (Lineage) las **3 libs nvram** + el **fichero GPS de nvram** (los 2 huecos).
2. Montar el prefijo `/system` + `/data/nvram/APCFG/...` en pmOS.
3. Correr mnld con strace → escalón (a)→(d). Si peta en `/dev/nvram`, portar/shimear el chardev.
4. NMEA → gpsd → geoclue → Phosh. **Fix real en EXTERIOR.**

*Receta 2026-07-14 (sesión Mac), sobre los binarios reales de `.123`. Los 2 huecos (3 libs nvram + fichero
GPS de nvram) son extraíbles por adb del Lineage que ya arranca. Riesgo abierto: `/dev/nvram` chardev MTK.*
