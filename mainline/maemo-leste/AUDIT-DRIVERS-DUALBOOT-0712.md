# Audit de drivers en dual-boot pmOS ↔ Maemo Leste (2026-07-12)

> Prueba de robustez: ¿cargan nuestros drivers en Maemo igual que en pmOS y qué modificación
> hace falta para que funcionen en los DOS a la vez? Mismo kernel + dtb para ambos (dual-boot),
> así que la conclusión es nítida y accionable.

## Regla que emerge del audit
**Los drivers de KERNEL cargan idénticos en ambos** (es el mismo zImage+dtb). **Todos los huecos
están en el PEGAMENTO userspace**, que en pmOS vive en `/etc/local.d/*` (OpenRC/Alpine) +
`/usr/local/bin/mt6582-*` + reglas udev/polkit, y Maemo (Debian/Leste) NO ejecuta. Portar ese
pegamento = portar drivers-en-userspace, no tocar el kernel.

## Matriz (Maemo, kernel #285)
| Subsistema | Kernel (¿proba?) | Userspace en Maemo | Acción para "los dos" |
|---|---|---|---|
| **Batería VBAT** (hwmon mt6323_auxadc) | ✅ `in0_input` = VBAT mV | ✅ lee | ok |
| **Carga FAN5405** | ⚠️ NO hay power_supply; el chip necesita kick i2c | ❌ nada → watchdog 22s revierte a 100mA → **DRENA → apagados** | ✅ **RESUELTO**: daemon `fan5405-charge.py` (i2c directo, sin i2c-tools) + servicio OpenRC |
| **Audio** (card0 mt6582audio) | ✅ AFE+codec | (PulseAudio de Maemo) | ok kernel |
| **Sensores IIO** (accel/gyro/ALS/mag) | ✅ iio:device0..4 (lsm330_accel, lsm330_gyro, tmd2772, mmc3516x) | ❌ sin iio-sensor-proxy/daemons | portar auto-rotación/auto-brillo (userspace) |
| **Táctil** (FT5336) | ✅ event0..4 | (X evdev) | ok |
| **FM** (/dev/fm) | ✅ | ❌ sin app | portar app FM (userspace) |
| **Display/backlight** | ✅ tras fix DSI modeset (0711) | ✅ | ok |
| **Power/keys** (mtk-pmic-keys + keypad) | ✅ input0/1 | (hildon) | ok kernel |
| **WiFi** (mtk_mtwifi) | ⚠️ **firmware `mediatek/mt6582/WIFI_RAM_CODE` no encontrado (-2)** en Maemo | ❌ falta el blob en /lib/firmware de Maemo | **copiar el firmware WiFi al /lib/firmware de Maemo** (en pmOS sí está) |
| **BT** (HCI init) | ✅ core; falta bring-up WMT | ❌ | portar bring-up (userspace) |

## LO CRÍTICO — carga (causa de los apagados)
Confirmado: `power_supply` VACÍO en Maemo + VBAT a **3101 mV** (crítico). El FAN5405 (I2C0 @0x6a)
tiene watchdog ~22s; sin kick revierte a 100mA/3.54V → drena → brownout/watchdog → reinicio
(`boot_reason=3`). **Fix portado y persistente**:
- `usr/local/sbin/fan5405-charge.py` — daemon i2c-directo (ioctl I2C_SLAVE, NO necesita i2c-tools
  que Maemo no trae). Valores idénticos al de pmOS: OREG 4.20V, IINLIM 800mA, TE/CE on, kick 10s.
- `etc/init.d/fan5405-charge` — servicio OpenRC (command_background) + fallback en rc.local.
- Tras habilitarlo: `STAT=CARGANDO`. Nota HW: con el móvil colgado del **puerto USB de la Pi**
  (~500mA) la carga es neta ~0 frente al consumo de la GUI (VBAT estable, ya NO cae). Para
  recuperar de verdad desde 3.1V → **cargador de pared 2A** (el daemon mantiene la carga sola).

## Huecos userspace restantes para paridad Maemo (no bloqueantes)
1. **WiFi firmware**: copiar `WIFI_RAM_CODE` (y patch/BT si aplica) al `/lib/firmware/mediatek/mt6582/`
   de Maemo — el driver lo busca por filesystem (error -2 = ENOENT). En pmOS está.
2. Sensores: portar auto-rotación (mount-matrix + regla) y auto-brillo (daemon ALS) a Maemo.
3. FM/BT/power-button daemons: userspace, portables cuando toque.

**Conclusión**: la base de kernel es sólida y común; Maemo "solo" necesita que le llevemos el
pegamento userspace de pmOS. La carga era el único hueco con consecuencia grave (apagados) y ya
está tapado de forma que sirve para ambos OSes.

*Sesión principal (Fable 5), 2026-07-12.*
