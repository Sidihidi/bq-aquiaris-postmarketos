# GPS Vía 1 (mnld) — chip_detector RESUELTO; gate final = bucle `mtk_gps_sys_init` de mnld (Ghidra) — 0714 (casa)

> Continuación de `RECETA-BIONIC-VIA1-0714.md` (sesión Mac). Avancé el gate: **mnld ya PASA
> chip_detector** con el shim v2; el blocker se movió a `launch_daemon_thread → mtk_gps_exit_proc`.
>
> **CIERRE de esta sesión (strace fino)**: descartado el candidato del read-offset del gpsdrv (era el EOF del
> config `/data/misc/mnl.prop`). El gpsdrv funciona bien. El gate real es el **bucle interno de
> `mtk_gps_sys_init` de mnld** que nunca abre `/dev/stpgps` → necesita **Ghidra sobre `mnld`** (o la Vía A
> soft-float, que evita el binario cerrado). Ver §"El gate real".

## Lo que estaba pasando (y lo arreglado)
El `mnld` stock (Vía 1) se quedaba en **bucle de `chip_detector`** → nunca forkeaba el runner. Raíz
localizada por strings del `mnld`: lee el property **`persist.mtk.wcn.combo.chipid`** (NO el
`persist.radio.mediatek.chipid` que ponía el shim v1). En pmOS no hay área de properties de bionic →
`property_get` devuelve vacío → chip_detector falla.

**Fix (shim v2, `gps-bionic-shim-v2.c`, VALIDADO)**: poblar el área de properties de bionic con el chipid
correcto. Dos claves respecto al v1:
1. La clave correcta = **`persist.mtk.wcn.combo.chipid` = "0x6582"** (28 chars). (Se añade también la
   `persist.radio.mediatek.chipid` por si acaso.)
2. Usar **`dlsym`** en runtime para `__system_property_area_init`/`__system_property_add` (SÍ los exporta
   `/system/lib/libc.so`, versión `@@LIBC`) — el v1 los referenciaba directo y petaba al relocar el shim
   `-nostdlib` con "symbol not found". Con dlsym el shim carga limpio.
Compilar: `arm-linux-gnueabihf-gcc -shared -nostdlib -fPIC -O1 -o libxlogshim.so gps-bionic-shim-v2.c -ldl`.
Correr: `env GPS_SET_CHIPID=1 LD_LIBRARY_PATH=/system/lib LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld`.

**Resultado**: el flujo del `mnld.log` ahora avanza: `main → chip_detector` (una vez, ya no bucle) `→
epoll_init → socket_handler → start_mnl_process → launch_daemon_thread → mtk_gps_sys_init →
launch_daemon_thread → mtk_gps_exit_proc`. **chip_detector superado.** (El shim v2 ya está desplegado en
`/system/lib/libxlogshim.so` del móvil; backup del v1 en `.v1`.)

## El SIGUIENTE blocker (bien acotado): launch_daemon_thread no forkea
Tras enviar `INIT(0x00)+START(0x03)` al socket `hal2mnld`, mnld llega a `launch_daemon_thread` pero hace
**`mtk_gps_exit_proc` ANTES de forkear** el runner (coincide con la nota del Mac: "launch_daemon_thread no
llega al fork()"). Comprobado:
- **No es el runner que muere**: el runner stock (`libmnlp_mt6582`) corre bien **a mano** con el env del
  shim (llega a `main` + carga properties) — probado con un wrapper `libmnla`/`libmnlp_mt6582` que inyecta
  `LD_PRELOAD`+`LD_LIBRARY_PATH` y exec-a el binario real. O sea, el runner NO es el problema; es mnld que
  no llega a lanzarlo.
- **Sospecha fuerte = el gate del "DSP listo"** (nota Mac noche): en START, libmnl (cerrado) escribe
  `pwrctl` y **espera que `/sys/class/gpsdrv/gps/status` diga "listo"** antes de forkear/abrir el DSP. El
  driver `mt6582-gpsdrv.c` reporta siempre **"0, none"** (visto en el status) → libmnl no ve "listo" →
  `mtk_gps_exit_proc`. La lógica exacta de "listo" está DENTRO de libmnl (no RE-able por fuente).

## Probado (casa 0714, NO cerró): sysfs_notify + state=START en el gpsdrv
Hipótesis: libmnl hace `poll()` de `status`/`state` esperando un `sysfs_notify` tras `pwrctl=RST`.
Parcheado `mt6582-gpsdrv.c`: en la rama `GPS_PWRCTL_RST` → `o->state=GPS_STATE_START` +
`sysfs_notify(kobj,"status")`+`sysfs_notify(kobj,"state")` (+ notify en cada `state_store`). Recompilado
(.ko) y recargado. **Resultado**: el `state` SÍ transiciona a 2 (START) tras el `pwrctl=RST` de mnld, pero
el flujo del mnld.log es **idéntico** (`launch_daemon_thread → mtk_gps_sys_init → launch_daemon_thread →
mtk_gps_exit_proc → main`). **El gate NO es el poll de status/state.** Está más adentro, en `mtk_gps_sys_init`
(la init del DSP DENTRO del `libmnl` cerrado — el daemon-thread se aborta ahí, no en la interfaz gpsdrv).
El driver con el notify quedó cargado en el móvil (inofensivo; arguablemente correcto). No RE-able por
la interfaz del driver → hace falta desensamblar `libmnl.so` (Ghidra) el `mtk_gps_sys_init`/`launch_daemon`
para ver qué comprueba antes de forkear el runner (¿otro fd? ¿un ioctl a stpgps? ¿un valor concreto?).

## ★★ STRACE del mnld (casa 0714) — MECANISMO REVELADO (corrige la suposición del fork)
`strace -f` de mnld (con chipid+shim) muestra la secuencia real tras el START:
- Abre los sysfs del gpsdrv (status/pwrctl/state/suspend/pwrsave) OK, crea un **socketpair [8,9]**,
  lee la config (`dev.dsp=/dev/stpgps dev.gps=/dev/gps debug_nmea=1 pmtk.conn=serial`).
- **NO hay fork+execv de un runner separado**: hace **`clone(CLONE_THREAD)`** = un HILO (el motor corre
  DENTRO de mnld usando `libmnl.so` directamente). → **la Vía 1 NO necesita el runner libmnlp aparte**
  (adiós al problema del ABI/args del runner). El símbolo `mnld` tenía los strings del gate (no libmnl):
  `launch_daemon_thread`/`mtk_gps_sys_init`/`mtk_gps_exit_proc` son log-tags de mnld.
- **Secuencia gpsdrv**: mnld escribe `pwrctl` **0→1→2** (OFF→ON→RST) en el fd, y luego lo **lee VACÍO**
  (`read(fd,"")=0`) — porque las 3 escrituras avanzaron el offset del fd más allá del valor del sysfs.
- **`/dev/stpgps` NUNCA se abre** → el hilo principal **gira en `mtk_gps_sys_init` SIN syscalls** (espera
  una condición en memoria) → nunca llega al DSP.
- El hilo daemon (tid nuevo) hace `connect(/dev/socket/property_service)` = **ENOENT** (no existe en pmOS).

## ⚠️ Candidato #1 (read-offset del gpsdrv) DESCARTADO
Análisis fino de la secuencia cruda del strace: el `read(fd,"")=0` que parecía un read del gpsdrv es en
realidad el **EOF del fichero de config `/data/misc/mnl.prop`** (mnld lo lee OK, 86 bytes:
`dev.dsp=/dev/stpgps dev.gps=/dev/gps debug_nmea=1 mnl=ff pmtk.conn=serial`). Los writes al gpsdrv van
cada uno a un **fd fresco** (openat por cada write) → NO hay problema de offset. El driver del gpsdrv
**funciona bien**; NO es el gate. (El patch de sysfs_notify queda como mejora inofensiva.)

## ⏭️ El gate real (refinado): el bucle de `mtk_gps_sys_init` DENTRO de mnld
Secuencia tras el START: `pwrctl=2`(RST) → lee la config OK → `mnl_utl_load_property` → **bucle de
`mtk_gps_sys_init`** (función INTERNA de mnld, no de libmnl — el string está en mnld) que **NUNCA intenta
abrir `/dev/stpgps`** (0 openat de stpgps en todo el strace) → tras ~20 vueltas, `mtk_gps_exit_proc`.
El bucle no hace syscalls → espera una condición en memoria. Candidatos restantes:
1. **Ghidra sobre `mnld`** (72KB, Thumb-2 stripped): desensamblar/decompilar `mtk_gps_sys_init` para ver
   qué comprueba antes de abrir el DSP y por qué reintenta+abandona. Es EL gate y es RE de binario.
   (objdump NO sirve: mal-desensambla el Thumb; Ghidra auto-detecta.) ⚠️ Disco de la Pi al 93% — Ghidra
   ocupa ~400MB, liberar espacio primero o correrlo en otra máquina (¿la del Mac?).
2. **property_service** (menor probabilidad): el hilo daemon falla al `connect(/dev/socket/property_service)`;
   si `mtk_gps_sys_init` espera un property que el daemon SETea por ahí, un property_service mínimo lo
   desbloquearía. Verificar primero con Ghidra si el bucle depende de eso.
3. **Alternativa (Vía A, ABI) — SIN RE de binario cerrado**: recompilar el runner de Fase A (`mnlp_static`)
   con sysroot **soft-float** → habla con el DSP (ya probado por el Mac) Y emitiría NMEA — evita TODO el
   gate de mnld/`mtk_gps_sys_init`. Es la vía que NO depende de decompilar mnld; si Ghidra se atasca, esta
   es el camino de menor riesgo hacia NMEA.

## Estado en el móvil (limpio)
Setup del Mac intacto: `gpsdrv` cargado, calibración TCXO real (`/data/nvram/APCFG/APRDEB/GPS`), runners
en `/system/xbin/`, **shim v2 en `/system/lib/libxlogshim.so`** (v1 en `.v1`). Binarios restaurados (sin
wrappers). Scripts de arranque de mnld: ver los `gps-mnld-run*.sh` (esta sesión, en scratchpad) o la
receta de arriba.

*Casa (Fable 5), 2026-07-14. chip_detector cerrado; siguiente = el gate del "DSP listo" del gpsdrv.*
