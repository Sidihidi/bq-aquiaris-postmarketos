# 🏆 HITO — WiFi WPA2 del BQ krillin FUNCIONA EN MAINLINE (2026-07-04)

Por primera vez, el BQ Aquaris E4.5 "krillin" (MediaTek MT6582) tiene **WiFi WPA2 + DHCP completo y
estable en el kernel mainline** (linux-7.0.12, postmarketOS). Vía: el **port del driver stock `mt_wifi`**.

## Qué funciona (todo validado en HW, kernel #235)
- **Arranca de serie**: el port es el driver WiFi por defecto (built-in, `CONFIG_MTK_MTWIFI=y`; driver A
  `mt6582-wifi.c` fuera). Al boot, `wlan0` sube sola (~14s, probe con bring-up diferida al WMT).
- **Conecta + DHCP**: `nmcli con up hola` → 4-way WPA2-CCMP → `lease` → `inet 10.181.211.x/24` → navega
  (ping GW 0% loss). El TX-encrypt del broadcast (el bug de fondo de toda la saga) resuelto por el
  command stream canónico del core stock (`enc_status=ENUM_ENCRYPTION3_ENABLED`).
- **Estable**: 3 ciclos connect/DHCP/disconnect sin un solo crash. El teardown que reseteaba al driver A
  (scan sobre el MAC activo → cuelgue del bus AHB) NO ocurre en el port.

## El camino (de "el DHCP no funciona" a navegar)
1. RE del FW (nds32, Ghidra): gates `[0x12e3]`/`[0x12f5]`, blind-pokes, FIX C — diagnóstico correcto pero
   el driver A monolítico no lograba emitir el command stream exacto.
2. Descubrimiento: el driver del krillin es `mt_wifi` (no `mt6628`); `enc_status=ENABLED` es correcto pero
   insuficiente en el driver A (le faltaban SECURITY_FRAME, flow-control TC4, clasificación RX).
3. **Vía ganadora — port del stock completo** (5 fases): KAL+shims+HIF PIO → probe/M1 (FW arranca) →
   gl_cfg80211/scan/M2 → TX+BSSID/4-way → **DHCP**. El core stock intacto emite el stream que el FW espera.

## Estado de los dos drivers
- **`mtk_mtwifi` (el port) = DRIVER OFICIAL.** Es con el que se continúa. `drivers/net/wireless/mtk_mtwifi/`
  en el árbol; versionado aquí en `mt_wifi_port/`.
- **`mt6582-wifi.c` (driver A) = FALLBACK deshabilitado.** Se conserva conmutable por Kconfig (descomentar
  `obj-y += mt6582-wifi.o` + `CONFIG_MTK_MTWIFI=n`) por si hiciera falta revertir. NO se desarrolla más.

## Connect por la GUI de Phosh/SXMO — RESUELTO (no era el driver)
Síntoma: la GUI escaneaba redes pero no conectaba (el `nmcli` CLI SÍ conectaba). Causa: la regla polkit
`50-org.freedesktop.NetworkManager.rules` autoriza el control de NM a `subject.isInGroup("netdev")`, pero
el usuario de la sesión (`sxmo`) **no estaba en `netdev`** (sí en wheel/audio/input/video...) → el connect
de la GUI se denegaba por polkit. **Fix: `addgroup sxmo netdev`** (aplica al próximo login/reboot de la
sesión). Además la conexión guardada se puso a **`connection.autoconnect yes` + permisos all-users**, así
el WiFi sube solo al boot sin depender del connect manual de la GUI. NADA de esto es del driver.

## Pendiente (no bloqueante)
- Estabilidad avanzada: roaming, suspend/resume, sesiones largas; TX-power/rate finos.

## Docs de la saga
`HANDOFF-MTWIFI-PORT-DHCP-CONSEGUIDO-0703.md` (DHCP), `MAKE-DEFAULT.md` (built-in), `STABILITY-0704.md`
(estabilidad), + los handoffs de Fases 1-5.

*El WiFi del krillin, de imposible en mainline a funcionando de serie. Sesiones Windows(.38)+Mac(.123).*
