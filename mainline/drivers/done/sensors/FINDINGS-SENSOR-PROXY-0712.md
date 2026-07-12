# Sensor stack (iio-sensor-proxy) — pulido 0712: proximidad, brújula, persistencia

> Al retomar el pulido de sensores descubrí que **el stack de sensores estaba CAÍDO** (ni system
> D-Bus ni iio-sensor-proxy corrían) → auto-rotación, ALS y proximidad NO funcionaban en este boot,
> pese a estar validados antes. Raíz + fixes abajo.

## 1. Raíz: no había SYSTEM D-Bus → no había iio-sensor-proxy
- `iio-sensor-proxy` es un servicio del **system bus** de D-Bus. Este boot NO tiene system bus:
  `/run/dbus/system_bus_socket` no existe, `rc-service dbus` = stopped, y arrancarlo falla con
  `ERROR: localmount needs service(s) root` (grafo OpenRC incompleto → ligado a la estabilización
  del boot, tarea #9). Sin system bus el proxy muere: *"cannot own its D-Bus name"*.
- Phosh corre bajo `dbus-run-session` (bus de sesión privado), independiente del system bus.
- **Fix de persistencia**: `etc/local.d/zzx-sensor-proxy.start` — crea `/run/dbus`, lanza
  `dbus-daemon --system --fork` si no está, y `iio-sensor-proxy` si no está. Idempotente. Con esto
  el stack (auto-rotación + ALS + proximidad) sobrevive al boot pase lo que pase con OpenRC.
- Verificado: tras arrancarlo, el proxy detecta **accelerometer** (tilt cambia en vivo), **ALS**
  y **proximity**. Las reglas polkit ya existían (`01-sensor-claim`, `50-krillin-sensor`).

## 2. Proximidad — EXPUESTA (faltaba una propiedad udev)
- iio-sensor-proxy detectaba el device (`iio-poll-proximity` en tmd2772) pero avisaba: *"Found
  proximity sensor but no PROXIMITY_NEAR_LEVEL udev property"* → decía "No proximity sensor".
- **Fix**: `etc/udev/rules.d/72-krillin-proximity.rules` → `ENV{PROXIMITY_NEAR_LEVEL}="100"` para
  `ATTR{name}=="tmd2772"`. Tras recargar udev + reiniciar el proxy: `Has proximity sensor (near: 0)`.
- **Caveat de HW (pendiente, bajo valor)**: el crudo `in_proximity0_raw` lejos = 0-8; **tapado apenas
  sube a ~18** (debería ir a 100s-1000s). El **emisor IR del tmd2772 necesita config** (PPULSE / PDRIVE
  / PGAIN, análogo al bug de ganancia del ALS) para separar cerca/lejos. Es un cambio de DT/driver;
  se difiere porque **sin módem no hay app que use la proximidad** (su uso típico = apagar pantalla en
  llamada). Ya queda expuesta para cuando haya consumidor. NO se toca por i2c crudo: el driver tsl2772
  tiene el chip (pokes directos corromperían su estado).

## 3. Brújula — NO-GO con iio-sensor-proxy 3.9 sin capa de fusión
- iio-sensor-proxy 3.9 **solo** tiene `iio-buffer-compass` (regla udev línea 10): exige un canal
  **`in_rot_from_north_magnetic_tilt_comp`** ya calculado y **bufferizado** (rumbo compensado por
  inclinación). NO convierte magnetómetro crudo en rumbo.
- Nuestro `mmc3516x` solo da `in_magn_{x,y,z}_raw` + scale (sin buffer/trigger, sin canal de rumbo).
  Por eso el proxy dice "No compass" y ni siquiera lista el device como magnetómetro.
- **Para una brújula real** haría falta una **capa de fusión** (magn + accel → heading con
  compensación de tilt + calibración hard/soft-iron) que exponga `in_rot_from_north_magnetic_tilt_comp`
  como scan_element bufferizado (patrón de los sensor-hubs Android / hid-sensor). Es tarea grande.
  Valor bajo (sin app consumidora conocida en la GUI). **Se difiere y documenta** — el magnetómetro
  crudo sigue disponible por sysfs para quien quiera calcular el rumbo a mano.

## Resumen accionable
| Sensor | Estado tras 0712 | Acción |
|---|---|---|
| Acelerómetro (rotación) | ✅ detectado, tilt vivo; ahora **persiste al boot** | hecho |
| ALS (luz) | ✅ detectado (lee 0 lux = bug de ganancia ya cubierto por el daemon de auto-brillo aparte) | ok |
| Proximidad | ✅ **expuesta** vía udev + persistente; ⚠️ crudo débil (emisor IR sin tunear) | tunear PPULSE/PDRIVE en DT cuando haya consumidor |
| Brújula | ⛔ necesita capa de fusión (proxy exige heading bufferizado) | diferida, documentada |

Ficheros: `mainline/pmos/etc/local.d/zzx-sensor-proxy.start`,
`mainline/pmos/etc/udev/rules.d/72-krillin-proximity.rules`.

*Sesión principal (Fable 5), 2026-07-12.*
