# 🔄 CONTINUAR — Linux MAINLINE 7.0.x en BQ Aquaris E4.5 (MT6582)

> Traspaso del trabajo de **mainline** (distinto del port pmOS 3.10, que está en
> README.md/CONTINUAR_AQUI.md). Última actualización: 2026-06-11.

## ⚡ TL;DR

Estamos portando **Linux mainline 7.0.12** al MT6582. **Arranca SMP 4 CPUs,
framebuffer, detecta la eMMC**. Quedaban 2 cosas para M3 (shell viva):
1. **USB gadget** — ✅ RESUELTO (g_ether + SOFTCONN; en builds busybox levantaba
   telnetd en 172.16.42.1). El problema de enumeración del lado host era ruido del
   preloader (cdc_acm), no del gadget.
2. **Montar la raíz ext4 de la eMMC** — ❌ ESTE es el bloqueante final, ya
   DIAGNOSTICADO (ver abajo).

## 🎯 DIAGNÓSTICO DEFINITIVO DEL BLOQUEANTE (confirmado por pantalla, v22/v24)

**El `mtk-sd` de mainline lee CEROS PUROS por el camino de datos de la eMMC.**
- Comandos OK: registra la tarjeta, lee CID/CSD/EXT_CSD → reporta bien `008G92
  7.28 GiB` (¡y SEC_COUNT del EXT_CSD es una lectura de datos de 512B que SÍ
  funcionó en el init!).
- Pero leer el superbloque ext4 (offset 1024 de mmcblk0p5): `read()` devuelve 512,
  **sin error**, pero `nonzero=0/512` → **512 bytes a cero**. No es basura: ceros.
- mount ext4 → falla EINVAL ("no es ext4") porque el superbloque leído es ceros.

**Interpretación:** el DMA "completa" sin error pero los datos nunca llegan al
buffer del CPU → camino DMA roto. Sospechas (orden): (1) timing/sampling a 12MHz
mal → probar 400kHz; (2) coherencia de caché / `dma-coherent`; (3) dirección
física DMA / dma-ranges mal para MT6582; (4) descriptor GPD/BD del MSDC.

## 🧪 PRÓXIMOS EXPERIMENTOS (en orden, el 1º es lo más barato)

Todos en el DTS `linux-7.0.12/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts`,
nodo mmc. Recompilar SOLO el dtb es rápido (`make ... dtbs`).

1. **Bajar el reloj al mínimo legacy**: `max-frequency = <400000>;` y QUITAR
   `cap-mmc-highspeed`. Fuerza el modo más lento. Si las lecturas dan datos →
   es timing/tuning → subir reloj bisecando. (HIPÓTESIS #1, más probable.)
2. **Coherencia DMA**: añadir `dma-coherent;` al nodo mmc. Si lo arregla → era
   caché. (Trivial.)
3. **Forzar PIO** (sin DMA) en mtk-sd.c: parchear `msdc_start_data`/
   `msdc_prepare_data` para no usar DMA. Más invasivo; si 1 y 2 fallan.
4. Revisar el `compatible` (usamos `mediatek,mt8135-mmc`): comparar el
   `mtk_mmc_compatible`/quirks de mt8135 vs lo que necesita un MSDC viejo
   (mt6589/mt6582) en mtk-sd.c.

> ⚠️ Investigación pendiente: el agente sobre "mtk-sd lee ceros DMA" NO llegó a
> correr (límite de gasto mensual). Relanzarlo o investigar a mano mtk-sd.c.

## 🟡 PROBLEMA SECUNDARIO: framebuffer amarillo (NO bloqueante)

El simplefb (fb del LK en 0xBF400000, 540x960, 32bpp, stride 2176 = 544×4,
formato a8r8g8b8 — todo correcto, NO tocar). El tinte amarillo NO es swap de
canales (negro→amarillo lo descarta). El auto-fix de capas OVL (v24) NO lo quitó
→ probablemente es **gamma/VCOM del panel hx8389** que deja el LK (su init LCM ≠
el del kernel Android). Fix real = driver DSI/panel, no hay poke que lo arregle.
Mapa completo de registros DISP en memoria: `reference_mt6582_disp_tint.md` y
fuentes en `research_mt6582_disp/` y `research_mt6582_lk/`.
**Aun amarillo, el usuario PUEDE leer los logs en pantalla** → sirve de canal.

## 🛠️ FLUJO DE TRABAJO (lo que funciona)

- **Máquina de compilación**: `cpcd@192.168.0.38` (pwd `cpcdupct`, sudo sin pwd).
  Kernel en `~/mainline/linux-7.0.12/build-krillin`, paquetes en `~/mainline/pkg/`.
- **Compilar**: `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage dtbs`
  luego `cd ~/mainline/pkg && ./assemble.sh` → `boot-mainline.img`.
- **Init de diagnóstico**: `tiny_init_vNN.c` (PID 1, imprime veredicto a /dev/kmsg
  → consola → pantalla, en bucle legible). v23 es el bueno (lee y reporta sin
  depender de escribir en eMMC). Compilar estático armv7 e incrustar en initramfs.
- **CANAL DE LECTURA**: ❗los ojos del usuario en la pantalla. El UART
  (/dev/serial0 de la Pi) NO entrega datos (no cableado o cmdline sin console=ttyS).
  ramoops no lo lee pmOS (sin pstore/devmem). Escribir verdict a p6 NO sirve
  (el data-path eMMC está roto → escribe ceros). Así que: **imprimir en pantalla
  y que el usuario lea** es el método que funciona ahora.
- **Flashear mainline**: SIEMPRE verificar. Por fastboot (fiable) o por `dd` con
  `flash_boot_dd_v2.sh` (verifica md5 releído, aborta sin reiniciar si difiere).
  El `dd` a través de /tmp del teléfono se truncaba → `dd_v24_safe.py` verifica
  md5 del scp con reintentos. NUNCA reiniciar sin "FLASH_VERIFICADO_MD5_OK".
- **Recuperar a pmOS 3.10**: fastboot → `fastboot flash boot ~/boot-pmos.img`.
  (boot-pmos.img se trajo de la Pi .112 con pmbootstrap.)
- **Ciclo típico**: pmOS accesible por `ssh user@172.16.42.1` (pwd 147147, clave
  instalada). Pi a `172.16.42.2/24` en usb0. Para mainline: flashear → arrancar →
  usuario lee pantalla → Power+Vol+ a fastboot → restaurar pmOS.

## 📊 HITOS

- M1 ✅ kernel arranca (SMP, fb, consola)
- M2 ✅ eMMC detectada + USB gadget + (display con tinte)
- **M3 ⏳ shell viva**: bloqueado SOLO por el mtk-sd-lee-ceros. Resuelto eso →
  monta raíz → switch_root → userspace → telnet shell. **Estamos a 1 fix.**
- M4 (futuro) display DRM real → Lima → GPU; M5 módem CCCI.
