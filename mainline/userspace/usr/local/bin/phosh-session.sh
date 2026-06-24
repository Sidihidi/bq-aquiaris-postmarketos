#!/bin/sh
# WAYLAND_DISPLAY/XDG_RUNTIME_DIR ya estan seteados por phoc -E
# XDG_CURRENT_DESKTOP=Phosh:GNOME -> gnome-control-center activa sus paneles moviles
export XDG_CURRENT_DESKTOP=Phosh:GNOME
export XDG_SESSION_DESKTOP=phosh
export XDG_SESSION_TYPE=wayland
gdbus call --session --dest org.freedesktop.DBus --object-path /org/freedesktop/DBus \
  --method org.freedesktop.DBus.UpdateActivationEnvironment \
  "{'WAYLAND_DISPLAY': '$WAYLAND_DISPLAY', 'XDG_RUNTIME_DIR': '$XDG_RUNTIME_DIR', 'GDK_BACKEND': 'wayland', 'XDG_CURRENT_DESKTOP': 'Phosh:GNOME', 'XDG_SESSION_DESKTOP': 'phosh', 'XDG_SESSION_TYPE': 'wayland'}" >/dev/null 2>&1
# Brillo por slider: el shim D-Bus provee org.gnome.SettingsDaemon.Power.Screen (propiedad Brightness)
# en ESTA sesion -> al mover el slider, Phosh escribe la propiedad -> shim -> /run/mt6582-bl-pct
# -> daemon mt6582-backlight -> registro PWM. Sin el shim corriendo aqui, el slider no hace nada.
python3 /usr/local/bin/mt6582-bl-shim.py >/dev/null 2>&1 &
exec /usr/local/libexec/phosh
