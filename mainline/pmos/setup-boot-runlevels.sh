#!/bin/sh
# setup-boot-runlevels.sh — restaura la config de runlevels de OpenRC del krillin (pmOS).
#
# RAIZ del "boot parcial / inestable" (0712): al runlevel `boot` le faltaban servicios
# ESENCIALES -> `root` (remonta / rw) no estaba en ningun runlevel -> `localmount`
# (need root) nunca arrancaba -> TODO lo que depende de localmount fallaba en cascada:
# dbus, networkmanager, polkit, bluetooth, chronyd, avahi... = sin system bus, sin red,
# sin sensores, con clock skew. El sistema arrancaba a medias (sshd/phosh por otro camino).
#
# Sintoma diagnostico: `rc-service <x> start` -> "ERROR: localmount needs service(s) root".
# Este script re-anade los servicios a sus runlevels. Idempotente (rc-update no duplica).
set -e
echo "[boot-runlevels] restaurando membresias de runlevel..."
# boot: base del sistema de ficheros + kernel
for s in root localmount swap sysctl modules hwclock; do
  rc-update add "$s" boot 2>/dev/null || true
done
# default: el system bus explicito (lo arrastran polkit/NM, pero mejor explicito)
rc-update add dbus default 2>/dev/null || true
echo "[boot-runlevels] listo. Runlevel boot:"
rc-update show 2>/dev/null | awk '/\| *boot/{print "  "$1}'
echo "[boot-runlevels] NOTA reloj: el RTC mt6323 debe estar puesto (hwclock -w) o habra"
echo "  clock skew hasta que chronyd sincronice. Con la bateria cargada el RTC lo mantiene."
