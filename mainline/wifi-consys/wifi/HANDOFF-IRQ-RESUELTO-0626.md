# HANDOFF — IRQ del HIF RESUELTO + DHCP acotado (2026-06-26)

> **Supersede `HANDOFF-IRQ-0626.md`**, que tenía el número de SPI mal (160) y culpaba erróneamente
> al "routing CONSYS". El IRQ se arregló con UNA línea del DT. Aquí lo correcto.

## TL;DR — lo gordo de hoy
- 🎯 **El IRQ del HIF DISPARA.** El bug era UNA cosa: **el número de SPI del DT estaba mal.**
  - `WF_HIF_IRQ_ID = 216` (downstream `x_define_irq.h`) → `GIC_SPI = 216 − 32 = `**`184`** (NO 160).
  - Confirmado por el patrón del propio DT: PMIC_WRAP es IRQ 147 → `GIC_SPI 115` (147−32). Mismo patrón.
  - **Fix (1 línea)** en `arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts`, nodo `wifi@180f0000`:
    ```
    interrupts = <GIC_SPI 184 IRQ_TYPE_LEVEL_LOW>;
    ```
  - Resultado: contador `209` pasa de **8 → 364** durante la conexión; dmesg "IRQ 209 (AHB_SLAVE_HIF)
    registrado → RX por interrupción". **NO era el routing CONSYS.**
- ⚠️ **GOTCHA grande**: `make zImage` **NO reconstruye el DTB**. Tras editar el DT hay que hacer
  `make dtbs` aparte; si no, se empaqueta el DTB viejo (sin `interrupts`) y el driver cae a polling
  (dmesg: "platform_get_irq=-6: IRQ index 0 not found"). Perdimos un ciclo por esto.
- ✅ **Los CRASHES están RESUELTOS** (#171 WHLPCR/driver-own). Decenas de connects/disconnects/intentos,
  uptime >1200s, **cero reboots espontáneos**. El ciclo de reboot-cada-3min (hard-lockup del PIO) está muerto.

## El DHCP — estado ACOTADO (no resuelto)
- **TX siempre sale**: AUTH, ASSOC-REQ, DISCOVER se transmiten.
- **RX entrega CASI TODO**: beacons (l0=220/309/359), AUTH-2 (l0=42 → mgmt-RX subtype=11), eventos
  (heartbeat `eid=0x0e` ~30ms, `eid=0x19`), y el FW encola hasta **n0=3** frames (visto con el diag l0/l0b).
- PERO **frames concretos caen intermitentemente**:
  - **El ASSOC-RESP no llega tras el ASSOC-REQ** → el connect se atasca (a menudo).
  - **La OFFER no llega post-GTK** → sin DHCP (visto en el único connect limpio, "diag1").
- Síntoma clave: **`.connect: sin grant CH_PRIVILEGE en 1s`** — el grant del FW tarda >1s.
- **Descartado**: RF-cal (VCN33 `regulator-always-on` está en el DTS, línea 400). NO es "RX roto" (entra
  casi todo). NO es TX. NO es el firmware (LineageOS navega con el mismo blob).

## EL SIGUIENTE ATAQUE (es de coreografía/timing, no de RX bruto)
1. Leerse la máquina de estados `.connect` en `mt6582-wifi.c`:
   `CH_PRIVILEGE request → grant → AUTH → AUTH-2 → ASSOC-REQ → ASSOC-RESP`.
2. El timeout del grant `CH_PRIVILEGE` (1s) parece corto — el FW tarda más. ¿Ampliarlo / reintentar?
3. ¿Por qué el ASSOC-RESP no llega aunque el AUTH-2 sí? ¿Se mantiene el CH_PRIVILEGE durante el ASSOC,
   o expira la ventana de privilegio de canal y el radio se va del canal → pierde el ASSOC-RESP?
4. Comparar con la SAA/CNM del downstream (`saa_fsm.c`, `cnm_*`).
5. Cuando el connect sea fiable, re-confirmar la OFFER post-GTK con el driver-diag.

## Cómo iterar (recordatorio operativo)
- **Build**: editar driver → `scp` a `~/mainline/linux-7.0.12/drivers/soc/mediatek/` en la Pi (.123) →
  `make O=build-krillin ... zImage` (**+ `make dtbs` si tocas el DT**) → empaquetar `boot-184.img`
  (cat zImage+dtb → `mtk_hdr.py` → `abootimg`) → `dd` a `/dev/mmcblk0` seek **83968** desde pmOS →
  **`reboot -f`** (¡con `-f`! Phosh bloquea el `reboot` normal y NO reinicia).
- **Diag**: el driver lleva `DIAG-DRAIN` (whisr/n0/n1/l0/l0b/mbox0) y `DIAG-RX0` (pt/l0/bytes). Quitar
  cuando se cierre.
- **Test WPA2**: red `hola` / `kakatua1`, canal 1. El connect es flaky (<3/4) — reintentar en bucle 5×.
