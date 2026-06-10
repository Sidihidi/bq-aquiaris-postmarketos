#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
S "(setsid /usr/local/bin/fb-refresh </dev/null >/dev/null 2>&1 &)"
sleep 2
echo "instancias fb-refresh: $(ps | grep fb-refre[s]h | wc -l)"
echo "sxmo/dwm vivos: $(ps | grep -cE "[d]wm|[s]xmo")"
echo "=== MIRA LA PANTALLA: deberia verse la barra de sxmo ==="
'
