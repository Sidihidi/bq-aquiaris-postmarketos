#!/bin/bash
echo "=== DHCP en usb0 (5s) ==="
sudo timeout 8 dhclient -v -1 usb0 2>&1 | grep -E 'DHCPOFFER|DHCPACK|bound|failed|Timeout' | head -3
ip addr show usb0 | grep 'inet '
echo "=== scan TCP 1-9000 (rapido) ==="
for p in 22 23 80 443 2222 8022 5900 8080; do
  timeout 1 nc -z 172.16.42.1 $p 2>/dev/null && echo "PUERTO $p ABIERTO"
done
echo "=== UDP 67 (dhcp server) ==="
timeout 2 nc -u -z 172.16.42.1 67 2>/dev/null && echo UDP67_RESPONDE || echo udp67_nada
echo FIN
