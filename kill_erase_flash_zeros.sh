#!/bin/bash
sudo pkill -f 'fastboot erase' && echo ERASE_MATADO || echo NO_HABIA_ERASE
sleep 3
sudo timeout 10 fastboot devices
echo "=== flash zeros a system ==="
sudo timeout 30 fastboot flash system /tmp/zeros4m.img 2>&1 | tail -4
