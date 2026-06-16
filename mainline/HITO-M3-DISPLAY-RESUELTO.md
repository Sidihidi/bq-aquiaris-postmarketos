# HITO M3 — Display mainline LEGIBLE (fin del tinte amarillo)

**Estado: RESUELTO** (2026-06-16). El framebuffer de mainline (simplefb sobre el
panel hx8389 qHD que deja el LK) ya se ve correcto; antes salía todo con un fuerte
tinte amarillo que impedía leer los logs.

## El síntoma
Con `simple-framebuffer` en el DT, el kernel mainline arrancaba y *escribía* en el
framebuffer (0xBF400000, la región que deja el LK), pero en pantalla el blanco se
veía **amarillo** (el negro seguía negro). Logs ilegibles.

## La causa raíz: DESAJUSTE DE FORMATO DE PÍXEL
- `simplefb` (nuestro DT) estaba declarado como **`a8r8g8b8`, stride 2176** (32 bpp).
- El OVL (overlay del MT6582) que *presenta* ese buffer estaba configurado, en el
  estado bueno de pmOS, como **RGB565 (CLRFMT=1), pitch 0x440=1088** (16 bpp).
- Resultado: el hardware leía un buffer de 32 bpp como si fuera 16 bpp → se perdía
  el canal **azul** → blanco (FFFFFF) se convertía en amarillo (FFFF00). El negro
  (0) es invariante, por eso "tintaba" en vez de romper la geometría.

Las hipótesis previas (gamma del BLS, matriz YUV C_CF_SEL, bypass del COLOR) eran
callejones sin salida: atacaban el síntoma, no el formato.

## El método que funcionó: clonar la config de pmOS
pmOS (kernel downstream 3.10) muestra la pantalla **bien**. Con un kernel pmOS con
`CONFIG_DEVMEM`, se leyeron por `/dev/mem` (mmap) los registros DISP reales y se
replicaron en mainline. Config "buena" capturada en vivo:

```
OVL  SRC_CON(0x2C) = 0x00000008          (solo capa L3 activa)
     L3 CON(0x90)  = 0x400010ff   KEY(0x94)=0xff000000
     L3 SRC_SIZE(0x98)=0x03c0021c (960x540)  OFFSET=0  ADDR(0xA0)=0xbf400000  PITCH(0xA4)=0x440
     ROI_BGCLR(0x28)=0   RDMA0_CON(0xC0)=0x03ff0000
RDMA CON(0x00)=0x3f  SIZE0(0x14)=0x21c  SIZE1(0x18)=0x3c0
BLS  EN(0x00)=0x00010001   SET(0x10)=0x00100007
COLOR CFG_MAIN(0x400)=0x20000000
MUTEX MOD(0x2C)=0x680  SOF(0x30)=1  EN(0x00)=0x303     <- el COMMIT real del OVL
```
Tabla CLRFMT del OVL MT6582 (de rebuild310.log): **1=rgb565, 2=argb8888, 3=pargb8888**.

## La solución (2 partes)
1. **DT**: `simple-framebuffer` pasa a **`format="r5g6b5"; stride=<1088>`** para que el
   buffer COINCIDA con lo que lee el OVL (recompilar solo el dtb).
2. **Reprogramar el OVL en RGB565** = escribir los registros de arriba (vía /dev/mem
   en el init de pruebas; lo ideal es hacerlo en el kernel). Clave: el **MUTEX**
   (`MOD=0x680, SOF=1, EN=0x303`, togglear EN 0→0x303) es el commit que presenta los
   cambios; sin él, nada se latchea.

## Bases físicas DISP
`MMSYS 0x14000000 | OVL 0x14007000 | RDMA 0x14008000 | BLS 0x1400A000 | COLOR 0x1400B000 | MUTEX 0x1400E000`

## Lecciones
- Verificar que el `format` del simplefb (DT) case con el CLRFMT que espera el OVL.
- No adivinar el pipeline: leer con memdump los registros del kernel que SÍ funciona
  (3.10) y replicarlos.
- El ramoops "console" es circular (console-size 0x80000): un `printk` en bucle lo
  llena y borra el volcado. Volcar en **líneas atómicas** (snprintf + 1 write, nivel
  `<0>`) tras `sleep(15)` (que el ruido de initcalls del kernel no lo pise) y luego
  `sleep` largo sin imprimir.

## CONSOLIDADO EN EL KERNEL (2026-06-16)
La reprogramación del OVL ya NO la hace el init de diagnóstico: vive en el kernel
como `drivers/video/fbdev/mt6582-dispfix.c` (un `late_initcall` que comprueba
`of_machine_is_compatible("bq,krillin")` y reprograma el OVL a RGB565 con los
registros de arriba). Integrado en `drivers/video/fbdev/Makefile`
(`obj-$(CONFIG_FB_SIMPLE) += mt6582-dispfix.o`).
**Verificado**: mainline arranca con pantalla LEGIBLE usando un init que NO toca el
display (tiny_init_m2c.c, solo levanta usb0). Sin `sleep(15)` ni amarillo prolongado.

## Pendiente (futuro)
Sustituir el `late_initcall` (que escribe registros con direcciones físicas
hardcodeadas) por un driver de display real (DRM/fbdev) para el MT6582 cuando se
aborde. Por ahora el fixup es la solución estable.
