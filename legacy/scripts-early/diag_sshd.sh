#!/bin/bash
{ sleep 2
  echo 'mount -t ext4 /dev/mmcblk0p5 /sysroot && echo MONTA_OK'
  sleep 3
  echo 'ls /sysroot/usr/sbin/sshd && echo SSHD_INSTALADO || echo SSHD_NO_EXISTE'
  sleep 2
  echo 'ls /sysroot/etc/runlevels/default/'
  sleep 2
  echo 'ls /sysroot/etc/ssh/'
  sleep 2
  echo 'tail -30 /sysroot/var/log/rc.log 2>/dev/null || echo SIN_RC_LOG'
  sleep 3
} | telnet 172.16.42.1 2>/dev/null
