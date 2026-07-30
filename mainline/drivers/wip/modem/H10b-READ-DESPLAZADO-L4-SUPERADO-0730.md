# H10b — L4 SUPERADO: el `READ` ponía los datos 4 bytes más allá

> Continúa de `H10-RE-0x290398-VALIDACION-DE-COPIA-0730.md`.
> Kernel **#82**, imagen `~/mainline/pkg/boot-modem-m1.img` (md5 `80ebd965618eccf7e1a86e641c7c1fa0`).

**El assert de L4 (`nvram_io.c:1202`, error 10) ha desaparecido.** El MD arranca ahora **sin ninguna
excepción** (`spm_md_ex` → `tipo=0 [INVALID]` tras 4 minutos de servicio), `BUSY` queda en **0** —
antes se quedaba en 1, señal de que el MD esperaba algo— y pasa a levantar sus canales de red y voz.

---

## La traza dirigida (H10a)

`spm_fs_quiet=0` cuesta ~30 ms/op y hay 1626 ops. En vez de eso se añadió `spm_fs_trace`
(por defecto `"MP0B"`): el driver recuerda el handle que devuelve el OPEN de un path que contenga esa
subcadena y loguea **sólo** sus ops, con flags, posición y los primeros bytes que devuelve el READ.

Lo que salió, y que descartó lo obvio:

```
H10a OPEN '/data/nvram/md/NVRAM/NVD_IMEI/MP0B_001' fl=00000700 oflag=2 -> h=1
H10a STAT h=1 -> size=120
H10a SEEK h=1 off=0 -> pos=0
H10a READ h=1 len=12 pos=12 -> 12 [54 5f 90 d0 e0 e1 65 4a aa 74 d3 ce ]
H10a CLOSE h=1
```

Es **exactamente** lo que hace el `fsd` real (`openat` `O_RDONLY`, `lseek(fd,0,SEEK_CUR)`,
`read(fd,…,12)`, `close`), el tamaño es el correcto y el fichero del móvil es **byte a byte idéntico**
al extraído del Lineage que funciona (`md5 88285878f00f146021a72adc80ee9327`). Es decir: ni el dato ni
la secuencia. El fallo tenía que estar en **cómo entregábamos los bytes**.

## La estructura del registro, del validador

`0x2901ec` empieza así:

```asm
2901f0  mov   r4, r3            ; r4 = tamaño de registro
2901fa  add.w r9, r4, #2        ; r9 = tamaño + 2      <- 2 bytes de checksum por registro
290208  mul.w r1, r9, r1        ; offset = (tam+2) * (indice-1)
```

`MP0B_001` son 120 bytes = **10 registros de 12** (10 de datos + 2 de checksum), y en el fichero los
12 bytes se repiten diez veces idénticos. El MD lee el primero para validarlo.

## El bug

El handler del `0x1003` declara 3 campos: `[4,resultado] [4,nread] [nread,DATOS]`. El MD camina el
buffer con `r9 += align4(len)+4`, así que **los datos del tercer campo empiezan en `+0x1c`** (su
longitud está en `+0x18`). Escribíamos:

```c
writel(nread, fs + boff + 0x18);              /* len_2 -> datos en +0x1c */
writel(0,     fs + boff + 0x1c);              /* <- un CERO donde van los datos */
memcpy_toio(  fs + boff + 0x20, tmp, nread);  /* y los datos 4 bytes mas alla */
length = 0x1c + nread;                        /* la longitud SI asumia +0x1c */
```

⇒ el MD leía `00 00 00 00 54 5f 90 d0 …` en lugar de `54 5f 90 d0 …`, y además se perdían los últimos
4 bytes, porque la longitud anunciada ya contaba desde `+0x1c`. El ground-truth lo confirma:

```
ffff1003 | 00000003 | 00000004 00000000 | 00000004 0000035a | 0000035a 1234abcd 01010201 ...
                                                              ^len_2   ^datos en +0x1c
```

**Fix**: `memcpy_toio(fs + boff + 0x1c, tmp, nread)` y fuera el `writel(0, …+0x1c)`.

### Por qué llevaba tanto tiempo escondido

Porque **casi nada del arranque comprueba el contenido**: la cascada entera de ficheros (1626 ops)
funcionaba igual con los datos desplazados. Lo destapa L4, que es la primera capa que **valida** lo
que lee — de ahí que el síntoma apareciera tan tarde y tan lejos del sitio del error.

Corolario para el futuro: **un ops-count creciente no es señal de que los datos sean correctos.**

## El resultado

| | antes (#81) | ahora (#82) |
|---|---|---|
| Registros de `MP0B_001` leídos | el 1 dos veces, y a morir | el 1 (offset 0) y el 2 (offset 12) |
| Excepción | `ASSERT nvram_io.c:1202` err 10, tarea `L4` | **ninguna** (`tipo=0 INVALID`) |
| `BUSY` al final del bucle | `00000001` | **`00000000`** |
| Cola de ops | bucle de recuperación | ciclo limpio `1001 1009 1002 1003 1005` |

Y aparecen los canales que el MD levanta después:

| `lch` | canal | `rsv` |
|---|---|---|
| 4 | `CCCI_PCM_RX` (voz) | — (llega con `id=af700000`) |
| 23 `0x17` | `CCCI_CCMNI1_TX_ACK` | 2 |
| 27 `0x1b` | `CCCI_CCMNI2_TX_ACK` | 3 |
| 31 `0x1f` | `CCCI_CCMNI3_TX_ACK` | 4 |
| 10 `0x0a` | — | 1 |
| 38 `0x26` | — | 5 |

## ▶️ Siguiente

Ya no hay excepción que perseguir: el frente pasa a ser **contestar en esos canales** (PCM y
CCMNI1/2/3), que es el data-path y territorio de RIL. Los `lch` 10 y 38 son nuevos y conviene
identificarlos en `dual_ccci/include/ccci_ch.h` antes de nada.

Conviene además **revisar los demás handlers con el mismo criterio** (dónde caen los datos de cada
campo según `align4(len)+4`), ahora que se sabe que un desplazamiento así puede pasar inadvertido
1600 operaciones.
