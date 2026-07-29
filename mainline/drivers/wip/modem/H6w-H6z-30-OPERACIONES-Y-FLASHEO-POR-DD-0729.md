# H6v→H6z — 30 operaciones, la cascada de lectura, y flasheo por dd (2026-07-29)

> Continúa de `H6r-H6t-PROTOCOLO-FS-DESCIFRADO-0729.md`. Kernels #43 → #48.

## 🎯 TL;DR

**De 13 a 30 operaciones**, y el MD entra por fin en la cascada de lectura real:

```
100e 1010 1001 1005 1010 1010 [1010 1007]×6 1001 1009 1002 1003 1005 1001 1009 1002 1003 1005 1012
                                             └──── OPEN·GetSize·seek·READ·close ────┘ ×2
```

**Está leyendo ficheros de su NVRAM de verdad.** Ese patrón (`1009`, `1003`, `1005`, `1001`, `1002`)
suma ~800 de las ~900 operaciones del arranque real de Lineage.

Y **el flasheo pasa a hacerse por `dd` sobre la eMMC viva**: ciclo completo (compilar → escribir →
verificar → reiniciar → probar) en **~4 minutos sin tocar el móvil**, frente a un ciclo de BROM.

---

## ★ La regla que faltaba: `+4` = número de campos de la RESPUESTA

No es un flag, ni el `c` de la petición. Es **cuántos campos lleva la respuesta**, y depende del op:

| Op | `c` de la petición | Campos en la respuesta | `+4` correcto |
|---|---|---|---|
| `100e` GetDrive | 2 | 2 | **2** ← coincidía por casualidad |
| `1010` GetFullPath | 1 | 1 | **1** |
| `1001` OPEN | 2 | 1 (el handle) | **1** ← ¡distinto del `c`! |
| `1009` GetSize | ? | 2 (handle + tamaño) | **2** |

Se descubrió al romperlo: poner "no tocar `+4`" por defecto (creyendo que había que conservar el `c`)
**rompió el OPEN**, que antes funcionaba escribiendo 1. La petición del OPEN trae `c=2` pero su
respuesta lleva un solo campo.

## Estructura completa del buffer (ya confirmada)

```
+0:  0xffff0000|op        marca "done" — SIEMPRE, sin excepciones
+4:  nfields              numero de campos de la RESPUESTA
+8:  len_0                longitud del campo 0
+c:  datos_0 ...          (alineado a 4)
     len_1  datos_1 ...   ... y asi nfields veces
```
El MD recorre los campos con `r9 += align4(len) + 4` y para cada uno exige **`capacidad >= len`**,
haciendo `memcpy` al destino que reservó al enviar la petición.

---

## Los cuatro bugs corregidos

### H6w-A — la marca de error con bit31 es inválida
Al fallar un OPEN escribíamos `fs_ops | 0x80000000`. La comprobación de la línea 520 exige
`0xffff0000|op` **siempre**. El registro lo dijo literal: *esperado `ffff1001`, recibido `80001001`*.
**Los errores van en el payload, no en la marca.**

### H6w-B — abríamos directorios con `O_RDWR`
El MD hace `OPEN "Z:\NVRAM"`, que es un directorio → `EISDIR`. Ahora hay reintento con
`O_RDONLY|O_DIRECTORY`. Estaba oculto detrás del bug A (el fallo se enmascaraba con la marca mala).

### H6y — respuesta mínima válida para ops sin handler
El `default:` no tocaba nada, así que en `+8` quedaba la longitud de la **petición** y el MD reventaba
en la línea 547. Como la regla es `capacidad >= longitud` y la capacidad mínima observada es 4,
**responder siempre 1 campo de 4 bytes es seguro para cualquier op**. Esto solo ya llevó de 7 a 21 ops.

### H6z — el `1009` (GetSize) mal formado
Declaraba **1** campo y escribía **2** (handle + tamaño), y ponía `len=8` para el tamaño cuando la
capacidad era **4** → el tamaño va en **32 bits**. Corregido: `+4=2`, `len_0=4` (handle),
`len_1=4` (tamaño), `length=0x14`. De 21 a 30 ops.

### H6v — el valor de retorno del `1010` SÍ importa
Barrido completo (kernel #43):

| Valor devuelto | Comportamiento |
|---|---|
| `0`, `1` | el MD recorre la NVRAM en círculo y muere en `0x1012` (13 ops) |
| **cualquiera con bit31** (`0xffffffff`, `0xfffff055…05d`) | **entra en la cascada** (30 ops) |

La respuesta honesta de "no encontrado" es lo que le hace avanzar. Todos los negativos se comportan
igual, así que basta con `0xffffffff`.

---

## ⚡ Flasheo por `dd` — se acabaron los ciclos de BROM

**Verificado antes de escribir**: el md5 de 20 MB leídos desde el sector 83968 de `/dev/mmcblk0`
coincidía **byte a byte** con la imagen flasheada por BROM. Es la partición boot, confirmado.

```sh
# 1) escribir por red, sin fichero intermedio en el movil
cat boot-modem-X.img | ssh root@<ip> "dd of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync; sync"

# 2) VERIFICAR releyendo (imprescindible antes de reiniciar)
ssh root@<ip> "dd if=/dev/mmcblk0 bs=512 skip=83968 count=40960 2>/dev/null | md5sum"

# 3) reiniciar (un 'reboot' normal NO reinicia esta pmOS -> sysrq)
ssh root@<ip> "sync; echo 1 > /proc/sys/kernel/sysrq; (sleep 1; echo b > /proc/sysrq-trigger) &"
```

- `mmcblk0` = eMMC interna; la raíz de pmOS vive en la SD (`mmcblk1p1`), así que no se toca.
- Sector 83968 × 512 = `0x2900000` = el mismo offset que usa `mtkclient wo 0x2900000`.
- Escritura + verificación: ~7 s cada una a 3 MB/s.
- **El DHCP cambia la IP en cada arranque** (`.108`…`.115`): localizar con barrido +
  `hostname` (`krillin-mainline`), nunca fiarse de la anterior.

⚠️ `abootimg --create` falla de forma **transitoria** dentro del script de build (deja el `.img` sin
crear); ejecutado a mano funciona. Comprobar siempre que la imagen existe antes de flashear.

---

## 📍 Estado y siguiente paso

Bloqueo actual: op **`0x1012`**, línea 528 (contador de campos), valor leído **1**. Es el mismo
patrón que ya hemos resuelto tres veces — hay que dar con el número de campos que espera. Como
`0x1012` no existe en el arranque real, lo más rápido es hacer **conmutable el número de campos de la
respuesta genérica** y barrer 0/1/2/3.

Ops que el MD ya recorre: `FAT3149C88D.log`, `NVRAM`, `IMPORTNT`, `NVD_IMEI`, `CALIBRAT`, `NVD_CORE`,
`NVD_DATA`, más dos ficheros leídos enteros.

## Parámetros en caliente (kernel #48)

```sh
S=/sys/module/mt6582_spm/parameters
echo 1          > $S/spm_md_smem_clr    # limpiar registro de excepcion (ANTES de cada ciclo)
echo 9          > $S/spm_fs_1010_mode   # forma corta (la buena)
echo 0xffffffff > $S/spm_fs_1010_val    # valor devuelto en el 1010 (acepta hex)
echo N          > $S/spm_fs_p0_mode     # +4: 0=el handler decide (correcto) 1=no tocar
echo 0          > $S/spm_fs_quiet       # logs detallados
echo XX         > $S/spm_md_smem        # visor hex de la SMEM (offset en HEX)
```

| Artefacto | Ruta |
|---|---|
| Imagen actual (#48) | `~/mainline/pkg/boot-modem-b.img` (md5 `730d7127…`) |
| Parches de esta tanda | `~/mainline/patch_{val,fix3,nf,def,1009,1009b}.py` |
| Script de prueba | `/root/run44.sh` en el móvil |
