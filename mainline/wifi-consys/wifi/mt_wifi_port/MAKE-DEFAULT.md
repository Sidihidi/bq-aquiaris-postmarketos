# Hacer el port `mtk_mtwifi` el driver WiFi POR DEFECTO (built-in) — punto 2 de consolidación

Con el port validado (DHCP funciona), se integra en el kernel como **built-in** y se deshabilita el
driver A (`mt6582-wifi.c`), para que al boot el chip esté **frío nativo** (nadie descarga el FW antes) y
el port haga probe solo → `wlan0` lista para NetworkManager/Phosh. **Sin `unbind`/`insmod` manuales.**

## Los 3 edits del árbol del kernel (linux-7.0.12) + el config
Estos ficheros son del kernel downstream (no versionado en el repo); aplicar sobre el árbol de build:

1. **`drivers/net/wireless/Kconfig`** — enganchar el Kconfig del port:
   ```
   source "drivers/net/wireless/mediatek/Kconfig"
   source "drivers/net/wireless/mtk_mtwifi/Kconfig"      # <-- AÑADIR
   ```

2. **`drivers/net/wireless/Makefile`** — compilar el subdir del port:
   ```
   obj-$(CONFIG_MTK_MTWIFI) += mtk_mtwifi/               # <-- AÑADIR
   ```

3. **`drivers/soc/mediatek/Makefile`** — deshabilitar el driver A (REVERTIBLE):
   ```
   # obj-y += mt6582-wifi.o   # LEGACY driver A -> reemplazado por el port net/wireless/mtk_mtwifi (=y)
   ```
   (Dejar `mt6582-consys.o` y `mt6582-btif.o` como `obj-y`: el port los usa para el consys/func_on/VCN33.)

4. **`.config`** (build-krillin): `CONFIG_MTK_MTWIFI=y` (+ `make olddefconfig`).

El árbol del port (`drivers/net/wireless/mtk_mtwifi/`, versionado aquí como `mt_wifi_port/`) ya trae su
`Makefile`/`Kconfig`; con `CONFIG_MTK_MTWIFI=y` sus 40 objetos entran en el vmlinux.

## Build + flash
```
cd ~/mainline/linux-7.0.12
make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j2 zImage
# package: cat zImage dtb | mtk_hdr | abootimg -> boot img -> dd a mmcblk0 seek 83968 (magic ANDROID!)
```

## Revertir (si el port built-in fallara al boot)
Descomentar `obj-y += mt6582-wifi.o`, `CONFIG_MTK_MTWIFI=n` (o dejar el subdir sin -y), rebuild + flash.
El SD-boot que crashee cae a **boot INTERNO** (eMMC, con el sistema viejo) = red de seguridad HW.

## Verificación en HW (tras cold-boot del kernel nuevo)
- `dmesg | grep mtk_mtwifi` → "cargado" + "wlan0 ARRIBA" (probe nativo, sin unbind).
- `dmesg | grep mt6582-wifi` → VACÍO (driver A fuera).
- `ip link` → `wlan0` presente. `nmcli dev` → wlan0 gestionable → conectar por la GUI/NM.

*2026-07-03. Tras DHCP conseguido (HANDOFF-MTWIFI-PORT-DHCP-CONSEGUIDO-0703.md).*
