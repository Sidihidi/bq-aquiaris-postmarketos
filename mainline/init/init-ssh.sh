#!/bin/busybox sh
# M2d v4 — SSH por USB en mainline (FUNCIONA). Display lo arregla el kernel.
#   - devpts montado (PTYs para telnetd/dropbear)
#   - owner root + perms estrictos en /root (dropbear los exige)
#   - dropbear SIN -E (esta build, --disable-syslog, ya loguea a stderr)
#   - telnetd en :23 como fallback
BB=/bin/busybox
$BB mount -t proc     proc   /proc    2>/dev/null
$BB mount -t devtmpfs dev    /dev     2>/dev/null
$BB mount -t sysfs    sys    /sys     2>/dev/null
$BB mkdir -p /dev/pts
$BB mount -t devpts   devpts /dev/pts 2>/dev/null
[ -e /dev/ptmx ] || $BB ln -s /dev/pts/ptmx /dev/ptmx
$BB --install -s /bin 2>/dev/null
export PATH=/bin:/sbin:/usr/bin:/usr/sbin HOME=/root
K=/dev/kmsg
log(){ echo "M2d: $*" > $K 2>/dev/null; }
log "init v4 (SSH) arrancando"

[ -e /dev/watchdog ] && printf 'V' > /dev/watchdog 2>/dev/null

# dropbear exige: /root y authorized_keys owned root y no escribibles por grupo/otros
chown -R 0:0 /root /etc/dropbear 2>/dev/null
chmod 700 /root /root/.ssh 2>/dev/null
chmod 600 /root/.ssh/authorized_keys 2>/dev/null

# red por USB
i=0; while [ $i -lt 40 ]; do [ -e /sys/class/net/usb0 ] && break; sleep 1; i=$((i+1)); done
ip link set lo up 2>/dev/null
ip addr add 172.16.42.1/24 dev usb0 2>/dev/null
ip link set usb0 up 2>/dev/null
log "usb0 = 172.16.42.1/24 (host Pi = 172.16.42.2)"

# telnetd (fallback, shell directa)
$BB telnetd -l /bin/sh -p 23
log "telnetd :23 (fallback)"

# dropbear SSH en :22 (auth por clave)
mkdir -p /etc/dropbear
[ -f /etc/dropbear/dropbear_ed25519_host_key ] || \
  /bin/dropbearmulti dropbearkey -t ed25519 -f /etc/dropbear/dropbear_ed25519_host_key > $K 2>&1
/bin/dropbearmulti dropbear -F -p 22 -r /etc/dropbear/dropbear_ed25519_host_key > $K 2>&1 &
sleep 2
{ echo "=== M2d escuchando: ==="; netstat -ltn 2>/dev/null; } > $K 2>&1
log "LISTO: ssh root@172.16.42.1   (o telnet 172.16.42.1)"

while true; do $BB sleep 3600; done
