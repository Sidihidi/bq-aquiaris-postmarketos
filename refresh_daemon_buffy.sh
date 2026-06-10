#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
S() { echo 147147 | sudo -S sh -c "$1" 2>/dev/null; }
echo "=== demonio de refresco (pan cada 250ms) ==="
S "cat > /usr/local/bin/fb-refresh << \"EOF\"
#!/bin/sh
# mtkfb decouple workaround: forzar present continuo via pan
while true; do
  echo 0,0 > /sys/class/graphics/fb0/pan 2>/dev/null
  sleep 0.25
done
EOF
chmod +x /usr/local/bin/fb-refresh"
S "cat > /etc/local.d/98-fb-refresh.start << \"EOF\"
#!/bin/sh
/usr/local/bin/fb-refresh &
EOF
chmod +x /etc/local.d/98-fb-refresh.start"
S "pgrep -f fb-refresh >/dev/null || /usr/local/bin/fb-refresh &"
sleep 1
echo REFRESH_DAEMON_ACTIVO
echo "=== lanzar buffyboard ==="
S "pgrep buffyboard >/dev/null || (buffyboard > /tmp/buffy.log 2>&1 &)"
sleep 2
pgrep buffyboard >/dev/null && echo BUFFYBOARD_CORRIENDO || { echo BUFFYBOARD_NO_ARRANCA; cat /tmp/buffy.log 2>/dev/null | head -5; }
'
