#!/bin/bash
PH=root@172.16.42.1
sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
UP=0
for i in $(seq 1 40); do ssh -n -o ConnectTimeout=4 -o StrictHostKeyChecking=no $PH true 2>/dev/null && { UP=1; echo "UP ~$((i*5))s"; break; }; sleep 5; done
[ $UP = 0 ] && { echo SIN-SSH; exit 1; }
echo "=== canal en boot ==="
ssh -n -o ConnectTimeout=8 -o StrictHostKeyChecking=no $PH 'dmesg | grep -iE "CANAL OK|BTIF-DMA init|trigger bring" | sed -E "s/^\[[0-9. ]+\] //" | tail -3'
echo "=== disparar bring-up (patch download) ==="
ssh -n -o ConnectTimeout=10 -o StrictHostKeyChecking=no $PH 'timeout 120 sh -c "echo 1 > /sys/kernel/debug/mt6582_btif/bringup"; echo TRIGGER-RC=$?'
echo "=== resultado ==="
ssh -n -o ConnectTimeout=10 -o StrictHostKeyChecking=no $PH 'dmesg | grep -iE "BRING-UP|canal OK|patch |DESCARGADO|EVT no|TIMEOUT|PATCH COMPLETO|frags|addr" | sed -E "s/^\[[0-9. ]+\] //" | tail -25'
