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

**Conviene poner `spm_fs_1010_mode = 9` como valor por defecto en el driver**, porque con el default
actual (0) el arranque se queda en 2 operaciones.

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
