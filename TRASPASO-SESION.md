# 🔄 TRASPASO DE SESIÓN — continuar en casa

> Lee esto primero, luego **[INDICE.md](INDICE.md)** (mapa + accesos).
> Última sesión: **2026-06-17 (tarde)**. Antes: 2026-06-17 (mañana), 2026-06-16, 2026-06-10.

---

## 🟢 ESTADO AL CIERRE (2026-06-17 tarde)

El teléfono corre **Linux MAINLINE 7.0.12 + Alpine 3.24** como sistema principal, kernel **#22**.
Sesión muy productiva. **Funciona TODO esto** (verificado en hardware):

| Subsistema | Estado |
|---|---|
| Arranque SMP 4×A7, display color OK (simplefb+dispfix), eMMC, USB-gadget+**SSH** | ✅ |
| I2C + GPIO (poke), **táctil FT5336** (userspace→event0), **GUI X11** (jwm) | ✅ |
| **Carga** (daemon FAN5405) | ✅ |
| 🆕 **PMIC MT6323** (pwrap + MFD + **31 reguladores**, vgp1@2.8V, Chip ID 0x2023) | ✅ |
| 🆕 **Batería %** (`battery`: VBAT por AUXADC, 3719mV ~33%) | ✅ |
| 🆕 **sshd al boot** (levanta solo, sin restart manual) | ✅ |
| fastboot (recuperado) + BROM/mtkclient (recuperación) | ✅ |

🔓 Bootloader desbloqueado, golden backup completo, TWRP. **Nada roto ni urgente.**

### Qué hicimos hoy (tarde)
1. **PMIC MT6323 al DeviceTree** (3 arreglos de driver) → [HITO-PMIC-MT6323.md](mainline/HITO-PMIC-MT6323.md).
2. **Recuperación de "fastboot roto"** por BROM **sin reinstalar Android** — y descubrimos que **`wf` está roto** (escribe en sector 0); el comando bueno es **`wo`** → [reference en el HITO].
3. **sshd robusto al boot** → `mainline/rootfs/sshd/`.
4. **Batería %** (reverse del AUXADC MT6323) → [HITO-BATERIA-WIP.md](mainline/HITO-BATERIA-WIP.md).

---

## ▶️ SIGUIENTE PASO: EINT (interrupciones) — **plan completo listo**

📋 **[mainline/HITO-EINT-PLAN.md](mainline/HITO-EINT-PLAN.md)** — toda la investigación hecha.
Resumen: hay que escribir un **driver gpio+eint del MT6582** (~250-300 líneas; la librería
`mtk-eint` exige un `gpio_chip` detrás, no vale un stub). Datos ya resueltos: registros ==
`mtk_generic_eint_regs`, base `0x1000B000`, 169 EINTs, IRQ `GIC_SPI 113`, táctil `EINT117` falling,
layout GPIO base `0x10005000`. Desbloquea: táctil-por-IRQ, **botones** (gpio-keys), y el driver
de kernel `edt-ft5x06` (que retiraría el daemon userspace del táctil).

Después de EINT, el roadmap (ver [ROADMAP-DRIVERS.md](ROADMAP-DRIVERS.md)): **WiFi** (VCN33_WIFI
ya disponible), **Audio** (VA/VRF18), y la **ruta a Phosh** (simpledrm→/dev/dri/card0→lima Mali-400).

---

## 🔧 CÓMO FLASHEAR EL KERNEL (dos vías)

El boot actual (#22) está en la Pi: `~/mainline/pkg/boot-pmic.img` (ANDROID!, 0xD18800 B).
Para reconstruir tras compilar: `cat zImage dts/mediatek/mt6582-bq-krillin.dtb > zimage-dtb;
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk; abootimg --create boot-pmic.img -f
/tmp/asm/cfg2 -k zimage-dtb-mtk -r /tmp/asm/initrd.img` (cfg2 = bootimg.cfg SIN la línea bootsize).

- **Fastboot** (funciona ahora): batería fuera/dentro → Vol+ + USB → `sudo fastboot flash boot ~/mainline/pkg/boot-pmic.img && sudo fastboot reboot`.
- **BROM** (si fastboot falla): `sudo ~/mtkclient/venv/bin/python ~/mtkclient/mtk.py wo 0x2900000 0xD18800 ~/mainline/pkg/boot-pmic.img` (reconnect-on-cue). **OJO: `wo`, NO `wf`.**

---

## ⚡ HECHOS CLAVE / TRAMPAS

1. **mtkclient: usar `wo <offset> <length> <img>`, NUNCA `wf`** — `wf` ignora el offset y
   escribe en **sector 0** (machaca MBR/proinfo). Offsets HEX: boot=`0x2900000`, uboot=`0x28A0000`,
   seccfg=`0x2880000`. Detalle en [[reference-mtkclient-krillin]] (memoria) y el HITO-PMIC.
2. **mtkclient en Pi5 = interactivo + reconnect-on-cue**: lanzar, y al "please reconnect"
   batería fuera→dentro + Vol− + USB. `sudo systemctl stop ModemManager` antes; `~/mtkclient/hwparam.json` debe ser de `cpcd`.
3. **NO restaurar el LK golden** — es de **Lollipop** (verifica firma → bootloop). El del equipo
   es KitKat 1.5.2 permisivo (`~/stock-1.5.2/lk.bin`). fastboot se recupera solo al arrancar un kernel limpio.
4. **PMIC pwrap**: la IRQ tormenteaba → driver sin `request_irq` (polling). MFD mt6397 con IRQ opcional.
5. **El LK ignora el cmdline del boot.img** → bootargs por el DT (`CMDLINE_EXTEND`).
6. **No martillear SSH anidado Pi→teléfono** (satura el sshd del móvil). Una sesión, comandos agrupados.
7. **Scripting**: `$(())`/`{}` se rompen en zsh(Mac)→ssh→bash; usar heredoc `<<'EOF'`.

---

## 🔌 ACCESOS

| Qué | Valor |
|---|---|
| Pi de construcción | `ssh cpcd@192.168.0.123` (Raspberry Pi 5, clave del Mac, sudo sin pass) |
| Teléfono mainline (desde la Pi) | `ssh root@172.16.42.1` (IP Pi en usb0: `172.16.42.2/24`) |
| Comandos útiles en el teléfono | `battery` (% batería), `charge-status`, `/usr/local/bin/pwrap_poke r/w` |
| mtkclient | `~/mtkclient/venv/bin/python ~/mtkclient/mtk.py ...` (sudo, interactivo) |
| Teléfono | unlocked, secure off; serial JB053237; CID 303847391101003022b9315f32009a1e |
| GitHub | https://github.com/Sidihidi/bq-aquiaris-postmarketos |

**Artefactos** (NO en GitHub): en la Pi `~/mainline/` (kernel, pkg, boot-pmic.img #22),
`~/golden/`, `~/stock-1.5.2/`; en el Mac `artifacts/` y `~/Downloads/1.5.2_krillin/`.
Rootfs Alpine del teléfono en p7 (cambios en `/etc/local.d/`, `/usr/local/bin/`; espejo en `mainline/rootfs/`).

> ⚠️ "Casa" = otra máquina: clona el repo (docs) + ssh a la Pi (artefactos). Las imágenes no van a GitHub.
