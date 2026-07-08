#!/bin/busybox sh
# init-menupick — initramfs con menu de seleccion de SO para el krillin.
# Muestra el menu en pantalla, lee Vol+/- + Power, y arranca el SO elegido.
#
# Entradas:
#   0: postmarketOS (Alpine + Phosh) — switch_root a mmcblk1p1
#   1: Android stock — kexec del kernel 3.10 desde /boot/android-boot.img
#   (futuro: 2: Maemo Leste — switch_root a mmcblk1p3)

BB=/bin/busybox
export PATH=/bin:/sbin:/usr/bin:/usr/sbin

# Montar lo basico
$BB mount -t proc     proc /proc  2>/dev/null
$BB mount -t sysfs    sys  /sys   2>/dev/null
$BB mount -t devtmpfs dev  /dev   2>/dev/null
$BB mount -t tmpfs    tmp  /tmp   2>/dev/null

echo "menupick: init started" > /dev/kmsg 2>/dev/null

# Esperar a que el framebuffer y los input devices aparezcan
i=0
while [ $i -lt 30 ]; do
    [ -e /dev/fb0 ] && [ -e /dev/input/event0 ] && break
    $BB sleep 1; i=$((i+1))
done

if [ ! -e /dev/fb0 ]; then
    echo "menupick: NO fb0 — auto-boot pmOS" > /dev/kmsg 2>/dev/null
    CHOICE=0
else
    # Ejecutar el menu. Devuelve 0 o 1 por stdout.
    # Auto-boot tras 10s si no hay seleccion.
    CHOICE=$(/bin/menupick "BQ Aquaris E4.5" "postmarketOS" "Android" 2>/dev/null)
    [ -z "$CHOICE" ] && CHOICE=0
fi

echo "menupick: seleccion=$CHOICE" > /dev/kmsg 2>/dev/null

case "$CHOICE" in
    0)
        # === postmarketOS: switch_root a mmcblk1p1 (SD) ===
        echo "menupick: arrancando postmarketOS..." > /dev/kmsg 2>/dev/null
        i=0; while [ $i -lt 20 ]; do [ -b /dev/mmcblk1p1 ] && break; $BB sleep 1; i=$((i+1)); done
        $BB mkdir -p /newroot
        if $BB mount -t ext4 /dev/mmcblk1p1 /newroot 2>/dev/null && [ -e /newroot/sbin/init ]; then
            $BB mount --move /dev  /newroot/dev  2>/dev/null
            $BB mount --move /proc /newroot/proc 2>/dev/null
            $BB mount --move /sys  /newroot/sys  2>/dev/null
            exec $BB switch_root /newroot /sbin/init
        fi
        echo "menupick: FALLO mount pmOS — emergencia" > /dev/kmsg 2>/dev/null
        ;;

    1)
        # === Android: kexec del kernel 3.10 ===
        echo "menupick: arrancando Android via kexec..." > /dev/kmsg 2>/dev/null
        # Montar la particion boot de la SD para leer android-boot.img
        $BB mkdir -p /sdboot
        # Android-boot.img esta en la SD (mmcblk1p1) o en una particion dedicada
        if $BB mount -t ext4 /dev/mmcblk1p1 /sdboot 2>/dev/null; then
            ANDROID_IMG=/sdboot/boot/android-boot.img
            if [ -f "$ANDROID_IMG" ]; then
                echo "menupick: encontrado android-boot.img, lanzando kexec..." > /dev/kmsg 2>/dev/null
                # kexec del boot.img completo (kexec lo parsea)
                /sbin/kexec --type=zImage --load="$ANDROID_IMG" 2>/dev/null || \
                /sbin/kexec --load="$ANDROID_IMG" --command-line="console=ttyMT0,921600n1 console=tty0 clk_ignore_unused" 2>/dev/null
                if [ $? -eq 0 ]; then
                    $BB umount /sdboot 2>/dev/null
                    exec /sbin/kexec -e
                fi
                echo "menupick: kexec fallo — fallback a pmOS" > /dev/kmsg 2>/dev/null
            else
                echo "menupick: no android-boot.img en SD — fallback a pmOS" > /dev/kmsg 2>/dev/null
            fi
            $BB umount /sdboot 2>/dev/null
        else
            echo "menupick: no SD para android-boot.img — fallback a pmOS" > /dev/kmsg 2>/dev/null
        fi
        # Fallback: arrancar pmOS
        CHOICE=0
        i=0; while [ $i -lt 20 ]; do [ -b /dev/mmcblk1p1 ] && break; $BB sleep 1; i=$((i+1)); done
        $BB mkdir -p /newroot
        if $BB mount -t ext4 /dev/mmcblk1p1 /newroot 2>/dev/null && [ -e /newroot/sbin/init ]; then
            $BB mount --move /dev  /newroot/dev  2>/dev/null
            $BB mount --move /proc /newroot/proc 2>/dev/null
            $BB mount --move /sys  /newroot/sys  2>/dev/null
            exec $BB switch_root /newroot /sbin/init
        fi
        ;;
esac

# === EMERGENCIA: si todo falla, shell por USB ===
echo "menupick: EMERGENCIA — dropbear por USB" > /dev/kmsg 2>/dev/null
$BB --install -s /bin 2>/dev/null
$BB mkdir -p /dev/pts; $BB mount -t devpts devpts /dev/pts 2>/dev/null
[ -e /dev/watchdog ] && printf 'V' > /dev/watchdog 2>/dev/null
i=0; while [ $i -lt 40 ]; do [ -e /sys/class/net/usb0 ] && break; $BB sleep 1; i=$((i+1)); done
$BB ifconfig usb0 172.16.42.1 netmask 255.255.255.0 up 2>/dev/null
mkdir -p /etc/dropbear
[ -f /etc/dropbear/dropbear_ed25519_host_key ] || /bin/dropbearmulti dropbearkey -t ed25519 -f /etc/dropbear/dropbear_ed25519_host_key >/dev/kmsg 2>&1
/bin/dropbearmulti dropbear -p 22 -r /etc/dropbear/dropbear_ed25519_host_key >/dev/kmsg 2>&1 &
while true; do $BB sleep 3600; done
