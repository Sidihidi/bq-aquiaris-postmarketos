#!/bin/sh
# WAYLAND_DISPLAY/XDG_RUNTIME_DIR ya estan seteados por phoc -E
gdbus call --session --dest org.freedesktop.DBus --object-path /org/freedesktop/DBus \
  --method org.freedesktop.DBus.UpdateActivationEnvironment \
  "{'WAYLAND_DISPLAY': '$WAYLAND_DISPLAY', 'XDG_RUNTIME_DIR': '$XDG_RUNTIME_DIR', 'GDK_BACKEND': 'wayland'}" >/dev/null 2>&1
exec /usr/local/libexec/phosh
