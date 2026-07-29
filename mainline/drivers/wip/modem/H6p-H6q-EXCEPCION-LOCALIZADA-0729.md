# H6o→H6q — La excepción del MD, localizada al byte (2026-07-29)

> Continúa de `H6h-H6o-BARRIDO-FS-Y-CONTROL-0728.md`.
> Kernels #37 (control) → #38 (volcado SMEM) → #39 (visor hex interactivo).

## 🎯 TL;DR

**Sabemos exactamente dónde y por qué revienta el MD.** No era el contenido de nuestras respuestas,
ni el transporte: es un **assert de consistencia** en el firmware del MD.

```
fs_ccci.c línea 528   ·   código 0xfffff057 (-4009)   ·   params: op=0x1010, valor=2
```

Y la instrucción culpable, desensamblada del firmware:
```asm
3d88:  ldr.w r3, [r7, r4, lsl #2]   ; r3 = tabla_buffers[idx]
3d8c:  ldr   r3, [r3, #4]           ; r3 = campo +4 (1ª palabra del payload)
3d8e:  cbz   r5, 0x3da8             ; si r5==0 -> lo MEMORIZA (mov r6,r3), sl=1
3d90:  cmp   r6, r3                 ; ¿lo memorizado sigue igual?
3d92:  beq.n 0x3db0                 ; sí -> continúa
3d96:  mov.w r2, #528               ; NO -> ASSERT, línea 528
3d9c:  ldr   r1, -> "…/fs_ccci.c"
3d9e:  ldr   r3, -> 0xfffff057
3da0:  str   r5, [sp,#0]            ; param = 0x1010 (el op)
3da2:  blx   0x3f9608               ; handler de excepción
```

**El MD memoriza el valor de `+4` y comprueba que no cambie entre peticiones.** El valor que
encuentra es **2** — y `2` es exactamente lo que escribimos nosotros en la respuesta al **GetDrive**:

```c
case 0x100e:                              /* GetDrive */
    writel(0x00000002, fs + boff + 4);    /* <- ESTE 2 */
```

**Hipótesis a atacar**: nuestra respuesta al `0x100e` deja `+4 = 2` en el buffer compartido, y el MD
espera que esa palabra conserve otro valor cuando procesa el `0x1010`.

---

## Lo que se descartó por el camino (todo medido, no supuesto)

| Hipótesis | Cómo se refutó |
|---|---|
| El **contenido** de nuestra respuesta al `0x1010` | Control H6o (modo 8 = no contestar nada) da **exactamente el mismo** resultado que contestar: HS ok, 2 ops FS, silencio (`RCHNUM=0 BUSY=0`). |
| El **encoding del not-found** | 7 variantes (H6j), todas igual. |
| El **formato de la respuesta** | 8 variantes (H6l), incluida la que reproduce el ground-truth byte a byte. |
| La **latencia** | 25 ms → sub-ms (H6m/H6n). Igual. |
| Nuestra respuesta al **GetDrive** | Idéntica palabra por palabra al `ccci_fsd` real: `[02][04][00][54]`. |
| El **transporte FS_TX** | Idéntico a `ccci_fs_send()` del stock: `data0`=vista MD del buffer, `data1`=len+4, canal 15, `reserved`=idx. |
| El **layout del buffer** | Confirmado por `ccci_fs.h`: `{u32 fs_ops; u8 buf[16384]}` = `0x4004`. ✓ |

**Prueba decisiva**: el registro de excepción es **BYTE A BYTE IDÉNTICO** con respuesta (modo 0) y sin
respuesta (modo 8). Si nuestra respuesta al `0x1010` fuera el problema, no podrían coincidir.

---

## H6p / H6q — cómo se leyó la excepción

`/dev/mem` está capado (`STRICT_DEVMEM`), así que el volcado se hizo desde el kernel.

- **H6p** (kernel #38): barrido de la SMEM buscando cadenas ASCII. Encontró `fs_ccci.c` en `+0x8d8`
  → primera prueba de que **el MD SÍ excepciona** (no estaba esperando).
- **H6q** (kernel #39): visor hex interactivo, para explorar sin reflashear:
  ```sh
  echo 1   > /sys/module/mt6582_spm/parameters/spm_md_smem   # barrido de cadenas
  echo 800 > /sys/module/mt6582_spm/parameters/spm_md_smem   # hex+ASCII 512B desde +0x800 (el valor va en HEX)
  ```

Registro de excepción tal cual sale:
```
+08d0: 00000000 00000000 635f7366 2e696363  |........fs_ccci.|
+08e0: 00000063 00000000 00000000 00000000  |c...............|
+08f0: 00000210 fffff057 00001010 00000002  |................|
        │        │        │        └─ valor leído (2)
        │        │        └─ op 0x1010
        │        └─ código de error
        └─ 0x210 = línea 528
```

---

## Ground-truth del `ccci_fsd` real (decodificado)

De `~/modem-fsd/gd-boot-full.out` (captura de Lineage). El MD real **pide el mismo fichero que el
nuestro y continúa**:

```
590845 REQ  100e GetDrive     c=2 len=0x08  "Z:\"
591291 RESP 100e GetDrive     c=2 len=0x04           <- [02][04][00][54]
591463 REQ  1010 GetFullPath  c=1 len=0x26  "Z:\FAT3149C88D.log"
591921 REQ  1001 OPEN         c=2 len=0x12  "Z:\NVRAM"          <- SIGUE (458us después)
592311 REQ  1010 GetFullPath  c=1 len=0x24  "Z:\NVRAM\NVD_DATA"
593172 RESP 1010              c=2 len=0x36  "Z:\NVRAM\NVD_CORE\MT00A000"
       ... luego OPEN/GETSIZE/SEEK/READ/CLOSE en cascada sobre MT00A000, MT00B000...
```

Decodificador reutilizable (UTF-16LE + op + c/len) en `~/modem-fsd/` — ver el script `dec.py` de la
sesión; el formato de cada línea es `t idx=N op=XXXXXXXX | w0 w1 w2 ...` donde `high16(op)=0xffff`
marca respuesta, `w0`=c, `w1`=len en bytes, `w2..`=payload (path en UTF-16LE).

---

## 🔬 Ingeniería inversa del firmware — método y datos

**No hizo falta Ghidra.** Con `objdump` sobre el binario crudo y localizando los literal pools:

| Dato | Valor |
|---|---|
| Firmware **CORRECTO** | `~/mainline/downstream/stock-firmware-0713/modem.img` (**5.172.580 B**) |
| Base de carga | **0x00000000** (confirmado: los punteros del pool son offsets del fichero) |
| Cadena `driver/sys_drv/ipc/src/fs_ccci.c` | `0x43a3e8` |
| Punteros a ella | `0x3ce4`, `0x3e9c`, `0x3ef4` |
| `0xfffff057` | pool `0x3ea4`; serie `f055…f05d` = **−4011…−4003**, códigos del módulo FS |
| **Assert línea 528** | **`0x3d96`** (mov.w r2,#528), llamada en `0x3da2`, handler `0x3f9608` |

Desensamblar:
```sh
arm-none-eabi-objdump -D -b binary -m armv7 -M force-thumb \
  --start-address=0x3d60 --stop-address=0x3dd0 \
  ~/mainline/downstream/stock-firmware-0713/modem.img
```

### ⚠️ GOTCHA que costó un rato
Se analizó primero `~/modem-h0/modem_1_wg_n.img` (**5.100.832 B**) — **NO es el que se carga**. El
driver usa el de `stock-firmware-0713` (5.172.580 B, el que está en `/lib/firmware/modem.img` del
móvil). Las líneas de assert no cuadraban (441/517/525 en vez de 528) precisamente por eso.
**Verificar siempre el tamaño contra el que reporta `H3 load: modem.img NNNN B`.**

### Técnica reutilizable
1. Buscar la cadena del fichero fuente en el binario (`grep -abo`).
2. Buscar punteros a ese offset (`struct.pack("<I", off)` alineados a 4) → si aparecen, **la base de
   carga es 0**.
3. Los pools contienen, junto al puntero, las constantes de la función (los códigos de error).
4. Buscar los `LDR Rt,[PC,#imm]` (16 bits `0x48xx-0x4Fxx`, o 32 bits `F8DF`) cuyo destino sea el
   pool → esas instrucciones están **dentro** de la función culpable.
5. `objdump -b binary -M force-thumb` alrededor → código legible.

---

## 🔌 UART del krillin: es 3,3 V (corrección)

La UART sale por los pines **D+/D− del micro-USB**, que son pads del PHY USB alimentados desde
**VUSB33 (3,3 V)**. **No hace falta divisor de tensión** — la idea del 1,8 V era una premisa falsa y
el divisor 1 kΩ/1,2 kΩ que se diseñó no aplica.

Comprobación definitiva (10 s con un multímetro): medir el **nivel en reposo de la línea TX del
móvil** contra masa. Una UART en reposo está a nivel alto, así que ese voltaje **es** el de su
lógica. ~3,3 V confirma; ~1,8 V lo desmentiría.

Entonces, ¿por qué no entran datos? Dos sospechosos, por orden de probabilidad:
1. **D+/D− cruzados** — encaja perfectamente con "salida sí, entrada no". Probar intercambiando los
   dos hilos.
2. **El camino de RX no activo en Darwin** — el multiplexado USB→UART lo deja puesto el
   preloader/LK; si Darwin no lo mantiene, la entrada se pierde.

### ★ Prueba A/B pendiente: UART en pmOS
**Habilitar una consola serie en pmOS sobre esa misma línea y probar a teclear.** Es el experimento
que separa hardware de software de un plumazo:
- **Entran teclas en pmOS** → cableado y niveles OK → el problema es **de Darwin** (mux/driver).
- **Tampoco entran** → es hardware/cableado (cruce de D+/D−, o el pin RX no llega).

Herramientas ya listas en la Pi: `~/uart-console.sh` (consola interactiva con screen sobre
`/dev/ttyAMA0` a 921600, y al salir reactiva la captura de fondo) y `~/uart-loopback.sh` (puentea
pin 8 ↔ pin 10 para validar que el TX de la propia Pi funciona).

---

## ▶️ SIGUIENTE: atacar la hipótesis del GetDrive

La sospecha concreta: el `writel(2, fs + boff + 4)` de nuestra respuesta al `0x100e` deja en `+4` un
valor que el MD espera distinto al procesar el `0x1010`.

Experimentos a hacer (todos con el visor SMEM ya disponible, sin recompilar salvo el 3):
1. **Ver qué hay en el buffer justo antes del assert**: volcar la SMEM en la zona de los buffers FS
   (`MD_SMEM_PHYS + 0xE000 + idx*0x4004`) tras el timeout, y comparar `+0`/`+4`/`+8` con lo que
   esperaríamos.
2. **Comprobar el índice de buffer**: si el `100e` y el `1010` usan `idx` distinto, el `2` que ve el
   MD no vendría de nuestra respuesta y la hipótesis cae.
3. **Barrer el valor de `+4` en la respuesta del GetDrive** (parámetro en caliente, como se hizo con
   `spm_fs_nf_mode`): probar 0, 1, 2, 4, 0x54… y ver si cambia el assert (línea/params) o desaparece.

**Nota**: el driver es **builtin**, así que cada cambio de código pide ciclo BROM completo. Si esto
se alarga, compensa pasarlo a `=m` (`CONFIG_MT6582_SPM=m`) y quedarse en `insmod`/`rmmod`: un
flasheo una sola vez y luego iteración en segundos.

## Artefactos

| Qué | Ruta |
|---|---|
| Imagen actual (visor hex) | `~/mainline/pkg/boot-modem-smem2.img` (md5 `82b1704e…`), kernel #39 |
| Imagen kernel #38 | `~/mainline/pkg/boot-modem-smem.img` (md5 `5f6fca3d…`) |
| Flasheo | `~/flash-smem.sh` |
| Ciclo de prueba | `/root/mdtest.sh [modo_1010] [nf_mode]` en el móvil; `~/modem-test.sh` desde la Pi |
| Parches | `~/mainline/patch_smem.py`, `patch_smem2.py` |
| Firmware a desensamblar | `~/mainline/downstream/stock-firmware-0713/modem.img` |
| Fuente AP del stock | `~/mainline/downstream/bq-src/mediatek/kernel/drivers/dual_ccci/{ccci_fs.h,ccci_fs_main.c}` |

**Ojo con la IP del móvil**: el DHCP se la cambia (ha estado en `.101`, `.108` y `.109`). Localizarlo
con un barrido y `hostname` (`krillin-mainline`) en vez de fiarse de la IP anterior.
