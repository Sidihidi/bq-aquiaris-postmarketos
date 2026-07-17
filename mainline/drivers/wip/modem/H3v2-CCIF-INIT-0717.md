# Módem H3 v2 — la pieza que faltaba del handshake: INIT del CCIF (0717)

> Retomando el módem tras cerrar el CONSYS. El 1er intento H3 (0714) dejó el
> andamiaje validado (load/remap/release seguros) pero **el MD no levantaba el
> handshake CCCI** (CON/RCHNUM=0). Candidato #1 del doc H3 era "CCIF init".
> **CONFIRMADO contra el source stock: sí, faltaba inicializar el CCIF.**

## El hallazgo

El stock inicializa el mailbox CCIF del lado AP en el probe del driver, MUCHO antes
de soltar el MD (`__ccif_v1_init`, `dual_ccci/ccci_hw.c:263`):

```c
ccci_write32(CCIF_CON(base), CCIF_CON_ARB);   // CON = 0x1  (modo arbitración)
ccci_write32(CCIF_ACK(base), 0xFF);           // ACK = 0xFF (todos los canales)
```

(y un lazo que reasegura `while(ARB != read(CON)) write(CON, ARB)`, ccci_hw.c:165).

Offsets (mt6582 `dual_ccci/include/ccci_platform_cfg.h`), base AP_CCIF = **0x1020A000**:

| reg | offset | |
|---|---|---|
| CCIF_CON | +0x00 | `CCIF_CON_ARB = 0x01`, `CCIF_CON_SEQ = 0x00` |
| CCIF_BUSY | +0x04 | |
| CCIF_START | +0x08 | |
| CCIF_TCHNUM | +0x0C | |
| CCIF_RCHNUM | +0x10 | |
| CCIF_ACK | +0x14 | |
| CCIF_TXCHDATA | +0x100 | |
| CCIF_RXCHDATA | +0x180 | (0x100+128) |

**Nuestro `spm_md_release` del H3 v1 solo LEÍA el CCIF; nunca escribía CON ni ACK.**
Con el CCIF sin poner en modo ARB, el AP no refleja las escrituras del MD → el
handshake nunca "registra" aunque el MD esté ejecutando MOLY. Por eso CON/BUSY/RCHNUM
se quedaban a 0 tras el release.

## El fix (aplicado a `mt6582-spm-H1.c`, spm_md_release)

Antes de los boot-slave keys, añadir:

```c
writel(0x1,  ccif + 0x00);   /* CCIF_CON = ARB */
writel(0xFF, ccif + 0x14);   /* CCIF_ACK = todos los canales */
(void)readl(ccif + 0x00);    /* flush */
```

## Estado y siguiente paso (test real, cuando se pueda hacer un ciclo cuidado)

⚠️ El código H3 (mt6582-spm-H1.c) es del árbol del hilo Mac (menupick23) y **NO está
en el árbol de diario actual** (#14 = boot-menupick24-consys, la línea CONSYS). Para
probar el fix hay que:

1. **Integrar el módulo H3 en el árbol #14** (reconciliar `mt6582-spm.c` actual con
   `mt6582-spm-H1.c`: son el mismo driver SPM + las triggers sysfs
   `spm_md_{load,remap,release,dump}` y el código H1/H2/H3).
2. **Añadir el nodo carveout** al DTS (24MB `no-map` bajo el CMA — ojo con el gotcha
   del `simple-bus`/`ranges` del H2, ver H2-CARVEOUT-REMAP-0713.md).
3. Build → **imagen de test aparte** (NO tocar el daily; como se hizo boot-stpfull
   para el consys) → flash → `md_load` → `md_remap` → `md_release`.
4. Éxito HS1 = tras el release, `CON`/`RCHNUM` del CCIF **cambian** (el MD habla) y/o
   dmesg del handler. Si HS1 llega, siguiente = runtime data (set_md_runtime) para HS2.

**Alternativa de oro (nueva capacidad de esta era): capturar el handshake en LineageOS
vivo.** El H0 anotó que el handshake CCCI "scrolleó del dmesg" y no se capturó — pero
ahora dominamos el método Lineage-vivo (flash lineage13-boot.img desde pmOS + adb desde
la Pi). Con logs CCCI en debug (`echo 8 > /proc/ccci_*` o el equiv, por explorar) se
puede grabar la secuencia EXACTA del arranque del MD en el sistema que funciona:
registros CCIF, timing, y sobre todo la **estructura runtime-data** (el grueso de HS2).
Esto de-riska H3 tanto como el truco wmt_dbg de-riskeó el CONSYS.

## Ficheros
- `mt6582-spm-H1.c` (este dir) — H1+H2+H3, con el fix CCIF-init aplicado (spm_md_release).
- Fuente stock: `~/mainline/downstream/bq-src/mediatek/kernel/drivers/dual_ccci/ccci_hw.c`
  (`__ccif_v1_init`:263, reset:223, arb-loop:165) + `.../platform/mt6582/.../include/ccci_platform_cfg.h`.
- Contexto: H3-ARRANQUE-MD-0714.md (1er intento), H3-BRINGUP-CCCI-SPEC-0713.md (receta completa).
