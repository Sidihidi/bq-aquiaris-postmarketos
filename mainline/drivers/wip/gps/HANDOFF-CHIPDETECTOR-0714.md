# GPS Vía 1 (mnld) — chip_detector RESUELTO, blocker avanza a launch_daemon_thread — 0714 (casa)

> Continuación de `RECETA-BIONIC-VIA1-0714.md` (sesión Mac). Avancé el gate: **mnld ya PASA
> chip_detector** con el shim v2; el blocker se movió a `launch_daemon_thread → mtk_gps_exit_proc`.

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

## ⏭️ Para cerrar (candidatos, orden de valor)
0. **Desensamblar `libmnl.so`** (`/system/lib/libmnl.so`, ARM, con Ghidra) el `mtk_gps_sys_init` +
   `launch_daemon_thread`: ver qué comprueba/abre justo antes del fork del runner y por qué hace
   `mtk_gps_exit_proc`. Es el gate real y está en el binario cerrado. (chip_detector ya no bloquea.)
1. **Hallar el valor "listo" que libmnl espera del gpsdrv** y hacer que el driver `mt6582-gpsdrv.c` lo
   reporte tras `pwrctl` (state/status). Sin strace en el móvil, opciones: (a) desensamblar el `libmnl.so`
   (Ghidra) el read de `/sys/class/gpsdrv/gps/{state,status}` tras el `write pwrctl`; (b) probar valores en
   `status`/`state` del driver (p.ej. state=2, status="1, fix" o el formato que espere) hasta que mnld
   pase de `launch_daemon_thread` al fork. El driver es nuestro → fácil de instrumentar/cambiar.
2. **Alternativa (Vía A, ABI)**: recompilar el runner de Fase A (`mnlp_static`) con sysroot **soft-float**
   → habla con el DSP (ya probado por el Mac) Y emitiría NMEA — evita todo el gate de mnld/libmnl.

## Estado en el móvil (limpio)
Setup del Mac intacto: `gpsdrv` cargado, calibración TCXO real (`/data/nvram/APCFG/APRDEB/GPS`), runners
en `/system/xbin/`, **shim v2 en `/system/lib/libxlogshim.so`** (v1 en `.v1`). Binarios restaurados (sin
wrappers). Scripts de arranque de mnld: ver los `gps-mnld-run*.sh` (esta sesión, en scratchpad) o la
receta de arriba.

*Casa (Fable 5), 2026-07-14. chip_detector cerrado; siguiente = el gate del "DSP listo" del gpsdrv.*
