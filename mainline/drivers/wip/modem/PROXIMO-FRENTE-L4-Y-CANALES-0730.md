# Después de M1: el frente real no es L1, es **L4** — y los canales de red (2026-07-30)

> Continúa de `M1-COMPLETO-NORMAL-BOOT-ID-0730.md`. Kernels #80-#81.
> Documento de arranque para la siguiente sesión.

---

## ★ El desbloqueo de esta sesión: seguir sirviendo pasado el hito

El bucle de servicio **salía en cuanto detectaba `NORMAL_BOOT_ID`**. Nos parábamos exactamente en el
hito, así que el MD se quedaba sin servicio FS justo al continuar el arranque — y su registro de
excepción salía limpio porque **no le dábamos tiempo a llegar a nada**.

Parámetro nuevo (H9a), en caliente:
```sh
echo 20000 > /sys/module/mt6582_spm/parameters/spm_fs_post_hs2_iters   # 0 = parar en el hito
```

Efecto inmediato: **de 900 a 1626 operaciones FS** y el MD entra en una fase que nunca habíamos
visto. `0` conserva el comportamiento histórico, así que la medida de M1 queda intacta.

## Corrección: L1 ya estaba superado

La nota de "siguiente frontera = `tarea='L1'`, `m12100.c:9064`" **está obsoleta**: ese assert lo cerró
`MISC_32K_LESS` en el `support_mask` (commit `26efd89`). El bloqueo medido hoy es otro y está una capa
antes de la radio.

---

## 📍 EL BLOQUEO ACTUAL (reproducible, medido dos veces)

```
H6 FS: 1626 ops servidos          (el fsd real hace 883 en el arranque entero)
*** HS2 LOGRADO: NORMAL_BOOT_ID ***
H7p EXCEPCION del MD: tipo=5 [ASSERT]  tarea='L4'
H7p   ASSERT en nvram_io.c:1202  p=0000000a 00000003 000000a1
H4 HS2: fin del bucle. RCHNUM=00000000 BUSY=00000001
```

**`L4` = la tarea de NVRAM del propio MD**, una capa por encima de nuestro proxy FS. No es radio.

### El assert, desensamblado

Cadena `service/nvram/src/nvram_io.c` en `0x47b330`; punteros a ella en `0x28ddc4`…`0x28e634`; el
`MOVW #1202` de `0x28e3c4` cae justo en ese rango → es el assert.

```asm
28e39e:  bl    0x28dc8c        ; -> r0/r5 = codigo de error
28e3a4:  cmp   r0, #0
28e3a6:  beq   0x28e48a        ; ==0 -> OK, se salta el assert
28e3a8:  bl    0x29001c        ; traduccion del codigo (si devuelve !=0)
28e3b8:  movs  r2, #3          ; param 2 = 3   (constante)
28e3be:  ldrh  r2, [r4, #0]    ; param 3 = halfword en [r4] = 0xa1 = 161
28e3c0:  ldr   r1, [pc,#240]   ; "service/nvram/src/nvram_io.c"
28e3c4:  movw  r2, #1202
28e3c8:  blx   0x3f9608        ; handler de excepcion
```

Los tres params son, en orden: **error = 10**, **3** (constante del sitio), **161** = el identificador
de registro que estaba procesando. El enum es `NVRAM_IO_ERRNO_*` (confirmado por la cadena
`read_cnf->result == NVRAM_IO_ERRNO_OK || read_cnf->result == NVRAM_IO_ERRNO_INIT` en `0x4767b0`),
pero **en el firmware solo hay strings de `_OK` y `_INIT`**, así que el 10 hay que sacarlo del
desensamblado de `0x28dc8c` — ese es el siguiente paso de RE.

---

## ✅ Lo que NO es el problema (medido — no volver a pisarlo)

### 1. El op nuevo `0x1021` — REFUTADO con barrido
Aparece solo en esta fase post-HS2 y no existe en el ground-truth. Barrido de su contador de campos
con `spm_fs_ov_op=0x1021` / `spm_fs_ov_nf`:

| `ov_nf` | Resultado |
|---|---|
| 1 (= el default genérico) | 1626 ops, HS2, assert L4 `nvram_io.c:1202` |
| 3 | 1612 ops, HS2, assert `fs_ccci.c:528` código −4009, op `0x1021`, valor 3 |

→ **1 campo es lo correcto y ya lo damos**. El `0x1021` está bien servido; el assert de L4 es idéntico
con y sin tocarlo.

### 2. El contenido de `X:` e `Y:` — REFUTADO comparando con el golden
El `strace` del `ccci_fsd` real revela el mapeo de volúmenes, que **no** es lo que suponíamos:

| Volumen | Ruta real del fsd |
|---|---|
| `Z:` | `/data/nvram/md/` |
| `X:` | **`/protect_f/md/`** |
| `Y:` | **`/protect_s/md/`** |

Son las dos **particiones protegidas** (el par espejo de registros de seguridad), no subdirectorios.
Nuestro fix H8f las traduce a `/data/nvram/md/X|Y/`, y el MD se creó allí los tres ficheros de cero.

Parecía la causa evidente… y **no lo es**: los ficheros que generó el MD son **byte a byte idénticos**
a los del backup golden (`~/golden/protect_{f,s}.img`, montables por loop):

```
239808745fdb8433320c63b8f0a8208f  MP0D_000   (4 B)     X: / protect_f
aee4e34a7996e2fedc8d1f28087b5c31  ST33A004   (2060 B)  X: / protect_f
aee4e34a7996e2fedc8d1f28087b5c31  ST33B004   (2060 B)  Y: / protect_s
```

El MD reconstruye esos registros exactamente igual que la fábrica. (Aun así **conviene mover el mapeo
a las rutas reales** por fidelidad; es cosmético, no desbloquea.)

### 3. El almacén de NVRAM — verificado fiel
El árbol del móvil es **idéntico en estructura y conteos** a la NVRAM extraída del Lineage real
(`~/modem-fsd/nvram/md`):

| Directorio | Ficheros |
|---|---|
| `NVRAM/CALIBRAT` | 105 |
| `NVRAM/IMPORTNT` | **0** (también vacío en el original) |
| `NVRAM/NVD_CORE` | 2 |
| `NVRAM/NVD_DATA` | 55 |
| `NVRAM/NVD_IMEI` | 3 |

No falta ningún fichero. El assert no es un "no encontrado".

---

## 🔥 LA HIPÓTESIS PRINCIPAL: el MD espera al AP en canales que solo miramos

`BUSY=00000001` al salir del bucle: el canal TX del MD **sigue ocupado**, es decir, el MD mandó algo y
espera el ACK del AP. Y en esta fase aparecen canales que antes no salían. Con los nombres de
`dual_ccci/include/ccci_ch.h`:

| `lch` | Canal | `rsv` |
|---|---|---|
| 0 | `CCCI_CONTROL_RX` | `0x5555ffff` = HS1 · `0` = HS2 · **`0x45584350` = `"EXCP"`** |
| 4 | `CCCI_PCM_RX` (voz) | 0 |
| 23 (`0x17`) | **`CCCI_CCMNI1_TX_ACK`** | 2 |
| 27 (`0x1b`) | **`CCCI_CCMNI2_TX_ACK`** | 3 |
| 31 (`0x1f`) | **`CCCI_CCMNI3_TX_ACK`** | 4 |

Tras el HS2 el MD está levantando sus **tres interfaces de red** (CCMNI1/2/3, una por contexto PDP) y
el canal de **PCM de voz**. `rsv` = el índice de interfaz (2/3/4). Nosotros los logueamos y ACKeamos,
pero **no contestamos nada** — el AP del stock sí tiene manejadores para ellos.

Encaja con H8e: declaramos y zeramos 6 regiones NetUL/DLCtrl porque el MD las exige, pero nunca las
usamos. Ahora las quiere de verdad.

---

## ▶️ SIGUIENTE PASO, en orden de coste

1. ~~RE de `0x28dc8c`~~ **HECHO** — ver la sección siguiente. El paso 1 pasa a ser **RE de
   `0x290398`** (la validación de UNA copia): qué comprueba y de dónde sale la segunda copia.
2. **Auditar el ACK del bucle**: confirmar que ACKeamos **todos** los canales que el MD dispara, no
   solo los que leemos de `RCHNUM`. `BUSY=1` al salir dice que algo queda sin ACKear.
3. **Localizar el registro 161**: correlacionar el assert con el último fichero que el MD toca.
   ⚠️ `spm_fs_quiet=0` cuesta ~30 ms/op → con 1626 ops hace falta subir `spm_fs_slow_iters` a ~6000
   (150 s de presupuesto). El intento de esta sesión con esos ajustes salió sin arrancar (ver abajo).
4. **Contestar en los canales CCMNI/PCM**: portar del stock lo mínimo para que el MD no se quede
   esperando. Aquí ya se entra en el data-path, que es territorio de RIL.

L1 (radio: calibración RF, rails/PLLs del frontend) sigue **detrás** de todo esto.

---

## ⚠️ Fiabilidad: el reintento en caliente falla ~la mitad de las veces

En el barrido, **3 de 5 ciclos dieron 0 operaciones y ningún HS2**. No puede ser el parámetro que se
barría (solo afecta a un op tardío, y sin FS no se llega ni al primero) → es el
`poweroff → poweron → load → remap → release → hs2` que **no reinicia limpio el MD una vez ha
petado**. Consecuencia práctica: **repetir cada medida hasta obtener HS2** antes de darla por válida,
y no interpretar un "0 ops" como resultado del experimento.

## 🎛️ Cómo reproducir

```sh
S=/sys/module/mt6582_spm/parameters
echo 1     > $S/spm_fs_quiet             # 0 = logs con ruta (~30ms/op)
echo 800   > $S/spm_fs_slow_iters        # 20s de presupuesto (x25ms)
echo 20000 > $S/spm_fs_post_hs2_iters    # H9a: seguir tras el HS2 (0 = parar)
echo 1 > $S/spm_md_smem_clr              # SIEMPRE antes de medir
echo 1 > $S/spm_md_poweroff; sleep 1; echo 1 > $S/spm_md_poweron; sleep 1
for p in spm_md_load spm_md_remap spm_md_release spm_md_hs2; do echo 1 > $S/$p; done
sleep 26
echo 1 > $S/spm_md_ex                    # decodifica el registro de excepcion
```

`/root/mdrun.sh` en el móvil hace el ciclo con **defaults puros** (no escribe ningún ajuste).
⚠️ **`mdtest.sh` no**: pone `spm_fs_1010_mode=0` y pisa el default bueno (9).

## Artefactos

| Qué | Ruta |
|---|---|
| Imagen actual (#81) | `~/mainline/pkg/boot-modem-m1.img` (Pi `.123`) |
| Parche de esta sesión | `fixes-0730/patch_posths2.py` |
| Golden de las particiones protegidas | `~/golden/protect_{f,s}.img` (ext4, montables por loop) |
| Mapeo de volúmenes | `~/modem-fsd/boot-fsd.strace` (rutas reales del fsd) |
| Nombres de canales CCCI | `~/mainline/downstream/bq-src/mediatek/kernel/drivers/dual_ccci/include/ccci_ch.h` |
| Firmware a desensamblar | `~/mainline/downstream/stock-firmware-0713/modem.img` (5.172.580 B) |


---

# H9b — `0x28dc8c` desensamblado: **error 10 = fallan las DOS copias**

Es la lectura de un registro de NVRAM **con doble copia y auto-reparación**:

```asm
28dc8c:  stmdb sp!, {r4-r9, sl, lr}
28dc90:  ldrh  sl, [r0, #16]      ; flags del descriptor del registro
28dcaa:  and   sl, #4             ; bit 2 = "este registro tiene 2a copia"
28dcce:  bl    0x290398           ; -> r5 = validacion de la copia A
28dcd8:  beq   0x28dd92           ; sin bit 2 -> una sola copia y fuera
28dd4c:  bl    0x290398           ; -> r7 = validacion de la copia B
28dd70:  cbnz  r5, 0x28dd76       ; A mal?
28dd72:  cbnz  r7, 0x28dd84       ;   no: B mal?
28dd74:  b     0x28ddb6           ;   las dos bien -> exito (devuelve 0)
28dd76:  cbnz  r7, 0x28ddba       ; A mal Y B mal ----------> 10
28dd7c:  bl    0x290170           ; A mal, B bien -> reconstruir A desde B
28dd88:  bl    0x290170           ; A bien, B mal -> reconstruir B desde A
28dd8e:  cbnz  r0, 0x28dd96       ;   si la reparacion falla -> 10
28ddba:  movs  r5, #10            ; <-- EL ERROR 10
28ddc0:  ldmia sp!, {r4-r9, sl, pc}
```

| Dirección | Qué es |
|---|---|
| `0x28dc8c` | leer registro con doble copia + auto-reparación (devuelve 0 = OK, 10 = ambas mal) |
| `0x290398` | **validar UNA copia** — el siguiente objetivo de RE |
| `0x290170` | reconstruir la copia mala desde la buena |
| `0x29001c` | accesor usado en los logs (líneas 1391/1404/1421/1451 del mismo fichero) |

Pool del assert: `0x28e4b0` → `"KAL_FALSE"` ⇒ es `EXT_ASSERT(KAL_FALSE, err=10, 3, lid)`, una rama de
error incondicional. El `3` es el identificador del sitio, no un dato.

## El registro 161 = `CALIBRAT/MPA8_000`

**Tabla de items de NVRAM localizada**: `0x4d41ca`–`0x4d566a`, entradas de **32 bytes**:

```
+0x00  nombre[4] + NUL      "MPA8\0"
+0x05  version[3] + NUL     "000\0"
+0x0e  u16 LID              0xa1 = 161
+0x10..+0x1f                otros campos (tamaño/flags/puntero) NO cuadrados aún
```

Ejemplo real (`0x4d55aa`):
```
4d 50 41 38 00 30 30 30 00 00 00 00 00 00 a1 00
0a 00 0a 00 00 00 c3 06 45 00 00 20 00 00 0e 00   |MPA8.000................E.. ....|
```

**El detalle del espejo sale de aquí**: la tabla guarda nombres de **4** caracteres mientras en disco
son de **8**, porque el 5º carácter es **`A`/`B` en los registros con copia y `_` en los que no**:
`MT00`→`MT00A000`+`MT00B000`, `ST6T`→`ST6TA001`+`ST6TB001`, `MPA8`→`MPA8_000`. Encaja con el bit 2.

⚠️ **Confianza**: el mapeo LID→nombre se apoya en que los LIDs son **consecutivos en entradas
consecutivas** (101, 161, 162, 163) y en que los nombres casan con ficheros reales (`MP0B_001` está en
`NVD_IMEI`, `MPA2_000` y `MPA8_000` en `CALIBRAT`). Los campos de tamaño **no** se han podido cuadrar
(la fila de `MPA2` contiene `0x548` = 1352, que es exactamente el payload de `MPA8_000` — o hay un
desfase de una entrada en mi agrupación de campos, o es otro campo). **Confirmarlo empíricamente**
(traza con `spm_fs_quiet=0`) antes de construir encima.

## ❌ Y el contenido NO es la causa (refutado en el sitio)

`MPA8_000` son **1352 ceros + `ef cd`** (marca de fin `0xCDEF`), idéntico byte a byte al de la
extracción del Lineage que funciona. Parecía la explicación… y no lo es: **57 de los 165 registros
están igual de vacíos**, con la misma marca, en la NVRAM del móvil que sí arranca la radio.

```
CALIBRAT/HL11_000..HL1J_000   (18 registros)   CALIBRAT/UL12_000..ULB9_000  (14)
CALIBRAT/MPA2_000  314 B      CALIBRAT/MPA8_000 1354 B     NVD_DATA/MT04_000 4234 B  ...
```

Un registro en blanco con su marca es **lo normal** para lo no calibrado.

## ▶️ Conclusión: el problema está en cómo servimos la SEGUNDA copia

Si las dos copias fallan con un fichero que es correcto y que funciona en el original, el fallo es de
**nuestra respuesta**, no del dato. Siguiente RE: **`0x290398`** — qué valida exactamente y **de dónde
sale la copia B** (otro fichero, otro offset del mismo, u otra región). Ahí está el desajuste.
