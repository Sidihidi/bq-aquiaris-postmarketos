# HANDOFF — KASAN + DHCP=RX + cómo seguir en casa (2026-06-26)

Estado tras una sesión muy larga. **3 hitos reales hoy** + el muro actual bien acotado + la herramienta
(KASAN) montándose. Para retomar en casa sin reconstruir nada.

## TL;DR — lo conseguido + lo que falta
- 🎯 **IRQ del HIF DISPARA** (`GIC_SPI 184`, no 160). Ver `HANDOFF-IRQ-RESUELTO-0626.md`. Commiteado.
- 🎯 **Conexión FIABLE** (grant-fix, commit `e951401`): `wifi_wait_grant` sondeaba `MCR_WRPLR` CRUDO (método
  roto que estancaba el RX del FW en mitad del handshake) → ahora usa `wifi_rx_drain` (bloque enhance,
  read-clear correcto). Conecta al 1er intento, AUTH-2→ASSOC-RESP→JoinComplete, cero BSS-ABSENCE.
- 🎯 **DHCP = problema RX, CONFIRMADO con dato (no intuición)**: instrumenté `TX-DONE` en `wifi_tx_data`
  (`pkt_seq!=0`) → el DISCOVER (seq=3, DATOS) sale con **status=0 (transmitido)**. **El TX funciona.**
  La OFFER (broadcast, cifrada con GTK) **NO vuelve a RX0**. Recibimos beacons (pt=3=MGMT) + heartbeats
  (pt=1=EVENT, eid=0x0e) pero **NINGÚN frame de datos (pt=0) post-GTK**.
- 🐛 **2 crashes cfg80211** (bloquean el testeo): `cfg80211_scan_done` UAF (rx_thread, `scan_req=0x70000001`
  basura) + `cfg80211_put_bss` UAF (kworker `cfg80211_event_work`, BSS basura `0x01ffffe4`). Los punteros
  basura están en **NUESTRO** struct → **corrupción de memoria**. Los dispara el scan en bucle de NM.
  **Workaround**: `nmcli radio wifi off` + `rfkill block wifi` → sin scan → sin crash (móvil estable).

## ⏩ ESTADO EXACTO AL CERRAR + QUÉ CAPTURAR (para retomar en casa)

**YA LISTO en la Pi .123** (`~/mainline/pkg/`):
- `boot-kasan.img` (21 MB) — kernel KASAN OUTLINE empaquetado y **cabe en la partición** (28.5 MB).
- `bootimg-kasan.cfg` — con **`ramdiskaddr=0x15000000`** (movido: el kernel DESCOMPRIMIDO es **57 MB**, así
  que el ramdiskaddr original `0x11000000` lo pisaba; lo subí a 80 MB, bien después del kernel).
- zImage KASAN = 20 MB (vs 14 normal). Config: `KASAN_GENERIC + OUTLINE + STACK + VMALLOC`.

**Lo último que se lanzó**: flash de `boot-kasan.img` + `reboot -f`. **FALTA CONFIRMAR si arranca** (1er boot
de un kernel de 57 MB con el ramdisk reubicado; puede que no a la primera).

### PASO 1 — ¿arranca el kernel-KASAN?
```bash
ssh root@172.16.42.1 'uname -a | grep -o "#[0-9]*"; dmesg | grep -iE "KASAN|KernelAddress" | head'
```
- Responde + "KernelAddressSanitizer initialized" → ✅ KASAN vivo → PASO 2.
- **NO arranca** (No route to host / bootloop) → recuperar con un boot bueno (`boot-184.img` por fastboot o
  dd a mmcblk0 83968) y luego: subir el ramdiskaddr a `0x16000000`, o reducir el kernel
  (`scripts/config -d KASAN_STACK -d KASAN_VMALLOC` y rebuild).

### PASO 2 — provocar la corrupción y CAPTURAR el informe  ⭐ ESTO ES EL OBJETIVO
```bash
ssh root@172.16.42.1 'wpa_passphrase hola kakatua1 > /tmp/wpa.conf
nmcli device set wlan0 managed no; ip link set wlan0 up
wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -B'
# dejar 1-2 min (que escanee/conecte). KASAN salta al PRIMER acceso malo. Guardar:
ssh root@172.16.42.1 'dmesg | grep -B2 -A45 "BUG: KASAN"' > kasan-report.txt
```
**QUÉ MIRAR en el informe (esto resuelve el crash, sin adivinar):**
1. `BUG: KASAN: <tipo> in <FUNCIÓN>+0x..` ← **la función que corrompe**.
2. `Write of size N at addr 0x..` ← qué escribe de más y dónde.
3. `Call Trace:` ← la pila exacta (qué línea de `mt6582-wifi.c` o cfg80211).
4. Apuesta: un `slab-out-of-bounds`/`stack-out-of-bounds` **write** de nuestro driver que pisa
   `scan_req`/`connect_bss` (ambos en el struct `mt6582_wifi`). Con la línea exacta, el fix es directo.

### PASO 3 — fix + DHCP
- Arreglar el overflow que cante KASAN (acotar el buffer / corregir índice/longitud).
- Crash muerto → atacar el DHCP-RX (sección de abajo): RX_FILTER del JoinComplete / GTK / OFFER unicast.
- **Quitar KASAN** del config y rebuild normal cuando se cierre (kernel de 57MB es solo para debug).

## POR QUÉ KASAN
La corrupción NO se caza leyendo (probé 5 hipótesis: scan_req UAF, hif_lock antipattern, overflow de `dlm`,
`flen` del inform_bss, profundidad de stack — ninguna confirma). KASAN instrumenta cada acceso a memoria y,
en cuanto algo escribe fuera de su sitio, suelta el informe EXACTO: **función + línea + buffer**. Cero adivinar.

## CÓMO COMPILAR EL KERNEL pmOS CON KASAN (en la Pi .123, `~/mainline/linux-7.0.12`)
```bash
cd ~/mainline/linux-7.0.12
cp build-krillin/.config build-krillin/.config.bak-pre-kasan          # backup

# 1) activar KASAN + silenciar avisos de frame (KASAN los infla)
scripts/config --file build-krillin/.config -e KASAN --set-val FRAME_WARN 0
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- olddefconfig

# 2) ⚠️ CLAVE: olddefconfig elige INLINE -> zImage de 29MB = NO CABE en el krillin (ver tamaños abajo).
#    Cambiar a OUTLINE (mete los chequeos en funciones; código MUCHO más pequeño):
scripts/config --file build-krillin/.config -d KASAN_INLINE -e KASAN_OUTLINE
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- olddefconfig

# 3) build (recompila TODO porque cambia el modo KASAN; ~10-15 min en la Pi)
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage
ls -la build-krillin/arch/arm/boot/zImage     # MIRAR EL TAMAÑO
```

### ⚠️ RESTRICCIONES DE TAMAÑO del krillin (el obstáculo real)
El `boot.img` tiene DOS límites duros:
- **Hueco kernel→ramdisk = 16 MB**: `kerneladdr=0x10008000`, `ramdiskaddr=0x11000000` (en `bootimg-sd.cfg`).
  Si el zImage > 16 MB, pisa el ramdisk en RAM → no bootea.
- **Partición de boot ≈ 28.5 MB**: el boot está en el sector **83968** (=41 MB), `sec_ro` empieza en ~69.5 MB
  (`blkdevparts=...6144K@71168K(sec_ro)...`). Un boot.img > 28.5 MB se sale a `sec_ro`.

Por eso INLINE (29 MB) NO vale. Con OUTLINE, según el tamaño que salga:
- **zImage < 16 MB** → empaquetar y flashear normal (cabe con las direcciones actuales).
- **zImage 16–28 MB** → mover `ramdiskaddr` en `bootimg-sd.cfg` (p.ej. `0x12000000` = 32 MB de hueco) para
  darle sitio, y empaquetar. (El ramdisk es pequeño y la RAM es 1 GB, hay sitio.)
- **zImage > 28 MB** → recortar más: `scripts/config -d KASAN_STACK -d DEBUG_INFO` (o `DEBUG_INFO_REDUCED`).

### Empaquetar + flashear (con el zImage KASAN)
```bash
cd ~/mainline/pkg
DTB=~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dtb
cat ~/mainline/linux-7.0.12/build-krillin/arch/arm/boot/zImage "$DTB" > zimage-dtb
python3 mtk_hdr.py KERNEL zimage-dtb zimage-dtb-mtk
abootimg --create boot-kasan.img -f bootimg-sd.cfg -k zimage-dtb-mtk -r initrd-sd-mtk
ls -la boot-kasan.img        # confirmar < 28.5 MB
# flash por dd (móvil vivo en pmOS) + reboot -f (¡con -f! Phosh bloquea el reboot normal):
sudo ip addr replace 172.16.42.2/24 dev usb0
scp boot-kasan.img root@172.16.42.1:/tmp/b.img
ssh root@172.16.42.1 "dd if=/tmp/b.img of=/dev/mmcblk0 bs=512 seek=83968 conv=fsync; sync; sleep 1; reboot -f"
```

### Reproducir el crash + leer el informe KASAN
```bash
# tras bootear el kernel-KASAN, conectar a 'hola' y dejar que escanee (NM ya escanea solo):
ssh root@172.16.42.1 'wpa_passphrase hola kakatua1 > /tmp/wpa.conf
nmcli device set wlan0 managed no; ip link set wlan0 up
wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -B'
# KASAN salta al PRIMER acceso malo (mucho antes del crash). Leer el informe:
ssh root@172.16.42.1 'dmesg | grep -A40 "BUG: KASAN"'
```
El informe dice: `Write of size N at addr X` + la pila (función+línea que corrompe `scan_req`/`connect_bss`).
Con eso el fix es directo — y, como intuyó el usuario, **probablemente destape también el DHCP-RX** (misma corrupción).

## EL DHCP, una vez estable (= RX)
TX confirmado. La OFFER broadcast cifrada no llega. Atacar (en orden):
1. **RX_FILTER del JoinComplete** — ¿filtra el group-data (broadcast)? Comparar con el downstream.
2. **GTK** — ¿bien instalada/aplicada? (`add_key idx=1 cipher=0xfac04 peer=ff:ff:ff:ff:ff:ff len=16`).
3. **OFFER unicast** — `udhcpc` con el broadcast flag a 0 → la OFFER vendría unicast (PTK) en vez de broadcast (GTK).

## Idea LineageOS-KASAN (del usuario)
Buena, PERO su kernel es el **3.10** y KASAN entró en el **4.0** → ahí no compila. Para comparar con el
downstream: ftrace/printk en sus puntos de RX/scan/decrypt. (El de pmOS, reciente, sí lleva KASAN — ese es el bueno.)

## Notas técnicas
- **pt (HIF RX packet types, mask 0x3)**: DATA=0, EVENT=1, MGMT=3.
- Driver con DIAG: `DIAG-DRAIN` (whisr/n0/n1/l0/l0b/mbox0) + `DIAG-RX0` (pt/l0/bytes) + TX-DONE en datos
  (`pkt_seq!=0`). **Quitar todos los DIAG** cuando se cierre.
- Iterar: editar driver → scp a la Pi → `make zImage` (+ `make dtbs` SOLO si tocas el DT) → empaquetar → dd a
  mmcblk0 seek 83968 → `reboot -f`.
