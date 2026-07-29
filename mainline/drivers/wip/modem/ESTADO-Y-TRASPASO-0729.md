# TRASPASO — Módem MT6582: 32 operaciones y el MD leyendo su NVRAM (2026-07-29)

> Documento de continuación. Resume TODA la sesión del 29-jul y deja el siguiente paso listo.
> Detalle por hitos en `H6p-H6q-*`, `H6r-H6t-*` y `H6w-H6z-*` (mismo directorio).

---

## 🏆 DÓNDE ESTAMOS

**De 2 a 32 operaciones FS servidas.** El MD arranca MOLY, hace el handshake, recorre su NVRAM y
**lee ficheros de verdad**, con dos ciclos completos de `OPEN → GetSize → seek → READ → close`:

```
100e 1010 1001 1005 1010 1010 [1010 1007]×6 1001 1009 1002 1003 1005
                                            1001 1009 1002 1003 1005 1012 1004
```

Ese patrón (`1009`/`1003`/`1005`/`1001`/`1002`) es ~800 de las ~900 operaciones del arranque real de
Lineage. Falta HS2 (`NORMAL_BOOT_ID`), que es el objetivo del hito M1.

**Bloqueo actual (una línea de código)**: el `1004` (WRITE) con `spm_fs_ov_nf=2` ya pasa el contador
de campos y muere en la comprobación de longitud — **esperado 4, devolvemos `0x358`** (856), que es
la longitud que traía la petición (el WRITE lleva datos). El override genérico no está fijando el
`+8` del segundo campo. Mismo patrón que ya se arregló en el `default`.

---

## ★ LA REGLA CENTRAL (lo que descifró todo)

El buffer de respuesta es una **lista de campos con longitud**:

```
+0:  0xffff0000|op        marca "done" — SIEMPRE, sin excepciones, sin bit31
+4:  nfields              NÚMERO DE CAMPOS DE LA RESPUESTA
+8:  len_0                longitud del campo 0
+c:  datos_0 ...          (alineado a 4)
     len_1  datos_1 ...   ... y así nfields veces
```

El MD recorre los campos con `r9 += align4(len) + 4` y para cada uno exige **`capacidad >= len`**,
haciendo `memcpy` al destino que reservó al enviar la petición.

### `nfields` es POR OP (medido uno a uno)

| Op | nfields | Cómo se supo |
|---|---|---|
| `100e` GetDrive | **2** | coincidía con el `c` de la petición → enmascaró el bug semanas |
| `1010` GetFullPath | **1** | |
| `1001` OPEN | **1** | ¡su petición trae `c=2`! |
| `1009` GetSize | **2** | handle + tamaño |
| `1007` | **1** | barrido: 0/2/3 fallan |
| `1012` | **3** | barrido: 0/1/2/4/5/6 fallan |
| `1004` WRITE | **2** | barrido: 0/1/3/4 fallan |

**No hay atajo**: cada op nuevo pide su propio contador y se descubre barriendo. Por eso existe el
override genérico (`spm_fs_ov_op` / `spm_fs_ov_nf`), para no añadir un parámetro por op.

### Las tres comprobaciones del MD (desensambladas de `fs_ccci.c`)

```
línea 520:  buffer[+0] == 0xffff0000|op        (código -4003)
línea 528:  buffer[+4] == nfields esperados    (código -4009)
línea 547:  capacidad >= len   por campo       (código -4010)
```

---

## Bugs corregidos hoy

| # | Bug | Efecto |
|---|---|---|
| H6w-A | La marca de error con **bit31** (`fs_ops\|0x80000000`) es inválida | el registro lo dijo literal: *esperado `ffff1001`, recibido `80001001`*. Los errores van en el **payload** |
| H6w-B | Abríamos **directorios** con `O_RDWR` → `EISDIR` | el MD hace `OPEN "Z:\NVRAM"`. Estaba oculto tras el bug A |
| H6y | Ops sin handler dejaban la longitud de la **petición** en `+8` | respuesta mínima genérica (1 campo, 4 bytes) → **de 7 a 21 ops** |
| H6z | `1009` declaraba 1 campo escribiendo 2, y `len=8` con capacidad 4 | tamaño en **32 bits** → **de 21 a 30 ops** |
| H7b | `1012` necesitaba 3 campos | **de 30 a 32 ops** |

### Y el valor devuelto en el `1010` importa
| Valor | Comportamiento |
|---|---|
| `0`, `1` | el MD recorre la NVRAM en círculo y muere en `0x1012` (13 ops) |
| **cualquiera con bit31** | **entra en la cascada de lectura** (30+ ops) |

Todos los negativos se comportan igual → basta `0xffffffff`. Es la respuesta honesta de "no
encontrado" lo que le hace avanzar.

---

## ⚡ FLASHEO POR DD — se acabaron los ciclos de BROM

**Verificado antes de escribir**: el md5 de 20 MB leídos del sector 83968 de `/dev/mmcblk0` coincidía
**byte a byte** con la imagen flasheada por BROM. Ciclo completo en **~4 min sin tocar el móvil**.

```sh
# 1) escribir por red (sin fichero intermedio en el movil)
cat boot-modem-X.img | ssh root@<ip> "dd of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync; sync"

# 2) VERIFICAR releyendo — imprescindible antes de reiniciar
ssh root@<ip> "dd if=/dev/mmcblk0 bs=512 skip=83968 count=40960 2>/dev/null | md5sum"

# 3) reiniciar (un 'reboot' normal NO reinicia esta pmOS -> sysrq)
ssh root@<ip> "sync; echo 1 > /proc/sys/kernel/sysrq; (sleep 1; echo b > /proc/sysrq-trigger) &"
```

- `mmcblk0` = eMMC interna. La raíz de pmOS vive en la **SD** (`mmcblk1p1`) → no se toca.
- Sector 83968 × 512 = `0x2900000` = el mismo offset que `mtkclient wo 0x2900000`.

### ⚠️ Trampas que costaron iteraciones (anotadas para no repetirlas)
1. **`abootimg` falla si se le redirige la salida a `/dev/null`** (deja el `.img` sin crear). Ejecutarlo
   con salida visible, y comprobar `[ -s img ]` antes de flashear.
2. **Esperar a `FIN` del build, no a que empiece el paso `[3]`** — si no, un `rm -f` posterior borra la
   imagen que el build acaba de escribir.
3. **El DHCP cambia la IP en cada arranque** (`.108`…`.115`): localizar con barrido +
   `hostname` (`krillin-mainline`), nunca fiarse de la anterior.
4. **Limpiar la SMEM antes de cada ciclo** (`spm_md_smem_clr`) o se comparan registros de excepción
   rancios — invalidó conclusiones enteras.
5. **El firmware que se carga es `stock-firmware-0713/modem.img` (5.172.580 B)**, NO
   `modem-h0/modem_1_wg_n.img` (5.100.832 B). Verificar contra el tamaño que reporta
   `H3 load: modem.img NNNN B`.

---

## 🔬 Ingeniería inversa del firmware — sin Ghidra

| Dato | Valor |
|---|---|
| Firmware | `~/mainline/downstream/stock-firmware-0713/modem.img` |
| Base de carga | **0x00000000** (confirmado: los punteros del pool son offsets del fichero) |
| Cadena `driver/sys_drv/ipc/src/fs_ccci.c` | `0x43a3e8` |
| Punteros a ella | `0x3ce4`, `0x3e9c`, `0x3ef4` |
| Códigos de error del módulo FS | pool `0x3ea0`+ : `0xfffff055…f05d` = −4011…−4003 |
| Assert línea 528 | `0x3d96`; línea 520 → `0x3d7a`; línea 547 → `0x3dda` |

```sh
arm-none-eabi-objdump -D -b binary -m armv7 -M force-thumb \
  --start-address=0x3d60 --stop-address=0x3e40 <firmware>
```

**Método reutilizable**: (1) buscar la cadena del fichero fuente; (2) buscar punteros a ese offset —
si aparecen, la base de carga es 0; (3) leer las constantes del literal pool contiguo; (4) localizar
los `LDR Rt,[PC,#imm]` (16 bits `0x48xx-0x4Fxx`, 32 bits `F8DF`) cuyo destino sea el pool → esas
instrucciones están **dentro** de la función; (5) `objdump -b binary -M force-thumb` alrededor.

---

## 🎛️ Parámetros en caliente (kernel #51)

```sh
S=/sys/module/mt6582_spm/parameters
echo 1          > $S/spm_md_smem_clr    # limpiar registro de excepcion (ANTES de cada ciclo)
echo 1          > $S/spm_md_poweroff    # apagar/encender el MD sin sacar la bateria
echo 1          > $S/spm_md_poweron
echo 9          > $S/spm_fs_1010_mode   # forma corta (la buena)
echo 0xffffffff > $S/spm_fs_1010_val    # valor devuelto en el 1010 (acepta hex)
echo 1          > $S/spm_fs_def_nf      # campos de la respuesta generica
echo 3          > $S/spm_fs_1012_nf     # campos del op 0x1012
echo 0x1004     > $S/spm_fs_ov_op       # override generico: a que op
echo 2          > $S/spm_fs_ov_nf       #                    cuantos campos
echo 0          > $S/spm_fs_quiet       # logs detallados (idx, op, path)
echo XX         > $S/spm_md_smem        # visor hex de la SMEM (offset en HEX)
```

Ciclo de prueba en el móvil: `/root/mdtest.sh [modo_1010] [nf_mode]`, o los scripts de barrido
`/root/sweep*.sh` que deja esta sesión.

---

## ▶️ SIGUIENTE PASO (concreto)

1. **Arreglar el `1004`**: el override genérico debe fijar `+8 = 4` (y el `len` del 2º campo) en vez
   de dejar la longitud de la petición. Es la misma corrección que ya lleva el `default`.
2. Seguir barriendo el contador de cada op nuevo que aparezca (`spm_fs_ov_op`/`spm_fs_ov_nf`).
3. Cuando el MD complete el mount de la NVRAM, debería llegar a **HS2 = `NORMAL_BOOT_ID`** → hito M1
   cerrado (arranque del MD sin RIL).

## Artefactos

| Qué | Ruta (Pi `.123`) |
|---|---|
| Imagen actual (#51) | `~/mainline/pkg/boot-modem-e.img` (md5 `b61ac603…`) |
| Driver | `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c` |
| Parches de la sesión | `~/mainline/patch_*.py` (versionados en `fixes-0729/`) |
| Ground-truth del fsd | `~/modem-fsd/gd-boot-full.out` + decodificador `~/mainline/dec12.py` |
| Fuente AP del stock | `~/mainline/downstream/bq-src/mediatek/kernel/drivers/dual_ccci/` |

## Estado del otro frente (Darwin)
Aparcado en **M138** con shell funcionando, solo salida. El UART del krillin es **3,3 V** (sale por
D+/D− del micro-USB, pads del PHY alimentados de VUSB33) — **no hace falta divisor**. Que no entren
datos apunta a **D+/D− cruzados** o al camino de RX. Ojo: en pmOS la UART está solo como consola de
arranque (`earlycon`), sin tty (`mt6577-uart 11002000.serial: probe failed -2`, falta un reloj en el
DT), así que **la prueba A/B de teclear en Linux no se puede hacer hasta arreglar ese probe**.
