# HITO M2b — Red por USB en mainline (gadget Ethernet)

**Estado: RESUELTO** (2026-06-16). El kernel mainline 7.0.12 enumera como gadget
USB Ethernet y hay **ping bidireccional** Pi <-> teléfono por USB.

```
[host] usb 1-1: Product: RNDIS/Ethernet Gadget
[host] usb 1-1: Manufacturer: Linux 7.0.12 with musb-hdrc
[host] cdc_ether 1-1:1.0 usb0: register 'cdc_ether' ... CDC Ethernet Device
[host] lsusb: ID 0525:a4a2 Linux-USB Ethernet/RNDIS Gadget
ping 172.16.42.1: 4 transmitted, 4 received, 0% packet loss, rtt ~0.2 ms
```

## Componentes
- **Driver glue**: `drivers/usb/musb/mt6582-musb.c` (derivado de mediatek.c). Modo
  peripheral, PIO (sin Inventra DMA), PHY MTK USB2 inline.
- **DT**: `usb@11200000`, compatible `mediatek,mt6582-musb`, reg MAC 0x11200000 +
  SIF 0x11210000, IRQ GIC_SPI 32 LEVEL_LOW, `dr_mode="peripheral"`.
- **.config**: `USB_MUSB_HDRC=y USB_MUSB_MT6582=y USB_MUSB_DUAL_ROLE=y
  NOP_USB_XCEIV=y USB_GADGET=y USB_ETH=y USB_ETH_RNDIS=y` (g_ether built-in se
  bindea solo al UDC al boot).

## Los 3 bugs resueltos (en orden de descubrimiento)

### 1) "irq 26: nobody cared" -> Disabling IRQ #26
El gadget se bindeaba (`g_ether ready`) y D+ se forzaba (`SOFTCONN`), pero a los
~5,3 s el kernel **deshabilitaba la IRQ del USB** y la enumeración moría.
- Causa: el `init` desenmascaraba **`USB_L1INTM = 0x1ff`** (bits 0..8), habilitando
  fuentes del agregador L1 de MediaTek que el ISR no atiende (bit3 DMA + grupo
  0x1f0). Una se dispara, `INTRUSB/INTRTX/INTRRX` están a 0, el ISR devuelve
  `IRQ_NONE`, y como la línea es *level* nunca baja -> "nobody cared".
- Fix (v15): **`L1INTM = TX|RX|USBCOM`** (0x7) y el handler solo va al core ISR si
  `L1INTS & L1INTM` tiene TX/RX/USBCOM. Igual que el path peripheral del downstream
  (PIO -> sin DMA; peripheral -> sin IDDIG).
- Registros L1 (mach-mt6582/mt_musb_reg.h): `L1INTS=0xa0 L1INTM=0xa4 L1INTP=0xa8`;
  `TX=bit0 RX=bit1 USBCOM=bit2 DMA=bit3 IDDIG=bit9`.

### 2) D+ no subía (el host nunca veía la conexión)
Con la IRQ ya viva, tras `SOFTCONN` no llegaba ningún reset/SETUP y la Pi no
detectaba el dispositivo.
- Causa: el PHY init era el `usb_phy_poweron()` **corto** y dejaba los
  **pull-downs / force-overrides** del PHY ASSERTED, así el pull-up de SOFTCONN
  competía con un pull-down activo -> D+ no subía.
- Fix (v16): portar la secuencia **`usb_phy_recover()` completa** del downstream:
  limpiar `0x68` (DPPULLDOWN 0x40/DMPULLDOWN 0x80/XCVRSEL 0x30/TERMSEL 0x04),
  `0x69` (DATAIN 0x3c), todos los force de `0x6a` (0x10/0x20/0x08/0x02/0x80),
  `0x1d` (PUPD_BIST 0x10); SET `0x1a`=0x10 (VBUSSCMP_EN); device-mode + slew-cal.
  **No tocar clocks** (UNIVPLL/PERI_USB): el LK los deja on; pokearlos rompía el
  MSDC/eMMC (lección v13).

### 3) Enumera pero carrier=0 (usb0 DOWN)
El gadget aparecía en el host pero sin enlace.
- Causa: el lado gadget (teléfono) no levantaba su `usb0`.
- Fix: el init configura `usb0` por `ioctl` (SIOCSIFADDR 172.16.42.1, NETMASK
  255.255.255.0, FLAGS IFF_UP). El initrd no trae `/sbin/ip`.

## Direcciones
Teléfono `usb0 = 172.16.42.1/24`, host (Pi) `usb0 = 172.16.42.2/24`.

## Pendiente
- **SSH por USB**: el initrd de pruebas no trae `sshd`. Opciones: (a) dropbear
  estático + claves en el initramfs, o (b) arrancar el rootfs real (ext4 p5) que ya
  trae dropbear. La RED ya funciona.
- Hacerlo permanente: mover la config de `usb0` al init real del rootfs.
