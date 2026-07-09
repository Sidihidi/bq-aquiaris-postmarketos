# Thermal CPU MT6582 — soporte en `auxadc_thermal.c` (sesión drivers 0709)

> **Estado: CÓDIGO COMPLETO + COMPILA** (`auxadc_thermal.ko` con la entrada mt6582; parche de 202 líneas
> sobre `drivers/thermal/mediatek/auxadc_thermal.c`). **NO probado en HW.**
> Fuente de verdad: downstream `thermal/mt6582/mtk_ts_cpu.c` (RE completo abajo).

## Hallazgo clave (por qué es un parche pequeño)
La init periódica del downstream (`thermal_reset_and_initial`) es **EXACTAMENTE el flujo VER_1** del
driver mainline: AUXADC ch11 immediate-mode vía CON1_SET/CLR bit11, `TEMPADCVALIDMASK=0x2C`,
`TEMPPNPMUXADDR = TS_CON1` del apmixedsys, ADCPNP por sensor. Y el **layout V1 del efuse es EL MISMO**
que el del MT6582 (bit a bit): VALID=buf0[0], GE=buf1[31:22], VTS1=buf0[25:17], VTS2=buf0[16:8],
DEGC=buf0[6:1], SLOPE=buf0[31:26], SIGN=buf0[7], ID=buf1[9]. El parser V1 vale sin tocarlo
(solo se añade extraer **ADC_OE=buf1[21:12]**, que la fórmula genérica v1 no usa).

## Lo que difiere (y por eso hay fórmula propia `raw_to_mcelsius_v1_mt6582`)
La fórmula genérica v1 usa `10000 + adc_ge` con el GE crudo; el MT6582 downstream usa:
- `gain = 10000 + (ge−512)·10000/4096` y `oe = adc_oe−512`
- `x_roomt = (vts + 3350 − oe)·10⁴/4096·10⁴/gain`
- factor de pendiente `15/18` (no `10/11` de v2) y constante `3350` (no `3105` de v2)
- signo: `sign==0 → /(165+slope)` (v2 lo tiene al revés) — implementado sobre el o_slope YA plegado
  con signo por el parser V1 → `/(165 + o_slope)` siempre.
- Nuance documentado: el downstream fuerza `slope=0` si `ID==0`; el parser V1 mainline no (solo pliega
  el signo con `ID&SIGN`). Solo diverge si el fuse trae ID=0 con bits de slope ≠0 (improbable).

## Datos MT6582 (del RE)
| Qué | Valor |
|---|---|
| Controlador térmico | **0x1100B000** (THERMAL_BASE) |
| AUXADC | 0x11001000, **canal 11** |
| APMIXEDSYS | **0x10209000**, TS_CON0@+0x600, TS_CON1@+0x604 |
| Buffer on | TS_CON0[7:6]=00 → `apmixed_buffer_ctl` mask `~GENMASK(7,6)`, set 0 |
| Sensores | 2: VTSMCU1 (mux 0, MSR0), VTSMCU2 (mux 1, MSR1) — como el init periódico downstream (`MONCTL0=0x3`) |
| Calibración efuse | index7→**0x10206100**(buf0), index8→**0x104**(buf1); defaults si no calibrado: ge=oe=512, degc=40, slope=0 (el probe ya los pone) |
| cali_val | 165 |

## Ficheros
- `auxadc_thermal.c` — fichero completo modificado (compila).
- `mt6582-thermal.patch` — diff limpio (202 líneas) para aplicar al árbol.
- En la Pi: `~/drivers-wip/thermal6582/` (fichero + parche + .ko de prueba).

## Integración
1. Aplicar el parche: `cd ~/mainline/linux-7.0.12 && patch -p0 drivers/thermal/mediatek/auxadc_thermal.c < mt6582-thermal.patch` (o copiar el fichero entero).
2. `.config`: `CONFIG_MTK_THERMAL=y`, `CONFIG_MTK_SOC_THERMAL=y` (+ `CONFIG_THERMAL_HWMON=y` recomendado)
   y para el efuse: `CONFIG_NVMEM=y`, `CONFIG_NVMEM_MTK_EFUSE=y`.
3. DTS (`mt6582-bq-krillin.dts`) — el nodo `auxadc@11001000` YA existe; añadir:
```dts
	apmixedsys: syscon@10209000 {
		compatible = "mediatek,mt6582-apmixedsys", "syscon";
		reg = <0x10209000 0x8000>;
	};

	efuse: efuse@10206000 {
		compatible = "mediatek,efuse";
		reg = <0x10206000 0x1000>;
		#address-cells = <1>;
		#size-cells = <1>;
		thermal_calibration: calib@100 {
			reg = <0x100 0xc>;
		};
	};

	therm_clk: therm-clk {
		compatible = "fixed-clock";
		#clock-cells = <0>;
		clock-frequency = <26000000>;
	};

	thermal: thermal@1100b000 {
		compatible = "mediatek,mt6582-thermal";
		reg = <0x1100b000 0x1000>;
		clocks = <&therm_clk>, <&auxadc_clk>;
		clock-names = "therm", "auxadc";
		mediatek,auxadc = <&auxadc>;
		mediatek,apmixedsys = <&apmixedsys>;
		nvmem-cells = <&thermal_calibration>;
		nvmem-cell-names = "calibration-data";
		#thermal-sensor-cells = <0>;
	};

	thermal-zones {
		cpu_thermal: cpu-thermal {
			polling-delay-passive = <1000>;
			polling-delay = <5000>;
			thermal-sensors = <&thermal>;
			trips {
				cpu_hot: cpu-hot {
					temperature = <85000>;
					hysteresis = <2000>;
					type = "hot";
				};
				cpu_crit: cpu-crit {
					temperature = <117000>;	/* downstream: shutdown a 117C */
					hysteresis = <2000>;
					type = "critical";
				};
			};
		};
	};
```
   (El driver no pide IRQ — polling del thermal core. Sin cpufreq no hay cooling device pasivo;
   el trip `critical` hace shutdown ordenado, que es la protección que importa.)

## Validación pendiente (HW)
1. `cat /sys/class/thermal/thermal_zone0/temp` → ~25-45 °C en idle (ambiente+delta).
2. Cargar CPU (`yes >/dev/null ×4`) → sube claramente; parar → baja.
3. Sanity de la calibración: dmesg no debe decir "Device not calibrated" (si lo dice, usa defaults — la
   temperatura absoluta puede desviarse ±5-10 °C pero la dinámica vale).
4. Riesgo conocido (estrategia-0707): si la IP térmica no calza byte-a-byte → comparar el raw MSR0 con
   el flujo downstream; fallback `thermal-generic-adc`. El RE dice que SÍ calza (VALIDMASK/flujo idénticos).

*Sesión drivers (Opus) 2026-07-09.*
