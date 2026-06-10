#!/bin/bash
{ sleep 2
  echo 'reboot -f'
  sleep 2
} | telnet 172.16.42.1 2>/dev/null | tail -2
echo REBOOT_ENVIADO
