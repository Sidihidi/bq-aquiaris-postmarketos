#!/bin/sh
# Recuperar usb0 tras resume: unbind/rebind del controlador musb fuerza que el
# gadget se re-presente al host USB (re-enumeracion). Legacy gadget (g_ether), sin configfs.
D=/sys/bus/platform/drivers/musb-hdrc
U=musb-hdrc.2.auto
echo "udc state antes: $(cat /sys/class/udc/$U/state 2>/dev/null)"
echo "$U" > $D/unbind 2>&1
sleep 1
echo "$U" > $D/bind 2>&1
sleep 3
echo "udc state despues: $(cat /sys/class/udc/$U/state 2>/dev/null)"
# el gadget legacy re-crea usb0; re-asegurar IP del lado movil
if ip link show usb0 >/dev/null 2>&1; then
  ip addr add 172.16.42.1/24 dev usb0 2>/dev/null
  ip link set usb0 up 2>/dev/null
fi
echo "usb0: $(ip -4 addr show usb0 2>/dev/null | grep inet || echo none)"
echo "RECOVERY-DONE"
