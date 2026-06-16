# HITO — I2C + GPIO en mainline, y el touchscreen (FT5336)

**Estado: I2C ✅, GPIO ✅, touch bloqueado por alimentación (PMIC VGP1).**

## I2C funciona en mainline MT6582 (desde cero)
- Nodo añadido al DT (`mt6582-bq-krillin.dts`):
  ```
  i2c0: i2c@11007000 {
      compatible = "mediatek,mt6577-i2c";   // mismo IP v1, sin tocar el driver
      reg = <0x11007000 0x70>, <0x11000200 0x80>;  // i2c + apdma
      interrupts = <GIC_SPI 44 IRQ_TYPE_LEVEL_LOW>;
      clocks = <&system_clk>, <&system_clk>;  clock-names = "main","dma";
      clock-div = <1>;            // OBLIGATORIO (sin él parse_dt da -EINVAL)
      clock-frequency = <100000>;
      #address-cells = <1>; #size-cells = <0>; status = "okay";
  };
  ```
  apdma = `AP_DMA_BASE(0x11000000) + 0x200 + 0x80*id` (downstream i2c.c). IRQ = GIC_SPI 44
  (downstream MT_I2C0_IRQ_ID 76 − 32). `CONFIG_I2C_MT65XX=y`.
- **Resultado:** `/dev/i2c-0` creado; `i2cdetect -y -r 0` ve **0x1d, 0x30, 0x39, 0x6a, 0x6b**
  (sensores del E4.5: acelerómetro / luz / giroscopio). **El I2C arranca con los clocks y
  pines que deja el LK — NO hizo falta pinctrl** (hallazgo clave: ahorra portar pinctrl-mt6582).

## GPIO por poke (devmem propio)
busybox no trae `devmem` y `memdump` no alinea a página → `devmem.c` propio (mmap
página+offset, lee/escribe u32, armhf estático). Layout GPIO MT6582 (`mt_gpio_base.h`):
`dir[11]@0x000, dout[11]@0x400, mode[36]@0x600`; VAL_REGS=16B (val@+0/set@+4/rst@+8);
banco=GPIO/16, bit=GPIO%16. `GPIO_BASE` físico = `0x10005000`.

## Touchscreen Focaltech FT5336
- Panel Truly OGS 540×960. I2C **bus 0, addr 0x38**, reset **GPIO115**, IRQ **EINT 117**
  (falling). Driver mainline: **`edt-ft5x06`** (compatible `edt,edt-ft5306`), ya en `.config`.
- **0x38 no responde** aunque el I2C funciona: GPIO115 (reset) ya está output+HIGH (no en
  reset). El bloqueo real es la **ALIMENTACIÓN**:
  `TPD_POWER_SOURCE_CUSTOM = MT6323_POWER_LDO_VGP1` → el touch cuelga del **regulador VGP1
  del PMIC MT6323**, que el LK no enciende (sí enciende VGP2 para el LCD).

## El siguiente sub-proyecto: PMIC MT6323 (desbloquea touch + batería + carga)
Buena noticia: **mainline ya trae los drivers** — `drivers/soc/mediatek/mtk-pmic-wrap.c`
(pwrap) y `drivers/regulator/mt6323-regulator.c` (MT6323, con VGP1). Falta:
1. Añadir compatible+data **mt6582** a `mtk-pmic-wrap` (portar de mt8135, contemporáneo) +
   nodo pwrap en DT (`PWRAP_BASE = 0x1000D000`).
2. Nodo mfd `mt6397/mt6323` + sus reguladores en DT.
3. `CONFIG_MTK_PMIC_WRAP=y + REGULATOR_MT6323=y + MFD_MT6397=y`.
4. El touch: `vin-supply = <&mt6323_vgp1>` + nodo `edt-ft5x06` (reg 0x38, irq EINT117).
   VGP1_EN = bit15 (PMIC_RG_VGP1_EN_SHIFT) de un reg DIGLDO del MT6323.
Atajo de prueba: encender VGP1 por poke pwrap (secuencia wacs2 sobre PWRAP_BASE) si el LK
dejó el pwrap inicializado.

Y para el IRQ del touch (EINT 117): portar un controlador EINT MT6582 mínimo, o parchear
edt-ft5x06 a polling.
