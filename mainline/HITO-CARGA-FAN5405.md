# HITO — Carga por USB en mainline (FAN5405) ✅ 2026-06-17

**El BQ Aquaris E4.5 ahora CARGA la batería mientras corre Linux mainline.** Antes,
sin driver de carga, el USB daba datos pero no cargaba → el sistema drenaba y el
teléfono moría a mitad de sesión (pasó 2 veces). Resuelto con un "driver" userspace.

## El hardware
- Cargador = **Fairchild FAN5405** (CONFIG_MTK_FAN5405_SUPPORT=y), I2C0 **@0x6a**
  (`0xD4>>1`; vendor=4 en CON3=0x94 confirma Fairchild). Bus 0 (FAN5405_BUSNUM=0).
- Registros (de downstream fan5405.h): CON0 TMR_RST=bit7 (watchdog); CON1 IINLIM=bits7-6,
  TE=bit3, CE=bit2 (activo-bajo); CON2 OREG=bits7-2 (CV); CON4 I_CHR=bits6-4 (corriente);
  CON6 ISAFE/VSAFE = topes HW de seguridad.

## El problema y el fix
- Sin nadie tocando el chip, su **watchdog de ~22-24s** expira y revierte a IINLIM=100mA
  (CON1=0x30) y OREG~3.54V (CON2=0x0a) → entrada 100mA < consumo → drena.
- **Fix**: daemon userspace `/usr/local/bin/fan5405-charge` que cada **10s** re-escribe
  valores seguros y kickea el watchdog (RMW CON0 bit7):
  - **CON2=0x8c** → OREG=35 = **4.20V** (CV estándar Li-ion; topado por VSAFE).
  - **CON1=0xb8** → IINLIM=**800mA**, TE=1 (termina al llenar), CE=0 (enable).
  - IOCHARGE queda en default **550mA** (I_CHR=0) — buena corriente, no se toca.
- Verificado: con el daemon, STAT=CARGANDO + OREG=35 + IINLIM=800mA se mantienen continuos.

## Persistencia y seguridad
- Arranca al boot: `/etc/local.d/zzy-charge.start` (start-stop-daemon, robusto — el
  `( cmd & )` vía SSH NO sobrevive; start-stop-daemon sí).
- Estado: `/usr/local/bin/charge-status`. Scripts en repo `mainline/rootfs/charge/`.
- **Seguridad**: CV fijo a 4.20V (nunca más), TE activa, corriente ≤800mA (0.4C para
  ~2150mAh), y CON6 VSAFE/ISAFE topan por HW. Aun así, supervisar (medidor USB, calor)
  en pruebas largas. Solo carga con VBUS presente; OTG/boost off.
- **Pendiente (battery %)**: leer VBAT requiere el AUXADC (sub-proyecto siguiente) para
  mostrar el porcentaje en la UI; la CARGA en sí ya funciona.
