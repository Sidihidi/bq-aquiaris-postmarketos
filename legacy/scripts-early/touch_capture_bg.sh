#!/bin/bash
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
echo 147147 | sudo -S sh -c "rm -f /tmp/touch.log; (timeout 90 evtest /dev/input/event3 > /tmp/touch.log 2>&1 &) ; echo CAPTURA_90S_INICIADA"
'
