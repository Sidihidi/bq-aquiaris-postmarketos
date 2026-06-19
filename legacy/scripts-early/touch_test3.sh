#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== TOCA LA PANTALLA REPETIDAMENTE durante 12 segundos ==="
S "timeout 12 evtest /dev/input/event3 2>/dev/null | grep -E \"POSITION|TRACKING|SYN_REPORT\" | head -12"
echo FIN
'
