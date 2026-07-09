# HANDOFF — Touch kernel edt-ft5x06 FUNCIONA + brillo restaurado (2026-07-02)

## TL;DR

- **Touch kernel edt-ft5x06**: ✅ **FUNCIONA** con patch chunked_read (multitouch 5 dedos, 0 CPU en reposo)
- **Brillo**: ✅ **restaurado** (gsd-power integrado en phosh-session.sh)
- **Sin daemon userspace ft5336** (desactivado, driver kernel nativo)

---

## Qué se hizo

### 1. Driver kernel edt-ft5x06 con patch chunked_read

**DTS** (`mt6582-bq-krillin.dts`): añadido nodo del touch al I2C0:
```dts
touchscreen@38 {
    compatible = "edt,edt-ft5x06";
    reg = <0x38>;
    interrupt-parent = <&eint>;
    interrupts = <117 IRQ_TYPE_EDGE_FALLING>;
    reset-gpios = <&eint 115 GPIO_ACTIVE_LOW>;
    vcc-supply = <&mt6323_vgp1_reg>;
    touchscreen-size-x = <540>;
    touchscreen-size-y = <960>;
};
```

**Patch edt-ft5x06.c**: la función `edt_ft5x06_ts_isr()` original hace un `regmap_bulk_read`
de 33 bytes (para 5 touch points). El I2C del MT6582 solo soporta lecturas fiables ≤8 bytes
(FIFO limit). El patch añade `edt_ft5x06_chunked_read()` que divide la lectura en bloques de
8 bytes:

```c
static int edt_ft5x06_chunked_read(struct regmap *regmap, u8 reg, u8 *buf, int len)
{
    int offset = 0;
    while (offset < len) {
        int chunk = min_t(int, 8, len - offset);
        int error = regmap_bulk_read(regmap, reg + offset, buf + offset, chunk);
        if (error)
            return error;
        offset += chunk;
    }
    return 0;
}
```

El ISR llama a `edt_ft5x06_chunked_read` en vez de `regmap_bulk_read`.

### 2. Touch-power.start conservado

GPIO115 (reset del touch) no funciona con el framework `gpiod` del kernel (readbacks 0x5/0xd,
hito 9 documentado). El driver edt-ft5x06 no puede resetear el chip. Solución: conservar
`touch-power.start` (servicio OpenRC que hace VGP1 on + GPIO115 toggle por pwrap/devmem).

### 3. Daemon ft5336 desactivado

`zzz-touch-input.start` renombrado a `.off`. El daemon userspace ya no corre.
El driver kernel edt-ft5x06 genera los eventos de input nativamente.

### 4. Brillo restaurado

`phosh-session.sh` corregido para lanzar `gsd-power` con suspend desactivado
antes de `exec phosh`:
```sh
gsettings set org.gnome.settings-daemon.plugins.power sleep-inactive-ac-type nothing
gsettings set org.gnome.settings-daemon.plugins.power sleep-inactive-battery-type nothing
gsettings set org.gnome.settings-daemon.plugins.power idle-dim false
/usr/libexec/gsd-power &
sleep 1
exec /usr/local/libexec/phosh
```

### 5. launch_phosh.sh corregido

Cambiado `LIBSEAT_BACKEND=logind` → `LIBSEAT_BACKEND=seatd` (sin systemd).

---

## Verificación en HW (2026-07-02)

```
Input devices:
  mt6779-keypad     (event0, Vol-)
  GKTW50SCED1R0     (event1, touch edt-ft5x06, 5 dedos MT-B)

IRQ EINT117: 1322+ interrupts (touch funcionando)
evtest: coordenadas reales (X=73 Y=768, X=112 Y=771) — NO ceros
Phosh: corriendo, responde al tacto
gsd-power: corriendo (PID 2081), slider de brillo funcional
daemon ft5336: NO corre (correcto)
```

---

## Estado del dispositivo

| Subsistema | Estado | Driver |
|---|---|---|
| **Touch multitouch** | ✅ **5 dedos, kernel nativo** | edt-ft5x06 + chunked_read |
| Display DRM + Phosh + lima | ✅ | mainline + adaptaciones |
| **Brillo slider** | ✅ **restaurado** | gsd-power + mt6582-backlight.py |
| USB gadget | ✅ | mt6582-musb.c |
| GPIO + EINT | ✅ | gpio-mt6582-eint.c |
| PMIC MT6323 | ✅ | mainline + patches |
| Bluetooth | ✅ | mt6582-btif.c |
| WiFi ABIERTO | ✅ | mt6582-wifi.c |
| WiFi WPA2 | ❌ DHCP falla | → port stock mt_wifi |
| Audio | ❌ | → port ALSA |
| GPS | 🟡 no streamea | → capturar START_SEQ |
| Vibrador + LEDs | ❌ | → driver trivial |
| Cámara | ❌ | ISP propietario |
| Módem | ❌ | CCCI propietario |

---

## Archivos modificados

| Fichero | Cambio |
|---|---|
| `arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts` | +nodo touchscreen@38 en i2c0 |
| `drivers/input/touchscreen/edt-ft5x06.c` | +chunked_read (I2C FIFO workaround) |
| `/usr/local/bin/phosh-session.sh` (móvil) | +gsd-power con suspend off |
| `/usr/local/bin/launch_phosh.sh` (móvil) | LIBSEAT_BACKEND=seatd |
| `/etc/local.d/zzz-touch-input.start` → `.off` | daemon ft5336 desactivado |
| `/etc/local.d/touch-power.start` | conservado (VGP1 + GPIO115 reset) |

## Reglas de oro
- `fastboot flash boot` NUNCA `fastboot boot`. mtkclient `wo` NUNCA `wf`.
- NO flashear preloader. NO restaurar LK Lollipop.
- `dd` desde pmOS funciona; desde Android NO (verity).
- Reboot: `echo 1 > /proc/sys/kernel/sysrq; echo b > /proc/sysrq-trigger`.
- pstore: `/sys/fs/pstore/console-ramoops-0`.

*Co-autor: opencode (glm-5.2).*
