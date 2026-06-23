# CONSYS / BTIF / STP-WMT + Bluetooth (M3a)

Núcleo del subsistema de conectividad (CONSYS) del MT6582 y el transporte serie
**BTIF** (AP↔CONSYS) sobre el que viajan STP/WMT. Aquí vive el bring-up del chip
(descarga del patch + `func_on` de las radios) y la pila **Bluetooth HCI** que
registra `hci0` para BlueZ/Phosh. **Estado: ✅ funcionando** — el CONSYS contesta,
el patch descarga, las 4 radios encienden y el BT empareja (S24) con toggle en Phosh.
Es la base de la que dependen WiFi (`../wifi/`) y GPS (`../gps/`): ambos esperan a
que este driver tenga el CONSYS vivo antes de arrancar.

Para el detalle técnico no leas este índice: ve a los `.md` de abajo.

## Ficheros

- **`mt6582-btif.c`** — driver del kernel: BTIF-DMA + bring-up STP/WMT del CONSYS +
  Bluetooth HCI (`hci0`). El bring-up se dispara por debugfs
  (`echo 1 > /sys/kernel/debug/mt6582_btif/bringup`). Incluye el **mutex** que
  serializa el bring-up (fin de la carrera de arranque, ver memoria del proyecto).
- **`consys-dt-btif-snippet.dts`** — nodo DeviceTree del BTIF (`btif@1100c000`,
  IRQ GIC_SPI 50) para pegar en `mt6582-bq-krillin.dts`.

## Documentación detallada

- **`HITO-WIFI-M3A.md`** — bitácora completa del hito M3a: iteraciones it.2→it.6,
  decisiones, registros y cómo se llegó al CONSYS vivo + HCI + `hci0`.
- **`SECUENCIA-ARRANQUE-CONSYS.md`** — secuencia exacta de power/STP/WMT del
  bring-up portada del downstream 3.10.
- **`PLAN-B-CAPTURA-ANDROID.md`** — plan alternativo: capturar el bring-up real
  desde Android para comparar/extraer la coreografía que falte.

## Auxiliares

- **`scripts/`** — `wifi-iter.sh` (build+flash iterativo), `wifi-bringup.sh` y
  `wifi-check.sh` (disparar el bring-up y comprobar estado por SSH al teléfono).
- **`captura/stock-consys-bringup.txt`** — log del bring-up del CONSYS capturado
  del kernel downstream (referencia de la secuencia WMT/STP correcta).
