# Maemo Leste: internet por USB (NAT) + arreglo de apt (keyring Devuan) — 0712

> Cómo dar internet a Maemo por el enlace USB a la Pi de build y dejar apt funcionando (autenticado).
> El WiFi de Maemo funciona pero el internet por el repetidor TKIP está roto (hueco conocido); el USB
> es la vía fiable para actualizar/instalar.

## 1. Internet por USB (compartir la conexión de la Pi vía NAT)
Topología: móvil `usb0=172.16.42.1` ↔ Pi `usb0=172.16.42.2` ↔ (wlan0) internet.

**En la Pi** (nftables; `ip_forward` ya era 1; FORWARD policy accept):
```sh
sudo nft add table ip nat
sudo nft 'add chain ip nat postrouting { type nat hook postrouting priority 100 ; }'
sudo nft add rule ip nat postrouting ip saddr 172.16.42.0/24 oifname "wlan0" masquerade
```
**En el móvil (Maemo)**:
```sh
ip route del default 2>/dev/null
ip route add default via 172.16.42.2 dev usb0
printf 'nameserver 1.1.1.1\nnameserver 8.8.8.8\n' > /etc/resolv.conf
```
Test: `ping 1.1.1.1` + `getent hosts deb.debian.org` → OK. (No persistente; reaplicar tras reboot de
cualquiera de los dos, o meterlo en local.d del móvil + nft.conf de la Pi.)

## 2. Gotcha de apt: falta el keyring de Devuan → paquetes "cannot be authenticated"
Maemo Leste es **basado en Devuan**. Los paquetes base vienen de `deb.devuan.org/merged excalibur`,
firmado con la clave **`9F8D6C74DE661075FD171BE3B3982868D104092C` = "Devuan Release Signing (Excalibur)"**,
que **faltaba** en el keyring de este rootfs → `E: The repository ... is not signed` y todo el upgrade
salía como no autenticado.

**NO usar `--allow-unauthenticated`** (salta la verificación GPG). Fix correcto:
1. Verificar el fingerprint contra la web OFICIAL (HTTPS) https://www.devuan.org/os/keyring
   (Excalibur = `9F8D 6C74 DE66 1075 FD17 1BE3 B398 2868 D104 092C`). ✔
2. Descargar el .deb del keyring y **verificar que contiene ese fingerprint** antes de instalar
   (así, aunque el .deb venga por HTTP, se valida su contenido contra lo verificado por HTTPS):
   ```sh
   URI=$(apt-get download --print-uris devuan-keyring | awk "{gsub(/'/,\"\",\$1);print \$1}" | head -1)
   curl -fsSL "$URI" -o /tmp/dk.deb && dpkg-deb -x /tmp/dk.deb /tmp/dk
   gpg --no-default-keyring --keyring /tmp/dk/usr/share/keyrings/devuan-archive-keyring.gpg \
       --list-keys --with-colons | grep 9F8D6C74DE661075FD171BE3B3982868D104092C   # debe aparecer
   install -m0644 /tmp/dk/usr/share/keyrings/devuan-archive-keyring.gpg /etc/apt/trusted.gpg.d/
   apt-get update    # 0 errores de firma
   ```

## 3. Actualización aplicada (0712)
`apt update` + `dpkg --configure -a` (arregla hildon-input-method a medias) + `apt upgrade`
(`--allow-downgrades` por `libcal1`, versión de Maemo; SIN `--allow-unauthenticated`). 47 paquetes
base + apt/apt-utils/libapt-pkg7.0 + gpgv. dpkg limpio, disco 25G libre. Carga viva todo el rato
(daemon fan5405 = sin apagones por undervoltage, a diferencia de intentos previos).

## ⚠️ NO hacer `apt full-upgrade` a ciegas
`full-upgrade` quiere instalar **1196 paquetes nuevos** (fuentes de todos los scripts + l10n de todos
los idiomas + plugins de teclado hildon) = bloat masivo, NO "lo que falta". Instalar apps concretas
por nombre, no el árbol de recommends entero. Los `autoremove` sugeridos (mesa-libgallium, libllvm19,
startpar) NO quitar sin pensar (mesa-libgallium lo usa GL/lima).

*Sesión principal (Fable 5), 2026-07-12.*
