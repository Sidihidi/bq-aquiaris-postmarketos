#!/bin/bash
# NAT en la Pi para dar internet al telefono por USB
# Telefono: 172.16.42.1, Pi: 172.16.42.2 como gateway
INET=$(ip route get 8.8.8.8 2>/dev/null | grep -oP 'dev \K\S+' | head -1)
[ -z "$INET" ] && INET=wlan0
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward > /dev/null
sudo ip addr add 172.16.42.2/24 dev usb0 2>/dev/null || true
sudo ip link set usb0 up
sudo iptables -t nat -A POSTROUTING -o "$INET" -s 172.16.42.0/24 -j MASQUERADE 2>/dev/null || true
sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT 2>/dev/null || true
sudo iptables -A FORWARD -s 172.16.42.0/24 -o "$INET" -j ACCEPT 2>/dev/null || true
echo "NAT: telefono -> Pi -> internet ($INET)"
