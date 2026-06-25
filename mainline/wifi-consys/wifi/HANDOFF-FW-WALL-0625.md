# HANDOFF — WiFi WPA2: el muro es FW-INTERNO (2026-06-25, sesión Mac)

Continuación de `HANDOFF-DHCP-0625.md`. Esta sesión cerró el diagnóstico del data-path WPA2 con un **ramoops** y **3 agentes** comparando byte a byte contra el driver original. **Conclusión: el bug NO está en el código del host — es interno del firmware.** Aquí el hallazgo + la estrategia para casa.

## TL;DR
- WPA2 asocia (PTK+GTK OK) pero **el FW deja de entregar RX cifrado tras instalar la GTK** → la OFFER del DHCP (broadcast) nunca sube al HIF → `rx=2` (solo los 2 EAPOL).
- **Los comandos host→FW COINCIDEN byte a byte con el driver original** (conn_soc/mt_wifi, verificado por 3 agentes). No falta ni sobra ningún comando del host.
- **3 fixes probados en HW, ninguno arregló** → es FW-interno.
- **Código REVERTIDO a #171** (estado bueno: OPEN navega, WPA2 roto-pero-estable). Sin regresión.

## El diagnóstico (evidencia: ramoops)
Secuencia capturada de una conexión WPA2 real:
```
JoinComplete (SET_BSS_INFO enc=6 + STA->STATE_3 + PS=CAM) -> CONNECTED
  RX0 DATA l0=149   <- EAPOL msg1  } los 2 unicos
  RX0 DATA l0=183   <- EAPOL msg3  } rx=2
  add_key PTK idx=0 CCMP peer=AP            tx_key=1
  add_key GTK idx=1 CCMP peer=ff:ff:ff:ff:ff:ff tx_key=0
  [ ... CERO RX0 DATA mas (ni OFFER del DHCP ni beacons) ... ]
```
El log `RX0 DATA` (`mt6582-wifi.c:831`) imprime CADA trama de datos que el FW sube al HIF. Solo salen 2 (los EAPOL, pre-clave); tras la GTK, cero. **El driver entregaria la OFFER si llegara — pero el FW no la sube.** TX funciona (DISCOVER salen). OPEN navega (sin cifrado, el RX de datos va).

## Descartado (NO repetir)
3 agentes compararon byte a byte vs `/tmp/ds-connsoc` (= conn_soc/mt_wifi, chip-id 6582 confirmado, el driver del MT6582 integrado). Todo COINCIDE: `enc_status=6`, el orden del JoinComplete, los structs (`CMD_802_11_KEY` 64B, `SET_BSS_INFO` 80B, `UPDATE_STA_RECORD`), la GTK (idx=1, CCMP, peer=broadcast, tx_key=0). Probado en HW SIN efecto:
1. Quitar `CMD_ID_DEFAULT_KEY_ID` (wpa_supplicant nunca llama a set_default_key) + `CMD_ID_SET_RX_FILTER` del join (#173) → `rx=2`.
2. Quitar el `aucKeyRsc` de la GTK (igualar al downstream que lo deja a 0; #174) → `rx=2`.

No hay "comando de abrir puerto controlado": el host (incluido el original) **no manda nada tras las claves** — el FW abre el RX solo desde la GTK. `enc_status` 6-vs-7 ya refutado en HW. → **En NUESTRO blob, el FW no arma el slot de RX de la group-key (WTBL) desde `CMD_802_11_KEY`.** Invisible desde el source del host.

## Recuperación / estado del código
- **Driver bueno (#171) a salvo**: git HEAD (md5 `51e9a4b`), revertido. Backups en Pi `.123`: `mt6582-wifi.c.bak-pre-gate-0624` (#162), `.bak-CURRENT-pre-step0` (#171), + cadena de biseccion completa.
- **Sin progreso perdido.** OPEN funciona con #171.

## ⭐ ESTRATEGIA PARA CASA — cazar la diferencia del FW

### A) Captura en LineageOS (LA MÁS FUERTE)
En LineageOS/Android la WPA2+DHCP **funciona**. Capturar la secuencia REAL del driver original en una conexión que SÍ va, y diffear contra la nuestra:
1. Arranca el Android interno (LineageOS).
2. Activa el debug del driver `mt_wifi`: mira `/proc/driver/wmt_*`, `/sys/module/wlan*/parameters/`, o un `dbglevel`/`DriverDbgLevel`; si no, `logcat -s WLAN -s wpa_supplicant` + `dmesg`.
3. Conecta a la WPA2 y deja que coja IP.
4. Captura: la secuencia de comandos al FW (sobre todo el `CMD_802_11_KEY` de la GTK y **lo que venga DESPUÉS**) + sus bytes.
5. Diff contra nuestra secuencia (el ramoops de arriba). Lo que el original mande y nosotros no = el fix.
   - **ALT (si el driver no loguea comandos): sniffer** — otro equipo en monitor mode durante el connect LineageOS → confirma que la OFFER vuela y que el móvil la ACKea (= el FW la recibe).

### B) Comparar el blob de FW
Nuestro FW = `0x0418` (del NIC_CAP, visto en dmesg). Extrae el blob que carga LineageOS/el original (`/etc/firmware/WIFI_RAM_CODE*`, `/vendor/firmware/`, etc.) y compara versión/bytes con el nuestro (en el árbol del driver). Si difiere, el comportamiento del WTBL/group-key puede estar ahí → probar a flashear el blob del original. **El `.ko` ya lo tenemos como source (es lo comparado); lo que falta cotejar es el FIRMWARE binario.**

### C) Sniffer sobre NUESTRO connect (parte el frente)
Otro equipo en monitor mode durante el connect de pmOS: ¿el AP manda la OFFER? ¿el móvil la ACKea a nivel 802.11 (= el FW la recibe pero la descarta por crypto) o ni la recibe? Decide entre "FW la tira" vs "no llega".

## Infra (recordatorio)
- Pi de build = **`.123`** (cpcd@192.168.0.123). Árbol `~/mainline/linux-7.0.12`. Build WiFi: `bash ~/wifi-iter-w.sh`. Downstream en `~/mainline/downstream/.../{mt6628,conn_soc/mt_wifi}/wlan`.
- Móvil: `root@172.16.42.1` vía usb0 desde la Pi (`sudo ip addr replace 172.16.42.2/24 dev usb0`; **re-hacerlo tras CADA reboot** — el usb0 se re-enumera, si no el poll se atasca).
- **`reboot` normal NO rearranca con Phosh arriba → usar `reboot -f`.**
- **Bug B (reconnect/disconnect-crash) SIGUE**: matar wpa_supplicant / 2ª conexión del mismo boot → el teardown cuelga el FW → WDT. Probar SIEMPRE en 1ª conexión de boot fresco.
- Receta DHCP manual fiable: ver `HANDOFF-DHCP-0625.md`.

## Verificación del revert (#171 intacto, sin regresión)
Tras revertir, el driver #171 **escanea y ve 8+ redes** (cpcd, eduroam, Open-UPCT, AvIoT...) con buena señal → plenamente funcional. El test "OPEN" no asoció solo porque el AP `cpcd` **seguía en WPA2** (`ec:08:6b:f3:e4:34 [WPA2-PSK-CCMP][WPS][ESS] cpcd` en el scan), NO abierto — el cambio del router no se aplicó. **No es regresión.** #171 es byte-exacto al estado bueno conocido (md5 `51e9a4b`); OPEN intacto. Para confirmar OPEN-DHCP en casa: poner un AP realmente abierto y `key_mgmt=NONE`.
