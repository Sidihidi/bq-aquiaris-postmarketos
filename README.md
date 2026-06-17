# BQ Aquaris E4.5 (`krillin`) — Linux en un MediaTek MT6582 de 2014

Reviviendo el **BQ Aquaris E4.5** (MediaTek MT6582, Cortex-A7 ×4 armv7, Mali-400 MP2,
1 GB RAM, 540×960; *el primer Ubuntu Phone, 2014*) con Linux libre. El proyecto tiene
**dos vías**, ambas funcionando en hardware real:

1. 🟢 **postmarketOS sobre el kernel 3.10 downstream** — estable, con GUI Sxmo y táctil.
   Port construido desde cero (no existía). Detalle: [README-PMOS-3.10.md](README-PMOS-3.10.md).
2. 🚀 **Linux MAINLINE 7.0.12 + Alpine 3.24** — la frontera: levantar un SoC casi sin
   soporte upstream, escribiendo/portando drivers. **Es donde está la acción ahora.**

> **Hito mayor (2026-06-17):** un teléfono de 2014 corriendo **Linux mainline 7.0.12**
> con **escritorio gráfico X11 táctil fluido** y **carga de batería por USB** funcionando.

📍 **Empieza por [INDICE.md](INDICE.md)** (mapa + estado + accesos) y, para retomar entre
sesiones/máquinas, [TRASPASO-SESION.md](TRASPASO-SESION.md).

---

## Estado — qué FUNCIONA

### Vía MAINLINE (Linux 7.0.12 + Alpine 3.24.1)
| Subsistema | Estado | Cómo |
|---|---|---|
| Arranque kernel (SMP 4 CPUs) | ✅ | boot.img formato MTK; el LK carga el zImage+dtb |
| Display (color correcto) | ✅ | `simplefb` RGB565 + `mt6582-dispfix.c` (reprograma el OVL) |
| eMMC | ✅ | `mtk-sd` + `blkdevparts=` (el parser MBR no traga el de MTK) |
| USB gadget (red) + **SSH** | ✅ | `mt6582-musb.c` (g_ether) + dropbear/openssh por USB |
| Rootfs | ✅ | **Alpine 3.24.1** en p7 (OpenRC, apk) |
| I2C / GPIO | ✅ | `i2c-mt65xx` (compat mt6582) / poke por `/dev/mem` |
| **Táctil** (FT5336) | ✅ | VGP1 (PMIC vía pwrap) + I2C0@0x38 + uinput → `/dev/input/event0`; reset GPIO115 fiable |
| **GUI X11 táctil** | ✅ | Xorg fbdev + jwm + matchbox-keyboard — **fluido**, al dedo |
| **Carga por USB** | ✅ | driver userspace del **FAN5405** (I2C0@0x6a): 4.20V/800mA + watchdog |
| **PMIC MT6323** (hub) | ✅ | pwrap (`mediatek,mt6582-pwrap`) + MFD + **31 reguladores** (vgp1@2.8V). [HITO](mainline/HITO-PMIC-MT6323.md) |
| **Batería %** | ✅ | VBAT por AUXADC MT6323 (`AUXADC_ADC0`) vía `pwrap_poke` — `/usr/local/bin/battery`. [HITO](mainline/HITO-BATERIA-WIP.md) |
| WiFi / Audio / GPU / Módem | ⬜ | energía ya disponible (VCN33_WIFI, VA…); ver [ROADMAP-DRIVERS.md](ROADMAP-DRIVERS.md) |

### Vía postmarketOS (kernel 3.10)
Arranca, consola + SSH por USB, **GUI Sxmo (X11)**, **táctil**, **WiFi**. Sin GPU
(el 3.10 no da Lima). Detalle técnico completo en [README-PMOS-3.10.md](README-PMOS-3.10.md).

---

## Hitos del bring-up mainline (en hardware real)
- **M1** — arranque + consola (`simplefb`) + userspace, 4 CPUs SMP.
- **M2** — eMMC + particiones + ext4 (montó la raíz y la listó en pantalla).
- **M2b** — USB gadget (driver MUSB propio `mt6582-musb.c`).
- **M2d** — **SSH por USB** (busybox+dropbear / openssh). → [HITO](mainline/HITO-M2d-SSH-RESUELTO.md)
- **M3** — display con color correcto (`mt6582-dispfix.c`). → [HITO](mainline/HITO-M3-DISPLAY-RESUELTO.md)
- **Rootfs** — Alpine 3.24 en p7. → [HITO](mainline/HITO-ROOTFS-ALPINE.md)
- **I2C/GPIO + Táctil** — FT5336 lee y emite eventos. → [HITO](mainline/HITO-TOUCH-DRIVER.md) · [CONTINUAR-TOUCH](mainline/CONTINUAR-TOUCH.md)
- **GUI X11** — escritorio táctil fluido. → [HITO](mainline/HITO-GUI-X11.md)
- **Carga FAN5405** — el teléfono carga en mainline. → [HITO](mainline/HITO-CARGA-FAN5405.md)

Guía técnica de compilar/flashear/depurar el kernel mainline: [mainline/README.md](mainline/README.md).

---

## Hardware
| Dato | Valor |
|---|---|
| SoC | MediaTek **MT6582** (4× Cortex-A7, armv7) |
| GPU | ARM **Mali-400 MP2** (Lima viable en kernel ≥5.2 → tenemos 7.0.12) |
| RAM / eMMC | 1 GB / 8 GB |
| Pantalla | 540×960 qHD, panel DSI hx8389 (Truly), táctil **FT5336** (I2C0@0x38) |
| PMIC | **MT6323** (vía pwrap @0x1000D000); reguladores incl. VGP1 |
| Cargador | **Fairchild FAN5405** (I2C0@0x6a) |
| Bootloader | LK MediaTek; **desbloqueado** (`unlocked: yes`, secure boot off) |

Direcciones clave: GPIO `0x10005000`, I2C0 `0x11007000`, MUSB `0x11200000`, fb `0xBF400000`.

---

## Recuperación — el teléfono es difícil de brickear
- **Regla de oro MTK:** no flashear NUNCA `preloader`. Lo demás es recuperable.
- **fastboot** (funciona en la Pi): `fastboot flash boot <img>` (estamos unlocked).
- **mtkclient / BROM** (red de seguridad): backup golden completo en `artifacts/golden/`
  (incl. preloader). Restaurar: `mtk w <part> <part>.img`. Guía: [GUIA-MTKCLIENT.md](GUIA-MTKCLIENT.md).
- **Softbrick** → reflashear KitKat stock con SP Flash Tool "Download Only" (NUNCA "Format All").
- **TWRP 3.0.2.0** instalado y permanente en `recovery`.

Estrategia de unlock / dual-boot / bootloader propio: [ESTRATEGIA-CUSTOM-BOOTLOADER.md](ESTRATEGIA-CUSTOM-BOOTLOADER.md).

---

## Cómo se trabaja
Una **Raspberry Pi** (`cpcd@192.168.0.123`) compila (cross armhf) y habla con el teléfono
por USB (fastboot + red usb0 `172.16.42.x`). El teléfono mainline se accede desde la Pi:
`ssh root@172.16.42.1`. El pmOS: `ssh user@172.16.42.1` (pass `147147`).

> ⚠️ No martillear SSH anidado Pi→teléfono: satura el sshd / el musb no sostiene tanto TCP.

---

## Próximos pasos (ver [ROADMAP-DRIVERS.md](ROADMAP-DRIVERS.md))
1. **Batería %** (AUXADC) — ver el nivel en la UI.
2. **PMIC MT6323 al DeviceTree** — el hub que ordena power/carga/audio.
3. **WiFi**, **Audio**, **EINT** (IRQ táctil + botones).
4. **Ruta a Phosh:** `simpledrm` → `/dev/dri/card0` → Phosh-Pixman → **lima (Mali-400)** → Phosh acelerado.
5. **Módem 3G** — el más difícil (camino realista: Halium sobre el 3.10).

---

*Proyecto de aficionado, bring-up en hardware real. Repo:
[github.com/Sidihidi/bq-aquiaris-postmarketos](https://github.com/Sidihidi/bq-aquiaris-postmarketos).
Todo el historial y los detalles en los `HITO-*.md` y en `INDICE.md`.*
