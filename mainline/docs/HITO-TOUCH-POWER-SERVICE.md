# HITO — Power del touchscreen automático al arranque (servicio OpenRC)

**Estado: RESUELTO y verificado tras reboot (2026-06-16).**

El touchscreen **FT5336** (I2C0 @0x38) necesita que se encienda su regulador de
alimentación **VGP1** (LDO del PMIC **MT6323**, vía el **pwrap** del MT6582) antes
de poder hablarle. En mainline todavía no existe el driver del regulador
(mt6323) ni el del touch (edt-ft5x06) integrados en el Device Tree, así que el
power se enciende desde **userspace** en cada arranque.

## Por qué userspace y no kernel

Probamos encender VGP1 con un `late_initcall` en el kernel
(`mt6582-pmic-fixup.c`): el busy-wait del pwrap durante el arranque **rompía el
boot de sshd** (Alpine arrancaba, `usb0` subía, pero OpenRC se quedaba colgado y
`sshd` nunca arrancaba). Revertido. **Lección: nunca encender VGP1 con un
busy-wait en late_initcall.** La forma segura es hacerlo en userspace **después**
de que la red esté arriba — o, en el futuro, con el driver pwrap+mt6323+regulator
real en el DT (entonces `vin-supply` del nodo del touch lo encenderá solo).

## El servicio

`/etc/local.d/touch-power.start` (servicio **`local`** de OpenRC, ya en runlevel
`default`):

```sh
#!/bin/sh
/usr/local/bin/pwrap_poke w 0x050A 0x8000   # MT6323 DIGLDO_CON7 bit15 = VGP1 EN
/usr/local/bin/devmem 0x10005478 0x8         # GPIO115 (reset) LOW
sleep 1
/usr/local/bin/devmem 0x10005474 0x8         # GPIO115 (reset) HIGH
logger -t touch-power "VGP1 ON + reset touch FT5336"
```

Herramientas (persistentes en el rootfs p7): `/usr/local/bin/pwrap_poke` y
`/usr/local/bin/devmem`.

Instalación:
```sh
rc-update add local default      # habilita el servicio "local" que corre los .start
```

## Verificación (tras `reboot`, sin fastboot y SIN poke manual)

```
i2cdetect -y -r 0   ->  0x38 presente     (el servicio encendió VGP1 solo)
i2ctransfer 0xA8    ->  0x5a  (vendor Truly)
i2ctransfer 0xA3    ->  0x14  (chip id FT5336)
uptime              ->  0 min (recién arrancado)   sshd OK a los ~20s
```

El touch tiene alimentación automáticamente en **cada arranque/reinicio**, sin
intervención y sin afectar a `sshd`.

## Siguiente paso (driver de verdad)

- Nodo `edt-ft5x06` en el DT: `reg = <0x38>`, `vin-supply = <&vgp1>`,
  `touchscreen-size-x/y = 540/960`, IRQ por **EINT117**.
- Regulador `mt6323` + `pwrap` MT6582 en el DT → enciende VGP1 por `vin-supply`.
- IRQ: portar el controlador EINT del MT6582 o usar polling → eventos
  `/dev/input`. Entonces este servicio se retira.

Ver también: `HITO-I2C-TOUCH.md`, `HITO-ROOTFS-ALPINE.md`, `ESTADO-MAINLINE.md`.
