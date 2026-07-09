# HANDOFF — Touch FUNCIONA + brillo pendiente (2026-07-02 noche)

## Logros de la sesión

### ✅ Touch kernel edt-ft5x06 — FUNCIONA
- Multitouch 5 dedos (MT-B), IRQ-driven EINT117, 0 CPU en reposo
- Patch chunked_read: lecturas I2C en bloques ≤8 bytes (FIFO MT6582)
- Coordenadas reales verificadas (X=73 Y=768)
- Daemon ft5336 desactivado, driver kernel nativo
- touch-power.start conservado (GPIO115 reset por pwrap, gpiod no funciona)
- Commit: `0885ea4`

### ❌ Brillo — NO funciona
- gsd-power corre dentro de la sesión D-Bus de Phosh
- mt6582-backlight.py corre y lee /sys/class/backlight
- **Problema**: Phosh da error `"Setting backlight on DSI-1 failed: GDBus.Error:System.Error.EISDIR: Is a directory"` — Phosh intenta controlar el backlight del conector DRM y falla, bloqueando el slider
- El daemon mt6582-backlight.py responde a escrituras manuales en /sys (echo 3 > brightness cambia el PWM)
- **Pero el slider de Phosh NO escribe a /sys/class/backlight** — Phosh 0.55 tiene su propio backlight manager que va por DRM connector, no por sysfs

### Próximo paso para el brillo
1. **Opción A**: quitar `backlight = <&backlight>` del nodo panel@0 del DTS → Phosh no intenta DRM backlight → gsd-power escribe a /sys → daemon PWM funciona
2. **Opción B**: parchear Phosh 0.55 para usar sysfs backlight en vez de DRM connector backlight
3. **Opción C**: hacer que el pwm-backlight del kernel funcione correctamente (eliminar el daemon y el workaround del hito 15 "colores raros")

**Recomendar Opción A**: es un cambio de 1 línea en el DTS (quitar `backlight = <&backlight>` del panel). Probar mañana.

## Estado del dispositivo
| Subsistema | Estado |
|---|---|
| Touch multitouch (5 dedos) | ✅ kernel nativo |
| Display DRM + Phosh + lima | ✅ |
| Brillo slider | ❌ Phosh EISDIR error |
| USB gadget + SSH | ✅ |
| Bluetooth (hci0) | ✅ |
| WiFi abierto | ✅ |
| WiFi WPA2 | ❌ → port stock |
| Audio | ❌ → port ALSA |

## Kernel actual en el móvil
- `boot-touch3.img` con chunked_read + DTS touch + todos los fixes previos
- Sector 83968

## Para mañana
1. **Brillo**: quitar `backlight = <&backlight>` del panel en el DTS → recompilar DTB → flash → test slider
2. **WiFi**: el plan de port del stock mt_wifi está listo (`HANDOFF-MTWIFI-PORT-PLAN-0702.md`)
3. **Plan de ports**: `PLAN-PORTS-DRIVERS.md` con 10 drivers priorizados

*Co-autor: opencode (glm-5.2).*
