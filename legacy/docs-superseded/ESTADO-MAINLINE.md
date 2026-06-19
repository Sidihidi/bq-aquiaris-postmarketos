# Estado del port MAINLINE — BQ Aquaris E4.5 (MT6582 "krillin")

Linux **mainline 7.0.12** en el MT6582, partiendo de un SoC casi sin soporte upstream.
Resumen del estado a 2026-06-16 (sesión maratón). Detalles en los HITO-*.md.

## Qué FUNCIONA
| Subsistema | Estado | Cómo |
|---|---|---|
| Arranque kernel 7.0.12 | ✅ | LK (KitKat 1.5.2) → boot.img (abootimg + MTK hdr) |
| Display | ✅ legible | simplefb r5g6b5/stride1088 + `mt6582-dispfix.c` (late_initcall reprograma el OVL a RGB565). Ver HITO-M3-DISPLAY-RESUELTO.md |
| eMMC (mtk-sd) | ✅ | mmc0, p1..p7 vía blkdevparts del LK |
| USB gadget (red) | ✅ ping | `mt6582-musb.c` v16 (g_ether). Ver HITO-M2b-USB-RESUELTO.md |
| SSH | ✅ | dropbear/openssh por USB. Ver HITO-M2d-SSH-RESUELTO.md |
| Rootfs | ✅ | Alpine 3.24.1 en p7, OpenRC, apk. Ver HITO-ROOTFS-ALPINE.md |
| I2C | ✅ | `i2c-mt65xx` + nodo DT (mt6577-compat). Ver HITO-I2C-TOUCH.md |
| GPIO (poke) | ✅ | devmem.c (mmap), layout mt_gpio_base |

## Qué FALTA (roadmap)
1. **PMIC MT6323** (pwrap mt6582 + DT) — desbloquea **touch** (VGP1), **batería**, **carga**.
   Drivers ya en mainline; falta compatible mt6582 + DT. ← siguiente sub-proyecto.
2. **Touchscreen** FT5336 — I2C+reset listos, falta power (VGP1) + EINT117. → GUI táctil.
3. **EINT** (controlador de interrupciones GPIO) — para el IRQ del touch y otros.
4. **GUI** (Sxmo) sobre simplefb una vez haya touch.
5. WiFi, audio, GPU Mali-400 (¿lima?), modem (3G, lo más difícil).
6. Empaquetar como port pmaports (kernel mainline + deviceinfo + APKBUILD).

## Imágenes de arranque (en la Pi ~/mainline/pkg/)
- `boot-mainline-rootfs.img` — **la principal**: switch_root a Alpine en p7.
- `boot-mainline-ssh.img` — initramfs de rescate (busybox+dropbear, no monta p7).
- `boot-mainline-i2c.img` — como rootfs pero con el kernel que trae I2C.
- `boot-mainline-m2c.img` — initramfs mínimo (display por kernel + usb0).

## Flujo de trabajo
- Entorno: Raspberry Pi (cpcd@192.168.0.38) compila (cross armhf) y habla con el teléfono
  por USB (fastboot + usb0 172.16.42.x). El teléfono se conecta a la Pi por USB.
- Reflashear: teléfono → **fastboot MANUAL** (Power+Vol+, batería dentro) →
  `fastboot flash boot <img>`.
- Una vez en Alpine: `ssh root@172.16.42.1`; para internet (apk) → NAT en la Pi.
- **GOLDEN RULE** (softbrick): reinstalar KitKat 1.5.2 con SP Flash Tool "Download Only"
  (NUNCA "Format All": borra NVRAM/IMEI). El LK de KitKat es permisivo; el de Lollipop
  verifica firma y hace bootloop con kernels propios.

## Artefactos clave (este repo, carpeta mainline/)
- `drivers-musb/mt6582-musb.c` (v16), `drivers-video/mt6582-dispfix.c`
- `dts/mt6582-bq-krillin.dts` (simplefb r5g6b5 + usb + i2c0)
- `init/` (init-switchroot.sh, init-ssh.sh, tiny_init_m2c/m3v3.c), `tools/devmem.c`
- `config-mainline-krillin` (.config del kernel)
