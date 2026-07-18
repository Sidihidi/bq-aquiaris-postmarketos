# Módem H4 — ✅ DATA ABORT RESUELTO: era SOLAPE de regiones del SMEM (0718)

> **BREAKTHROUGH.** El data abort del HS2 (bloqueante desde que el MD aceptaba el runtime) era
> **regiones del SMEM que se SOLAPABAN** por tamaños incorrectos. Con las regiones a su tamaño REAL
> y espaciadas sin solape, **el MD ya NO aborta**: arranca hasta la fase de FS-init y se queda
> esperando que el AP le sirva el protocolo CCCI. Es el mayor avance del módem hasta ahora.

## La causa: FS de 80KB pisando las regiones de después
`CCCI_FS_SMEM_SIZE = sizeof(fs_stream_buffer_t) * 5 = 16388 * 5 = 0x14014 (80KB)`. Le dábamos a FS un
size placeholder de 0x2000 (8KB) en el runtime, PERO **el MD usa el tamaño REAL (80KB)**, no el que
le decimos. FS estaba en SMEM+0xE000, así que en realidad ocupa 0xE000–0x22014. Las regiones que el
barrido añadió (Mdlog@0x10000, Uart@0x18000, Net@0x1C000...) caían DENTRO de ese rango → el MD, al
escribir en FS, corrompía Mdlog/Uart/Net; al leerlas luego = garbage → **DATA ABORT**.
(Antes de añadirlas, tras FS había memoria vacía → el overrun no rompía nada visible = por eso el
runtime mínimo "aceptaba" pero abortaba en otra cosa, y añadir regiones sin espaciar lo empeoró.)

## El fix (kernel #24): layout NO-solapado con tamaños reales
Regiones secuenciales, cada una con hueco ≥ su tamaño real, todo en los 2MB del SMEM:
```
IPC   @0x2000(0x2000)  PCM @0x4000(0x8000)  RPC @0xC000(0x2000)
FS    @0xE000  size=0x14014 REAL           (ends 0x22014)
Mdlog @0x30000  Uart @0x40/50/60000 (0xA000 c/u)
MDULNet @0x70000(0x4B000=300K)  MDDLNet @0xC0000(0x50000)
NetULCtrl @0x120/130/140000  NetDLCtrl @0x150/160/170000
```
(Mdlog chico=0x8000 porque mdlog está off; los grandes con tamaño real 300K/320K. Fin 0x178000 < 2MB.)

## Resultado (CCIF leído por devmem, estable en 3s)
```
Con SOLAPE (test previo #23):  RCHNUM=0x0C  ch2=ffffffff 00000004 (DATA ABORT)
Sin solape / real (#24):       RCHNUM=0x02
   ch0 = ffffffff 00000000 00000000 5555ffff   (HS1, stale)
   ch1 = 4160e000 00000020 0000000e 00000000   (FRESCO: msg canal 14=FS_RX, data0=base FS)
   ch2,ch3 = basura stale (NO en RCHNUM -> sin excepcion)
```
**La excepción desapareció.** El MD manda un mensaje del canal FS (ready/init de FS con la base que
le dimos) y se queda idle — SIN abortar. Reproducible.

## Dónde estamos ahora
El MD **arranca sin abortar** y llega a mandar tráfico operativo (FS). Pero **no llega a
NORMAL_BOOT_ID** todavía: se queda esperando porque nuestro `spm_md_hs2` es one-shot (manda el
runtime + MD_INIT_START_BOOT y hace poll), NO sirve el protocolo CCCI (no ACKea los mensajes del MD
ni responde al canal FS). El MD necesita que el AP:
1. ACKee sus mensajes (CCIF_ACK por canal recibido).
2. Sirva/responda el canal FS (y los que vengan).
3. Siga el bucle hasta que el MD emita NORMAL_BOOT_ID (HS2 = M1 completo).

Eso ya no es "un campo del runtime" — es **el bucle de servicio del CCCI** (un trozo del driver ccci
real). Es el siguiente hito, acotado y claro.

## Siguiente paso
Convertir el `spm_md_hs2` de one-shot a un mini-bucle que: (a) ACKee cada mensaje entrante (RCHNUM →
CCIF_ACK), (b) loguee los mensajes del MD para ver qué pide, (c) responda lo mínimo (FS init) hasta
NORMAL_BOOT_ID. O integrar el kthread RX del CCIF. Con el abort resuelto, esto es camino directo.

## Estado del HW / coordinación
Móvil restaurado a la imagen de diario (#14). El fix está en la COPIA LOCAL `spm-sweep` (scratchpad);
el `mt6582-spm-H1.c` compartido NO se ha tocado — **el Mac debería integrar este layout no-solapado
con tamaños reales** (los offsets/sizes están arriba). Test #24 `boot-modem-hs2.img`.

*2026-07-18, sesión Windows (Fable 5). DATA ABORT RESUELTO (solape de regiones). Falta el bucle de
servicio CCCI para NORMAL_BOOT_ID.*
