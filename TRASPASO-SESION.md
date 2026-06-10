# 🔄 TRASPASO DE SESIÓN — 2026-06-10 → casa

> Para la sesión de Claude en casa: lee este archivo primero, luego
> `CONTINUAR_AQUI.md` (acceso/credenciales/historia) y `mainline/README.md`
> (guía técnica mainline completa). Con esos tres tienes TODO el contexto.

## 🏆 Lo conseguido hoy (sesión épica)

1. **Port pmOS reconstruido desde cero** en la Pi nueva (`cpcd@192.168.0.123`):
   kernel 3.10 + rootfs OpenRC + Sxmo + táctil evdev + WiFi pendiente.
   El teléfono es un daily-driver estable y auto-recuperable.
2. **M1 — Linux 7.0.12 mainline ARRANCA** en el krillin: 4 CPUs, consola
   simple-framebuffer en pantalla, userspace propio. Primera vez en este hardware.
3. **M2 — eMMC + ext4 EN MAINLINE**: driver mtk-sd parcheado por nosotros
   (pinctrl opcional), particiones vía `blkdevparts=`, y la root de pmOS
   **montada y listada en pantalla** por el kernel 2026.

## ⚠️ ESTADO FÍSICO DEL TELÉFONO AL CIERRE

- En la partición `bootimg` está flasheada **boot-mainline-v8.img** (mainline).
  → al encender, el teléfono arranca el test M2 (pingüinos + listado de la root).
  **NO arranca pmOS hasta restaurar el boot.**
- La rootfs de pmOS en `mmcblk0p5` está INTACTA (v8 la monta read-only).
- El combo Power+Vol+ dejó de responder al final (¿pulsación corta?); se iba a
  sacar la batería. **Tras quitar batería: los logs ramoops en RAM se pierden**
  (sin más consecuencia; M2 ya documentado).

### Para volver a pmOS (5 min)
1. Batería fuera y dentro. **Mantener Vol+ y conectar USB** → fastboot.
2. En la Pi: `sudo fastboot flash boot ~/pmos-artifacts/boot-pstore.img && sudo fastboot reboot`
   (la imagen también está en `artifacts/boot-pstore.img` de este repo... NO:
   los .img no se suben a git — está en la Pi y en el Mac del trabajo en
   `pmos-krillin/artifacts/`. Si no la tienes: regenerar con `mainline/README.md` §4
   o reconstruir todo con `CONTINUAR_AQUI.md` §5).
3. pmOS arranca solo a Sxmo. SSH: `user@172.16.42.1` / `147147`.

## 📍 Dónde está cada cosa

| Qué | Dónde |
|---|---|
| Pi de construcción | `ssh cpcd@192.168.0.123` (¿accesible desde casa? si no, todo lo crítico está en este repo) |
| Árbol mainline compilado | Pi: `~/mainline/linux-7.0.12` (build en `build-krillin/`, YA con parche mtk-sd) |
| Herramientas empaquetado | Pi: `~/mainline/pkg/` y repo: `mainline/pkg/` |
| Imágenes boot (pmOS/mainline) | Pi: `~/pmos-artifacts/` y `~/mainline/pkg/`; Mac trabajo: `pmos-krillin/artifacts/` |
| dts del krillin (v8, con MSDC) | repo: `mainline/dts/mt6582-bq-krillin.dts` |
| Parche driver | repo: `mainline/patches/0001-mmc-mtk-sd-make-pinctrl-optional.patch` |
| Config kernel mainline | repo: `mainline/config-mainline-krillin` |
| Logs de cada iteración | repo: `mainline/dmesg-mainline-v{3,5,6,7}.txt` (v8 perdido por batería) |
| aports pmOS (3.10) | repo: `aports/` |

## 🧭 Por dónde seguir (en orden sugerido)

1. **Restaurar pmOS** (arriba) y verificar Sxmo. 
2. **WiFi en pmOS** (pendiente de reaplicar; en la sesión antigua funcionó —
   firmware MTK, ver scripts `wifi*/firmware` de la sesión original).
3. **Rootfs Alpine mínima en `usrdata` (p7, 5.7 GB)** para mainline: busybox +
   getty en fbcon → un sistema completo arrancando en mainline. El init actual
   (mainline/pkg/tiny_init.c) ya sabe montar; cambiarlo por switch_root.
4. **M2b — USB gadget en mainline**: portar el driver musb del MT6582
   (downstream: `drivers/misc/mediatek/usb20/`) o probar el musb genérico con
   nodo DT (base 0x11200000 aprox — verificar en downstream). Con red USB →
   SSH en mainline → desarrollo rápido.
5. **M3 — display**: driver DSI/DISP propio (mata el amarillo, el decouple y
   abre la puerta a DRM → Lima → GPU → **Phosh**).

## 💡 Trampas conocidas (no tropezar dos veces)

- El LK **ignora el cmdline del boot.img** → todo por bootargs del DT (CMDLINE_EXTEND).
- El parser MSDOS de 7.x **no traga el MBR de MTK** → `blkdevparts=` (ya en el dts v8).
- El init de mainline **debe montar devtmpfs** o no hay /dev/mmcblk0pN.
- `fastboot boot` (RAM) no funciona en este LK; `reboot2 bootloader` sí (binario en pkg/).
- **Verificar md5 antes de cualquier dd-flash** (una subida truncada = logo BQ congelado).
- /tmp del teléfono es tmpfs pequeño: limpiar imágenes viejas antes de subir otra.
- En el kernel 3.10: NUNCA tocar `fb0/blank` (pánico). `fb0/pan` es seguro.
- ramoops en `0xBF300000` sobrevive reboots calientes pero NO al corte de batería.

## 🔑 Credenciales y accesos

Ver tabla completa en `CONTINUAR_AQUI.md` §2. Esencial: teléfono
`user@172.16.42.1` pass `147147`; Pi `cpcd@192.168.0.123` (clave SSH del Mac
del trabajo — desde casa, añadir tu clave o usar contraseña de cpcd).
