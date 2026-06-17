# sshd robusto al boot (mainline / Alpine)

Problema: tras arrancar, `sshd` no quedaba escuchando (puerto 22 cerrado pese a
ping OK); había que hacer `rc-service sshd restart` a mano. Causa: el init de
`sshd` tiene `need net`, pero en este equipo la red es la interfaz **gadget
`usb0`**, que sube **tarde** y por `/etc/local.d/usb0.start` (no es un servicio
OpenRC que provea `net`). Así que la dependencia `net` no se satisface a tiempo.

## Arreglo (dos capas)

1. **Quitar la dependencia `net`** — añadir a `/etc/conf.d/sshd`:
   ```
   rc_need="!net"
   ```
   sshd se ata a `0.0.0.0:22` y funciona en cuanto `usb0` aparezca (no necesita
   esperar a un proveedor de `net`).

2. **Red de seguridad** — copiar `zz-sshd.start` a `/etc/local.d/` (`chmod +x`).
   Corre al final del boot, después de `usb0.start`, y arranca sshd si no está.

Verificado: tras reiniciar, el puerto 22 abre solo a los ~20 s, sin tocar nada.
