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

## ✅ STATUS 0714 — los 2 huecos CERRADOS + bundle listo (sesión Mac)
Extraídos por adb del LineageOS (adbd ya root), staged en `.123` (`stock-gps-0713/`, proprietarios NO al repo):
- **3 libs nvram** ✓ ELF ARM válidas: `libcustom_nvram.so`(51K) `libnvram_platform.so`(5.3K) `libnvram_sec.so`(9.3K).
- **Calibración GPS** ✓ REAL (no ceros): `APCFG/APRDEB/GPS` (54B) contiene `"/dev/stpgps"` + `gps_tcxo_hz=`
  **`0x018CBA80 = 26 000 000 = 26 MHz`** (TCXO real) + `APRDCL/FILE_VER`.
- **Bundle ensamblado**: `~/gps-bionic-prefix.tar.gz` (1.8M) = el prefijo `/system` (linker+11 libs+mnld+gps.conf)
  + `/data/nvram/APCFG/{APRDEB/GPS,APRDCL/FILE_VER}`, listo para extraer en `/` de pmOS.

### Despliegue + run (cuando pmOS esté arrancado; el móvil está ahora en Lineage)
```bash
# desde la Pi, con pmOS vivo en root@172.16.42.1:
cat ~/gps-bionic-prefix.tar.gz | ssh root@172.16.42.1 'tar xzf - -C /'   # crea /system + /data/nvram
# arrancar el radio GPS (WMT func_on[GPS]) — abrir /dev/stpgps; confirmar que existe
ssh root@172.16.42.1 'ls -l /dev/stpgps'
# correr mnld (proceso bionic) con strace:
ssh root@172.16.42.1 'strace -f -s 512 -e openat,read,write,ioctl /system/xbin/mnld 2>/tmp/mnld.strace & sleep 25;
  echo "--- nvram/TCXO leidos? ---"; grep -aE "APRDEB/GPS|ENOENT.*nvram" /tmp/mnld.strace | tail;
  echo "--- START 0xAAF0 al DSP? ---"; grep -aE "stpgps|\\\\xaa\\\\xf0" /tmp/mnld.strace | tail;
  echo "--- NMEA emitido? ---"; grep -aE "GPGGA|GPRMC|GPGSV" /tmp/mnld.strace | tail'
```
Escalón de éxito: (a) lee `/data/nvram/APCFG/APRDEB/GPS` sin ENOENT → (b) abre `/dev/stpgps` + escribe START
0xAAF0 → (c) DSP responde → (d) mnld emite `$GPGGA/$GPRMC`. Luego NMEA→pty→gpsd→geoclue→Phosh, y **fix exterior**.
Si peta pidiendo `/dev/nvram` (chardev MTK ausente en mainline) = el riesgo abierto → shim/portar ese chardev.

## 🎉 STATUS 0714 tarde — **mnld CORRE en pmOS (bionic) y queda a 1 comando del GPS**
Probado en HW (móvil #293, pmOS): el runtime **bionic funciona en el kernel mainline** (el linker + 12 libs
cargan y ejecutan). mnld arranca **del todo**, con la **radio GPS ON** (`func_on[GPS]: RADIO ENCENDIDO` en
dmesg + `/dev/stpgps` registrado), crea sus sockets HAL (`/data/gps_mnl/hal2mnld`) y **se queda IDLE
esperando el comando de inicio de sesión del HAL** — NO abre `/dev/stpgps` ni manda el START hasta recibirlo.

### Gates que hubo que superar (todos resueltos, en orden real):
1. `libc++.so` faltaba en el bundle → **añadida** (+ `libhardware.so`). *(Actualizar el bundle: son 13 libs, no 11.)*
2. `__xlog_buf_printf` (xlog de MTK, la liblog de LineageOS no lo exporta) → **shim no-op** (`gps-bionic-shim.c`).
3. `bind()` del socket a `/data/gps_mnl/hal2mnld` fallaba ENOENT → **crear `mkdir -p /data/gps_mnl`**.
4. `/sys/class/gpsdrv/gps/status` (interfaz sysfs del driver GPS **downstream** de MTK, ausente en mainline)
   → **interpositor `open`/`openat`** en `gps-bionic-shim.c` redirige `/sys/class/gpsdrv*` a
   `/data/gps_mnl/fakestatus` (`printf 1 > /data/gps_mnl/fakestatus`). *(El riesgo `/dev/nvram` NO se materializó:
   la calibración se leyó bien del fichero `/data/nvram/APCFG/APRDEB/GPS`.)*

### Receta de arranque FINAL (verificada, mnld corre):
```bash
# prefijo /system desplegado (bundle) + shim compilado en /system/lib/libxlogshim.so
mkdir -p /data/gps_mnl && printf 1 > /data/gps_mnl/fakestatus
chmod +x /system/bin/linker /system/xbin/mnld
env LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld   # arranca, radio ON, idle en hal2mnld
```

### ⏭️ ÚLTIMO ESLABÓN (siguiente sesión): el comando de inicio de sesión del HAL
mnld escucha en el socket UNIX-dgram **`/data/gps_mnl/hal2mnld`** los comandos que la HAL de Android
(`gps.default.so`) le manda. Falta enviarle el **"start navigation"** → dispara: `open /dev/stpgps` →
**START 0xAAF0** al DSP → medidas crudas → libmnl calcula → **NMEA**. RE-able del `gps.default.so` extraído
(en `stock-gps-0713/lib/`, tiene la codificación de los mensajes a `hal2mnld`) o del protocolo de mnld.
Con ese comando + cielo despejado = **fix**. El shim (`gps-bionic-shim.c`) es nuestro código, versionado aquí.

## 🔬 STATUS 0714 (noche) — RE del comando HAL HECHO + gate real localizado
**El comando del HAL RE-verificado y PROBADO en HW**: el protocolo `hal2mnld` = **bytes sueltos**
(`enum` en `mnl_common_6620.h`): `INIT=0x00 · STOP=0x02 · START=0x03 · RESTART=0x04 · RESTART_HOT=0x05…`.
Enviado `0x00`+`0x03` por un socket UNIX-dgram a `/data/gps_mnl/hal2mnld` (con un listener nuestro en
`/data/gps_mnl/mnld2hal` haciendo de HAL) → **mnld LOS RECIBE** (`read(3,"\0")`+`read(3,"\3")` en el strace).

**Pero mnld NO abre aún `/dev/stpgps`** — descubierto el gate REAL: en `START`, mnld (vía el `libmnl`
CERRADO) **controla el power del DSP por `/sys/class/gpsdrv/gps/{pwrctl,state,status,suspend,pwrsave}`**
(escribe "2" y **espera a que el status diga "listo"** antes de abrir el DSP). Confirmado en el binario:
`dsp_dev(/dev/stpgps)` + esas 5 rutas sysfs. Nuestro interpositor manda TODAS a un solo `fakestatus`
→ el status no transiciona como mnld espera → mnld se queda **puliendo el status con un POSIX-timer**, sin
abrir el DSP. La lógica de "listo" está DENTRO de `libmnl` (no en la glue abierta) → no RE-able por fuente.

### ⏭️ Camino (bien definido) para cerrar el GPS
**El DSP YA está encendido en nuestro HW** (`func_on[GPS]` por el open de `/dev/stpgps` al boot) y
`dsp_dev=/dev/stpgps`. Solo falta que mnld **crea que el DSP está listo** por la interfaz gpsdrv:
1. **[Robusto] Portar el driver `gps.c` (`mt3326_gps`) del downstream** (~1107 LOC, wrapper FINO del STP):
   crea el `/sys/class/gpsdrv/gps/` real + `/dev/gps`; al escribir pwrctl "arranca" (para nosotros = el
   canal STP GPS, ya ON) y reporta `state=listo`. Es el fix propio y limpio (estilo del proyecto).
2. **[Hack rápido] Interpositor más listo**: redirigir cada `/sys/class/gpsdrv/gps/X` a un fichero
   SEPARADO (`/data/gps_mnl/gpsdrv_X`) y **pre-sembrar `state`/`status` con el valor "listo"** que espera
   `libmnl` (desconocido → hallarlo trazando los `read` de mnld tras el `write "2"`, o probando valores).
   Si acierta el valor → mnld abre `/dev/stpgps` → **START 0xAAF0** → NMEA. Barato pero de valor incierto.

Recomendación: **(1)** para cerrar de verdad; **(2)** para un PoC rápido de que el resto de la cadena
(open `/dev/stpgps` → burst → libmnl → NMEA) funciona antes de invertir en el driver.

## 🧩 STATUS 0714 (tarde-2) — `gps.c` PORTADO (funciona) + arquitectura COMPLETA + `/dev/stpgps` ABIERTO
**Portado el driver**: `mt6582-gpsdrv.c` (~300L, fiel al `gps.c`/mt3326 del downstream, mainline-limpio).
Compilado como módulo (`.ko`) y **cargado en la pmOS viva (#293)**: crea el `/sys/class/gpsdrv/gps/`
{pwrctl,state,status,suspend,pwrsave,rdelay} REAL + `/dev/gps`. Hallazgo del port: `mt3326_gps_power` es
un **stub** (el power del combo va por WMT/STP, ya ON) y `/dev/gps` es un **buffer loopback** (no va al STP;
el dato del DSP va por `/dev/stpgps`) → port autocontenido, sin STP ni power real.

**Con el gpsdrv real, mnld escribe pwrctl/state bien** (INIT: pwrctl=0+state=1; START: pwrctl=2=RST) — pero
seguía sin abrir el DSP. **Arquitectura COMPLETA RE-ada** (`MNLD__src__mnld_6620.c:792`): mnld hace
**`fork()` + `execv` de un binario SEPARADO** `/system/xbin/libmnlp_mt<chip>` (o `/system/xbin/libmnla` por
defecto — sin el property `persist.radio.mediatek.chipid` sale el default). **ESE `libmnlp` (=el runner de
Fase A `mnl_process_6620.c`+`libmnl_6628.a`) es quien abre `/dev/stpgps` y corre libmnl (el motor PVT).**
Solo habíamos desplegado `mnld`.

**Desplegado el runner** (`~/gps/fase-a/mnlp_static`, ELF ARM **estático** → corre sin bionic) en
`/system/xbin/libmnla` (+ `libmnlp_mt6628/6620`) → **mnld lo fork+exec-ó y ABRIÓ `/dev/stpgps`** (visto:
un proceso con `/dev/stpgps` en sus fds). ¡La cadena entera funciona hasta el DSP! Pero: **NMEA=0** y el
runner es **flaky** (una corrida abrió el DSP, otra no) → el runner de Fase A (build propio, riesgo ABI
soft/hard-float) necesita depuración, y/o falta cielo despejado para datos.

### ⏭️ Último tramo (para cerrar el fix)
1. **Estabilizar el runner libmnlp**: usar el **stock `libmnlp` bionic** si se extrae (mejor que el build
   estático de Fase A, evita el riesgo ABI), o depurar el `mnlp_static` (que arranca y abre el DSP pero no
   emite). Verificar que recibe bien los fds del socketpair (`argv[2],argv[3]`) de mnld.
2. Confirmar el **START 0xAAF0** al `/dev/stpgps` (strace del runner) + respuesta del DSP.
3. **Cielo despejado** para el primer fix (interior = 0 satélites, sin NMEA de posición).
4. Encadenar NMEA (mnld2hal / pty) → gpsd → geoclue → Phosh.

**Piezas listas y versionadas**: `mt6582-gpsdrv.c` (driver, compila+carga+crea interfaz ✓),
`gps-bionic-shim.c` (xlog no-op ✓), bundle bionic + calibración en `.123`. Deploy del runner:
`mnlp_static → /system/xbin/libmnla`.

*Receta 2026-07-14 (sesión Mac). gps.c PORTADO y funcionando; arquitectura completa mapeada; `/dev/stpgps`
abierto por el runner. Falta estabilizar el libmnlp runner + cielo despejado para el fix.*
