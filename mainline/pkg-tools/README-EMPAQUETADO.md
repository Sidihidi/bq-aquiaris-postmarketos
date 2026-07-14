# Herramientas de empaquetado del boot (git-backed tras perderlas 0714)

> ⚠️ El 0714 se perdió `~/mainline/pkg/` de la Pi (percance) → sin estas herramientas no se puede
> generar NI recuperar ninguna imagen de boot. Ahora viven en git para que no vuelva a pasar.
> Se recuperaron extrayéndolas de la imagen de boot que corría en el móvil (`abootimg -x` de la
> partición boot leída con `dd skip=83968`), + recreando `mtk_hdr.py` del formato de cabecera.

## Ficheros
- **`mtk_hdr.py`** — antepone la cabecera MTK de 512 bytes al kernel. Formato (verificado byte-idéntico):
  magic `88 16 88 58` + `size(LE u32 del payload)` + nombre padded a 32 con nulls + `0xff` hasta 512.
  Uso: `python3 mtk_hdr.py KERNEL <zImage+dtb> <salida-mtk>`.
- **`bootimg.cfg`** — config de `abootimg` (bootsize 0x1400000, pagesize 0x800, kerneladdr 0x10008000,
  ramdiskaddr 0x11000000, cmdline `console=tty0 clk_ignore_unused quiet`). (= el viejo `bootimg-mp7.cfg`.)
- **`initrd-menupick-mtk`** — el initrd del MULTIBOOT (menú pmOS/Android/Maemo). Binario (1.8MB). Su
  FUENTE está en `mainline/bootmenu/` (`menupick.c`, `init-menupick.sh`, `font5x7.h`).
- **`ganador-h2b.dtb`** — DTB BUENO (arranca) + el carveout del módem `modem-region@b8000000`. **USAR
  ESTE**, no el dtb del build-dir (que la sesión Mac rompe en el DTS compartido). Base `ganador.dtb`
  (sin carveout) = decompilar este y quitar el nodo, o extraer de una imagen que no lo tenga.

## Receta de empaquetado (reconstruir cualquier imagen)
```bash
cd ~/mainline/pkg    # copiar aquí estos 4 ficheros si se pierde pkg de nuevo
K=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot
cat $K/zImage ganador-h2b.dtb > zimage-dtb
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk
abootimg --create boot-nueva.img -f bootimg.cfg -k zimage-dtb-mtk -r initrd-menupick-mtk
# flashear: stage dd desde pmOS (seek=83968) o fastboot flash boot
```

## Recuperar `pkg` si se vuelve a perder
1. Leer la imagen de boot del móvil: `ssh root@movil 'dd if=/dev/mmcblk0 bs=512 skip=83968 count=40960'` → `.img`.
2. `abootimg -x <img>` → recupera `zImage` (con cabecera MTK), `initrd.img` (=initrd-menupick-mtk), `bootimg.cfg`.
3. El dtb: buscar el magic `d0 0d fe ed` con `totalsize` (u32 BE en +4) de ~12-13KB en el `zImage` extraído.
4. O `git checkout` de estos ficheros (¡por eso están aquí!).

*Recuperado y versionado 2026-07-14.*
