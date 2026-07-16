# Capturas ground-truth del CONSYS en LineageOS (0716)

El command-stream canónico del stock (LineageOS, GPS viendo **13 satélites** en el mismo HW), capturado por
`adb root` + `/proc/driver/wmt_dbg` con el logging del driver WMT a máximo. Es la "verdad de tierra" para
replicar el bring-up del CONSYS en pmOS. Ver el análisis en `../../PLAN-WMT-LIB-PORT-0716.md`.

## Cómo se capturó
```sh
adb root
adb shell 'echo "9 4" > /proc/driver/wmt_dbg'   # WMT log level = LOUD (4)
adb shell 'echo "a 4" > /proc/driver/wmt_dbg'    # STP log level = 4
# drenaje continuo del ring del kernel:
adb shell '(i=0; while [ $i -lt 300 ]; do dmesg -c; i=$((i+1)); sleep 0.3; done) > /data/local/tmp/wmt.log &'
adb shell 'echo "6 0" > /proc/driver/wmt_dbg'    # whole-chip-reset -> re-corre el sw_init COMPLETO
# (para snoopear el GPS-on: echo "7 2 1" = func_on(GPS=2))
```
Tokens de `wmt_dbg` = `<func> <par2> <par3>` en HEX: `6 0`=chip reset, `7 <t> <0|1>`=func off/on
(t: 0=BT 1=FM 2=GPS 3=WIFI), `9 <n>`=nivel log WMT, `a <n>`=nivel log STP, `3 0`=hw_ver, `8 0`=chip_id.

## Ficheros

| Fichero | Qué es |
|---|---|
| `wmt-reset-swinit.log.gz` | **Captura CRUDA COMPLETA** del `6 0` (chip-reset → sw_init entero). 94 MB → gunzip. Es la verdad literal (incluye el spam de bluedroid entrelazado). |
| `wmt-reset-sin-spam.log` | La misma, filtrando el spam obvio (bluedroid/FH/dvfs). Legible. |
| `wmt-reset-stream-filtrado.log` | Solo las líneas WMT-IC/WMT-CORE/WMT-FUNC/dumps + `address command`. El stream en limpio. |
| `wmt-gps-on-snoop.log.gz` | Snoop del `7 2 1` (func_on GPS) por separado: GPS_SYNC + LNA + func_on + de-sense. |

## ⚠️ Cómo leerlas (aprendido a base de golpes)
- El log del kernel MTK **entrelaza líneas de 2 CPUs** en la misma línea física → usar `grep -a` (hay bytes
  binarios que hacen que grep lo trate como binario) y **filtros POSITIVOS**; el spam de `[MTK-BT] BT_read`
  de bluedroid contamina las mismas líneas de los dumps.
- Los payloads WMT salen como `start of dump>[rx (uart):] len=N ... 0xNN 0xNN <end of dump`.

## La secuencia REAL observada (resumen; detalle en el PLAN)
`rails → HW RST → BTIF open → STP reset → PATCH_ADDRESS_CMD → P_ADDRESS(addr 00 00 0e f0) →
UN SOLO PATCH 81 frags (ROMv1_patch_1_0, 80KB) → WMT_RESET → PALDO ON (BT+WIFI) → RF_CAL(372ms) →
PALDO OFF → coex(ant_mode=1) → FM-COMM (01 05 02 00 02 02)`.
**El stock NO manda** init_table_1_2/4/5 (SET_STP), ni hw_ver por WMT, ni GEN_HCR, ni osc_type, ni merge_pcm.
De-sense: muere en `send_command_to_daemon: invalid native process pid` (sin daemon) y aun así 13 sats
→ **de-sense refutado**.

*Capturado en el móvil real (Pi 192.168.0.123 → `ssh root@172.16.42.1`), 2026-07-16.*
