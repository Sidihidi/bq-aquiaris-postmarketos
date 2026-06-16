# 🧭 Estrategia: root, unlock, custom kernels y bootloader propio (MT6582 krillin)

> Investigado 2026-06-16. Contexto: tras volver a Android stock, el `seccfg`
> quedó en estado "locked" y no deja arrancar kernels custom. Objetivo: un flujo
> de desarrollo robusto que NO dependa del ciclo "stock → TWRP → ROM → kernel".

## 💡 La idea clave (esto lo cambia todo)

En MediaTek, **el verdadero cimiento NO es TWRP ni una ROM custom — es el modo
BROM / Preloader (descarga a nivel de SoC)**. El BootROM del MT6582 está en la
silicio, es inmutable, y entra en modo descarga USB por combinación de teclas /
batería fuera, **independientemente de lo que haya (o no haya) instalado**.

Con la herramienta **mtkclient** (Linux, el reemplazo libre de SP Flash Tool)
podemos, vía BROM:
- **Leer y escribir CUALQUIER partición** directamente por USB (boot, recovery,
  lk/uboot, system, seccfg, preloader...).
- Hacer un **backup completo "golden"** de todo el teléfono.
- **Desbloquear el bootloader** escribiendo el `seccfg` (`mtk da seccfg unlock`).
- **Restaurar cualquier desastre** con una orden, sin Android stock de por medio.

➡️ **Mientras el preloader + BootROM estén intactos (y no quememos el efuse de
secure boot — que en el krillin NO está quemado, por eso arrancaban kernels
custom), el teléfono es prácticamente imposible de brickear de forma permanente.**

Esto **elimina el dolor**: no hace falta TWRP para instalar kernels (se flashea
`boot` directamente), no hace falta ROM custom como requisito, y un brick se
arregla con `mtk w boot golden_boot.img`, no rehaciendo todo el ciclo.

## 🛠️ Herramientas confirmadas (junio 2026)

| Herramienta | Para qué | Notas |
|---|---|---|
| **mtkclient** (bkerler) | leer/escribir particiones, unlock seccfg, backup, restore vía BROM | **Soporta MT6582** (carga `mt6582_payload.bin`); conecta en modo preloader ("Device detected", "Preloader - Detected regular mode!"). Corre en Linux (la Pi) |
| SP Flash Tool | alternativa Windows de lo mismo | usa el scatter `MT6582_Android_scatter.txt` de la ROM stock |
| TWRP krillin | recovery (backups nandroid, flashear zips) — OPCIONAL | oficial: twrp.me/bq/bqaquarise45.html; fuentes: `Pablito2020/android_recovery_bq_krillin` (mismo autor que nuestro kernel) y `Kra1o5/android_device_bq_krillin` |
| U-Boot mt6582 | bootloader propio (ver §4) | `mediatek-mainline/u-boot`, defconfig `mt6582_prestigio_pmt5008_3g` (¡mismo device de referencia que nuestro dts!) |

## ⚠️ Gotcha conocido del MT6582 en mtkclient

El MT6582 reporta una **tabla MBR con tamaños imposibles** (la partición data
sale como ~2 PB; entrada extendida con tamaño 0xFFFFFFFF). Es el MISMO problema
que nos frenó en mainline (por eso usamos `blkdevparts=`). Efecto: la
auto-detección de particiones de mtkclient puede fallar y un `mtk rl` (dump all)
de la partición "data" da error. **Workaround**: operar por
**offset/longitud explícitos** — y los tenemos exactos del `/proc/dumchar_info`:

```
preloader  0x0000000     (0x1400000)   ← NO TOCAR salvo emergencia
seccfg     0x1d00000     0x20000        ← el flag de lock/unlock vive aquí
uboot(lk)  0x1d20000     0x60000        ← el bootloader LK
bootimg    0x1d80000     0x1400000      ← kernel (pmOS / mainline)
recovery   0x3180000     0x1400000      ← aquí iría TWRP
android(p5)0x5900000     0x40000000     ← root de pmOS
usrdata(p7)0x71500000    (hasta fin)    ← 5.7 GB libres
```

## 📋 Plan por capas (de cero riesgo a ambicioso)

### Capa 0 — La red de seguridad (HACER PRIMERO, una sola vez)
1. Instalar mtkclient en un Linux (la Pi cuando vuelvas, o cualquier PC Linux):
   ```sh
   git clone https://github.com/bkerler/mtkclient
   cd mtkclient && pip install -r requirements.txt
   sudo cp mtkclient/Library/*.rules /etc/udev/rules.d/   # reglas udev
   sudo udevadm control --reload-rules
   ```
2. Entrar en BROM: teléfono apagado → `mtk r preloader preloader.bin` esperando →
   conectar USB manteniendo **Vol-** (o batería fuera + enchufar). Enumera como
   `0e8d:2000` (preloader) o `0e8d:0003` (brom).
3. **Backup golden de todo** (por nombre cuando se pueda, por offset lo demás):
   ```sh
   mtk rl dump_golden --skip=userdata   # vuelca todas las particiones a una carpeta
   # o partición a partición: mtk r seccfg seccfg.bin ; mtk r lk lk.bin ; etc.
   ```
   Guardar `dump_golden/` fuera del teléfono (y anotar dónde — son varios cientos
   de MB; el preloader/lk/seccfg/nvram/proinfo son los críticos e irremplazables).

### Capa 1 — Desbloquear para kernels custom
```sh
mtk da seccfg unlock      # escribe el flag de unlock en seccfg
# si da "Couldn't detect seccfg": escribir el seccfg unlocked a mano por offset,
#   o restaurar un seccfg unlocked conocido. El krillin SÍ tiene seccfg (0x1d00000).
```
- Como el **efuse de secure boot NO está quemado**, el LK no verifica firmas →
  con el flag unlocked, los `boot.img` custom arrancan.
- Fallback: `fastboot oem unlock` (el LK de BQ lo soportaba históricamente).

### Capa 2 — Flujo de desarrollo de kernels (SIN TWRP ni ROM)
- Flashear pmOS / mainline directamente a `boot`:
  ```sh
  mtk w boot boot-pmos.img        # vía BROM (siempre funciona)
  # o, con bootloader unlocked y sistema arrancando:
  fastboot flash boot boot-mainline-vN.img
  ```
- Mantener imágenes "golden" (`boot-pstore.img` de pmOS, `boot-mainline-vN.img`).
- Brick / no arranca → `mtk w boot boot-pmos.img` y listo. **Fin del ciclo dolor.**

### Capa 3 — TWRP (opcional, comodidad)
Solo si quieres backups nandroid o flashear zips desde el propio teléfono:
```sh
mtk w recovery twrp-krillin.img      # a la partición recovery (0x3180000)
```
Imagen: la oficial de twrp.me o construida desde `Pablito2020/android_recovery_bq_krillin`.
**No es necesaria para nuestro trabajo de kernels.**

### Capa 4 — Bootloader propio (lo que preguntaste — ambicioso)
Dos caminos, ambos flasheables a la partición `uboot`/`lk` (0x1d20000) por
mtkclient, con BROM como red de seguridad:

**4a. Parchear/recompilar el LK existente (esfuerzo medio, recomendado para menú de boot)**
- Ya tenemos `~/firmware-stock/lk.bin` en la Pi (277 KB, mboot de MTK, cabecera
  `0x58881688` label "LK"). Sus strings confirman la lógica de boot/recovery.
- **Fuente del LK del MT6582 disponible**: vive en los árboles de vendor CM13
  (`mediatek/platform/mt6582/lk/`), incluido `mt_disp_drv.c` (init de display —
  útil también para el tinte amarillo de M3). Recompilar > parchear binario.
- Parchear: saltar verificación + **menú de arranque** (pmOS / mainline / Android).
  Mantiene display + fastboot + USB del LK de MTK.
- Reflashear `lk` por mtkclient. Si algo va mal → restaurar `lk.bin` golden por BROM.

### 🔀 Dual-boot pmOS + Android: lo que YA permite el LK (sin parchear)

**Hallazgo clave (strings de nuestro lk.bin):** el LK del krillin **ya arranca
dos targets de forma nativa** — `boot` (normal) y `recovery` (con la tecla de
recovery, o un bit en la RTC). Código `mboot_android_load_recoveryimg_hdr`
presente. Eso es un **dual-boot gratis, sin tocar el bootloader**:
- `boot` (0x1d80000, 20 MB) → pmOS (o mainline)
- `recovery` (0x3180000, 20 MB) → boot.img del segundo sistema (Android, u otro pmOS/mainline)
- **Cambiar de sistema**: mantener la combinación de recovery al encender, **o**
  desde el SO poner el bit de modo en la RTC (`reboot recovery`) → arranque por
  software sin teclas (ideal para desarrollo headless).

**El menú NO es el trabajo difícil — el reparto de almacenamiento sí.** Cada SO
necesita su rootfs/system. Layout actual del krillin:
`android/p5` (1 GB, root de pmOS ahora), `cache/p6` (700 MB), `usrdata/p7` (5.7 GB).
Para convivir pmOS + Android hay que planear particiones (p.ej. recortar p7 para
una segunda raíz, o Android en sus particiones nativas + raíz pmOS en una porción).
mtkclient puede reparticionar (GPT/scatter). **Esto es el diseño real a pensar.**

➡️ **Recomendación**: empezar por el dual-boot de dos slots nativo (riesgo CERO en
el LK) + resolver el layout de almacenamiento; dejar el menú custom en LK (4a)
para cuando los dos slots se queden cortos.

**4b. U-Boot mt6582 (esfuerzo alto, elegante a largo plazo)**
- `mediatek-mainline/u-boot`, defconfig `mt6582_prestigio_pmt5008_3g` (nuestro
  mismo SoC de referencia). Estado: **"works uart, sd-card"**, arranca Linux desde
  SD/UART, pero **display/USB/fastboot NO funcionan aún** (haría falta portar el
  display — se solapa con nuestro M3). Permite multiboot estilo extlinux.
  ```sh
  make mt6582_prestigio_pmt5008_3g_defconfig && make -j4
  # envolver con cabeceras MTK (como nuestro assemble.sh) y flashear a lk
  ```

> 🟡 **Recomendación honesta sobre el bootloader propio:** es un hito molón para
> el futuro (multiboot, matar verificación de raíz), PERO **no resuelve tu dolor
> actual** — la red BROM + mtkclient ya lo hace, y escribir el bootloader AÑADE
> riesgo (tocas la partición `lk`). Orden sensato: primero Capas 0-2 (te
> devuelven el desarrollo sin miedo YA), el bootloader como Capa 4 por diversión
> / multiboot cuando el resto esté rodado. El 4a (menú en el LK) da el mejor
> retorno: calidad de vida real con riesgo acotado por BROM.

## ✅ Activos que YA tenemos en la Pi (cpcd@192.168.0.123)
- **mtkclient instalado** (`~/mtkclient`, con `mt6582_payload.bin` y DA legacy).
- **`~/firmware-stock/lk.bin`** (LK stock) + **`~/firmware-stock/boot.img`** (Android stock).
- **`~/pmos-artifacts/`**: `boot-pstore.img` (pmOS 3.10 con DEVMEM), `boot-clean.img`,
  `boot-debug.img`, `config-bq-krillin.armv7`, `zeros4m.img`.
- Árbol mainline en `~/mainline/linux-7.0.12` (hasta v25) + driver USB M2b.
- ⚠️ Falta el **backup golden completo** (preloader/seccfg/nvram/proinfo) — Capa 0.

## 🎯 Próximos pasos concretos (cuando tengas el teléfono + un Linux)
1. mtkclient + reglas udev en la Pi (o cualquier Linux).
2. Entrar en BROM y **backup golden** (sobre todo preloader, lk, seccfg, nvram, proinfo).
3. `mtk da seccfg unlock` → verificar con `fastboot oem unlock` / arrancar un boot.img custom.
4. Reflashear pmOS (`boot-pstore.img`) → recuperar el sistema estable.
5. Retomar mainline (M2b USB gadget / Alpine en p7) — ver `mainline/README.md`.
6. (Opcional) Capa 4a: menú de arranque en el LK.

## 🔍 Pendiente de averiguar / preguntar al usuario
- **¿Qué "par de problemas" obligaron a volver a stock?** Saberlo evita repetirlos
  (¿no arrancaba pmOS? ¿brick parcial? ¿pérdida de fastboot?).
- ¿El `fastboot oem unlock` del LK stock sigue respondiendo, o ya está bloqueado?
- ¿Hay un Linux disponible donde está el teléfono ahora, o hay que esperar a la Pi?

## Fuentes
- mtkclient: https://github.com/bkerler/mtkclient — discusión MT6582 (MBR gigante):
  https://github.com/bkerler/mtkclient/discussions/163
- TWRP krillin: https://twrp.me/bq/bqaquarise45.html ·
  https://github.com/Pablito2020/android_recovery_bq_krillin
- U-Boot mt6582: https://github.com/mediatek-mainline/u-boot
- LK MTK (referencia): https://github.com/littlekernel/lk
