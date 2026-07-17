# Módem H4 — HS2 data abort: iteraciones (0718, sesión Windows) — sigue temprano

> Continuación de [H4-HS2-ACEPTADO-DATAABORT-0718.md]. El MD acepta el runtime y arranca,
> pero hace **DATA ABORT muy temprano** (`ex_type=4`, `MD_EX`+`MD_EX_REC_OK` por CCIF).
> Iteré 3 hipótesis más — **ninguna lo movió**. El abort es antes de montar los canales de
> datos. **Siguiente paso definitivo: capturar el runtime COMPLETO de Lineage (DBG on) y
> diffear** — ya no vale adivinar regiones.

## Iteraciones probadas (kernels #21, #22) — SIN cambio (mismo ex_type=4)
1. **Regiones IPC/PCM/RPC/FS** (runtime campos 39/40, 10/11, 31/32, 29/30) con bases válidas en
   el SMEM. Hipótesis: el MD monta esos canales y desreferencia base=0. → NO cambió → el abort
   es ANTES del montaje de canales de datos.
2. **memset de la región misc_info** (0x400 bytes) antes de escribir — el stock hace
   `memset(&misc_info,0,sizeof)` y nosotros no (feature_1..15/reserved quedaban con basura del
   carveout). Fix correcto igualmente. → NO cambió.
3. **Reubicar Exce→SMEM+0x800, MDExExpInfo→0x1000** (había solape latente misc[0x400-0x800] con
   Exce[0x600]). El MD escribió su excepción en la nueva Exce (0x800) → confirma que lee bien el
   `ExceShareMemBase` del runtime. → NO cambió el abort.

## Verificado CORRECTO (no tocar; descartados como causa)
- Geometría BANK4/runtime_data_base (32MB: MD 0x41600000→0xB9600000; el MD lee/escribe ahí OK).
- Constantes: CCCI_CONTROL_RX=0, TX=1, MD_INIT_START_BOOT=0, MD_INIT_CHK_ID=0x5555FFFF.
- Formato msg TX del CCIF + trigger TCHNUM (BUSY b0 se limpia = el MD lo lee).
- Tag @ CCIF+0x140 (confirmado = CCIF_STD_V1_RUN_TIME_DATA_OFFSET).
- CheckSum (nunca se usa, 0 ok). Platform_L/H "MT65"/"82E1". DriverVersion 0x20121001.
- `feature_0_val[0]=0xB8000000` = `get_md_mem_start_addr` = `md_resv_mem_addr` (base física
  reservada, 32M-align) — idéntico al stock.
- `ccci_sys_smem_size=0` en el stock (SYS no se usa) — nuestro 0 es correcto.

## Naturaleza del abort (leído del CCIF+Exce por devmem)
```
CCIF ch1: id=4=MD_EX, chk=0x45584350 "EXCP"     (notifica excepción)
CCIF ch2: id=6=MD_EX_REC_OK, chk=0x45524543 "CERE" (excepción registrada)
Exce+0x00: ex_type=4 = MD_EX_TYPE_DATA_ABT
Exce+0x04..: version MOLY ".MD.WG.MP.V1" (EX_HEADER + EX_ENVINFO)
resto: casi 0 (sin dump de registros -> abort ANTES de poblar el contexto = MUY temprano)
```
El abort es a los ~0ms tras recibir nuestro msg, con contexto de excepción mínimo → el MD lee el
runtime y falla casi inmediatamente, antes de la fase de canales de datos. NO es una región de
datos que falte (probado). Sospecha: un campo del runtime que el MD desreferencia en la validación
temprana, o una diferencia sutil que solo el runtime REAL de Lineage revelaría.

## 🎯 SIGUIENTE PASO (definitivo, no adivinar): runtime completo de Lineage con DBG on
`ccci_dump_runtime_data` vuelca TODOS los campos (incluidos los ShareMem base/size) a nivel DBG,
gateado por `ccci_msg_mask`. En Lineage:
1. Habilitar el log DBG del CCCI (nodo de `register_filter_func`, p.ej.
   `echo "-l ..." > /proc/driver/... ` — localizar el path; o subir el loglevel).
2. Forzar un reboot del MD (o capturar el dmesg del boot a los ~16s como en el ROM-remap, que
   alcanza t=0).
3. Leer los ShareMem base/size REALES que Lineage pone (en SU carveout 0xBC/BD) y traducirlos a
   NUESTRO layout (0xB8/B9) con la misma fórmula. Diffear campo a campo contra los nuestros.
Alternativa: desensamblar el early-boot de MOLY (qué campo del runtime desreferencia).

## Estado
HS1 ✅. Runtime **ACEPTADO** ✅ (el MD arranca y procesa). Data abort temprano = frontera abierta.
Driver `mt6582-spm-H1.c` (todos los fixes de aceptación + IPC/PCM/RPC/FS + misc memset + regiones
reubicadas + instrumentación del hs2). Test #22 `boot-modem-hs2.img`. Backups en la Pi. Móvil
restaurado a la imagen de diario tras el test.

*2026-07-18, sesión Windows (Fable 5). Iteración de regiones agotada; toca el runtime real de Lineage.*
