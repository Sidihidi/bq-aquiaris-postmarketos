# HANDOFF — fwdump ARREGLADO (lee la RAM del FW) + gates localizados; el read CONECTADO crashea (Mac, 2026-07-01)

## TL;DR
- ✅ **`fwdump` FUNCIONA** (commits `c4e50d8` + `77e5354`). Verificado: `f0063590` → ASCII **"wifi/mgmt/privacy.c"**
  (`69666977 6d676d2f 72702f74 63617669`), `f0020000` da valores distintos por dirección. Eran **2 bugs**:
  1. leía RX0/**puerto 0** con `resp_reserve=0` → el FW no reservaba respuesta + puerto equivocado → `deadbeef`.
     Fix: `wifi_send_cmd(..., resp_reserve)` + `wifi_poll_event(EVENT_ID_ACCESS_REG, ..., puerto 1)` (patrón de
     `wifi_phase1_hello`).
  2. el body tenía prefijo `{u8 sq; u8 rsv[3]}` → el FW parsea `CMD_ACCESS_REG={u4Address, u4Data}` con el
     address en offset 0, así que leía `{sq,rsv}`=0 como address → leía `0x0` → constante `0x6627` para TODO.
     Fix: `body = {__le32 address; __le32 data}` (verificado vs downstream `nic_cmd_event.c:356` +
     `wlanoidQueryMcrRead`).
- ✅ **`fwpoke`** (write, `set_query=1`) también en el driver (mismo fix de body). Sirve para diagnóstico, NO
  como fix fiable (no re-programa el bit tx-enc de la WTBL).

## Mecanismo del GATE (síntesis del workflow de RE, confianza media)
2 flags de una struct runtime del FW, **ambos deben valer 1** para el DHCP:
- **`[0x12e3]`** (puerto/unicast + fallback broadcast) — lo pone el `enc_status` del SET_BSS_INFO (memcpy de
  bloque en `f002222c`). `KEY_ABSENT` → 0. También `f004bb2c` pone el bit tx-enc de la WTBL SOLO si
  `CCMP && [0xf7c]==0 && [0x12e3]!=0`. **Dirección: `0x020a137b`** (= `*0x020a0064`(=`0x020a0098`) + `0x12e3`
  = BSS `0x020a1000` + `0x37b`; byte 3 de la palabra `0x020a1378`).
- **`[0x12f5]`** (grupo/broadcast) — setter `f0034610(1,0)`, invocado por **computed-call** en el sec-FSM tras
  la GTK (Ghidra no resolvió el destino). **Dirección: `0x020a138d`** (BSS+`0x38d`; byte 1 de `0x020a138c`).
- Decisor del broadcast = **`f0034a98`**: exige `[0x12f5]!=0` **Y** `[0x12e3]!=0`. El DHCP DISCOVER es broadcast
  → sin ambos, sale sin cifrar → el AP lo tira → 0 OFFER. **Ese es el bug.**
- **FIX A REFUTADO** (source-check): nuestra `CMD_802_11_KEY` de la GTK YA coincide con el downstream
  (`key_type=0`=group, igual que `wlanoidSetAddKey`). El bug NO es la codificación del comando.

## Estado empírico
- **Baseline IDLE** (leído con fwdump, SIN colgar): `[0x12e3]@0x137b = 0`, `[0x12f5]@0x138d = 0`. Punteros:
  `0x020a0068`→`0x020a1000` (BSS array, ya resuelto en idle); `0x020a0098`→`0` (idle, se resuelve al conectar);
  `0x020a0064`→`0x020a0098`.
- **⚠️ El read CONECTADO crashea el móvil (reboot).** Conectar WPA2 a "hola" + leer `0x020axxxx` con la conexión
  activa → reset (perdí la captura de los gates). Pasa incluso con la versión puerto-1.

## SIGUIENTE — leer los gates CONECTADO (DIAGNÓSTICO del crash confirmado, 2026-07-01 PM)
El read conectado dio: **`0x020a0098 = deadbeef`** (el fwdump TIMEOUT con la conexión activa — en IDLE iba
perfecto) y luego el móvil crasheó al leer la región del gate. **CAUSA:** con la conexión activa el puerto 1
tiene OTROS eventos (del connect/tráfico), y `wifi_poll_event` devuelve `-EBADMSG` al PRIMER evento que no sea
`EVENT_ID_ACCESS_REG` (NO lo salta — líneas ~503-506) → deadbeef. Las lecturas repetidas fallando → HIF/FW en
mal estado → reset.
**FIX (pequeño):** que `wifi_runtime_reg_read` use un poll ROBUSTO que **SALTE** los eventos no-`ACCESS_REG`
(bucle propio con contador de loops, NO `wifi_poll_event` que phase1 necesita estricto), y leer **1 palabra
sola** (no 12) para minimizar el riesgo. Con eso el read conectado debería ir → confirmar `[0x12e3]`@0x137b y
`[0x12f5]`@0x138d = 0 tras el 4-way → aplicar el fix (FIX C = NOP del check en f004bb2c, o el trigger de
[0x12f5]).
- (alternativa si aún crashea: volcar los gates a un fichero/dmesg que sobreviva al crash y leer del pstore.)

## RESULTADO EMPÍRICO del skip-poll (Windows/Opus, build #217, 2026-07-01 tarde) — el skip-poll NO basta
Implementé EXACTAMENTE ese fix: **`wifi_poll_event_skip`** (bucle propio que SALTA los eventos no-ACCESS_REG,
consumiendo el paquete completo, dentro del MISMO `hif_lock`; `wifi_runtime_reg_read` lo usa; `wifi_poll_event`
intacto para phase1) + instrumentación **`wifi_diag_gates`** en `add_key` (lee los gates al 4-way, log síncrono
a console-ramoops, pocas lecturas) + debugfs **`poke_gates`**. Commit `b1e2f2b`, build **#217** flasheado y
verificado (`poke_gates` + `wifi_poll_event_skip` en System.map).
**RESULTADO EN HW: sigue dando `0xdeadbeef`, pero ahora por TIMEOUT** (el skip-poll recorre TODO el puerto 1 y
NUNCA encuentra `EVENT_ID_ACCESS_REG`). → **NO es orden de eventos: el FW NO responde a ACCESS_REG sobre la
data RAM `0x020axxxx` con la conexión ACTIVA** (idle sí responde). La conexión **completa** y `add_key` dispara
`wifi_diag_gates` **SIN colgar** (pocas lecturas al 4-way evitan vuestro crash), pero la respuesta no llega.
**→ El skip-poll queda DESCARTADO como suficiente. No lo reimplementéis.**
### Siguientes para leer/pokear los gates conectado:
1. **Blind-poke (lo más directo para PROBAR el fix sin leer):** `wifi_runtime_reg_write` (SET) NO espera
   respuesta → puede que SÍ funcione conectado. Pokear los gates a 1 con `bss` HARDCODED `0x020a1000`
   (idle-confirmado): word `0x020a1378` byte3=1 (`[0x12e3]`) y `0x020a138c` byte1=1 (`[0x12f5]`). OJO: el write
   ciego clobbea los otros 3 bytes del word (no los conocemos) → riesgo, pero prueba el fix directo (¿cae el DHCP?).
2. **Pausar el `rx_thread`/IRQ durante el read** (disable_irq + parar el drain) para dejar el puerto 1 quieto y
   que la respuesta ACCESS_REG llegue. Más limpio, más código.
3. **Timeout mayor** (500→3000ms) por si el FW responde lento en data-mode (poco probable: es timeout total, no lento).

## Una vez confirmados los gates a 0
- **FIX C (más fiable):** patch binario del FW = NOP del check `[0x12e3]!=0` en `f004bb2c` (bit tx-enc
  incondicional en CCMP). No toca beacon/TSF → no cuelga.
- O resolver el trigger de `[0x12f5]` (el handler por computed-call del sec-FSM).

## Recetas
- fwdump/fwpoke: `echo "<addr_hex> <nwords>" > /sys/kernel/debug/mt6582_wifi/fwdump_cfg; cat .../fwdump`.
  `echo "<addr_hex> <val_hex>" > .../fwpoke`.
- Build (Pi .123): `make O=build-krillin ... -j2` (**NO -j4**, reinicia la Pi por pico de CPU) →
  `bash ~/wifi-iter-w.sh` empaqueta + flashea (magic+md5) + reboot. Móvil via Pi `ssh root@172.16.42.1`
  (sshd flaky → power-cycle a mano; `sudo ip addr replace 172.16.42.2/24 dev usb0` en la Pi).
- Síntesis completa del workflow de RE: output del run `wqhw4t4hm` (6 subagentes; 2 fallaron, síntesis buena).

*Sesión Mac (Opus 4.8), 2026-07-01. fwdump por fin operativo; falta SOLO el read conectado seguro para cerrar
la Fase 0 (confirmar gates=0), luego el fix (FIX C o el trigger de [0x12f5]).*
