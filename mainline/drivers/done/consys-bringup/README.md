# CONSYS bring-up COMPLETO (mt6582-btif.c + mt6582-consys.c) — consolidado 0717

**Estado: ✅ FUNCIONANDO.** Bring-up completo del combo CONSYS (BT/GPS/FM/WiFi) del MT6582
con el firmware ROMv1 stock, RF-CAL incluida. Es la culminación del trabajo 0714–0717.
Imagen de diario: **boot-menupick14-consys.img** (md5 `8ded10b3...`, = kernel #14 + fix DSI
+ SPM + batería + todos los drivers previos; backups boot-GOOD actualizados en Pi y móvil,
los anteriores renombrados `-pre0717`).

## Secuencia de bring-up (bring_up_chip, mt6582-btif.c)

1. `mt6582_consys_hw_rst()` (MTCMOS+rails off→on, poll chipId) + `btif_hw_init`.
2. QUERY_STP (simple) → **SET_STP `DF 0E 68 01`** → flip a **STP FULL mode** + 10ms.
3. QUERY_STP2 (ya en full — valida el framing).
4. **⚡ 3× "power on DLM"** (SET_REG `0x80100060`, valor 0, máscaras 0xF00/0xF0/0x8):
   encienden la RAM del MCU. LA PIEZA que faltaba (el 3.4 de bq-src no la tiene; está
   en el 3.10 de LineageOS). Sin esto el patch se pierde y el fw salta a RAM vacía.
5. Multi-patch ROMv1: `patch_1_1` (22 frags) @ `00 00 0e f0` (=EMI 0xF00E0000) → RESET-1
   → `patch_1_0` (81 frags) @ `00 00 06 00` (=RAM interna 0x60000) → RESET-2.
   Addresses = bytes 24-27 del header de cada patch. Los RESET van EN SECUENCIA
   (el chip descarta frames fuera de orden en silencio).
6. Tras el EVT del RESET-2 (el fw nuevo emite inband reset `80 00 00 80` + EVT seq=0):
   **resincronizar el host a txseq=0** (el fw arranca con contadores frescos).
7. PALDO BT+WiFi on (VCN33 modo-HW) → **RF-CAL** (`01 14 01 00 01`) → PALDO off.
8. COEX ant_mode=1 → FM-COMM strap → func_on(BT) → GSYNC GPS (0x80050078[30:28]=1)
   → func_on(GPS).

## STP FULL mode (lo que exige el ROMv1)

- TX: header `0x80+(txseq<<3)+txack | (type<<4)|len_hi | len_lo | (h0+h1+h2)&0xff`
  + payload + CRC-16/ARC del payload (`crc16()` del kernel).
- ACK suelto de 4B tras cada frame de datos del chip; frames len=0 se consumen sin
  avanzar secuencia (así se traga también el inband reset).
- El chip es ESTRICTO: un frame fuera de secuencia se descarta sin respuesta.

## mt6582-consys.c

- `mt6582_consys_hw_rst()` (power-cycle completo), `mt6582_consys_bt_vcn33()` /
  `wifi_vcn33()` (PALDO modo-HW 0x416[5]/0x418[14]), EMI remap 1MB (0x10001310),
  OSC 26M (0x10001f00 bit10), **`mt6582_consys_cpupcr()`** (PC del MCU en 0x18070160,
  oro para diagnosticar el fw: sano = botando en el rango del patch; crash = barrido
  lineal a ~26B/µs).

## Verificado (0717, kernel #14)

- RF-CAL OK, COEX OK, FM-COMM OK, BT hci0 real (scan funciona), GPS radio+GSYNC OK,
  WiFi conecta a la primera (`wlan0 ARRIBA` + DHCP), FM /dev/fm presente, batería/
  sensores/audio sin regresión. PC del MCU estable en ~0x66312 (fw idle).
- GPS: tubería completa fluye; adquisición pendiente de test con cielo real (Lineage
  tampoco veía sats en la ventana de test — hipótesis "0xCA=ruido" invalidada).

## Historia y detalle del debugging

Ver `../../wip/gps/FINDINGS-STP-FULL-0717.md` (y CONTINUAR-GPS-RF-0715.md para el
contexto previo). Truco estrella: bootear LineageOS en vivo (flash `lineage13-boot.img`
desde pmOS + adb desde la Pi) y espiar el chip funcionando (`wmt_dbg` op 0xb = leer
memoria del chip por WMT; dmesg con `echo 9 4`/`a 4`; `echo 6 0` = sw_init completo).
Volver: `adb reboot bootloader` + `fastboot flash boot` (dd desde Android NO persiste).
