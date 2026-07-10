# Boot con drivers WIP integrados — `boot-menupick6-drivers.img` (2026-07-10)

> Sesión Mac (Fable 5). Integrados **3 drivers** al kernel del dual-boot + empaquetados en una imagen
> nueva, flasheada y **VALIDADA EN HW** (kernel #276 arrancó; dual-boot intacto).
>
> ## RESULTADO HW (mismo día, tras reboot)
> - ✅ **Magnetómetro**: `iio:device4 mmc3516x` automático por DT, lecturas vivas.
> - ✅ **Accdet**: probe OK (`accdet ready, micbias 2.8V`), **EINT14 correcto** — 5 ciclos
>   enchufar/desenchufar detectados (`jack: headphone (AB=0)` / `jack: removed`, 17 IRQs en
>   `mt-eint 14`). Pendiente menor: probar botones inline y micro (AB=0 puede ser settle 250ms).
> - ⚙️ **Thermal**: `thermal_zone0 cpu-thermal` registra y la **dinámica funciona**
>   (idle −1.2 °C → carga 20s +7.2 °C → baja al parar) pero el **offset absoluto está ~30 °C bajo**
>   → afinar calibración efuse/fórmula (comparar raw MSR0 con el flujo downstream). No bloquea.

## Qué se hizo
Integrados **in-tree** en `~/mainline/linux-7.0.12` (build-krillin) los 3 WIP de `drivers-wip/`:

| Driver | Estado | Integración |
|---|---|---|
| **Magnetómetro** MMC3516x | ✅ **CONFIRMADO EN HW** (runtime) | `drivers/iio/magnetometer/mmc3516x.c` + Kconfig `MMC3516X` + Makefile + DT `magnetometer@30` en `&i2c0` |
| **Thermal** MT6582 | ⏳ compila, **sin probar HW** | patch `auxadc_thermal.c` + `MTK_THERMAL`/`MTK_SOC_THERMAL`/`NVMEM_MTK_EFUSE` + nodos DT (apmixedsys/efuse/therm_clk/thermal/thermal-zones) |
| **Accdet** MT6323 | ⏳ compila, **sin probar HW** | `drivers/input/misc/mt6323-accdet.c` + Kconfig `INPUT_MT6323_ACCDET` + Makefile + **celda MFD** en `mt6397-core.c` (`mt6323-accdet`, IRQ `MT6323_IRQ_STATUS_ACCDET`) + DT `accdet` en `&pmic` |

**Magnetómetro ya validado antes de compilar** (por runtime, sin reflashear): `insmod mmc3516x.ko` +
`echo mmc3516x 0x30 > /sys/bus/i2c/devices/i2c-0/new_device` → `iio:device4` con lecturas vivas
(X≈179 Y≈321 Z≈−60, fluctúan = sensor real). `i2cdetect -y 0` confirma el chip en **0x30**.

## Cambios exactos en el DTS (bloques override añadidos AL FINAL de `mt6582-bq-krillin.dts`)
```dts
&i2c0 { magnetometer@30 { compatible = "memsic,mmc3516x"; reg = <0x30>; }; };

&pmic {
	accdet {
		compatible = "mediatek,mt6323-accdet";
		mediatek,micbias-2v8;
		jack-detect-gpios = <&eint 14 GPIO_ACTIVE_LOW>;   /* EINT14 — PIN A VERIFICAR EN HW */
	};
};

/ {
	therm_clk: therm-clk { compatible="fixed-clock"; #clock-cells=<0>; clock-frequency=<26000000>; };
	thermal-zones { cpu_thermal: cpu-thermal {
		polling-delay-passive=<1000>; polling-delay=<5000>; thermal-sensors=<&thermal>;
		trips { cpu_hot { temperature=<85000>; hysteresis=<2000>; type="hot"; };
		        cpu_crit { temperature=<117000>; hysteresis=<2000>; type="critical"; }; };
	}; };
};

&{/soc} {
	apmixedsys: syscon@10209000 { compatible="mediatek,mt6582-apmixedsys","syscon"; reg=<0x10209000 0x8000>; };
	efuse: efuse@10206000 { compatible="mediatek,efuse"; reg=<0x10206000 0x1000>;
		#address-cells=<1>; #size-cells=<1>; thermal_calibration: calib@100 { reg=<0x100 0xc>; }; };
	thermal: thermal@1100b000 { compatible="mediatek,mt6582-thermal"; reg=<0x1100b000 0x1000>;
		clocks=<&therm_clk>,<&auxadc_clk>; clock-names="therm","auxadc";
		mediatek,auxadc=<&auxadc>; mediatek,apmixedsys=<&apmixedsys>;
		nvmem-cells=<&thermal_calibration>; nvmem-cell-names="calibration-data";
		#thermal-sensor-cells=<0>; };
};
```
Backups en la Pi: `*.bak-pre-0710-drivers` (dts, .config, mt6397-core.c, auxadc_thermal.c).

## Imagen + flasheo
- Empaquetada `~/mainline/pkg/boot-menupick6-drivers.img` (receta `assemble.sh`: `cat zImage dtb` →
  `mtk_hdr.py KERNEL` → `abootimg --create` con `initrd-menupick-mtk`, cmdline `console=tty0
  clk_ignore_unused quiet`). 15075328 B, 71% de la partición. md5 `7f175ffe05276dddc864202e37833c5d`.
- **Flasheada (staged)** al eMMC sector 83968 desde pmOS viva, readback md5 OK. **Sin reboot**: pmOS
  sigue con su kernel en RAM; los drivers nuevos entran en el PRÓXIMO arranque.

## Cómo probar (tras reiniciar a pmOS con la imagen nueva)
1. **Mag** (ahora permanente por DT): `cat /sys/bus/iio/devices/iio:device*/name` → uno = `mmc3516x`;
   `cat .../in_magn_{x,y,z}_raw`.
2. **Thermal**: `cat /sys/class/thermal/thermal_zone*/temp` → ~25-45 °C idle; `yes>/dev/null &`×4 sube;
   `dmesg | grep -i thermal` (no debe colgar el boot; "not calibrated" = usa defaults, OK).
3. **Accdet**: `evtest` sobre el input `mt6323-accdet` → enchufar auriculares = `SW_HEADPHONE_INSERT 1`.
   Si el probe falla en `dmesg` (`no jack-detect gpio`), **el pin EINT14 `<&eint 14>` es incorrecto** →
   ajustar el número de GPIO real (falla acotada: no rompe PMIC/batería/keys).

## Rollback
`boot-menupick5.img` (bueno) sigue en `~/mainline/pkg/`. Si el arranque falla:
`base64 boot-menupick5.img | ssh root@movil 'base64 -d>/root/b5.img'; dd if=/root/b5.img of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync` (desde pmOS; si no bootea ninguno → BROM/mtkclient).

## Maemo (paralelo): instrumentado para el próximo boot
El negro-con-backlight ya NO es el abort de sesión (el fix `49maemo-forcestartup` funciona; la sesión
llega a hildon-desktop). Causa: `hildon-desktop` (Clutter 0.8 eglx sobre lima) arranca vía `dsmetool`,
inicializa sound/xrecord y **nunca escribe `/tmp/hildon-desktop/desktop-started.stamp`** →
`21hildon-desktop-wait` bloquea la sesión → negro. Su stderr iba bajo dsmetool (invisible).
**Instrumentado**: `20hildon-desktop` reescrito para lanzar `/usr/local/bin/hd-launch.sh` que loguea a
`/var/log/hd-desktop.log` (+ CLUTTER_DEBUG/EGL_LOG_LEVEL/LIBGL_DEBUG). Al próximo boot de Maemo →
leer ese log = causa exacta del fallo de Clutter/GL. Fallback listo: `swrast_dri.so` presente →
si es lima, forzar `LIBGL_ALWAYS_SOFTWARE=1`.

*Sesión Mac (Fable 5), 2026-07-10.*
