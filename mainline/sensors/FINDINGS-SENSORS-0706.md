# Sensores del krillin — auto-rotación FUNCIONA (2026-07-06)

## TL;DR
El **acelerómetro (LSM330)** funciona en mainline y **la auto-rotación de Phosh FUNCIONA**
(validada en HW por el usuario). Cadena: driver IIO `st_accel` → `iio-sensor-proxy` → Phosh.
Los otros sensores (giro, magnetómetro, luz/proximidad) están identificados pero aún NO portados.

## Chips (por scan I2C en bus 0, i2c-mt65xx @11007000)
| Sensor | Chip | Addr | Estado |
|---|---|---|---|
| Acelerómetro | ST **LSM330** | 0x1d | ✅ IIO `st_accel` + **auto-rotación** |
| Giroscopio | ST LSM330 | 0x6b | ✅ IIO `st_gyro` (#243, lee velocidad angular) |
| Luz + Proximidad | AMS **TMD2772** | 0x39 | ✅ `tsl2772` (#243): **luz detectada por iio-sensor-proxy**; proximidad lee a nivel kernel (`in_proximity0_raw`) pero iio-sensor-proxy NO la expone |
| Magnetómetro | MEMSIC **MMC3516x** | 0x30 | ❌ sin driver mainline (`mmc35240`/`mmc5633` son chips distintos); queda sin bindear |
| (Táctil FT5336) | — | 0x38 | ya funcionaba |
| (Cargador FAN5405) | — | 0x6a | ya funcionaba |

## Lo que se hizo para la rotación (kernel #242)
1. **Config → =y** (el móvil no carga módulos): `CONFIG_IIO_ST_ACCEL_3AXIS`,
   `CONFIG_IIO_ST_ACCEL_I2C_3AXIS`, `CONFIG_IIO_ST_SENSORS_I2C`, `CONFIG_IIO_ST_SENSORS_CORE`
   (estaban en `=m`). En `build-krillin/.config` de la Pi .38.
2. **DTS** (`mt6582-bq-krillin.dts`, nodo bajo `i2c0@11007000`):
   ```
   accelerometer@1d {
       compatible = "st,lsm330-accel";
       reg = <0x1d>;
       mount-matrix = "0", "1", "0",
                      "1", "0", "0",
                      "0", "0", "-1";
   };
   ```
   El **mount-matrix** se calibró en HW: el downstream da `.direction = 7`
   (`cust_acc.c`, tabla hwmsen = `{{-1,-1,-1},{1,0,2}}` → swap X↔Y). Empíricamente,
   con la matriz "pura" del direction-7 salía **boca abajo**; la correcta (validada por el
   usuario) es con X,Y positivos: `[[0,1,0],[1,0,0],[0,0,-1]]`.
3. **Userspace**:
   - `iio-sensor-proxy` (ya instalado, 3.9-r2) → `rc-update add iio-sensor-proxy default`.
   - **Regla polkit** `etc/polkit-1/rules.d/50-krillin-sensor.rules`: autoriza a `sxmo` a
     reclamar sensores. SIN ella, Phosh (sesión aislada dbus-run-session, sin sesión elogind →
     no "activa" para polkit) recibe `AccessDenied: Not Authorized: Sensor claim not allowed`
     y NO rota. (Misma clase de problema que inhibit/loginctl de la sesión aislada.)

## Gotchas
- **Phosh arranca antes que iio-sensor-proxy** al boot → loguea "Failed to connect to sensor-proxy",
  pero **re-reclama dinámicamente** cuando el proxy aparece (verificado) → no hace falta ordenar el boot.
- `iio-sensor-proxy` lee el mount-matrix de `/sys/bus/iio/devices/iio:device0/mount_matrix`
  (atributo `mount_matrix`, NO `in_accel_mount_matrix`).
- La orientación es "undefined" con el móvil plano (normal); definida al inclinarlo.

## Añadido en #243 (giro + luz/proximidad)
- Config `=y`: `CONFIG_IIO_ST_GYRO_3AXIS`, `CONFIG_IIO_ST_GYRO_I2C_3AXIS`, `CONFIG_TSL2772`.
- DTS: `gyroscope@6b` (`st,lsm330-gyro`, con la misma mount-matrix que el accel — mismo package) y
  `light-sensor@39` (`amstaos,tmd2772`).

## Pendiente
- **Auto-brillo**: la luz ambiente ya la detecta iio-sensor-proxy, pero falta que Phosh/gsd-power la use
  para conducir el backlight custom (shim D-Bus mt6582). Integración userspace.
- **Proximidad en iio-sensor-proxy**: lee a nivel kernel pero el proxy no la expone (¿necesita eventos/
  umbral del tsl2772?). Uso principal (llamadas) irrelevante sin módem.
- **Magnetómetro** (brújula): MMC3516x sin driver mainline — habría que portarlo/escribirlo.
- **Lockscreen no rota**: a propósito de Phosh (`fixup_lockscreen_orientation` lo fija a portrait);
  permitirlo = parchear Phosh (`/root/build/phosh`).
- (Opcional) IRQ data-ready del accel/giro para buffer con trigger HW (ahora polling).
