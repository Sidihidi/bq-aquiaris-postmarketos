# Kernel mainline en el BQ Aquaris E4.5 (krillin) — guía completa

Cómo compilar, flashear, recuperar y depurar el kernel mainline (Linux 7.x) en
el BQ Aquaris E4.5 (MediaTek MT6582). Estado: **M1 conseguido** (arranque SMP
4 CPUs + consola simplefb + userspace). Ver [HITO-M1.md](HITO-M1.md).

Todo el flujo corre en una **Raspberry Pi** (Debian, aarch64) con el teléfono
por USB. El teléfono lleva pmOS (kernel 3.10 downstream) como sistema estable.

## Requisitos en la Pi

```sh
sudo apt install gcc-arm-linux-gnueabihf bc bison flex libssl-dev \
                 fastboot abootimg sshpass
```

## 1. Compilar el kernel mainline

```sh
cd ~/mainline
wget https://cdn.kernel.org/pub/linux/kernel/v7.x/linux-7.0.12.tar.xz
tar xJf linux-7.0.12.tar.xz && cd linux-7.0.12

# dts del krillin: copiar dts/mt6582-bq-krillin.dts (este repo) a
#   arch/arm/boot/dts/mediatek/  y añadirlo al Makefile de ese directorio
# config: usar config-mainline-krillin (este repo) como build-krillin/.config
#   o regenerar: make multi_v7_defconfig + las opciones de abajo

make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- olddefconfig
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage dtbs
```

Opciones clave sobre `multi_v7_defconfig` (ya en `config-mainline-krillin`):
`ARM_APPENDED_DTB`, `ARM_ATAG_DTB_COMPAT` + **`CMDLINE_EXTEND`** (¡el LK ignora
el cmdline del boot.img — solo los bootargs del DT + ATAGs del LK llegan!),
`FB_SIMPLE`, `FRAMEBUFFER_CONSOLE`, `LOGO`, `PSTORE_RAM`, `MMC_MTK`.

Parche aplicado: `mtk-sd` con pinctrl opcional (no hay driver de pines mt6582;
el LK ya configura los pines del eMMC). Ver pkg/ o el árbol parcheado en la Pi.

## 2. Empaquetar el boot.img (formato MTK)

```sh
cd pkg/   # las herramientas de este repo
./assemble.sh   # = cat zImage+dtb → cabecera MTK "KERNEL" (mtk_hdr.py)
                #   + initramfs mínimo (tiny_init) con cabecera "ROOTFS"
                #   + abootimg --create con offsets MTK (bootimg.cfg)
```

Datos MTK críticos (verificados): magic cabecera `0x58881688`, labels
`KERNEL`/`ROOTFS`, pagesize 2048, kerneladdr 0x10008000, ramdiskaddr 0x11000000.
La partición `bootimg` del teléfono = **20 MB** en `mmcblk0` offset `0x1D80000`
(sector 60416).

## 3. Flashear y probar (ciclo de iteración)

```sh
# A) desde pmOS corriendo (sin botones): subir imagen + flash por dd
sshpass -p 147147 ssh user@172.16.42.1 "cat > /tmp/boot.img" < boot-mainline-vN.img
sshpass -p 147147 ssh user@172.16.42.1   # dentro:
  sudo /tmp/flash_boot_dd.sh /tmp/boot.img   # verifica magic ANDROID! antes de escribir
  sudo reboot -f
# B) mirar la pantalla del teléfono (pingüinos + mensajes del init)
```

Notas:
- `fastboot boot` (arrancar desde RAM) NO funciona en este LK (acepta y no arranca).
- Entrada a fastboot por software desde pmOS: `/tmp/reboot2 bootloader` (pkg/reboot2.c).

## 4. Volver al kernel viejo (pmOS) — SIEMPRE recuperable

```sh
# en el teléfono (mainline corriendo): combo Power+Vol+ mantenido (~10 s)
#   ⚠️ NO sacar la batería si quieres conservar los logs en RAM
# sigue sujetando Vol+ → entra en fastboot. En la Pi:
sudo fastboot flash boot ~/pmos-artifacts/boot-pstore.img   # pmOS 3.10 (con /dev/mem)
sudo fastboot reboot
```

(`boot-pstore.img` = pmOS con CONFIG_DEVMEM; regenerable: ver CONTINUAR_AQUI.md)

## 5. Leer los logs del intento mainline (autopsia ramoops)

El DT reserva **`0xBF300000` (1 MB)** para ramoops — hueco verificado que no
toca ningún kernel y **sobrevive a reboots calientes y al combo Power+Vol+**.

```sh
# con pmOS de vuelta:
sshpass -p 147147 ssh user@172.16.42.1 "cat > /tmp/memdump" < pkg/memdump   # (compilar memdump.c)
ssh ... 'sudo /tmp/memdump 0xBF300000 0x100000 > /tmp/ramoops.bin'
scp el .bin y: strings -n 5 ramoops.bin | grep -E "^\[ *[0-9]" > dmesg-mainline.txt
```

⚠️ `read()` sobre `/dev/mem` falla en highmem ("Bad address") — `memdump` usa
`mmap()`, que sí funciona.

## Datos de hardware (extraídos del downstream y verificados)

| Qué | Valor |
|---|---|
| Framebuffer físico | `0xBF400000`, 540×960×32, stride 2176 |
| Formato fb (mtkfb) | `a8r8g8b8` (R@16, B@0) — el tinte amarillo en mainline NO es el formato: es la config gamma/color del pipeline que deja el LK (pendiente M3) |
| RAM | 2 bancos: 0x80000000-0xBBFFFFFF + 0xBD800000-0xBF2FFFFF |
| Hueco ramoops | `0xBF300000` (1 MB) |
| MSDC0 (eMMC) | regs `0x11230000`, IRQ GIC_SPI 39 |
| UART | `0x11002000` (ttyMT0 921600) — sin clock driver mainline, probe falla |
| Watchdog | `0x10007000` mtk-wdt — FUNCIONA en mainline |
| Particiones | bootimg=20MB@0x1D80000; root pmOS=mmcblk0p5; usrdata 5.7GB=p7 |

## Estado / hitos

- [x] **M1**: arranque + consola simplefb + userspace (4 CPUs SMP) — 2026-06-10
- [ ] **M2**: eMMC con mtk-sd (`mt8135-mmc` + parche pinctrl) — v5 en pruebas
- [ ] M2b: USB gadget (MUSB mt6582 — sin driver mainline, habría que portarlo)
- [ ] M3: driver display DSI/DRM → colores correctos, Lima/GPU → Phosh
