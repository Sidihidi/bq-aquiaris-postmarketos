# HANDOFF — LED de notificaciones: diagnosticado + fix compilado, pendiente flash+validar (Mac/.123, 2026-07-06)

## TL;DR
El LED RGB de notificaciones NO se enciende con Phosh porque **al kernel le falta el trigger
`pattern`** (`CONFIG_LEDS_TRIGGER_PATTERN`), que es *exactamente* por donde feedbackd conduce los LEDs.
**Fix = habilitar ese config + rebuild + flash.** Ya está compilado en la .123 (falta SOLO flashear,
la Pi se cayó de la red antes del `dd`).

## Lo que está BIEN (verificado en HW #238)
- **Hardware RGB funciona**: `echo N > /sys/class/leds/{red,green,blue}:indicator/brightness` enciende
  cada color (validado por el usuario: "sí cambió"). `max_brightness=6`. También `white:button-backlight`.
- El nodo DT ya existe (`mt6582-bq-krillin.dts:469`, `mediatek,mt6323-led`, ISINK0-3). Driver `mt6323-led`
  bindeado, LEDs en `/sys/class/leds/`.
- **El error "mt6323-pwrc: Failed to locate of_node" del dmesg NO es del LED** — es del *power-controller*
  del MT6323 (sub-device del MFD), inofensivo y sin relación. (El ROADMAP lo atribuía mal al LED.)
- Permisos OK: LEDs `root:video` 664, `sxmo` está en grupo `video` → **sxmo escribe los LEDs** (probado).
- La regla udev de feedbackd **SÍ matchea**: `72-feedbackd.rules:41` (`SUBSYSTEM=="leds",
  DEVPATH=="*/*:indicator"` → `ENV{FEEDBACKD_TYPE}="led"` + `RUN fbd-ledctrl -t pattern`). `udevadm info`
  confirma `FEEDBACKD_TYPE=led`. Grupo `feedbackd` (gid 104) existe. Tema `default.json` tiene feedback
  `Led` para notificaciones (`notification-missed-generic`=azul, `battery-caution`=rojo, `phone-missed-call`
  =cian, `message-missed-*`=azul/morado).

## LA CAUSA RAÍZ (confirmada)
`cat /sys/class/leds/blue:indicator/trigger` → la lista **NO incluye `pattern`**;
`modprobe ledtrig-pattern` → "module not found"; `.config` → `# CONFIG_LEDS_TRIGGER_PATTERN is not set`.
feedbackd conduce los LEDs vía `fbd-ledctrl -t pattern` (trigger `ledtrig-pattern`) → sin ese trigger,
feedbackd NO registra ningún LED → `fbcli -E notification-missed-generic` da **"No feedback found"** a
CUALQUIER nivel (full incluido), y el muestreo software del brillo confirma que feedbackd nunca lo toca.

## EL FIX (ya hecho, falta flashear)
```
cd ~/mainline/linux-7.0.12
./scripts/config --file build-krillin/.config --enable CONFIG_LEDS_TRIGGER_PATTERN   # -> =y  (HECHO)
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- olddefconfig        # HECHO
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2 zImage           # HECHO (zImage ready)
bash ~/wifi-iter-w.sh   # <-- PENDIENTE (la Pi .123 se cayo de la red aqui)
```
Mismo árbol que casa (audio mt6582 + Fase5 WiFi + `MTWIFI=y` presentes en .123) → build = #238 + el trigger,
**sin regresión** de WiFi/audio.

## VALIDACIÓN pendiente (tras el flash)
1. `cat /sys/class/leds/blue:indicator/trigger` → debe listar `pattern`.
2. Perfil feedbackd a `full` (el feedback `Led` de las notificaciones está a nivel full en `default.json`):
   `gsettings set org.sigxcpu.feedbackd profile full` (en la sesión sxmo).
   `su sxmo -s /bin/sh -c "export XDG_RUNTIME_DIR=/run/user/1000 DBUS_SESSION_BUS_ADDRESS=<el de /proc/$(pgrep -f feedbackd)/environ>; fbcli -E notification-missed-generic"` → **el LED debe parpadear azul**.
3. Si el trigger `pattern` aparece pero feedbackd sigue sin encenderlo: puede necesitar el COLOR por udev
   (nuestros LEDs no exponen el atributo sysfs `color`; la regla genérica `:indicator` no pone
   `FEEDBACKD_LED_COLOR`). Fix: regla udev que añada `ENV{FEEDBACKD_LED_COLOR}="red|green|blue"` por cada
   `{red,green,blue}:indicator` (como las líneas `lp5523:r/g/b` de `72-feedbackd.rules:49-51`).
4. Perfil final: `full` da LED+sonidos+háptico-de-botón; `quiet` (el que dejamos para la vibración) NO
   incluye el `Led` de notificaciones en el tema stock. Decidir: o `full`, o un tema propio
   `bq,krillin.json` que ponga el `Led` a nivel `quiet`/`silent` (lo normal en móviles: LED de notif
   visible aunque el resto esté silenciado).

## Estado
- Vibración: ✅ RESUELTO y subido (`edce205`, perfil `silent`→`quiet`).
- Batería %: ✅ ya funcionaba (VBAT real + % + carga vía daemon); curva interim correcta (OCV cruda
  regresaría bajo carga sin sensado de corriente).
- LED: ⏳ este handoff — fix compilado, pendiente flash+validar cuando vuelva la .123.

*Sesión Mac (Opus 4.8), 2026-07-06. Infra: móvil vía Pi .123 (usb0), sshd flaky → power-cycle a mano.*
