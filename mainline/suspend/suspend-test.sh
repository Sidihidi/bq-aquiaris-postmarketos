#!/bin/sh
# Test de suspend s2idle con AUTO-WAKE por RTC, log PERSISTENTE (/root, sobrevive reboot).
# Objetivo: confirmar que (a) el RTC despierta solo, (b) la conectividad vuelve tras resume.
sleep 3
LOG=/root/susp-test.log
{
echo "===== RTC-WAKE TEST $(date) ====="
echo "before: uptime=$(cut -d. -f1 /proc/uptime)s"
echo "RTCWAKE-BEGIN-MARK" > /dev/kmsg 2>/dev/null
sync
t0=$(date +%s)
/usr/sbin/rtcwake -m freeze -s 25
rc=$?
t1=$(date +%s)
echo "RTCWAKE-RESUMED-MARK" > /dev/kmsg 2>/dev/null
echo "rtcwake_rc=$rc  wall=$((t1-t0))s  (~25 si el RTC desperto solo; ~0 si no suspendio)"
echo "after: uptime=$(cut -d. -f1 /proc/uptime)s"
sleep 3
echo "--- conectividad tras resume ---"
echo "usb0: $(ip -4 addr show usb0 2>/dev/null | grep inet || echo NADA)"
echo "wlan0: $(nmcli -t -f DEVICE,STATE dev status 2>/dev/null | grep wlan0 || echo NADA)"
echo "--- intento recuperar wifi ---"
nmcli dev connect wlan0 >/dev/null 2>&1
sleep 2
echo "wlan0 tras reconnect: $(nmcli -t -f DEVICE,STATE dev status 2>/dev/null | grep wlan0)"
echo "wlan0 IP: $(ip -4 addr show wlan0 2>/dev/null | grep -o 'inet [0-9.]*')"
echo "DONE"
} >> $LOG 2>&1
sync
