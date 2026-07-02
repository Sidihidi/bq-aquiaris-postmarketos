# HANDOFF — Test key_type WPA2 + análisis firmware NDS32 (2026-06-30)

## TL;DR

- **Test key_type invertido**: WPA2 handshake OK, DHCP sigue fallando. **El key_type NO era el bug.**
- **Análisis del firmware WiFi (NDS32)**: descompilado con Ghidra, identificadas funciones de `privacy.c`, `nic_privacy.c`, `hal.c`.
- **El bug real del WPA2 sigue SIENDO el data-path RX**: 0 paquetes DATA recibidos tras el 4-way handshake.
- **Próximo paso**: comparar `wifi_send_join()` con `aisFsmRunEventJoinComplete` del downstream comando a comando.

---

## 1. Test key_type (0=pairwise vs 1=group)

### Hipótesis
El downstream define `ENUM_WPI_PAIRWISE_KEY=0, ENUM_WPI_GROUP_KEY=1`. Nuestro driver tenía `key_type = pairwise ? 1 : 0` (invertido). Se cambió a `pairwise ? 0 : 1`.

### Resultado en HW
- **WPA2 handshake**: ✅ COMPLETED (`pairwise=CCMP`, `group=CCMP`, `key_mgmt=WPA2-PSK`)
- **PTK instalada**: `add_key: PTK idx=0 cipher=0xfac04 peer=e6:92:82:f4:ce:44 len=16 tx_key=1`
- **GTK instalada**: `add_key: GTK idx=1 cipher=0xfac04 peer=ff:ff:ff:ff:ff:ff len=16 tx_key=0`
- **DHCP**: ❌ SIGUE FALLANDO (6 DISCOVERs, 0 OFFERs)
- **rx=10789** (todos beacons del scan pre-connect), **tx=157707** (retransmisiones)
- **BSS_BEACON_TIMEOUT (0x1b)** a los 30s post-connect
- **tcpdump en wlan0**: 0 paquetes (ningún data frame llega a la interfaz)

### Conclusión
**El key_type NO era el bug.** Se ha revertido al valor original (`pairwise ? 1 : 0`).
El `ucKeyType` en el `CMD_802_11_KEY` probablemente usa un mapeo distinto al `ENUM_WPI_KEY_T`
(0=group, 1=pairwise — al revés del WPI enum, que es para WAPI chino).

### Datos del test
```
AP: "hola" WPA2-PSK, pass "kakatua1", BSSID e6:92:82:f4:ce:44, ch=10 (freq 2457)
Scan: OK (7 redes encontradas, "hola" visible)
Connect: wpa_state=COMPLETED en ~8s
DHCP: 6 DISCOVER broadcasts, 0 OFFERs recibidos
Ping 8.8.8.8: 100% packet loss
```

---

## 2. Análisis del firmware WiFi (arquitectura NDS32)

### Descubrimiento clave
El MCU del CONSYS es un **Andes NDS32** (no ARM). El SoC principal son 4× Cortex-A7,
pero el CONSYS tiene su propio core NDS32 que corre un RTOS propietario de MediaTek.

### Cifrado del firmware
- `WIFI_RAM_CODE_SOC` (207KB): 4 secciones MTKW. La sección 2 (código principal, 180KB) está **cifrada con AES** (`CFG_ENABLE_FW_ENCRYPTION=1`).
- La clave AES está **hardcoded en la BootROM del NDS32** — NO aparece en el driver downstream.
- **Se obtuvo la RAM descifrada** leyendo el chip con `wifi_runtime_reg_read` (CMD_ID_ACCESS_REG=0xc2 por puerto 1). ~2000 palabras/s.
- La sección 2 descifrada se volcó a `fw_f0020000.bin` (196KB) y se descompiló con Ghidra.

### Patches WMT
- `mt6572_82_patch_e1_0_hdr.bin` (40KB): parchea la ROM del NDS32. Contiene handlers de WiFi/GPS ON/OFF, coexistencia GPS↔WiFi (DESENSE).
- `mt6572_82_patch_e1_1_hdr.bin` (20KB): parchea la ROM. Contiene exception handler (coredump), WIFI_READY logic ("Titus - VAR_WIFI_READY"), timeout del host (TIME_HOST_SYNC_TOO_LONG).

### Módulos del FW (de la rodata)
```
wifi/mgmt/: bcm bss cnm hem hem_cmd hem_mbox hem_pwr mqm privacy pwr_mgt_fsm
            pwr_apsd rate rlm rlm_domain rlm_protection rlm_statistics rxm scan
            scan_fsm stats tdls txm
wifi/mgmt/mt6582/: roaming_fsm hem_efuse rlm_phy rlm_sensor rlm_test rlm_txpwr
wifi/nic/: nic nic_dma nic_pm nic_privacy
wifi/nic/HAL/mt6582/: hal
wifi/Service/: wifi_init
```

### Funciones descompiladas clave (Ghidra)
| Función | Origen | Rol |
|---|---|---|
| `FUN_ram_f002a480` | privacy.c:0x191 | Comprueba si `enc_status > 2` → cifrado activo |
| `FUN_ram_f002a554` | privacy.c:0x504 | Comprueba flag `0x35c` (clave instalada) |
| `FUN_ram_f002a4cc` | privacy.c | **Port control**: decide si un frame pasa o se descarta según cifrado |
| `FUN_ram_f004bb2c` | nic_privacy.c | **Arma el cifrado TX**: lee WTBL, instala clave en hardware |
| `FUN_ram_f004be5c` | hal.c | Acceso a WTBL (tabla por-STA con clave/cipher) |
| `FUN_ram_f004b6cc` | hal.c | Configura WTBL para cifrado cuando `enc_status > 2` |

### String addresses en rodata (para xrefs en Ghidra)
```
"wifi/mgmt/privacy.c"     @ 0xf0063590
"wifi/nic/nic_privacy.c"  @ 0xf006486c
"wifi/nic/HAL/mt6582/hal.c" @ 0xf0064898
"wifi/nic/nic.c"          @ 0xf0064834
"wifi/mgmt/txm.c"         @ 0xf006380c
```

### Ficheros generados (en workspace Windows)
```
fw_decompiled.c          190KB  — firmware descompilado a C por Ghidra
fw_functions.txt         12KB   — lista de 648 funciones con tamaños
fw_key_functions.txt     1.2KB  — xrefs de strings a funciones (privacy/hal/nic_privacy)
fw_f0020000.bin          196KB  — dump RAM del chip descifrado (código principal NDS32)
fw_6a000_stub.bin        6KB    — stub de arranque NDS32
fw_0209f800_data.bin     12KB   — datos/BDU
fw_f0063000_rodata.bin   8KB    — rodata con strings de módulos
fw_strings.txt           13KB   — strings extraídos de cada sección
```

---

## 3. El bug real del WPA2 — dónde estamos

### Confirmado (NO es la causa)
- ❌ `enc_status` 6 vs 7: probados ambos, DHCP falla igual
- ❌ `key_type` 0 vs 1: probado invertido, DHCP falla igual
- ❌ MAC del netdev: ya usa `w->mac` (fix aplicado)
- ❌ `set_default_key`: implementado pero wpa_supplicant no lo llama (el downstream también es no-op)
- ❌ `tx_key` en GTK: ya corregido (solo PTK es tx_key)

### Síntomas invariantes
- WPA2 handshake: ✅ COMPLETED (PTK+GTK CCMP instaladas)
- DHCP: ❌ 0 OFFERs recibidos
- `rx_packets` post-connect: solo beacons, **0 data frames**
- `tx_packets`: sube (retransmisiones de DISCOVER)
- TX-DONE: solo seq=1,2 (mgmt AUTH+ASSOC), **0 TX-DONE para datos**
- `0x1b` (BSS_BEACON_TIMEOUT) a los 30s
- tcpdump en wlan0: 0 paquetes

### Hipótesis principales (a investigar)
1. **Faltan comandos en `wifi_send_join()`**: comparar con `aisFsmRunEventJoinComplete` del downstream. Candidatos:
   - `CMD_ID_SET_BSS_RLM_PARAM(0x1d)` como comando **separado** (no embebido en SET_BSS_INFO)
   - `CMD_ID_UPDATE_WMM_PARMS(0x21)`
   - `CMD_ID_INDICATE_PM_BSS_CREATED(0x19)` con parámetros correctos (beacon_interval, DTIM, AID)

2. **El FW no transiciona de handshake→data-mode**: tras instalar las claves, el FW debería activar el cifrado en la WTBL automáticamente. Pero quizá necesita un comando adicional para esto.

3. **BSS_BEACON_TIMEOUT (0x1b) mata el BSS**: el FW pierde beacons a los 30s. El CAM (power-save always-on) debería evitarlo pero no lo hace. El DHCP se lanza dentro de esa ventana de 30s pero tampoco funciona.

### Próximos pasos recomendados
1. **Comparar `wifi_send_join()` con `aisFsmRunEventJoinComplete`** del downstream (`~/mainline/downstream/.../drv_wlan/mt6628/wlan/mgmt/ais_fsm.c`). Listar TODOS los CMD_ID que manda el downstream tras el assoc, en orden. Añadir los que falten.

2. **Usar `wifi_runtime_reg_read` para volcar la WTBL** tras el 4-way + add_key. La WTBL tiene la entrada del STA con la clave y el cipher. Comparar con lo que el FW espera. Dirección de la WTBL: buscar en `hal.c` descompilado (FUN_ram_f004be5c).

3. **Añadir instrumentación RX**: log de `l0/pt` en cada pasada de `wifi_rx_drain`. Así veremos si la OFFER llega al HIF (FW entrega, driver falla) o nunca llega (FW no la inyecta).

---

## 4. Estado del dispositivo
| Subsistema | Estado |
|---|---|
| Display + Phosh (lima) | ✅ |
| Touch + teclado | ✅ |
| Backlight slider | ✅ (gsd-power) |
| Bluetooth (hci0) | ✅ |
| USB gadget | ✅ |
| WiFi ABIERTO | ✅ navega |
| WiFi WPA2 | 🟡 handshake OK, DHCP falla (rx=0 data) |
| iw scan conectado | ⚠️ a veces crashea (cfg80211_scan_done) |
| GPS | 🟡 no streamea |
| FM | ❌ |

---

## 5. Entorno
- **Pi de casa**: `cpcd@192.168.0.38` (sudo NOPASSWD). Kernel `~/mainline/linux-7.0.12`.
- **Móvil**: `root@172.16.42.1`. SSH: `sudo ip addr replace 172.16.42.2/24 dev usb0`.
- **Build+flash**: `~/wifi-iter-w.sh` en la Pi (build zImage + dd sector 83968 + reboot).
- **Downstream**: `~/mainline/downstream/drivers/misc/mediatek/combo/drv_wlan/mt6628/wlan/`.
- **FW descompilado**: workspace Windows `C:\Users\jferr\Desktop\pmos-krillin\fw_decompiled.c`.
- **Ghidra**: `C:\Users\jferr\Desktop\ghidra_12.1.2_PUBLIC_20260605` (procesador NDS32:LE:32).
- **pstore**: `/sys/fs/pstore/console-ramoops-0` en el móvil (leer antes de reiniciar).

## 6. Reglas de oro
- `fastboot flash boot` NUNCA `fastboot boot`. mtkclient: `wo` NUNCA `wf`.
- NO flashear preloader. NO restaurar LK Lollipop.
- Reboot: `echo 1 > /proc/sys/kernel/sysrq; echo b > /proc/sysrq-trigger`.
- Si el móvil se cuelga: power-cycle físico (Power ~10s o quitar batería).
- NO hacer `iw dev wlan0 scan` estando conectado WPA2 (a veces crashea).

## 7. Test WPA2 reproducible
```sh
# en el móvil (root@172.16.42.1):
rfkill unblock wlan; ip link set wlan0 up
# wpa.conf ya existe en /tmp/wpa.conf (hola/kakatua1)
wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -B
sleep 8
wpa_cli -i wlan0 status | grep wpa_state   # COMPLETED
udhcpc -i wlan0 -t 6 -n -q                 # falla sin OFFER
cat /sys/class/net/wlan0/statistics/rx_packets   # ~10789 (solo beacons)
```
AP: SSID "hola", WPA2-PSK, pass "kakatua1", BSSID e6:92:82:f4:ce:44, ch=10 (2457MHz).

---

*Co-autor: opencode (glm-5.2) + Claude Opus 4.8.*
