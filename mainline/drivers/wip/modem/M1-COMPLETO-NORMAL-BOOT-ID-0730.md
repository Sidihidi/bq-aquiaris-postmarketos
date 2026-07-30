# 🏆 HITO M1 COMPLETO — el MD arranca: `NORMAL_BOOT_ID` (2026-07-30)

> Cierra la campaña H0→H7 del módem. Continúa de `ESTADO-Y-TRASPASO-0729.md`.
> Kernel **#66** (trabajo de casa, H7a→H7r) + `spm_fs_1010_mode=9`.

---

## 🎯 EL RESULTADO

```
*** HS2 LOGRADO: NORMAL_BOOT_ID (stage 2 = M1 COMPLETO) ***
```

El MD (procesador de radio) **arranca entero**: carga MOLY, hace HS1, acepta el runtime, **monta su
NVRAM completa** y alcanza **HS2 = `NORMAL_BOOT_ID`**, que es la definición del hito M1
(arranque del MD, sin RIL, con secure-boot OFF).

**476 respuestas del proxy FS** en un ciclo, con lecturas de hasta 3100 bytes y **escrituras** de
vuelta a la NVRAM (`ffff1004`). El MD abre, busca, lee, escribe y cierra decenas de ficheros reales
(`NVD_DATA\MT18_00`, `MT0Y_02`, `MT6K_04`…).

### Reproducibilidad verificada
| Prueba | HS2 | Respuestas FS |
|---|---|---|
| Intento 1 | ✅ | 476 |
| Intento 2 | ✅ | 476 |
| Control: con `FAT3149C88D.log` presente | ✅ | 476 |

**Nota honesta**: se sospechó que el `FAT3149C88D.log` residual (creado en una prueba antigua)
bloqueaba el arranque, porque hacía que el `1010` respondiera FOUND. **El control lo refuta**: el
resultado es idéntico con y sin él. Lo único que faltaba era el parámetro.

---

## La configuración que arranca

```sh
S=/sys/module/mt6582_spm/parameters
echo 9          > $S/spm_fs_1010_mode   # ← LA CLAVE (el default 0 solo da 2 ops)
echo 0xffffffff > $S/spm_fs_1010_val
echo 1          > $S/spm_fs_enum
# ciclo
echo 1 > $S/spm_md_smem_clr
echo 1 > $S/spm_md_poweroff; sleep 1; echo 1 > $S/spm_md_poweron; sleep 1
for p in spm_md_load spm_md_remap spm_md_release spm_md_hs2; do echo 1 > $S/$p; done
```

### ✅ Ya es el comportamiento por defecto (kernel #67)
`spm_fs_1010_mode = 9` está fijado en el driver. **El MD arranca sin tocar ningún parámetro**:
```
kernel #67 — defaults puros
  spm_fs_1010_mode=9  spm_fs_1010_val=0  spm_fs_enum=1
  respuestas FS: 894   HS2: 1
  *** HS2 LOGRADO: NORMAL_BOOT_ID (stage 2 = M1 COMPLETO) ***
```

### El `val` por defecto (0) es además el CORRECTO
| `spm_fs_1010_val` | HS2 | Peticiones FS |
|---|---|---|
| **0** (default) | ✅ | **894** |
| `0xffffffff` | ✅ | 476 |

El `ccci_fsd` real hace **883** peticiones → con `val=0` hacemos 894, prácticamente el mismo mount.
Con `0xffffffff` se alcanza HS2 igual pero **con la mitad de operaciones**: el MD llega al hito con
parte de su NVRAM sin leer. O sea que el default no solo arranca, **reproduce el arranque de
fábrica**. Solo hubo que cambiar el modo.

---

## Lo que aportó cada tramo

### Sesión del 29 (H6p→H7c) — el protocolo
- El MD **sí excepcionaba** (no esperaba): se leyó su registro en la SMEM desde el driver
  (`/dev/mem` está capado por `STRICT_DEVMEM`).
- Desensamblando `fs_ccci.c` del firmware salieron las **tres comprobaciones** del MD:
  ```
  línea 520:  buffer[+0] == 0xffff0000|op     (-4003)
  línea 528:  buffer[+4] == nfields           (-4009)
  línea 547:  capacidad >= len  por campo     (-4010)
  ```
- **La regla central**: el buffer es una **lista de campos con longitud** y `+4` es el **número de
  campos de la RESPUESTA** — distinto por op, no el `c` de la petición.
- Bugs: marca de error con bit31 (inválida), OPEN de directorios con `O_RDWR`, ops sin handler
  dejando la longitud de la petición, `1009` mal formado.
- De 2 → 32 operaciones.

### Sesión de casa (H7d→H7r) — la semántica
- **H7j/H7l**: enumeración real de la NVRAM (FindFirst/FindNext) con tabla de 96 rutas. **El orden
  importa**: los `NVD_*` primero y alfabético (`NVD_CORE < NVD_DATA`); empezar por `CALIBRAT` hacía
  excepcionar al MD.
- **H7k**: la enumeración **solo** vale para consultas de `/NVRAM`. El primer `1010` pregunta por
  `Z:\FAT….log` y hay que contestar NO ENCONTRADO; devolverle una ruta de la NVRAM lo mata al instante.
- **H7m/H7o**: decodificación del ground-truth **palabra por palabra** en vez de deducir el formato.
  El `0x54` del GetDrive **no era un escalar**: es un segundo campo de 84 bytes a cero (info de
  unidad). El FindFirst responde 2 campos: path de 54 bytes + atributos `0x700`.
- **H7p**: **decodificador del registro de excepción** con el formato real `EX_LOG_T` de `ccci_md.h`
  del bq-src → tipo, serie, **nombre de la tarea en texto**, y `ASSERT fichero:línea + 3 params`.
  Herramienta decisiva: convierte cada fallo en un diagnóstico legible.
- **H7r**: buffers TTY del CCCI (`shared_mem_tty_t` de `ccci_tty.h`).

---

## ▶️ LA SIGUIENTE FRONTERA: L1 (radio)

Tras el hito, el MD sigue arrancando y excepciona en otro sitio completamente distinto:

```
H7p EXCEPCION del MD: tipo=5 [ASSERT] nvram=0 serie=0
H7p   tarea='L1' boot_mode=1
H7p   ASSERT en m12100.c:9064  p=00000000 00000000 00000000
```

**`tarea='L1'`** = la capa física de radio. Ya no es el sistema de ficheros: el MD ha arrancado y
ahora falla inicializando la radio. Es la frontera natural, porque L1 necesita:
- calibración RF real (los `NVD_*` de `CALIBRAT` que ya servimos, pero quizá con contenido que no
  le vale, o incompleto),
- el frontend de RF encendido (rails, relojes, PLLs) — nada de eso lo toca todavía el driver,
- posiblemente más regiones de memoria compartida (de ahí el trabajo de H7r con las TTY).

Para retomar: desensamblar `m12100.c:9064` en el firmware con el mismo método que funcionó con
`fs_ccci.c` (buscar la cadena del fichero, localizar los punteros del literal pool, encontrar los
`LDR` PC-relativos, `objdump -b binary -M force-thumb`).

**Ojo con el alcance**: L1 funcionando implica RF, y eso es un proyecto aparte del arranque del MD.
El hito M1 tal como estaba definido —`NORMAL_BOOT_ID`, sin RIL— **está cerrado**.

---

## Artefactos

| Qué | Ruta (Pi `.123`) |
|---|---|
| Imagen que arranca el MD | `~/mainline/pkg/boot-modem-h7r.img` (kernel #66) |
| Driver | `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c` |
| Firmware del MD | `~/mainline/downstream/stock-firmware-0713/modem.img` (5.172.580 B) |
| Ground-truth del fsd | `~/modem-fsd/gd-boot-full.out` |
| Cabeceras del stock | `~/mainline/downstream/bq-src/mediatek/kernel/drivers/dual_ccci/` |

Flasheo por `dd` (sin BROM), verificando siempre releyendo antes de reiniciar — ver
`ESTADO-Y-TRASPASO-0729.md`.

---

# ✅ ACTUALIZACIÓN — L1 SUPERADO (kernel #71)

El assert de L1 **no era RF**: era **una característica sin declarar** en el `support_mask` del
`misc_info`.

## El diagnóstico (desensamblado)

El assert de `m12100.c:9064` llama a una función en `0x6148` con `(r0=1, r1=0, r2=0)` y exige que
devuelva **1 o 2**:
```asm
6158:  ldr   r1, [pc,#56]    ; r1 = runtime[+0x108] = MiscInfoBase   (offset 264 = índice 66 ✓)
6160:  ldr   r5, [r1, #0]
6162:  cmp   r5, r4          ; ¿*(MiscInfoBase) == 'CCIF'?   -> el prefijo SÍ lo escribíamos
6164:  bne   0x618e          ;    no -> return 0
6166:  ldr   r5, [r1, #4]    ; support_mask
6168:  lsls  r4, r0, #1      ; r0*2 = 2
616a:  lsrs  r4, r5, r4      ; support_mask >> 2
616e:  and   r4, r4, #3      ; bits [3:2]  ← el valor devuelto
```
El `support_mask` es un **mapa de 2 bits por característica** (`ccci_common.h`):
```
estado : NOT_EXIST=0  NOT_SUPPORT=1  SUPPORT=2  PARTIALLY_SUPPORT=3
feature: MISC_DMA_ADDR=0  MISC_32K_LESS=1  MISC_RAND_SEED=2  MD_COCLK=3
```
Escribíamos `support_mask = 0x1`, así que **`MISC_32K_LESS` quedaba en `NOT_EXIST` (00)** → la función
devolvía 0 → assert. El MD **necesita saber** si hay cristal de 32 kHz; "no me consta" no le vale.

## El fix y la medición

Parámetro nuevo `spm_md_misc_mask` (default **0x5**). Barrido en HW:

| `support_mask` | Tarea que falla | Assert |
|---|---|---|
| `0x1` (anterior) | **L1** | `m12100.c:9064` |
| `0x5`, `0x9`, `0x6`, `0xa` | **UPS** | `ccci_uart_drv.c:2594` |

Los cuatro valores válidos se comportan igual → lo que importaba era **declarar** la característica,
no su valor concreto.

## Estado con defaults puros (kernel #71)
```
misc_mask=5  uart_ports=6  uart_len=16384  1010_mode=9
respuestas FS = 894      HS2 = 1
*** HS2 LOGRADO: NORMAL_BOOT_ID ***      tarea='UPS'  ASSERT en ccci_uart_drv.c:2594
```

## ▶️ Frontera nueva: tarea `UPS` / `ccci_uart_drv.c:2594`

Desensamblado del sitio (`0x22d2`–`0x230c`):
```asm
22e8:  subs r4, #37       ; el índice de puerto sale de un CANAL CCCI menos 37
22ea:  ldr  r3, [r7, #0]  ; máscara de puertos activos
22ec:  lsls r5, r4
22ee:  tst  r5, r3
22f0:  bne  0x231c        ; puerto activo -> OK
22fa:  ldr  r1, [r1, #4]  ; si NO: descriptor INTERNO del MD (tabla stride 88)
22fc:  ldr  r0, [r1, #0]  ; debe ser 0
2300:  ldr  r1, [r1, #8]  ; debe ser 0
2302:  cbnz r1, ASSERT
```

### Descartado por medición (no perder tiempo repitiéndolo)
| Prueba | Resultado |
|---|---|
| `rx/tx length` = 0 vs 16K | **sin efecto** — el descriptor es interno del MD, no nuestro bloque |
| `UartPortNum` = 7, 8 (tope subido de 6 a 8) | **sin efecto** |
| `UartPortNum` = 0 ó 1 | **regresión**: sin HS2, assert en `ccci_uart_drv.c:793` |

O sea: **6 puertos hacen falta para HS2**, y el problema no está en lo que escribimos en la memoria
compartida del TTY.

### Siguiente paso propuesto
El puntero que el MD valida (`tabla[puerto].campo_4`) es **interno suyo**, así que hay que averiguar
**quién lo rellena**: localizar la tabla del literal `0x2348` y buscar dónde se escribe su `campo_4`
durante el arranque. Con eso se sabrá qué dato del runtime alimenta ese descriptor. Método de siempre
(punteros al literal → `LDR` PC-relativos → `objdump -b binary -M force-thumb`).

**Nota de método**: se llegó al fix de L1 tras dos interpretaciones equivocadas —primero se supuso que
L1 implicaba entrar en RF, y luego que faltaba una región entera sin su `'CCIF'`—. Leer la función
**hasta el retorno** (no solo las diez primeras instrucciones) fue lo que lo aclaró.
