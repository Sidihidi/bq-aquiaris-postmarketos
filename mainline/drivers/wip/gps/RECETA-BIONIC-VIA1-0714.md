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

## 🛰️ STATUS 0714 (noche-2) — **EL DSP GPS ESTÁ VIVO Y COMUNICÁNDOSE** (0xAAF0 bidireccional)
Corriendo el **runner de Fase A DIRECTO** (`/system/xbin/libmnla 1Hz=y`, salta a `single_process()` porque
mnld pasa argc=5 y el `main` espera argc==3/4): **abre `/dev/stpgps`, escribe frames `AA F0` al DSP** (incl.
`AA F0 09 00 05 FE ...` = el tipo-0x05 del START) **Y EL DSP RESPONDE** con frames `AA F0` (ej.
`read=AA F0 0E 00 31 FE ...`). **35 frames AA F0 en 40s.** → La parte más incierta (¿el DSP arranca y habla
en nuestro HW?) **RESUELTA: sí**. `dsp_dev=/dev/stpgps` ✓, `nmea_port=/dev/gps` ✓ (verificados en el binario).

**PERO: 0 NMEA.** Dos runners, dos problemas complementarios:
- **(A) Runner de Fase A** (`mnlp_static`, glue abierto + `libmnl_6628.a`, estático): **habla con el DSP** pero
  **NUNCA emite NMEA** (ni un `$GP`) → encaja con el **riesgo ABI soft/hard-float** de Fase A (las comms del
  DSP son byte/int → OK; el PVT/salida NMEA usa floats por valor → roto). Fix = recompilar con **sysroot
  soft-float** (`-mfloat-abi=soft`, instalar el multilib `gnu/stubs-soft.h`) para casar el ABI de los blobs.
- **(B) `libmnlp` STOCK bionic** (`libmnlp_mt6582`, usa la `libmnl.so` stock = **ABI correcto**): **linka**
  (tras stubear el símbolo AGPS `mtk_gps_sys_agps_disaptcher_callback` no-op en el shim) y **abre
  `/dev/stpgps`**, pero standalone **no lo maneja** — su `main` **necesita el pipe + comandos de mnld**
  (argc==3/4). Y **el `fork()`+`execv` de mnld al hijo es FRÁGIL** (a veces lanza el runner, a veces no).

### ⏭️ Para cerrar (dos vías, cualquiera vale)
1. **Arreglar el ABI del runner de Fase A** (recompilar soft-float) → emite NMEA standalone (más control). O
2. **Arreglar el fork de mnld** para que lance de forma fiable el **libmnlp STOCK** (ABI-correcto) con el pipe
   → el stock maneja el DSP + emite NMEA a `/dev/gps`, y mnld lo relaya a `mnld2hal`.
Luego: NMEA `/dev/gps` → gpsd → geoclue → Phosh, y **cielo despejado** para el fix (interior = 0 sat).

## 🧪 STATUS 0714 (noche-3) — **ABI DESCARTADO**; el bloqueo del NMEA está más adentro
Recompilado el runner con toolchain **soft-float** (`gcc-arm-linux-gnueabi`, instalado; `-mfloat-abi=soft`
por defecto) → `mnlp_soft`. **Enlaza LIMPIO, sin `--no-warn-mismatch`** (el ABI ahora casa con los blobs
soft-float; sin `VFP_args`). **Resultado en HW: IDÉNTICO al hard-float** — abre `/dev/stpgps` + **manda 36
frames AA F0** al DSP + **abre `/dev/gps`** (el nmea_port), pero **CERO NMEA** (ni un `$GP` write, `/dev/gps`
vacío). → **El ABI NO era la causa.** (Contradice el temor de Fase A, pero confirmado empíricamente.)

**El bloqueo del NMEA está DENTRO de libmnl/la cadena de salida**, no en el ABI. Hipótesis a investigar
(siguiente sesión):
1. **libmnl no emite NMEA de estado sin medidas** — el DSP responde AA F0 pero quizá son ACK/status, no
   pseudorangos (0 satélites en interior). El stock SÍ emitía `$GPGSA,A,1` en interior (H0) → diferencia con
   `libmnl_6628.a` (¿otra versión?) o falta un trigger/config. **Probar con CIELO DESPEJADO** (variable real).
2. **El callback de salida NMEA** — libmnl entrega el NMEA por un callback (`mtk_gps_sys_nmea_output_to_app`)
   que la glue (`mnl_process_6620.c`) debe cablear a `/dev/gps`/socket. Verificar que ese callback está
   implementado y no stubeado en los shims de Fase A. **← candidato nº1** (revisar el glue).
3. Puede que el motor necesite la **orquestación de mnld** (comando por el pipe) para pasar de "adquirir" a
   "emitir NMEA" — aunque single_process() corra standalone.

Binarios: `mnlp_static` (hard, `~/gps/fase-a/`), `mnlp_soft` (soft, ídem). Build soft:
`arm-linux-gnueabi-gcc -O2 -I inc -I shim -include shim/compat.h -c src/*.c` + `-c shim/bionic_shims.c` +
`-static -o mnlp_soft obj_*.o lib/*.a -lpthread -lm -lrt`.

## 🔑 STATUS 0714 (noche-4) — **CAUSA RAÍZ CERRADA: glibc ≠ bionic (pthread_mutex_t) + el motor bionic SÍ produce NMEA**

### 1) Por qué el runner de Fase A NUNCA emitió NMEA (mutex, no float)
Corriendo `mnlp_soft` con strace + captura, el runner **CRASHEA**:
```
Fatal glibc error: ../nptl/pthread_mutex_lock.c:94 (__pthread_mutex_cond_lock):
  assertion failed: mutex->__data.__owner == 0
```
El ABI que importaba **NO era float — es el layout de `pthread_mutex_t`**: `libmnl_6628.a` se compiló contra
headers **bionic** (mutex de 4 bytes) y al enlazarlo con **glibc** (Fase A estático, mutex de 24-40 B) el hilo
del motor corrompe el struct → aserción fatal de glibc → **el hilo muere → 0 NMEA**. Las comms del DSP (byte a
byte, sin mutex caliente) sí iban; la sincronización de hilos del PVT, no. **→ Recompilar Fase A es un CALLEJÓN
SIN SALIDA** (glibc es fundamentalmente incompatible con el `.a` bionic por el tamaño del mutex). Cerrado.

### 2) El `libmnlp` STOCK bionic CORRE en mainline y **PRODUCE NMEA a 1Hz**
Corrido `env LD_LIBRARY_PATH=/system/lib LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/libmnlp_mt6582 1Hz=y`
(directo, el kernel usa `PT_INTERP=/system/bin/linker`). Con el **shim de xlog ahora VISIBLE** (reenvía a
stderr; `GPS_XLOG_QUIET=1` lo silencia) se ve la narración entera:
`main → mnl_utl_load_property → single_process → mtk_gps_sys_init(×5) → linux_gps_init → **mtk_gps_sys_nmea_output_to_app ×40 (≈1Hz)**`.
**El motor bionic ESTÁ EMITIENDO NMEA** (llama al callback de salida periódicamente). Y por el camino:
lee `/data/nvram/APCFG/APRDEB/GPS` ✓, **abre `/dev/stpgps` + `read()`=`AA 58` del DSP** ✓, arranca **5 hilos** ✓.
ABI-nativo → **sin crash de mutex**.

**Gate resuelto**: standalone salía con `exit 255` en `openat("sc/EPO.DAT",O_RDWR)=ENOENT` (ruta RELATIVA al
cwd). Fix = `mkdir sc; dd if=/dev/zero of=sc/EPO.DAT bs=1 count=2304` (EPO vacío = cold start; sólo exige que
exista). Con eso pasa de EPO y navega.

### 3) El único hueco que queda: el **fd de salida del NMEA**
Standalone, `mtk_gps_sys_nmea_output_to_app` se llama a 1Hz **pero el NMEA se descarta**: el fd de "la app" es
el **pipe que normalmente crea mnld** (fork+execv con `argv[1]=fd0 argv[2]=fd1`), ausente en standalone → 0
bytes en `/dev/gps`. Dos formas de cerrarlo (para casa, con móvil + **cielo despejado**):
- **(a) Vía mnld (la intencionada)**: mnld forkea `/system/xbin/libmnla` (default sin el prop
  `persist.radio.mediatek.chipid`) → **poner ahí el STOCK bionic** (`cp libmnlp_mt6582 → /system/xbin/libmnla`,
  ya hecho) → mnld le da el pipe, lee el NMEA y lo relaya. **PENDIENTE**: mnld **no forkeó el hijo** con solo
  `INIT(0x00)+START(0x03)` por `hal2mnld` (queda en `launch_daemon_thread`/`mtk_gps_sys_init`, aparece
  `mtk_gps_exit_proc`). Falta la **secuencia/estado exactos** que disparan el fork (¿ack de INIT?, ¿estado
  DSP-ready por `/sys/class/gpsdrv`?, ¿el prop del chipid para elegir `libmnlp_mt6582`?). RE del state-machine
  de `mnld` (`MNLD__src__mnld_6620.c`) = el siguiente paso.
- **(b) Cablear el fd standalone**: darle al `single_process()` un fd de salida válido apuntando a `/dev/gps`
  (más control, evita pelear con el fork de mnld). Requiere ver dónde el stock coge el "app fd" y forzarlo.

**Reglas de arranque (verificadas)**: driver `mt6582-gpsdrv.ko` cargado (`/dev/gps` + `/dev/stpgps` existen);
shim `libxlogshim.so` (con xlog visible) en `/system/lib`; `sc/EPO.DAT` en el cwd; `mkdir -p /data/gps_mnl`.

*Receta 2026-07-14 (sesión Mac, noche-4). **El motor GPS bionic corre y produce NMEA a 1Hz en pmOS mainline.**
Falta sólo enrutar ese NMEA a `/dev/gps` (fork de mnld o fd standalone) + cielo despejado. Fase A (glibc)
descartada por el mutex ABI. Móvil: multiboot sector 83968; ver [[reference-mtkclient-krillin]].*

## 🔬 STATUS 0714 (noche-4b) — CONECTAR LA SALIDA: mapa COMPLETO del data-flow + los gates

Objetivo de la sesión: enrutar el NMEA a un fd consumible. **Resultado: el data-flow está mapeado byte a byte;
la salida limpia NO es un fix rápido — necesita portar mnld o un decodificador binario (trabajo de casa).**

### Qué hace el motor standalone (probado)
Con `libmnlp_mt6582 1Hz=y` (ó argc=4 `libmnlp <fd0> <fd1> <assist>`): navega, llama a
`mtk_gps_sys_nmea_output_to_app` ~1Hz. **Pero standalone NO emite NMEA a NINGÚN sitio accesible:**
- **`dae_snd_fd` (el fd1 del pipe) NO recibe el NMEA** — le pasé un fichero por fd1 y quedó a 0 bytes.
- El motor SÍ escribe **reportes BINARIOS de 116 B que empiezan `AA F0` (0xAAF0, protocolo MTK, la posición)**
  a `dae_snd_fd` — pero `dae_snd_fd` **quedó en fd 0** (default; `write(0,…)=EBADF`), NO tomó mi arg de fd.
  (Los fds del motor colisionan: usa fds bajos 3-8 internamente; pasarle fd5/fd6 los machaca — usar fds altos
  pero aun así `dae_snd_fd` no honró el arg → el stock parsea distinto o lo resetea.)
- **NO hay NMEA en stdout, ni en `__xlog` (stderr), ni en `logdw` (ALOGD).** Forcé `debug.debug_nmea=1` +
  `debug.mnl=ff` vía `/data/misc/mnl.prop` (formato `key=val`, claves en `set_prop`: `dev.dsp/dev.gps/
  pmtk.conn/debug.debug_nmea/debug.mnl…`) y nada. El NMEA se genera DENTRO de libmnl pero no se serializa
  standalone. → **el `mtk_gps_sys_nmea_output_to_app` del stock NO escribe el NMEA a un fd standalone;
  la posición sale como el binario 0xAAF0 que mnld convierte a NMEA.**

### Por qué mnld no forkea el hijo (los gates del entorno Android en pmOS)
mnld LEE mis comandos (`read(3,"\0")` INIT + `read(3,"\3")` START ✓), escribe `pwrctl`/`state` a
`/sys/class/gpsdrv` ✓ (`mnl_write_attr` OK: `char buf[]={attr+'0'}`, sizeof=1, sin bug), entra en
`launch_daemon_thread` — **pero NO completa el `fork()`+`execl`** (0 `execute:`, 0 `we get MT6582`, 0 hijo
`libmnla`). Gates encontrados (mnld espera el entorno Android que pmOS no tiene):
1. **`chip_detector` en bucle** (21× en el xlog) — no detecta el combo chip en pmOS.
2. **`property_get("persist.radio.mediatek.chipid")` falla** (no hay property service:
   `/dev/socket/property_service`+`/dev/__properties__` → ENOENT). En el hijo forkeado, si no casa con un chip
   hace `goto error` (NO `execl`). **La interposición LD_PRELOAD de `property_get` NO funciona en bionic**
   (solo interpone símbolos INDEFINIDOS; `property_get` está definido en libcutils → gana libcutils). El shim
   la trae igualmente por si el linker de otra build la respeta.
3. **AGPS**: `bind /data/agps_supl/agps_to_mnl` falla (ENOENT) — ruidoso, probablemente no bloqueante.

### Herramientas construidas esta sesión (en el móvil / repo)
- **Listener de `logdw`** (`/tmp/logdw.py`): bind `SOCK_DGRAM /dev/socket/logdw` → captura TODO el ALOGD/ALOGE
  (así se leyeron los ERR de MNL2AGPS). Sin interposición.
- **Shim con `property_get` + xlog conmutable** (`gps-bionic-shim.c`, en repo): `GPS_XLOG_QUIET=1` silencia.
- `mnl.prop` de config, `sc/EPO.DAT`, `launch.py` (lanza el motor en modo pipe con fds altos).

### ⏭️ Cerrar la salida (dos vías, ambas = TRABAJO DE CASA, no triviales)
- **(A) Portar mnld más allá de los gates** (la limpia): stubear/satisfacer `chip_detector` +
  `property_get(chipid)=0x6582` (via un mini property-service en `/dev/socket/property_service`, como el
  listener de logdw, o parcheando el binario mnld) + AGPS → mnld forkea el hijo bionic, lee el 0xAAF0 del `c2p`
  y lo convierte a NMEA → escribe donde lea el HAL/gpsd. Requiere RE de `chip_detector` en `mnld_6620.c`.
- **(B) Decodificador 0xAAF0→NMEA propio**: darle al motor un `dae_snd_fd` VÁLIDO (arreglar por qué no honra
  el arg), capturar los reportes binarios 0xAAF0 de 116 B, y portar el parser de mnld (posición→sentencia
  NMEA). Evita pelear con el entorno Android de mnld pero hay que decodificar el binario.

Luego (cualquier vía): NMEA → `/dev/gps`/socket → gpsd → geoclue → Phosh, + **cielo despejado** (interior=0 sat).

*Noche-4b: el "connect an output" resultó ser el tramo duro real — el NMEA del motor NO se serializa
standalone (sale como binario 0xAAF0 que mnld convierte), y mnld tiene gates de entorno Android en pmOS
(chip_detector/property/AGPS). Data-flow 100% mapeado; falta portar mnld o el decodificador 0xAAF0.*

## 🧩 STATUS 0714 (noche-4c) — Vía A intentada: property RESUELTO, pero el fork de mnld es un muro (binario ≠ fuente)

Intento de la **vía A** (que mnld forkee el hijo). Dos resultados:

### ✅ AVANCE REAL: el gate del chipid/property RESUELTO con el área de bionic
La interposición LD_PRELOAD de `property_get` no va en bionic, pero **poblar el ÁREA real de properties SÍ**:
en el `constructor` del shim → `__system_property_area_init()` + `__system_property_add("persist.radio.mediatek.chipid",29,"0x6582",6)`
(símbolos exportados por `/system/lib/libc.so`, verificado). **Crea `/dev/__properties__` (128KB) con el chipid**
(`strings` lo confirma: `persist.radio.mediatek.chipid`=`0x6582`), y **el bionic lo abre y lo lee** (fd 3). Así
`property_get` del stock funciona NORMAL, sin interponer, y se hereda al hijo forkeado. **Técnica reutilizable
para correr cualquier daemon Android/bionic en pmOS que dependa de properties.** (Conmutable: `GPS_SET_CHIPID=1`.)

### ❌ MURO: mnld NO llega al `fork()` — y la fuente `nu3001` NO coincide con el binario
Con o sin property, mnld es **idéntico y consistente**: `start_mnl_process` → escribe `pwrctl`/`state` a
`/sys/class/gpsdrv` ✓ → **`launch_daemon_thread` se traza (×4) pero NUNCA hace `pipe()` ni `fork()`** (strace
`-f -e pipe,pipe2,clone,fork,vfork` = 0 forks, 0 pipes; 0 hijos `libmnla`; `mtk_gps_exit_proc`=0). Según
`nu3001/MNLD__src__mnld_6620.c` (la fuente que leí) **entre el `MND_TRC()` de entrada y el `pipe()` no hay NADA**
→ imposible que se trace la entrada y no llame a pipe. **Conclusión: el binario `mnld` de LineageOS NO es esta
versión de la fuente** — hace algo distinto antes del fork (¿otra detección de chip?, ¿un gate de estado?, ¿AGPS
bloqueante?). El RE remoto sobre la fuente equivocada es poco fiable.

### ⏭️ Para casa (con el móvil en mano)
- **Desensamblar el `mnld` real** (`/system/xbin/mnld`, ~72KB, ARM) alrededor de `start_mnl_process`/
  `launch_daemon_thread` para ver el gate real del fork (Ghidra/objdump), O
- **Arrancar LineageOS** (dual-boot) y `strace -f` su `mnld` durante un fix real → ver la secuencia CORRECTA
  (qué properties/ficheros/estado necesita antes de forkear) y replicarla, O
- **gdb interactivo** sobre el mnld en pmOS (breakpoint en el fork) para ver por qué no llega.
- El property ya no es el gate (resuelto). El bloqueo es el camino al fork dentro del binario mnld.
- Alternativa: **vía B** (decodificador 0xAAF0→NMEA propio) sigue en pie y NO depende de mnld.

*Noche-4c: property/chipid gate RESUELTO (área bionic real, reutilizable). El fork de mnld sigue bloqueado
pero la causa es que la fuente nu3001 ≠ binario — necesita desensamblado/gdb/Lineage-ref (casa).*
