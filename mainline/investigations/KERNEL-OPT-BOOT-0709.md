# Kernel: optimización + secuencia de arranque — investigación 0709 (sesión paralela)

> Misión: "cómo optimizar el kernel (eficiencia/velocidad/compatibilidad) y tener la mejor secuencia
> de arranque para evitar los errores de booteo". Basado en `build-krillin/.config` real, `pkg/`
> (bootimgs+initrds+assemble.sh) y el historial de bugs de boot. **Nada aplicado — solo análisis +
> receta**; aplica la sesión principal en su ciclo de flasheo.

## 1 ★ MISTERIO RESUELTO: el `spm_cpu_pdn=1` fantasma
`ESTADO-PROYECTO-0709` §2: *"algo pone spm_cpu_pdn=1 al boot (no es cmdline ni bootimg ni local.d…
¿modprobe.d? ¿initramfs?)"*. **Es el propio kernel** — `build-krillin/.config` línea 712:
```
CONFIG_CMDLINE="no_console_suspend mt6582_spm.spm_cpu_pdn=1 mt6582_spm.spm_wake_sec=15"
CONFIG_CMDLINE_EXTEND=y
```
Se horneó para el soak del #269 (método del HANDOFF-SPM) y sigue en todos los kernels posteriores.
`CMDLINE_EXTEND` lo CONCATENA al cmdline del bootimg → aparece aunque el `.cfg` esté limpio.
(Verificado: ningún initrd de `pkg/` contiene `spm_cpu_pdn`; ningún `bootimg-*.cfg` lo lleva.)
**Fix**: al cerrar el debug SPM, dejar `CONFIG_CMDLINE="no_console_suspend"` (o vacío) y
reconstruir. Mientras tanto `zzz-spm-safe.start` ya lo neutraliza — pero conviene quitar la raíz
para que un initramfs/rootfs viejo no arranque con deep-sleep armado por sorpresa.

## 2. Arranque MÁS RÁPIDO (medible, bajo riesgo)
1. **cmdline: quitar `ignore_loglevel`** del bootimg estándar (assemble.sh lo fija:
   `console=tty0 clk_ignore_unused ignore_loglevel`). Con consola en framebuffer, CADA printk pinta
   en pantalla ⇒ segundos de boot. Usar `quiet loglevel=3` (como ya hace `bootimg-maemo-debug.cfg`);
   el dmesg completo sigue en el buffer y en pstore-console. Mantener `no_console_suspend` SOLO
   mientras dure el debug SPM (también serializa la consola en suspend).
2. **zImage LZ4** (`CONFIG_KERNEL_LZ4=y`; hoy GZIP): en un A7 la descompresión LZ4 es ~3-4× más
   rápida (~1s menos de pantalla negra). Coste: imagen ~+20% (13→~16 MB; boot.img ~17-18 MB, cabe en
   los 20 MB — `assemble.sh` ya avisa si no cabe). Si algún día no cabe: volver a GZIP o partir dtb.
3. **`CONFIG_DEBUG_INFO=y` → quitar** para builds diarios: no cambia el runtime pero compila
   bastante más rápido y el vmlinux pasa de cientos de MB a decenas (la Pi está al 89% de disco).
   Reactivar solo cuando haga falta depurar con símbolos (ramoops ya da direcciones + kallsyms).
4. Higiene del árbol: los ~15 `.bak-*` de wifi en `drivers/soc/mediatek/` fuera del árbol (no
   afectan al build pero ensucian greps y ocupan; ya hay copia en GitHub).

## 3. Memoria: **ZRAM** (el mayor "smoothness win" pendiente)
`# CONFIG_ZRAM is not set` — Phosh con 1 GB corre HOY sin swap comprimido (el roadmap original lo
marcaba obligatorio). Receta:
```
CONFIG_ZRAM=y  CONFIG_ZSMALLOC=y  CONFIG_CRYPTO_LZO=y   (lzo-rle default; LZ4 opcional)
```
\+ userspace: `zram-init` de Alpine o 6 líneas en local.d (`modprobe`-less al ser =y:
`echo lzo-rle > /sys/block/zram0/comp_algorithm; echo 512M > disksize; mkswap+swapon -p100`).
Efecto: menos OOM/thrashing con 2-3 apps GTK, sesión más fluida. Riesgo ~0 (si no se activa, no hace nada).

## 4. Secuencia de arranque robusta (evitar los errores de booteo)
**Raíces YA identificadas y su estado** (histórico + hoy):
| Causa raíz | Estado |
|---|---|
| Orden mmcblk0/1 **no determinista** (mtk-sd) | ✅ atacada por `initrd-robust` (escanea particiones + SSH de emergencia). Extender: montar por **PARTUUID/label** también en fstab del rootfs |
| Entropía (crng 236s) | ✅ seedrng+haveged (06-21) |
| Carrera bring-up CONSYS (3 disparadores) | ✅ `bringup_lock` + zz-consys async (06-22) |
| sshd flaky post-crash SPM | 🟡 mitigado con retry; se irá con el fix del BUG_ON (#273) |
| `init-menupick` (multiboot) nuevo actor en la cadena | ⬜ auditar (plan ESTADO §en-curso 2) |
| local.d = **orden alfabético**, sin dependencias reales | ⬜ propuesta abajo |

**Propuestas concretas:**
1. **Servicios OpenRC reales para lo crítico** en vez de `local.d zz*` encadenados por alfabeto:
   un `.initd` por daemon (powerkey, backlight, battery, consys) con `depend() { need localmount; after modules; }`
   → arranque paralelo donde se puede, orden garantizado donde importa, `rc-status` los supervisa y
   `respawn` (supervise-daemon) los reintenta — se acaba el patrón "bucle de reintento a mano".
2. **Watchdog en la cadena**: el MTK WDT ya funciona — armar `supervise-daemon`/openrc-watchdog
   temprano de forma que un cuelgue de runlevel `boot` (el sospechoso udev-settle de junio) haga
   reset limpio a los N s en vez de "power-cycle a mano". (En initramfs-emergencia ya se toca
   `/dev/watchdog` — mismo patrón.)
3. **Medir antes/después** (el plan del ESTADO pedía "medir N boots con timestamps"):
   - una vez: cmdline `initcall_debug` + `dmesg | grep -E 'initcall.*returned' | sort -k4` → top de
     probes lentos del kernel (candidatos a `=m` o deferred).
   - `rc_log_path=/dev/kmsg` (ya puesto) + `awk` de timestamps → top de servicios lentos de OpenRC.
   - objetivo de referencia: LK→login < 20 s estable (hoy 14-19 s en los boots buenos).
4. **Compatibilidad**: `CONFIG_PREEMPT` (full) es la única palanca de config con efecto perceptible
   en latencia táctil/UI en un A7; coste ~2-5% throughput. Probar UNA build A/B y decidir con el
   dedo, no con benchmarks.

## 5. Qué NO tocar
- `PSTORE ram+console` (la caja negra del debug SPM) y `no_console_suspend` hasta cerrar SPM.
- `clk_ignore_unused` (obvio — sin CCF real, apagar clocks "sin dueño" mata la plataforma).
- El patrón fixed-clocks + LK-deja-encendido: es la base de todo el port.
- `initrd-robust`: ya es la pieza buena de la cadena; construir sobre él, no sustituirlo.

## 6. Orden de aplicación sugerido (1 flash cada uno, con A/B de tiempos)
1. Limpiar `CONFIG_CMDLINE` (§1) + quitar `ignore_loglevel` (§2.1) — mismo build.
2. \+ ZRAM (§3) — mismo build que 1 si se quiere (riesgo ~0).
3. \+ LZ4 (§2.2) — build aparte para medir su segundo.
4. Migración local.d→OpenRC por fases (empezar por consys+powerkey) + watchdog (§4).
5. (Opcional) A/B de PREEMPT.

*Sesión paralela drivers (Fable 5), 2026-07-09 noche. Todo es receta — nada flasheado.*
