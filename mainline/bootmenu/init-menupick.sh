#!/bin/busybox sh
# init-menupick v2 (ROBUSTO) — initramfs con menu de seleccion de SO para el krillin.
# Muestra el menu en pantalla, lee Vol+/- + Power, y arranca el SO elegido.
#
# Entradas:
#   0: postmarketOS (Alpine + Phosh) — switch_root a mmcblk1p1
#   1: Android stock — kexec del kernel 3.10 desde /boot/android-boot.img (mmcblk1p1)
#   2: Maemo Leste (Devuan + Hildon) — switch_root a mmcblk1p3
#
# v2: boot_rootfs() robusto — espera el device, reintenta el mount, cae a -o noload
# si el journal esta sucio, acepta /sbin/init como fichero O symlink (-e/-L), remonta rw,
# y solo va a EMERGENCIA si de verdad no hay rootfs arrancable.

BB=/bin/busybox
export PATH=/bin:/sbin:/usr/bin:/usr/sbin

$BB mount -t proc     proc /proc  2>/dev/null
$BB mount -t sysfs    sys  /sys   2>/dev/null
$BB mount -t devtmpfs dev  /dev   2>/dev/null
$BB mount -t tmpfs    tmp  /tmp   2>/dev/null

klog() { echo "menupick: $*" > /dev/kmsg 2>/dev/null; }
klog "init v2 started"

# boot_rootfs <device> — monta el rootfs ext4 y hace switch_root. NO vuelve si tiene exito.
# Devuelve 1 si no se pudo (para caer al siguiente fallback).
boot_rootfs() {
    dev="$1"
    # 1) esperar a que el block device exista (SD tarda en enumerar)
    i=0; while [ $i -lt 25 ]; do [ -b "$dev" ] && break; $BB sleep 1; i=$((i+1)); done
    [ -b "$dev" ] || { klog "no existe $dev"; return 1; }
    $BB mkdir -p /newroot
    # 2) montar rw: plano, y si falla (journal sucio) con noload. Reintentar cada uno.
    mounted=0
    for opt in "" "-o noload"; do
        j=0; while [ $j -lt 3 ]; do
            if $BB mount -t ext4 $opt "$dev" /newroot 2>/dev/null; then mounted=1; break; fi
            $BB sleep 1; j=$((j+1))
        done
        [ "$mounted" = 1 ] && break
    done
    if [ "$mounted" != 1 ]; then klog "mount $dev FALLO (todas las opciones)"; return 1; fi
    # 3) validar init: fichero real O symlink (el symlink resuelve tras el pivot; -e solo
    #    seguiria el enlace contra el root del initramfs y daria falso negativo).
    if [ -e /newroot/sbin/init ] || [ -L /newroot/sbin/init ] || [ -e /newroot/init ]; then
        klog "montado $dev OK -> switch_root"
        $BB mount -o remount,rw /newroot 2>/dev/null   # por si vino de noload
        $BB mount --move /dev  /newroot/dev  2>/dev/null
        $BB mount --move /proc /newroot/proc 2>/dev/null
        $BB mount --move /sys  /newroot/sys  2>/dev/null
        exec $BB switch_root /newroot /sbin/init
    fi
    klog "sin /sbin/init en $dev"
    $BB umount /newroot 2>/dev/null
    return 1
}

# --- Menu en pantalla ---
i=0; while [ $i -lt 30 ]; do [ -e /dev/fb0 ] && [ -e /dev/input/event0 ] && break; $BB sleep 1; i=$((i+1)); done
if [ ! -e /dev/fb0 ]; then
    klog "NO fb0 — auto-boot pmOS"; CHOICE=0
else
    CHOICE=$(/bin/menupick "BQ Aquaris E4.5" "postmarketOS" "Android" "Maemo Leste" 2>/dev/null)
    [ -z "$CHOICE" ] && CHOICE=0
fi
klog "seleccion=$CHOICE"

case "$CHOICE" in
    2)
        klog "-> Maemo Leste (mmcblk1p3)"
        boot_rootfs /dev/mmcblk1p3
        klog "Maemo no arranco -> fallback pmOS"
        boot_rootfs /dev/mmcblk1p1
        ;;
    1)
        klog "-> Android (kexec)"
        $BB mkdir -p /sdboot
        if $BB mount -t ext4 -o ro,noload /dev/mmcblk1p1 /sdboot 2>/dev/null; then
            IMG=/sdboot/boot/android-boot.img
            if [ -f "$IMG" ]; then
                /sbin/kexec --type=zImage --load="$IMG" 2>/dev/null || \
                /sbin/kexec --load="$IMG" --command-line="console=ttyMT0,921600n1 console=tty0 clk_ignore_unused" 2>/dev/null
                if [ $? -eq 0 ]; then $BB umount /sdboot 2>/dev/null; exec /sbin/kexec -e; fi
                klog "kexec fallo"
            else
                klog "no android-boot.img en la SD"
            fi
            $BB umount /sdboot 2>/dev/null
        fi
        klog "Android no arranco -> fallback pmOS"
        boot_rootfs /dev/mmcblk1p1
        ;;
    *)
        klog "-> postmarketOS (mmcblk1p1)"
        boot_rootfs /dev/mmcblk1p1
        ;;
esac

# === EMERGENCIA: si todo falla, shell por USB (dropbear en usb0) ===
klog "EMERGENCIA — dropbear por USB (172.16.42.1)"
$BB --install -s /bin 2>/dev/null
$BB mkdir -p /dev/pts; $BB mount -t devpts devpts /dev/pts 2>/dev/null
[ -e /dev/watchdog ] && printf 'V' > /dev/watchdog 2>/dev/null
i=0; while [ $i -lt 40 ]; do [ -e /sys/class/net/usb0 ] && break; $BB sleep 1; i=$((i+1)); done
$BB ifconfig usb0 172.16.42.1 netmask 255.255.255.0 up 2>/dev/null
mkdir -p /etc/dropbear
[ -f /etc/dropbear/dropbear_ed25519_host_key ] || /bin/dropbearmulti dropbearkey -t ed25519 -f /etc/dropbear/dropbear_ed25519_host_key >/dev/kmsg 2>&1
/bin/dropbearmulti dropbear -p 22 -r /etc/dropbear/dropbear_ed25519_host_key >/dev/kmsg 2>&1 &
while true; do $BB sleep 3600; done
