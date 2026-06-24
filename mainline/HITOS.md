# HITOS — Port de Linux mainline al BQ Aquaris E4.5 ("krillin", MediaTek MT6582)

Historia unificada del proyecto: arrancar un kernel **mainline moderno (Linux 7.0.12)** + un
userspace **Alpine/postmarketOS** moderno en un teléfono MediaTek MT6582 de 2014, hasta llegar
a **Phosh** con táctil, GPU, carga, batería, y la saga de conectividad **CONSYS** (WiFi/BT/GPS/FM).

Este documento sustituye a los `HITO-*.md` sueltos como referencia histórica. Para cada hito:
qué se logró, el detalle técnico clave (registros / secuencias / causa raíz) y los ficheros
implicados. Cuando hubo versión WIP/PLAN y RESUELTO (M2b, EINT), se documenta la resuelta.

Plataforma confirmada: 4× Cortex-A7 @ MT6582, 1 GB RAM, panel qHD 540×960 (HX8389 / Truly OGS),
táctil Focaltech FT5336, PMIC MT6323, cargador FAN5405, GPU Mali-400 MP2, combo CONSYS MT6582.

---

## Índice

| # | Hito | Estado | Fecha |
|---|------|--------|-------|
| 1 | M1 — Primer boot mainline (SMP + simplefb) | ✅ RESUELTO | 2026-06-10 |
| 2 | M2b — Red por USB (gadget Ethernet, MUSB) | ✅ RESUELTO | 2026-06-16 |
| 3 | M2d — SSH por USB (shell root) | ✅ RESUELTO | 2026-06-16 |
| 4 | M3 — Display legible (fin del tinte amarillo) | ✅ RESUELTO | 2026-06-16 |
| 5 | Rootfs Alpine + mainline en eMMC (pmOS moderno) | ✅ RESUELTO | 2026-06-16 |
| 6 | I2C + GPIO + táctil FT5336 (lectura validada) | ✅ RESUELTO | 2026-06-17 |
| 7 | PMIC MT6323 al DeviceTree (el "hub") | ✅ RESUELTO | 2026-06-17 |
| 8 | Touch-power: servicio OpenRC al arranque | ✅ RESUELTO | 2026-06-16 |
| 9 | Driver de touch FT5336 (uinput, userspace) | ✅ RESUELTO | 2026-06-17 |
| 10 | GUI X11 táctil | ✅ RESUELTO | 2026-06-17 |
| 11 | Carga por USB (FAN5405) | ✅ RESUELTO | 2026-06-17 |
| 12 | Batería % (lectura de VBAT por AUXADC) | ✅ RESUELTO | 2026-06-17 |
| 13 | Wayland (weston) sobre simpledrm | ✅ RESUELTO | 2026-06-17 |
| 14 | Phosh arranca | ✅ RESUELTO | 2026-06-17 |
| 15 | Quick wins: apps, OSK; backlight (diagnóstico) | 🟡 PARCIAL | 2026-06-17 |
| 16 | EINT (interrupciones externas) MT6582 | ✅ RESUELTO | 2026-06-17 |
| 17 | Keypad (volumen) MT6582 | 🟡 PARCIAL (Vol↓ OK) | 2026-06-17 |
| 18 | lima / Mali-400 (GPU) | 🟡 PARCIAL → ✅ (ver nota) | 2026-06-18 |
| 19 | CONSYS WiFi/BT/GPS/FM — la saga | 🟡 EN CURSO (gran avance) | 2026-06-18→22 |
| 20 | Plugins de Phosh (quick-settings + lockscreen, 17) | ✅ RESUELTO | 2026-06-24 |
| 21 | Batería en Phosh — indicador % + cargando/descargando | ✅ RESUELTO | 2026-06-25 |
| 22 | Sesión elogind activa + diagnóstico del slider de brillo | 🟡 (sesión ✅, slider ⬜) | 2026-06-24 |
| 23 | WiFi WPA2 — scan-crash resuelto + CCMP diagnosticado | 🟡 (host OK, CCMP = hueso FW) | 2026-06-24 |

Estado global a **2026-06-22**: Display DRM nativo + Phosh con GPU lima; táctil end-to-end;
carga + batería; **WiFi escanea 16 redes reales vía `iw dev wlan0 scan`** (falta connect/WPA2);
**GPS protocolo `0xAAF0` decodificado** (falta 1 recaptura); **BT emparejado con un S24**.

Actualización **2026-06-25**: **stack Phosh muy avanzado** — 17 plugins compilados (toggles del
panel), **batería completa en la UI** (% real + icono de carga con rayo), **sesión elogind activa**
conseguida (base para suspend/power). WiFi: red **ABIERTA navega** (lease+ping); WPA2 asocia pero el
**CCMP de datos es hueso del FW** (host verificado byte-a-byte). Brillo por comando `bl` (el slider
es problema del source de Phosh, no del HW).

---

## 1. M1 — Primer boot mainline (SMP + simplefb) ✅ (2026-06-10)

Primer arranque documentado de Linux 7.0.12 mainline en el MT6582. Kernel desde cero, **sin
driver de display propio**: consola por `simple-framebuffer` sobre el FB que deja el LK.

**Logrado:** boot SMP 4× Cortex-A7 (`8283 BogoMIPS`), gestor de memoria (~926 MB libres de
1 GB), framebuffer/consola, initramfs, userspace estable >4 min (`crng init done`).

**Datos de HW confirmados (base de todo lo siguiente):**
- FB físico `0xBF400000`, 540×960×32, stride 2176; **formato real `a8r8g8b8`** (rojo@16, azul@0).
- 4× Cortex-A7, `mpidr 0x8000000N`.
- **Hueco de RAM seguro para ramoops post-mortem: `0xBF300000`** (sobrevive al reboot) — canal de
  depuración principal junto con la pantalla (no hay UART: `11002000.serial` falla con -2, falta clock).
- Watchdog `10007000.watchdog` (mtk-wdt) OK, timeout 31 s.
- **El LK ignora el cmdline del boot.img** → mainline necesita `CMDLINE_EXTEND`.

**Iterar sin cables serie:** pmOS → `reboot2 bootloader` → fastboot → la Pi flashea
`boot-mainline-vN.img` → mirar pantalla; recuperación combo Power+Vol+ → fastboot. Volcar el
dmesg del intento desde pmOS viva: `memdump 0xBF300000 0x100000`.

**Ficheros:** `dts/mt6582-bq-krillin.dts` (simplefb + ramoops), `dmesg-mainline-v3.txt` (evidencia).

---

## 2. M2b — Red por USB (gadget Ethernet, MUSB) ✅ (2026-06-16)

El kernel enumera como **gadget USB Ethernet** y hay **ping bidireccional** Pi ↔ teléfono
(`ID 0525:a4a2 Linux-USB Ethernet/RNDIS Gadget`, RTT ~0.2 ms). El MT6582 lleva un **Mentor MUSB
con glue MediaTek**, que mainline no soporta para este SoC → driver portado.

**Componentes:**
- Driver glue **`drivers/usb/musb/mt6582-musb.c`** (derivado de `mediatek.c`). Modo peripheral,
  **PIO** (sin Inventra DMA), PHY MTK USB2 inline.
- DT: `usb@11200000` compatible `mediatek,mt6582-musb`, reg MAC `0x11200000` + SIF `0x11210000`,
  IRQ `GIC_SPI 32 LEVEL_LOW`, `dr_mode="peripheral"`.
- `.config`: `USB_MUSB_HDRC=y USB_MUSB_MT6582=y USB_MUSB_DUAL_ROLE=y NOP_USB_XCEIV=y
  USB_GADGET=y USB_ETH=y USB_ETH_RNDIS=y` (g_ether built-in se bindea solo al UDC al boot).

**HW (verificado en el downstream):** MUSB MAC `0x11200000`; SIF/PHY `0x11210000` (regs 8-bit en
`+0x800+off`); clock PHY 48M = `UNIVPLL_CON0 0x10209220` bit26; clock MAC = `PERI_PDN0_CLR
0x10003010` bit10; reset IP = `PERICFG 0x10003000` bit29. **Aviso:** no pokear UNIVPLL/PERI_USB en
caliente — el LK los deja on; tocarlos rompía el MSDC/eMMC.

**Los 3 bugs resueltos:**
1. **"irq 26: nobody cared" → Disabling IRQ** (~5,3 s y muere la enumeración). Causa: el init
   desenmascaraba `USB_L1INTM = 0x1ff` (bits 0..8) → fuentes del agregador L1 que el ISR no
   atiende; como la línea es *level*, nunca baja → IRQ_NONE infinito. **Fix:** `L1INTM = TX|RX|USBCOM`
   (`0x7`) y entrar al core ISR solo si `L1INTS & L1INTM` tiene TX/RX/USBCOM. Regs L1
   (`mt_musb_reg.h`): `L1INTS=0xa0 L1INTM=0xa4 L1INTP=0xa8`; `TX=b0 RX=b1 USBCOM=b2 DMA=b3 IDDIG=b9`.
2. **D+ no subía** (el host no veía la conexión tras `SOFTCONN`). Causa: el PHY init era el
   `usb_phy_poweron()` corto y dejaba pull-downs/force-overrides ASSERTED → el pull-up competía con
   un pull-down activo. **Fix:** portar la secuencia **`usb_phy_recover()` completa** del downstream:
   limpiar `0x68` (DPPULLDOWN/DMPULLDOWN/XCVRSEL/TERMSEL), `0x69` (DATAIN), los force de `0x6a`,
   `0x1d` (PUPD_BIST); SET `0x1a`=0x10 (VBUSSCMP_EN); device-mode + slew-cal.
3. **Enumera pero carrier=0** (`usb0` DOWN). **Fix:** el init configura `usb0` por `ioctl`
   (`SIOCSIFADDR 172.16.42.1`, netmask `255.255.255.0`, `IFF_UP`) — el initrd no trae `/sbin/ip`.

**Direcciones:** teléfono `usb0 = 172.16.42.1/24`, host (Pi) `172.16.42.2/24`.

---

## 3. M2d — SSH por USB (shell root) ✅ (2026-06-16)

`ssh root@172.16.42.1` entra a una shell root en mainline por USB (telnet :23 como fallback).
Verificado de paso: 1 GB RAM y **eMMC completa visible** (mmcblk0 + boot0/boot1 + p1..p7 + rpmb).

**Arquitectura:** initramfs = mini-rootfs en RAM con **busybox 1.36.1** estático armhf
(`CONFIG_STATIC=y`), **dropbear 2022.83** estático (auth por clave), init `init-ssh.sh` que levanta
`usb0` + `telnetd` + dropbear, y `authorized_keys` de la Pi. El display lo arregla el kernel
(`late_initcall krillin_dispfix`); el init no lo toca.

**Los 4 fixes:**
1. dropbear no compilaba (`crypt() required`) → deshabilitar password auth
   (`DROPBEAR_SVR_PASSWORD_AUTH 0`), solo pubkey.
2. No linkaba estático (`-fPIE + -static` → "dangerous relocation") → `--disable-harden`.
3. telnetd/dropbear morían al instante: faltaba **devpts** (PTYs) → `mount -t devpts devpts
   /dev/pts` + symlink `/dev/ptmx`.
4. dropbear no escuchaba / rechazaba la clave: `-E` no existe en esta build (sin syslog) → quitarlo;
   y `/root must be owned by ... not writable by group/others` → cpio `--owner=0:0` + `chown -R 0:0
   /root /etc/dropbear`, `chmod 700 /root /root/.ssh`, `chmod 600 authorized_keys`. (No hubo problema
   de NSS: glibc estática resolvió `/etc/passwd` con nsswitch "files".)

**Recetas cross-armhf** (busybox defconfig+STATIC; dropbear `MULTI=1 STATIC=1
--disable-{zlib,syslog,lastlog,utmp,wtmp,harden}`) en el fichero original.

---

## 4. M3 — Display legible: fin del tinte amarillo ✅ (2026-06-16)

El framebuffer mainline (simplefb sobre el panel HX8389 qHD que deja el LK) se veía con fuerte
**tinte amarillo** (negro OK, blanco amarillo) → logs ilegibles. **Resuelto.**

**Causa raíz: DESAJUSTE DE FORMATO DE PÍXEL.** El `simplefb` del DT estaba como `a8r8g8b8` stride
2176 (32 bpp), pero el **OVL** (overlay del MT6582) que *presenta* ese buffer estaba (en el estado
bueno de pmOS) en **RGB565 (CLRFMT=1), pitch 0x440=1088** (16 bpp). El HW leía un buffer de 32 bpp
como 16 bpp → se perdía el canal **azul** → `FFFFFF` → `FFFF00` (amarillo). El negro (0) es
invariante, por eso "tintaba" sin romper la geometría. (Las hipótesis previas — gamma del BLS,
matriz YUV, bypass del COLOR — eran callejones: atacaban el síntoma, no el formato.)

**Método que funcionó:** clonar la config de pmOS (3.10, que pinta bien). Con un kernel pmOS con
`CONFIG_DEVMEM` se leyeron por `/dev/mem` los registros DISP reales y se replicaron. Tabla CLRFMT
del OVL MT6582: **1=rgb565, 2=argb8888, 3=pargb8888**.

**Solución (2 partes):**
1. DT: `simple-framebuffer` → `format="r5g6b5"; stride=<1088>` (recompilar solo el dtb).
2. Reprogramar el OVL en RGB565. Config buena capturada en vivo (extracto):
   `OVL SRC_CON(0x2C)=0x08` (solo L3); `L3 CON(0x90)=0x400010ff`; `L3 SRC_SIZE(0x98)=0x03c0021c`
   (960×540), `ADDR(0xA0)=0xbf400000`, `PITCH(0xA4)=0x440`; `RDMA CON(0x00)=0x3f`;
   `BLS EN(0x00)=0x00010001`; **`MUTEX MOD(0x2C)=0x680 SOF(0x30)=1 EN(0x00)=0x303`** ← el COMMIT real
   (togglear `EN` 0→0x303 latchea; sin él nada se aplica).
   **Bases físicas DISP:** `MMSYS 0x14000000 | OVL 0x14007000 | RDMA 0x14008000 | BLS 0x1400A000 |
   COLOR 0x1400B000 | MUTEX 0x1400E000`.

**Consolidado en el kernel:** la reprogramación vive en **`drivers/video/fbdev/mt6582-dispfix.c`**
(un `late_initcall` que comprueba `of_machine_is_compatible("bq,krillin")` y reprograma el OVL a
RGB565), integrado en `drivers/video/fbdev/Makefile` (`obj-$(CONFIG_FB_SIMPLE) += mt6582-dispfix.o`).
Sustituido más tarde por el DRM nativo (`mtk_drm`) — ver hitos 13 y 19.

**Lección ramoops:** el "console" es circular (`console-size 0x80000`); un printk en bucle lo borra.
Volcar en **líneas atómicas** (snprintf + 1 write, nivel `<0>`) tras `sleep(15)` y luego dormir.

---

## 5. Rootfs Alpine + mainline en eMMC (pmOS moderno) ✅ (2026-06-16)

El teléfono arranca **Alpine Linux 3.24.1** sobre **kernel mainline 7.0.12**, rootfs **persistente
en eMMC**, **OpenRC** + **OpenSSH** (root por clave) + **apk** operativo. Es un postmarketOS moderno
en su núcleo — más moderno que el pmOS "oficial" del E4.5 (kernel downstream 3.10).

**Mapa de particiones** (del `blkdevparts` que pasa el LK; **no hay GPT/MBR**):
`mmcblk0: ... 1048576K@91136K(android), 716800K@1140736K(cache), -@1856512K(usrdata)` →
**p5=android (system 1 GB), p6=cache, p7=usrdata (~5,6 GB)**. El rootfs Alpine va en **p7** (p5 lo
sobrescribe cualquier reinstalación de Android).

**Receta:** `alpine-minirootfs-3.24.1-armhf.tar.gz` → imagen ext4 → `dd` a p7; `apk add openrc
openssh util-linux ifupdown-ng ca-certificates`; runlevels OpenRC estándar (devfs/procfs/sysfs en
sysinit; hostname/bootmisc/networking en boot; sshd/local en default); `/etc/network/interfaces`
usb0 estático; `authorized_keys`; `PermitRootLogin prohibit-password`. Internet para apk vía **NAT
en la Pi** (`ip_forward=1` + MASQUERADE en wlan0 + FORWARD usb0↔wlan0; ruta y DNS en el teléfono).

**Arranque (stage-1 initramfs → switch_root):** `boot-mainline-rootfs.img` = kernel + initramfs
`init-switchroot.sh` que monta p7 y `exec switch_root /newroot /sbin/init`. Si p7 falla, FALLBACK de
emergencia que levanta SSH (busybox+dropbear) para no perder acceso. `boot-mainline-ssh.img` =
initramfs-only de rescate.

---

## 6. I2C + GPIO + táctil FT5336 (lectura validada end-to-end) ✅ (2026-06-17)

I2C, GPIO y la **lectura de coordenadas del táctil FT5336** validados en hardware. El muro real del
táctil era la **alimentación** (regulador VGP1 del PMIC), superado.

**I2C desde cero (hallazgo clave: NO hizo falta pinctrl).** Nodo DT `i2c@11007000` compatible
`mediatek,mt6577-i2c` (mismo IP v1, sin tocar driver), `reg = <0x11007000 0x70>, <0x11000200 0x80>`
(i2c + apdma = `AP_DMA_BASE 0x11000000 + 0x200 + 0x80*id`), `interrupts = GIC_SPI 44`, `clock-div =
<1>` (OBLIGATORIO; sin él `-EINVAL`), `clock-frequency = 100000`. `CONFIG_I2C_MT65XX=y`. Resultado:
`/dev/i2c-0`; `i2cdetect` ve `0x1d, 0x30, 0x39, 0x6a, 0x6b`. **El I2C arranca con los clocks/pines
que deja el LK** → ahorra portar pinctrl-mt6582.

**GPIO por poke** (busybox sin `devmem`): `devmem.c` propio (mmap página+offset, armhf estático).
Layout GPIO MT6582 (`GPIO_BASE = 0x10005000`): `dir@0x000, dout@0x400, din@0x500`; banco = `(gpio>>4)*0x10`,
bit = `gpio&0xf`; VAL_REGS de 16 B (val@+0 / set@+4 / clr@+8).

**Pwrap + VGP1 validados (el touch despierta):**
- **pwrap MT6582 funciona por poke** (`PWRAP_BASE = 0x1000D000`): secuencia wacs2 (`CMD@+0x9C,
  RDATA@+0xA0, VLDCLR@+0xA4`; `cmd=(w<<31)|((adr>>1)<<16)|wdata`; FSM idle=0/wfvldclr=6 en
  bits16-18). El LK deja el pwrap listo. **CID MT6323 leído = 0x2023.**
- **VGP1 = DIGLDO_CON7 (reg PMIC `0x050A`), bit 15.** `pwrap_poke w 0x050A 0x8000` → el FT5336
  aparece en `i2cdetect` (0x38) y devuelve `0x5a` (panel-ID Truly).

**Lectura write-then-read (el fix definitivo del I2C):** el `mt6577_compat` (`auto_restart=0`) daba
timeout en write-then-read (el FT5x06 necesita **repeated-START** real). Solución: nuevo
**`mt6582_compat`** en `drivers/i2c/busses/i2c-mt65xx.c` con `auto_restart=1`, sin quirk COMB,
`max_dma_support=32`, y of_match `{ "mediatek,mt6582-i2c", &mt6582_compat }`; el nodo DT usa
`compatible="mediatek,mt6582-i2c"`. **Resultado:** vendor(0xA8)=`0x5a` (Truly), chipid(0xA3)=`0x14`
(FT5x06), fwver(0xA6)=`0x15`; tocando: TD_STATUS=1, **X=275 Y=759** en 540×960.

**Datos del panel:** Truly OGS 540×960, I2C bus 0 addr `0x38`, reset **GPIO115**, IRQ **EINT117**
(falling). `TPD_POWER_SOURCE = MT6323_POWER_LDO_VGP1`.

**Lección:** encender VGP1 con un `late_initcall` (busy-wait del pwrap) **rompe el boot de sshd** →
hacerlo en userspace tras la red (ver hito 8) o con el driver real en el DT (hito 7).

---

## 7. PMIC MT6323 al DeviceTree — el "hub" ✅ (2026-06-17, kernel #22)

El MT6323 entra entero por el DeviceTree vía el **PMIC wrapper (pwrap)**. **Prerequisito de
batería%, WiFi, BT, audio y de la ruta a Phosh** (gestiona los rails de GPU/display/RF).

**Verificado:** `1000d000.pwrap` probe 0; `mt6323` MFD probe 0; `mt6323-regulator: Chip ID =
0x2023`; **31 reguladores** (VA, VCAMA, …, **VCN33_BT, VCN33_WIFI**, …, **vgp1** = táctil); `vgp1:
2800000uV enabled`; storm de interrupciones inesperadas = 0.

**Los 3 fixes + el DT** (nodo `pwrap@1000d000` compatible `mediatek,mt6582-pwrap` → `mt6323` →
`mt6323-regulator` → `ldo_vgp1`):
1. **pwrap_mt6582 con `caps=0`**: la variante mt2701 tiene `PWRAP_CAP_RESET` → pedía un
   reset-controller inexistente en MT6582 → probe -2.
2. **`int_en_all=0` + saltar `request_irq`**: con la IRQ del pwrap activa había **tormenta infinita**
   `unexpected interrupt int=0x2` que inundaba la consola framebuffer y colgaba el arranque. Poner
   `INT_EN=0` no bastaba (la escritura va al offset de mt2701) → **no registrar el handler** (pwrap
   por polling).
3. **MFD `mt6397-core`: IRQ del PMIC opcional**: el probe exigía `interrupts` (`-ENXIO: IRQ index 0
   not found`). No cableamos la IRQ del PMIC → `platform_get_irq_optional` + saltar `irq_init`.

Config: `CONFIG_MFD_MT6397=y`, `CONFIG_REGULATOR_MT6323=y`. Ficheros: `patches/0002..0004`,
`dts/mt6582-bq-krillin.dts`.

**⚠️ Lecciones de oro de mtkclient / BROM (esta sesión "rompió fastboot" por un storm y se recuperó
por BROM):**
- **`wf --offset` ESTÁ ROTO** en el DA legacy: ignora el offset y escribe en **sector 0** (machaca
  MBR/proinfo). **USAR `wo <offset> <length> <fichero>`** (posicional). Ej.:
  `mtk.py wo 0x2900000 0xD18800 boot.img`. **Offsets en HEX.**
- **Offsets de partición (espacio mtkclient = dumchar + 0xB80000):** `seccfg 0x2880000 · uboot(LK)
  0x28A0000 · boot 0x2900000` (boot = uboot+0x60000). Verificar firma: uboot `88 16 88 58`, boot
  `ANDROID!`. Tamaño `boot-pmic.img = 0xD18800` (13 731 840 B).
- Setup Pi: `systemctl stop ModemManager` (roba el VCOM 0e8d:2000); reglas udev de mtkclient;
  `hwparam.json` propiedad de `cpcd`; correr con el venv. BROM en Pi5 = interactivo (lanzar comando,
  al ver el bucle de espera: batería fuera→dentro + mantener Vol− + conectar USB).
- **El LK golden es de LOLLIPOP (verifica firma → bootloop con kernels propios). NO restaurarlo.** El
  bueno es **KitKat 1.5.2** (permisivo; primeros 247184 B == `stock-1.5.2/lk.bin`).

---

## 8. Touch-power: servicio OpenRC al arranque ✅ (2026-06-16)

El FT5336 necesita su regulador **VGP1** encendido antes de hablarle. Como aún no hay driver
regulador integrado para el touch en el DT, se enciende desde **userspace** en cada arranque.

**Por qué userspace y no kernel:** un `late_initcall` (`mt6582-pmic-fixup.c`) con busy-wait del
pwrap **rompe el boot de sshd** (Alpine arranca, usb0 sube, pero OpenRC se cuelga antes de sshd).
**Lección: nunca encender VGP1 con busy-wait en `late_initcall`.**

**Servicio** `/etc/local.d/touch-power.start` (OpenRC `local`, runlevel default):
```sh
/usr/local/bin/pwrap_poke w 0x050A 0x8000   # MT6323 DIGLDO_CON7 bit15 = VGP1 EN
/usr/local/bin/devmem 0x10005478 0x8        # GPIO115 (reset) LOW
sleep 1
/usr/local/bin/devmem 0x10005474 0x8        # GPIO115 (reset) HIGH
```
Herramientas persistentes en p7: `/usr/local/bin/{pwrap_poke,devmem}`. **Verificado tras reboot
(sin poke manual):** `i2cdetect` ve `0x38`, vendor `0x5a`, chipid `0x14`, sshd OK. Se retirará
cuando el `edt-ft5x06` pida VGP1 con `vin-supply` en el DT.

---

## 9. Driver de touch FT5336 (uinput, userspace) ✅ (2026-06-17)

Driver de espacio de usuario **`ft5336_touch.c`**: polling del FT5336 por I2C0 (0x38), emite el
toque por **`/dev/uinput`** (single-touch + MT protocol B, 1 slot). Pipeline I2C→protocolo→uinput→
evtest completo y validado.

**Hallazgos depurados en HW:**
1. **El I2C del MT6582 solo lee fiable ≤ 8 bytes** (FIFO de 8 B; lecturas más largas → APDMA → que
   en mainline **da timeout**). Verificado: r1/r4/r7/r8 OK; r9/r16/r33 fallan. Por eso el driver lee
   **7 bytes** (status + toque 1) → single-touch. Multitouch real exige trocear o arreglar el apdma.
2. **Detección del dedo por EVENT-FLAG, no por `td_status`** (reg 0x02 inestable: 0x00/0x0f/0xff).
   Fiable: bits 7-6 de `XH` (reg 0x03): `0`=down/`2`=contact → dedo; `1`=up/`3`=none → sin dedo.
   `X = ((XH&0x0f)<<8)|XL` (0x03,0x04); `Y = ((YH&0x0f)<<8)|YL` (0x05,0x06).
3. En reposo el chip entra en **modo monitor** y los regs leen `0xFF` → tratar como "sin dedo".

**Infraestructura de input (completa):** `CONFIG_INPUT_EVDEV=y` ya estaba;
**`CONFIG_INPUT_UINPUT` faltaba**; `CONFIG_TOUCHSCREEN_EDT_FT5X06=y` compilado. Como reflashear los
13,7 MB del boot.img por USB **reinicia el teléfono** (el musb no aguanta la carga), `uinput` se
compiló **como módulo** (`=m`) contra el mismo `.config` (vermagic coincide) y se carga con `insmod`
(74 KB, sin reflashear). `ft5336_touch` crea `/dev/input/event0` (Name="ft5336", PROP_DIRECT).

**Validación:** arrastre limpio capturado `(85,575)→…→(235,320)`, ~50 Hz, 0 timeouts.

**Bloqueante histórico (resuelto por reboot, luego por EINT/DRM):** el reset por **GPIO115 no era
fiable** (readbacks `0x5`/`0xd`) → power-cycles degradaban el chip; solo el reboot lo recuperaba.
La "forma correcta" (nodo `edt-ft5x06` con `reset-gpios` + `interrupts=EINT117` + `vin-supply=&vgp1`)
quedó como objetivo, habilitada por el hito 16 (EINT).

---

## 10. GUI X11 táctil ✅ (2026-06-17)

Escritorio **X11 con táctil** sobre mainline + Alpine. Verificado: fluido en 2D, el dedo controla la
GUI.

**Stack:** Xorg con driver **fbdev** sobre `/dev/fb0` (simplefb 540×960 16bpp RGB565); táctil
`xf86-input-evdev` sobre `/dev/input/event0` (el daemon `ft5336_touch`); WM **jwm** (matchbox-wm no
está en Alpine 3.24); teclado `matchbox-keyboard`; dispositivos **explícitos** en `xorg.conf`
(`AutoAddDevices off`) → sin udev. Config en `mainline/rootfs/x11/`. Render por software (pixman),
aún así fluido. (Más tarde se descubrió que el táctil en X11 fallaba por **ausencia de udev** — ver
hito 13.)

**⚠️ Bloqueante operativo (2×): SIN CARGA en mainline.** El teléfono se quedó sin batería a mitad de
sesión: el USB daba datos pero no cargaba. Mitigación: cargar **apagado** (el LK carga). → Disparó el
hito 11 (carga) como prioridad real.

---

## 11. Carga por USB (FAN5405) ✅ (2026-06-17)

El teléfono **carga la batería corriendo mainline**. "Driver" userspace que doma el cargador
**Fairchild FAN5405** (I2C0 `@0x6a`; vendor=4 en CON3=0x94).

**Problema:** sin nadie tocando el chip, su **watchdog ~22-24 s** expira y revierte a IINLIM=100mA +
OREG~3.54V → entrada < consumo → drena. **Fix:** daemon `/usr/local/bin/fan5405-charge` que cada
**10 s** kickea el watchdog (RMW CON0 bit7) y re-escribe valores seguros:
- **CON2=0x8c** → OREG=35 = **4.20 V** (CV Li-ion; topado por VSAFE).
- **CON1=0xb8** → IINLIM=**800 mA**, TE=1 (termina al llenar), CE=0 (enable).
- IOCHARGE en default **550 mA**.

Regs (`fan5405.h`): CON0 TMR_RST=b7; CON1 IINLIM[7:6]/TE=b3/CE=b2(activo-bajo); CON2 OREG[7:2];
CON4 I_CHR[6:4]; CON6 ISAFE/VSAFE = topes HW. Persistencia: `/etc/local.d/zzy-charge.start`
(start-stop-daemon — el `( cmd & )` por SSH no sobrevive). Scripts en `mainline/rootfs/charge/`.
Seguridad: CV fijo 4.20 V, TE activa, ≤800 mA (~0.4C), VSAFE/ISAFE por HW. Solo con VBUS; OTG/boost off.

---

## 12. Batería % (lectura de VBAT por AUXADC) ✅ (2026-06-17)

Lector userspace **`/usr/local/bin/battery`** (`mainline/rootfs/battery`) por `pwrap_poke`, **sin
recompilar kernel**. Verificado: **VBAT 3719 mV (~33%)**, estable.

VBAT sale del **AUXADC del PMIC MT6323, canal 7 = BATSNS** (NO el AUXADC del SoC). Direcciones clave
(de `upmu_common.c` downstream; la zona ADC `0x0714+`, no la zona CON `0x0742+`):
- **OUT+RDY de BATSNS = `AUXADC_ADC0 = 0x0714`** (bits0-14 valor, bit15 ready).
- **VBUF_EN = `AUXADC_CON11 = 0x0758` bit4.**
- request canal 7 = **`CON22 0x076E` bit7** (ya venía set en vivo, 0x00a0).

**Conversión:** `VBAT_mV = raw × 4 × 1800 / 32768` (r_val=4, full-range=1800, precise=32768) → 4.2 V
≈ raw 19117. Pendiente no bloqueante: curva OCV→% real (hoy piecewise: 3.40 V=0%, 3.70=30, 3.85=60,
4.00=80, 4.20=100); **el voltaje es exacto**.

---

## 13. Wayland (weston) sobre simpledrm ✅ (2026-06-17)

Un compositor **Wayland completo (weston 14)** corre **fluido y táctil** con **simpledrm + renderer
Pixman (software)**. **Implicación clave:** si Wayland por software ya va fluido a qHD, el ancho de
banda del MT6582 basta → **Phosh por Pixman es viable sin GPU**; lima sería un extra.

- Output DRM **540×960@60** (shadow FB), sRGB. libinput asoció **event0 = keypad** y **event1 =
  FT5336**. **simpledrm soporta ATOMIC modesetting** → wlroots/phoc/Phosh OK.

**Receta:** `CONFIG_DRM_SIMPLEDRM=y`, `CONFIG_FB_SIMPLE` off, `mt6582-dispfix` → `obj-y` (mantiene el
fix del OVL) → da `/dev/dri/card0` + `/dev/fb0` (`DRM_FBDEV_EMULATION`). `apk add weston
weston-backend-drm weston-shell-desktop seatd eudev libinput mesa-dri-gallium`. **`eudev` es
OBLIGATORIO** (libinput; sin él weston aborta "no input devices") — **esto también arreglaba el
táctil en X11**. `seatd` + `LIBSEAT_BACKEND=seatd`; `XDG_RUNTIME_DIR=/run/wl`; matar X11/jwm antes
(libera card0); `weston --backend=drm-backend.so --renderer=pixman`.

---

## 14. Phosh arranca ✅ (2026-06-17)

**Phosh 0.55.0** (compilado desde fuente) corriendo sobre mainline + Alpine, sobre `simpledrm`. Un
teléfono de 2014 con la GUI de Librem 5 / PinePhone. `Phosh ready after 4.04s`; `card0 connected
540x960`; phoc sobre simpledrm; táctil por `ft5336_touch`→uinput→libinput.

**El bloqueo clave: schema `org.gnome.shell.keybindings`.** gnome-shell no está en Alpine armhf, pero
phosh hace `g_settings_new("org.gnome.shell.keybindings")` y lee keys concretas → sin el schema,
**SIGABRT** (`Settings schema ... is not installed`). phosh 0.55 lee **exactamente 8 keys**:
`screenshot, toggle-overview, toggle-application-view, toggle-message-tray, screen-brightness-up/-down,
screen-brightness-up-monitor/-down-monitor`. **Solución:** schema mínimo con esas 8 keys (tipo `as`,
default `[]`) → `zz-phosh-shell.gschema.xml` en `/usr/share/glib-2.0/schemas/` + `glib-compile-schemas`.
(Pista: cada relanzamiento delata la siguiente key que falta; extraerlas todas de un golpe con `grep
-hE '#define [A-Z_]*KEY[A-Z_]* +"' src/*.c`. Bajar el schema entero de gnome-shell NO sirve.)

**Lanzamiento:** `udevd` (eudev, obligatorio) + `seatd -g seat`; usuario no-root **`sxmo`** (uid
1000, grupos video/input/seat/audio — phosh/phoc NO corren como root); `XDG_RUNTIME_DIR=/run/user/1000`;
`WLR_RENDERER=pixman`, `LIBSEAT_BACKEND=seatd`, `XDG_DATA_DIRS=/usr/local/share:/usr/share`; `exec
dbus-run-session phoc -E /usr/local/libexec/phosh` (phosh en **libexec**, ruta absoluta). Deps (apk):
`feedbackd, gnome-settings-daemon, gnome-backgrounds, squeekboard, adwaita-icon-theme, mutter, eudev,
seatd` (lista de 736 en `apk-installed.txt`). Backup: `usrlocal.tar.gz`, `launch_phosh.sh`.

**Cada warning = una pieza que falta:** NetworkManager (WiFi en quick-settings), UPower (icono de
batería), backlight (brillo), power button (PMIC pwrkey), suspend/PM.

---

## 15. Quick wins: apps + teclado en pantalla; backlight (diagnóstico) 🟡 (2026-06-17)

**✅ Apps GTK4 + multitarea:** `gnome-calculator/clocks/text-editor` arrancan, multitarea fluida.
GTK4 cae a render software (cairo); los warnings `ZINK vkCreateInstance failed` / `libEGL` son
**esperados e inofensivos**.

**✅ Teclado en pantalla (squeekboard) — RESUELTO.** Problema: el paquete Alpine **no trae el
`.service` D-Bus** → `sm.puri.OSK0 was not provided`. Fix (3 pasos): (1) crear
`/usr/share/dbus-1/services/sm.puri.OSK0.service` (`Exec=/usr/bin/squeekboard`); (2) inyectar
`WAYLAND_DISPLAY` con `UpdateActivationEnvironment` vía **gdbus** (no busctl — no existe en Alpine; y
como usuario **sxmo**, el bus rechaza root); (3) wrapper `/usr/local/bin/phosh-session.sh` que hace
ese UpdateActivationEnvironment y luego `exec /usr/local/libexec/phosh`, lanzado por `phoc -E`.
(Layouts embebidos en GResource — `keyboards/` vacío es normal.) PIN del lockscreen (sxmo): **1234**.

**🔦 Backlight — registro VALIDADO, driver genérico NO encaja (pendiente: driver custom).**
- ✅ **VALIDADO:** `PWM_DUTY @0x1400A0A0` (0..1023) controla el brillo real; `BLS_EN @0x1400A000 =
  0x10001` (bit16 PWM_EN + bit0 BLS, ya on por dispfix).
- ❌ **Causa de "colores raros"** (confirmada en `pwm-mtk-disp.c` L136-143): el driver mt2701, en
  **cada** `apply`, escribe `BLS_DEBUG @0xb0 |= 0x3` (**desactiva el doble-buffer**) → descuadra el
  pipeline de color del dispfix. Síntoma idéntico a tocar BLS_DEBUG a mano.
- ✅ **Solución elegida (pendiente):** driver **custom mínimo** `mt6582-backlight.c` (~80 líneas,
  estilo dispfix): crea `/sys/class/backlight`, en `set_brightness` escribe `PWM_DUTY @0xA0` + re-aplica
  el commit del MUTEX, **sin romper colores**. NO usar `pwm-mtk-disp` genérico.

**Ciclo kernel→flash dominado (lecciones):** cambiar `.config` → `scripts/config --file
build-krillin/.config --enable X` + `make O=build-krillin olddefconfig` **antes** de `make zImage
dtbs`. Empaquetar: `cat zImage dtb > z; python3 mtk_hdr.py KERNEL z z-mtk; abootimg --create out.img
-f cfg2 -k z-mtk -r initrd.img`. **El LK KitKat NO soporta `fastboot boot`** (carga en RAM, timeout)
→ usar **`fastboot flash boot <img>`**; `reboot bootloader` no entra en fastboot → **solo manual**
(Power+Vol↑).

---

## 16. EINT (interrupciones externas) MT6582 ✅ (2026-06-17)

Driver **GPIO + EINT** del MT6582 funcionando. El táctil dispara su línea (EINT117) y el kernel
entrega las IRQ. Valida la infraestructura para **táctil-por-IRQ**, **botones HW** (gpio-keys) y el
driver `edt-ft5x06`.

**Verificado:** `mt6582-eint 1000b000...: MT6582 GPIO+EINT listo (169 pines, irq 25)`;
`/proc/interrupts: 143: mt-eint 117 Edge touch-int-test`; un gpio-keys de prueba sobre EINT117 da un
flood de `KEY_F1` al tocar (el INT del FT5336 pulsa por muestra, ~60-100 Hz).

**Driver `drivers/pinctrl/mediatek/gpio-mt6582-eint.c`:** gpio_chip mínimo (get/set/direction/to_irq)
sobre GPIO `0x10005000` + EINT (librería `mtk-eint`) sobre `0x1000b000`. `mtk_eint_do_init(eint,
NULL)` autoconstruye los 169 pines (`regs=NULL` → usa `mtk_generic_eint_regs`, cuyo layout coincide
EXACTO con el MT6582: stat=0x0 ack=0x40 mask=0x80/set0xc0/clr0x100 sens=0x140 soft=0x200 pol=0x300
dom_en=0x400 dbnc=0x500). `mtk_eint_xt`: `get_gpio_n` 1:1 (eint_n==gpio_n), `set_gpio_as_eint` =
DIR=input. `mt6582_eint_hw`: `port_mask=7, ports=6, ap_num=169, db_cnt=16`. **IRQ padre `GIC_SPI
113`** level-high (`MT_EINT_IRQ_ID=145`, 145−32). Detalles 7.0.12: `gpio_chip` usa `fwnode`; `.set`
devuelve `int`. **Restricción de tamaño:** `mtk_eint_irq_request_resources()` llama a
`gpiochip_lock_as_irq()` → la librería exige un `gpio_chip` real (no vale stub) → por eso el gpio_chip
mínimo.

**DT:** nodo `interrupt-controller@1000b000` compatible `mediatek,mt6582-eint`, `reg = <0x1000b000
0x1000>, <0x10005000 0x1000>`, `interrupts = GIC_SPI 113 LEVEL_HIGH`. Kconfig `GPIO_MT6582_EINT`
(`select EINT_MTK GPIOLIB IRQ_DOMAIN`). **Construcción del boot.img:** `abootimg --create` (no `-u`:
el zImage nuevo es ~4 KB mayor y no cabe en el hueco fijo).

---

## 17. Keypad (volumen) MT6582 — Vol↓ funcionando 🟡 (2026-06-17)

El controlador de matriz de teclado (KPD) funciona con el driver `mediatek,mt6779-keypad` **sin
cambios** (offsets idénticos a MT6779/MT8516). **Vol↓ genera `KEY_VOLUMEDOWN`** ✓.

**HW del KPD:** base **`0x10011000`**; `KP_STA@0x00`, `KP_MEM1..5@0x04..0x14` (bits[15:0] = estado,
1=no pulsada/0=pulsada), `KP_DEBOUNCE@0x18`, `KP_SEL@0x20`, `KP_EN@0x24`; **IRQ `GIC_SPI 116`** EDGE;
clock "kpd" = `fixed-clock` (el LK lo deja activo). DT: `keypad@10011000` con `num-rows=1
num-columns=2`, keymap `(0,0)=VOLUMEUP (0,1)=VOLUMEDOWN`. `CONFIG_KEYBOARD_MT6779=y`.

**Vol↑ NO responde:** leyendo `KP_MEM1..5` por devmem, su pulsación no baja ningún bit (reposo ==
pulsado), y `KP_SEL 0x10011020` no acepta escritura → no se puede forzar su escaneo por SW. Necesita
**configuración de pin (pull-up/pinmux)** que mainline no hace (no hay pinctrl-mt6582). **Pendiente:**
mapear el GPIO de la columna de Vol↑ y forzar su pull, o portar pinctrl. (Diagnóstico: devmem a los
regs KP — 1=suelta, 0=pulsada; OJO no confundir `KP_EN @...24` con `KP_SEL @...20`.)

---

## 18. lima / Mali-400 MP2 (GPU) 🟡→✅ (2026-06-18)

Objetivo: aceleración GPU (Mesa lima) para que Phosh pase de pixman a acelerado. El DRM real
(`mtk_drm`) = prerequisito cumplido. **Estado a 2026-06-22 (roadmap): Phosh corre con GPU lima** — la
secuencia de encendido del MFG que faltaba abajo se resolvió.

**Lo que se logró el 18-jun (kernel #34):** `CONFIG_DRM_LIMA=y` (+ DRM_GEM_SHMEM_HELPER + DRM_SCHED);
lima acepta `"arm,mali-400"`. Nodo DT `gpu@13010000` (`reg = <0x13010000 0x10000>`; interrupts
**GIC_SPI 170-175** = gp/gpmmu/pp0/ppmmu0/pp1/ppmmu1; clocks fixed-stub bus 312MHz + core 416MHz).
lima **encontró el GPU** (`13010000.gpu` proba).

**El muro (resuelto después): el power-domain del MFG estaba apagado** → `gpmmu dte write test fail`
→ -5 (EIO) (el LK no deja el GPU encendido para mainline). **Secuencia de encendido del MFG**
(`mt_spm_mtcmos.c:spm_mtcmos_ctrl_mfg`), **SPM físico `0x10006000`**:
- `SPM_POWERON_CONFIG_SET=0x10006000` (unlock, key `0x0b160001`); `SPM_MFG_PWR_CON=0x10006214`;
  `SPM_PWR_STATUS=0x1000660c` (bit4=MFG); `SPM_PWR_STATUS_S=0x10006610`.
- Bits de MFG_PWR_CON: `PWR_RST_B=1<<0, PWR_ISO=1<<1, PWR_ON=1<<2, PWR_ON_S=1<<3, PWR_CLK_DIS=1<<4,
  MFG_SRAM_ACK=1<<12, SRAM_PDN≈1<<8`.
- Power-ON: unlock → `|= PWR_ON; |= PWR_ON_S`; esperar `PWR_STATUS & PWR_STATUS_S` bit4 → `&=
  ~PWR_CLK_DIS; &= ~PWR_ISO; |= PWR_RST_B; &= ~SRAM_PDN`; esperar `!(MFG_PWR_CON & 1<<12)`.

Implementación: poke del MFG en el kernel **antes** de lima (early_initcall o mini-genpd) o `=m` +
script userspace para probar. La misma plantilla MTCMOS se reutilizó para el CONSYS (hito 19).
Ficheros: `drivers/.../mt6582-mfg-power.c`, nodo `gpu@13010000` en el DT. Pendiente fino: **color
rojizo leve** del display (formato OVL RGB_SWAP/BYTE_SWAP en `mtk_disp_ovl.c`, o gamma).

---

## 19. CONSYS — WiFi / BT / GPS / FM (la saga) 🟡 EN CURSO (2026-06-18 → 22)

El driver más duro: **no hay soporte mainline** para el combo de conectividad del MT6582. El CONSYS
es el **conectividad-combo** del SoC (WiFi+BT+GPS+FM como "funciones" del mismo **WMT** sobre el mismo
canal **BTIF**: `WMTDRV_TYPE_BT=0 FM=1 GPS=2 WIFI=3`). Se levanta el chip una vez y cada radio se
activa con `mtk_wcn_wmt_func_on(tipo)` exponiendo su interfaz (`stpbt` BT/HCI, `stpgps` GPS/NMEA, FM,
`wmtWifi`). Tras el bring-up, **BT y GPS son los más fáciles** (char devs → BlueZ / gpsd) y **WiFi el
más duro** (802.11/cfg80211, ~133K líneas downstream).

### Mapa de HW (físico MT6582, investigado del downstream)
| Bloque | Físico | Uso |
|---|---|---|
| SPM | `0x10006000` | power MTCMOS del CONSYS (reusa `mt6582-mfg-power.c`) |
| INFRACFG_AO | `0x10001000` | protección bus TOPAXI (0x220/0x228) |
| TOPRGU (AP_RGU) | `0x10007000` | reset MCU CONSYS (0x18 bit12, key `0x88<<24`) |
| CONN_MCU_CONFIG | `0x18070000` | **CHIP_ID @+0x08 (=0x6582)**, ACR @0x110, DELSEL @0x114 |
| CONN_TOP_CR | `0x180B0000` | AFE/PLL WiFi (@+0x2000) |
| CONN_HIF_CR | `0x180F0000` | **HIF AHB del WiFi** (WCIR @+0x0 = `0x00106582`) |
| BTIF | `0x1100C000` | transporte serie AP↔CONSYS (STP); **no existe en mainline** |

IRQ (SPI): `CONN_WDT=163, WF_HIF=164 (el del WiFi, LEVEL_LOW), BTIF_WAKEUP=165, MT_BTIF=50, BTIF
DMA TX/RX=71/72`. Reguladores (MT6323): `ldo_vcn18 1.8V, vcn28 2.8V, vcn33_wifi 3.3V, vcn33_bt`.

### Secuencia de encendido del CONSYS (plantilla MFG con otra dirección)
`SPM+0x280 = CONN_PWR_CON`, ack mask `1<<1` (vs MFG `0x214`/`1<<4`): PMIC VCN_1V8+VCN28(+VCN33_WIFI)
→ SPM unlock `0x0b160001` → `CONN_PWR_CON |= PWR_ON; |= PWR_ON_S` → esperar `PWR_STATUS(0x60c) &
PWR_STATUS_S(0x610)` bit1 → `&= ~PWR_CLK_DIS; &= ~PWR_ISO; |= PWR_RST_B; &= ~SRAM_PDN` → `TOPAXI_PROT_EN
(INFRA+0x220) &= ~0x104`, esperar `PROT_STA1(0x228)&0x104==0` → ungate clock CONNMCU → poll
`CHIP_ID(0x18070008)==0x6582`.

### Progreso (cronológico, todo probado en HW)
- **M1 — bring-up + chip-id ✅ (18-jun, `boot-wifi1.img`):** `mt6582-consys 18070000.consys: CONSYS
  VIVO: chip-id=0x6582 (PWR_CON=0xd)`. Primer probe -517 (EPROBE_DEFER, reguladores) → re-probe a
  3.5s OK. La secuencia MTCMOS portada del MFG funcionó a la primera. (`code/mt6582-consys.c` +
  reguladores VCN bajo el MT6323 en el DT.)
- **M2 — firmware ✅ (18-jun):** extraídos del stock `system.img` (sparse → `simg2img` → mount ro) y
  copiados a `/lib/firmware/`: `WIFI_RAM_CODE_MT6582` (160KB, el de nuestro chip) + variantes,
  `mt6572_82_patch_e1_0/1_hdr.bin` (patch WMT rev E1), `WMT_SOC.cfg` (`coex_wmt_ant_mode=1`,
  `co_clock_flag=0` = CONSYS usa su propio clock). Blobs propietarios → NO al repo público.
- **M3a — BTIF (canal de control) y su bootloop:** transporte = **BTIF** (no SDIO),
  memory-mapped @`0x1100C000`. Driver PIO `wifi-consys/m3a/mt6582-btif.c`. El primer
  `boot-m3a.img` hizo **bootloop** (sospecha: tormenta IRQ SPI50 o BTIF sin clock); diagnóstico y
  aislamiento por fases. **Causa raíz del boot inestable (descubierta después): el bring-up del
  CONSYS no estaba serializado** (dos servicios lo disparaban a la vez → doble bring-up) y era
  síncrono → **fix = mutex en `mt6582-btif.c`** (luego async).

### Breakthrough 19→22 jun (sesión nocturna) — el WiFi escanea, el GPS habla
El viernes el WiFi solo "respondía" (leía su id por HIF); el lunes **escanea redes reales** y el GPS
tiene el protocolo descifrado. Hitos, todos en HW:

1. **El MAC WiFi arranca (`WLAN_READY=1`).** El muro de toda la saga: faltaba la **calibración RF**
   (`01 14 01 00 01`) que el OEM corre tras los parches; sin ella el PHY/PLL arrancaba sin calibrar y
   el firmware se colgaba en la entrada. Va en `mt6582-btif.c` (arranca BT/GPS/MAC).
2. **WiFi cmd/event runtime:** el FW responde `NIC_CAPABILITY` + la MAC, por **TC4 / puerto-1**.
3. **WiFi SCAN — 14 beacons reales** (vecinos de Cartagena: vodafone, MOVISTAR, Telecartagena;
   canales 1-13; RSSI −63…−95 dBm). Comandos por puerto-1, beacons MGMT por puerto-0.
4. **WiFi cfg80211 / `wlan0`:** `wlan0` registra (wiphy+netdev) y **`iw dev wlan0 scan` lista 16
   redes reales desde userspace** (Open-UPCT, eduroam, MikroTik, cpcd…). Falta NetworkManager/Phosh +
   el `.connect` (Fase 2: `CMD_ID_INFRASTRUCTURE` + `SET_BSS_INFO` + STA-record; `.add_key`). Nota
   regdb: cfg80211 built-in pide `regulatory.db` a 0.96 s (antes de montar el rootfs) → mitigar con
   `REGULATORY_CUSTOM_REG` o meter el regdb en el initramfs.
5. **GPS: protocolo `0xAAF0` decodificado + bridge nativo C.** Frame `AA F0│LEN│SEQ│TYPE│FE│
   payload(XOR-0xCA)│AA 0F`; **el chip calcula la posición**. Bridge `userspace/mtkgps_aaf0.c`, sin
   mnld/bionic. Falta 1 recaptura (el comando RUN exacto, `t=0x05`) vía Lineage + GPSLogger + strace.
6. **Bluetooth:** RF arreglado (fix VCN33) + **emparejado con un S24**, toggle en Phosh.

### Estado por subsistema (2026-06-22)
| Subsistema | Estado |
|---|---|
| WiFi MAC / cmd-event / SCAN | ✅ arranca, responde, escanea |
| WiFi cfg80211 `wlan0` | ✅ `iw dev wlan0 scan` lista 16 redes |
| WiFi connect / WPA2 / data-path | ⬜ Fase 2-3 (no empezadas) |
| Bluetooth | ✅ emparejado (S24) + toggle Phosh |
| GPS | 🟡 protocolo decodificado, falta 1 recaptura |
| Boot | 🟡 estable la mayoría; **sshd cae ≈1/4 reboots** → power-cycle |

**Ficheros (repo `Sidihidi/bq-aquiaris-postmarketos`):** `mainline/wifi-consys/{m3a,wifi,gps}`;
`mainline/drivers/mt6582-btif.c` (RF-cal + mutex, commit `daa77829`), `mt6582-wifi.c` (cmd/event +
scan + cfg80211, ~1059 líneas, `1b47c5b0`), `mt6582-wifi-reg.h` (`488fd6c1`),
`userspace/mtkgps_aaf0.c` (`3b305604`). Memorias `.claude`: `reference_mt6582_wifi_hif`,
`reference_mt6582_gps`, `reference_mt6582_bt_rf_fix`, `reference_mt6582_boot_stability`.

### Recetas operativas
- Disparar el WiFi: `echo 1 > /sys/kernel/debug/mt6582_wifi/bringup`.
- Build+flash: `~/wifi-iter.sh` (build zImage + boot.img + `dd` sector **83968** + reboot).
- Flash mainline (SD): `fastboot flash boot ~/mainline/pkg/boot-btifDMA-sd.img`. **mtkclient: `wo
  <off> <len> <img>` NUNCA `wf`; fastboot `flash boot` NUNCA `boot`; NUNCA restaurar el LK Lollipop.**

---

## 20. Plugins de Phosh (quick-settings + lockscreen) ✅ (2026-06-24)

Los **toggles del panel rápido** de Phosh (datos móviles, hotspot WiFi, caffeine, modo oscuro,
night-light, ubicación, escalado, pomodoro, syncthing) + los **widgets del lockscreen** (calendario,
próximos eventos, reproductores, info de emergencia, launcher, ticket-box). **17 plugins compilados.**

**Logrado:** Alpine armhf NO empaqueta `phosh` ni sus plugins → **build NATIVA en el propio móvil**
(fuente en `/root/build/phosh`, Phosh 0.55, con meson 1.11 + ninja + gcc; sin WSL ni cross-compile).

**Detalle clave:**
- Los plugins NO son `-Dplugins` sino **3 opciones meson**: `quick-setting-plugins`,
  `lockscreen-plugins`, `status-icon-plugins` (todas `false` por defecto).
- Deps que faltaban: `apk add qrcodegen-dev` (wifi-hotspot) + `evince-dev` (ticket-box del lockscreen).
- `meson configure _build -Dquick-setting-plugins=true -Dlockscreen-plugins=true -Dstatus-icon-plugins=true`
  → `ninja -C _build install` → `/usr/local/lib/phosh/plugins/*.so`.
- **HABILITAR (trampa crítica):** `gsettings set sm.puri.phosh.plugins quick-settings "[ids]"` (+ `lock-screen`).
  El `Id` = basename del `.plugin`. **Hay que usar el bus D-Bus REAL de phosh** (de
  `/proc/$(pgrep phosh)/environ`, `DBUS_SESSION_BUS_ADDRESS`), NO `/run/user/1000/bus` o `dconf` falla.

**Ficheros:** `/usr/local/lib/phosh/plugins/*.so` (17). Memoria `.claude`: `reference_phosh_quicksetting_plugins`.

---

## 21. Batería en Phosh — indicador % + cargando/descargando ✅ (2026-06-25)

El **indicador de batería en la UI de Phosh**: porcentaje real + distinción **cargando (icono con
rayo) vs descargando**. (El hito 12 logró LEER el VBAT; este lo lleva hasta UPower → Phosh.)

**Logrado:** como **no hay driver mainline que lea VBAT en el MT6582** (el AUXADC del SoC
`mt6577-auxadc` no soporta mt6582 y el AUXADC del PMIC MT6323 no tiene driver IIO), se hace un
**puente userspace** que no recompila el kernel base ni reflashea:

```
battery (pwrap, VBAT MT6323 — hito 12) ─┐
charge-status (FAN5405 — hito 11) ──────┼─► battery-upower (daemon) ─► test_power.ko ─► UPower ─► Phosh
                                        ┘
```

**Detalle clave:**
- **`test_power.ko`**: el driver de power_supply de pega ya existe en el source
  (`drivers/power/supply/test_power.c`). `CONFIG_TEST_POWER=m` + `make O=build-krillin ARCH=arm
  CROSS_COMPILE=arm-linux-gnueabihf- drivers/power/supply/test_power.ko`. **vermagic `7.0.12 SMP
  mod_unload ARMv7 p2v8` = el del móvil** → `insmod` SIN reflashear (como uinput, hito 9). Crea
  `/sys/class/power_supply/{test_battery,test_ac,test_usb}` con params escribibles en runtime.
- El daemon `battery-upower` (cada 15 s) lee VBAT + carga y escribe
  `/sys/module/test_power/parameters/{battery_capacity,battery_voltage,battery_status,battery_current,ac_online,usb_online}`.
- **★ CLAVE (lo que costó): UPower decide cargando/descargando por el SIGNO de `current_now`, NO por el
  string `status`.** Con `status=charging` pero corriente negativa (default -1600), UPower mostraba
  `discharging`. **Fix: `battery_current` POSITIVO (+500000) al cargar, NEGATIVO (-300000) al
  descargar** → `state=charging` + icono `battery-full-charging-symbolic` (rayo).
- Resultado en `upower -i .../DisplayDevice`: `percentage=92%`, `state=charging`. Probado en HW.

**Persistencia:** `/etc/local.d/zzv-battery.start` (insmod `test_power.ko` + `start-stop-daemon`).
Sobrevive reinicios.

**Ficheros:** `/usr/local/lib/test_power.ko`, `mainline/rootfs/battery-upower`,
`mainline/rootfs/zzv-battery.start` (+ reutiliza `battery`/`charge-status` de hitos 11-12).
Memoria `.claude`: `reference_mt6582_battery_upower`.

---

## 22. Sesión elogind activa + diagnóstico del slider de brillo (2026-06-24)

**Sesión elogind ACTIVA para `sxmo` ✅** (crackeó el "BANCADO" del hito 15) — base para
batería/suspend/power UI. **Slider de brillo: diagnosticado, es problema del source de Phosh ⬜.**

**La sesión activa (logrado):** `su`=busybox no tiene PAM, pero `pam_elogind.so` sí existe →
- Mini-helper en C `/usr/local/bin/phosh-pam-session`: `pam_start("phosh-session","sxmo")` + `pam_putenv`
  de **`XDG_VTNR=1` (¡el que faltaba! sin él `pam_open_session` da `rc=14`, con él `rc=0`)** +
  `XDG_SEAT=seat0` + type/class + `pam_open_session` + **`pam_getenvlist`→`putenv` (propaga
  `XDG_SESSION_ID` a phoc)** + setuid sxmo + exec launch_phosh.
- `/etc/pam.d/phosh-session` con `session required pam_elogind.so`.
- `launch_phosh.sh`: **`LIBSEAT_BACKEND=logind`** (phoc usa la sesión elogind como asiento).
- Resultado: `loginctl` da sesión `sxmo seat0 **Active=yes**`; phoc arranca heredando `XDG_SESSION_ID`.

**El slider (diagnóstico):** aun con la sesión activa, sigue inerte. Con `G_MESSAGES_DEBUG=all`,
gsd-power dice **`No org.gnome.Shell.Brightness support`** → en este build **gsd-power DELEGA el brillo
de pantalla al compositor (`org.gnome.Shell.Brightness`), que Phosh no provee** → no expone `.Screen`.
**No es la sesión ni el HW; es el source de Phosh.** Brillo usable hoy: **comando `bl 0-100`** (hito 15).

**Ficheros:** `/usr/local/bin/phosh-pam-session` (+ `/root/phosh-pam-session.c`),
`/etc/pam.d/phosh-session`. Memoria `.claude`: `reference_mt6582_backlight` (sección 06-24).

---

## 23. WiFi WPA2 — scan-crash resuelto + CCMP diagnosticado 🟡 (2026-06-24)

Complementa la saga WiFi (hito 19). **Scan-crash RESUELTO**; **WPA2 asocia + handshake**, pero el
**CCMP de datos resulta ser hueso del FW**.

**Scan-crash (resuelto):** estando conectado, un `iw scan` colgaba el móvil (hard-lockup + WDT). Causa:
`wifi_port_read_pio` leía el FIFO sin guard. **Fix: guard WCIR por-palabra** en `wifi_port_read_pio` y
`wifi_port1_read_pio` (re-sondea `MCR_WCIR`, registro estático, antes de cada lectura del FIFO; si
`!= WIFI_CHIP_ID_6582` aborta limpio). Probado: el móvil **sobrevive `iw scan` conectado**.

**WPA2/CCMP (diagnosticado):** el **host hace TODO bien** (verificado byte-a-byte vs downstream):
claves CCMP (`cipher=0xfac04`, PTK `tx_key=1`, GTK), struct `cmd_802_11_key`, `CIPHER_CCMP=4`, StaRec
STATE_3 + `EVENT_ACTIVATE_STA_REC`, `wpa_cli COMPLETED`. **Pero el FW no cifra/descifra DATOS**:
`tx=55 rx=2`, 0 respuestas (la red **ABIERTA sí navega**). `enc_status=ENABLED` probado → no arregla +
causa `eid=0x1b` → revertido. ⚠️ Falso positivo típico: un `ping 8.8.8.8` que "funciona" sale por
`usb0` (NAT de la Pi), no por wlan0.

**Ficheros:** `mainline/wifi-consys/wifi/mt6582-wifi.c` (guard scan). Memorias `.claude`:
`reference_mt6582_wifi_wpa2`, `reference_mt6582_wifi_channel_bug`.

---

## Roadmap pendiente (resumen)

- **F0 — Boot:** estabilizar el `sshd`-down recurrente (≈1/4 reboots) — endurecer `zz-sshd.start`
  o el cuelgue temprano del runlevel boot; verificar 5 reboots limpios.
- **F1 — WiFi:** cerrar cfg80211 hacia NetworkManager; **Fase 2** (`.connect` red abierta → WPA2:
  EAPOL + `.add_key`); **Fase 3** data-path (`ndo_start_xmit` + RX + PDMA + IRQ HIFSYS → DHCP+ping).
- **F2 — GPS:** capturar el START_SEQ/comando RUN de mnld → fix real → gpsd → geoclue → Phosh.
- **F3 — Phosh:** brillo por slider (driver backlight custom, hito 15); botones power/volumen
  (Vol↑ pendiente, hito 17); sensores + autorrotación; audio (codec MT6582); suspend/wake + toggles.
- **F4 — Periféricos:** FM, vibrador (cámara/módem = difícil).
- **F5 — futuro:** estudio de viabilidad de Android 12.
