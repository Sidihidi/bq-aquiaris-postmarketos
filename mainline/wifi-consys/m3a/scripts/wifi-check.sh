#!/bin/bash
PH=root@172.16.42.1
sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
UP=0
for i in $(seq 1 40); do ssh -n -o ConnectTimeout=4 -o StrictHostKeyChecking=no $PH true 2>/dev/null && { UP=1; echo "UP ~$((i*5))s"; break; }; sleep 5; done
[ $UP = 0 ] && { echo SIN-SSH; exit 1; }
ssh -n -o ConnectTimeout=10 -o StrictHostKeyChecking=no $PH 'dmesg | grep -iE "BTIF-DMA|STP TX|DIAG|CONSYS CONTESTA|RX=0" | tail -28'
