# Sensores del krillin — auto-rotación FUNCIONA (2026-07-06)

## TL;DR
El **acelerómetro (LSM330)** funciona en mainline y **la auto-rotación de Phosh FUNCIONA**
(validada en HW por el usuario). Cadena: driver IIO `st_accel` → `iio-sensor-proxy` → Phosh.
Los otros sensores (giro, magnetómetro, luz/proximidad) están identificados pero aún NO portados.

## Chips (por scan I2C en bus 0, i2c-mt65xx @11007000)
| Sensor | Chip | Addr | Estado |
|---|---|---|---|
| Acelerómetro | ST **LSM330** | 0x1d | ✅ IIO `st_accel` + rotación |
| Giroscopio | ST LSM330 | 0x6b | pendiente (driver `st_gyro` existe) |
| Magnetómetro | MEMSIC **MMC3516x** | 0x30 | pendiente (mainline `mmc35240` ≠ chip; verificar) |
| Luz + Proximidad | AMS **TMD2772** | 0x39 | pendiente (driver `tsl2772` existe, =m→=y) |
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

## Pendiente
- **Lockscreen no rota**: es a propósito de Phosh (`fixup_lockscreen_orientation` fija el
  lockscreen en portrait). Para permitirlo haría falta parchear Phosh (build en `/root/build/phosh`).
- Portar el resto: giroscopio (`st_gyro` =y + nodo @0x6b), luz/proximidad (`tsl2772` =y + nodo @0x39
  → auto-brillo + apagar pantalla en llamada), magnetómetro (brújula; verificar driver MMC3516x).
- (Opcional) IRQ data-ready del accel para buffer con trigger HW (ahora polling).
