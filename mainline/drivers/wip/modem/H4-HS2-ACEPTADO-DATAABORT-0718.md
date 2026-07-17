# Módem H4 — HS2: el MD ACEPTA el runtime y arranca → DATA ABORT (0718, sesión Windows)

> **AVANCE GRANDE.** Con Exce + MDExExpInfo + el support_mask/postfix del misc_info, el MD
> pasó de "rechazo silencioso" a **aceptar el runtime, arrancar y responder**. Responde con
> una **EXCEPCIÓN (DATA ABORT, type 4)** en vez de NORMAL_BOOT_ID → el MD boota más lejos pero
> accede a memoria inválida. Siguiente frontera: la región/puntero que falta.

## Lo que cerró la ACEPTACIÓN del runtime (4 fixes, sobre el Platform_H previo)
Diagnóstico decisivo (instrumenté el `spm_md_hs2`): el MD envía HS1 correcto
`ffffffff 00000000 00000000 5555ffff` (`{magic, MD_INIT_START_BOOT=0, CCCI_CONTROL_RX=0,
MD_INIT_CHK_ID=0x5555FFFF}`), lee nuestro SMEM (read-back OK) y **lee nuestro msg TX** (BUSY b0→0).
Con el runtime mínimo (solo Platform) el MD lo leía y NO respondía. Los fixes que lo hicieron
aceptar:
1. **Exce region** (runtime campos 35/36): `ExceShareMemBase` = MD-view(SMEM+0x600),
   `ExceShareMemSize` = `0x800` (MD_EX_LOG_SIZE). El MD la EXIGE — de hecho escribe ahí su
   propio dump de excepción (lo confirmamos leyéndola).
2. **MDExExpInfo** (campos 62/63): base = MD-view(SMEM+0xE00), size = 12 (`sizeof(modem_exception_exp_t)`).
3. **misc_info support_mask** = `0x1` (`FEATURE_SUPPORT<<MISC_DMA_ADDR`), estaba a 0.
4. **misc_info postfix** "CCIF" en el offset **+284** (tras prefix/support_mask/index/next +
   feature_0..15_val[4] + reserved_2[3]). `config_misc_info` pone prefix Y postfix; nos faltaba
   el postfix.

## La respuesta del MD = DATA ABORT (leído del CCIF por devmem)
```
RCHNUM=0x6 (canales 1 y 2)
CCIF RX ch1 (0x1020A190): ffffffff 00000004 00000000 45584350
   -> {magic, id=4, channel=CCCI_CONTROL_RX=0, reserved=0x45584350}
      id=4 = MD_EX_TYPE_DATA_ABT ; 0x45584350 = MD_EX_CHK_ID ("EXCP")
CCIF RX ch2 (0x1020A1A0): ffffffff 00000006 00000000 45524543  ("CERE")
```
Región Exce (SMEM+0x600, lo que escribió el MD):
```
+0x00: 0x00000004               (ex type = DATA_ABORT)
+0x04: ".MD.WG.MP.V1"           (versión MOLY = header estándar de excepción MD)
+0x10: 0x000000ff
+0x24: 0x7003425c               (posible fault addr / offender en espacio-MD)
resto: 0                        (abort MUY temprano, sin contexto completo de registros)
```

## Interpretación y siguiente paso
El MD acepta el runtime, empieza a bootear y a los ~0ms hace un DATA ABORT accediendo a una
dirección inválida (0x7003425c, fuera de nuestro carveout 0xB8xxxxxx / vista-MD 0x40xxxxxx).
Hipótesis principal: **una sub-región ShareMem que el MD desreferencia y que dejamos a 0**
(base=0 → vista-MD 0x0 = ROM vía BANK0; el MD lee un puntero basura de ahí y lo desreferencia).
La receta ([H3-HS2-RECETA-0717.md](H3-HS2-RECETA-0717.md)) ya lo anticipaba: "iterar — si el MD
se queja de una región, dársela".

**Candidatos de la región faltante (por probabilidad de uso temprano en boot):**
1. **SysShareMem** (campos 37/38) — intercambio system/EINT AP↔MD, se usa pronto.
2. **IPCShareMem** (39/40), **Mdlog** (8/9), **IPCMDIlm** (64/65).
3. Alternativa robusta: replicar el layout COMPLETO del smem_table (todas las regiones con base
   válida no-solapada en los 2MB de SMEM), como el stock. Requiere ampliar el `ioremap` (hoy 0x2000)
   y calcular tamaños de `cal_md_smem_size`.
**Mejor aún**: decodificar la struct EX del lado-MD (firmware MOLY) para leer la fault-addr exacta
del +0x24 y saber QUÉ región/puntero es — evita iterar a ciegas.

## Estado
HS1 ✅ (BANK0). Runtime **ACEPTADO** por el MD ✅ (primer MT6582 que llega aquí en mainline). HS2 =
data abort, iteración en curso. Driver `mt6582-spm-H1.c` (snapshot con los 4 fixes + instrumentación
diagnóstica del hs2). Test `boot-modem-hs2.img` (#20). El `spm_md_hs2` ahora vuelca: msg HS1 del MD,
read-back del SMEM, estado post-TX, y poll de RCHNUM+START.

## ⚠️ Coordinación
HS2 es iteración ACTIVA del Mac. Reparto: Mac escribe driver, Windows (Fable) aporta diagnóstico HW +
ground-truth. Los 4 fixes de aceptación + la instrumentación están en el snapshot; sincronizar antes de
seguir para no pisar el `mt6582-spm.c` compartido.

*2026-07-18, sesión Windows (Fable 5). HS2: runtime aceptado; data abort = siguiente frontera.*
