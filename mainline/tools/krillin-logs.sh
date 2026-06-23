#!/bin/bash
# krillin-logs.sh — captura logs del telefono a un dir con fecha (en la Pi). Uso: bash krillin-logs.sh
PH=root@172.16.42.1
O="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=5"
sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
ssh -n $O $PH true 2>/dev/null || { echo "sin SSH -> krillin-recover.sh forcereboot"; exit 1; }
D=~/krillin-logs/$(date +%Y%m%d-%H%M%S)
mkdir -p "$D"
ssh -n $O $PH 'dmesg'                       > "$D/dmesg.txt"          2>/dev/null
ssh -n $O $PH 'cat /var/log/rc.log'         > "$D/rc.log"            2>/dev/null
ssh -n $O $PH 'cat /tmp/phosh-boot.log'     > "$D/phosh-boot.log"    2>/dev/null
ssh -n $O $PH 'rc-status -a'                > "$D/rc-status.txt"     2>/dev/null
ssh -n $O $PH 'free -m; echo; ps w'         > "$D/procs-mem.txt"     2>/dev/null
ssh -n $O $PH 'cat /sys/fs/pstore/* 2>/dev/null' > "$D/pstore.txt"   2>/dev/null
echo "logs en: $D"
ls -la "$D"
