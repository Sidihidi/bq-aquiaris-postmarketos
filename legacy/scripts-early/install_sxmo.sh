#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
# reiniciar demonio refresco (patron seguro: matar por PID del script)
for p in $(ps | grep "fb-refre[s]h" | awk "{print \$1}"); do S "kill $p"; done
S "(/usr/local/bin/fb-refresh </dev/null >/dev/null 2>&1 &)"
echo REFRESH_OK
# instalar sxmo + driver fbdev de X y evdev en background
S "nohup apk add postmarketos-ui-sxmo-de-dwm xf86-video-fbdev xf86-input-evdev > /tmp/sxmo_install.log 2>&1 &"
echo INSTALACION_LANZADA
'
