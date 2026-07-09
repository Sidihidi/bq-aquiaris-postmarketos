# Magnetómetro MMC3516x — driver IIO nuevo (sesión drivers 0709)

> **Estado: CÓDIGO COMPLETO + COMPILA** (`mmc3516x.ko`, out-of-tree contra build-drv, vermagic 7.0.12).
> **NO probado en HW** (el móvil estaba ocupado por la sesión SPM). Escrito sobre el template mainline
> `mmc35240.c` (misma topología de registros) con el flujo/timings/inversión-Z del downstream
> `magnetometer/mmc3516x_auto/mmc3516x.c`.

## Qué hace
- IIO magnetómetro clásico (`in_magn_{x,y,z}_raw` en mili-gauss + `scale` 0.001 → gauss).
- Single-shot TM + poll del bit DS (como el downstream; sin modo continuo).
- **Degauss** (REFILL 50ms → SET 1ms → 0 → REFILL 50ms → RESET 1ms → 0) al probe y **cada 150 medidas**
  (cadencia downstream `MMC3516X_RESET_INTV`).
- **Z invertida** en el driver (downstream hace `vec[2] = 65536 - raw`): `z = -(raw - 32768)`.
- Sensibilidad 2048 counts/Gauss, NFO 32768 (16-bit slow, `CTRL1=0`).
- Sin OTP (eso es del MMC35240) — compensación identidad.

## Integración
### Opción A — módulo out-of-tree (sin tocar el árbol)
```
# en la Pi:
cd ~/mainline/linux-7.0.12
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=/home/cpcd/drivers-wip/mmc3516x modules
# el .ko ya está compilado en ~/drivers-wip/mmc3516x/mmc3516x.ko (contra build-drv, mismo kernel)
scp ~/drivers-wip/mmc3516x/mmc3516x.ko root@172.16.42.1:/usr/local/lib/
# en el móvil: insmod /usr/local/lib/mmc3516x.ko
```
### Opción B — in-tree
- Copiar `mmc3516x.c` a `drivers/iio/magnetometer/`.
- `drivers/iio/magnetometer/Kconfig`: entrada `config MMC3516X` (tristate, depends I2C, select REGMAP_I2C),
  p.ej. clonando la de MMC35240.
- `drivers/iio/magnetometer/Makefile`: `obj-$(CONFIG_MMC3516X) += mmc3516x.o`.
- `.config`: `CONFIG_MMC3516X=y` (el móvil no carga módulos del initrd; =y como los demás sensores).

### DT (hijo de `&i2c0`, junto al accel/gyro/light)
```dts
magnetometer@30 {
	compatible = "memsic,mmc3516x";
	reg = <0x30>;
	/* mount-matrix: calibrar en HW (empezar con la del accel; el eje Z
	   ya va invertido en el driver, como en el downstream) */
};
```
### Instanciación runtime SIN reflashear DTB (para probar antes del próximo flash)
```
insmod mmc3516x.ko
echo mmc3516x 0x30 > /sys/bus/i2c/devices/i2c-0/new_device
cat /sys/bus/iio/devices/iio:device*/name       # -> mmc3516x
cat /sys/bus/iio/devices/iio:device*/in_magn_x_raw
```

## Validación pendiente (HW)
1. `in_magn_{x,y,z}_raw` cambian al girar el móvil / acercar un imán.
2. Magnitud del campo terrestre ~250-650 mG (|B| = sqrt(x²+y²+z²) con scale aplicado).
3. Ejes vs orientación física → fijar `mount-matrix` (el downstream krillin usa su cust_mag; validar empíricamente como se hizo con el accel).
4. Nota estrategia-0707: **sin consumidor en Phosh** (la brújula no lo usa) — es completitud IIO; útil para apps tipo GNOME Maps si algún día consumen brújula vía iio-sensor-proxy (que sí expone `HasCompass`).

*Sesión drivers (Opus) 2026-07-09.*
