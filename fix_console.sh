#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== 1. servicio de arranque: despertar pantalla ==="
S "mkdir -p /etc/local.d"
S "cat > /etc/local.d/99-display-kick.start << \"EOF\"
#!/bin/sh
# El driver mtkfb entra en modo decouple al arrancar y deja de presentar
# el framebuffer; un ciclo blank/unblank + PUT_VSCREENINFO lo re-engancha.
sleep 1
echo 1 > /sys/class/graphics/fb0/blank
sleep 1
echo 0 > /sys/class/graphics/fb0/blank
fbset -depth 32 2>/dev/null
EOF
chmod +x /etc/local.d/99-display-kick.start"
echo KICK_INSTALADO
echo "=== 2. autologin en tty1 ==="
S "sed -i \"s|tty1::respawn:/sbin/getty 38400 tty1|tty1::respawn:/bin/login -f user|\" /etc/inittab"
grep tty1 /etc/inittab
S "kill -HUP 1"
echo "=== 3. tactil y uinput ==="
ls /dev/input/ 2>/dev/null
ls -la /dev/uinput 2>/dev/null || echo SIN_UINPUT
for f in /sys/class/input/input*/name; do echo "$f: $(cat $f)"; done 2>/dev/null
'
