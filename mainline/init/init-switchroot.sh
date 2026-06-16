#!/bin/busybox sh
# Stage-1 initramfs: monta p7 (Alpine) y switch_root. Si falla, emergencia SSH.
BB=/bin/busybox
$BB mount -t proc     proc /proc 2>/dev/null
$BB mount -t sysfs    sys  /sys  2>/dev/null
$BB mount -t devtmpfs dev  /dev  2>/dev/null
echo "stage1: buscando /dev/mmcblk0p7" > /dev/kmsg 2>/dev/null

i=0; while [ $i -lt 20 ]; do [ -b /dev/mmcblk0p7 ] && break; $BB sleep 1; i=$((i+1)); done
$BB mkdir -p /newroot
if $BB mount -t ext4 /dev/mmcblk0p7 /newroot 2>/dev/null && [ -e /newroot/bin/busybox ]; then
    echo "stage1: p7 montada -> switch_root a Alpine" > /dev/kmsg 2>/dev/null
    $BB mount --move /dev  /newroot/dev  2>/dev/null
    $BB mount --move /proc /newroot/proc 2>/dev/null
    $BB mount --move /sys  /newroot/sys  2>/dev/null
    exec $BB switch_root /newroot /sbin/init
fi

# ---- FALLBACK de emergencia: no perder acceso ----
echo "stage1: switch_root FALLO -> emergencia SSH" > /dev/kmsg 2>/dev/null
$BB --install -s /bin 2>/dev/null
$BB mkdir -p /dev/pts; $BB mount -t devpts devpts /dev/pts 2>/dev/null
[ -e /dev/watchdog ] && printf 'V' > /dev/watchdog 2>/dev/null
i=0; while [ $i -lt 40 ]; do [ -e /sys/class/net/usb0 ] && break; $BB sleep 1; i=$((i+1)); done
$BB ifconfig usb0 172.16.42.1 netmask 255.255.255.0 up 2>/dev/null
$BB chown -R 0:0 /root 2>/dev/null; $BB chmod 700 /root /root/.ssh 2>/dev/null; $BB chmod 600 /root/.ssh/authorized_keys 2>/dev/null
mkdir -p /etc/dropbear
[ -f /etc/dropbear/dropbear_ed25519_host_key ] || /bin/dropbearmulti dropbearkey -t ed25519 -f /etc/dropbear/dropbear_ed25519_host_key >/dev/kmsg 2>&1
/bin/dropbearmulti dropbear -p 22 -r /etc/dropbear/dropbear_ed25519_host_key >/dev/kmsg 2>&1 &
$BB telnetd -l /bin/sh -p 23 2>/dev/null
while true; do $BB sleep 3600; done
