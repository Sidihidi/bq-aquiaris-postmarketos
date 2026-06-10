# 🏆 M1 CONSEGUIDO — Linux 7.0.12 mainline en el BQ Aquaris E4.5 (2026-06-10)

Primer arranque documentado de un kernel mainline moderno en el BQ Aquaris E4.5
(MediaTek MT6582, 2014). Kernel compilado desde cero, sin driver de display
propio: consola en pantalla vía `simple-framebuffer` sobre el FB que deja el LK.

## Evidencia (dmesg leído de RAM vía ramoops — ver dmesg-mainline-v3.txt)

```
Linux version 7.0.12 (cpcd@raspberrypi) ... #1 SMP Wed Jun 10 2026
smp: Brought up 1 node, 4 CPUs
SMP: Total of 4 processors activated (8283.75 BogoMIPS).
Memory: 926872K/1048576K available
simple-framebuffer bf400000.framebuffer: fb0: simplefb registered!
Console: switching to colour frame buffer device 67x60
Run /init as init process
random: crng init done    <- userspace estable >4 min
```

**Logrado:** boot SMP 4×Cortex-A7, gestor de memoria (~1GB), framebuffer/consola,
initramfs, userspace (init propio imprimiendo "MAINLINE VIVO").

## Datos de hardware confirmados (para los siguientes hitos)
- FB: `0xBF400000`, 540×960×32, stride 2176, **formato real `a8r8g8b8`**
  (rojo@16, azul@0; con a8b8g8r8 sale amarillo — bug del color ya corregido en el dts)
- 4× Cortex-A7, mpidr 0x8000000N
- Hueco de RAM seguro para ramoops post-mortem: **`0xBF300000`** (sobrevive reboot)
- UART `11002000.serial` falla (-2, falta clock) → sin consola serie; pantalla+ramoops son el canal
- Watchdog `10007000.watchdog` (mtk-wdt) OK, timeout 31s
- El LK ignora el cmdline del boot.img → mainline necesita `CMDLINE_EXTEND`

## Qué falta para un sistema usable (roadmap)
- **M2 — eMMC**: añadir nodo `mediatek,mtk-msdc` (drivers/mmc/host/mtk-sd.c, EN mainline)
  con regs/clocks del MT6582 → montar root real desde mmcblk0p5/p7.
- **M2b — USB gadget**: el MT6582 usa MUSB con glue MediaTek; mainline no trae driver
  para este SoC → investigar port (o priorizar eMMC+pantalla+táctil para sistema autónomo).
- **M3 — DRM/Lima**: requiere driver de display DSI mainline (no existe para mt6582) —
  largo plazo; el premio (Phosh + GPU) está aquí.

## Cómo iterar (sin cables serie)
1. pmOS corriendo → `reboot2 bootloader` (software) → fastboot
2. La Pi flashea `boot-mainline-vN.img` por fastboot → reboot
3. Mirar pantalla. Combo Power+Vol+ (NO sacar batería) → fastboot → Pi restaura pmOS
4. Desde pmOS: `memdump 0xBF300000 0x100000` → dmesg completo del intento mainline
