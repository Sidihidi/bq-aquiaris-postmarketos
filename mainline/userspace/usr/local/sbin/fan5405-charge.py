#!/usr/bin/env python3
# fan5405-charge.py — mantenedor de carga del BQ Aquaris E4.5 ("krillin") por i2c DIRECTO.
#
# Por qué existe (además del fan5405-charge en /bin de pmOS): Maemo Leste NO trae i2c-tools
# (i2cset/i2cget) pero SÍ /dev/i2c-0 + python3. Este daemon habla el bus por ioctl I2C_SLAVE,
# así que funciona en CUALQUIER rootfs del dual-boot sin dependencias de paquetes.
#
# HW: FAN5405 en I2C bus 0, dir 0x6a (vendor=4 confirmado). El watchdog del chip es ~22-24s:
# si no se le "patea" (kick), revierte a 100mA/3.54V y el sistema drena mas de lo que entra
# -> la bateria cae -> apagados/brownout (era la causa raiz de los reinicios de Maemo con
# el rootfs sin gestion de carga). Kick cada 10s.
#
# Valores SEGUROS (identicos al daemon de pmOS): CV=4.20V (OREG=35, topado por VSAFE/CON6),
# IINLIM=800mA, IOCHARGE=default, TE=1 (termina al llenar), CE=enable.
#
# Boot: servicio OpenRC /etc/init.d/fan5405-charge (command_background) o linea en rc.local.
import fcntl, os, time, sys

I2C_SLAVE = 0x0703
ADDR = 0x6a
BUS = "/dev/i2c-0"


def openbus():
    fd = os.open(BUS, os.O_RDWR)
    fcntl.ioctl(fd, I2C_SLAVE, ADDR)
    return fd


def wr(fd, reg, val):
    os.write(fd, bytes([reg & 0xff, val & 0xff]))


def rd(fd, reg):
    os.write(fd, bytes([reg & 0xff]))
    return os.read(fd, 1)[0]


def cycle(fd):
    wr(fd, 0x02, 0x8c)          # CON2: OREG=4.20V
    wr(fd, 0x01, 0xb8)          # CON1: IINLIM=800mA, TE=1, CE=on
    c0 = rd(fd, 0x00)
    wr(fd, 0x00, c0 | 0x80)     # CON0: kick watchdog (bit7 = TMR_RST)
    return c0


STAT = {0: "ready", 1: "CARGANDO", 2: "COMPLETA", 3: "FALLO"}

if __name__ == "__main__":
    once = "--once" in sys.argv
    fd = openbus()
    c0 = cycle(fd)
    print("FAN5405 kick OK, STAT=%s (CON0=0x%02x)" % (STAT.get((rd(fd, 0x00) >> 4) & 3, "?"), c0))
    if once:
        sys.exit(0)
    while True:
        time.sleep(10)
        try:
            cycle(fd)
        except Exception:
            try:
                fd = openbus()
            except Exception:
                pass
