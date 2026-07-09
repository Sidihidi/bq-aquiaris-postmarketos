# Accdet MT6323 — jack de auriculares + botones inline (sesión drivers 0709)

> **Estado: CÓDIGO COMPLETO + COMPILA** (`mt6323-accdet.ko`). **NO probado en HW.**
> Port del downstream `accdet/mt6582/accdet.c` (1613 L) a un driver mainline compacto (~450 L),
> con la config del krillin horneada (EINT + multi-key + modo 2.8V).

## Qué da
- **Detección de jack**: SW_HEADPHONE_INSERT + SW_MICROPHONE_INSERT (input switches — WirePlumber
  y el ecosistema ALSA los entienden; también sirve para un daemon de ruteo propio, que es como
  esta plataforma maneja el codec hoy).
- **Distingue auricular-con-micro vs sin micro** (comparador AB de ACCDET_STATE_RG[7:6] tras
  ~250 ms de settle del micbias).
- **Botones inline** (headsets 3-botón): AUXADC sw-ch8 al pulsar → 0-90 mV = **KEY_PLAYPAUSE**,
  90-240 = **KEY_VOLUMEUP**, 240-500 = **KEY_VOLUMEDOWN** (umbral downstream `key_check`).
  Release al volver el comparador a mic-bias.

## RE del downstream (decisiones clave)
| Qué | Valor / fuente |
|---|---|
| Plug detect | **EINT14 del AP** (cust_eint.h krillin: `CUST_EINT_ACCDET_NUM=14`, TRIGGER_LOW, debounce 256ms) — NO el comparador |
| Bloque ACCDET | PMIC offsets 0x077A-0x0794 (reg_accdet.h): CTRL 0x77C, SWCTRL 0x77E, PWM 0x780/2, EN_DELAY 0x784, DEBOUNCE0/1/3 0x786/8/C, IRQ_STS 0x790 (clr=bit8), STATE_RG 0x794 (AB=[7:6]) |
| Clock/reset | TOP_CKPDN bit9 (0x108/10A/10C), TOP_RST_ACCDET bit4 (0x114/6/8) |
| Cust krillin | pwm 0x900/0x400, fall/rise 1/0x3F0, debounce 0x800/0x800/0x20; press-debounce 0x400 |
| Micbias 2.8V | `ACCDET_RSV(0x77A)=0x5A10` + **reg 0x0400 bit14** (flujo 28V_MODE del downstream) |
| ADC botones | sw-ch8 = **hw CH5** → out **AUXADC_ADC7 (0x0722)** bit15=rdy; request = toggle bit8 de CON22(0x76E); **mV = raw·1800/32768** (r_val=1); ch8 sin delay de 300µs (quirk HW) |
| IRQ PMIC | `MT6323_IRQ_STATUS_ACCDET` (=18, bit2 de INT_CON/STATUS **1**) — ya en el enum mainline |

## Integración
### 1. Celda MFD (parche a `drivers/mfd/mt6397-core.c`)
El MFD no tiene celda accdet para MT6323; añadir junto a `mt6323-keys`:
```c
static const struct resource mt6323_accdet_resources[] = {
	DEFINE_RES_IRQ_NAMED(MT6323_IRQ_STATUS_ACCDET, "accdet"),
};
/* ... en mt6323_devs[]: */
	{
		.name = "mt6323-accdet",
		.num_resources = ARRAY_SIZE(mt6323_accdet_resources),
		.resources = mt6323_accdet_resources,
		.of_compatible = "mediatek,mt6323-accdet",
	},
```
(mt6397-core es =y → requiere rebuild+flash del kernel; encaja con el ciclo de flasheo normal.)

### 2. DT (hijo del nodo `pmic: mt6323`)
```dts
	accdet {
		compatible = "mediatek,mt6323-accdet";
		mediatek,micbias-2v8;
		/* EINT14: mapear al GPIO correspondiente del MT6582 (activo-bajo).
		   Con la infra EINT ya montada (táctil usa EINT117), el pin de EINT14
		   sale de la tabla de pines / dws del krillin. */
		jack-detect-gpios = <&pio 14 GPIO_ACTIVE_LOW>;	/* AJUSTAR pin real */
	};
```
### 3. Driver
- Copiar `mt6323-accdet.c` a `drivers/input/misc/` + Kconfig (`config INPUT_MT6323_ACCDET`, depends
  `MFD_MT6397`) + Makefile, o cargarlo como módulo out-of-tree (ya compilado en
  `~/drivers-wip/accdet/mt6323-accdet.ko`; requiere igualmente la celda MFD + DT → flash).

## Validación pendiente (HW)
1. `evtest /dev/input/eventN` (mt6323-accdet) → enchufar auriculares → `SW_HEADPHONE_INSERT 1`
   (+ `SW_MICROPHONE_INSERT 1` si tienen micro). Desenchufar → 0.
2. Con headset de botones: PLAY/VOL± generan eventos con el voltaje esperado (dev_dbg imprime mV).
3. Ajustes probables en HW: el pin real de EINT14, la polaridad del GPIO, y el settle de 250 ms
   (si detecta "sin micro" a auriculares con micro, subirlo).
4. Ruteo de audio al detectar jack: userspace (WirePlumber/daemon) — el codec-enable.sh actual ya
   sabe encender HP vs SPK; engancharlo a los switches de este input device.

*Sesión drivers (Opus) 2026-07-09.*
