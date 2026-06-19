#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== el error de la instalacion ==="
grep -iE "error|ERROR" /tmp/sxmo_install.log | head -5
echo "=== espacio tras instalar ==="
df -h / | tail -1
echo "=== como arranca sxmo? (greeter/tinydm) ==="
ls /etc/sxmo* 2>/dev/null; which sxmo_winit.sh startx 2>/dev/null
S "rc-update show default 2>/dev/null | grep -iE \"dm|greet|tinydm|sxmo\""
echo "=== driver X disponible ==="
ls /usr/lib/xorg/modules/drivers/ 2>/dev/null
'
