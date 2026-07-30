# H10 — RE de `0x290398`: validar UNA copia, y dos correcciones al diagnóstico de L4

> Continúa de `PROXIMO-FRENTE-L4-Y-CANALES-0730.md`, cuyo paso 1 era exactamente este RE.
> Todo lo de aquí es análisis estático del firmware + contraste con el `strace` del `fsd` real y
> con la NVRAM en disco. No ha hecho falta el móvil.

---

## Resumen: el frente cambia de sitio

El paso 1 preguntaba dos cosas: **qué comprueba** `0x290398` de una copia y **de dónde sale la
copia B**. La primera está resuelta abajo. La segunda **resulta no aplicar a nuestro caso**, y por
dos motivos independientes que se refuerzan:

1. **La tabla de items estaba desalineada 18 bytes** ⇒ el registro **161 no es `CALIBRAT/MPA8_000`
   sino `NVD_IMEI/MP0B_001`** (el registro del IMEI). `MPA8` es el LID **101**.
2. **El error 10 no significa sólo "fallan las DOS copias"**: la rama de **copia única** también
   devuelve 10. Y `MP0B_001` no tiene espejo.

⇒ **El MD no consigue validar la ÚNICA copia de `Z:\NVRAM\NVD_IMEI\MP0B_001`.** No hay copia B que
buscar; el desajuste está en cómo servimos ese fichero.

---

## `0x290398(nombre, carpeta, …)` — abrir, validar, cerrar

```
2903a6  ldr r0,[0xf0b65a74]; bl 0x28ffde   ; tomar contexto/handles -> 0  => err 3   (linea 1125)
2903c2  bl 0x28f934(carpeta, nombre, &buf) ; sprintf(buf, "%s\%s", carpeta, nombre)
2903cc  bl 0x2e9c80(&buf, #0x700)          ; OPEN, flags 0x700
2903d0  cmn r0,#9   -> r0 == -9            ;                          => err 5   (linea 1137)
2903e4  cmp r0,#0   -> r0 <= 0             ;                          => err 6   (linea 1143)
2903f6  bl 0x2e9da6(handle, &pos)          ; posicion/tamaño
2903fa  if (r0 == 0 && pos == 0)           ;                          => err 5   (linea 1152)
290426  bl 0x2901ec(handle, carpeta, …)    ; <-- LA VALIDACION DEL CONTENIDO (7 args)
290458  bl 0x2e9cec(handle)                ; CLOSE
```

Devuelve 0 = copia buena. Cualquier otro valor la marca como mala. El `0xf0b65aa4` es la global
"última línea" que el firmware deja escrita para el log; `0xf0b65aa0` guarda el código.

**Lo que hace el `fsd` real con ese fichero** (`boot-fsd.strace`, 4 aperturas):

```
openat(…"/data/nvram/md/NVRAM/NVD_IMEI/MP0B_001", O_RDONLY|O_LARGEFILE) = 7
lseek(7, 0, SEEK_CUR) = 0
read(7, …, 12) = 12          <-- la validacion empieza leyendo una CABECERA DE 12 BYTES
```

Es decir: el `0x700` del OPEN se traduce a **sólo lectura**, el `0x2e9da6` a un **`lseek(…,SEEK_CUR)`**
(posición, no tamaño) y `0x2901ec` empieza por leer 12 bytes. `MP0B_001` son 120 bytes = 12 de
cabecera + 108.

## `0x28f934` — la tabla de carpetas (`nvram_multi_folder.c`)

`tbb` de 7 casos; cada uno hace `sprintf(dest, "%s\%s", carpeta, nombre)`:

| índice | carpeta |
|---|---|
| 0 | `Z:\NVRAM\NVD_DATA` |
| 1 | `Z:\NVRAM\NVD_CORE` |
| 2 | `Z:\NVRAM\CALIBRAT` |
| 3 | `Z:\NVRAM\NVD_IMEI` |
| 4 | `X:\` |
| 5 | `Y:\` |
| 6 | `Z:\NVRAM\IMPORTNT` |

Fuera de rango ⇒ assert en `nvram_multi_folder.c:495`.

## `0x28f898(attr, esCopiaA)` — qué bit elige qué carpeta

```
bit 0   -> 1  NVD_CORE
bit 12  -> 2  CALIBRAT
bit 13  -> 3  NVD_IMEI
bit 14  -> 4 (X:\) si es copia A,  5 (Y:\) si es copia B     <-- el UNICO caso que separa volumenes
bit 15  -> 4 (X:\)   las dos copias
ninguno -> 0  NVD_DATA
```

**Verificado 3/3 contra el disco**: `MT00` attr `0x401` (bit 0) está en `NVD_CORE`; `MPA8` attr
`0x1000` (bit 12) en `CALIBRAT`; `MP0B` attr `0x2000` (bit 13) en `NVD_IMEI`.

## `0x28ff50(desc, buf, esCopiaA)` — el nombre

```
28ff62  ldrh r1,[desc+16]      ; flags
28ff64  tst  r1, #4            ; bit 2 = tiene 2a copia
28ff68  r1 = desc+18           ; base del nombre (4 chars)
28ff6c  r3 = desc+23           ; version (3 chars)
28ff74  'A'   si copia A       28ff78  'B'  si copia B      28ff7c  '_'  si NO hay 2a copia
```

Nombre = `base(4) + {A|B|_} + version(3)`, exactamente los 8 caracteres del disco.

---

## ★ Corrección 1 — la tabla de items estaba desalineada 18 bytes

El desensamblado dice que en el descriptor el **nombre está en `+18`** y la **versión en `+23`**, así
que la estructura **empieza 18 bytes antes del nombre**, no en él. Con eso los campos cuadran:

| offset | campo | comprobación |
|---|---|---|
| `+0` | **LID** (u16) | `MPA2`=100, `MPA8`=101, `MP0B`=161, consecutivos |
| `+4` | tamaño del payload | `MPA8` → `0x548` = **1352**, que es exactamente su payload |
| `+12` | attr (bits de carpeta) | verificado 3/3 arriba |
| `+16` | flags (u16) | contiene el bit de espejo |
| `+18` | nombre (4) | |
| `+23` | versión (3) | |

La tabla empieza en **`0x4d41b8`** (= `0x4d41ca − 18`) y tiene **165 entradas de 32 bytes**, que son
exactamente los **165 registros** de la NVRAM. Antes el tamaño de `MPA8` aparecía en la fila de
`MPA2` — ese era el síntoma del desfase, ya anotado como sospecha en el doc anterior.

⇒ **LID 161 = `MP0B` v`001`, attr `0x2000` = `Z:\NVRAM\NVD_IMEI`.** Es `MP0B_001`, 120 bytes: el
registro del **IMEI**. `MPA8_000` es el LID 101 y no tiene nada que ver con el assert.

## ★ Corrección 2 — el error 10 también sale de la rama de copia ÚNICA

```
28dd92  cbz r5, 0x28ddbc     ; copia unica: si la copia A valido bien -> exito
28dd96  ldrh r4,[r4,#0]      ; si no: LOG con el LID (linea 1451)
28ddba  movs r5, #10         ; <-- ERROR 10, tambien por aqui
```

O sea que `10` = "**no hay ninguna copia buena**", que con espejo son las dos y sin espejo es la
única. El doc anterior lo daba como "fallan las DOS copias", lo cual mandaba a buscar una copia B
que en este registro **no existe**.

**Cuántos registros tienen espejo de verdad**: en el árbol extraído del Lineage que funciona hay
**una sola** pareja `A`/`B` (`NVD_CORE/MT00A000` + `MT00B000`), y el `strace` del `fsd` real
confirma que en ejecución sólo se abren esos dos con letra; todo lo demás va con `_`. (`ST6TA001` /
`ST6TB001` existen en `NVD_IMEI` pero `ST6T` no aparece en esta tabla, así que pertenece a otro
espacio de LIDs.)

⚠️ **Cabo suelto honesto**: el bit 2 del `+16` de la tabla está puesto en 115 de los 161 registros
sin espejo, así que **el `+16` de la tabla no es el mismo campo que el `[desc+16]` de ejecución** —
el descriptor se construye en algún sitio que no he localizado. No afecta a la conclusión (el
`strace` y el disco ya fijan quién tiene espejo), pero conviene saber que ese bit no se puede leer
directamente de la tabla.

---

## ▶️ Siguiente paso (ahora mucho más estrecho)

Ya no hay que buscar una copia B: hay que ver **qué le contestamos mal a
`Z:\NVRAM\NVD_IMEI\MP0B_001`**. Los candidatos salen del propio `0x290398`:

1. **El OPEN con flags `0x700`** — el real lo abre `O_RDONLY`. Si nuestro handler lo interpreta de
   otra forma y devuelve `<= 0` o `-9`, la copia se marca mala sin llegar a leerse. (H8i ya tocó
   los flags del OPEN: leerlos del sitio correcto.)
2. **El `0x2e9da6`** (posición) — el real responde con un `lseek(fd, 0, SEEK_CUR)` = 0. La
   comprobación del firmware es `if (r0 == 0 && pos == 0) -> error`, así que **importa qué
   devolvemos en el registro de estado**, no sólo la posición.
3. **La lectura de la cabecera de 12 bytes** que hace `0x2901ec`.

Medición concreta: trazar en el driver **sólo** las ops cuyo path contenga `MP0B` (evita el coste
de `spm_fs_quiet=0` con 1626 ops) e imprimir op, flags, handle, posición y bytes devueltos. Eso
señala cuál de los tres puntos es.
