#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
echo "=== total lineas de eventos reales ==="
grep -c "^Event: time" /tmp/touch.log 2>/dev/null
echo "=== muestra de eventos ==="
grep "^Event: time" /tmp/touch.log 2>/dev/null | head -10
echo FIN
'
