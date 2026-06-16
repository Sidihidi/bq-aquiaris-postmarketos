# HITO — Driver del touchscreen FT5336 (lectura por I2C validada)

**Estado: lectura del touch VALIDADA en hardware (2026-06-17).** Falta soporte de
input en el kernel (`/dev/input`, `/dev/uinput`) para emitir eventos de verdad.

Driver de espacio de usuario `ft5336_touch.c`: hace polling del FT5336 por I2C0
(0x38) y emite el toque como dispositivo de pantalla táctil (single-touch +
MT protocol B con 1 slot) por `/dev/uinput`.

## Hallazgos clave (depurados en hardware)

1. **El I2C del MT6582 (mainline) solo lee fiable ≤ 8 bytes.** El controlador
   tiene FIFO de 8 bytes; las transferencias más largas necesitan APDMA, que en
   mainline **da timeout** (`Connection timed out`). Verificado: `r1/r4/r7/r8`
   funcionan siempre; `r9/r16/r33` fallan (2 de 3 `r33` dan timeout). Por eso el
   driver lee **7 bytes** (status + toque 1) → single-touch. Multitouch real
   exige trocear en lecturas ≤ 8 o arreglar el apdma del i2c-mt65xx.

2. **Detección del dedo por EVENT-FLAG, no por `td_status`.** `td_status` (reg
   0x02) es inestable (a veces 0x00, 0x0f, 0xff). Fiable: bits 7-6 de `XH`
   (reg 0x03): `0`=down, `2`=contact → dedo; `1`=up, `3`=none → sin dedo.
   - `X = ((XH & 0x0f) << 8) | XL`  (reg 0x03,0x04)
   - `Y = ((YH & 0x0f) << 8) | YL`  (reg 0x05,0x06).  Pantalla **540×960**.

3. **En reposo el chip entra en modo monitor y los regs de toque leen `0xFF`.**
   El driver trata `0xFF`/sin-evento como "sin dedo".

4. **Arranque del sensado: power-cycle de VGP1.** Tras un simple "encender" (si ya
   estaba on) el chip responde por I2C pero el sensado puede quedar dormido. Un
   power-cycle (apagar→esperar→encender) + reset GPIO115 lo arranca limpio (igual
   que hace un reboot del teléfono). Lo aplica `touch-power.start`. *Nota: no está
   confirmado al 100% que sea imprescindible (en las pruebas no se controló si se
   tocaba); se mantiene por seguridad, coste ~4 s al boot.*

## Validación

Tras power-cycle, `ft5336_touch --raw` capturó un arrastre continuo y limpio:
```
(85,575)→(88,568)→(91,561)→(97,541)→ ... →(227,325)→(235,320)
404 muestras · 0 errores i2c · 61 con dedo · ~50 Hz · X:85-354 Y:191-575
```
Trayectoria coherente = lectura correcta del panel.

## Infraestructura de input: COMPLETA (2026-06-17)

- `CONFIG_INPUT_EVDEV=y` ya estaba built-in; **`CONFIG_INPUT_UINPUT` faltaba**.
- `CONFIG_TOUCHSCREEN_EDT_FT5X06=y` ya está compilado (driver nativo disponible).
- El kernel tiene `CONFIG_MODULES=y`, así que en vez de reflashear (transferir el
  boot.img de 13.7 MB por la red USB **reinicia el teléfono** — el musb no aguanta
  esa carga) se compiló **`uinput.ko` como módulo** (=m) contra el MISMO `.config`
  (el vermagic coincide) y se carga con `insmod` (74 KB, sin reflashear).
- Con uinput cargado, `ft5336_touch` crea el dispositivo:
  `/dev/input/event0`, `Name="ft5336"`, `PROP=INPUT_PROP_DIRECT`, EV=SYN|KEY|ABS.
  `evtest` lo reconoce correctamente.

## ⚠️ Bloqueante actual: arranque/wake FIABLE del chip

El sensado del FT5336 **no arranca de forma determinista** con power/reset manual
desde userspace. El chip queda en estados indefinidos:
- `0xFF` en los regs de toque = dormido/monitor (no despierta al tocar si está
  atascado, porque no atendemos la línea INT).
- `0x00` en los regs = estado de reset indefinido (mi daemon lo malinterpretaba
  como toque en (0,0); corregido con el filtro `st!=0xFF && evt∈{0,2} && (x|y)`).
- datos reales = solo se logró tras ciertos power-cycles "con suerte" y tras un
  **reboot** completo (el bootloader deja el GPIO de reset en buen estado).

Causa raíz: el **reset por GPIO115 no es fiable** — los `devmem 0x10005474/78`
dan readbacks raros (`0x5`/`0xd`), señal de que no controlan bien el pin (registros
SET/CLR mal identificados o polaridad incorrecta). Los power-cycles manuales con ese
reset dudoso DEGRADAN el chip; solo el reboot lo recupera.

**Pendiente (siguiente fase) — la "forma correcta":**
1. Identificar bien GPIO115 (registros/polaridad del GPIO MT6582) para un reset
   fiable, o definirlo en el DT (`reset-gpios`).
2. Atender la línea **INT (EINT117)** (o hacer polling del pin) para sincronizar.
3. Idealmente: nodo DT **`edt-ft5x06`** (`reg=0x38`, `vin-supply=&vgp1`,
   `reset-gpios`, `interrupts=EINT117`) → el driver del kernel hace reset+INT+lectura
   bien. Requiere antes **arreglar el apdma del i2c-mt65xx** para lecturas >8 bytes
   (el edt-ft5x06 lee 33 bytes de una vez).

## Lo que SÍ está probado

Cuando el chip está en buen estado (post-reboot), el daemon lee y decodifica un
arrastre continuo correcto: `(85,575)→…→(235,320)`, ~50 Hz, 0 timeouts. El pipeline
I2C→protocolo→uinput→evtest está completo y validado salvo el wake fiable.

Ver: `HITO-TOUCH-POWER-SERVICE.md`, `HITO-I2C-TOUCH.md`.
