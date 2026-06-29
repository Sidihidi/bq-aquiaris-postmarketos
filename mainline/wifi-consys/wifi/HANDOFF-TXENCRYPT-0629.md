# HANDOFF — TX-ENCRYPT: bug CAZADO, el fix DISPARA pero el re-envío crashea (2026-06-29)

Sesión Mac larga. **Cazado el bug raíz del DHCP/WPA2** y acotado al milímetro. El fix DISPARA (confirmado en
HW por pstore), pero el mecanismo del re-envío crashea el FW → hay que activar el cifrado de otra forma.

## TL;DR
- 🎯 **El bug es el TX-ENCRYPT** (no el RX). Un **sniff** (Pi en cpcd + `tcpdump`) prueba que el AP **no ve ni
  nuestro DISCOVER ni un ARP** (sí ve el broadcast de OTROS clientes → no hay AP-isolation) → el AP recibe
  nuestra trama cifrada, no la descifra, la tira → nunca hay OFFER. **Es UN bug (TX), no dos.** El RX (el EAPOL
  sin cifrar SÍ llega: `RX0 DATA l0=149/183`) está bien.
- 🎯 **El mecanismo**: el FW activa el cifrado del TX con la **transición `enc_status: KEY_ABSENT → ENABLED`**
  tras instalar la PTK. El downstream MT6628 lo calcula con **`fgTransmitKeyExist`** (`wlanoidSetEncryptionStatus`,
  `common/wlan_oid.c:3247`): CCMP+clave → `ENCRYPTION3_ENABLED`, CCMP-sin-clave → `ENCRYPTION3_KEY_ABSENT`.
  Nosotros hardcodeábamos `ENABLED` UNA vez en el JoinComplete (ANTES de la clave) → el FW nunca recibía el
  "clave lista, cifra" → no cifraba el TX → el AP tiraba todo dato cifrado.
- ✅ **Fix implementado + DISPARA en HW**: (1) JoinComplete `enc_status=KEY_ABSENT`; (2) re-enviar
  `SET_BSS_INFO(ENABLED)` en `.add_key`. El pstore confirma el log `*** TX-ENCRYPT: re-SET_BSS_INFO(ENABLED) ***`
  tras instalar las claves, con connect OK (ASSOC-RESP CONNECTED, EAPOL, PTK, GTK todos ✅).
- ❌ **PERO el re-envío CRASHEA el FW (WDT), da igual el timing**: v1 (re-send tras PTK → GTK → crash) y v2
  (GTK → re-send → crash) crashean AMBOS **justo tras el re-envío** (el log del pstore corta exactamente en esa
  línea). **El FW peta al procesar un 2º `SET_BSS_INFO` a mitad de conexión.** → **el re-envío queda DESCARTADO.**

## Dato clave del downstream (la pista para el fix correcto)
`mtk_cfg80211_add_key` (`os/linux/gl_cfg80211.c:158`) **NO re-envía `SET_BSS_INFO`** — solo llama a
`wlanoidSetAddKey` (la clave). El `eEncStatus` se fija en el CONNECT (`wlanoidSetEncryptionStatus`). O sea:
el FW transiciona `KEY_ABSENT→ENABLED` **internamente al instalar la clave**, SIN un 2º `SET_BSS_INFO`.
Hay que replicar ESO, no re-enviar.

## SIGUIENTE (sin re-envío) — 2 vías
- **Vía A (segura, rápida)**: `KEY_ABSENT` *sin* el re-envío (quitar el bloque `if (!pairwise ...)` del add_key)
  → a ver si el FW auto-activa el cifrado al instalar la PTK (como el downstream). Sin re-send = sin crash.
  Confirmar con el sniff (¿aparece el DISCOVER del móvil en el aire?). (Se probó `KEY_ABSENT` solo en el pasado
  con resultado dudoso y driver viejo; re-testear con el banco de sniff lo zanja.)
- **Vía B (definitiva)**: bucear el `aisFsm`/`secFsm` del downstream — CUÁNDO manda su ÚNICO `SET_BSS_INFO`
  y con qué `enc_status` exacto en el flujo cfg80211 (connect → 4-way → add_key). Árbol downstream COMPLETO
  en la Pi: `~/mainline/downstream/drivers/misc/mediatek/combo/drv_wlan/mt6628/wlan/`.

## El fix actual en el driver (COMMITEADO con este handoff — el re-send hay que QUITARLO/REEMPLAZARLO)
- `mt6582-wifi.c` JoinComplete: `bi.enc_status = ENC_STATUS_CCMP_KEY_ABSENT;` (era `CCMP_ENABLED`). ← esto SÍ va.
- `mt6582-wifi.c` `.add_key`: `if (!pairwise && w->connect_wpa2 && !w->enc_enabled)` → re-send `saved_bi` con
  `enc_status=ENABLED`. **Este re-send CRASHEA el FW — quitar o reemplazar por el mecanismo correcto (vía A/B).**
- `mt6582-wifi-reg.h`: `ENC_STATUS_CCMP_KEY_ABSENT 7` (ya estaba).

## Banco de pruebas (montado, reproducible)
- **Sniff**: móvil en cpcd (CCMP) vía `wpa_supplicant` manual (`wpa_passphrase cpcd <clave> > /tmp/w.conf`);
  `tcpdump` en la Pi (`sudo apt install tcpdump` ya hecho). Filtro `"arp or port 67 or port 68"`. Si la Pi ve el
  DISCOVER/ARP del móvil (su MAC wlan0) → el TX cifrado sale → fix OK.
- **Crash log**: `/sys/fs/pstore/console-ramoops-0` (sobrevive al reboot, captura el connect+crash). `rm` para limpiar.
- **Build/flash**: editar driver → `bash ~/build-diag.sh` (compila+empaqueta `boot-diag.img`, incremental ~2min)
  → scp al móvil `/tmp/b.img` → `dd if=/tmp/b.img of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync; sync; reboot -f`.
- Backups del driver en la Pi: `.bak-pre-txencfix-0629` (pre-fix), `.bak-txencfix-v1` (v1, re-send tras PTK).

## Notas
- Redes hoy: hola y cpcd son **CCMP/CCMP** (el `GTK=TKIP` del logcat 06-25 era otra config; **TKIP DESCARTADO**).
- El connect en sí es FIABLE (grant-fix). El RX sin cifrar funciona. **Solo falta activar el cifrado del TX.**
- (Ortogonal, pendiente) Crash PIO-hang silencioso en `disconnect→settle-expiry` (sin oops, WDT).
- El móvil reinicia con cada crash del re-send → al iterar la vía A/B, el test en sí (sin re-send) no debería crashear.
