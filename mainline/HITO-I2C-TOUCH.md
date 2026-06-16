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

## ★ ACTUALIZACIÓN: pwrap + VGP1 validados — EL TOUCH ESTÁ VIVO
- **pwrap MT6582 funciona** (poke con `tools/pwrap_poke.c`): mmap `PWRAP_BASE=0x1000D000`,
  secuencia wacs2 (CMD@+0x9C, RDATA@+0xA0, VLDCLR@+0xA4; `cmd=(w<<31)|((adr>>1)<<16)|wdata`;
  FSM idle=0/wfvldclr=6 en bits16-18). El LK deja el pwrap listo. **CID MT6323 leído = 0x2023.**
- **VGP1 = DIGLDO_CON7 (reg PMIC `0x050A`), bit 15** (de mainline mt6323-regulator.c).
  `pwrap_poke w 0x050A 0x8000` → **el touch aparece en i2cdetect (0x38) y `i2cget -y 0 0x38`
  devuelve `0x5a`** = panel-ID Truly (= firmware downstream "Truly0x5a"). ¡El FT5336 está
  encendido y respondiendo!
- **Pendiente fino:** la lectura write-then-read (poner reg + leer) da timeout = *repeated-start*
  del i2c-mt65xx en MT6582. Afinar (quirk I2C / clock). Luego EINT117 + nodo edt-ft5x06, y
  llevar pwrap+mt6323 al kernel (DT) en vez del poke. Reset = GPIO115 (ya high).
- Validado de punta a punta: I2C → GPIO → pwrap → PMIC → power → touch responde. El muro
  principal del touch (alimentación) está SUPERADO.

## ★★ TOUCH LEE COORDENADAS — validación HW completa
**Fix del write+read**: el `mt6577_compat` (auto_restart=0) daba timeout en write-then-read
(el FT5x06 necesita repeated-START real). Solución: nuevo `mt6582_compat` en
`drivers/i2c/busses/i2c-mt65xx.c` con **`auto_restart=1`** y SIN quirk COMB, + of_match:
```c
static const struct mtk_i2c_compatible mt6582_compat = {
    .regs = mt_i2c_regs_v1, .pmic_i2c = 0, .dcm = 1, .auto_restart = 1,
    .aux_len_reg = 0, .timing_adjust = 0, .max_dma_support = 32,
};
/* en mtk_i2c_of_match: */
{ .compatible = "mediatek,mt6582-i2c", .data = &mt6582_compat },
```
y el nodo i2c0 del DT usa `compatible = "mediatek,mt6582-i2c"`.

**Resultado** (i2ctransfer write+read tras VGP1 ON + reset GPIO115):
vendor(0xA8)=**0x5a** (Truly), chipid(0xA3)=**0x14** (FT5x06), fwver(0xA6)=0x15.
Tocando: TD_STATUS=1 dedo, **X=275, Y=759** (en 540×960). **¡El touch reporta coordenadas!**

**Nota usb0+i2c**: el i2c retrasa el boot → g_ether enumera tarde → usb0 sube a ~40s (no
es que el i2c rompa la red; es timing; esperar o hacer el up de usb0 robusto).

**Siguiente (driver-level)**: nodo `edt-ft5x06` (DT) + IRQ EINT117 (portar EINT o polling)
+ pwrap/mt6323 al kernel (regulador VGP1 vin-supply) → eventos /dev/input → GUI.

## Power VGP1: NO por late_initcall a pelo (rompe el boot)
Probado `tools/mt6582-pmic-fixup.c` (late_initcall que enciende VGP1 vía pwrap): Alpine
arranca y usb0 sube, pero **sshd NO arranca** (OpenRC se atasca antes; revertido → sshd
vuelve OK). El busy-wait del pwrap en un initcall síncrono interfiere con el boot.
**Formas válidas de encender VGP1:**
1. **poke userspace** (suficiente ahora): un servicio Alpine `local.d` que ejecute
   `pwrap_poke w 0x050A 0x8000` tras la red. No toca el boot del kernel → seguro y persistente.
2. **driver completo** (lo correcto/upstream): pwrap(mt6582)+mt6323+regulator en el DT;
   el `edt-ft5x06` pide VGP1 con `vin-supply` → encendido ordenado por el regulator framework.
`mt6582-pmic-fixup.c` queda como referencia de la secuencia wacs2 en C-kernel (base para
el driver), pero NO debe usarse como late_initcall.
