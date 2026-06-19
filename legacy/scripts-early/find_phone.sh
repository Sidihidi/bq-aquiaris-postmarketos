#!/bin/bash
sudo ip addr flush dev usb0 2>/dev/null
sudo timeout 10 dhclient -v -1 usb0 2>&1 | grep -E 'DHCPACK|bound to' | head -2
ip addr show usb0 | grep 'inet '
for ip in 172.16.42.1 10.42.0.1 192.168.42.1; do
  ping -c1 -W1 $ip >/dev/null 2>&1 && echo "RESPONDE: $ip" && timeout 2 nc -z $ip 22 2>/dev/null && echo "SSH ABIERTO EN: $ip"
done
echo FIN
