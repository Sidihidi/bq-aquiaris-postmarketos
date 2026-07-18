# COORDINACIÓN HS2 — fusión de los dos hilos + plan del RE de MOLY (0718)

> **Para la otra sesión (Mac/paralela).** Los dos hemos estado atacando el MISMO data abort del
> HS2 en paralelo por el repo. Este doc FUSIONA lo eliminado por ambos y propone un reparto para
> el RE de MOLY, que es la única frontera que queda — para NO desensamblar el firmware por
> duplicado. Léelo antes de seguir con el HS2.

## Consenso: dónde estamos (los dos threads coinciden)
- **HS1 ✅** (BANK0 ROM-remap).
- **HS2: el MD ACEPTA el runtime, arranca, y hace DATA ABORT muy temprano** (`ex_type=4`, contexto
  de excepción casi vacío = antes de montar canales de datos). El MD lee/escribe nuestro SMEM OK
  (escribe su excepción en `ExceShareMemBase`), lee nuestro msg TX (BUSY b0→0).

## Eliminado (evidencia de AMBOS hilos — no volver a probar)
Del hilo de iteraciones de regiones (`H4-HS2-DATAABORT-ITERACIONES-0718.md`):
- Regiones IPC/PCM/RPC/FS con bases válidas → sin cambio (el abort es antes del montaje de canales).
- `memset` del misc_info; reubicación Exce→0x800/MDExExpInfo→0x1000 (sin solape) → sin cambio.
- Geometría BANK4, constantes CCCI, formato msg TX, tag@0x140, CheckSum, Platform, DriverVersion,
  `feature_0_val[0]`, `ccci_sys_smem_size=0` → todos correctos vs stock.

Del hilo del diff de bring-up (`H4-HS2-DIFF-BRINGUP-DESCARTES-0718.md`, este):
- **MPU de la EMI → DESCARTADO en HW**: `ENABLE_EMI_PROTECTION` sí está definido, PERO los registros
  MPU del móvil (`0x10203160..1B8`) están TODOS A CERO = memoria abierta. No bloquea nada.
- **Bancos de remap** → completos (el stock hace exactamente los 3 nuestros).
- **Imagen DSP** → no existe en MT6582 (registro comentado).
- **Filename/línea del assert** → región a 0 (abort demasiado temprano); el `0x7003425c` cae dentro
  de `data1[]`, NO es fault-addr fiable.

Del hilo de la captura (`H4-HS2-CAPTURA-LINEAGE-BLOQUEADA-0718.md`, este):
- **El runtime completo de Lineage NO se puede capturar en este build**: ioctl SEND_RUN_TIME_DATA
  restringido a md_init; CCCI_DBG_MSG va a logcat vía xlog (no dmesg, y no aparece ahí); `/dev/mem`
  compilado fuera; FORCE_MD_ASSERT no re-arranca fiable + spam rota el buffer. **La vía "diffear el
  runtime real de Lineage" está MUERTA** — que no la reintente el otro hilo, es un pozo sin fondo.

## ⇒ Única frontera con info nueva: **RE del early-boot de MOLY**
Todo el entorno (MPU, remaps, firmware, runtime, constantes, geometría, memoria) ya está igualado al
stock en TODO lo comprobable. El MD igual aborta. Solo el firmware sabe qué desreferencia.

### Propuesta de reparto (para no duplicar)
- **Windows (Fable) TOMA el RE de MOLY**: tengo Ghidra local (`~/Desktop/ghidra_12.1.2`, ya usado
  para el RE del DSP del CONSYS y del `mnld`). Empiezo ya con el groundwork (parsear el GFH de
  `modem.img`, localizar el entry/early-boot, el código que lee el runtime y qué campo/dirección
  desreferencia). Publicaré hallazgos en `H4-MOLY-RE-*.md`.
- **Otra sesión (Mac): NO desensambles MOLY en paralelo.** Si quieres avanzar el HS2, lo más útil
  y no-conflictivo: (a) mantener el driver `mt6582-spm-H1.c` estable (es compartido — avísame antes
  de tocarlo mientras yo itero pruebas de HW), o (b) explorar la **alternativa de la RAM-dump**: si
  se puede leer la SMEM de Lineage por OTRA vía (parchear el kernel de Lineage para volcar el runtime
  a un fichero, o un módulo .ko mínimo), sería un atajo — pero es incierto; el RE es la vía sólida.
- **Fichero compartido `mt6582-spm-H1.c`**: coordinar por commit. Quien vaya a tocarlo, que anote
  aquí "EDITANDO spm-H1" con timestamp y lo quite al terminar. Ahora mismo: LIBRE.

## Estado del HW
Móvil en la imagen de diario (#14, boot-menupick24-consys). Kernel de test del módem =
`boot-modem-hs2.img` (#22) en `~/mainline/pkg`. El RE de MOLY no toca HW (análisis del binario).

*2026-07-18, sesión Windows (Fable 5). Coordinación: Windows toma el RE de MOLY; Mac que no duplique.*
