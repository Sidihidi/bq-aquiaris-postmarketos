# Patchset krillin sobre Linux vanilla — captura completa (2026-07-13)

> **TODO el delta de kernel del proyecto, versionado.** Hasta hoy el árbol vivía solo en la Pi de
> build (sin git); esto lo captura y convierte cualquier migración de versión en un proceso mecánico.
> Capturado del árbol vivo de la Pi (`~/mainline/linux-7.0.12`, el que compila los boot-menupick*).

## Contenido
| Fichero | Qué es |
|---|---|
| `krillin-over-7.0.12.patch` | **Diff completo sobre vanilla 7.0.12** (101 ficheros, ~1MB, formato `a/ b/` → `git apply -p1` o `patch -p1`). Incluye TODOS los drivers propios y modificaciones EXCEPTO el driver WiFi (aparte, abajo). |
| `mtk_mtwifi/` | **Driver WiFi completo** (port del mt_wifi stock, 141 ficheros fuente). Drop-in: copiar a `drivers/net/wireless/mtk_mtwifi/`. Su hook en Kconfig/Makefile del directorio padre SÍ va en el patch. |
| `krillin-7.0.12.config` | `.config` completo del build (con `CONFIG_CMDLINE` horneado — ver nota abajo). |
| `krillin-config-symbols.txt` | Los símbolos clave nuestros/MTK activados (resumen legible). |

## Qué contiene el patch (inventario por subsistema)
- **DTS**: `arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts` (el device tree entero del krillin).
- **Drivers propios (ficheros nuevos)**: `drivers/soc/mediatek/mt6582-{spm,btif,consys,wifi}.c` (+`-wifi-reg.h`),
  `drivers/misc/mediatek/fmradio/` (FM completo, ~30 ficheros), `sound/soc/mediatek/mt6582/` (audio AFE),
  `drivers/gpu/drm/panel/panel-himax-hx8389.c` (con el fix bring-up-una-vez del 0711),
  `drivers/usb/musb/mt6582-musb.c`, `drivers/iio/magnetometer/mmc3516x.c`,
  `drivers/input/misc/mt6323-accdet.c`.
- **Vanilla tocado**: `drivers/gpu/drm/mediatek/{mtk_crtc,mtk_dsi,mtk_disp_rdma}.c`, `mtk-mmsys.c`,
  `drivers/mfd/mt6397-core.c` (celda accdet), `mtk-pmic-keys.c`, `pwm-mtk-disp.c` (fix EN-bit backlight),
  `drivers/thermal/mediatek/auxadc_thermal.c` (MT6582), más Kconfig/Makefile de cada subsistema.
- **SPM**: la versión de la Pi del **0713** (la del soak M3 resuelto; 608 líneas) — también
  sincronizada a `mainline/drivers/done/spm/mt6582-spm.c`.

## Reconstruir un árbol de build desde cero
```bash
wget https://cdn.kernel.org/pub/linux/kernel/v7.x/linux-7.0.12.tar.xz && tar xJf linux-7.0.12.tar.xz
cd linux-7.0.12
git init && git add -A && git commit -m vanilla     # recomendado: protege el árbol
patch -p1 < .../krillin-over-7.0.12.patch
cp -r .../mtk_mtwifi drivers/net/wireless/
mkdir -p build-krillin && cp .../krillin-7.0.12.config build-krillin/.config
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- olddefconfig zImage dtbs -j$(nproc)
# empaquetado: mainline/pkg/assemble.sh (cat zImage+dtb → mtk_hdr.py → abootimg)
```
⚠️ `CONFIG_CMDLINE` va horneado en la config (`no_console_suspend mt6582_spm.*`...) — revisar al migrar.

## Decisión de versión de kernel (0713, verificado en kernel.org)
- **7.0.x = EOL** (final 7.0.14). **7.1.3 = stable actual pero NO LTS** (muere cuando salga 7.2, ya en rc3).
- LTS vigentes: 6.18/6.12/6.6 (hacia atrás — no aplican). **La próxima LTS se designa a fin de año.**
- **DECISIÓN: seguir en 7.0.12 durante la campaña de drivers (módem M1, GPS) y migrar a la PRÓXIMA
  LTS en la fase de estabilización** — un solo salto con una sola campaña de validación HW, en vez
  de perseguir stables que caducan en meses. Riesgo aceptable: móvil no expuesto; nuestros bugs
  están en nuestros drivers. CVEs críticas → cherry-pick de 7.1.x.

## Playbook de migración (cuando toque)
1. Bajar la LTS nueva + `git init` + aplicar este patchset (`git apply --reject`).
2. Los `.rej` se concentrarán en los ~11 ficheros vanilla tocados (DRM mtk, mfd, thermal, pwm, keys)
   — resolver a mano mirando qué cambió upstream. Los drivers propios (ficheros nuevos) entran limpios
   salvo cambios de API (sondas: `platform_driver`, `drm_panel_funcs`, ASoC, IIO).
3. `olddefconfig` con nuestra config → revisar símbolos renombrados (`krillin-config-symbols.txt`).
4. Compilar + campaña de validación HW por subsistema: boot/pantalla/táctil → WiFi → audio → BT/FM →
   sensores → suspend (SPM) → dual-boot Maemo.
5. Regenerar este patchset contra la versión nueva y actualizar este README.

## Mantenimiento
**Este patchset es una FOTO (0713).** Cada cambio nuevo en el árbol de la Pi debe: (a) re-generarse
(receta abajo) o (b) al menos copiar el fichero tocado al repo como hasta ahora. Receta de regeneración
en la Pi: descargar+extraer vanilla en `~/patchset-work/vanilla/`, `diff -ruN` excluyendo
`build-*`/objetos/`mtk_mtwifi`, normalizar cabeceras a `a/ b/` (ver historial de esta sesión), y OJO
con el disco (~1.5GB para el vanilla; el system.img stock se puede gzip temporalmente).
**Mejor aún (pendiente): `git init` en el árbol de la Pi** y commitear allí; este dir quedaría como
espejo de exportación.

*Sesión Mac (Fable 5), 2026-07-13.*
