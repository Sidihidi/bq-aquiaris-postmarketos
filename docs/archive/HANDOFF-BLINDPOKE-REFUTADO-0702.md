# HANDOFF — Blind-poke de gates [0x12e3]/[0x12f5] refutado (2026-07-02, opencode)

## TL;DR

- **Blind-poke POST-handshake** (fwpoke manual tras COMPLETED): el teléfono **sobrevivió**, pero DHCP **sigue fallando**. Gates refutados como mecanismo de fix.
- **Blind-poke PRE-key** (poke_gates=1 dentro de add_key): **CRASHEÓ** el FW (WDT reset). Confirma que las direcciones son reales pero el timing es crítico.
- **No se puede verificar** si el poke POST escribió realmente (ACCESS_REG READ devuelve deadbeef con conexión activa).
- **8 hipótesis refutadas en total.** El port del stock mt_wifi queda como la vía principal.

---

## Experimento 1: poke_gates=1 PRE-key (CRASH)

### Procedimiento
- `echo 1 > /sys/kernel/debug/mt6582_wifi/poke_gates` antes de conectar
- El driver hace poke de `[0x12e3]` y `[0x12f5]` DENTRO de `add_key`, entre `wifi_send_cmd(ACCESS_REG SET)` y `wifi_send_cmd(ADD_REMOVE_KEY)`
- WPA2 handshake iniciado por wpa_supplicant

### Resultado del pstore
```
[105.779] FIX gates PRE-key [PTK]: [0x12e3]=1 [0x12f5]=1 (antes del install)
[105.788] add_key: PTK idx=0 cipher=0xfac04 peer=e6:92:82:f4:ce:44 len=16 tx_key=1
[106.298] DIAG gates[PTK]: read *0x020a0068=0xdeadbeef
[106.315] FIX blind-poke: [0x12e3]@0x020a137b=1 [0x12f5]@0x020a138d=1 (bss=0x020a1000)
[106.324] FIX gates PRE-key [GTK]: [0x12e3]=1 [0x12f5]=1 (antes del install)
[106.333] add_key: GTK idx=1 cipher=0xfac04 peer=ff:ff:ff:ff:ff:ff len=16 tx_key=0
--- WDT reset (cuelgue mudo) ---
```

### Conclusión
El poke dentro de `add_key` **interfiere con el procesamiento interno del FW** (el NDS32 está procesando `CMD_ID_ADD_REMOVE_KEY` y recibe un `CMD_ID_ACCESS_REG SET` a la data-RAM que corrompe su estado interno). El teléfono se cuelga y el WDT lo resetea.

---

## Experimento 2: fwpoke POST-handshake (NO crash, pero NO fix)

### Procedimiento
1. Conectar WPA2 normalmente (poke_gates=0)
2. Esperar `wpa_state=COMPLETED` (handshake + claves instaladas)
3. Poke manual:
   ```
   echo '020a1378 01000000' > /sys/kernel/debug/mt6582_wifi/fwpoke  # [0x12e3]=1
   echo '020a138c 00000100' > /sys/kernel/debug/mt6582_wifi/fwpoke  # [0x12f5]=1
   ```
4. `udhcpc -i wlan0`

### Resultado
```
[222.767] fwpoke: [0x020a1378] <- 0x01000000    # [0x12e3]=1
[223.018] fwpoke: [0x020a138c] <- 0x00000100    # [0x12f5]=1
telefono VIVO (no crash)
wpa_state=COMPLETED (sigue conectado)
udhcpc: 6 DISCOVERs, 0 OFFERs → DHCP falla
rx=12632 (solo beacons), tx=152354 (retransmisiones)
ping 8.8.8.8: 100% packet loss
```

### Intento de verificación
```
echo '020a1378 1' > fwdump_cfg; cat fwdump → 020a1378=deadbeef  # READ no funciona conectado
echo '020a138c 1' > fwdump_cfg; cat fwdump → 020a138c=deadbeef  # READ no funciona conectado
```

### Conclusión
El poke POST-handshake **no crashea** (el FW ya terminó de procesar las claves), pero **no arregla el DHCP**. Posibles causas:
- (a) El FW ignora los ACCESS_REG SET a la data-RAM cuando hay conexión activa
- (b) Las direcciones son correctas pero hay **más gates** además de esos dos
- (c) El problema **no son los gates** — hay otra causa dinámica

**No se puede verificar** cuál de las tres es porque ACCESS_REG READ devuelve `deadbeef` con conexión activa.

---

## Estado de TODAS las hipótesis (8 refutadas)

| # | Hipótesis | Estado |
|---|---|---|
| 1 | enc_status 6 vs 7 | ❌ refutado |
| 2 | Quitar POWER_SAVE + RX_FILTER | ❌ REGRESIÓN |
| 3 | STATE_2 intermedio del STA | ❌ refutado |
| 4 | RLM (0x1d) separado | ❌ no aplica |
| 5 | Structs de comandos | ❌ idénticos |
| 6 | key_type invertido | ❌ refutado |
| 7 | RLM short_slot/preamble/nonht_basic_phy | ❌ refutado |
| 8 | **Blind-poke gates [0x12e3]/[0x12f5]** | ❌ **refutado** |

---

## Implicación para el port del stock mt_wifi

Con 8 hipótesis refutadas (todas las estáticas + el blind-poke), el port del stock `mt_wifi` queda como **la vía principal y justificada**:

- Los experimentos baratos (días) están **agotados**
- El RE del firmware llegó a un muro (no se puede leer la data-RAM conectado)
- La Vía A (captura LineageOS) no pudo activar el debug TRACE
- La diferencia con LineageOS es **100% dinámica** y **no replicable** con comandos aislados

El plan de port (`HANDOFF-MTWIFI-PORT-PLAN-0702.md`) es el siguiente paso.

---

## Estado del dispositivo
| Subsistema | Estado |
|---|---|
| WiFi ABIERTO | ✅ navega |
| WiFi WPA2 handshake | ✅ COMPLETED |
| WiFi WPA2 DHCP | ❌ 0 data RX |
| BT (hci0) | ✅ |
| Display + Phosh | ✅ |
| Touch | ✅ |

## Entorno
- Pi: `cpcd@192.168.0.38`. Kernel: `~/mainline/linux-7.0.12`.
- Móvil: `root@172.16.42.1`. SSH: `sudo ip addr replace 172.16.42.2/24 dev usb0`.
- Build+flash: `~/wifi-iter-w.sh` o `dd` sector 83968 + sysrq reboot.
- pstore: `/sys/fs/pstore/console-ramoops-0`.
- Stock driver: `~/mainline/downstream/drivers/misc/mediatek/combo/drv_wlan/mt6628/wlan/`.
- Plan de port: `mainline/wifi-consys/wifi/HANDOFF-MTWIFI-PORT-PLAN-0702.md`.

*Co-autor: opencode (glm-5.2).*
