#!/bin/bash
sudo sysctl -w net.ipv4.ip_forward=1 >/dev/null
sudo nft list table ip pmosnat >/dev/null 2>&1 || {
  sudo nft add table ip pmosnat
  sudo nft 'add chain ip pmosnat post { type nat hook postrouting priority 100 ; }'
  sudo nft add rule ip pmosnat post ip saddr 172.16.42.0/24 oifname "wlan0" masquerade
}
echo "NAT nft listo"
ssh -o StrictHostKeyChecking=no user@172.16.42.1 '
ping -c2 -W3 1.1.1.1 >/dev/null 2>&1 && echo INTERNET_OK || echo SIN_INTERNET
nslookup postmarketos.org >/dev/null 2>&1 && echo DNS_OK || echo SIN_DNS
echo 147147 | sudo -S apk add fbset 2>&1 | tail -1
which fbset && echo FBSET_INSTALADO
'
