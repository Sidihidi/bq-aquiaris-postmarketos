# HANDOFF — Port de robustez STP (resync) sobre mt6582-btif (Mac, 2026-07-02)

## Hallazgo clave (cambia la estrategia del PLAN)
Nuestro `stp_send` (mt6582-btif.c:185) manda **checksum(byte3)=0 y CRC(trailer)=0x0000** hardcodeados,
y **BT FUNCIONA igual** (emparejado con S24). → **El FW por BTIF NO valida checksum ni CRC en RX.**

Implicaciones:
- **CRC/checksum en TX = irrelevante** y es el MAYOR riesgo de regresión (no tocar el TX).
- La única mejora de valor es el **RX**: hoy `stp_pop_frame` (btif.c:216) lee el header **a ciegas**
  (no comprueba el sync-bit de byte0) → un byte espurio desincroniza el parser PARA SIEMPRE.
- **Validar el CRC entrante para descartar = dudoso**: si el FW manda CRC=0 también, no sirve.
  Antes de validar, CAPTURAR en HW frames reales del FW y ver si el checksum/CRC vienen no-cero.

## Formato de frame STP (verificado vs stp_core.c del downstream)
```
byte0: 0x80 | (txseq&7)<<3 | (txack&7)     ; sync-bit=0x80. Nosotros: seq/ack=0 (ok, FW no los usa)
byte1: (nak&1)<<7 | (type&7)<<4 | lenhi4   ; type: 0=BT 1=FM 2=GPS 3=WIFI 4=WMT
byte2: lenlo8                              ; length = (byte1&0x0f)<<8 | byte2   (payload 0..2048)
byte3: checksum de byte0..2 (mod 256)      ; FW lo ignora -> nosotros 0
payload[length]
trailer: CRC16_lo, CRC16_hi               ; CRC-16/ARC (poly 0x8005 reflejado, init 0), FW lo ignora -> 0
```
- **Resync del stock** (stp_core.c:2137): el FW inserta **4×0x7f continuos** para forzar resync.
  Estados MTKSTP_RESYNC1..4 = contar 4 bytes 0x7f seguidos (cualquier no-0x7f reinicia el conteo).
- CRC (osal.c:288): `crc=0; for b: crc=(crc>>8)^crc16_table[(crc^b)&0xff]` sobre el PAYLOAD (no header).

## Parche propuesto (BAJO riesgo, listo para pegar en stp_pop_frame) — SIN TESTEAR aún
En el caso feliz (byte0=0x80, type<=4) el comportamiento es IDÉNTICO al actual; solo añade
recuperación donde hoy se rompería. Requiere un contador `u32 resync_drops` en `struct mt6582_btif`
(diagnóstico) y `#define MAX_STP_PAYLOAD 2048`.
```c
static int stp_pop_frame(struct mt6582_btif *b, u8 *out, u32 max, u8 *type)
{
	u32 len, frame, n;

	/* RESYNC: descartar bytes hasta un header STP plausible.
	 * (1) 0x7f = byte de resync del FW (stp_core.c:2137) -> descartar
	 * (2) byte0 debe tener sync-bit 0x80 y type (bits6-4) valido 0..4 */
	while (b->rxlen >= 4) {
		u8 h0 = b->rxbuf[0];
		u8 chan = (b->rxbuf[1] >> 4) & 0x07;
		u32 l = ((b->rxbuf[1] & 0x0f) << 8) | b->rxbuf[2];

		if (h0 != 0x7f && (h0 & 0x80) && chan <= 4 && l <= MAX_STP_PAYLOAD)
			break;				/* header plausible */
		b->resync_drops++;
		memmove(b->rxbuf, b->rxbuf + 1, --b->rxlen);	/* avanzar 1 byte */
	}
	if (b->rxlen < 4) return 0;

	*type = (b->rxbuf[1] >> 4) & 0x0f;
	len = ((b->rxbuf[1] & 0x0f) << 8) | b->rxbuf[2];
	frame = 4 + len + 2;
	if (b->rxlen < frame) return 0;			/* frame incompleto, esperar mas bytes */
	n = min_t(u32, len, max);
	memcpy(out, b->rxbuf + 4, n);
	memmove(b->rxbuf, b->rxbuf + frame, b->rxlen - frame);
	b->rxlen -= frame;
	return n;
}
```
Opcional fase 2 (cuando haya captura HW que confirme CRC no-cero del FW): validar el CRC-16/ARC del
payload y, si falla, tratar el frame como ruido (descartar byte0 y volver a resync) + `crc_errs++`.

## Plan de test (cuando el móvil esté libre — arriesga BT, hacerlo con red de seguridad)
1. Flashear con el parche. Emparejar/verificar BT normal (hci0, transfer) = SIN regresión (caso feliz).
2. Estrés: `l2ping` o transfer grande + `cat /sys/kernel/debug/.../resync_drops` (debe crecer solo si hay ruido).
3. Inyección: forzar un byte espurio (difícil sin hook) — o confiar en el uso real (GPS+BT simultáneo).
4. Si BT regresiona → revertir (es 1 función); el flag de módulo `stp_resync=0` puede envolverlo para A/B.

## Veredicto
NO portar stp_core.c entero (arrastra psm_core/btm_core ~1500 LOC + ACK/retransmisión que este FW
—que ignora hasta el CRC— no necesita). Basta el resync RX de arriba (~15 LOC). El CRC de validación
es fase 2 y depende de confirmar en HW que el FW lo manda. Prioridad MEDIA: BT ya funciona, esto es
robustez, no un bloqueo. Integrar+probar cuando el móvil no esté ocupado por la sesión WiFi.

*Sesión Mac, 2026-07-02. Datos verificados contra el downstream; código sin testear (no meto parser a
ciegas en el path de BT que funciona). Análisis asistido por subagente Explore (run a1b0a59434a06b054).*
