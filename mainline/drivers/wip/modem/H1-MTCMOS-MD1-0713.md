# Módem M1 — H1: dominio MTCMOS `SYS_MD1` — HECHO (2026-07-13)

> **Resultado: el dominio de potencia del MD ya está ENCENDIDO al arrancar (lo deja el LK).**
> Nuestra secuencia MTCMOS (clon de la del CONNSYS, probada) corre limpia e idempotente. El
> `PWR_STATUS bit0` del MD1 = 1 desde el boot. **H1 satisfecho + de-risk de M1** (un paso menos).

## Cómo se hizo
Módulo out-of-tree `md1-mtcmos.c` (cargable en caliente, como el magnetómetro — CERO reflasheo,
CERO riesgo al boot). Mapea SPM (`0x10006000`) + INFRACFG_AO (`0x10001000`), expone
`/sys/kernel/debug/md1_mtcmos/power` (read = status, write "1" = power-on). Clon EXACTO de
`mt6582-consys.c::consys_spm_power_on()` con los regs del MD1:
- `SPM_MD_PWR_CON = 0x284` (CONN es 0x280), **`MD1_PWR_STA_MASK = bit0`** (CONN = bit1),
  **`MD1_PROT_MASK = 0x00B8`** (CONN = 0x0104). Layout de bits del PWR_CON idéntico.
Build: `make -C build-krillin M=$PWD ... modules` → vermagic `7.0.12 SMP` (carga en #284).

## Medición (pmOS #284, HW)
```
baseline: CON=0x0000000d  STA=0x3f5f(md1=1)  STA_S=0x3f5f(md1=1)  PROT_STA1=0xe000(md1=0)
tras echo 1 > power: CON=0x0d (sin cambio), md1=1, prot wait=0us, sistema VIVO (uptime intacto)
```
Interpretación de `CON=0x0d` = `PWR_RST_B | PWR_ON | PWR_ON_S` con ISO/CLK_DIS/SRAM_PDN a 0 =
**dominio completamente encendido, sin aislar, SRAM on, reset liberado**. Bus TOPAXI del MD1 ya
sin protección (`PROT_STA1 & 0x00B8 == 0`). Coherente: el LK enciende el MD para que Android lo
arranque por CCCI; deja el dominio vivo aunque el OS no lo use.

## Qué significa para M1
- **La secuencia MTCMOS NO es bloqueante** — el MD está alimentado. En `mt6582-spm.c` la añadimos
  como función defensiva/idempotente (por si en algún flujo el MD se apagara), pero el bring-up
  NO depende de encenderlo.
- El trabajo real de M1 se concentra en **H2 (carveout 24MB/32M + remap BANK4)** y **H3 (port
  CCCI boot-only + boot-slave keys + IRQ 132)**. La CPU del MD sigue sin correr: eso lo gobiernan
  los boot-slave keys (`Key 0x2019379C`, `Vector 0x20190000`, `En 0x20195488`) + el handshake CCCI,
  no el MTCMOS. Nuestro power-on libera el reset del *dominio*, no arranca el *MD CPU*.

## Caveat honesto
Como MD1 ya estaba ON, la prueba confirma que nuestros regs/máscaras son correctos y que la
secuencia es segura e idempotente, pero **no demostró encender un dominio que estuviera OFF**
(no escribí `power_off`). Confirmación opcional futura: añadir off→on y togglear (seguro en pmOS,
que no usa el MD). No bloquea: para M1 basta con que el dominio esté disponible, y lo está.
Nota menor: la asignación `bit0=MD1` viene del RE del plan; consistente con el resto de bits
observados (CONN=bit1, etc.), pero si en H3 el MD no responde, re-verificar el mapeo de bits.

## Ficheros
- `md1-mtcmos.c` (este dir) — el módulo de prueba; reutilizable en H3 para power-cycle del MD.
- En la Pi: `~/modem-h0/md1-test/` (fuente + Kbuild + .ko).

*Sesión Mac (Fable 5), 2026-07-13. H0 ✅ + H1 ✅ → siguiente H2 (carveout + remap).*
