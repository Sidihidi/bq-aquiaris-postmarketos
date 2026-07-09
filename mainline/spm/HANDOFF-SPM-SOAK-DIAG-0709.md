# HANDOFF — SPM soak: el crash NO es el offline, es el DEEP-SLEEP de CPU0 (2026-07-09)

> Sesión Mac. Hallazgo que **redirige** la investigación del M3. Driver: `mt6582-spm.c` + `platsmp.c`.

## 🎯 HALLAZGO PRINCIPAL (refuta el marco anterior)
El crash intermitente del soak de suspend profundo (M3, `spm_cpu_pdn=1`) **NO está en el offline de los
cores secundarios**. Estaba oculto por un **artefacto de visibilidad**: la consola se suspende justo
después de "Disabling non-boot CPUs", así que ESA parecía la línea del cuelgue. Con `no_console_suspend`
horneado + instrumentación, el trace real (console-ramoops, kernel #268) muestra:

```
SPMDBG-SOAK cycle 1
PM: suspend entry (deep)
Disabling non-boot CPUs ...
die3: enter → gic off, wfi → kill3: enter → kill3: off done pwr=0x3d5f
die2: enter → gic off, wfi → kill2: enter → kill2: off done pwr=0x395f
die1: enter → gic off, wfi → kill1: enter → kill1: off done pwr=0x315f
   ← TERMINA AQUÍ. Sin "wake:", sin "Enabling non-boot CPUs" → WDT reset (boot_reason=4)
```

**Los 3 cores secundarios (3,2,1) se apagan LIMPIAMENTE**: `die`+`kill`+power-off MTCMOS completos,
**cero `m2_poll TIMEOUT`**, `pwr status` limpia bits bien (`0x3d5f→0x395f→0x315f`). El offline funciona.
El crash es **después**, en `spm_suspend_enter` → el **dormant/resume de CPU0** (PCM sleep +
`cpu_pm_enter`/`cpu_cluster_pm_enter`/`cpu_suspend(finisher)` → BootROM). Nunca despierta.

**Implicación:** el fix `gic_cpu_if_down(0)` de `cpu_die` (platsmp) NO era el fix (el offline nunca fue el
problema). El foco correcto = el path de sueño de CPU0 en `mt6582-spm.c:408-427`.

## MÉTODO DE CAPTURA (esto es lo que por fin hizo visible el crash — reutilizar)
El crash es **invisible al pstore por defecto** (la consola se suspende antes del punto del cuelgue). Fix:
**hornear en el cmdline** (sobrevive reboots + efectivo desde el arranque; runtime NO sirve porque el
crash resetea los params):
```
CONFIG_CMDLINE_EXTEND=y
CONFIG_CMDLINE="no_console_suspend mt6582_spm.spm_cpu_pdn=1 mt6582_spm.spm_wake_sec=15"
```
- `no_console_suspend` → la consola sigue viva durante el suspend → el trace del offline+sleep va a
  `console-ramoops`, que **sobrevive al WDT reset**.
- `spm_cpu_pdn=1` + `spm_wake_sec=15` → cada `echo mem` es M3 profundo con wake a los 15s (soak rápido),
  sin depender de escribir params en runtime (que se pierden en el reboot).
- Soak: `while :; do echo mem > /sys/power/state; sleep 2; done` con marca por ciclo a `/dev/kmsg`.
- Leer tras el crash: `grep SPMDBG /sys/fs/pstore/console-ramoops-0` (tras un power-cycle si sshd flaky).
- GOTCHA: tras cada crash el móvil queda con **sshd flaky → power-cycle a mano** para leer el ramoops.

## INSTRUMENTACIÓN aplicada (en el árbol de build de la Pi .123, #269 — re-aplicar si se regenera)
En `arch/arm/mach-mediatek/platsmp.c`:
- `m2_poll()`: `pr_warn("SPMDBG m2_poll TIMEOUT off=0x%x mask=0x%x want=0x%x got=0x%x\n", ...)` antes del
  `return -ETIMEDOUT`.
- `mtk_cpu_die()`: `pr_info("SPMDBG die%u: enter\n", cpu)` al inicio + `pr_info("SPMDBG die%u: gic off, wfi\n")` tras `gic_cpu_if_down`.
- `mtk_cpu_kill()`: `pr_info("SPMDBG kill%u: enter\n")` + `pr_info("SPMDBG kill%u: off done pwr=0x%x\n", cpu, m2_r(M2_SPM_PWR_STATUS))` tras `m2_cpu_power_off`.

En `drivers/soc/mediatek/mt6582-spm.c` (`spm_suspend_enter`, bloque `if (spm_cpu_pdn && s->bootvec)`):
- `pr_info("SPMDBG spm: pre-sleep cpu_pm_enter\n")` antes de `cpu_pm_enter()`.
- `pr_info("SPMDBG spm: cpu_suspend CPU0 dormant\n")` antes + `pr_info("SPMDBG spm: RESUMED\n")` después de `cpu_suspend(0, finisher)`.
- `pr_info("SPMDBG spm: exit ok\n")` tras `cpu_pm_exit()`.
(NO instrumentar el finisher: printk tras el save de cpu_suspend rompe el sleep.)

## SIGUIENTE (pendiente: leer el trace del #269 tras power-cycle)
Kernel **#269** flasheado con la instrumentación del sleep. El soak crasheó (ciclo ~1) pero quedó sin sshd;
**falta un power-cycle para leer `console-ramoops`** y ver cuál de estos es:
- `pre-sleep cpu_pm_enter` SIN `cpu_suspend CPU0 dormant` → cuelgue en el save GIC/VFP del cluster.
- `cpu_suspend CPU0 dormant` SIN `RESUMED` → **CPU0 no vuelve del BootROM** (salto caliente / finisher /
  PCM sleep). ← hipótesis más probable (encaja con "el BootROM no toma el salto caliente" del intento M3-1).
- `RESUMED` SIN `exit ok` → cuelgue en la restauración del GIC (`cpu_cluster_pm_exit` → `gic_dist_restore`).

Según cuál sea:
- Si es "CPU0 no vuelve": revisar el protocolo warm-boot del BootROM (bit31/llave mágica de 0x10001800/0x804),
  el `mt6582_spm_finisher` (`v7_exit_coherency_flush` + wfi), y si el PCM realmente arranca el sleep vector.
- Si es GIC restore: el `cpu_cluster_pm_exit`/`gic_dist_restore` intermitente (candidato irq-mtk-cirq).

## Estado del móvil
Kernel #269 con la instrumentación + config horneado. `spm_cpu_pdn=1` por cmdline (M3 activo). Para volver
a uso normal seguro: flashear un kernel sin `spm_cpu_pdn=1` en el cmdline (M1/s2idle) o `spm_cpu_pdn=0`.

*Sesión Mac (Fable 5), 2026-07-09. El offline está resuelto; el trabajo real es el dormant/resume de CPU0.*
