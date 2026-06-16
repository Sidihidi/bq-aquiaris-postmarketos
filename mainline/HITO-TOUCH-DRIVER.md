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

## Pendiente

- **Kernel:** `CONFIG_INPUT_EVDEV=y` + `CONFIG_INPUT_UINPUT=y` (ahora no existen
  `/dev/input/` ni `/dev/uinput`) → recompilar zImage + reflashear.
- Ejecutar `ft5336_touch` (modo uinput) y validar con `evtest`.
- Servicio OpenRC para lanzarlo al arranque (tras `touch-power`).
- Futuro: driver `edt-ft5x06` en DT (reg 0x38, `vin-supply=&vgp1`, `reset-gpios`,
  IRQ EINT117) — requiere primero arreglar lecturas i2c >8 bytes (apdma).

Ver: `HITO-TOUCH-POWER-SERVICE.md`, `HITO-I2C-TOUCH.md`.
