#!/usr/bin/python3
# mt6582-backlight — aplica el brillo al registro real PWM_DUTY +0xa0 del BLS (0x1400a000).
#
# El kernel pwm-mtk-disp escribe el registro EQUIVOCADO (+0xac), por eso este daemon (OpenRC) lo
# corrige escribiendo +0xa0 vía /dev/mem.
#
# Lee de DOS fuentes (gana la que cambia):
#   - /sys/class/backlight/backlight/brightness  <- lo escribe Phosh (slider) vía logind SetBrightness
#   - /run/mt6582-bl-pct                          <- lo escribe el comando `bl`
#
# Remapea 0..100% -> MIN_PCT..100% para que el mínimo del slider NO deje la pantalla negra.
import os, mmap, struct, time

BASE = 0x1400a000; DUTY = 0xa0; EN = 0x00; ENBIT = 0x10000; DMAX = 0x3ff
MIN_PCT = 10
PCT = "/run/mt6582-bl-pct"; SYS = "/sys/class/backlight/backlight/"

fd = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
mm = mmap.mmap(fd, 0x1000, offset=BASE)

def w(o, v): mm[o:o + 4] = struct.pack("<I", v & 0xffffffff)
def r(o): return struct.unpack("<I", mm[o:o + 4])[0]
def rdf(n, d=0):
    try: return int(open(SYS + n).read().strip())
    except Exception: return d

MX = rdf("max_brightness", 10) or 10

def sys_pct():
    return max(0, min(100, int(round(rdf("brightness", 0) * 100.0 / MX))))

def run_pct():
    try: return max(0, min(100, int(open(PCT).read().strip())))
    except Exception: return None

last_sys = sys_pct(); last_run = run_pct()
cur = last_run if last_run is not None else last_sys
if not os.path.exists(PCT):
    try:
        with open(PCT, "w") as f: f.write(str(cur))
        os.chmod(PCT, 0o666); last_run = cur
    except Exception: pass

while True:
    try:
        s = sys_pct(); rn = run_pct()
        if s != last_sys: cur = s; last_sys = s          # slider de Phosh (logind -> /sys)
        if rn is not None and rn != last_run: cur = rn; last_run = rn   # comando bl
        if (r(EN) & ENBIT) == 0: w(EN, r(EN) | ENBIT)
        if rdf("bl_power", 0) != 0:
            duty = 0                                      # DPMS-off
        else:
            eff = MIN_PCT + cur * (100 - MIN_PCT) / 100.0  # 0..100% -> 10..100%
            duty = int(round(eff / 100.0 * DMAX))
        w(DUTY, duty)
    except Exception:
        pass
    time.sleep(0.1)
