# 2026-07-02 · frente mac-ports (drivers no-WiFi + UX)

Sesión en la Pi `.123` con el móvil conectado por USB, en paralelo al frente `wifi` (port stock en `.38`).

## Hecho (probado en HW salvo donde diga)
- **Quick-wins de drivers — ✅ PROBADOS (#230, commit 8d907fd):** vibrador (`regulator-haptic`),
  LEDs RGB+botones (`leds-mt6323` ISINK0-3), botón power (`mtk-pmic-keys`), RTC (`rtc-mt6397`).
  Confirmado por el usuario: **vibra, LEDs y botones iluminan**. Fix clave: `mt6323` como
  **interrupt-controller** (`interrupt-parent=<&eint>; interrupts=<25 ...>` = GPIO25/EINT25) + subnodo
  `rtc`; y borrar un **nodo `vibrator` duplicado** (`mt6582-vibrator` draft) + su `.o` del build.
- **Audio (driver 2) MAPEADO + esqueleto AFE** que compila (`sound/soc/mediatek/mt6582/`, `=m`, sin
  nodo DT → inerte). Mapa verificado en `mainline/audio/HANDOFF-AUDIO-PORT-0702.md`.
- **STP (driver 5) analizado**: el FW por BTIF ignora CRC/checksum → solo falta resync RX; parche de
  ~15 LOC listo en `mainline/wifi-consys/HANDOFF-STP-CORE-PORT-0702.md` (SIN integrar, arriesga BT).
- **Limpieza del repo**: docs superadas → `docs/archive/`; README reescrito a estado real.
- **Herramienta**: `wifi-iter-w.sh` ahora **compila `dtbs` además de `zImage`** y **reinicia por sysrq**
  (el `reboot` plano NO reinicia esta pmOS → si no, arrancas el kernel viejo y crees que "no tomó").

## Bugs que atacamos al final (2 de UX)
1. **Slider de brillo no funciona.** Causa: Phosh 0.55 intenta el backlight por el **conector DRM**
   (DSI-1) y falla con `EISDIR`. **Fix aplicado:** quitar `backlight = <&backlight>` del `panel@0`
   en el DTS → el conector deja de exponer backlight → Phosh cae al sysfs. El `pwm-backlight`
   (`/sys/class/backlight/backlight`) sigue vivo + el daemon `mt6582-backlight.py`.
   ⚠️ **OJO (memoria [[project-brillo-gsd-power]]):** el camino que funcionaba era
   **logind/backlight-sysfs**, NO `gsd-power` (que además SUSPENDÍA el móvil). `gsd-power` está
   corriendo ahora — si tras el fix el slider sigue mal o el móvil se suspende solo, **matar/inhibir
   gsd-power** y comprobar que Phosh usa el path logind. **[verificar en HW → resultado abajo]**
2. **El botón de bloquear deja el LCD encendido.** Causa: Phosh bloquea la sesión pero el backlight
   PWM (daemon aparte) no se apaga; además el daemon viejo `mt6582-powerkey` **sondeaba el registro
   INT0 del PMIC y lo limpiaba**, peleándose con el driver kernel `mtk-pmic-keys` que ahora posee ese
   IRQ → botón flaky. **Fix aplicado:** reescrito `mt6582-powerkey` para **leer KEY_POWER del evento
   del kernel** (`/dev/input/eventN` de mtk-pmic-keys) y hacer **toggle del backlight a 0/restaurar**
   (apaga el LCD de verdad). Canónico en `mainline/userspace/usr/local/bin/mt6582-powerkey`.
   **[verificar en HW → resultado abajo]**

## Resultado HW de los 2 fixes
- Brillo: _[pendiente de rellenar tras el flash/test]_
- Botón/LCD: _[pendiente de rellenar tras desplegar el daemon y pulsar]_

## Cómo seguir (para casa)
1. **Brillo**: si el slider aún no mueve el PWM → `gsd-power` es el sospechoso (memoria). Probar:
   `rc-service` / matar gsd-power, o inhibirlo, y confirmar que `phosh` escribe
   `/sys/class/backlight/backlight/brightness` vía logind. Daemon actual: `mt6582-backlight.py`.
2. **Botón**: confirmar que pulsación corta apaga/enciende el LCD sin flakiness. Si se quiere
   **pulsación larga = menú de apagar**, ir al "camino Phosh nativo" (elogind
   `HandlePowerKey=ignore` + gsd/phosh manejando el key) — se dejó el daemon como paso 1.
3. **Thermal (driver 9)**: AUXADC probea pero expone **0 canales** `in_voltage` en sysfs — investigar
   por qué (¿faltan consumidores io-channels / atributos raw?). Y el NTC de batería lee 0 mV → rail
   pull-up apagado (buscar VTCXO/VIO28/VGP).
4. **Audio (driver 2)**: validar el reloj del AFE por `devmem 0x11220000` ANTES de escribir el PCM.
5. **STP (driver 5)**: integrar el parche de resync + probar BT sin regresión.

## Entorno / recetas
- Pi `.123` (`cpcd@192.168.0.123`) → móvil `ssh root@172.16.42.1`
  (`sudo ip addr replace 172.16.42.2/24 dev usb0`). Build+flash: `bash ~/wifi-iter-w.sh`.
- Reinicio fiable: `echo 1 > /proc/sys/kernel/sysrq; echo b > /proc/sysrq-trigger`.
- pstore salvado en cada boot: `/var/log/pstore/boot-*`.
- DTS canónico: `mainline/dts/mt6582-bq-krillin.dts`. Plan: `PLAN-PORTS-DRIVERS.md`.
