#!/usr/bin/python3
# MT6582 BLS backlight: lee el brillo (0-100) de /run/mt6582-bl-pct (que escribe el
# shim D-Bus de Phosh) y lo aplica al registro real PWM_DUTY +0xa0.
import os, mmap, struct, time
BASE=0x1400a000; DUTY=0xa0; EN=0x00; ENBIT=0x10000; DMAX=0x3ff
PCT="/run/mt6582-bl-pct"; SYS="/sys/class/backlight/backlight/"
fd=os.open("/dev/mem", os.O_RDWR|os.O_SYNC); mm=mmap.mmap(fd,0x1000,offset=BASE)
def w(o,v): mm[o:o+4]=struct.pack("<I", v & 0xffffffff)
def r(o): return struct.unpack("<I", mm[o:o+4])[0]
def rdf(n,d=0):
    try: return int(open(SYS+n).read().strip())
    except Exception: return d
if not os.path.exists(PCT):
    mx=rdf("max_brightness",10) or 10
    try:
        with open(PCT,"w") as f: f.write(str(int(round(rdf("actual_brightness",mx)/mx*100))))
        os.chmod(PCT,0o666)
    except Exception: pass
def getpct():
    try: return max(0,min(100,int(open(PCT).read().strip())))
    except Exception: return 75
while True:
    try:
        if (r(EN)&ENBIT)==0: w(EN, r(EN)|ENBIT)
        blank=rdf("bl_power",0)!=0
        pc=getpct()
        duty=0 if (blank or pc<=0) else max(0x10,int(round(pc/100.0*DMAX)))
        w(DUTY,duty)
    except Exception: pass
    time.sleep(0.1)
