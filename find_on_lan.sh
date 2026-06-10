#!/bin/bash
echo "=== red de la Pi ==="
ip -4 addr show wlan0 | grep inet
SUBNET=$(ip -4 addr show wlan0 | grep -oE '192\.168\.[0-9]+' | head -1)
echo "subnet: $SUBNET.0/24"
echo "=== vecinos ARP conocidos ==="
ip neigh show dev wlan0 | grep -iE 'REACHABLE|STALE' | head
echo "=== barrido SSH (puerto 22) en la LAN ==="
for n in $(seq 1 254); do
  ip="$SUBNET.$n"
  [ "$ip" = "$(hostname -I | awk '{print $1}')" ] && continue
  timeout 1 bash -c "echo >/dev/tcp/$ip/22" 2>/dev/null && echo "SSH abierto: $ip" &
done
wait
echo "=== ¿hostname bq-krillin? ==="
getent hosts bq-krillin 2>/dev/null || ping -c1 -W1 bq-krillin.local 2>/dev/null | head -1 || echo "no resuelve"
echo FIN
