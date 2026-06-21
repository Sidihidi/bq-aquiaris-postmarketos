---
name: reference_mt6582_bt_rf_fix
description: ★★ BT krillin ARREGLADO EN HW (2026-06-21): regresión VCN33 (el cambio consys del 06-20 dejó de encender el raíl RF al arranque → RX/TX=0) RESUELTA encendiendo VCN33_BT (ANALDO_CON16 0x0416 bit7) + VCN33_WIFI antes del bringup en zz-consys-bt.start vía pwrap_poke (sin reflashear); + agente auto-yes en zzz-bt-agent.start. S24 VINCULADO.
metadata:
  node_type: memory
  type: reference
  originSessionId: 25e1ad97-4628-4d7e-af11-7c14402deec4
---

**SÍNTOMA:** BT mainline radiaba el viernes 19 (el S24 lo veía como "BlueZ 5.86") pero el 21 `hciconfig hci0` = `RX bytes:0 / TX bytes:0`, `Discovering:no`, nadie lo veía. El controlador HCI estaba VIVO (responde a reset/read-features/event-mask con status 0x00) y `func_on[BT]: *** RADIO ENCENDIDO ***` → el radio se "encendía" por WMT pero no radiaba.

**CAUSA (regresión):** el cambio de `mt6582-consys.c` (06-20 21:48, hecho para WiFi; backup `.bak-pre-vcn33`) QUITÓ `regulator_enable(cs->vcn33)` del arranque del CONSYS, con el comentario "BT/GPS no usan VCN33". **FALSO: VCN33 es el LDO de 3.3V del PA de RF.** Sin él al arranque, el firmware BT se inicializaba SIN potencia RF → no transmitía. Hay **DOS raíles VCN33** en el MT6323: **VCN33_BT** (enable `ANALDO_CON16`=**0x0416 bit7**) y **VCN33_WIFI** (enable `ANALDO_CON17`=**0x0418 bit12** + bit14 HW-mode). Ambos a 3.3V (vsel bits2-3=0). Estado por `cat /sys/kernel/debug/regulator/regulator_summary`; PMIC por `/usr/local/bin/pwrap_poke r|w <reg> <val>`.

**POR QUÉ NO BASTÓ ENCENDERLOS EN CALIENTE:** el firmware BT inicializa el RF UNA SOLA VEZ, en el bringup (`echo 1 > /sys/kernel/debug/mt6582_btif/bringup`, lo dispara `zz-consys-bt.start` al boot). `bringup()` está protegido por `brought_up` (línea 421 "ya levantado") → no se re-inicializa en caliente, y los drivers son **builtin** (no módulos, `lsmod` vacío, no se recargan). → hay que encender los raíles ANTES del bringup, lo que obliga a un reboot (NO un reflash).

**FIX (userspace, sin reflashear, PERMANENTE):** en `/etc/local.d/zz-consys-bt.start`, ANTES del `echo 1 > .../bringup`, encender los raíles con read-modify-write: `set_bit(){ cur=$(pwrap_poke r $1|sed 's/.*= //'); pwrap_poke w $1 $(printf 0x%04x $((cur|$2))); }` · `set_bit 0x0416 0x0080` (VCN33_BT) · `set_bit 0x0418 0x5000` (VCN33_WIFI). Tras reboot: `CON16=0x0090`, `func_on[BT] RADIO ENCENDIDO` con RF alimentada → **el S24 ve "krillin-bq"** (validado en HW). Backup `zz-consys-bt.start.bak-pre-vcn33`.

**EMPAREJAMIENTO (capa aparte):** el S24 CONECTA (BR/EDR connected) pero `bluetoothctl` PREGUNTA "Confirm passkey XXXXXX (yes/no)" y espera stdin → si nadie responde, `auth failed 0x05 (Authentication Failed)`. NO existe `bt-agent` (solo `bluetoothctl`+`btmgmt`). FIX: agente persistente `/etc/local.d/zzz-bt-agent.start` = `bluetoothctl` (agent NoInputNoOutput + default-agent + pairable + discoverable) alimentado con `echo yes` cada 2s → auto-confirma el passkey. **S24 "S24 Ultra de Juan" (E4:92:82:F4:8E:43) VINCULADO**; claves en `/var/lib/bluetooth/<adapter>/E4:92...`, `Bonded:yes Paired:yes` (reconecta solo). MAC del krillin = placeholder `00:00:46:65:82:01` (del chip-id; identidad real perdida, ver [[reference_mt6582_nvram_rf_cal]]).

**PENDIENTE (limpieza, no bloquea):** (1) fix LIMPIO en el driver `mt6582-consys.c` (encender VCN33_BT+WIFI en el probe, usa el `pmic_regmap` que ya tiene) en vez del boot script. (2) **audio A2DP NO va** (sin ALSA/sink, subsistema aparte sin empezar). (3) descubrimiento ACTIVO (krillin escanea otros) por reverificar con RF ya buena. (4) agente auto-yes es hacky (spam de "yes"); mejor un Agent1 D-Bus python. La cal RF está en eFUSE [[reference_mt6582_nvram_rf_cal]]. CONSYS/capas [[reference_mt6582_wifi_consys]]; WiFi sigue con `WLAN_READY=0` [[reference_mt6582_wifi_hif]].
