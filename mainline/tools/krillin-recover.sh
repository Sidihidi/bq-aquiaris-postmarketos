#!/bin/bash
# krillin-recover.sh — recuperar el telefono (pmOS o Android). Ejecutar EN LA PI.
# Uso: bash krillin-recover.sh {softreboot|forcereboot|fastboot <img>|brom}
PH=root@172.16.42.1
O="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o ConnectTimeout=5"
IMGDIR=~/mainline/pkg
op="${1:-help}"
case "$op" in
  softreboot)
    sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
    ssh -n $O $PH "reboot" 2>/dev/null && echo "reboot normal enviado" ;;
  forcereboot)
    # salta el apagado de OpenRC (util cuando se cuelga en 'Stopping System Message Bus')
    sudo ip addr replace 172.16.42.2/24 dev usb0 2>/dev/null
    ssh -n $O $PH "sync; reboot -f" 2>/dev/null && echo "reboot -f enviado (salta apagado colgado)" ;;
  fastboot)
    img="${2:?uso: krillin-recover.sh fastboot <img.img>}"
    [ -f "$img" ] || img="$IMGDIR/$img"
    [ -f "$img" ] || { echo "no existe la imagen: $img"; ls "$IMGDIR"/*.img 2>/dev/null; exit 1; }
    echo ">> Pon el telefono en FASTBOOT (apagado + Vol- mientras enchufas USB). Enter para flashear:"
    echo "   $img"; read _
    sudo fastboot flash boot "$img" && sudo fastboot reboot && echo "flasheado + reboot" ;;
  brom)
    echo "RECUPERACION BROM (ladrillo total) — ver GUIA-MTKCLIENT.md. Resumen:"
    echo "  1) telefono apagado / bateria fuera"
    echo "  2) en la Pi: bash ~/capa0-backup.sh   (lee/diagnostica, NO escribe)"
    echo "  3) enchufar USB manteniendo Vol- (probar Vol+ si no entra)"
    echo "  REGLA DE ORO: NUNCA flashear el preloader."
    echo "  Restaurar pmOS booteable:  krillin-recover.sh fastboot boot-btifDMA-sd.img" ;;
  *)
    echo "Uso: krillin-recover.sh {softreboot|forcereboot|fastboot <img>|brom}"
    echo "  softreboot    reboot normal (si responde SSH)"
    echo "  forcereboot   reboot -f (si el apagado/reinicio se cuelga)"
    echo "  fastboot IMG  flashear boot por fastboot (recupera pmOS o Android segun la img)"
    echo "  brom          guia de recuperacion de ladrillo (mtkclient/golden backup)" ;;
esac
