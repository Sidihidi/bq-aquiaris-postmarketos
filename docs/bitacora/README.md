# Bitácora de sesiones

Registro por sesión de **qué se hizo y cómo seguir**, para coordinar los frentes que trabajan
sobre este repo (sesión WiFi/port stock, sesión de drivers/UX, etc.) sin pisarse.

## Convención
- **Un fichero por sesión**: `AAAA-MM-DD-<frente>.md` (ej. `2026-07-02-mac-ports-ux.md`).
  Si una sesión abarca varios días, se puede sufijar (`-2`, `-tarde`) o continuar el mismo.
- **Frentes actuales:** `wifi` (port stock mt_wifi, Pi de casa `.38`) · `mac-ports` (drivers
  no-WiFi + UX, Pi `.123`). Poned el vuestro en el nombre.
- **Estructura sugerida de cada entrada:**
  1. **Hecho** — qué quedó funcionando (con commit) y qué probado en HW vs solo compilado.
  2. **Bugs abiertos** — lo que quedó roto o a medias, con el síntoma exacto.
  3. **Cómo seguir** — próximos pasos concretos + ficheros/recetas clave.
- **Antes de tocar el WiFi**, mirad la última entrada `*-wifi.md`; antes de tocar drivers/DTS/UX,
  la última `*-mac-ports.md`. El DTS canónico es `mainline/dts/mt6582-bq-krillin.dts`.

## Índice
- [2026-07-08 · Menú multiboot visual (menupick)](2026-07-08-menupick-multiboot.md) — menú en
  pantalla navegable con Vol+/- y Power. FUNCIONA. pmOS + Android (kexec). Sin U-Boot.
- [2026-07-08 · FM Radio — port del stock MT6627](2026-07-08-fm-port.md) — stock 12K LOC compila y
  se linkea al vmlinux; btif extendido con canal FM; M0 transporte validado. **Falta**: power-cycle
  + verificar `/dev/fm`.
- [2026-07-07 · powerkey — fix del driver mtk-pmic-keys](2026-07-07-powerkey-fix.md) — toggle del
  `RG_PWRKEY_INT_SEL` (de "nunca" a "2 toques") + kthread polling de CHRSTATUS (en depuración) +
  limpieza de sockets dbus en el supervisor + bug udevd descubierto.
- [2026-07-07 · wifi — WPA2+DHCP+datos FUNCIONAN](2026-07-07-wifi-funciona.md) — el "DHCP falla" era
  un falso diagnóstico de rutas; el port del stock `mt_wifi` ya tenía datos cifrados reales. Fix
  de stats del netdev (`.ndo_get_stats`) + config de red persistente.
- [2026-07-02 · mac-ports-ux](2026-07-02-mac-ports-ux.md) — quick-wins (vibr/LED/power/RTC) probados;
  limpieza del repo; UX del botón + brillo en curso.

*Para el estado global ver [README](../../README.md) y [PLAN-PORTS-DRIVERS.md](../../PLAN-PORTS-DRIVERS.md).*
