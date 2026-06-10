# 🔧 M2b (WIP) — USB gadget MUSB en mainline para el MT6582

Estado: **driver escrito y COMPILA** en linux-7.0.12. Sin probar en hardware aún.
Objetivo: que el teléfono aparezca como tarjeta de red USB (g_ether) sobre
mainline → **SSH en mainline** → desarrollo rápido de M3 (display).

## Qué se ha hecho

- **Driver nuevo**: `drivers/usb/musb/mt6582-musb.c` (copia aquí en
  `mainline/drivers-musb/`). Derivado de `mediatek.c` de mainline porque el
  MT6582 lleva un **Mentor MUSB** con el mismo wrapper MediaTek (interrupción L1
  agregada en `0xa0/0xa4`, registros toggle RX/TX en `0x80-0x86`). Cambios:
  - **Clocks por pokes directos** (no hay driver CCF para mt6582):
    `UNIVPLL_CON0` (APMIXEDSYS `0x10209220`) bit26 = clock PHY 48M;
    `PERI_PDN0_CLR` (PERICFG `0x10003010`) bit10 = clock MAC USB.
  - **Reset del IP USB**: PERICFG `0x10003000` bit29 (set → 10 ms → clear).
  - **PHY inline**: secuencia de encendido portada de `usb20_phy.c`
    (`usb_phy_poweron`, modo device) + calibración de slew-rate. Registros PHY
    de 8 bits en `SIF + 0x800 + offset`.
  - **Peripheral-only + PIO** (sin DMA Inventra) para minimizar fallos al inicio.

- **Nodo DT** (`mainline/dts/usb-node.dtsi`, añadido al krillin.dts):
  `compatible="mediatek,mt6582-musb"`, `reg = <0x11200000 0x1000>,<0x11210000 0x1000>`,
  `interrupts=<GIC_SPI 32 IRQ_TYPE_LEVEL_LOW>`, `dr_mode="peripheral"`.

- **Config** (build-krillin): `USB_MUSB_MT6582=y`, `USB_MUSB_HDRC=y`,
  `USB_MUSB_DUAL_ROLE=y`, `MUSB_PIO_ONLY=y`, `NOP_USB_XCEIV=y`,
  `USB_GADGET=y`, `USB_ETH=y` (+RNDIS), `USB_LIBCOMPOSITE=y`. **zImage compila.**

## Datos de hardware (verificados en el downstream usb20/mt6582)

| Recurso | Valor |
|---|---|
| MUSB MAC | `0x11200000` |
| USB SIF / PHY | `0x11210000`, regs 8-bit en `+0x800+off` |
| IRQ | `GIC_SPI 32` (downstream hwirq 64 = priv+32), LEVEL_LOW |
| Clock PHY 48M | UNIVPLL_CON0 `0x10209220` bit26 |
| Clock MAC | PERI_PDN0_CLR `0x10003010` bit10 (clear PDN = on) |
| Reset USB IP | PERICFG `0x10003000` bit29 |

## Próximo paso: probar enumeración (semi-automatizable desde la Pi)

1. Ensamblar `boot-mainline-v9.img` con el zImage nuevo (`pkg/assemble.sh`).
2. Desde pmOS (sin botones): `reboot2 bootloader` → fastboot.
3. Pi: `fastboot flash boot boot-mainline-v9.img && fastboot reboot`.
4. **Señal de éxito**: en la Pi aparece un dispositivo USB NUEVO (gadget g_ether
   RNDIS/ECM) en `dmesg`/`lsusb`, distinto del de fastboot. La enumeración es a
   nivel de kernel, así que no hace falta userspace completo para el primer test.
5. Si NO enumera → leer ramoops `0xBF300000` desde pmOS (ver mainline/README §5)
   y depurar PHY/clock/IRQ. Recuperar pmOS: combo Power+Vol+ → fastboot →
   `fastboot flash boot ~/pmos-artifacts/boot-pstore.img`.

## Riesgos conocidos / a vigilar

- **Clock MAC**: si el gate de PERI_PDN0 bit10 no es el correcto, el MUSB no
  responde (lecturas 0x00/0xff). El LK quizá ya lo deja encendido (usó USB para
  fastboot) → puede que ni haga falta.
- **Polaridad IRQ**: copiada del mmc (LEVEL_LOW) que sí funciona.
- **PHY**: la secuencia es de device-mode; si enumera pero falla la negociación
  HS, revisar slew-cal y los pasos de suspendm.
- **g_ether** se autoenlaza al UDC al arrancar (built-in) → enumeración sin init.
