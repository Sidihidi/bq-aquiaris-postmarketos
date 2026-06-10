#!/bin/bash
# Receptor en el teléfono (via telnet, en background) y envío desde la Pi
{ sleep 2
  echo "nc -l -p 9999 | dd of=/dev/mmcblk0p5 bs=1M &"
  sleep 3
} | telnet 172.16.42.1 >/dev/null 2>&1

sleep 2
echo "=== ENVIANDO pmos-root.img (653 MB) ==="
time nc -N -w 15 172.16.42.1 9999 < /tmp/pmos-root.img
echo "=== ENVIO TERMINADO, verificando ==="
sleep 5
{ sleep 2
  echo "sync"
  sleep 3
  echo "blkid /dev/mmcblk0p5"
  sleep 2
} | telnet 172.16.42.1 2>/dev/null | grep -E "mmcblk0p5|pmOS|LABEL"
