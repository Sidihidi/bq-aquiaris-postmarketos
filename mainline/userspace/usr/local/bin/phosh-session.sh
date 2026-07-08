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
# Vibracion/haptico: feedbackd trae el perfil por defecto en 'silent' (no vibra NADA).
# 'quiet' = haptico en notificaciones/eventos, sin sonidos. ('full' anade sonidos +
# haptico al pulsar botones/teclado; el motor es regulator-haptic FF en event2.)
gsettings set org.sigxcpu.feedbackd profile quiet 2>/dev/null
# Animaciones fuera: el fade del ScreenSaver.Lock tardaba ~1.3s (jank al despertar
# justo tras bloquear) y en este A7 las animaciones cuestan CPU que no sobra.
gsettings set org.gnome.desktop.interface enable-animations false 2>/dev/null
/usr/libexec/gsd-power &
# Brillo por slider: el shim D-Bus provee org.gnome.SettingsDaemon.Power.Screen (Brightness)
# en ESTA sesion -> slider -> /run/mt6582-bl-pct -> daemon mt6582-backlight -> PWM.
[ -f /usr/local/bin/mt6582-bl-shim.py ] && python3 /usr/local/bin/mt6582-bl-shim.py >/dev/null 2>&1 &
# Boton fluido: helper persistente en la sesion (socket $XDG_RUNTIME_DIR/powerkey.sock);
# mt6582-powerkey le manda lock/unlock/menu sin forkear su+gdbus (latencia ~ms).
[ -f /usr/local/bin/mt6582-session-helper ] && python3 /usr/local/bin/mt6582-session-helper >/dev/null 2>&1 &
# Audio: arrancar PulseAudio (crea el sink de la card mt6582audio). Las apps tambien lo
# autospawn-ean al reproducir, pero arrancarlo aqui deja el sink listo desde el boot y evita
# el retardo del autospawn. El driver del AFE enciende el codec+amp al abrir el sink.
command -v pulseaudio >/dev/null 2>&1 && pulseaudio --start --exit-idle-time=-1 >/dev/null 2>&1 &
sleep 1
# SIN exec: si phosh muere, phoc NO se entera solo (visto 0705: phosh muerto, phoc vivo,
# sesion zombi sin lockscreen). Al salir phosh tumbamos phoc ($PPID) -> dbus-run-session
# cae -> launch_phosh.sh (supervisor) relanza la sesion entera.
/usr/local/libexec/phosh
kill $PPID 2>/dev/null
