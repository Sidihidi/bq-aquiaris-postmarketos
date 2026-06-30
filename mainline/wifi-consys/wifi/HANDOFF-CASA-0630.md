# HANDOFF CASA — sesión 30-jun-2024 (Claude glm-5.2, maratoniana)

**Lee esto PRIMERO.** Tiene el estado exacto, qué probar al sentarte, y por dónde seguir.

---

## TL;DR — dónde estamos

- **WPA2 control-plane**: ✅ (handshake COMPLETED, PTK+GTK CCMP).
- **WPA2 data-path**: ❌ (`rx=2` = solo EAPOL, 0 data; DHCP sin OFFER).
- **Causa acotada**: **100% del driver host** (LineageOS funciona con FW idéntico).
- **5 hipótesis refutadas** (enc_status, POWER_SAVE/RX_FILTER, STATE_2, RLM, structs).
- **Firmware descompilado y mapeado** (`fw-analysis/`).
- **`fwdump` añadido al driver** pero la RAM de datos del FW (0x020axxxx) cuelga el bus.

---

## ⚠️ ESTADO DEL MÓVIL AL CERRAR

- **pmOS** arrancado, kernel 7.0.12 con `fwdump` en debugfs.
- El último flash completó (dd desde pmOS + `reboot -f`). El kernel tiene `fwdump` (verificado: `grep -c fwdump /proc/kallsyms` = 3).
- **PERO la corrección del puerto 0 en `wifi_runtime_reg_read` NO se ha testado todavía** (el rebuild+flash completó pero no llegamos a probar fwdump antes de cerrar).
- La Pi (`192.168.0.123`) está viva. Acceso: `ssh root@172.16.42.1` (USB gadget, `sudo ip addr replace 172.16.42.2/24 dev usb0` en la Pi).

---

## ▶️ PASO 1 al sentarte — verificar fwdump (2 min)

Conecta por SSH al móvil y prueba:

```sh
# reconectar USB (en la Pi):
sudo ip addr replace 172.16.42.2/24 dev usb0
ssh root@172.16.42.1

# test fwdump con dirección de código FW (conocida):
echo "f0020000 4" > /sys/kernel/debug/mt6582_wifi/fwdump_cfg
cat /sys/kernel/debug/mt6582_wifi/fwdump
```

**Interpretación:**
- Si devuelve `f0020000=6400ff46` (o cualquier valor DISTINTO por dirección) → ✅ **fwdump funciona con la corrección del puerto 0**. Seguir al PASO 2.
- Si devuelve `f0020000=00006627` (todos iguales) → ❌ **la corrección no flasheó**. Rebuild + flash de nuevo:
  ```sh
  # en la Pi:
  scp mt6582-wifi.c de este repo a ~/mainline/linux-7.0.12/drivers/soc/mediatek/
  cd ~/mainline/linux-7.0.12 && make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage dtbs
  # empaquetar + dd (ver PASO 0 abajo)
  ```

## ▶️ PASO 2 — leer el estado interno del FW (LA VÍA B)

Si fwdump funciona (PASO 1 OK), probar leer la RAM de datos del FW:

```sh
# PRECAUCIÓN: esto PUEDE COLGAR el móvil (WDT reset). Estar preparado para power-cycle.
echo "020a0068 1" > /sys/kernel/debug/mt6582_wifi/fwdump_cfg
cat /sys/kernel/debug/mt6582_wifi/fwdump
```

**Interpretación:**
- **Devuelve un valor** (un puntero, ej `020a0068=6030xxxx`) → ✅ **la RAM de datos es legible**. Seguir leyendo:
  - El valor es un puntero al BSS record del AIS. Sumarle offsets:
    - `puntero + 0xa8` = el state del BSS (**debe ser 3** para que el data-path funcione).
    - `puntero + 0x35c` = flag de "clave instalada" (**debe ser != 0**).
  - `echo "<puntero+a8 en hex> 1" > fwdump_cfg; cat fwdump`
  - Si `+0xa8 ≠ 3` o `+0x35c == 0` → **BUG CAZADO**: el FW no transicionó su estado interno.

- **Cuelga (móvil se reinicia)** → ❌ **la RAM de datos NO es legible por ACCESS_REG** (era el caso en la sesión del 30-jun). La Vía B está bloqueada para los state variables. **Pasar a la VÍA A** (PASO 3).

## ▶️ PASO 3 — VÍA A: capturar los CMD_ID del driver original en LineageOS

Si la Vía B está bloqueada, la alternativa más fuerte es capturar la secuencia EXACTA de comandos que el driver original (`mt_wifi`) manda al FW en una conexión WPA2+DHCP exitosa, y compararla con la nuestra.

### 3a. Arrancar LineageOS
```sh
# en la Pi: entrar en fastboot y flashear LineageOS
ssh root@172.16.42.1 "reboot bootloader"   # o: echo b > /proc/sysrq-trigger NO (eso es warm reboot)
# si no entra en fastboot por software: combo manual Power ~10s → Power+Vol↑
fastboot flash boot ~/lineage13-boot.img     # o donde esté lineage13-boot.img
fastboot reboot
```

### 3b. Activar el debug del driver mt_wifi
El móvil arranca LineageOS (Android 6). Acceder por **adb** (la Pi ve el dispositivo USB como `18d1:4ee2`):
```sh
adb root                          # ya funciona (uid=0)
adb shell setenforce 0            # relajar SELinux
# el debug del WMT:
echo "15 1" > /proc/driver/wmt_psm    # activar PSM log (wmt_dbg lo pide)
cat /proc/driver/wmt_dbg              # ver si devuelve logs del driver
# habilitar WiFi y conectar:
input keyevent 224; svc power stayon true
svc wifi enable
sleep 25
# el supplicant autoconecta a cpcd/hola (están en wpa_supplicant.conf)
dumpsys wifi | grep mWifiInfo         # confirmar SSID + IP
```

### 3c. Capturar la secuencia
```sh
# limpiar dmesg + reconectar
adb shell "dmesg -c >/dev/null; svc wifi disable; sleep 3; svc wifi enable"
sleep 30
# volcar el dmesg del driver durante la conexión exitosa
adb shell "dmesg | grep -iE 'wlan|aisFsm|saaFsm|wlanoid|nic|CMD|set_key|add_key'" > lineage_cmd_trace.log
```

### 3d. Comparar
- Los logs del driver MTK muestran las transiciones del FSM (AIS, SAA) y llamadas a `wlanoidSet*`.
- Difear contra nuestra secuencia (la del ramoops en `HANDOFF-DHCP-0625.md`).
- **Lo que el original mande y nosotros no = el fix.**
- Si el debug level no muestra los CMD_ID individuales (solo FSM states), buscar:
  - `/sys/module/wlan/parameters/` para un flag de debug level.
  - La app EngineerMode de MTK (puede activar el driver log).
  - Compilar un tool que invoque el ioctl privado `WLAN_IOCTL_OID` con `OID_GET_DEBUG_LEVEL`.

### 3e. Volver a pmOS
```sh
adb reboot bootloader
fastboot flash boot ~/mainline/pkg/boot-btifDMA-sd.img
fastboot reboot
```

---

## Lo que ya sabemos (NO repetir)

| Hipótesis | Resultado | Detalle |
|---|---|---|
| enc_status=6 vs 7 | ❌ refutado | Ambos probados, DHCP falla igual. 7 es correcto (downstream). |
| Quitar POWER_SAVE + RX_FILTER del join | ❌ REGRESIÓN | El handshake ni completa (rx=0). Son necesarios en nuestro driver. |
| STATE_2 intermedio del STA record | ❌ refutado | `cnmStaRecChangeState(STATE_2)` no manda al FW (cnm_mem.c:994 — solo state interno del host). |
| RLM (0x1d) separado | ❌ no aplica | No se manda en el join. Campos idénticos al embebido. |
| Structs de comandos | ❌ idénticos | SET_BSS_INFO, CMD_802_11_KEY, UPDATE_STA_RECORD coinciden byte a byte. |
| Firmware distinto | ❌ idéntico | 7 blobs (WIFI_RAM_CODE + patches) mismo md5 stock vs LineageOS. |

---

## Infra / recetas

### PASO 0 — Build + flash del driver WiFi (en la Pi)
```sh
# subir driver editado (desde el Mac o donde trabajes):
scp mt6582-wifi.c mt6582-wifi-reg.h cpcd@192.168.0.123:/tmp/
# en la Pi:
cp /tmp/mt6582-wifi.c ~/mainline/linux-7.0.12/drivers/soc/mediatek/
cp /tmp/mt6582-wifi-reg.h ~/mainline/linux-7.0.12/drivers/soc/mediatek/
cd ~/mainline/linux-7.0.12
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage dtbs
# empaquetar:
cd ~/mainline/pkg
Z=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/zImage
D=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dtb
cat "$Z" "$D" > zimage-dtb
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk >/dev/null
abootimg --create boot-btifDMA-sd.img -f bootimg-sd.cfg -k zimage-dtb-mtk -r initrd-sd-mtk
# flashear DESDE pmOS (dd funciona; desde Android NO por verity):
scp boot-btifDMA-sd.img root@172.16.42.1:/tmp/b.img
ssh root@172.16.42.1 "dd if=/tmp/b.img of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync; sync; reboot -f"
# esperar ~60s al reinicio. Si sshd no responde → power-cycle físico.
```

### Dual-boot pmOS ↔ LineageOS
- **pmOS → LineageOS**: `fastboot flash boot lineage13-boot.img` (entrar en fastboot: `adb reboot bootloader` si estás en LineageOS, o power-cycle + Power+Vol↑).
- **LineageOS → pmOS**: `fastboot flash boot boot-btifDMA-sd.img` + `fastboot reboot`.
- **dd desde pmOS**: funciona (sin verity). **dd desde Android**: NO funciona.
- `adb reboot bootloader` entra en fastboot desde LineageOS (probado y confirmado).

### Test WPA2 en pmOS (sin crashear)
```sh
pkill wpa_supplicant; rfkill unblock wlan; ip link set wlan0 up
printf 'ctrl_interface=/var/run/wpa_supplicant\nnetwork={\n  ssid="cpcd"\n  psk="cpwebcd_2022"\n  key_mgmt=WPA-PSK\n  scan_ssid=1\n}\n' > /tmp/wpa.conf
wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -B
sleep 12
wpa_cli -i wlan0 status | grep wpa_state    # COMPLETED
udhcpc -i wlan0 -t 3 -T 2 -n                # falla sin OFFER
cat /sys/class/net/wlan0/statistics/rx_packets   # =2 (solo EAPOL)
```
**NO hacer `iw dev wlan0 scan` estando conectado** (crashea el móvil).
Redes: `hola`/`kakatua1`, `cpcd`/`cpwebcd_2022`.

### fwdump (leer RAM del FW en runtime)
```sh
echo "<addr_hex> <nwords>" > /sys/kernel/debug/mt6582_wifi/fwdump_cfg
cat /sys/kernel/debug/mt6582_wifi/fwdump
# ATENCIÓN: direcciones en 0x020axxxx (data RAM) PUEDEN COLGAR el bus → WDT reset.
# Solo probado seguro: 0xf0020000+ (código), 0xf0063000+ (rodata).
```

### RE del FW (Ghidra en el Mac)
- Ghidra 12.1.2 en `/opt/homebrew/Cellar/ghidra/12.1.2/libexec/`.
- Procesador: **NDS32:LE:32:default** (incluido en Ghidra).
- **JDK arm64 obligatorio**: `JAVA_HOME=/opt/homebrew/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home` (el JDK x86_64 de Oracle busca el decompile nativo equivocado).
- Dump del FW: `fw-analysis/fw_f0020000.bin` (código) + `fw-analysis/rodata.bin` (strings).
- Scripts: `fw-analysis/decompile_all_functions.java`, `map_modules2.py`.

---

## Archivos importantes

| Fichero | Qué es |
|---|---|
| `mainline/wifi-consys/wifi/HANDOFF-CASA-0630.md` | **ESTE DOCUMENTO** |
| `mainline/wifi-consys/wifi/HANDOFF-RE-LINEAGE-0630.md` | Handoff de la sesión anterior (más detalle de lo refutado) |
| `fw-analysis/` | FW descompilado + scripts + mapeo de módulos |
| `mainline/wifi-consys/wifi/mt6582-wifi.c` | Driver (con fwdump + todos los fixes de estabilidad) |
| `mainline/wifi-consys/wifi/mt6582-wifi-reg.h` | Registros + structs (+ CMD_ID_ACCESS_REG 0xc2) |

En la Pi:
| Fichero | Qué es |
|---|---|
| `~/wifi-fw/lineage_fsm_capture.log` | Captura FSM de LineageOS (connect exitoso) |
| `~/wifi-fw/extracted/` | Blobs FW originales (idénticos stock vs LineageOS) |
| `~/mainline/downstream/` | Source downstream mt6628/mt_wifi (referencia) |

---

## Reglas de oro (no cambiar)
1. `fastboot flash boot` NUNCA `fastboot boot`. mtkclient `wo` NUNCA `wf`. No flashear preloader.
2. NO restaurar LK Lollipop (bueno = KitKat 1.5.2).
3. `dd` desde pmOS funciona; desde Android NO (verity).
4. NO `iw dev wlan0 scan` estando conectado WPA2 en pmOS (crashea).
5. Si sshd no arranca tras reboot → power-cycle físico (Power ~10s).
6. `reboot -f` es más fiable que `reboot` en pmOS.
7. fwdump a 0x020axxxx PUEDE COLGAR el móvil → power-cycle.

*Co-autor: Claude (glm-5.2). Sesión del 30-jun-2024.*
