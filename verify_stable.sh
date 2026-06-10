#!/bin/bash
sleep 12
echo "=== test de estabilidad: 10 pings ==="
sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
ping -c10 -i0.5 -W2 172.16.42.1 | tail -3
echo "=== estado completo via SSH ==="
ssh -o ConnectTimeout=8 -o StrictHostKeyChecking=no user@172.16.42.1 '
echo "uptime: $(uptime)"
echo "tinydm: $(rc-service tinydm status 2>&1 | tail -1)"
echo "Xorg vivos: $(ps | grep -c [X]org)"
echo "refresh: $(ps | grep -c fb-refre[s]h)"
echo "--- driver tactil (log actual) ---"
L=$(ls -t /var/log/Xorg.0.log /var/log/Xorg.0.log.old 2>/dev/null | head -1)
grep -E "Using input driver.*mtk-tpd" "$L" 2>/dev/null | tail -3
echo "--- double tracking en ese log ---"
grep -c "double tracking" "$L" 2>/dev/null
'
