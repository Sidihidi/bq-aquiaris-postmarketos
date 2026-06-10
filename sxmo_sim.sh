#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== espacio actual ==="
df -h / | tail -1
echo "=== simulacion sxmo-de-dwm ==="
S "apk add --simulate postmarketos-ui-sxmo-de-dwm 2>&1 | tail -3"
echo "=== refresco a 10fps ==="
S "sed -i \"s/sleep 0.25/sleep 0.1/\" /usr/local/bin/fb-refresh"
S "pkill -f fb-refresh; (/usr/local/bin/fb-refresh &)"
echo REFRESH_10FPS
'
