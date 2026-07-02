# HANDOFF — EAPOL por cmd-queue (SECURITY_FRAME) en el driver A (Mac, 2026-07-02)

## Qué es esto
Experimento del **transplante de stream nº1** (la divergencia de cifrado MÁS FUERTE hallada por el
análisis del port, NO incluida en las 8 hipótesis refutadas): el stock entrega el EAPOL del 4-way por
la **cola de comandos** (`COMMAND_TYPE_SECURITY_FRAME` → `wlanSendSecurityFrame` → `nicTxCmd` →
**puerto 1/TC4**), nuestro driver A lo mandaba por el **data-path** (puerto 0 + FLAG_1X). Mecanismo
plausible: el sec-FSM del FW solo dispara el setter de `[0x12f5]` (broadcast-encrypt) si ve el 4-way
por la ruta de seguridad; como data opaca no se entera → gates a 0 → DISCOVER sin cifrar → 0 OFFER.

## Implementado en `mt6582-wifi.c` (este commit)
- **`wifi_tx_eapol()`**: si `ethertype==0x888E` y hay `struct hif_tx_header` → sale por
  `wifi_port1_write_pio` (TC4/puerto 1) con descriptor **byte-igual** al del stock
  (`nicTxCmd` rama security-frame, nic_tx.c:1697-1719): TC4<<2, pkt_type=DATA(0), hdr_len=14,
  ether_type_offset=14, FLAG_1X (0x40), BURST_END, sta_rec=0, sin ACK.
- **v2**: + **dword-cero terminador** tras el frame (el stock lo mete en **CADA TX de AMBOS puertos**,
  `HAL_WRITE_TX_PORT` hal.h:307-309; nuestro `wifi_tx_data` ya lo hacía en el puerto 0, en el 1 faltaba)
  y el write ahora es `ALIGN(total,4)+4`. + prints **pre-write / OK** para clavar el punto de un cuelgue.
- `wifi_tx_data()` delega en `wifi_tx_eapol` para EAPOL; resto de datos sin cambios.

## Estado empírico (2026-07-02 mañana)
- **v1 (#222/#223)**: NM auto-conectó a `hola` al arrancar; el AP mandó el **M1** (visto en consola:
  `rx0 data l0=127 ... e6 92 ...` = 12B HIF + ~115B EAPOL) y **crash MUDO** (WDT) **antes** del print
  post-write del M2 → el cuelgue está en/alrededor del write del M2 al puerto 1 (o el FW murió
  procesándolo y el siguiente PIO-read colgó el AHB). El pstore de ese crash se PERDIÓ (el boot
  intermedio del WDT lo machacó) → por eso el guardián de abajo.
- **v2 (#225, terminador + prints): FLASHEADO pero SIN PROBAR** — en el boot del test wlan0 no
  apareció (bring-up flaky) y la sesión se cortó ahí.
- Misterio menor: un boot acabó con `reboot: Restarting system` **limpio** + re-read de particiones
  de mmcblk0, ~3 s tras arrancar wpa_supplicant (¿alguien flasheó/reinició a mano?). No era el WDT.

## Infra nueva en el MÓVIL (persistente)
- **`/etc/local.d/00-pstore-save.start`**: salva `/sys/fs/pstore/*` a `/var/log/pstore/boot-<fecha>/`
  en CADA arranque (rota, guarda 7). **Los crashes ya no se pierden** aunque el boot intermedio del
  WDT no levante sshd. Tras un crash: buscar el log en `/var/log/pstore/`.

## SIGUIENTE (para la sesión que lleve el WiFi)
1. Con **#225** (ya flasheado): bring-up (si wlan0 no está: `echo 1 > /sys/kernel/debug/mt6582_wifi/bringup`)
   → parar NetworkManager → wpa_supplicant a `hola` → mirar dmesg:
   - ¿`EAPOL-TX pre-write` sin `EAPOL-TX OK`? → el write del puerto 1 cuelga el AHB (HSTCR/WHIER/WTDR1).
   - ¿`pre-write` + `OK` y crash después? → el FW muere PROCESANDO el frame (estado del staRec/sec-fsm).
   - ¿`OK` + M3/M4 + COMPLETED? → **udhcpc** = el test del siglo.
2. Si crashea: leer `/var/log/pstore/boot-*/console-ramoops-0` (ya no se pierde).
3. Ojo NM: auto-conecta a `hola` al arrancar → para tests controlados, `rc-service networkmanager stop`.

*Sesión Mac, 2026-07-02. El driver A con esto queda como banco de pruebas del transplante de stream;
el port stock (mt_wifi_port/) sigue siendo la vía principal en paralelo.*
