#!/bin/sh
# Captura UART del krillin (un SOLO lector). Uso: sudo ~/uart-capture.sh
DEV=/dev/ttyAMA0
pkill -x cat 2>/dev/null
fuser -k $DEV 2>/dev/null
sleep 1
stty -F $DEV 921600 raw -echo -crtscts
rm -f /tmp/uart.log; touch /tmp/uart.log; chmod 666 /tmp/uart.log
setsid sh -c "exec cat $DEV >> /tmp/uart.log 2>/dev/null" </dev/null >/dev/null 2>&1 &
echo "captura UART activa -> /tmp/uart.log (921600 8N1)"
