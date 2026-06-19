#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
echo "=== instancias fb-refresh ==="
ps | grep "fb-refre[s]h" | wc -l
echo "=== top CPU ==="
top -bn1 2>/dev/null | head -12 || ps -o pid,comm,time | head -15
echo "=== instalacion sxmo? ==="
ls -la /tmp/sxmo_install.log 2>/dev/null && tail -3 /tmp/sxmo_install.log || echo NO_HAY_LOG
pgrep apk >/dev/null && echo APK_CORRIENDO || echo APK_NO_CORRE
'
