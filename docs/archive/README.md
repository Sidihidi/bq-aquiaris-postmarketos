# Archivo histórico

Documentos **superados** pero conservados por su valor de contexto (razonamiento, registros,
causas raíz). No reflejan el estado actual — para eso ver el [README](../../README.md),
[HITOS.md](../../mainline/HITOS.md) y [PLAN-PORTS-DRIVERS.md](../../PLAN-PORTS-DRIVERS.md).

## `ROADMAP-FINAL.md`, `HANDOFF-CASA-0624.md`, `handoff-wifi-0624/`
Roadmap y handoffs previos al pivote de WiFi. El roadmap maestro vigente es
`PLAN-PORTS-DRIVERS.md`. `handoff-wifi-0624/` incluye una copia antigua del driver
(`mt6582-wifi.c`/`-reg.h`) — la viva está en `mainline/wifi-consys/wifi/`.

## `wifi-re-history/` — la saga de ingeniería inversa del FW WiFi (0624–0630)
Toda la investigación del **gate del cifrado WPA2** por RE del firmware nds32 (fwdump, gates
`[0x12e3]`/`[0x12f5]`, IRQ, KASAN, TX-encrypt, key-type, RLM…). **8 hipótesis refutadas** →
el **2026-07-02 se decidió portar el driver stock `mt_wifi`** en vez de seguir parcheando el
driver A. La conclusión que cierra esta etapa está en
`mainline/wifi-consys/wifi/HANDOFF-BLINDPOKE-REFUTADO-0702.md` (activo). Los 0701 (fwdump/gates)
se mantienen en el árbol activo porque el plan del port aún los referencia.

## `superseded-drafts/` — borradores de drivers reemplazados por mainline
Borradores propios (`mt6323-isink-led.c`, `mt6582-thermal.c`, `mt6582-vibrator.c`) de una sesión
temprana. **Reemplazados** por drivers ya en mainline (`leds-mt6323`, `mt6577_auxadc`,
`regulator-haptic`) que hacen lo mismo con solo DT+config — ver `PLAN-PORTS-DRIVERS.md`. Se conservan
por sus mapas de registros (ISINK, AUXADC) como referencia.

*Archivado 2026-07-02 al limpiar el repo. Nada borrado: git conserva todo el historial.*
