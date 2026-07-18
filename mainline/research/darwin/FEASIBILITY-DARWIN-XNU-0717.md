# Viabilidad: portar el kernel Darwin/XNU al krillin (MT6582) — estudio 0717

> Investigación de viabilidad (sesión Windows, Fable 5) a raíz del port de
> [darwin-on-arm/xnu](https://github.com/darwin-on-arm/xnu) que arrancó XNU en el Nokia N900.
> **Veredicto: TÉCNICAMENTE VIABLE como hito de investigación (arrancar XNU a una consola
> serie multiusuario), con una base de partida EXCELENTE — pero NO es un camino a un SO
> usable en el móvil.** Meses de trabajo para el "login por serie", person-años para algo útil.

## Qué logró darwin-on-arm (el listón realista)

El port al N900 arrancó el kernel XNU a un **sistema multiusuario mínimo por consola serie**
— **sin UI, sin drivers de periféricos, no usable como SO**. Es el hito "el kernel de Apple
bootea en este hardware y da un prompt", equivalente a lo que perseguiríamos. NO es iOS/macOS.

## Cadena de arranque (cómo bootea XNU en ARM)

```
LK (bootloader MTK, ya lo usamos)
  → GenericBooter  (2º stage: convierte ATAGs de Linux → boot-args/devicetree de Darwin,
                    carga el Mach-O del kernel, aplana el device tree, salta a XNU)
      → mach_kernel   (Mach-O sin comprimir dentro de un Image3, magic "krnl")
      → ramdisk       (volumen HFS+ crudo en Image3, magic "rdsk")
      → DeviceTree.img3 (formato Apple/iOS, NO el FDT de Linux)
```
GenericBooter se arranca "como si fuera el kernel Linux" desde el bootloader → **encaja con
nuestro flujo**: se empaqueta como el "kernel" (imagen ARM cruda) dentro de nuestra boot.img
MTK (mtk_hdr + abootimg), LK salta a él por fastboot. Console serie ya la tenemos.

## Por qué el krillin es MEJOR candidato que el N900

| | Nokia N900 (lo que portaron) | krillin MT6582 (nosotros) |
|---|---|---|
| CPU | OMAP3430, **Cortex-A8** single | **Cortex-A7** quad, ARMv7-A |
| IRQ ctrl | OMAP **INTC** (propietario TI) | **ARM GICv1** (estándar) |
| pexpert XNU análogo | `pe_omap3430.c` | **`pe_apq8060.c`** (ARMv7 + **GICv1**, ya existe) |

**Clave**: el pexpert de XNU-on-arm ya trae **GIC** (Qualcomm APQ8060 y ARM Realview) —
`qgic_dist_init()`, distributor+CPU-interface, ack por `GIC_CPU_EOI`. El MT6582 usa el GIC
estándar del Cortex-A7 → el pexpert se **modela 1:1 sobre `pe_apq8060.c`**, solo cambian bases.

## Superficie del port (lo que hay que escribir)

Todas las direcciones HW ya las conocemos de nuestro port Linux:

### 1. `pe_mt6582.c` (platform expert de XNU) — el grueso
- **GIC** (copiar `pe_apq8060.c`): Distributor **0x10211000**, CPU-interface **0x10212000**
  (del DTS: `arm,cortex-a7-gic`, GICD/GICC/GICH/GICV en 0x10211/12/14/16000). Estándar GICv1.
- **Timer**: o el **MTK GPT @0x10008000** (IRQ SPI 112, simple estilo OMAP GPT) o el
  **generic timer ARM del A7** (PPI, CNTP) — el A7 lo tiene built-in.
- **UART consola**: **0x11002000**, 16550-compatible (`mtk8250`), 921600n8 — earlycon YA
  funciona en Linux aquí. `putc/getc` triviales.
- **sysirq MTK @0x10200100** (extensión de polaridad entre GIC y periféricos) — puede
  ignorarse al principio o replicarse.

### 2. GenericBooter para MT6582 (2º stage)
UART console (0x11002000), mapa de memoria (**RAM @0x80000000**, 1GB), boot-args. Modelar
sobre el port OMAP3. Pequeño.

### 3. DeviceTree formato Apple para MT6582
CPUs (4× cortex-a7), memoria, GIC, timer, UART. Adaptar el `devicetree.dtsi` del N900
(compatible `"nokia,omap3-n900","AppleARM"` → nuestro `"bq,krillin","AppleARM"`).

### 4. Low-level ARMv7 del A7
Cache/MMU/errata en el `start.s` de XNU-arm. A7 vs A8/A9 = diferencias menores en ARMv7-A
(tamaño de línea de cache, algún errata). Manejable.

### 5. Ramdisk (userland Darwin/ARM mínimo)
Volumen HFS+ con launchd + dyld + libSystem + un shell, cross-compilado a ARMv7. **La parte
más tediosa** del hito "multiusuario"; el ramdisk de darwin-on-arm es prebuilt/oscuro.

## Nuestras ventajas (enormes)
- **Conocemos el MT6582 al dedillo**: cada base (GIC, timer, UART, PMIC, SPM, memoria), el
  boot chain (boot.img MTK, fastboot), recuperación por fastboot, y **consola serie
  funcionando** (921600n8 @0x11002000).
- Toolchain de kernels ARM montado en la Pi (cross-compile), empaquetado de boot.img, y
  ciclo flash/recuperar dominado.
- Dual-boot: podemos probar sin perder pmOS/Maemo.

## Riesgos y límites HONESTOS
1. **Toolchain de build de XNU-arm = el hurdle #1**: fork de ~2013-14, ARMv7, sin mantener;
   build finicky (clang/SDK viejos o los scripts de cross-build de darwin-on-arm). De-riskear
   ESTO primero (M0) antes que nada.
2. **Sin framebuffer/DSI/táctil** al principio (consola serie, como el N900). El panel Himax
   hx8389 sería un IOKit driver aparte = esfuerzo enorme y separado.
3. **Cero drivers de periféricos** en IOKit: sin WiFi, sin display, sin storage útil, sin
   audio. Cada uno = driver IOKit desde cero (person-años para un SO usable).
4. **Cortex-A7 ≠ A8**: SMP quad (el N900 era single-core; XNU-arm SMP en ARMv7 es frágil),
   cache/timer distintos.
5. Resultado máximo realista = **Darwin multiusuario por serie, sin periféricos** = curiosidad
   de investigación, MISMO listón que el N900. No "corre iOS".

## Hitos propuestos (de-risk incremental)
- **M0 — Build**: conseguir que compile `darwin-on-arm/xnu` + `GenericBooter` (toolchain).
  El hurdle real; hasta no pasarlo, lo demás es teoría.
- **M1 — Booter habla**: GenericBooter empaquetado en boot.img MTK, LK salta a él, imprime
  "hello" por UART 0x11002000. Prueba el handoff del boot.
- **M2 — XNU early boot**: `pe_mt6582.c` (GIC+timer+UART copiado de apq8060) → XNU imprime
  sus primeros logs por serie.
- **M3 — VM/ramdisk**: XNU llega a init de VM + monta el ramdisk HFS+.
- **M4 — multiusuario**: launchd + login por consola serie = **iguala el hito del N900**.

## Recomendación
Proyecto de investigación **paralelo y de bajo riesgo** (no toca el daily; todo por
fastboot/serie). **Empezar por M0** (montar el build de XNU-arm en la Pi) — es el filtro:
si compila, el resto es adaptación mecánica sobre `pe_apq8060.c` con direcciones que ya
tenemos. Si el build resulta impracticable (toolchain podrido), se aborta barato ahí.
El valor es didáctico/hito, no un SO usable — encuadrarlo así.

## Fuentes
- [darwin-on-arm/xnu](https://github.com/darwin-on-arm/xnu) (pexpert/arm: pe_apq8060.c=GIC ARMv7)
- [darwin-on-arm/GenericBooter](https://github.com/darwin-on-arm/GenericBooter) (2º stage)
- [darwin-on-arm/DeviceTrees](https://github.com/darwin-on-arm/DeviceTrees) (formato Apple, N900 RX51)
- wiki darwin-on-arm "Building a bootable system"
- Nuestro `mt6582.dtsi` (GIC 0x10211000, timer 0x10008000, UART 0x11002000, RAM 0x80000000)

*Estudio de viabilidad 0717, sesión Windows (Fable 5). Solo research; ningún cambio en HW.*
