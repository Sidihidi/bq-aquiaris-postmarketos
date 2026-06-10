#!/bin/bash
echo "--- dmesg USB reciente ---"
sudo dmesg | grep -iE 'usb 1-1|rndis' | tail -6
echo "--- puertos abiertos en el telefono ---"
for p in 22 23 80 5900 8022 2222; do
  timeout 2 nc -z 172.16.42.1 $p 2>/dev/null && echo "PUERTO $p ABIERTO"
done
echo "--- arp ---"
ip neigh show dev usb0
echo FIN
