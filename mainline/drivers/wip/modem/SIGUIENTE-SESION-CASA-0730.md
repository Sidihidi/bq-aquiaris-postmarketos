# Arranque de la sesión de casa — módem MT6582 (2026-07-30)

> **Empieza por aquí.** Documento operativo: qué hacer, en qué orden y con qué comandos.
> El detalle está en `PROXIMO-FRENTE-L4-Y-CANALES-0730.md` (frente actual) y
> `M1-COMPLETO-NORMAL-BOOT-ID-0730.md` (hito + auditoría).

---

## Estado en tres líneas

**M1 CERRADO**: el MD arranca (`NORMAL_BOOT_ID`) con defaults puros, 900 ops FS. Reproducible.

Con `spm_fs_post_hs2_iters` (nuevo) se sigue sirviendo pasado el hito → **1626 ops** y aparece el
bloqueo siguiente: **`tarea='L4'`** (la capa de NVRAM del propio MD), `ASSERT nvram_io.c:1202`,
error **10** = **fallan las DOS copias espejo** del registro **161** (`CALIBRAT/MPA8_000`).

Kernel actual **#81**, imagen en la Pi `.123`: `~/mainline/pkg/boot-modem-m1.img`.

---

## ⚠️ TAREA 0 — sincronizar el driver (riesgo de continuidad)

**La copia del driver en el repo está desfasada**: `wip/modem/mt6582-spm-H1.c` son **1323 líneas**
(snapshot de `dd8690e`, era H6i) mientras el árbol de la Pi ronda las **2500**. Entre medias hay 14
commits y **33 parches** en `fixes-0729/` + `fixes-0730/`, y **varios se pisan entre sí** (H8f
sustituyó una traducción de rutas anterior; H8k modificó lo que hizo H8g), así que reconstruirlo
aplicándolos en orden **no es fiable**.

Lo primero, antes de tocar nada:

```bash
scp cpcd@192.168.0.123:~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c \
    mainline/drivers/wip/modem/mt6582-spm-H9.c
```

y commitear. El árbol de la Pi **no es git** → hasta que eso esté hecho, el estado real del driver
vive en un solo sitio sin respaldo.

*(No se pudo hacer al cerrar esta sesión: la Pi `.123` dejó de responder — el móvil `.113` sí
respondía, así que era la Pi, no la red.)*

---

## ▶️ Los pasos, en orden

### 1. RE de `0x290398` — la validación de UNA copia  *(offline, sin HW, barato)*

Es el paso con mejor relación información/coste. `0x28dc8c` ya está desensamblado (sección H9b del
otro doc) y sabemos que llama a `0x290398` **dos veces**, una por copia. Lo que no sabemos:

- **qué comprueba** una copia (¿checksum? ¿versión? ¿marca `0xCDEF`?),
- **de dónde sale la copia B** — otro fichero, otro offset del mismo, u otra región.

Eso es lo que decide el arreglo, porque el contenido del fichero ya está descartado como causa
(ver "refutado" abajo).

```bash
F=~/mainline/downstream/stock-firmware-0713/modem.img     # base de carga = 0
arm-none-eabi-objdump -D -b binary -m armv7 -M force-thumb \
  --start-address=0x290398 --stop-address=0x290500 $F
```

Piezas ya identificadas: `0x290170` = reconstruir la copia mala desde la buena; `0x29001c` = accesor
de los logs; `0x28f8c8` / `0x28ff50` / `0x28f898` = llamadas del preámbulo (descriptor → handles).

### 2. Confirmar que el registro 161 es `MPA8_000`  *(el mapeo no está cerrado)*

Sale de la tabla de items (`0x4d41ca`–`0x4d566a`, entradas de 32 B, LID en `+0x0e`), y los nombres
casan con ficheros reales — pero **los campos de tamaño no cuadran**, así que puede haber un desfase
de una entrada. Confirmarlo con la traza antes de construir encima:

```sh
S=/sys/module/mt6582_spm/parameters
echo 0    > $S/spm_fs_quiet        # logs con ruta — CUESTA ~30 ms/op
echo 6000 > $S/spm_fs_slow_iters   # con 1626 ops hacen falta ~150 s de presupuesto
# ...ciclo... y mirar el ultimo fichero que toca antes del assert
```

### 3. Auditar el ACK del bucle

`BUSY=00000001` al salir → el MD mandó algo y **sigue esperando el ACK del AP**. Confirmar que
ACKeamos **todos** los canales que dispara, no solo los que leemos de `RCHNUM`.

### 4. Contestar en los canales CCMNI/PCM

Tras el HS2 el MD levanta sus tres interfaces de red y el canal de voz. Los logueamos y ACKeamos pero
**no contestamos nada**:

| `lch` | Canal | `rsv` |
|---|---|---|
| 0 | `CCCI_CONTROL_RX` | `0x5555ffff`=HS1 · `0`=HS2 · `0x45584350`=`"EXCP"` |
| 4 | `CCCI_PCM_RX` (voz) | 0 |
| 23 / 27 / 31 | `CCCI_CCMNI1/2/3_TX_ACK` | 2 / 3 / 4 |

Nombres en `~/mainline/downstream/bq-src/mediatek/kernel/drivers/dual_ccci/include/ccci_ch.h`.
Aquí ya se entra en el data-path (territorio RIL). Encaja con H8e: declaramos y zeramos 6 regiones
NetUL/DLCtrl y nunca las usamos.

**L1 (radio: calibración RF, rails/PLLs del frontend) va detrás de todo esto.** La nota de "siguiente
frontera = L1, `m12100.c:9064`" está **obsoleta**: ese assert lo cerró `MISC_32K_LESS` en `26efd89`.

---

## ❌ Refutado por medición — no volver a pisarlo

| Hipótesis | Cómo se refutó |
|---|---|
| El op nuevo `0x1021` necesita otro nº de campos | Barrido `ov_op`/`ov_nf`: con **1** (lo que ya damos) pasa; con 3 protesta en `fs_ccci.c:528`. El assert de L4 es idéntico. |
| El contenido de `X:`/`Y:` está mal | El `fsd` real mapea `X:`→`/protect_f/md`, `Y:`→`/protect_s/md` (particiones aparte), pero los ficheros que el MD se crea son **byte a byte idénticos** a `~/golden/protect_{f,s}.img`. |
| Falta algún fichero de NVRAM | El árbol del móvil es idéntico en estructura y conteos al extraído del Lineage real (`IMPORTNT` está vacío **también** en el original). |
| `MPA8_000` está en blanco → por eso falla | Son 1352 ceros + marca `0xCDEF`… pero **57 de los 165 registros están igual** en la NVRAM del móvil que sí arranca la radio. Un registro en blanco con su marca es **lo normal**. |

---

## 🔧 El ciclo de trabajo (≈4 min, sin tocar el móvil)

```bash
# 1) compilar
bash ~/mainline/build-modem.sh                    # deja ~/mainline/pkg/boot-modem-m1.img

# 2) flashear por dd  (¡OJO: ssh -T en el interno, NO -n!)
IP=192.168.0.113
cat ~/mainline/pkg/boot-modem-m1.img | ssh -T root@$IP \
    "dd of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync; sync"

# 3) VERIFICAR releyendo — imprescindible antes de reiniciar
md5sum ~/mainline/pkg/boot-modem-m1.img
ssh -n root@$IP "dd if=/dev/mmcblk0 bs=512 skip=83968 count=40960 2>/dev/null | md5sum"

# 4) reiniciar (un reboot normal NO reinicia esta pmOS)
timeout 8 ssh -n root@$IP "sync; echo 1 > /proc/sys/kernel/sysrq; \
    (echo b > /proc/sysrq-trigger &) >/dev/null 2>&1"
```

Sector 83968 × 512 = `0x2900000`, el mismo offset que `mtkclient wo`. `mmcblk0` = eMMC interna; la
raíz de pmOS vive en la **SD** (`mmcblk1p1`) → no se toca.

### Medir

```sh
S=/sys/module/mt6582_spm/parameters
echo 1     > $S/spm_fs_quiet
echo 800   > $S/spm_fs_slow_iters        # 20 s de presupuesto (x25 ms)
echo 20000 > $S/spm_fs_post_hs2_iters    # H9a: seguir tras el HS2 (0 = parar en el hito)
echo 1 > $S/spm_md_smem_clr              # SIEMPRE antes de medir
echo 1 > $S/spm_md_poweroff; sleep 1; echo 1 > $S/spm_md_poweron; sleep 1
for p in spm_md_load spm_md_remap spm_md_release spm_md_hs2; do echo 1 > $S/$p; done
sleep 26
echo 1 > $S/spm_md_ex                    # decodifica el registro de excepcion
```

`/root/mdrun.sh` en el móvil hace el ciclo con **defaults puros** (no escribe ningún ajuste).

---

## ⚠️ Trampas que ya han costado iteraciones

1. **`cat img | ssh -n root@ip "dd ..."` escribe 0 bytes** — el `-n` cierra stdin. En el ssh interno
   va **`-T`**.
2. **`mdtest.sh` pisa el default bueno**: pone `spm_fs_1010_mode=0`. Usar `mdrun.sh`.
3. **El reintento en caliente falla ~la mitad de las veces**: 3 de 5 ciclos dieron **0 ops y ningún
   HS2** (el `poweroff→poweron` no reinicia limpio el MD tras un assert). **Repetir hasta obtener
   HS2** antes de dar una medida por válida, y **no leer un "0 ops" como resultado del experimento**.
4. **Limpiar la SMEM (`spm_md_smem_clr`) antes de cada ciclo** o se comparan registros rancios — ya
   invalidó conclusiones enteras.
5. **`abootimg` falla si se le redirige la salida a `/dev/null`.** El `build-modem.sh` ya lleva la
   guarda; esperar a `FIN`, no a que empiece el paso `[3]`.
6. **El DHCP cambia la IP del móvil en cada arranque** (`.108`…`.115`): barrer + `hostname`
   (`krillin-mainline`).
7. **El firmware que se carga es `stock-firmware-0713/modem.img` (5.172.580 B)**, NO
   `modem-h0/modem_1_wg_n.img`. Verificar contra el tamaño que reporta `H3 load:`.
8. **Leer los warnings del build.** De los cuatro defectos de la auditoría de hoy, **dos los estaba
   avisando el compilador desde el principio**. El fichero ahora compila **sin ninguno** — mantenerlo así.

---

## Artefactos

| Qué | Ruta |
|---|---|
| Driver (fuente de verdad, **no git**) | Pi `.123`: `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c` |
| Imagen #81 | Pi `.123`: `~/mainline/pkg/boot-modem-m1.img` |
| Script de build | Pi `.123`: `~/mainline/build-modem.sh` |
| Parches versionados | `fixes-0729/` (18) + `fixes-0730/` (15) |
| Firmware a desensamblar | `~/mainline/downstream/stock-firmware-0713/modem.img` |
| Ground-truth del `fsd` | `~/modem-fsd/gd-boot-full.out` + `boot-fsd.strace` (rutas reales) |
| NVRAM real extraída | `~/modem-fsd/nvram/md` (165 registros) |
| Golden de las protegidas | `~/golden/protect_{f,s}.img` (ext4, loop-mount) |
| Fuente AP del stock | `~/mainline/downstream/bq-src/mediatek/kernel/drivers/dual_ccci/` |

## Otro frente

**Darwin/XNU** aparcado en **M138**: shell funcionando, solo salida. El UART del krillin es **3,3 V**
(no hace falta divisor) → que no entren datos apunta a **D+/D− cruzados** o al camino de RX. La prueba
A/B en pmOS está bloqueada: la UART solo está como `earlycon`, sin tty
(`mt6577-uart 11002000.serial: probe failed -2`, falta un reloj en el DT).
