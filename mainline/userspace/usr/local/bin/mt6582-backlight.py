#!/usr/bin/python3
# MT6582 backlight -> PWM_DUTY +0xa0. Pantalla on/off por el flag /run/mt6582-screen-off (power button).
# Brillo de /sys (slider de Phosh) + /run (cmd bl). Remapeo 0-100%->10-100%. NO honra bl_power (DPMS de phosh roto).
import os, mmap, struct, time
BASE=0x1400a000; DUTY=0xa0; EN=0x00; ENBIT=0x10000; DMAX=0x3ff; MIN_PCT=10
PCT="/run/mt6582-bl-pct"; SYS="/sys/class/backlight/backlight/"; OFF="/run/mt6582-screen-off"
fd=os.open("/dev/mem", os.O_RDWR|os.O_SYNC); mm=mmap.mmap(fd,0x1000,offset=BASE)
def w(o,v): mm[o:o+4]=struct.pack("<I", v & 0xffffffff)
def r(o): return struct.unpack("<I", mm[o:o+4])[0]
def rdf(n,d=0):
    try: return int(open(SYS+n).read().strip())
    except Exception: return d
MX=rdf("max_brightness",10) or 10
def sys_pct(): return max(0,min(100,int(round(rdf("brightness",0)*100.0/MX))))
def run_pct():
    try: return max(0,min(100,int(open(PCT).read().strip())))
    except Exception: return None
last_sys=sys_pct(); last_run=run_pct(); cur=last_run if last_run is not None else last_sys
if not os.path.exists(PCT):
    try:
        with open(PCT,"w") as f: f.write(str(cur))
        os.chmod(PCT,0o666); last_run=cur
    except Exception: pass
while True:
    try:
        s=sys_pct(); rn=run_pct()
        if s!=last_sys: cur=s; last_sys=s
        if rn is not None and rn!=last_run: cur=rn; last_run=rn
        if os.path.exists(OFF):
            # duty=0 NO corta la luz en este IP con el PWM habilitado (el usuario veia el
            # LCD iluminado con duty=0): hay que DESHABILITAR el PWM entero (bit EN).
            if r(EN)&ENBIT: w(EN, r(EN)&~ENBIT)
            duty=0
        else:
            if (r(EN)&ENBIT)==0: w(EN, r(EN)|ENBIT)
            eff=MIN_PCT+cur*(100-MIN_PCT)/100.0; duty=int(round(eff/100.0*DMAX))
        w(DUTY,duty)
    except Exception: pass
    time.sleep(0.1)
