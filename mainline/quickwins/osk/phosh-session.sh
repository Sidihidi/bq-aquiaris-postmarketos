#!/bin/sh
# /usr/local/bin/phosh-session.sh
# Wrapper de sesion para Phosh en el BQ Aquaris E4.5 (MT6582 / krillin).
# phoc lo ejecuta via `phoc -E /usr/local/bin/phosh-session.sh`, asi que
# WAYLAND_DISPLAY y XDG_RUNTIME_DIR ya estan seteados aqui.
#
# Inyecta esas variables en el entorno de activacion de D-Bus de la sesion,
# para que squeekboard (el teclado en pantalla, sm.puri.OSK0) — que D-Bus
# activa a demanda via su .service — herede el WAYLAND_DISPLAY y pueda
# conectarse al compositor. Sin esto, squeekboard arranca pero no encuentra
# el display ("Address has no colon" / "connection is closed").
gdbus call --session --dest org.freedesktop.DBus --object-path /org/freedesktop/DBus \
  --method org.freedesktop.DBus.UpdateActivationEnvironment \
  "{'WAYLAND_DISPLAY': '$WAYLAND_DISPLAY', 'XDG_RUNTIME_DIR': '$XDG_RUNTIME_DIR', 'GDK_BACKEND': 'wayland'}" \
  >/dev/null 2>&1

exec /usr/local/libexec/phosh
