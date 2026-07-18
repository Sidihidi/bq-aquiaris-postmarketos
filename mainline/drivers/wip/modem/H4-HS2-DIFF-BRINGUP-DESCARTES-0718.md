# Módem H4 — diff del bring-up: MPU de la EMI y 3 sospechosos más DESCARTADOS (0718)

> Ataque al diff "bring-up completo de Lineage vs nuestro mínimo", empezando por el MPU de la EMI.
> **Resultado: 4 descartes limpios, ninguno era la causa del data abort.** Todos verificados en HW o
> en el source, sin gastar ciclos de flash a ciegas. Valor: la búsqueda queda mucho más acotada.

## 1. MPU de la EMI — DESCARTADO (medido en HW) ⭐
El doc previo del H3 decía que `emi_mpu_set_region_protection` "se compila OFF en bring-up".
**Eso era FALSO**: `ENABLE_EMI_PROTECTION` **SÍ está definido** (`ccci_platform_cfg.h:86`, con el
comentario "//disable for bring up" — o sea, está activo). Así que Lineage SÍ configura el MPU y
nosotros no → parecía el sospechoso perfecto.

**Pero al leer los registros del MPU en el móvil, están TODOS A CERO:**
```
EMI_BASE fisica = 0x10203000  (virtual downstream 0xF0203000; mapeo MTK 0xF0->0x10)
addr regs r0..r7 (MPUA..H = +0x160,168,170,178,180,188,190,198): TODOS 0x00000000
perm regs (MPUI..L = +0x1A0,1A8,1B0,1B8):                        TODOS 0x00000000
```
Registros a 0 = **ninguna región definida y permisos NO_PROTECTION para todos los dominios** =
memoria completamente ABIERTA. El MPU **no está bloqueando nada**. Configurarlo solo AÑADIRÍA
restricciones — nunca arreglaría un data abort. **Descartado.**

Referencia para el futuro (si alguna vez hace falta portarlo, p.ej. para proteger la RAM del AP
del MD): la tabla y el encoding quedan documentados:
- `SET_ACCESS_PERMISSON(d3,d2,d1,d0) = (d3<<9)|(d2<<6)|(d1<<3)|d0`; dominios D0=AP, D1=MD, D2=CONNSYS, D3=MM.
- Permisos: `NO_PROTECTION=0, SEC_RW=1, SEC_RW_NSEC_R=2, SEC_RW_NSEC_W=3, SEC_R_NSEC_R=4, FORBIDDEN=5`.
- Regiones MD1: 1=ROM `(FORBIDDEN,FORBIDDEN,SEC_R_NSEC_R,SEC_R_NSEC_R)`=0xB64;
  2=R/W `(FORBIDDEN,FORBIDDEN,NO_PROTECTION,FORBIDDEN)`=0xB45;
  3=Share `(FORBIDDEN,FORBIDDEN,NO_PROTECTION,NO_PROTECTION)`=0xB40;
  6=AP `(NO_PROTECTION,FORBIDDEN,SEC_R_NSEC_R,NO_PROTECTION)`=0x160.
- Direcciones: `(addr-0x80000000)>>16` (granularidad 64KB), reg = `(start<<16)|end`.
- Por región: addr en MPUA+8*r; permiso en MPUI+8*(r/2), mitad baja si r par, alta si impar.

## 2. Bancos de remap — COMPLETOS (descartado)
El stock hace **exactamente 3 remaps**, los mismos que nosotros:
`set_ap_smem_remap(0x40000000)` → AP_BANK4_MAP0/1; `set_md_smem_remap(0x40000000)` → MD1_BANK4_MAP0/1;
`set_md_rom_rw_mem_remap(0x00000000)` → MD1_BANK0_MAP0/1. No hay BANK1/2/3 ni ventanas extra que nos falten.

## 3. Imagen DSP — NO EXISTE en esta plataforma (descartado)
`load_std_firmware` maneja `DSP_INDEX`, pero el registro de la imagen DSP está **comentado**
(`//md_img_info[md_id][DSP_INDEX]... //no dsp image on MT6589`). Solo se carga `modem.img`. No falta firmware.

## 4. Filename/línea del assert — NO DISPONIBLE (el abort es demasiado temprano)
`modem_assert_log` tiene `char filename[24]; int linenumber;` (~offset 216) — sería oro para saber
qué le falla a MOLY. Re-ejecuté la secuencia (kernel #22) y leí la región Exce entera:
**offsets 192-288 todos a CERO**. El MD aborta antes de poblar el assert detallado. Lo único que
escribe: `+0x00 ex_type=4 (DATA_ABT)`, `+0x04 ".MD.WG.MP.V1"` (versión), `+0x10 0xff`, `+0x24 0x7003425c`.
⚠️ Ojo: el `0x7003425c` cae DENTRO de `data1[212]` (que empieza en +4), así que **no es un campo
"fault address" fiable** — es contenido del buffer del assert. No sobre-interpretarlo.

## Dónde queda la búsqueda
Descartados hasta ahora (todos con evidencia): geometría BANK4, constantes del mensaje CCCI, CheckSum,
offset del tag, **MPU de la EMI**, **bancos de remap**, **imagen DSP**, y los campos del runtime
(damos todas las regiones no-cero del source). El MD acepta el runtime, arranca y aborta muy temprano
con memoria totalmente abierta.

**Siguiente frontera realista:** desensamblar el early-boot de MOLY (`modem.img`, 5.17MB, ARM;
localizar qué hace tras leer el runtime y qué dirección desreferencia). Es el único camino que queda
con información nueva — el resto del entorno ya está igualado al stock en todo lo comprobable.
Alternativa de menor coste: instrumentar el CCIF para capturar TODOS los mensajes del MD (canales 1 y 2
llegan: id=4 "EXCP" y id=6) y decodificar el id=6, que quizá lleve más contexto.

*2026-07-18, sesión Windows (Fable 5). MPU descartado en HW; búsqueda acotada al firmware MOLY.*
