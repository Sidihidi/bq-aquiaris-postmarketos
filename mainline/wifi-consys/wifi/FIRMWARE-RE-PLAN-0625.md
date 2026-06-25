# MISIÓN 1 — Plan de RE del firmware original (2026-06-25, sesión autónoma)

Encargo: crear y ejecutar un plan para hacer ingeniería inversa al firmware WiFi original del MT6582.
Aquí: lo **ejecutado sin reinicio** (el usuario estaba fuera; un boot colgado necesita power-cycle
físico) + el plan completo con el código **listo para flashear** para la parte que requiere reinicio.

---

## TL;DR
- El blob WIFI_RAM_CODE va **CIFRADO** (entropía 7.99). El **cifrado es chip-side**: el host lo envía con
  el flag `DOWNLOAD_BUF_ENCRYPTION_MODE` y **el ROM del chip lo descifra al descargarlo** → el host NUNCA
  tiene el plaintext del blob. NO se puede descifrar el fichero por software.
- **PERO el firmware descifrado vive en la RAM del MCU y SÍ lo podemos LEER** vía `INIT_CMD_ID_ACCESS_REG`
  (una palabra por comando). Nuestro driver ya lo hace (`wifi_access_reg_read`, `mt6582-wifi-PI.c:327`;
  el "FWdump" del boot vuelca 64 palabras de 0x6a000). **→ El RE ES FACTIBLE: volcar la RAM descifrada y
  desensamblar.**
- **Hay DOS MCUs:** el **CONN_MCU** (conectividad/WMT, su PC = `CONN_MCU_CPUPCR` 0x18070160) y el **MAC
  WiFi** (donde corre WIFI_RAM_CODE, en `0x6a000`). El que nos interesa es el MAC WiFi.
- ⚠️ **CAVEAT IMPORTANTE:** la Misión 2 ya localizó que el **fix del muro WPA2-DHCP es HOST-SIDE**
  (Experimento A: leer el bloque enhance de WHISR). **El RE del firmware NO es necesario para el fix** —
  es para entender el "porqué" (teoría 3) y como seguro si A no bastara. Priorizar el Experimento A.

---

## Lo EJECUTADO esta sesión (sin reinicio)

### 1. Confirmado: cifrado chip-side (no se descifra el fichero)
`common/wlan_lib.c` (`wlanImageSectionDownloadAggregated:3042`): con `CFG_ENABLE_FW_ENCRYPTION` el host
pone el flag `DOWNLOAD_BUF_ENCRYPTION_MODE` en cada chunk; el chip descifra. No hay clave ni algoritmo en
el source del driver → **el plaintext solo existe en la RAM del MCU tras la descarga.**

### 2. Confirmado: la RAM descifrada es legible (RE factible)
`wifi_access_reg_read(addr)` (`mt6582-wifi-PI.c:327-360`) usa `INIT_CMD_ID_ACCESS_REG` (query) y devuelve
la palabra de 32 bits en `addr`. El FWdump del boot (`:1612-1614`) lee `0x6a000 + i*4` para i=0..63. Los
bytes salen **estructurados** (no aleatorios) → el chip descifró bien y los leemos correctamente.

### 3. Muestra de FW descifrado (0x6a000, blob SOC cargado) + desensamblado → INCONCLUSO
Volqué los 256 bytes del FWdump actual y los desensamblé (objdump de la Pi) en ARM/Thumb: **no es código
lineal limpio** — es una **tabla/loader** (registros de 32B que se repiten cada 0x20: `10440800 101c0000
014c0100 ....fe7f`). Coherente con que en el blob **SOC** a `0x6a000` solo va un **stub de 6048B**; el
código grande del SOC está en **`0xf0020000`** (180720B). En el blob **160480** sí va el código grande a
`0x6a000` (151552B). → Para desensamblar código real hay que volcar esas regiones grandes (paso 1 del
plan, requiere reinicio).

**Arquitectura del MAC WiFi:** sin confirmar (la muestra era tabla). Candidatos a probar al desensamblar
el código real: **ARM Thumb-2** (algunos `adr`/`ldr [pc]` plausibles en la muestra), **Andes (nds32)** o
**Xtensa** (típicos en MCUs de conectividad MediaTek). El disassembler decide: el arch correcto produce
código sano (prólogos de función, saltos coherentes).

---

## EL PLAN (pasos que requieren reinicio → listos para cuando vuelvas)

### Paso 1 — Volcar la RAM de código descifrada COMPLETA (driver + reinicio)
La forma actual (FWdump de 64 palabras a dmesg) no escala (151552B = ~38000 palabras; dmesg=4M no llega).
**Añadir una interfaz debugfs de volcado por rango** que lee vía `wifi_access_reg_read` y entrega hex:

```c
/* debugfs: echo "<addr> <nwords>" > /sys/kernel/debug/mt6582_wifi/fwdump_cfg
 *          cat /sys/kernel/debug/mt6582_wifi/fwdump   -> "addr=val\n" por palabra */
static u32 g_dump_addr, g_dump_words;
static ssize_t fwdump_cfg_write(struct file *f, const char __user *u, size_t n, loff_t *o)
{ char b[32]={0}; if(n>=sizeof(b))return -EINVAL; if(copy_from_user(b,u,n))return -EFAULT;
  sscanf(b,"%x %u",&g_dump_addr,&g_dump_words); return n; }
static int fwdump_show(struct seq_file *m, void *v)
{ struct mt6582_wifi *w=g_wifi; u32 i; mutex_lock(&w->hif_lock);
  for(i=0;i<g_dump_words && i<262144;i++)
    seq_printf(m,"%08x=%08x\n", g_dump_addr+i*4, wifi_access_reg_read(w, g_dump_addr+i*4));
  mutex_unlock(&w->hif_lock); return 0; }
/* registrar en wifi_probe junto al 'bringup' existente:
 *   debugfs_create_file("fwdump_cfg",0200,w->dbg,w,&fwdump_cfg_fops);
 *   debugfs_create_file("fwdump",0400,w->dbg,w,&fwdump_fops);  // single_open(fwdump_show) */
```
Uso (1ª conexión de boot fresco, NO conectado, para no chocar con el hif_lock del rx):
```sh
# blob 160480 (código a 0x6a000, 151552B): revertir el firmware antes (ver abajo) y:
echo "6a000 37888" > /sys/kernel/debug/mt6582_wifi/fwdump_cfg
cat /sys/kernel/debug/mt6582_wifi/fwdump > /root/fw_6a000.txt
# blob SOC (código a 0xf0020000, 180720B):
echo "f0020000 45180" > /sys/kernel/debug/mt6582_wifi/fwdump_cfg
cat /sys/kernel/debug/mt6582_wifi/fwdump > /root/fw_f0020000.txt
```
(~38000 lecturas ACCESS_REG ≈ varios minutos; cada una es un CMD+evento por el HIF. Hacerlo SIN conexión
activa para no competir con el rx_thread por el hif_lock.)

> Revertir al blob 160480 (su 0x6a000 = código principal, el más directo de desensamblar):
> `cp /lib/firmware/mediatek/mt6582/WIFI_RAM_CODE.bak-160480-claude /lib/firmware/mediatek/mt6582/WIFI_RAM_CODE` + reboot.

### Paso 2 — Convertir el dump a binario e identificar la arquitectura
```sh
# de "addr=val" a binario LE
awk -F= '{printf "%s",$2}' fw_6a000.txt | xxd -r -p ... # (script: empaquetar cada val como u32 LE)
# probar arquitecturas (la que dé código sano gana):
arm-linux-gnueabihf-objdump -D -b binary -m arm -M force-thumb --adjust-vma=0x6a000 fw.bin | less
# si no: Andes nds32 (toolchain andes) o Xtensa (xtensa-esp32-elf-objdump) o radare2/Ghidra (auto-arch)
```
Ghidra es lo ideal (auto-detecta arch + análisis de funciones). Cargar `fw.bin` en base `0x6a000`.

### Paso 3 — Localizar las rutinas clave (entender el "porqué")
Buscar en el desensamblado: el manejo de la **GTK/WTBL** (por qué deja de re-armar RX0_DONE tras la
group-key), el **RX DMA / el bloque enhance de WHISR** (cómo el FW rellena `ENHANCE_MODE_DATA_STRUCT_T`),
y el descifrado broadcast. Cruzar con los símbolos/strings del source downstream (mismos nombres de cmd).

### Paso 4 (alternativa de verdad-terreno) — extraer el blob/driver de LineageOS
La partición Android interna (`mmcblk0`, partición `android@91136K`) tiene el driver `mt_wifi` original y
su `WIFI_RAM_CODE`. Montarla (read-only) y comparar: (a) ¿qué blob carga el stock?; (b) su `.ko` (binario,
RE con Ghidra) para ver el data-path real. Esto desambigua el Experimento A sin desensamblar el FW cifrado.

---

## Conclusión de la Misión 1
- **RE factible**: el FW se descifra en RAM y lo leemos por ACCESS_REG; falta extender el volcado (Paso 1,
  requiere reinicio) y desensamblar (Pasos 2-3).
- **El cifrado del fichero es un muro** (chip-side, sin clave en el source) → NO se ataca el blob; se ataca
  la RAM descifrada.
- **Pero el fix NO depende de esto**: la Misión 2 mostró que el muro WPA2-DHCP se cierra host-side
  (Experimento A, bloque enhance de WHISR). El RE del firmware es el plan B / la confirmación de la
  teoría 3, no el camino crítico.
