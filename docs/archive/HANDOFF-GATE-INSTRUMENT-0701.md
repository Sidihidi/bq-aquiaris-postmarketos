# HANDOFF — Instrumentación de los GATES en add_key (Windows/Opus 4.8, 2026-07-01)

Aporte a vuestro bloqueo "**leer `0x020axxxx` conectado crashea**". No toco el móvil (regla de un solo móvil);
dejo el código **compile-verificado** (en la Pi .38, build-krillin: `CC mt6582-wifi.o` OK; solo el warning de
frame-size preexistente de la línea 932). Flashéalo tú cuando tengas el móvil.

## Qué añadí a `mt6582-wifi.c`
`wifi_diag_gates(w, pairwise)`, llamada desde `wifi_cfg_add_key` **tras soltar `hif_lock`** (evita deadlock:
`wifi_runtime_reg_read` re-lockea) y **al 4-way** (puerto 1 aún quieto). Lee y **LOGUEA con `dev_info`**
(console-ramoops es SÍNCRONO → el valor queda capturado aunque una lectura posterior cuelgue) los gates del BSS:
- `bss = *0x020a0068` (idle → `0x020a1000`), y `*0x020a0098`.
- `bss+0x37b` (word) = `[0x12e3]`, `bss+0x38d` (word) = `[0x12f5]`, + candidatos `+0x347`/`+0x359`.
- Con **`poke_gates=1`** (nuevo debugfs) fuerza `[0x12e3]=1` y `[0x12f5]=1` (RMW por palabra de 32b) → **prueba el fix**.

## Por qué esto esquiva vuestro crash
El `fwdump` de RANGO por debugfs, con la conexión en tráfico de datos, desincroniza el stream de eventos
ACCESS_REG del puerto 1 (compiten con los data frames) → el driver/FW se desincroniza → reset. Aquí: **pocas
lecturas**, **al 4-way** (antes del tráfico de datos pesado), en el contexto del driver, con **log síncrono** que
sobrevive al crash. Si aun así cuelga, en console-ramoops ya está el valor del gate leído antes.

## Receta de test
1. **Flashear** (build-krillin → `wifi-iter-w.sh`).
2. **Fase 0 (confirmar el bug):** conectar WPA2 a "hola" (receta `HANDOFF-CASA-0630.md`). Tras el connect
   (aunque crashee/reinicie), leer:
   ```sh
   cat /sys/fs/pstore/console-ramoops-0 | grep 'DIAG gates'
   ```
   → esperado `[0x12e3]` y `[0x12f5]` = **0** (el bug). Fija qué byte exacto es cada flag (el que está a 0).
3. **Fase 1 (probar el fix):**
   ```sh
   echo 1 > /sys/kernel/debug/mt6582_wifi/poke_gates
   # reconectar (re-dispara add_key → el poke pone los gates a 1)
   udhcpc -i wlan0 -t 6 -n
   ```
   → si llega la **OFFER** / hay IP: **el gate ES el bug y el poke lo desbloquea** (fix confirmado). Si no, mirar
   en console-ramoops el log `FIX gates POKE` + releer los gates para ver si el poke prendió/persistió.

## Notas
- Si el poke al 4-way no persiste (el FW resetea el gate tras el add_key), pokear DESPUÉS con `fwpoke` a la
  dirección ya conocida (bss+0x37b / bss+0x38d, byte correcto), o mover el poke a un punto posterior del connect.
- Si el read en `wifi_diag_gates` sigue crasheando (incluso pocas lecturas al 4-way): reducir a 1 sola palabra
  (`bss+0x378`) o pausar el `rx_thread` alrededor (`irq` off / flag) durante las lecturas.
- Backups en .38: `mt6582-wifi.c.bak-pre-gates-0701`, `mt6582-wifi-reg.h.bak-pre-gates-0701`.

*Co-autor: Claude Opus 4.8 (sesión Windows). Sobre `HANDOFF-FWDUMP-GATES-0701.md` (Mac).*
