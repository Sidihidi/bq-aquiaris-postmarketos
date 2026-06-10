#!/bin/bash
ssh -o ConnectTimeout=8 -o StrictHostKeyChecking=no user@172.16.42.1 '
echo "uptime: $(uptime)"
echo "--- procesos con X en el nombre ---"
ps | grep -i xorg | grep -v grep
ps | grep -E "tinydm|xinit|startx" | grep -v grep
echo "--- rc-status default (resumen) ---"
rc-status default 2>/dev/null | grep -iE "tinydm|local"
echo "--- logs de X presentes ---"
ls -la /var/log/Xorg* 2>/dev/null || echo NO_HAY_LOGS_X
echo "--- ultimo dmesg ---"
dmesg | tail -3
echo "--- local.d se ejecuto? (refresh corriendo=si) ---"
ps | grep fb-refre[s]h | grep -v grep | head -2
'
