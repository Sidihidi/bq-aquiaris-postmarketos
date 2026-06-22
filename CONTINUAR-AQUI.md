# 🔌 SEGUIR POR AQUÍ — sesión 2026-06-22 (retomar en casa)

> Gran sesión. Resumen + estado + próximos pasos. Repo: **Sidihidi/bq-aquiaris-postmarketos**.
> Pi `cpcd@192.168.0.123` (o `.38`), teléfono `root@172.16.42.1` (USB). Boot = `dd` a sector **83968**.

---

## ✅ Lo conseguido esta sesión

### 1. Recuperación del teléfono
El boot se lió (un señuelo de "openssh roto" + confusión con los offsets de la partición boot).
**Fix**: `fastboot flash boot boot-btifDMA-sd.img` → pmOS-SD estable.
**Lecciones**: `dd` a 83968 SOLO funciona desde **pmOS viva** (no desde el initramfs de emergencia); **fastboot siempre fiable**; `boot-color1.img` = pmOS de la INTERNA (no usar, cae a emergencia).

### 2. ⭐ BOOT FIX DEFINITIVO (el gran avance)
**Causa raíz de los arranques flaky, por fin entendida** — dos cosas:
- **Carrera**: el bring-up del CONSYS lo disparan 3 sitios SIN serializar — `zz-consys-bt` (debugfs), `zzz-gps` (abre `/dev/stpgps`) y el WiFi (`func_on`) → doble bring-up concurrente → el CONSYS falla.
- **Atasco**: el `echo > bringup` **síncrono** se cuelga en estado **D (ininterrumpible)** si el bring-up falla, y `timeout` **NO puede matar un proceso en D** → `local.d` se bloquea para siempre → sshd/Phosh nunca arrancan = boot atascado.

**Fix (commit `4336263`, pusheado)**:
- **Mutex `bringup_lock`** en `mt6582-btif.c` (`bringup`→`__bringup` + envoltura que serializa) → bring-up **ÚNICO**.
- `zz-consys-bt.start` → **ASYNC** `( ... ) &`, esperando a `phoc` (Phosh) primero (sin contienda de I/O de SD).
- **Verificado**: sshd ~29s, hci0 fiable, Phosh OK, bring-up 1 vez, **NUNCA se atasca**.

### 3. WiFi `.connect` (Fase 2) — coreografía completa implementada
- Añadido el **`UPDATE_STA_RECORD`** (registro del AP) que faltaba + fix del bug **`CMD_ID 0x18`→`0x1d`** (0x18 era REMOVE_STA_RECORD) + RLM embebido en SET_BSS_INFO + **`CH_PRIVILEGE`** (conceder el canal antes del TX).
- Coreografía: `CH_PRIVILEGE → BSS_ACTIVATE → UPDATE_STA_RECORD → SET_BSS_INFO(+canal)`.
- Valores exactos del downstream mt6628 (`STA_TYPE_LEGACY_AP=0x41`, `STA_STATE_3=2`, `PHY_BG=0x03`, rates `0xfff/0x0f`).
- **Verificado**: `wpa_supplicant` ahora ARRANCA `wlan0` (antes se negaba: "Driver does not support connect"); el `.connect` se ejecuta con el **BSSID real** del scan (`*** .connect: SSID='hola' BSSID=fa:6a:24:00:df:aa (OPEN+CHPRIV+STA-rec) ***`).
- **FALTA confirmar que ASOCIA** — bloqueado por el scan flaky (ver abajo). Commit `bcbdb01` (.connect) + el CH_PRIVILEGE (en el último commit de este wrap).

---

## 📍 Estado del teléfono (al cerrar)
- pmOS Alpine 3.24 en la SD (`mmcblk1p1`); boot en sector 83968 = `boot-btifDMA-sd.img` con **mutex (boot fix) + .connect completo (UPDATE_STA_RECORD + CH_PRIVILEGE)**.
- `/etc/local.d/zz-consys-bt.start` = ASYNC (no se atasca).
- BT (hci0) / GPS / WiFi-MAC (WLAN_READY) suben tras el bring-up.

---

## ▶️ Próximos pasos (prioridad)

1. **WiFi connect — verificar/afinar** (el CH_PRIVILEGE ya está flasheado):
   - **Bloqueo nº1 = scan flaky**: el RX devuelve **0 beacons en muchos boots** (en boots buenos encuentra ~16 redes, incl. "hola"). Para probar el connect: `iw scan` hasta cachear "hola" **y `iw connect` en el MISMO comando** (la caché cfg80211 expira ~30s).
   - Si con CH_PRIVILEGE NO asocia: esperar el `EVENT_ID_CH_PRIVILEGE` (0x18, grant) ANTES del assoc; probar `CMD_ID_INFRASTRUCTURE` (0x0a) primero; revisar el orden en `mgmt/ais_fsm.c` del downstream.
2. **Scan flakiness** (lo que bloquea el connect): investigar el RX a 0 beacons. Pistas: `cfg80211: failed to load regulatory.db (-2)` (country 00 DFS-UNSET → ¿canales restringidos / solo pasivo?); el `.scan` es PASIVO (probar activo); ¿RF-cal del MAC flaky por boot? Meter el `regulatory.db` en el initramfs o forzar `iw reg set ES`.
3. **WiFi Fase 3 — data-path** (navegar): `ndo_start_xmit` + RX data + PDMA + IRQ del HIFSYS; luego WPA2 (`.add_key` + EAPOL).
4. **GPS 100%**: recaptura del `t=0x05` (RUN) — Lineage(sec 83968) + GPSLogger + `strace -p $(pidof mnld) -s 1024` → pegar en `mtkgps_aaf0.c`.
5. **Phosh 100%**: brillo (slider), power button, toggles WiFi/BT/GPS, sensores + autorrotación (LSM330).

---

## 🔧 Recetas para retomar
- **Reconectar**: `sudo ip addr replace 172.16.42.2/24 dev usb0; ssh root@172.16.42.1`.
- **Build+flash kernel** (desde la Pi): editar `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-{btif,wifi}.c`, `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage`, en `~/mainline/pkg`: `cat zImage dtb > zimage-dtb; python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk; abootimg --create boot-btifDMA-sd.img -f bootimg-sd.cfg -k zimage-dtb-mtk -r initrd-sd-mtk`, y `dd` a sector 83968 **desde la pmOS viva** + reboot.
- **Bring-up CONSYS+WiFi**: `echo 1 > /sys/kernel/debug/mt6582_wifi/bringup` (el mutex serializa, sin carrera).
- ⚠️ **El repo (Mac) y la fuente de build (Pi) DIVERGEN** en algunos ficheros — sincronizar con `scp` + comparar md5 antes de editar.
- Downstream de referencia (en la Pi): `~/mainline/downstream/drivers/misc/mediatek/combo/drv_wlan/mt6628/wlan/include/nic_cmd_event.h`.

---

## 📂 Ficheros tocados (`mainline/wifi-consys/`)
| Fichero | Qué |
|---|---|
| `m3a/mt6582-btif.c` | **mutex `bringup_lock`** (boot fix: serializa el bring-up) |
| `wifi/mt6582-wifi.c` | `.connect` con `UPDATE_STA_RECORD` + `CH_PRIVILEGE` |
| `wifi/mt6582-wifi-reg.h` | structs/CMD_IDs (CH_PRIVILEGE, STA_RECORD, fix 0x18→0x1d) |
| `rootfs/bluetooth/zz-consys-bt.start` | bring-up **ASYNC** (no atasca el boot) |
