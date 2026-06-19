#!/bin/bash
{ sleep 2
  echo 'mount -t ext4 /dev/mmcblk0p5 /sysroot && echo MONTA_OK'
  sleep 4
  echo 'ls /sysroot'
  sleep 2
  echo 'umount /sysroot'
  sleep 2
} | telnet 172.16.42.1 2>/dev/null | grep -vE '^Trying|^Connected|^Escape|debug shell|postmarketos.org|Device:|Kernel:|OS ver|initrd:|continue boot|pmOS_init'
