#!/usr/bin/env python3
# Amp externo del altavoz (GPIO118) — tren de pulsos de Sound_Speaker_Turnon (yusu_android_speaker.c):
# HIGH,udelay2, LOW,udelay2, HIGH,udelay2, LOW,udelay2, HIGH, msleep(40). Selecciona modo/ganancia por conteo.
import mmap, os, struct, time
fd = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
mm = mmap.mmap(fd, 0x1000, offset=0x10005000)
def w32(off, val): mm[off:off+4] = struct.pack("<I", val)
SET, RST, BIT = 0x474, 0x478, 0x40   # DOUT_SET/RST reg7, GPIO118 bit6
def hi(): w32(SET, BIT)
def lo(): w32(RST, BIT)
def udelay(us):
    e = time.perf_counter() + us/1e6
    while time.perf_counter() < e: pass
# asegurar dir out (DIR_SET reg7 bit6) por si acaso
w32(0x074, BIT)
hi(); udelay(2)
lo(); udelay(2)
hi(); udelay(2)
lo(); udelay(2)
hi()                 # queda HIGH
time.sleep(0.040)    # SPK_WARM_UP_TIME
print("GPIO118 pulse done, DOUT reg7 =", hex(struct.unpack("<I", mm[0x470:0x474])[0]))
