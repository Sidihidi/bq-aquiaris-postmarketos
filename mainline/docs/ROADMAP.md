# Hoja de ruta — mainline Linux / postmarketOS en BQ Aquaris E4.5 "krillin" (MT6582)

_Actualizado: 2026-06-20_

Kernel mainline 7.0.12 (armv7) + Alpine 3.24 + Phosh, rootfs en SD; eMMC conserva Lineage-13 (para captura OEM).
Teléfono: `root@172.16.42.1` (USB) desde la Pi `cpcd@192.168.0.38`. Flash mainline = `dd` a `/dev/mmcblk0` **sector 83968**.

---

## 0. ESTADO ACTUAL — qué FUNCIONA

| Subsistema | Estado | Notas |
|---|---|---|
| **Display** | ✅ | DRM nativo `mtk_drm` (panel HX8389 qHD 540×960) + **Phosh acelerado por GPU lima** (Mali-400 MP2) |
| **Táctil** | ✅ | FT5336 (I2C0, EINT117) end-to-end en Phosh |
| **Almacenamiento** | ✅ | eMMC `msdc0` (mmcblk0) + SD `msdc1` (mmcblk1, rootfs) |
| **Red / consola** | ✅ | USB gadget `cdc_ether` + SSH (`172.16.42.1`) |
| **GPS** | 🟡 | Protocolo **capturado** (mnld `0xAAF0` sobre `/dev/stpgps` + NMEA); en mainline `/dev/stpgps`+bridge+gpsd+geoclue desplegados. **Falta el MOTOR.** |
| **Bluetooth** | 🟡 | `hci0` registrado, el controlador responde (`broken local ext features` = vivo). **Falta BlueZ + probar scan/pairing.** |
| **WiFi** | 🔴 | bring-up llega a `WIFI_START`, descarga FW OK; **el MAC no arranca** (`WLAN_READY`=0). VCN33 descartado. → §3 |

---

## 1. BLOQUEANTES TRANSVERSALES (máxima prioridad — ralentizan TODO)

1. **★ Boot inestable de la mainline.** A veces no completa (`usb0 carrier=0`, sshd no llega), necesita power-cycle con suerte. **Cada test depende de esto** → es el cuello de botella nº1.
   - Diagnóstico: capturar el boot fallido (ramoops/serie a 921600); ¿qué servicio/driver cuelga tras enumerar el gadget? ¿race en el bring-up del consys/btif al boot?
2. **Dual-boot LK SD↔eMMC** (plan **kexec** del workflow `wf_9305022b`). LK normal → kernel-chooser → menú → `kexec` a mainline-SD o Android-eMMC. **Sin tocar lk.bin.** Elimina el reflasheo+power-cycle de cada ciclo y conserva Lineage (captura). Guardar primero el boot stock.

## 2. QUICK WINS (≈1 ciclo de kernel cada uno)

- **Backlight** — fix del workflow: escribir SOLO `BLS_EN`(0x10000) + `PWM_DUTY`(10-bit, max 1023, @`0x1400A0A0`) + gamma LUT; **NUNCA `BLS_DEBUG`** (eso rompía los colores con `pwm-mtk-disp`). Desbloquea el brillo.
- **Power-button** — EINT (hacerlo ANTES que suspend; el EINT es wakeup).
- **Sensores / autorrotación** — `st_accel`(LSM330@0x1d), `st_gyro`(L3GD20@0x6b), mag(MMC3516X@0x30), ALS(TMD2772@0x39); drivers mainline existen, nodos hijos `&i2c0` en polling (sin IRQ). `iio-sensor-proxy` ya en Alpine. NO instanciar CM36283 ni ST480@0x0c (ausentes → I2C_ACKERR).
- **Batería → UPower** — gauge/PMIC MT6323.

## 3. WiFi — SUB-PROYECTO GRANDE (enfoque METÓDICO, no más tiros sueltos)

**Estado:** `func_on(WIFI)` OK → `WCIR=0x6582` → descarga FW (2 secc, ACKs) → `WIFI_START` (WHISR TX_DONE) → **`WLAN_READY`=0 + mbox `D2HRM0R`=0** (el MAC no responde).

**DESCARTADO en hardware:** firmware (`WIFI_RAM_CODE_MT6582` 160480B = el del OEM), patch (1432), chip-id, **VCN33 enable + modo-HW** (`ANALDO_CON17=0xd000`, bit14 confirmado). El path software del scaffold es byte-idéntico al OEM `wlanAdapterStart`.

**Hipótesis restantes (probar de UNA en una):**
1. **Los pasos de init que el scaffold SALTA** vs OEM `wlanAdapterStart`: `nicInitSystemService` (MGMT mem + STA_REC) → `nicTxInitialize` → `nicRxInitialize` → `nicDisableInterrupt` → **`nicTxInitResetResource`** (créditos TX TC0-5 desde lo que reporta el FW) → descarga → WIFI_START. El scaffold solo escribe WHIER. ← **sospechoso nº1 ahora.**
2. **Verificar la carga REAL del FW** en el chip (readback de lo descargado).
3. ¿El FW arranca pero **señala distinto** a `WCIR.WLAN_READY`? (revisar eventos/mbox tras WIFI_START.)

Driver: `mt6582-wifi.c` (HIF @0x180F0000). Mapa de registros HIF + protocolo en memoria `reference_mt6582_wifi_hif`.

## 4. GPS — cerrar el motor

bridge `/dev/stpgps→pty→gpsd→geoclue` desplegado. Falta el motor que hable **`0xAAF0`** (lo que hace `mnld`). Vías: (a) correr el `mnld` de Android (qemu/bionic — falló por `__system_properties_init`), (b) motor mínimo nativo desde la captura (`wifi-work/mnld-*.strace`), (c) ¿modo NMEA-directo del chip GNSS?

## 5. Bluetooth — terminar

`hci0` ya responde. → instalar **BlueZ** + probar `bluetoothctl scan`/pairing (RF real). `gnome-bluetooth` en Phosh ya compilado.

## 6. MÁS LEJANO

Cámara · Audio (ALSA/MTK) · Modem (CCCI → Halium o skip) · FM (libfmmt6628) · Suspend (s2idle).

## 7. DOCUMENTAR + GitHub (cuando WiFi + BT estén operativos)

Subir el hito a `Sidihidi/bq-aquiaris-postmarketos`: los 3 drivers (`mt6582-consys/btif/wifi`), el firmware, la **receta de flasheo** (sector 83968, sin `conv=`, verificar por readback de 27544 sectores), la **secuencia OEM** capturada (WiFi/GPS) y el **mapa de registros HIF**.

---

### Orden sugerido para "continuar eficiente"
1. **Estabilizar el boot** (§1.1) — sin esto cada test es una lotería.
2. **Backlight** (§2) — win rápido y visible mientras tanto.
3. **WiFi hipótesis nº1** (§3) — `nicTxInitResetResource` + los init que faltan.
4. En paralelo: **dual-boot** (§1.2) para dejar de reflashear.
5. GPS motor / BT BlueZ / sensores según apetezca.
