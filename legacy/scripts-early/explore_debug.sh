#!/bin/bash
sudo ip addr add 172.16.42.2/24 dev usb0 2>/dev/null
ping -c2 -W2 172.16.42.1 >/dev/null 2>&1 && echo "PING OK" || { echo "PING FALLA"; exit 1; }
which telnet >/dev/null || sudo apt-get install -y -q telnet >/dev/null 2>&1
# explorar por telnet: particiones y herramientas disponibles
{ sleep 2
  echo "cat /proc/partitions"
  sleep 2
  echo "cat /proc/dumchar_info 2>/dev/null | head -25"
  sleep 2
  echo "which nc dd blkid; ls /dev/mmcblk0* | head"
  sleep 2
} | telnet 172.16.42.1 2>/dev/null
