# Módem H1 — MTCMOS del dominio del MD: EJECUTADO Y VALIDADO EN HW — 0713

> Primer hito medible del port del módem ([PLAN-M1](PLAN-M1-ARRANQUE-MD-0712.md) §H1). Añade el
> power-on MTCMOS del dominio del MD (modem processor) a `mt6582-spm.c`, disparable por sysfs.
> **RESULTADO: la secuencia ejecuta sin error y deja `SPM_MD_PWR_STATUS` bit0 = 1** (criterio de éxito).

## Qué se hizo
- `mt6582-spm-H1.c` (source completo en este dir): añade defines del dominio MD + `spm_md_power_on()`
  + `module_param_cb(spm_md_poweron, 0200)`. Secuencia 1:1 con `spm_mtcmos_ctrl_mdsys1(STA_POWER_ON)`
  del stock (`SPM_MD_PWR_CON` 0x10006284: PWR_ON→PWR_ON_S→wait PWR_STATUS bit0→clr CLK_DIS/ISO→
  set RST_B→clr SRAM_PDN; luego clr MD1_PROT_MASK 0xb8 en INFRA TOPAXI_PROT_EN 0x10001220 + wait STA1).
- **ioremap PEREZOSO**: el `ioremap(0x10001000, 0x400)` del INFRACFG es LOCAL a la función (map/use/
  iounmap), NO en el probe → **nada del código del MD toca el arranque** (el probe queda idéntico al
  bueno). Disparo: `echo 1 > /sys/module/mt6582_spm/parameters/spm_md_poweron`.

## Resultado en HW (dmesg, kernel con menupick18)
```
MD power-on: PWR_STATUS bit0 antes=1 CON=0xd
MD power-on OK: PWR_STATUS=0x3f5f CON=0xd PROT_STA1=0xe000 -> bit0=1
```
- `SPM_MD_PWR_CON = 0xd` = PWR_RST_B(b0)|PWR_ON(b2)|PWR_ON_S(b3), ISO/CLK_DIS/SRAM_PDN limpios = estado
  "encendido" correcto.
- **`SPM_PWR_STATUS` bit0 = 1** = SPM confirma el dominio del MD ALIMENTADO → **criterio de éxito H1 ✅**.
- `PROT_STA1 = 0xe000` → los bits `MD1_PROT_MASK` (0xb8) están LIMPIOS = **protección del bus TOPAXI del
  MD RELEASED** (el MD puede acceder al bus).

## Matiz honesto (para H2/H3)
`bit0 antes=1`: **el MD ya estaba alimentado** al arrancar (el LK deja el dominio ON, o el kernel no lo
apaga). La secuencia lo **confirma/mantiene** en el estado correcto, no lo enciende de 0→1. Implicaciones:
- El dominio MD está **disponible desde el boot** (no hay que encenderlo para H2/H3 — un plus).
- Para una validación 0→1 estricta haría falta un `spm_md_power_off()` previo (fuera del scope de H1).
La secuencia es correcta (idéntica al stock, deja el estado esperado) → **MTCMOS del MD dominado**.

## Lecciones de esta iteración (importantes)
- **El culpable de los bootloops NO era H1** ni el EXTRA_FIRMWARE: era el **DTB del árbol compartido
  modificado por la sesión Mac (.123) hoy 11:43** (quitó el interrupt-controller del PMIC + rtc "por
  bootloop en .123", re-activó nvmem del thermal, status=disabled en consys). Ese DTS rompe NUESTRO árbol.
  → menupick14/15/16/17 (todos con ese dtb) hacían bucle; GANADOR (dtb viejo) arranca.
- **Fix de aislamiento**: se extrajo el **DTB exacto de GANADOR** de su imagen (`ganador.dtb`, magic
  d00dfeed @0xc92c00, md5 85f7266a) y se empaquetó **menupick18 = zImage(H1) + ganador.dtb** → arranca →
  H1 validado sin el ruido del DTS del Mac.
- ⚠️ **COORDINACIÓN**: la sesión Mac sigue metiendo cambios en el DTS compartido que rompen nuestros
  builds (recurrente). Regla pendiente de aplicar de verdad: DTS del krillin = por-sesión o marcado.

## Siguiente (H2)
Nodo `reserved-memory` 24MB/32M-align + ioremaps CCIF (0x1020A000/B000) + boot-slave (0x20190000/379C/
5488) + RGU (0x20050000) + replicar los BANK-remap (bloqueante #1 del plan). Firmware ya extraído (H0):
`MOLY.WR8.W1449.MD.WG.MP.V1`. Fuente H1 = `mt6582-spm-H1.c` (este dir) / en la Pi `mt6582-spm.c`.

*H1 ejecutado y validado en HW 2026-07-13 (menupick18: zImage H1 + dtb GANADOR).*
