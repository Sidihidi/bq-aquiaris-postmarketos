# H6r→H6t — Protocolo del FS descifrado: de 2 a 13 operaciones (2026-07-29)

> Continúa de `H6p-H6q-EXCEPCION-LOCALIZADA-0729.md`. Kernels #40 y #41.

## 🚀 TL;DR

El MD pasa de servir **2 operaciones** a **13**, con la **misma secuencia que el arranque real de
Lineage**, recorriendo su NVRAM de verdad. Las dos reglas del protocolo que faltaban salieron del
desensamblado del firmware — no están documentadas en ninguna parte.

```
antes:  100e 1010                                          -> EXCP
ahora:  100e 1010 1001 1005 1010 1010 1010 1010 1010 1010 1010 1010 1012
        (idéntica al ground-truth hasta el 1012)
```
Rutas que el MD recorre ya: `FAT3149C88D.log`, `NVRAM`, `NVRAM/IMPORTNT`, `NVRAM/NVD_IMEI`,
`NVRAM/CALIBRAT`, `NVRAM/NVD_CORE`, `NVRAM/NVD_DATA`.

---

## ⚠️ Primero: un FALLO DE MÉTODO que invalidó conclusiones anteriores

**La SMEM no se limpia entre ciclos.** El registro de excepción que leíamos podía ser el de la
corrida ANTERIOR. Por eso el control H6o parecía dar "el mismo registro respondiendo y sin
responder" — era literalmente el mismo registro, rancio. **Esa conclusión ("da igual lo que
respondamos") era falsa.**

**Fix (H6r)**: `spm_md_smem_clear()` + parámetro:
```sh
echo 1 > /sys/module/mt6582_spm/parameters/spm_md_smem_clr   # ANTES de cada ciclo
```
Todas las medidas de esta sesión se hicieron limpiando antes. Sin esto, ninguna comparación vale.

---

## Las DOS reglas del protocolo (del desensamblado)

El MD valida el buffer de respuesta con tres comprobaciones consecutivas en `fs_ccci.c`:

### Línea 520 — la marca "done"
```asm
3d66:  ldr   r2, [r3, #0]        ; buffer[+0]
3d6a:  mvn.w r3, r5, lsl #16     ;  r3 = 0xffff0000 | op
3d6e:  mvn.w r3, r3, lsr #16
3d72:  cmp   r3, r2
3d74:  beq.n 0x3d88              ; OK
```
`buffer[+0] == 0xFFFF0000 | op`. **Esta ya la cumplíamos.** ✓

### Línea 528 — el campo `+4` (código −4009)
```asm
3d88:  ldr.w r3, [r7, r4, lsl #2]
3d8c:  ldr   r3, [r3, #4]        ; buffer[+4]
3d90:  cmp   r6, r3
3d92:  beq.n 0x3db0              ; OK
```
**`buffer[+4]` debe conservar el `c` de la PETICIÓN.**
- GetDrive llega con `c=2` y escribíamos 2 → **coincidencia afortunada que enmascaraba el bug
  durante semanas**.
- `0x1010` llega con `c=1` y escribíamos 2 → assert.

### Línea 547 — la longitud (código −4010)
```asm
3dc2:  add.w r9, r3, #8          ; r9 = buffer + 8  (campo LEN)
3dc8:  ldr.w r4, [r9]            ; len que devolvemos
3dce:  ldr.w r3, [fp, r7, lsl #3]; tamaño esperado
3dd2:  cmp   r3, r4
3dd4:  bcs.n 0x3de8              ; OK sólo si esperado >= devuelto
```
**La longitud en `+8` no puede exceder lo que el MD reservó.** Nosotros devolvíamos `0x26` (eco de
la petición) contra un esperado de 4.

---

## Cómo se confirmó (medidas, con registro limpio)

`spm_fs_p0_mode` (H6s) controla en caliente qué se escribe en `+4`:

| `+4` que escribimos | Assert resultante | Ops servidas |
|---|---|---|
| `2` (comportamiento previo) | línea **528**, −4009, op `0x1010`, valor **2** | 2 |
| **no tocar** (queda el `1` del MD) | línea **547**, −4010, params **4** y **0x26** | 2 |
| `0` | línea **528**, −4009, op **`0x100e`**, valor `0` | **1** (falla ya en el GetDrive) |

Cada variante da un assert **distinto** → `+4` es exactamente lo que el MD valida. Hipótesis
confirmada. Y "no tocar" **supera la línea 528** y avanza hasta la 547.

## El fix (H6t) — `spm_fs_1010_mode` 9 y 10

Cumplen ambas reglas: no tocan `+4` y devuelven longitud corta.
- **9**: `+8=4`, `+0xc=0`, `length=12`
- **10**: clon de la forma del GetDrive (`+8=4`, `+0xc=0`, `+0x10=0x54`, `length=16`)

**Dan idéntico resultado** → el `+0x10=0x54` del GetDrive era irrelevante; lo que contaba era no
tocar `+4` y acortar la longitud.

---

## 📍 El muro nuevo: `0x1012` no existe en el arranque real

Inventario de op-codes del `ccci_fsd` de Lineage (`gd-boot-full.out`):
```
1009×201   1003×177   1005×173   1001×150   1002×97   1011×75   1004×6   1010×2   100e×1   101c×1
```
**`0x1012` no aparece ni una sola vez.** Nuestro MD se va por una rama que el original nunca pisa,
porque contestamos "vacío" a todos los `1010`.

Lo que hace el real y nosotros no:
```
REQ  1010 c=1 len=0x24  "Z:\NVRAM\NVD_DATA"
RESP 1010 c=2 len=0x36  "Z:\NVRAM\NVD_CORE\MT00A000"    <- RESUELVE a otra ruta
      ↓
1001 OPEN → 1009 tamaño → 1002 seek → 1003 read → 1005 close   (×cientos)
```

Ya no es un problema de protocolo sino de **semántica del sistema de ficheros**: el `fsd` traduce
nombres lógicos de NVRAM a ficheros físicos. La NVRAM del móvil **ya tiene esa estructura exacta**:
```
/data/nvram/md/NVRAM/NVD_CORE/{MT00A000, MT00B000}
/data/nvram/md/NVRAM/NVD_DATA/{MT03_000, MT04_000, MT0X_000, ...}
/data/nvram/md/NVRAM/{CALIBRAT, IMPORTNT, NVD_IMEI}
```

### ⚠️ Aviso para implementarlo
El ground-truth responde con `c=2` mientras que a nosotros escribir 2 en `+4` nos hacía petar → **la
expectativa del MD depende de su estado**, no es una constante. Hacerlo **conmutable en caliente** y
barrer, como todo lo demás, en vez de adivinar.

Además sólo hay **una** respuesta `ffff1010` capturada en todo el arranque (el poller se pierde la
mayoría), así que de momento sólo conocemos ese mapeo: `NVD_DATA → NVD_CORE\MT00A000`.

---

## Parámetros en caliente disponibles (kernel #41)

```sh
S=/sys/module/mt6582_spm/parameters
echo 1  > $S/spm_md_smem_clr      # limpiar registro de excepcion (ANTES de cada ciclo)
echo 1  > $S/spm_md_poweroff      # apagar/encender el MD sin sacar la bateria (H6k)
echo 1  > $S/spm_md_poweron
echo N  > $S/spm_fs_p0_mode       # +4: 0=actual 1=no-tocar 2=cero 3=eco del c
echo N  > $S/spm_fs_1010_mode     # 0..7 variantes previas, 8=no contestar, 9/10=forma corta
echo N  > $S/spm_fs_nf_mode       # encoding del not-found (0..6)
echo 0  > $S/spm_fs_quiet         # logs detallados (idx, op, path)
echo XX > $S/spm_md_smem          # visor hex de la SMEM (offset en HEX; <0x100 = barrido de cadenas)
```

Ciclo completo: `/root/mdtest.sh [modo_1010] [nf_mode]` en el móvil.

## Artefactos

| Qué | Ruta |
|---|---|
| Imagen actual (kernel #41) | `~/mainline/pkg/boot-modem-shape.img` (md5 `b398a68c…`) |
| Kernel #40 (p0_mode + smem_clr) | `~/mainline/pkg/boot-modem-p0.img` (md5 `e10a0904…`) |
| Parches | `~/mainline/patch_{p0,p0b,shape}.py` |
| Firmware a desensamblar | `~/mainline/downstream/stock-firmware-0713/modem.img` (5.172.580 B) |
| Ground-truth | `~/modem-fsd/gd-boot-full.out` |

**Ojo**: el DHCP cambia la IP del móvil constantemente (`.101`, `.108`, `.109`, `.110`, `.111`…).
Localizarlo con barrido + `hostname` (`krillin-mainline`), no fiarse de la IP anterior.

## ▶️ Siguiente

Implementar la **resolución de rutas** del `0x1010`/`0x1011`, replicando primero el ground-truth
(`NVD_DATA → NVD_CORE\MT00A000`) para comprobar si es eso lo que desbloquea, y luego generalizar.
Después vienen `1009`/`1002`/`1003`/`1005`, que ya son operaciones de fichero normales sobre
ficheros que existen.
