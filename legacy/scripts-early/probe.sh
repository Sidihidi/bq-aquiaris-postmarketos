#!/bin/bash
# sondeo completo del estado del telefono via USB net
sudo ip addr add 172.16.42.2/24 dev usb0 2>/dev/null
ping -c1 -W2 172.16.42.1 >/dev/null 2>&1 && echo "PING: OK" || echo "PING: no"
for p in 22 23; do
  timeout 2 nc -z 172.16.42.1 $p 2>/dev/null && echo "PUERTO $p: ABIERTO" || echo "PUERTO $p: cerrado"
done
L=$(sudo timeout 6 dhclient -v -1 usb0 2>&1 | grep -cE 'DHCPACK|bound')
echo "DHCP userspace: $L"
echo "USB: $(lsusb | grep -c MediaTek 2>/dev/null || true) dispositivo(s) MTK"
sudo dmesg | tail -2
