# HANDOFF — SPM M3: el crash del soak es un BUG_ON de hotplug en el OFFLINE (2026-07-09, actualizado casa)

> Actualiza y **corrige** la conclusión anterior de este doc. Driver: `mt6582-spm.c` + `platsmp.c`.

## 🎯 HALLAZGO DEFINITIVO (refuta la hipótesis "es el deep-sleep de CPU0")
Con `no_console_suspend` horneado + instrumentación de fase (SPMDBG), el trace real del crash
(console-ramoops + dmesg-ramoops, kernel #270) es un **oops con BUG explícito, NO un WDT mudo**:

```
kernel BUG at kernel/cpu.c:1929!
PC is at freeze_secondary_cpus+0x16c/0x200
  → BUG_ON(num_online_cpus() > 1)
Call trace: freeze_secondary_cpus ← suspend_devices_and_enter ← pm_suspend ← state_store ← write
```

- El **deep-sleep de CPU0 FUNCIONA**: en los ciclos que sobreviven, el trace imprime la secuencia
  completa `spm: pre-sleep cpu_pm_enter → cpu_suspend CPU0 dormant → RESUMED → exit ok → wake:
  r12=...TIMER`. O sea CPU0 se apaga, el BootROM salta a `cpu_resume`, el GIC se restaura y despierta.
  **Eso ya no es el problema** (era la conclusión del doc anterior — refutada).
- El crash es **intermitente y en el OFFLINE de los cores secundarios** (`freeze_secondary_cpus`),
  no en el sleep. `BUG_ON(num_online_cpus() > 1)` = tras "downear" los 3 cores secundarios, más de
  uno sigue online → carrera de hotplug/MTCMOS en el offline rápido y consecutivo del suspend
  (a diferencia del hotplug manual 1-a-1 con sleeps, que SÍ es fiable = validación de M2).

## FIX APLICADO (kernel #272+): blindaje de `mtk_cpu_kill` (platsmp.c)
Convierte el **crash duro (BUG_ON) en un aborto LIMPIO del suspend**: `cpu_kill` ya no apaga un core
que no haya confirmado WFI (apagar el MTCMOS de un core vivo cuelga el AHB), y verifica el power-off:
```c
if (m2_poll(SLEEP_TIMER_STA, APMCUX_SLEEP(cpu), ..., 200000)) {
    pr_warn("mt6582: CPU%u no confirmo WFI; suspend abortado\n", cpu);
    return 0;   /* kill fallido -> el generico aborta el suspend, sin BUG */
}
m2_cpu_power_off(cpu);
if (m2_r(PWR_STATUS) & FCX_STA(cpu)) { pr_warn(... "no se apago" ...); return 0; }
```
Esto NO elimina la carrera de raíz, pero degrada el fallo de "crash+WDT reset" a "el suspend no entra
esta vez" (el sistema sigue vivo). Con `spm_cpu_pdn=0` por defecto, el `mem` de diario es M2 y ni
siquiera intenta el dormant de CPU0.

## SIGUIENTE (raíz de la carrera del offline — pendiente)
El `num_online_cpus() > 1` intermitente apunta a que un core "downeado" no queda realmente offline en
la contabilidad. Candidatos a investigar (con la instrumentación SPMDBG ya en el árbol):
- Timing entre `cpuhp_ap_report_dead()` (dying) y nuestro `gic_cpu_if_down(0)` + `m2_cpu_power_off`
  (killing): ¿el report_dead se hace visible antes de cortar la caché/GIC del core?
- `mtk_cpu_die`: ¿el `while(1) wfi()` puede reanudar y reonlinear el core si una IPI llega tras el
  `gic_cpu_if_down` pero antes del power-off? (mirar si hace falta enmascarar más o un handshake).
- El `m2_poll` de `APMCUX_SLEEP`: ¿el bit es fiable por-core o hay solape entre cores consecutivos?
- Comparar con el `hotplug.c` del downstream (secuencia exacta de WFI-check + MTCMOS por core).

## MÉTODO DE CAPTURA (reutilizar — así se hizo visible el crash)
- Config horneado (SOLO para debug; QUITAR para uso normal, auto-activa M3):
  `CONFIG_CMDLINE_EXTEND=y` + `CONFIG_CMDLINE="no_console_suspend mt6582_spm.spm_cpu_pdn=1 mt6582_spm.spm_wake_sec=15"`.
- Guardián de pstore: `/etc/local.d/00-pstore-save.start` copia `/sys/fs/pstore/*` a `/root/pstore-logs/<ts>/`
  al arranque (el WDT reset conserva ramoops pero un 2º reboot lo machaca; esto lo preserva).
- Soak: `while :; do echo "SPMDBG-SOAK cycle $i" > /dev/kmsg; echo mem > /sys/power/state; ...; done`.
- Leer tras el crash: `grep SPMDBG /root/pstore-logs/<ultimo>/dmesg-ramoops-0` (power-cycle a mano si sshd flaky).

## Estado del móvil / kernel
Kernel **#272+** (seguro): `spm_cpu_pdn=0` por defecto, SIN cmdline horneado, `mtk_cpu_kill` blindado.
`mem` de diario = M2 (offline manual fiable, 4 cores vuelven). M3 (`spm_cpu_pdn=1`) = experimental,
aborta limpio si la carrera del offline pica. GOTCHA de build: forzar `rm` de los `.o` de `mt6582-spm`
y `platsmp` + `vmlinux/zImage` antes de recompilar (si no, cambios de fuente pueden no relinkar).

*Sesión casa (Fable 5), 2026-07-09. M1+M2 sólidos; M3 funcional en ciclos sueltos; la carrera del
offline en soak es el trabajo de raíz pendiente, ya con crash→abort-limpio de red de seguridad.*
