# Cómo continuar el TOUCHSCREEN (handoff para retomar desde otra máquina)

Estado a 2026-06-17. Todo el trabajo se hace **en la Raspberry Pi** (tiene el
toolchain, el árbol del kernel y el puente USB al teléfono). Desde tu Mac solo
necesitas **SSH a la Pi**; no hace falta nada instalado en la Mac.

---

## 0) Mapa: dónde está cada cosa

- **Raspberry Pi** — `ssh <tu_usuario>@192.168.0.38` (tus credenciales de la Pi).
  - Kernel mainline: `~/mainline/linux-7.0.12/`, build dir **`build-krillin/`** (se
    compila con `O=build-krillin`). Toolchain `arm-linux-gnueabihf-` ya en el PATH.
  - Imágenes, herramientas y binarios: `~/mainline/pkg/`.
  - Llave SSH al teléfono: `~/.ssh/id_ed25519` (en la Pi).
- **Teléfono** (BQ E4.5, Alpine mainline en partición p7) — accesible **solo desde
  la Pi** por red USB: `ssh -i ~/.ssh/id_ed25519 root@172.16.42.1`.
  - Si no responde: en la Pi `sudo ip link set usb0 up; sudo ip addr replace 172.16.42.2/24 dev usb0`.
  - Herramientas en el teléfono (persistentes, /usr/local/bin): `ft5336_touch`,
    `pwrap_poke`, `devmem`. Servicio: `/etc/local.d/touch-power.start`.
- **GitHub** (este repo):
  - `mainline/drivers-touch/ft5336_touch.c` — driver userspace del touch.
  - `mainline/modules/uinput.ko` — módulo uinput compilado (vermagic `7.0.12 SMP`).
  - `mainline/rootfs/touch-power.start` — servicio de power del touch.
  - `mainline/HITO-TOUCH-DRIVER.md`, `HITO-TOUCH-POWER-SERVICE.md`, `HITO-I2C-TOUCH.md`.

---

## 1) Qué está HECHO ✅ y qué FALTA ⚠️

**Hecho:**
- Driver userspace `ft5336_touch.c`: lee el FT5336 por I2C0 y emite por uinput.
  Probado leyendo un arrastre continuo correcto `(85,575)→(235,320)`, ~50 Hz.
- Infra de input en el kernel: `INPUT_EVDEV` ya estaba; **`uinput` se cargó como
  módulo** (`insmod uinput.ko`) sin reflashear (el kernel tiene `CONFIG_MODULES=y`).
- Con uinput cargado y el daemon corriendo: `/dev/input/event0`, `Name="ft5336"`,
  `INPUT_PROP_DIRECT`; **`evtest` reconoce el dispositivo**.

**Falta (bloqueante):**
- **Wake/reset FIABLE del chip.** El sensado no arranca de forma determinista con
  power/reset manual. Causa raíz: el **reset por GPIO115 no es fiable** (los
  `devmem 0x10005474/0x10005478` dan readbacks `0x5`/`0xd` → no controlan bien el
  pin). Solo un **reboot** del teléfono devuelve el chip a estado bueno.

---

## 2) Reactivar el touch (con el chip en buen estado, p.ej. recién reiniciado)

Desde la Pi (el `uinput.ko` se pierde en cada reboot del teléfono, hay que recargarlo):
```sh
ssh -i ~/.ssh/id_ed25519 root@172.16.42.1
# en el teléfono:
[ -e /dev/uinput ] || insmod /root/uinput.ko   # si no está, scp uinput.ko -> /root/
/etc/local.d/touch-power.start                  # power del touch (ya corre al boot)
pkill ft5336_touch; nohup /usr/local/bin/ft5336_touch >/tmp/ft.log 2>&1 &
ls -l /dev/input/event0
evtest /dev/input/event0                         # y tocar la pantalla
# diagnóstico crudo (imprime lo que lee el I2C):
/usr/local/bin/ft5336_touch --raw                # 0xff=dormido, down=1 + coords=toque real
```

## 3) Recompilar (en la Pi)
```sh
# daemon userspace:
cd ~/mainline/pkg && arm-linux-gnueabihf-gcc -O2 -static -o ft5336_touch ft5336_touch.c
# kernel / módulos:
cd ~/mainline/linux-7.0.12
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- O=build-krillin -j4 zImage dtbs
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- O=build-krillin modules     # .ko
# boot.img (el assemble.sh con `abootimg --create` falla rc=22; usar -u):
cat build-krillin/arch/arm/boot/zImage build-krillin/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dtb > zimage-dtb
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk
cp boot-mainline-i2c.img boot-nuevo.img && abootimg -u boot-nuevo.img -k zimage-dtb-mtk
# flashear por dd DESDE el teléfono (sin fastboot): flash_boot_dd_v2.sh <img>
#   ⚠️ transferir imgs grandes (13 MB) por usb0 REINICIA el teléfono (el musb no
#   aguanta) -> usar fastboot, o trocear el scp en piezas pequeñas con pausas.
```

## 4) Datos técnicos clave (referencia rápida)
- FT5336 en **I2C0 @0x38**, pantalla **540×960**, **INT=EINT117**, **reset=GPIO115**.
- **I2C lee fiable ≤8 bytes** (FIFO=8; >8 necesita apdma → timeout). El driver lee
  7 bytes = status + toque1 = single-touch.
- Detección de dedo por **event-flag** (XH bits 7-6: 0=down, 2=contact; 1=up, 3=none).
  `X=(XH&0x0f)<<8|XL`, `Y=(YH&0x0f)<<8|YL`. td_status (reg 0x02) es inestable.
- Power **VGP1**: `pwrap_poke w 0x050A 0x8000` (PMIC MT6323 DIGLDO_CON7 bit15).
- ⚠️ Reset GPIO115: `devmem 0x10005478`/`0x10005474` → readbacks raros (0x5/0xd),
  no fiable. **Identificar bien estos registros es la clave del wake.**

---

## 5) Las TRES opciones para el siguiente paso

### Opción A — Reset GPIO fiable  ⭐ (recomendada, camino más corto)
Identificar correctamente **GPIO115** en el GPIO del MT6582 (base `0x10005000`):
los registros DIR/DOUT/MODE y sus variantes SET/CLR, y la polaridad del reset.
Con un reset determinista, el **daemon actual ya queda usable** (single-touch) sin
más cambios. Es además prerrequisito de la opción B.
- Pistas: leer/mapear el área `0x10005000+` con `devmem`/`memdump`; comparar con
  `pinctrl-mtk-common`/datasheet MT6582; banco de GPIO115 = 115/16=7, bit=115%16=3.
- Validar: tras el reset correcto, en reposo los regs de toque NO deben quedar en
  `0xff` permanente; al tocar deben dar `evt 0/2` con coords.

### Opción B — Driver nativo `edt-ft5x06` + Device Tree (solución definitiva)
`CONFIG_TOUCHSCREEN_EDT_FT5X06` ya está =y. Añadir el nodo en el DT:
```dts
&i2c0 {
    touchscreen@38 {
        compatible = "edt,edt-ft5306";  /* familia FT5x06 */
        reg = <0x38>;
        interrupt-parent = <&eint>; interrupts = <117 IRQ_TYPE_EDGE_FALLING>;
        reset-gpios = <&pio 115 GPIO_ACTIVE_LOW>;
        vin-supply = <&vgp1>;
        touchscreen-size-x = <540>; touchscreen-size-y = <960>;
    };
};
```
Requiere ANTES: (1) un controlador GPIO/pinctrl + EINT para MT6582 que provea
`&pio`/`&eint` (hoy `mt6582.dtsi` no los tiene); (2) **arreglar el apdma del
`i2c-mt65xx`** para lecturas >8 bytes (el edt-ft5x06 lee 33 B de golpe). Da
multitouch real y reset+INT gestionados por el kernel. Más trabajo; engloba A.

### Opción C — Pausar el touch y avanzar con otro frente
La infra de input queda lista y retomable. Frentes alternativos: GUI sobre el
framebuffer (X11+fbdev; Wayland/Sxmo necesitaría driver DRM, que no existe en
MT6582 mainline), batería/carga (PMIC, pwrap ya validado), WiFi, audio, módem.

---
*Generado durante la sesión del 2026-06-17. Driver, módulo e hitos están en este
repo; los detalles completos en `mainline/HITO-TOUCH-DRIVER.md`.*
