# HITO — Keypad (volumen) MT6582 — VOL DOWN funcionando (2026-06-17)

El controlador de matriz de teclado (KPD) del MT6582 funciona en mainline con el
driver `mediatek,mt6779-keypad` **sin cambios** (los offsets de registros son
idénticos a los del MT6779/MT8516). **Vol↓ genera `KEY_VOLUMEDOWN`** ✓.

## Datos del KPD MT6582
- Base física **`0x10011000`** (downstream `KP_BASE = 0xF0011000` virt − 0xE0000000).
- Registros: `KP_STA@0x00`, `KP_MEM1..5@0x04..0x14` (bits [15:0] = estado de teclas,
  1=no pulsada, 0=pulsada), `KP_DEBOUNCE@0x18`, `KP_SEL@0x20`, `KP_EN@0x24`.
- IRQ **`GIC_SPI 116`** EDGE (downstream `MT_KP_IRQ_ID = 32+116 = 148`).
- Clock: el driver pide "kpd"; se usa un `fixed-clock` (el LK lo deja activo).

## DT (`mt6582-bq-krillin.dts`)
```dts
/ { kpd_clk: kpd-clk { compatible="fixed-clock"; #clock-cells=<0>; clock-frequency=<32768>; }; };
&{/soc} {
    keypad: keypad@10011000 {
        compatible = "mediatek,mt6779-keypad";
        reg = <0x10011000 0x1000>;
        interrupts = <GIC_SPI 116 IRQ_TYPE_EDGE_FALLING>;
        clocks = <&kpd_clk>; clock-names = "kpd";
        keypad,num-rows = <1>; keypad,num-columns = <2>;
        linux,keymap = <0x00000073   /* (0,0) KEY_VOLUMEUP   */
                        0x00010072>; /* (0,1) KEY_VOLUMEDOWN */
        wakeup-source; status = "okay";
    };
};
```
`CONFIG_KEYBOARD_MT6779=y` (+ `INPUT_MATRIXKMAP=y`, que ya estaba).

## Resultado
- **Vol↓ = `KEY_VOLUMEDOWN`** ✓. En la matriz cae en `bit1 = (fila0,col1)` →
  scancode 1. (También activa `bit10=(1,1)` → scancode 3, sin mapear = inocuo.)
- **Vol↑ NO responde**: leyendo `KP_MEM1..5` por `devmem`, su pulsación **no baja
  ningún bit** (reposo == pulsado). Además `KP_SEL (0x10011020)` no acepta escritura
  (`rb=0`), así que no se puede forzar el escaneo de su fila/columna por software.
  → la línea de vol↑ necesita **configuración de pin (pull-up/pinmux)** que el
  downstream hacía y mainline no (no hay driver **pinctrl** para el MT6582).
  Pendiente: mapear el GPIO de la columna de vol↑ y forzar su pull, o portar
  pinctrl-mt6582.

## Nota de diagnóstico
`devmem` directo a los registros KP es la vía para depurar (1=suelta, 0=pulsada).
OJO: `KP_EN` es `0x...24` y `KP_SEL` es `0x...20` (no confundir; escribir `KP_EN`
con bit0=0 deshabilita el escaneo → MEM se congela en 0).
