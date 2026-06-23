# Evaluación: ¿QEMU para acelerar las pruebas del krillin?

**Pregunta**: ¿montar una VM QEMU que "emule el teléfono" para sacar logs sin el HW?
**Respuesta corta: NO compensa para el trabajo de drivers.** Sirve solo para una parte pequeña.

## Por qué QEMU NO ayuda con lo difícil
QEMU emula *máquinas* concretas (`-M virt`, `vexpress-a9`, …), **no el SoC MT6582**. No existe un modelo QEMU del MT6582 ni de sus periféricos propietarios:
- **CONSYS** (WiFi/BT/GPS/FM), el **HIF** del WiFi, el **BTIF/STP/WMT** — no modelados.
- **PMIC MT6323**, **AUXADC**, **pwrap**, **EINT**, **MUSB MTK**, panel **HX8389**, GPU **Mali-400** — no modelados.

El ~95 % del trabajo que queda (WiFi connect, GPS, audio, sensores) ES justo esos periféricos. QEMU no daría ni un log útil de ellos: el driver haría `ioremap` de registros que en QEMU no existen → falla o lee basura.

## Para qué SÍ podría servir (valor limitado)
- Comprobar que el kernel **compila + arranca** en una VM ARM genérica (smoke test del build).
- Probar lógica de **userspace / initramfs / OpenRC** (los `.start`, el `switch_root`) sin flashear.

Pero no compensa montarlo solo para esto: el ciclo `wifi-iter.sh` (build+flash+reboot) ya es rápido, y el **teléfono real** da los logs que QEMU no puede.

## Recomendación
**Saltarse QEMU.** La aceleración real viene de:
1. El **tooling** (`~/tools/krillin-*`) — diagnóstico/recuperación en 1 comando ✅ (hecho).
2. La **captura de Android** (ver `ROADMAP-FINAL.md` §extracción) — sacar las secuencias propietarias que faltan.
3. Mantener el **downstream** (`~/mainline/downstream/`) como referencia de registros/coreografías.

*(Lo más cercano a "emular el teléfono" sería escribir un device-model custom de QEMU para el MT6582: esfuerzo enorme, no justificado para este proyecto.)*
