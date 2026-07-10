# SPM M3: el cuelgue post-resume del soak — caracterización (0710, sesión casa)

## Patrón (4 soaks instrumentados, kernels #273-#277)
- Muere tras **~6 ciclos** M3 consecutivos (6,6,6,4), **1-3 s DESPUÉS** del último resume completo
  (suspend exit impreso, userspace corriendo), EN IDLE (una vez en mitad de una pausa de 120 s).
- Muerte SÚBITA total: heartbeats de 200 ms a kmsg se cortan en seco (último a +2.4 s del ciclo 6);
  sin oops, sin panic, con no_console_suspend → bus/SoC lockup instantáneo → WDT reset (boot_reason=4).
- EXONERADOS por experimento: WiFi (soak con wlan0 down = igual), daemons de polling (parados = igual),
  tail "normal" del PCM (SW_RESET del PCM entre ciclos en #277 = igual, murió en ciclo ~5),
  alarma RTC (los soaks usan PCM_TIMER).
- Registros SPM estables ciclo a ciclo hasta la muerte: PCM_FSM_STA=0x48490, PWR_STATUS=0x3f5d
  (muestreados 1/s a kmsg; sin deriva). OJO: la columna "clk" de soakG leía 0x11220400 (AFE, error mío).

## Hipótesis abiertas para la siguiente iteración
1. **Falta el baile del GIC del stock**: mt_irq_mask_all + unmask-solo-SPM + CIRQ clone/flush alrededor
   del ciclo (nosotros dormimos con el GIC abierto). Una IRQ/estado marginal podría corromperse una
   fracción por ciclo. El driver mainline irq-mtk-cirq (offsets v1 OK) + nodo DT es el camino limpio.
2. Acumulación en el dominio 26M/CLKSQ o DRAM self-refresh marginal (¿corrupción silenciosa que
   estalla al tocar una página concreta?) — probaría memtester tras N ciclos.
3. Algo del reloj del sistema/timekeeping que degenera con N inyecciones de sleep-time.

## Cómo reproducir + leer
- `echo 1 > /sys/module/mt6582_spm/parameters/spm_cpu_pdn; echo 10 > .../spm_wake_sec`
- bucle `echo mem > /sys/power/state` con log a /root/ y sync; heartbeat 200ms + regs a /dev/kmsg.
- Tras el WDT reset: el guardián (00-pstore-save.start) deja el console-ramoops en /root/pstore-logs/.
