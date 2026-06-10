#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== reiniciar tty1 (por PID) ==="
PID=$(ps | grep "/sbin/getty" | grep tty1 | grep -v grep | awk "{print \$1}")
[ -n "$PID" ] && S "kill $PID" && echo "getty tty1 ($PID) reiniciado" || echo "getty tty1 ya no estaba"
echo "=== instalar buffyboard y evtest ==="
S "apk add buffyboard evtest" 2>&1 | tail -1
which buffyboard evtest
echo "=== TOCA LA PANTALLA AHORA (8 segundos) ==="
S "timeout 8 evtest /dev/input/event3 2>&1 | tail -10"
echo FIN_TEST_TACTIL
'
