# HANDOFF — Análisis WiFi WPA2: RE firmware + LineageOS + comparativa exhaustiva (2026-06-30)

Sesión muy larga (Claude glm-5.2). **Avances sólidos pero el bug del data-path WPA2 NO está cerrado.**
El control-plane (assoc + handshake + claves) funciona; el data-path cifrado no (rx=2, 0 OFFER).
Esto es lo aprendido, lo probado, lo refutado, y las vías que quedan.

---

## TL;DR — estado

- **WPA2 control-plane**: ✅ (`wpa_state=COMPLETED`, PTK+GTK CCMP, handshake OK)
- **WPA2 data-path**: ❌ (`rx=2` = solo EAPOL, 0 data frames; DHCP sin OFFER)
- **Estabilidad (reinicios/crashes)**: ✅ (guards `wifi_hif_alive`, rx_thread idle-pause)
- **Firmware RE**: ✅ descompilado y mapeado (ver `fw-analysis/`)
- **Bug acotado**: 100% del driver host (NO del FW ni del HW — LineageOS funciona con blobs idénticos)

---

## Lo confirmado esta sesión

### 1. El firmware es caja blanca ahora
- `fw-analysis/fw_f0020000.bin` (192KB): dump del código NDS32 descifrado (0xf0020000-0xf004fffc).
- `fw-analysis/rodata.bin` (8KB): rodata con strings de módulos (0xf0063000).
- `fw-analysis/fw_all_labeled.c` (860KB): 633 funciones descompiladas a C (Ghidra 12.1.2, NDS32:LE:32).
- `fw-analysis/fw_key_functions.txt`: índice función → módulo `.c` (192 funcs mapeadas, 39 módulos).
- **Cómo reproducir**: dump por `fwdump` debugfs (en bloques de 256 palabras con pausas; en bloque grande cuelga). Ghidra headless con JDK arm64 (openjdk@21) — el truco: el decompile nativo solo está para mac_arm_64.

### 2. LineageOS funciona → el bug es del driver
- Flasheado `lineage13-boot.img` por **fastboot** (`adb reboot bootloader` → `fastboot flash boot`). El `dd` desde Android NO flashea (protección de bloque/verity).
- LineageOS conecta a "cpcd" WPA2 y **coge DHCP** (IP `192.168.0.111`, COMPLETED).
- **Blobs idénticos** stock vs LineageOS (7 ficheros, mismo md5) → el FW es el mismo.
- Conclusión: el driver original (mt_wifi, source `conn_soc/drv_wlan/mt_wifi`) SÍ activa el data-path. **Nuestro driver no. La diferencia es 100% software del host.**

### 3. Captura del FSM del driver original
- `~/wifi-fw/lineage_fsm_capture.log` (en la Pi): 287 líneas del dmesg de LineageOS durante un connect exitoso.
- Secuencia AIS FSM: `[0]→[2](SCAN)→[0]→[1]→[6](JOIN)→[7](JOIN_COMPLETE)→[10](NORMAL_TR)`.
- Secuencia SAA FSM: `[0]→[1](AUTH tx)→[2](AUTH wait)→[5](ASSOC tx)→[6](ASSOC wait)→[0]`.
- Los **CMD_ID individuales NO se capturaron** (el driver los loguea a nivel TRACE, apagado; `/proc/wlan` no existe en este build; `/proc/driver/wmt_dbg` requiere `/proc/driver/wmt_psm`).

---

## Lo PROBADO y REFUTADO (no repetir)

| Hipótesis | Resultado |
|---|---|
| `enc_status=ENABLED(6)` vs `KEY_ABSENT(7)` | Ambos probados, DHCP falla igual. **7 es correcto** (downstream `nic.c` manda KEY_ABSENT). |
| Quitar `POWER_SAVE_MODE` + `SET_RX_FILTER` del join (clon downstream) | **REGRESIÓN**: el handshake ni completa (rx=0, ni EAPOL). Esos comandos SON necesarios en nuestro driver (el downstream los manda en otros sitios que nosotros no replicamos). Revertido. |
| `STATE_2` intermedio del STA record (hipótesis SAA FSM) | **REFUTADO**: `cnmStaRecChangeState(STATE_2)` no manda al FW (cláusula `do nothing` en cnm_mem.c:994 — solo state interno del host). El FW recibe STATE_1 y STATE_3 = lo mismo que nosotros. |
| `SET_BSS_RLM_PARAM(0x1d)` separado | **No aplica**: `rlmSyncOperationParams` no se llama en el join (solo en rlm.c:1295 / p2p), y sus campos son idénticos a nuestro `rlm` embebido. |
| `key_type` invertido | Probado, no era. |
| Structs de los comandos | **Coinciden byte a byte** (SET_BSS_INFO, CMD_802_11_KEY, UPDATE_STA_RECORD). |

---

## Las vías que quedan (la diferencia es DINÁMICA, no estática)

Todo lo comparable a nivel de source/structs/secuencia coincide. La diferencia está en el **comportamiento runtime** del driver completo vs el nuestro.

### 🔴 Vía A — Capturar los CMD_ID exactos del driver original (LA DECISIVA)
El driver `mt_wifi` de LineageOS manda los CMD al FW por el HIF. Si capturamos esa secuencia en una conexión exitosa y la comparamos con la nuestra, **la diferencia es el fix**.
- **Bloqueo**: necesita el debug level TRACE del driver, que requiere `/proc/wlan` (no existe en este build) o recompilar LineageOS.
- **Alternativas**: (a) buscar el module param/sysfs del debug level (`find /sys/module -iname *dbg*` ya probado, vacío); (b) el driver MTK tiene ioctls privados para el debug — invocarlos via la app EngineerMode o un tool; (c) recompilar el kernel de LineageOS con `CFG_WIFI_DBG=log` (impracticable).

### 🟠 Vía B — Dump comparativo de la RAM del FW (MUY FUERTE)
LineageOS conectado y funcionando deja el FW en un estado interno correcto. pmOS conectado (roto) lo deja en estado incorrecto. Si dumpeamos la RAM del FW en ambos estados y los comparamos, **la diferencia en los campos internos (+0xa8, +0x35c, +0x289) revela qué no transiciona**.
- Funciones del FW ya identificadas (en `fw-analysis/`): `nic_privacy_arm_tx_enc` (f004bb2c), `privacy_enc_check` (f002a480), `privacy_port_control` (f002a4cc), `hal_config_wtbl_enc` (f004b6cc).
- **Bloqueo**: el driver mt_wifi de LineageOS NO expone `fwdump` (es de nuestro driver). Habría que escribir un módulo para el kernel 3.10 de LineageOS que lea la RAM del FW via ACCESS_REG — involved pero factible con adb root.

### 🟡 Vía C — Construcción de los frames AUTH/ASSOC
Nuestro driver construye AUTH/ASSOC a mano (`wifi_send_auth`/`wifi_send_assoc`). El downstream usa el SAA FSM con paths distintos. Diferencias en los IEs (RSN, rates, capabilities) podrían afectar cómo el FW/AP configuran el BSS cifrado. Comparar los bytes del ASSOC-REQ nuestro vs el del downstream.

---

## Infra / recetas

- **Dual-boot**: `adb reboot bootloader` → `fastboot flash boot boot-btifDMA-sd.img` (pmOS) / `fastboot flash boot lineage13-boot.img` (LineageOS). El `dd` desde Android NO flashea.
- **LineageOS**: `adb` funciona con root tras `adb root`. WiFi habilitar: `svc wifi enable` (a veces necesita `input keyevent 224; svc power stayon true` para evitar early_suspend). Redes en `/data/misc/wifi/wpa_supplicant.conf` (hola, cpcd configuradas).
- **pmOS**: `ssh root@172.16.42.1` via usb0 (la Pi es `172.16.42.2`). Si sshd no arranca tras reboot → power-cycle físico.
- **Test WPA2 pmOS** (sin crashear): ver `HANDOFF-DHCP-0625.md` (wpa_supplicant manual + udhcpc; NO `iw scan` estando conectado).
- **RE del FW**: `fw-analysis/` en el repo + scripts reproducibles. Ghidra 12.1.2 con NDS32 y JDK arm64.
- **pstore**: `/sys/fs/pstore/console-ramoops-0` (leer antes de reiniciar).

## Módulos del FW relevantes (del mapeo)
- `txm.c` (26 funcs) — TX manager
- `rxm.c` (15 funcs) — RX manager
- `privacy.c` (2), `nic_privacy.c` (1), `hal.c` (1) — cifrado
- `pwr_mgt_fsm.c` (39), `cnm.c` (13), `bss.c` (8) — gestión de conexión/estado

---

## Reglas de oro (no cambiar)
- `fastboot flash boot` NUNCA `fastboot boot`. mtkclient `wo` NUNCA `wf`. No flashear preloader.
- NO hacer `iw dev wlan0 scan` estando conectado WPA2 en pmOS (crashea).
- `reboot` normal en pmOS a veces no rearranca con Phosh arriba → `reboot -f`.
- Si sshd no arranca en pmOS → power-cycle físico.

*Co-autor: Claude (glm-5.2).*
