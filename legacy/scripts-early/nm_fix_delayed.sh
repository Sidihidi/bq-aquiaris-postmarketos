#!/bin/bash
# golpe persistente: en cuanto entre, escribe el fix y reinicia NM CON RETARDO
# para que el comando SSH retorne antes de que el enlace caiga.
for i in $(seq 1 80); do
  sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
  R=$(timeout 8 ssh -o ConnectTimeout=4 -o StrictHostKeyChecking=no -o BatchMode=yes \
        user@172.16.42.1 \
        "echo 147147 | sudo -S sh -c 'mkdir -p /etc/NetworkManager/conf.d; printf \"[keyfile]\nunmanaged-devices=interface-name:usb0\n\" > /etc/NetworkManager/conf.d/99-usb-unmanaged.conf; nohup sh -c \"sleep 2; rc-service networkmanager restart\" >/dev/null 2>&1 &' ; echo APLICADO" 2>/dev/null)
  if echo "$R" | grep -q APLICADO; then
    echo "FIX APLICADO en intento $i"
    exit 0
  fi
  sleep 4
done
echo SIN_VENTANA
