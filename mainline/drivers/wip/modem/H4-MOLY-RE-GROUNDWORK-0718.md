# Módem H4 — RE del firmware MOLY: groundwork para el data abort del HS2 (0718)

> Base para desensamblar `modem.img` y encontrar qué desreferencia el MD tras leer el runtime
> (el data abort del HS2). **Groundwork hecho** (estructura, modo, handler del runtime localizado);
> el trace completo es trabajo de Ghidra de varios días. Aquí queda todo para continuar sin re-descubrir.

## El binario
- `~/mainline/downstream/stock-firmware-0713/modem.img` (5.172.580 B, md5 `0414422b`), NO en repo (blob).
- `MOLY.WR8.W1449.MD.WG.MP.V1` (MT6582 WCDMA/HSPA). Es un FICHERO (`request_firmware`), no partición.

## Estructura para cargar en Ghidra
- **Base de carga/enlace = 0x0** (espacio del MD; via BANK0 el MD ve el firmware en su 0x0).
- **Vectores ARM en 0x0** (`e59ff018` = `ldr pc,[pc,#0x18]`). Targets (tabla en 0x20-0x3c):
  reset→**0x98**, y handlers en 0x2e6xxx: undef 0x2e68d4, SWI 0x2e68f4, prefetch 0x2e6930,
  **data-abort 0x2e6950** (5º vector, offset 0x10 → target en 0x30).
- **Modo MIXTO**: el boot (desde 0x98) es **ARM**; el código principal es **Thumb-2**
  (verificado: `10 b5`=push, `04 46`=mov, `f0xx`=Thumb-2 32b). En Ghidra: ARM v7, dejar que
  auto-detecte Thumb, o marcar Thumb desde ~0x1000.
- GFH: no hay magic "MMM" al inicio; el check-header va al final (no crítico para el RE).
- Herramienta CLI: `arm-linux-gnueabihf-objdump -D -b binary -m arm [-M force-thumb] -EL
  --adjust-vma=0x0 --start-address=0xADDR --stop-address=0xADDR modem.img`.

## ⭐ Handler del runtime LOCALIZADO
El constante prefix "CCIF" (0x46494343) está en un pool en **0x6198**. La función que lo carga y
compara (= el validador del runtime que el MD ejecuta tras el HS1) está justo antes:
```
0x6148: cmp r0, #4          ; checks
0x6150: cmp r1, #15
0x6158: ldr r1, =[0x6194]
0x615a: ldr r4, =[0x6198]   ; <-- carga 0x46494343 "CCIF" (el prefix a comparar)
0x615c: ldr.w r1, [r1, #264]
...
```
**Punto de entrada del RE**: desensamblar la función completa que contiene 0x615a, seguir qué hace
con r4 (el prefix) y con la base del runtime, y trazar el PRIMER acceso a memoria que puede fallar
(un campo del runtime usado como puntero sin validar). El data abort es a los ~0ms tras el HS2 →
está en esta ruta temprana, antes del montaje de canales de datos.

## Patrón de assert de MOLY (para reconocer validaciones fallidas)
```
movs r2, #<linenum>      ; nº de línea del fichero fuente
ldr  r1, =<filename_ptr>
ldr  r0, =<msg_ptr>
blx  0x3fa2f0            ; <-- función de assert/exception del MD
```
Ejemplo real en 0x61ae-0x61b4 (línea 198). Si el data abort pasa por un assert (no un fault HW puro),
el filename+línea apuntan al check exacto. **Buscar en Ghidra los xrefs a 0x3fa2f0** cerca del handler
del runtime = lista de todo lo que valida en el boot temprano.

## Alternativas más baratas que el RE completo (evaluar antes de días de Ghidra)
1. **Bisección de campos en HW**: podemos disparar el abort a voluntad (`spm_md_hs2`). El runtime
   actual es ACEPTADO. Variar campos y ver si el abort cambia acota sin RE. Caveat: los campos
   requeridos (Prefix/Platform/Exce...) no se pueden quitar sin perder la aceptación; bisecar solo
   los opcionales/direcciones.
2. **Deriva del runtime de Lineage desde el SOURCE** (no captura — la captura está muerta): recalcular
   CADA campo con la fórmula del stock sobre NUESTRO layout y diffear byte a byte contra lo que
   escribimos. La RECETA lo hizo para los principales; faltaría un barrido EXHAUSTIVO de los 70 ints
   (BootAttributes, BootReadyID, los *PortNum, IPCMDIlm, etc.) por si alguno no-cero se nos escapa.
3. **Data-abort handler (0x2e6950)**: desensamblarlo para ver si guarda el DFAR (fault address) en
   algún sitio del SMEM que podamos leer por devmem — daría la dirección exacta del fallo sin RE del
   parser. (El Exce estaba a 0, pero el handler puede escribir el contexto en OTRA región.)

## Recomendación
Antes de meter días en Ghidra, agotar (2) el barrido exhaustivo de los 70 ints del runtime vs source
y (3) el análisis del data-abort handler (podría dar la fault-addr barata). El RE completo del parser
es el plan B sólido si esos no cierran.

*2026-07-18, sesión Windows (Fable 5). Groundwork del RE hecho; handler del runtime en ~0x6148. Falta
el trace completo (Ghidra) o agotar las alternativas baratas primero.*
