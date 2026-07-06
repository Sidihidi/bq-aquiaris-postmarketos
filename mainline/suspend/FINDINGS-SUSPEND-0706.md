# Suspend/resume del krillin — hallazgos (2026-07-06)

## TL;DR
**s2idle (suspend-to-idle) FUNCIONA de punta a punta.** El kernel mainline suspende y resume
correctamente; el RTC despierta solo, el botón despierta, y el WiFi reconecta tras el resume. El
único subsistema que no vuelve limpio es **usb0 (gadget musb)**, con un fix conocido (unbind/rebind).
NO hay deep-suspend (power-gating del SoC) porque falta el SPM del mt6582 en mainline.

## Lo confirmado en HW
- `/sys/power/state` = `freeze mem`; `mem_sleep = [s2idle]` → `mem` cae a **s2idle** (no hay `deep`).
- **Ciclo completo validado por ramoops** (persistente, sobrevive al power-cycle): tras `PM: suspend
  entry (s2idle)` → 121s suspendido → `Restarting tasks: Done` + `PM: suspend exit`. Todos los
  callbacks de resume de los drivers COMPLETARON (si alguno colgara, esas líneas no saldrían).
- **RTC auto-wake**: `/usr/sbin/rtcwake -m freeze -s 25` → `rtcwake_rc=0 wall=26s`. El mt6323-rtc
  (`/sys/class/rtc/rtc0`, `wakealarm`) despierta solo. **Red de seguridad de test**.
- **Botón de encendido** = wakeup source (`10011000.keypad`); despertó el móvil en un test.
- **WiFi reconecta** tras resume (NM autoconnect); es el canal fiable que SOBREVIVE al suspend
  (usb0 no). Tras un suspend, alcanzar el móvil por WiFi (`root@192.168.0.x`) para leer resultados.

## El bug de usb0 (y su fix)
Tras el resume, el gadget **musb no re-enumera al host**: el lado móvil tiene usb0 con IP
(172.16.42.1) pero el host USB (Pi) da `device descriptor read error -71` / `unable to enumerate`.
UDC `musb-hdrc.2.auto` queda en estado `not attached`. **Fix (validado): unbind/rebind del musb**
→ estado pasa a `configured`, el host re-enumera (`RNDIS/Ethernet Gadget`, `cdc_ether usb0`), ping OK.
Ver `usb-recover.sh`. Para automatizarlo hace falta un **hook de resume** (no hay elogind que gestione
la sesión → sin `system-sleep.d` estándar): el propio wrapper de suspend hará el rebind tras resumir.

## Lo que NO hay (y por qué)
- **Deep suspend / SPM**: no existe driver de plataforma `mt_spm_sleep`/SPM del mt6582 en mainline
  (solo el vcodec pm, irrelevante). Sin él, `mem` = s2idle. El deep-suspend (power-gating de cluster
  A7 + dominios del SoC) daría el ahorro grande de standby, pero portearlo del downstream es un
  proyecto propio (RE del SPM firmware/secuencias). s2idle ahorra MENOS (CPU→WFI, devices suspendidos,
  DRAM self-refresh) pero es lo que hay hoy y funciona.
- **`CONFIG_PM_DEBUG`** NO está → sin `/sys/power/pm_test` (test por etapas con auto-resume a 5s) ni
  `/sys/power/pm_print_times` (tiempos por device). Para debug fino de suspend conviene rebuild con
  `CONFIG_PM_DEBUG=y`.

## Gotchas operativos
- `echo freeze > /sys/power/state` SIN `pm_test` = **suspend REAL** (no test). Sin `CONFIG_PM_DEBUG`,
  `pm_test` no existe → nunca asumir que gatea. Suspender siempre con `rtcwake -m freeze -s N` (auto-wake).
- Durante s2idle la red se suspende (por diseño); usb0 muere. **Loguear a `/root` (persistente), NO a
  `/tmp` (tmpfs, se pierde al reiniciar).** ramoops (`/sys/fs/pstore/console-ramoops-0`) sobrevive al
  power-cycle → post-mortem de cuelgues.
- Un s2idle sin wakeup armado se queda suspendido esperando (correcto) → parece "colgado" pero no lo
  está; despertar con botón o RTC.

## Siguientes pasos posibles
1. **Wrapper de suspend** (`mt6582-suspend`) que suspenda + rebind-musb en el resume → usb0 sobrevive.
2. **Auto-suspend en idle** (la feature de ahorro): daemon que suspenda al apagar backlight tras N s,
   wake por botón. Integración en la sesión aislada.
3. **Medir el ahorro real** de s2idle (drain en batería) para decidir si merece o si hace falta deep.
4. (Grande) **Deep suspend**: portear el SPM del mt6582.
