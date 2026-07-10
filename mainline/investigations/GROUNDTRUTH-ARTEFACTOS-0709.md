# Ground truth de drivers — verificación por ARTEFACTOS, no por docs (0709 noche)

> Misión: "verificar de forma real qué drivers están ya" (tras el susto del FM audio, que los docs
> daban por pendiente y llevaba resuelto desde 0708). Método: **la verdad es el árbol de la Pi**
> (`~/mainline/linux-7.0.12`, que produce el kernel flasheado) + `build-krillin/.config` + `pkg/`.
> Complementa (no repite) a `ESTADO-PROYECTO-0709.md` §1 y `VERIFICACION-DRIVERS-PARALELA-0709.md`.

## 1. Matriz artefacto-verificada (kernel que se flashea HOY)

| Subsistema | Artefacto verificado en el árbol | Estado |
|---|---|---|
| Audio AFE + **FM audio** | `sound/soc/mediatek/mt6582/mt6582-afe-pcm.c` + `-regs.h` (**mtime Jul 8 20:19** = el fix del FM: RESTORE_SEARCH=RampDown + CONN1 b16/CONN2 b1) | ✅ IN-TREE, in-kernel |
| SPM (suspend profundo) | `drivers/soc/mediatek/mt6582-spm.c` (mtime **Jul 10 01:52** — en edición AHORA por la sesión SPM) + `arch/arm/mach-mediatek/platsmp.c` (Jul 9 18:07) | ✅ IN-TREE, iterándose |
| WiFi | `drivers/soc/mediatek/mt6582-wifi.c` + `-reg.h` | ✅ IN-TREE |
| BT/CONSYS/FM-core | `drivers/soc/mediatek/mt6582-btif.c` | ✅ IN-TREE |
| EINT | `drivers/pinctrl/mediatek/gpio-mt6582-eint.c` | ✅ IN-TREE |
| GPU power | `drivers/soc/mediatek/mt6582-mfg-power.c` | ✅ IN-TREE |
| Vibrador | `drivers/soc/mediatek/mt6582-vibrator.c` | ✅ IN-TREE |
| Display fix legado | `drivers/video/fbdev/mt6582-dispfix.c` (retirado del uso: DRM nativo) | 📦 presente, inactivo |
| Batería | mt6323-auxadc (hwmon) | ✅ (repo `mainline/drivers/`) |
| **Mag / Thermal / Accdet (sesión paralela)** | `~/drivers-wip/` (build-drv) + repo `mainline/drivers/wip/` | 🧪 compilan, **NO integrados aún** en build-krillin |
| GPS runner Fase A | `~/gps/fase-a/mnlp_static` (binario) | 🧪 enlaza, sin probar |

Config flasheado (extractos relevantes): PSTORE ram+console ✅ · NO_HZ_IDLE+HIGH_RES ✅ ·
PREEMPT_VOLUNTARY · HZ=100 · CC_OPTIMIZE_FOR_PERFORMANCE ✅ · KERNEL_GZIP · **ZRAM no set** ·
DEBUG_INFO=y · **CONFIG_CMDLINE horneado con `spm_cpu_pdn=1`** (ver KERNEL-OPT-BOOT-0709 §1 ★).

## 2. Hallazgos colaterales del ground truth
1. **El árbol de la Pi NO es un repo git** (`fatal: not a git repository`). Toda la historia vive en
   copias del repo GitHub + ~15 ficheros `.bak-*` de wifi/btif dentro de `drivers/soc/mediatek/`.
   Riesgo: un `rm`/corrupción pierde estado no versionado. Recomendación: `git init` + commit inicial
   en el árbol (barato, no cambia el build) o snapshot tar periódico; y mover los `.bak` a un dir
   fuera del árbol (higiene; no afectan al build).
2. **FM audio confirmado in-tree** — la lección operativa ya está codificada en ESTADO-PROYECTO regla
   #3 (leer ESTADO+HITOS antes de trabajar un subsistema). Este doc añade la herramienta: *ante la
   duda, `ls -lat` del subsistema en el árbol de la Pi* — el mtime no miente.
3. Los boot.img se ensamblan con `pkg/assemble.sh` (zImage+dtb → mtk_hdr → abootimg) y se flashean
   con `flash_boot_dd_v2.sh` (verificación md5 completa ✅). Partición bootimg = 20 MB; imagen actual
   14.7 MB (73%).

*Sesión paralela drivers (Fable 5), 2026-07-09 noche.*
