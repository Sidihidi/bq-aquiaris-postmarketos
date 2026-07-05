#!/bin/sh
# WAYLAND_DISPLAY/XDG_RUNTIME_DIR ya estan seteados por phoc -E
# XDG_CURRENT_DESKTOP=Phosh:GNOME -> gnome-control-center activa sus paneles moviles
export XDG_CURRENT_DESKTOP=Phosh:GNOME
export XDG_SESSION_DESKTOP=phosh
export XDG_SESSION_TYPE=wayland
gdbus call --session --dest org.freedesktop.DBus --object-path /org/freedesktop/DBus \
  --method org.freedesktop.DBus.UpdateActivationEnvironment \
  "{'WAYLAND_DISPLAY': '$WAYLAND_DISPLAY', 'XDG_RUNTIME_DIR': '$XDG_RUNTIME_DIR', 'GDK_BACKEND': 'wayland', 'XDG_CURRENT_DESKTOP': 'Phosh:GNOME', 'XDG_SESSION_DESKTOP': 'phosh', 'XDG_SESSION_TYPE': 'wayland'}" >/dev/null 2>&1
# que gsd-power no duerma/atenue (el suspend del SoC no funciona aun)
gsettings set org.gnome.settings-daemon.plugins.power sleep-inactive-ac-type nothing 2>/dev/null
gsettings set org.gnome.settings-daemon.plugins.power sleep-inactive-battery-type nothing 2>/dev/null
gsettings set org.gnome.settings-daemon.plugins.power idle-dim false 2>/dev/null
/usr/libexec/gsd-power &
# Brillo por slider: el shim D-Bus provee org.gnome.SettingsDaemon.Power.Screen (Brightness)
# en ESTA sesion -> slider -> /run/mt6582-bl-pct -> daemon mt6582-backlight -> PWM.
[ -f /usr/local/bin/mt6582-bl-shim.py ] && python3 /usr/local/bin/mt6582-bl-shim.py >/dev/null 2>&1 &
sleep 1
# SIN exec: si phosh muere, phoc NO se entera solo (visto 0705: phosh muerto, phoc vivo,
# sesion zombi sin lockscreen). Al salir phosh tumbamos phoc ($PPID) -> dbus-run-session
# cae -> launch_phosh.sh (supervisor) relanza la sesion entera.
/usr/local/libexec/phosh
kill $PPID 2>/dev/null
