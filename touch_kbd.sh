#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== reiniciar tty1 para autologin ==="
S "pkill -f \"getty 38400 tty1\"" ; echo TTY1_REINICIADO
echo "=== instalar buffyboard y evtest ==="
S "apk add buffyboard evtest" 2>&1 | tail -1
which buffyboard evtest
echo "=== TOCA LA PANTALLA AHORA (5 segundos) ==="
S "timeout 5 evtest /dev/input/event3 2>&1 | tail -8"
'
