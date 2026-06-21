# HITO: Quick wins (día 2) — Apps, teclado y diagnóstico de backlight — 2026-06-17

Continuación de `mainline/phosh/HITO-PHOSH-ARRANCA.md`. Phosh ya arrancaba; este día
añade **apps + teclado en pantalla** y ataca el primer quick win (**backlight**).

## ✅ Apps GTK4 + multitarea
`gnome-calculator`, `gnome-clocks`, `gnome-text-editor` (apk add). Arrancan y la
multitarea va **fluida**. GTK4 cae a render **software (cairo)** sin GPU — los warnings
`MESA: ZINK vkCreateInstance failed` / `libEGL` son **esperados e inofensivos** (GTK4
intenta Vulkan/GL, no hay, usa cairo). Aparecen en el grid (.desktop sin `NoDisplay`).

## ✅ Teclado en pantalla (squeekboard / OSK) — RESUELTO
**Problema:** el paquete Alpine de squeekboard **NO trae el `.service` D-Bus** → Phosh
aborta el OSK con `sm.puri.OSK0 was not provided by any .service files`. (Los layouts
van **embebidos en GResource**, no como archivos sueltos — `keyboards/` vacío es normal.)

**Fix (3 pasos):**
1. Crear `/usr/share/dbus-1/services/sm.puri.OSK0.service` (`Exec=/usr/bin/squeekboard`).
2. squeekboard activado por D-Bus necesita `WAYLAND_DISPLAY` → inyectarlo con
   `UpdateActivationEnvironment` (usar **gdbus**, NO busctl —no existe en Alpine—; y
   ejecutar como usuario **sxmo**, el bus de sesión rechaza a root).
3. Permanente: wrapper `/usr/local/bin/phosh-session.sh` que hace el
   UpdateActivationEnvironment con `$WAYLAND_DISPLAY` (ya seteado por `phoc -E`) y luego
   `exec /usr/local/libexec/phosh`. El launcher usa `phoc -E /usr/local/bin/phosh-session.sh`.

PIN del lockscreen (usuario `sxmo`): **1234** (`echo "sxmo:1234" | chpasswd`).

## 🔦 Backlight (quick win #1) — registro VALIDADO, driver genérico NO encaja
- ✅ **VALIDADO en HW**: `PWM_DUTY @0x1400A0A0` (0..1023) controla el brillo real
  (barrido con python `mmap /dev/mem`; `busybox devmem` no está compilado).
  `BLS_EN @0x1400A000 = 0x10001` (bit16 PWM_EN + bit0 BLS, ya encendido por dispfix).
- DTS añadido: `pwm@1400a000` (`mediatek,mt2701-disp-pwm`) + 2 fixed-clock (main/mm) +
  `pwm-backlight`. Config `PWM_MTK_DISP=y`. (Ver `mt6582-bq-krillin-quickwins.dts`.)
- **Kernel #26 compilado + flasheado: ARRANCÓ bien (llegó a la GUI), NO colgó — pero COLORES RAROS.**
- ❌ **CAUSA (confirmada en `pwm-mtk-disp.c` L136-143):** el driver mt2701, en **cada**
  `apply` (cada cambio de brillo), escribe `BLS_DEBUG @0xb0 |= 0x3` (**desactiva el
  doble-buffer**) → descuadra el pipeline de color que el `dispfix` deja montado. Síntoma
  idéntico a tocar BLS_DEBUG a mano: "texto legible pero colores raros".
- ✅ **SOLUCIÓN ELEGIDA (pendiente, próxima sesión):** driver de backlight **custom mínimo**
  (estilo `mt6582-dispfix.c`, ~80 líneas): crea `/sys/class/backlight`, en `set_brightness`
  escribe `PWM_DUTY @0xA0` (validado) + re-aplica el commit del MUTEX, **conviviendo con el
  dispfix sin romper colores**. NO usar `pwm-mtk-disp` genérico.

## 🔧 Ciclo kernel → flash dominado (lecciones clave)
- **Cambiar `.config`:** `./scripts/config --file build-krillin/.config --enable X` +
  `make ARCH=arm O=build-krillin olddefconfig` **ANTES** de `make ... zImage dtbs`
  (si no, no recompila el kernel; solo el dtb).
- **Empaquetar boot.img:** `cat zImage dtb > z; python3 mtk_hdr.py KERNEL z z-mtk;
  abootimg --create out.img -f /tmp/asm/cfg2 -k z-mtk -r /tmp/asm/initrd.img`.
- **fastboot:** el **LK KitKat NO soporta `fastboot boot`** (cargar en RAM) → da timeout y
  arranca el de la eMMC. Usar **`fastboot flash boot <img>`** (permanente). `reboot bootloader`
  **no** entra en fastboot (falta driver reboot-mode) → fastboot **solo manual** (Power+Vol↑).
- **Backup del #25 bueno = `~/mainline/pkg/boot-simpledrm.img`** (re-flashear para recuperar).
- Sensores `accel@1d`/`light@39` **NO** fueron el problema (i2cget 0x1d 0x0f = `0x40` = WAI del
  LSM330, OK; módulos `=m` no instalados, falta `make modules`).

## Estado al consolidar
Teléfono en **#25 bueno**: Phosh + apps + teclado + colores OK. Todo guardado (memoria + GitHub).

## Próximo (primer paso de la próxima sesión)
Escribir el **driver de backlight custom** (`mt6582-backlight.c`) → quick win #1 completo.
Luego: power-button (revertir patch mt6397-core + DT) y sensores (`make modules` + DT).
