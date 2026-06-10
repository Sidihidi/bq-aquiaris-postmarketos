#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== liberar cache apk ==="
S "rm -rf /var/cache/apk/* /root/.cache /home/user/.cache 2>/dev/null"
df -h / | tail -1
echo "=== Xorg.conf: forzar fbdev + evdev tactil ==="
S "mkdir -p /etc/X11/xorg.conf.d"
S "cat > /etc/X11/xorg.conf.d/10-fbdev.conf << \"EOF\"
Section \"Device\"
    Identifier \"mtkfb\"
    Driver \"fbdev\"
    Option \"fbdev\" \"/dev/fb0\"
EndSection
EOF"
echo XORG_CONF_OK
echo "=== asegurar demonio refresco activo ==="
ps | grep "fb-refre[s]h" | wc -l
echo "=== arrancar tinydm (lanza sxmo) ==="
S "rc-service tinydm start 2>&1 | tail -2"
sleep 8
echo "=== procesos X/sxmo ==="
ps | grep -E "[X]org|[d]wm|[s]xmo" | head -8
'
