# Fixes de la sesión 0727 (M119→M133) — Darwin/XNU en el krillin

Los `.py` son los parches tal cual se aplicaron sobre el árbol XNU de la Pi `.123`
(`~/darwin-krillin/xnu/`). Los `.txt` son el código resultante, extraído del árbol,
para poder leer el fix sin ejecutar nada.

El árbol de la Pi **no es git**, así que esto es la foto de los cambios.
Contexto completo y razonamiento: `../M128-MURO-ROTO-LAUNCHD-EJECUTA-0727.md`.

| Fix | Fichero XNU | Qué arregla |
|---|---|---|
| **M128** ★ | `osfmk/arm/trap.c` | **El bug grande**: `sleh_abort` invalidaba la TLB solo en los fallos de KERNEL; la rama de USUARIO reintentaba con la entrada vieja RO en la TLB → bucle infinito de fallos de permisos. `TLBIMVA`+`TLBIALL` antes del reintento en las 2 rutas de usuario. |
| M120 | `osfmk/kern/sched_prim.c` | `csw_check()` tenía un `return AST_PREEMPT\|AST_URGENT` hardcodeado **de upstream** que anulaba toda su lógica. |
| M130 | `pexpert/arm/pe_mt6582.c` | `mt6582_getc` giraba 100.000 lecturas MMIO esperando una tecla, cada 16 ms y a prioridad 95 → se comía el 85% de la CPU. Ahora es sondeo instantáneo. |
| M131 | `iokit/Kernel/IOPlatformExpert.cpp` | Sin RTC, `getGMTTimeOfDay()` devuelve 0 → el calendario arrancaba en `epoch = 0 − uptime` (negativo). Fecha fija provisional. |
| M132 | `osfmk/arm/pmap.c` | `pmap_sync_page_data_phys()` era un stub vacío y la I-cache no se invalidaba nunca al mapear páginas de código. |
| M133 | `osfmk/arm/trap.c` | (Diagnóstico, no fix) vuelca los bytes reales de memoria en el fallo de instrucción indefinida. |
