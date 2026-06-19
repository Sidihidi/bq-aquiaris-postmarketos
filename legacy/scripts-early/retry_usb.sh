#!/bin/bash
sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
for k in 1 2 3 4 5 6; do
  if ping -c1 -W2 172.16.42.1 >/dev/null 2>&1; then echo "PING OK (intento $k)"; break; fi
  echo "intento $k: sin respuesta"
  sleep 3
done
timeout 3 nc -z 172.16.42.1 22 2>/dev/null && echo SSH_ABIERTO || echo SSH_CERRADO
echo "carrier usb0: $(cat /sys/class/net/usb0/carrier 2>/dev/null)"
