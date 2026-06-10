#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
echo 147147 | sudo -S sh -c "nohup apk add postmarketos-ui-sxmo-de-dwm xf86-video-fbdev xf86-input-evdev > /tmp/sxmo_install.log 2>&1 &"
sleep 3
pgrep apk >/dev/null && echo APK_INSTALANDO || echo APK_NO_ARRANCO
head -2 /tmp/sxmo_install.log 2>/dev/null
'
