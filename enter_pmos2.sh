#!/bin/bash
PUB=$(cat ~/.ssh/id_ed25519.pub)
SSHOPTS="-o StrictHostKeyChecking=no -o PreferredAuthentications=password -o PubkeyAuthentication=no"
sshpass -p 147147 ssh $SSHOPTS juan@172.16.42.1 "mkdir -p ~/.ssh && echo '$PUB' >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys && echo CLAVE_INSTALADA"
echo "=============================================="
ssh -o StrictHostKeyChecking=no -o IdentitiesOnly=yes -i ~/.ssh/id_ed25519 juan@172.16.42.1 'echo "*** POSTMARKETOS EN EL BQ AQUARIS E4.5 ***"; uname -a; grep PRETTY /etc/os-release; echo; uptime; free -m | head -2; df -h / | tail -1; echo "--- servicios default ---"; rc-status default 2>/dev/null | head -22'
