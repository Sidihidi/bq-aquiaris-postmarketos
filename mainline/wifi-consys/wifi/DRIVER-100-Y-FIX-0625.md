# MISIÓN 2 — Driver al 100% + el FIX (2026-06-25, sesión autónoma)

Encargo: subir el entendimiento del driver original del 75% al 100% (el 25% inexplorado).
Resultado: **el 25% que faltaba era el data-path de control RX**, y ahí está el fix de los 2 bugs.
Verificado byte-a-byte contra el downstream (`conn_soc/mt_wifi` = mismo chip 6582; ref. `mt6628`) y
contra nuestro `mt6582-wifi-PI.c` / `mt6582-wifi-reg.h` línea a línea por 4 agentes + síntesis.

> **Corrige 2 conclusiones previas erróneas:**
> 1. `HANDOFF-FW-WALL-0625.md` dijo "bug FW-interno, comandos coinciden byte-a-byte". Cierto que los
>    **comandos** coinciden — pero el bug **NO es un comando**, es el **mecanismo de lectura RX del HIF**
>    (no se comparó). El bug A es **host-side**.
> 2. El comentario `0624g` dijo "SET_BSS_INFO(DISCONNECTED) cuelga el FW siempre". **Falso**: el original
>    lo manda en cada disconnect sin colgar. El cuelgue era por orden/formato (ver Exp C).

---

## EL 25% QUE FALTABA (ahora entendido al 100%)

### Bug A (sin DHCP OFFER) y Bug B (crash reconnect) = MISMA raíz: el handshake RX de WHISR
El original tiene **`CFG_SDIO_INTR_ENHANCE=1`** (`config.h:1000`). En cada pasada de RX,
`nicSDIOReadIntStatus` (`nic.c:1065`) hace **UNA lectura de bloque de 152B desde `MCR_WHISR`**
(`ENHANCE_MODE_DATA_STRUCT_T`) que trae: WHISR (word[0]), créditos TX, **`u2NumValidRx0Len`** (cuántas
tramas) y **`au2Rx0Len[16]`** (longitud de cada una). **Esa lectura ES el read-clear que re-arma
`RX0_DONE` en el FW** — un HANDSHAKE.

Nosotros sondeamos `MCR_WRPLR` crudo (registro de longitud, 0x50, **distinto** de WHISR 0x10) y hacemos
W1C (`wr(WHISR, rd(WHISR))`). Ni leemos el bloque ni completamos el read-clear → tras los primeros
paquetes el FW **deja de re-armar RX** (bug A: la OFFER no sube) y sondear WRDR0 a ciegas **cuelga el
AHB** (bug B). Además hoy drenamos **1 sola trama por puerto** → perdemos OFFER/EVENT encoladas.

### Modelo de interrupción: read-clear, sin IRQ
El original NO usa W1C: pone WHCR en **read-clear** (`W_INT_CLR_CTRL=BIT1` a **0**,
`HAL_SET_INTR_STATUS_READ_CLEAR`, `wlan_lib.c:1824`, "restore to hardware default"). En read-clear, el
acto de **leer** el bloque WHISR borra los flags y re-arma. El IRQ real (`HifAhbISR`) **no toca WHISR**:
solo despierta el thread. → **El polling del bloque WHISR equivale al IRQ.** Nuestro rx_thread por polling
es válido SIEMPRE que (a) WHCR esté en read-clear y (b) el drenado empiece por la lectura del bloque.

### Teardown: el original SÍ manda DISCONNECTED (sin colgar)
`aisFsmDisconnect` (`ais_fsm.c:3953`): **(1)** `nicPmIndicateBssAbort` (`CMD 0x1b`) **(2)**
`SET_BSS_INFO(conn_state=DISCONNECTED)` con **`sta_rec_idx_of_ap=0xFE`** (`STA_REC_INDEX_NOT_FOUND`).
El FW entonces libera sus STA-records + flush de colas QM. Host-side, `nicUpdateBss` hace
`cnmStaFreeAllStaByNetType` + `qmFreeAllByNetType`. **Eso limpia el estado stale que, sin él, cuelga el
reconnect (bug B).** El original NO demueve el STA a STATE_1 (lo dejó comentado, `ais_fsm.c:4035`).

### Eventos y claves (cerrado)
- **EVENT_ACTIVATE_STA_REC (0x13)** y **DEACTIVATE (0x14)**: bajo `#if 0` en el downstream → el host **NO
  responde nada**. Ignorarlos es CORRECTO (no son causa de ningún bug). `BEACON_TIMEOUT (0x1b)` tampoco.
- **CMD_DEFAULT_KEY_ID (0x09)**: existe solo como enum; el downstream **nunca lo envía**;
  `set_default_key` es NO-OP. Nosotros lo mandamos → **comando de más** (riesgo de crash). Quitar.
- **MIC-swap TKIP**: el downstream intercambia los 2 bloques MIC de 8B si `key_len==32`
  (`gl_cfg80211.c:240`). Nos falta → con TKIP la MIC va invertida. Obligatorio para TKIP (inofensivo CCMP).
- **CMD_UPDATE_WMM_PARMS (0x21)**: parte del JoinComplete en APs QoS; nos falta. No bloquea los 2 bugs;
  mejora robustez TX en APs WMM. Secundario.

---

## LOS 4 EXPERIMENTOS (código verificado, listo para pegar)

Orden recomendado: **A+B juntos** (B es prerequisito de A: sin read-clear, la lectura del bloque no
ack-ea) → cierran bug A. **C** → cierra bug B. **D** → TKIP + limpieza. Cada uno revertible.

### Experimento A — leer el bloque enhance de WHISR (★★★★★, bug A)

`reg.h` — struct + constantes:
```c
#define HIF_TARGET_WHISR        4       /* TRANS_TARGET_WHISR (mtreg.h E_TRANS_TARGET_T) */
#define HIF_RX_HW_APPENDED_LEN  4       /* DW de status que el HW añade tras la trama */
#define SDIO_ENHANCE_SIZE       152     /* sizeof(ENHANCE_MODE_DATA_STRUCT_T) */

struct enhance_mode_data {              /* 152 bytes */
    u32 u4WHISR;            /* word[0]  RX0_DONE=BIT1, TX_DONE=BIT0, ... */
    u8  ucTQCnt[6];         /* word[1..2] créditos TX por TC */
    u16 u2Rsrv;
    u16 u2NumValidRx0Len;   /* word[3].lo  nº tramas RX0 */
    u16 u2NumValidRx1Len;   /* word[3].hi  nº tramas RX1 */
    u16 au2Rx0Len[16];      /* word[4..11] longitud de cada trama RX0 */
    u16 au2Rx1Len[16];      /* word[12..19] longitud de cada trama RX1 */
    u32 u4RcvMailbox0;      /* word[21] */
    u32 u4RcvMailbox1;      /* word[22] */
} __packed;
```

`mt6582-wifi.c` — clon de `wifi_port_read_pio` con target WHISR (el handshake) + reescribir
`wifi_rx_drain` para iterar `u2NumValidRx0Len`/`au2Rx0Len[]` (RX0 por WRDR0) y RX1 (WRDR1), leyendo cada
trama `ALIGN(len + HIF_RX_HW_APPENDED_LEN, 4)`. Quita el `rd(MCR_WRPLR)`. (Código completo en el agente
RX-ENHANCE del output del workflow; aplicado en el commit junto a este doc.)

**Por qué arregla bug A:** la lectura del bloque desde WHISR re-arma RX0_DONE; iterar n tramas saca las
OFFER/EVENT encoladas. **Por qué ayuda bug B:** las longitudes vienen de `u2NumValidRxNLen` (autoridad
del FW), no de un WRPLR stale → no se lee WRDR0 a ciegas.

### Experimento B — WHCR read-clear, quitar el W1C (★★★★, prerequisito de A)
En el bringup (`~L1680-1694`): añadir `whcr &= ~WHCR_W_INT_CLR_CTRL;` (bit1=0 → read-clear) y **borrar**
`wr(MCR_WHISR, rd(MCR_WHISR))` (W1C). Con read-clear, leer el bloque (Exp A) ya limpia y re-arma.
**Crítico:** sin B, el HW puede no estar en read-clear → la lectura de A no borra → bug A persiste.

### Experimento C — teardown bien formado (★★★★, bug B)
`reg.h`: `#define STA_REC_INDEX_NOT_FOUND 0xFE`, `#define CMD_ID_INDICATE_PM_BSS_ABORT 0x1b`,
`struct cmd_pm_bss_abort { u8 net_type_idx; u8 rsv[3]; }`.
`wifi_cfg_disconnect`: **borrar** el demote STA→STATE_1 y el BSS_ACTIVATE(deact); en su lugar, en orden:
1. `PM_BSS_ABORT(AIS)` (CMD 0x1b).
2. `SET_BSS_INFO` sobre **copia local** de `saved_bi` con `conn_state=DISCONNECTED(1)` y
   `sta_rec_idx_of_ap=0xFE`. (No corromper `w->saved_bi`.)

### Experimento D — MIC-swap TKIP + set_default_key NO-OP (★★★, limpieza)
- `wifi_cfg_add_key`: si `key_len==32`, swapear `key_material[16:24]`↔`[24:32]` (con la fuente
  `params->key[24]`). Mapear `algorithm_id` por longitud (5=WEP40, 13=WEP104, 16=CCMP, 32=TKIP).
- `wifi_cfg_set_default_key`: convertir en NO-OP (quitar `CMD_ID_DEFAULT_KEY_ID`).

---

## Verificación / límites
- Todo el C está verificado contra structs/líneas reales (los 4 agentes leyeron `nic.c`, `nic_rx.c`,
  `mtreg.h`, `hal.h`, `ais_fsm.c`, `gl_cfg80211.c`, `wlan_oid.c` + nuestro PI.c/reg.h).
- **Lo que sigue siendo caja negra:** el binario cifrado del FW (cómo aplica la GTK al WTBL). Pero los
  Experimentos A-D son **host-side** y NO requieren entenderlo — por eso el RE del firmware (Misión 1)
  es plan B, no el camino crítico.
- **Test:** flashear A+B en 1ª conexión de boot fresco, WPA2 + `udhcpc`. Si llega la OFFER (rx>2, IP) →
  bug A cerrado. Luego C: desconectar + reconectar en el mismo boot sin colgar → bug B cerrado.
