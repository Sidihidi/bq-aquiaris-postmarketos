#!/bin/bash
ping -c2 -W2 172.16.42.1 >/dev/null 2>&1 && echo PING_OK || echo SIN_PING
ssh -o ConnectTimeout=6 -o StrictHostKeyChecking=no user@172.16.42.1 '
echo "X vivos: $(ps | grep -c [X]org)"
echo "refresh: $(ps | grep -c fb-refre[s]h)"
echo "tinydm: $(rc-service tinydm status 2>&1 | tail -1)"
echo "--- driver tactil ---"
grep -E "Using input driver.*mtk-tpd" /var/log/Xorg.0.log | tail -2
echo "--- double tracking (total) ---"
grep -c "double tracking" /var/log/Xorg.0.log
'
