# 2026-07-07 · powerkey — fix del driver mtk-pmic-keys (toggle INT_SEL + polling)

## TL;DR
El botón power del krillin **no respondía al desbloquear tras idle** (2+ min). Diagnóstico profundo del
driver del kernel `mtk-pmic-keys.c` comparándolo con el original de MediaTek (`pmic_mt6323.c`).
**Mejora conseguida:** de "nunca desbloquea" a "desbloquea con 2 toques". El fix del toggle del
`INT_SEL` es sólido; el polling de CHRSTATUS quedó pendiente de depurar.

## Hecho (código, probado en HW)
- **Fix del toggle `RG_PWRKEY_INT_SEL`** en `mtk-pmic-keys.c` (`mainline/input/mtk-pmic-keys.c`):
  el driver mainline **no re-armaba** la detección de flanco del PMIC tras cada evento. El driver
  original MTK (`pwrkey_int_handler` en `pmic_mt6323.c`) togglea el bit 4 de `INT_MISC_CON` (0x016C)
  en cada evento: press detectado → `INT_SEL=1` (esperar release); release → `INT_SEL=0` (esperar
  press). Sin esto, el PMIC MT6323 deja de generar IRQs tras el primer press+release. **Confirmado
  en HW:** tras este fix, el botón pasó de "nunca responde tras idle" a "responde (con 2 toques)".
- **Kthread de polling de CHRSTATUS** (mismo fichero): el IRQ del MFD no se genera en el primer toque
  tras idle aunque `CHRSTATUS` (0x0142) sí cambia con cada press/release (medido: 61 eventos en
  CHRSTATUS mientras el IRQ del MFD solo disparaba ~10). Añadido un kthread `mtk-pmic-keys-poll`
  que lee el debounce register cada 100ms y reporta el input. **Estado:** corre (PID visible) pero
  los eventos no llegaron al daemon powerkey como esperábamos — queda por depurar por qué el
  `regmap_read` en kthread context o el `input_report_key` no propagan. Boot `boot-poll-debug.img`.
- **Daemon `mt6582-powerkey`** (`mainline/userspace/usr/local/bin/mt6582-powerkey`): añadido
  reopen del fd del evdev cada 30s (workaround del grab "atascado" tras idle) + logging de timing
  detallado (flag/screen_on/output_on con ms). El reopen no arregló el problema de fondo pero
  hace el daemon más robusto.
- **Supervisor `launch_phosh.sh`** (`mainline/userspace/usr/local/bin/launch_phosh.sh`): añadida
  limpieza de sockets `/tmp/dbus-*` huérfanos antes de (re)lanzar phoc. Visto en HW: 328 sockets
  acumulados tras días de crashes/reboots del WiFi → degradaba el `phosh_env()` del daemon powerkey
  y la GUI. Fix permanente de la fuga.

## Diagnóstico del PMIC (lección para retomar)
- **El PMIC MT6323 SÍ detecta el botón siempre** (CHRSTATUS bit1 cambia con cada toque físico,
  confirmado con `pwrap_poke r 0x0142` en polling a 20Hz).
- **El IRQ del MFD (mt6397-irq) NO se genera en el primer toque tras idle** (delta IRQ=0 medido).
  El PMIC entra en un estado donde el primer toque solo "lo reactiva" sin generar IRQ.
- El driver original MTK compensa con el kthread `pmic_thread_kckett` que se auto-despierta, pero
  también depende del EINT del PMIC → no es polling puro.
- **El primer toque tras idle sigue perdido** incluso con el polling añadido — falta depurar por
  qué el kthread no reporta (posible: regmap_read en kthread context, o input_report_key pisado
  por el handler del IRQ). Hay printk de debug en `boot-poll-debug.img` para investigar.

## Bug de fondo descubierto (NO bloqueante hoy)
- **udevd hace segfault** en `main()` tras cargar libs: mismatch de ABI entre `eudev` (Nov 2025) y
  `libblkid`/`libcrypto` (Jun 2026, más recientes). Un `apk upgrade` anterior actualizó las libs.
  Síntoma: phoc falla con "libinput: no input devices" en algunos boots (sin udevd, NM no gestiona
  wlan0, los permisos de /dev/dri/* pueden salir `crw------- root root`). Curiosamente en otros boots
  phosh arranca bien porque los permisos salen correctos por devtmpfs. **Fix:** `apk fix eudev
  libblkid xz-libs zstd-libs` cuando haya red estable (wlan0 conectada). No urgente.

## Cómo seguir (powerkey)
1. **Depurar el kthread de polling**: el printk añadido (`mtk-pmic-keys poll: key0 PRESSED`) debería
   aparecer en dmesg al pulsar. Si NO aparece → el `regmap_read` en kthread context falla o devuelve
   siempre lo mismo (quizás cache del regmap). Probar `regmap_force_cache_only` o leer sin cache.
2. **Si el polling funciona pero el daemon no recibe**: investigar el EVIOCGRAB vs phoc. Posible
   solución: quitar el daemon powerkey del todo y dejar que phoc gestione el botón (con el polling
   del driver, phoc recibiría los eventos y haría lock/unlock nativo).
3. **Alternativa de fondo**: portar el kthread `pmic_thread_kckett` del downstream tal cual (con su
   wait_event + wake_up_pmic desde el EINT del PMIC), en vez del IRQ del MFD. Es la ruta que usa
   Android y es la más probada.

## Entorno / recetas
- Boot de hoy: `boot-poll-debug.img` (con toggle + polling + printk debug). Sector 83968.
- Logs clave: `/tmp/powerkey.log` (daemon, con timestamps ms), dmesg `mtk-pmic-keys poll:`.
- PMIC peek: `pwrap_poke r 0x0142` (CHRSTATUS pwrkey bit1), `r 0x016C` (INT_MISC_CON INT_SEL bit4),
  `r 0x0172` (INT_STATUS0 pwrkey bit5), `r 0x0160` (INT_CON0 máscara pwrkey bit5).
- Driver canónico: `mainline/input/mtk-pmic-keys.c` (parcheado sobre mainline 7.0.12).
