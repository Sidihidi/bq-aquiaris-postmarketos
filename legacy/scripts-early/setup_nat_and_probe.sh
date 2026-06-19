#!/bin/bash
# 1. NAT en la Pi: compartir internet con el telefono
UPLINK=$(ip route show default | awk '{print $5; exit}')
echo "uplink de la Pi: $UPLINK"
sudo sysctl -w net.ipv4.ip_forward=1 >/dev/null
sudo iptables -t nat -C POSTROUTING -s 172.16.42.0/24 -o "$UPLINK" -j MASQUERADE 2>/dev/null || \
  sudo iptables -t nat -A POSTROUTING -s 172.16.42.0/24 -o "$UPLINK" -j MASQUERADE
sudo iptables -C FORWARD -s 172.16.42.0/24 -j ACCEPT 2>/dev/null || sudo iptables -A FORWARD -s 172.16.42.0/24 -j ACCEPT
sudo iptables -C FORWARD -d 172.16.42.0/24 -j ACCEPT 2>/dev/null || sudo iptables -A FORWARD -d 172.16.42.0/24 -j ACCEPT
echo "NAT configurado"
# 2. ruta + dns en el telefono, test de internet, instalar fbset
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
echo 147147 | sudo -S sh -c "ip route add default via 172.16.42.2 2>/dev/null; printf \"nameserver 1.1.1.1\nnameserver 8.8.8.8\n\" > /etc/resolv.conf"
ping -c2 -W3 1.1.1.1 >/dev/null 2>&1 && echo INTERNET_OK || echo SIN_INTERNET
echo 147147 | sudo -S apk add fbset 2>&1 | tail -1
echo "=== probar refresco con fbset ==="
echo 147147 | sudo -S sh -c "cat /dev/urandom > /dev/fb0" 2>/dev/null
echo 147147 | sudo -S fbset -depth 32 2>&1 | head -2
echo RUIDO_Y_FBSET_HECHOS
echo "=== knobs del driver en sys ==="
ls /sys/module/ | grep -iE "disp|fb|mtk" | head
find /sys -iname "*decouple*" 2>/dev/null | head -5
ls /proc/mtkfb* /proc/disp* 2>/dev/null
'
