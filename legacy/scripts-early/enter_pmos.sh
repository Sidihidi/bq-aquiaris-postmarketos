#!/bin/bash
which sshpass >/dev/null || sudo apt-get install -y -q sshpass >/dev/null 2>&1
[ -f ~/.ssh/id_ed25519 ] || ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519 -N '' -q
sshpass -p 147147 ssh-copy-id -o StrictHostKeyChecking=no juan@172.16.42.1 >/dev/null 2>&1 && echo CLAVE_INSTALADA
echo "=============================================="
ssh -o StrictHostKeyChecking=no juan@172.16.42.1 'echo "*** POSTMARKETOS EN EL BQ AQUARIS E4.5 ***"; uname -a; grep PRETTY /etc/os-release; echo; uptime; free -m | head -2; df -h / | tail -1; echo; echo "--- servicios ---"; rc-status default 2>/dev/null | head -20'
