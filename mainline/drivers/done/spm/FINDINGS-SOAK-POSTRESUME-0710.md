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

## Ronda 2 (0710 tarde, kernels #277-#278) — más exoneraciones
- **L2CTLR/CA7_CACHE_CONFIG: REFUTADO** — instrumentado compare+restore post-resume
  (LK: L2CTLR=0x3800001, CACFG=0x60): **drift=0 en todos los ciclos** y murió igual (ciclo 6).
  El restore queda en el driver (inofensivo, defensa en profundidad).
- **musb/usb0: REFUTADO** — soak con musb desligado TODO el rato: murió en ciclo ~5.
- Patrón consolidado: muerte en ciclo 4-6, ~2s post-resume, silenciosa (heartbeats cortados).

## Exonerados (todos por experimento directo)
WiFi · daemons de polling · tail "normal" del PCM · alarma RTC · config L2/MCUSYS · musb/usb0

## Cola de hipótesis (próxima sesión)
1. **CONNSYS/BT**: el BTIF+STP+chip conn estuvo ACTIVO en todos los soaks (hci0 up, btif-rx thread).
   Test: soak con `mt6582_consys_func_off` de BT/FM o rmmod-equivalente (WMT off).
2. Integridad DRAM tras self-refresh repetido: memtester entre ciclos.
3. El 26M/CLKSQ settle acumulando (SPM_CLK_SETTLE re-cálculo cada enter).

## ✅ RAÍZ ENCONTRADA Y RESUELTA (0710): era el WATCHDOG, nunca un cuelgue
- mtk-wdt (TOPRGU) armado por el LK con timeout=31s; sin userspace que lo abra, lo rearma el
  [watchdogd] del kernel via hrtimer (CLOCK_MONOTONIC) → los sueños M3 CONGELAN el monotonic
  mientras el HW cuenta tiempo real → los pings se retrasan acumulativamente → reset a los ~4-6
  ciclos. Explica TODO: muerte súbita sin oops, en idle, exoneración de todos los subsistemas,
  DRAM íntegra.
- FIX: daemon watchdog en userspace (busybox `watchdog -T 30 -t 5 /dev/watchdog`, local.d
  01-watchdog.start) → watchdog_active=true → mtk_wdt_suspend PARA el HW en cada ciclo.
- VALIDADO: soak 12/12 ciclos M3 limpios, suspend_stats=12, sin reboot. **M3 ESTABLE.**

## 🏆 M4 VALIDADO (0710): SPM COMPLETO — M1+M2+M3+M4
Con el watchdog en userspace: **M4 (infra_pdn=1) funcionó A LA PRIMERA** y soak 5/5 limpio
(stats 18/0, mismo boot). Suspend profundo completo: cluster CPU fuera, INFRA/DDRPHY fuera,
DDR self-refresh, 26M cortado, resume BootROM→0x10001800→cpu_resume. Activación:
spm_cpu_pdn=1 + spm_infra_pdn=1 + wake por EINT (botón/RTC) o PCM_TIMER; musb rebind post-ciclo.
Pendiente de integración de producto: decidir cuándo el autosuspend usa mem-M4 vs freeze
(p.ej. tras 20 min = híbrido actual → M4), y validar pantalla/periféricos tras M4 en uso GUI real.
