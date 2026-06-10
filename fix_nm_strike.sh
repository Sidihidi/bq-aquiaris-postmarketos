#!/bin/bash
# espera ventana de conectividad y aplica el fix de NetworkManager en un solo golpe
for i in $(seq 1 60); do
  sudo ip addr add 172.16.42.2/24 dev usb0 2>/dev/null
  if timeout 3 nc -z 172.16.42.1 22 2>/dev/null; then
    echo "ventana detectada (intento $i), aplicando fix..."
    timeout 20 ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no user@172.16.42.1 '
      echo 147147 | sudo -S sh -c "
        mkdir -p /etc/NetworkManager/conf.d
        printf \"[keyfile]\nunmanaged-devices=interface-name:usb0\n\" > /etc/NetworkManager/conf.d/99-usb-unmanaged.conf
        rc-service networkmanager restart >/dev/null 2>&1
      " && echo NM_FIX_APLICADO
      uptime
    ' && exit 0
    echo "fallo en la ventana, sigo esperando..."
  fi
  sleep 5
done
echo SIN_VENTANA_EN_5MIN
