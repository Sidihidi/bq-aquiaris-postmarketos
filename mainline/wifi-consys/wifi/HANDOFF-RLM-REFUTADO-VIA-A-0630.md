# HANDOFF — Fix RLM (short_slot/preamble) refutado + Vía A LineageOS (2026-06-30)

## TL;DR

- **Fix RLM (`short_slot`, `short_preamble`, `nonht_basic_phy`)**: aplicado, probado en HW. **WPA2 handshake OK, DHCP sigue fallando.** El fix RLM **NO era la causa.**
- **Todas las hipótesis "estáticas" del join están AGOTADAS**. La diferencia con el downstream es **dinámica**.
- **Próximo paso**: **Vía A — capturar CMD_ID exactos del driver original en LineageOS.**

---

## Test del fix RLM (2026-06-30, sesión opencode)

### Qué se aplicó
En `wifi_send_join()`, se añadieron 3 parámetros RLM que el downstream extrae del ASSOC-RESP del AP pero nosotros siempre mandábamos a 0:

```c
// ANTES (todo a 0 del bi = {}):
// bi.rlm.short_slot = 0
// bi.rlm.short_preamble = 0
// bi.nonht_basic_phy = 0

// DESPUÉS (extraído del ASSOC-RESP):
w->assoc_cap_info = le16_to_cpu(*(__le16 *)(frame + 24));  // cap_info del AP
bi.rlm.short_slot = (w->assoc_cap_info & BIT(10)) ? 1 : 0;
bi.rlm.short_preamble = (w->assoc_cap_info & BIT(5)) ? 1 : 0;
bi.nonht_basic_phy = 1;  // PHY_TYPE_ERP_INDEX (11g)
```

### Resultado en HW
```
AP: "hola" WPA2-PSK, BSSID e6:92:82:f4:ce:44, ch=10 (2457MHz), signal -27dBm
Scan: ✅ (5 redes visibles)
Handshake: ✅ COMPLETED (pairwise=CCMP, group=CCMP, key_mgmt=WPA2-PSK)
PTK: add_key idx=0 cipher=0xfac04 peer=e6:92:82:f4:ce:44 len=16 tx_key=1
GTK: add_key idx=2 cipher=0xfac04 peer=ff:ff:ff:ff:ff:ff len=16 tx_key=0
DHCP: ❌ 6 DISCOVERs, 0 OFFERs
rx=4101 (beacons), tx=48415 (retransmisiones)
Ping 8.8.8.8: 100% packet loss
```

**Idéntico al comportamiento sin el fix.** Los parámetros RLM no afectan al data-path cifrado.

---

## Estado de TODAS las hipótesis probadas

| # | Hipótesis | Estado | Probado por |
|---|---|---|---|
| 1 | `enc_status` 6 vs 7 | ❌ refutado | Claude 0624 |
| 2 | Quitar POWER_SAVE + RX_FILTER del join | ❌ REGRESIÓN | Claude 0630 |
| 3 | STATE_2 intermedio del STA | ❌ refutado | Claude 0630 |
| 4 | RLM (0x1d) separado | ❌ no aplica | Claude 0630 |
| 5 | Structs de comandos | ❌ idénticos byte a byte | Claude 0630 |
| 6 | `key_type` invertido | ❌ refutado | opencode 0630 |
| 7 | **RLM `short_slot`/`short_preamble`/`nonht_basic_phy`** | ❌ **refutado** | **opencode 0630** |

**Todas las hipótesis estáticas (comandos, structs, parámetros del join) están agotadas.**
La diferencia con el downstream es **100% dinámica** — algo que el driver hace en runtime
que no se puede inferir del source code estático.

---

## La causa confirmada

- **LineageOS funciona con WPA2 + DHCP** (probado, IP 192.168.0.111).
- **Mismo FW, mismo HW, mismos blobs** (7 ficheros, md5 idéntico).
- **La diferencia es 100% del driver host.**

El driver original (`mt_wifi`) hace algo en runtime que activa el data-path cifrado.
Nuestro driver no lo hace. No es un comando que falte en el join ni un parámetro incorrecto.
Es un **comportamiento dinámico** del driver completo.

---

## ▶️ VÍA A — Capturar CMD_ID del driver original en LineageOS

### Objetivo
Arrancar LineageOS, conectar WPA2 a "hola"/"kakatua1" (que SÍ funciona),
y capturar la secuencia EXACTA de comandos que el driver original manda al FW.
Comparar con nuestra secuencia. **La diferencia ES el fix.**

### Paso 1 — Arrancar LineageOS

```sh
# EN LA PI (cpcd@192.168.0.38 o .123):

# Si el móvil está en pmOS:
ssh root@172.16.42.1 "echo 1 > /proc/sys/kernel/sysrq; echo b > /proc/sysrq-trigger"
# NO funciona (warm reboot no entra fastboot). Hacer:
# POWER-CICLE FÍSICO + mantener Vol+ para entrar en fastboot
# O si ya estás en pmOS por SSH:
ssh root@172.16.42.1 "sync; reboot -f"
# esperar a que arranque, luego entrar en fastboot:
# (combinación Power+Vol+ o Power ~10s luego Power+Vol+)

# Una vez en fastboot:
sudo fastboot devices    # debe mostrar el dispositivo
sudo fastboot flash boot ~/lineage13-boot.img
sudo fastboot reboot
```

### Paso 2 — Configurar LineageOS para debug

```sh
# esperar ~40s a que LineageOS arranque
# adb funciona (USB gadget se enumera como 18d1:4ee2)
adb devices              # debe aparecer
adb root                 # root prompt (uid=0)
adb shell setenforce 0   # relajar SELinux

# activar pantalla + evitar suspend:
adb shell "input keyevent 224; svc power stayon true"

# verificar que el driver está cargado:
adb shell "ls /sys/module/wlan_hdr/ 2>/dev/null; ls /proc/driver/wmt* 2>/dev/null; ls /sys/module/wlan/ 2>/dev/null"
```

### Paso 3 — Activar el debug del driver MTK

El driver `mt_wifi` tiene un debug level que controla qué logs imprime.
Por defecto está en INFO (solo FSM transitions, no CMD_ID individuales).
Necesitamos subirlo a TRACE o DEBUG para ver los comandos.

**OPCIÓN A — Module parameters (probar primero):**
```sh
adb shell "
  # buscar el debug level parameter
  find /sys/module -iname '*dbg*' -o -iname '*debug*' -o -iname '*log*' 2>/dev/null | grep -i wlan
  # buscar en /proc
  ls /proc/driver/wmt* 2>/dev/null
  cat /proc/driver/wmt_psm 2>/dev/null
  # intentar activar:
  echo '15 1' > /proc/driver/wmt_psm 2>/dev/null
  cat /proc/driver/wmt_dbg 2>/dev/null
"
```

**OPCIÓN B — Driver ioctl privado:**
```sh
# El driver MTK expone ioctls privados para el debug level.
# Se pueden invocar via la app EngineerMode o via iwpriv:
adb shell "iwpriv wlan0 driver 'debug 5'" 2>/dev/null
adb shell "iwpriv wlan0 driver 'dbg 5'" 2>/dev/null
# Probar varios niveles (0-5):
for i in 0 1 2 3 4 5; do
  adb shell "iwpriv wlan0 driver 'debug $i'" 2>/dev/null
done
```

**OPCIÓN C — Kernel module parameter:**
```sh
adb shell "
  # buscar el parámetro del módulo del kernel
  find /sys/module -name '*debug*' -path '*wlan*' 2>/dev/null
  ls /sys/module/wlan_hdr/parameters/ 2>/dev/null
  # intentar:
  echo 5 > /sys/module/wlan_hdr/parameters/fgDriverDebugLevel 2>/dev/null
  # o:
  echo 5 > /sys/module/wlan/parameters/fgDriverDebugLevel 2>/dev/null
"
```

**OPCIÓN D — wpa_supplicant debug (si nada del driver funciona):**
```sh
# Si no se puede activar el debug del driver, al menos capturar el wpa_supplicant:
adb shell "stop wpa_supplicant"
adb shell "wpa_supplicant -iwlan0 -Dnl80211 -dd -t -c/data/misc/wifi/wpa_supplicant.conf" &
# -dd = debug máximo, -t = timestamps
# conectar al AP:
adb shell "wpa_cli -iwlan0 scan; sleep 5; wpa_cli -iwlan0 scan_results"
adb shell "wpa_cli -iwlan0 add_network; wpa_cli -iwlan0 set_network 0 ssid '\"hola\"'; wpa_cli -iwlan0 set_network 0 psk '\"kakatua1\"'; wpa_cli -iwlan0 enable_network 0"
sleep 10
adb shell "wpa_cli -iwlan0 status | grep wpa_state"   # COMPLETED
adb shell "ip addr show wlan0 | grep inet"             # IP
```

### Paso 4 — Capturar la secuencia de conexión WPA2

```sh
# limpiar dmesg
adb shell "dmesg -c > /dev/null"

# ASEGURARSE de que el debug del driver está activo (ver Paso 3)

# conectar a "hola" WPA2:
adb shell "svc wifi disable; sleep 3; svc wifi enable"
sleep 30

# verificar conexión:
adb shell "dumpsys wifi | grep mWifiInfo"   # debe mostrar SSID + IP

# volcar el dmesg COMPLETO:
adb shell "dmesg" > ~/wifi-fw/lineage_cmd_trace.log

# también capturar el logcat del wpa_supplicant:
adb shell "logcat -d -b all | grep -iE 'wpa|wlan|wifi|CMD|cmd|key|encrypt|EAPOL|join|assoc|auth'" >> ~/wifi-fw/lineage_wpa_trace.log
```

### Paso 5 — Analizar la captura

```sh
# buscar CMD_ID individuales en el dmesg:
grep -iE 'CMD|cmd_id|wlanoid|nicUpdateBss|SET_BSS|ADD_KEY|UPDATE_STA|POWER_SAVE|RX_FILTER|CH_PRIV|PM_BSS|WMM|RLM|ACCESS_REG|SCAN_REQ' ~/wifi-fw/lineage_cmd_trace.log

# buscar transiciones del FSM:
grep -iE 'aisFsm|saaFsm|AIS_STATE|JOIN_COMPLETE|NORMAL_TR' ~/wifi-fw/lineage_cmd_trace.log

# buscar cualquier comando que el driver manda al FW y nosotros no:
# comparar con nuestra secuencia en wifi_send_join():
# SET_BSS_INFO -> UPDATE_STA_RECORD(STATE_3) -> POWER_SAVE_MODE(CAM) -> SET_RX_FILTER(0x0B) -> INDICATE_PM_BSS_CONNECTED -> CH_PRIVILEGE(ABORT)
```

### Paso 6 — Volver a pmOS

```sh
adb reboot bootloader
sudo fastboot flash boot ~/mainline/pkg/boot-rlm-fix.img   # o boot-btifDMA-sd.img
sudo fastboot reboot
# esperar ~50s, luego: ssh root@172.16.42.1
```

---

## Lo que buscar (qué comandos puede mandar el driver original que nosotros no)

### Comandos que YA mandamos (confirmado):
- `CMD_ID_SET_BSS_INFO` (con conn_state, auth_mode, enc_status, RLM, rates)
- `CMD_ID_UPDATE_STA_RECORD` (STATE_3, con rates, phy_type, need_resp=1)
- `CMD_ID_POWER_SAVE_MODE` (CAM = 0)
- `CMD_ID_SET_RX_FILTER` (0x0B = DIRECTED|MULTICAST|BROADCAST)
- `CMD_ID_INDICATE_PM_BSS_CONNECTED` (con AID, beacon_interval, DTIM)
- `CMD_ID_CH_PRIVILEGE` (ABORT, release channel)
- `CMD_ID_ADD_REMOVE_KEY` (PTK pairwise + GTK group, CCMP)

### Comandos que NO mandamos (candidatos):
- `CMD_ID_SET_BSS_RLM_PARAM` (0x1d) como comando **separado** (ya refutado)
- `CMD_ID_UPDATE_WMM_PARMS` (0x21) — **NO refutado** todavía. El downstream
  llama `mqmProcessAssocRsp()` que parsea WMM IEs del ASSOC-RESP y podría
  mandar este comando internamente.
- Cualquier `CMD_ID_*` que el driver mande en respuesta a eventos del FW
  (no en el join, sino tras recibir `EVENT_ACTIVATE_STA_REC(0x13)` etc.)

### Lo que también puede diferir (dinámico):
- **Timing**: el driver original manda los comandos con pausas/eventos entre ellos
  (via el FSM de ais). Nuestro driver los manda todos en un bloque síncrono.
- **Orden**: el FSM de ais manda los comandos en un orden específico dictado
  por las transiciones de estado. Nuestro orden podría ser diferente.
- **Comandos secundarios**: funciones como `cnmStaRecChangeState()` o
  `rlmSyncBssParameter()` pueden mandar CMD_IDs al FW como efecto secundario.

---

## Estado del dispositivo
| Subsistema | Estado |
|---|---|
| WiFi ABIERTO | ✅ navega |
| WiFi WPA2 handshake | ✅ COMPLETED |
| WiFi WPA2 DHCP | ❌ rx=4101 (beacons), 0 data frames |
| BT (hci0) | ✅ |
| Display + Phosh | ✅ |
| Touch | ✅ |
| Brillo slider | ✅ |

---

## Kernel actual en el móvil
- `boot-rlm-fix.img` (kernel con fix RLM + todos los fixes previos de estabilidad).
- Sector 83968. Compilado 30-jun 01:51 BST.
- El fix RLM **no rompe nada** (redes abiertas siguen funcionando, handshake WPA2 OK).

## Reglas de oro
- `fastboot flash boot` NUNCA `fastboot boot`. mtkclient `wo` NUNCA `wf`.
- NO `iw dev wlan0 scan` estando conectado WPA2 en pmOS (a veces crashea).
- `dd` desde pmOS funciona; desde Android NO (verity).
- Reboot: `echo 1 > /proc/sys/kernel/sysrq; echo b > /proc/sysrq-trigger`.
- Si sshd no arranca → power-cycle físico.
- `/sys/fs/pstore/console-ramoops-0` — leer antes de reiniciar tras crash.

*Co-autor: opencode (glm-5.2).*
