# STP FULL mode + salto del fw ROMv1 — hallazgos 0716/0717 (sesión Windows)

## Estado en una línea

**El STP FULL mode funciona y la descarga multi-patch es byte-idéntica al stock, pero el fw
ROMv1 crashea nada más saltar tras el RESET-2: el PC del MCU acaba barriendo linealmente
desde ~0x80000 (región vacía). Falta descubrir qué pone el stock en esa región.**

## Lo que se implementó y FUNCIONA (mt6582-btif.c, kernels #9–#13, boot-stpfull.img)

1. **STP FULL mode** (tras el SET_STP `01 04 05 00 03 DF 0E 68 01`):
   - TX: header `0x80+(txseq<<3)+txack | (type<<4)|len_hi | len_lo | (h0+h1+h2)&0xff`
     + payload + **CRC-16/ARC** del payload (`crc16()` del kernel, `<linux/crc16.h>`,
     idéntico a la tabla `crc16_table` del stock `osal.c`).
   - ACK suelto de 4B (`0x80+txack, 0, 0, cksum`) tras cada frame de datos RX.
   - RX: parsea seq/ack, sigue la secuencia del chip (`txack = seq recibido`), consume
     los frames len=0 (ACKs sueltos / inband) sin avanzar secuencia.
   - Estado inicial `txseq=0, txack=7, exp_rxseq=0`; el flip a full se hace justo tras
     ENVIAR el SET_STP (sin esperar EVT) + 10ms + flush, como el sw_init BTIF del stock.
   - **Verificado**: QUERY-STP2 responde en full; los 22+81 fragmentos y ambos RESET
     también. El krillin stock usa **BTIF full mode** (thread `btif_rxd` en la captura),
     no UART: nuestro framing es el correcto.

2. **El chip es ESTRICTO con la secuencia**: un frame fuera de orden se descarta EN
   SILENCIO (probado: RESET con seq=0 forzado → rxlen=0, ni EVT ni reboot; todo lo
   posterior en secuencia "corrida" también muere). Los WMT_RESET deben ir EN secuencia.

3. **Resincronización post-RESET-2**: el fw nuevo emite el INBAND RESET `80 00 00 80`
   (stp_core.c:2234 "Resync STP with firmware!") y su EVT con seq=0. El host debe
   resetear `txseq=0` DESPUÉS del EVT (no antes de enviar el reset). Con esto el
   siguiente comando sale con header `0x80` como en la captura stock.

## La descarga está VERIFICADA correcta

- **Flujo multi-patch** = fuente stock (bq-src y ubports/kernel_krillin en GitHub,
  idénticos): por cada patch → `WMT_PATCH_ADDRESS_CMD` + `WMT_PATCH_P_ADDRESS_CMD`
  (con el address en el offset 12) + frags de 1000B (flags 1ST/MID/LAST = 1/2/3)
  + `WMT_RESET` (init_table_3). Comandos byte-idénticos (verificado contra el source).
- **Addresses CONFIRMADOS**: van dentro del header del patch (offset 24-27):
  - `ROMv1_patch_1_1_hdr.bin`: `21 00 0e f0` → `00 00 0e f0` = **0xF00E0000**
    (= ventana EMI del chip + 0xE0000)
  - `ROMv1_patch_1_0_hdr.bin`: `22 00 06 00` → `00 00 06 00` = **0x00060000** (RAM interna)
  - El launcher (`/system/bin/6620_launcher`, extraído del system de LineageOS —
    ver "Acceso al system" abajo) los lee de ahí ("read patch info:...").
  - md5 de los patches idéntico en: kernel nuestro, ~/gps-groundtruth/fw-stock y
    el /system/etc/firmware de LineageOS.
- **patch_1_1 verificado en la EMI física** (0xBB100000+0xE0000, ver devmem): el body
  aparece a partir de **+8** (los primeros 8 bytes = 0x00; ¿hueco by-design del ROM o
  finalización que falta?). El resto coincide con el fichero.
- EMI remap `0x10001310 = 0x1bb1` (base 0xBB100000) OK; OSC 26M (0x10001f00 bit10) OK;
  PALDO on devuelve 0/0; TOP1_PWR_CTRL=0xd y chipId=0x6582 = ground truth.

## El crash, acotado con el PC del MCU (CPUPCR 0x18070160)

Helper nuevo `mt6582_consys_cpupcr()` (consys) + ráfagas en el btif tras el RESET-2:

- **+0µs tras el EVT**: PC sano botando por 0x300–0xF6xx (boot/vectores del ROM).
- **+1ms**: PC en barrido LINEAL desde ~0x80000, a ~26 B/µs (nds32 @26MHz ejecutando
  memoria vacía), recorriendo 0x60000–0x100000 con wrap. Para siempre.
- **Ni un byte del fw en la EMI** (escaneo completo del MB: solo el patch_1_1).
  El crash es ANTES de cualquier init del fw.

**Interpretación**: el ROM arranca el boot del fw y el salto acaba en ~0x80000, donde
no hay nada en nuestro setup. En el stock DEBE haber código/datos ahí (¿alias de la
EMI? ¿sección que el ROM copia y a nosotros no nos copia? ¿el hueco de +8 debía
contener algo?).

## SIGUIENTE PASO (decisivo): dump del chip VIVO en LineageOS

Bootear **Android** (menupick) = LineageOS (CM, fstab.sprout). Su wmt levanta el chip
al arrancar (BT/GPS funcionan ahí). Desde el Pi (adb instalado):

```sh
adb devices && adb root
# EMI base de Lineage (memblock steal) en su dmesg:
adb shell dmesg | grep -iE "memblock done|consys start phy|EMI"
# leer memoria CHIP-SIDE por el enlace WMT vivo (resultado en dmesg):
adb shell "echo 'b 0x00060000 0xffffffff' > /proc/driver/wmt_dbg"  # hueco patch_1_0
adb shell "echo 'b 0x00060008 0xffffffff' > /proc/driver/wmt_dbg"  # body patch_1_0
adb shell "echo 'b 0x00080000 0xffffffff' > /proc/driver/wmt_dbg"  # zona del salto
adb shell "echo 'b 0xF00E0000 0xffffffff' > /proc/driver/wmt_dbg"  # hueco EMI chip-side
# CPUPCR sano de referencia:
adb shell "echo 'b 0x80000160 0xffffffff' > /proc/driver/wmt_dbg"  # ojo: CPUPCR es AP-side 0x18070160, leerlo con devmem/dd de /dev/mem
```
Diff contra nuestro estado → qué falta en 0x80000 / si el hueco de +8 va relleno.

## Acceso al system de LineageOS desde pmOS (útil)

El system NO está en las particiones MBR: ext4 real en el **sector 205824** del eMMC
(1GiB justo). Montaje: `losetup -o $((205824*512)) --sizelimit $((2097152*512)) /dev/loopX /dev/mmcblk0`.

## Ficheros

- `mt6582-btif.c.stpfull-0717` / `mt6582-consys.c.cpupcr-0717` (esta carpeta) = snapshot
  de los drivers de esta sesión (árbol vivo: `~/mainline/linux-7.0.12/...` en la Pi;
  backups previos `.pre-stpfull-0716-2021` y `.pre-cpupcr-0717`).
- Imagen de test: `~/mainline/pkg/boot-stpfull.img` (zImage nuevo + ganador-h2b.dtb +
  initrd-menupick). El kernel BUENO de diario sigue siendo boot-menupick13/#278.
