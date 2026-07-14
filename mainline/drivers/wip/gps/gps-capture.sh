#!/bin/sh
# ============================================================================
# gps-capture.sh — capturador del GPS con feedback visual (VÍA B).
# Correr en el MÓVIL (root@172.16.42.1) CON VISTA AL CIELO.
#
#   sh /data/gps-capture.sh
#
# Arranca el motor GPS stock (libmnlp bionic), captura TODO lo útil y muestra
# feedback en vivo. Ctrl-C para parar. Deja los ficheros en /data/gps_mnl/ para
# que la sesión de casa los lea:
#   - report.bin : stream binario 0xAAF0 del motor (truco del fd0 escribible).
#   - logd.cap   : log ALOGD (con debug_nmea=1: si el motor loguea NMEA con un
#                  fix real, aquí saldrán las sentencias $GP directas).
#   - nmea21.bin : lo que salga por el fd1 del pipe (por si acaso).
#   - xlog.txt   : narración del motor.
# Analizar report.bin en casa con:  python3 aaf0-parse.py report.bin
# Ver RECETA-BIONIC-VIA1-0714.md (STATUS noche-4d/4e).
# ============================================================================
OUT=/data/gps_mnl
mkdir -p "$OUT" /tmp/sc /data/misc

# --- prerequisitos ---
if [ ! -e /dev/stpgps ]; then
  echo "ERROR: /dev/stpgps no existe — el DSP GPS no está listo."
  echo "       Hace falta el bring-up del CONSYS/GPS (radio ON). Revisa dmesg / servicios."
  exit 1
fi
if [ ! -e /system/xbin/libmnlp_mt6582 ] || [ ! -e /system/lib/libxlogshim.so ]; then
  echo "ERROR: falta el prefijo bionic (/system/xbin/libmnlp_mt6582 o /system/lib/libxlogshim.so)."
  echo "       Restaurar de ~/gps-bionic-prefix.tar.gz (Pi)."
  exit 1
fi

# EPO.DAT (gate del motor: sin él sale exit 255)
[ -s /tmp/sc/EPO.DAT ] || dd if=/dev/zero of=/tmp/sc/EPO.DAT bs=1 count=2304 2>/dev/null

# config: activar el logging de NMEA (por si con fix el motor lo emite por ALOGD)
cat > /data/misc/mnl.prop <<'PROP'
dev.dsp=/dev/stpgps
dev.gps=/dev/gps
debug.debug_nmea=1
debug.mnl=ff
pmtk.conn=serial
PROP

# listener de logdw (captura ALOGD sin interposición)
cat > /tmp/logdw.py <<'PY'
import socket, os
p = '/dev/socket/logdw'
os.makedirs('/dev/socket', exist_ok=True)
try:
    os.unlink(p)
except OSError:
    pass
s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
s.bind(p); os.chmod(p, 0o666); s.settimeout(2)
f = open('/data/gps_mnl/logd.cap', 'wb')
while True:
    try:
        f.write(s.recv(8192) + b'\n'); f.flush()
    except socket.timeout:
        continue
    except Exception:
        break
PY

# launcher del motor: fd0 ESCRIBIBLE -> report.bin (ahí caen los frames 0xAAF0)
cat > /tmp/launchB.py <<'PY'
import os
os.chdir('/tmp')
report = os.open('/data/gps_mnl/report.bin', os.O_RDWR | os.O_CREAT | os.O_TRUNC, 0o644)
os.dup2(report, 0)
r, w = os.pipe()
nmea = os.open('/data/gps_mnl/nmea21.bin', os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
os.dup2(r, 20); os.dup2(nmea, 21)
for fd in (0, 20, 21):
    os.set_inheritable(fd, True)
os.set_inheritable(w, True)
os.environ['LD_LIBRARY_PATH'] = '/system/lib'
os.environ['LD_PRELOAD'] = '/system/lib/libxlogshim.so'
os.environ['GPS_XLOG_QUIET'] = '1'
os.execv('/system/xbin/libmnlp_mt6582', ['libmnlp', '20', '21', '0'])
PY

# --- arrancar ---
pkill -f libmnlp 2>/dev/null; pkill -f logdw.py 2>/dev/null; sleep 1
rm -f "$OUT/report.bin" "$OUT/logd.cap" "$OUT/nmea21.bin" "$OUT/xlog.txt"
python3 /tmp/logdw.py >/dev/null 2>&1 &
sleep 1
python3 /tmp/launchB.py > "$OUT/xlog.txt" 2>&1 &

cleanup() {
    echo ""; echo "== parando =="
    pkill -f libmnlp 2>/dev/null; pkill -f logdw.py 2>/dev/null
    echo "LISTO. Ficheros para casa en $OUT/ :"
    ls -la "$OUT/"report.bin "$OUT/"logd.cap "$OUT/"xlog.txt 2>/dev/null
    echo "En casa:  python3 aaf0-parse.py report.bin"
    exit 0
}
trap cleanup INT TERM

echo "======== CAPTURA GPS ARRANCADA — MANTÉN EL MÓVIL CON VISTA AL CIELO ========"
echo "(Ctrl-C para parar cuando lleves unos minutos / tengas fix)"; sleep 2

# --- feedback en vivo ---
while true; do
    sleep 4
    python3 - <<'PYFB'
import struct, collections, re


def load(p):
    try:
        return open(p, 'rb').read()
    except Exception:
        return b''


data = load('/data/gps_mnl/report.bin')
SYNC, TRAIL = b'\xaa\xf0', b'\xaa\x0f'
i, fr = 0, []
while i < len(data) - 6:
    if data[i:i + 2] != SYNC:
        i += 1; continue
    ln = struct.unpack('<H', data[i + 2:i + 4])[0]
    e = i + 4 + ln
    if e + 2 > len(data) or data[e:e + 2] != TRAIL:
        i += 1; continue
    fr.append((struct.unpack('<H', data[i + 4:i + 6])[0] if ln >= 2 else -1, ln, data[i + 4:e]))
    i = e + 2
inv = collections.Counter((f[0], f[1]) for f in fr)
big = [f for f in fr if f[1] >= 100]
distinct = len(set(f[2] for f in big))
log = load('/data/gps_mnl/logd.cap')
gps = re.findall(rb'\$G[PN][A-Z]{3}[^*\r\n]*\*[0-9A-Fa-f]{2}', log)

print("\033[2J\033[H", end="")
print("========= CAPTURA GPS EN CURSO  (Ctrl-C para parar) =========")
print("report.bin: %d bytes, %d frames  |  logd.cap: %d bytes" % (len(data), len(fr), len(log)))
for (t, ln), c in sorted(inv.items(), key=lambda x: -x[1]):
    print("   frame 0x%04X  len=%-4d  x%d" % (t, ln, c))
print("   actividad 0xFE08: %d frames, %d payloads DISTINTOS" % (len(big), distinct))
if gps:
    print("")
    print(">>> !!!!!  NMEA DETECTADO: %d sentencias  !!!!!" % len(gps))
    for g in gps[-8:]:
        print("     " + g.decode('ascii', 'replace'))
else:
    print("")
    print(">>> sin NMEA todavia. 'payloads distintos' sube al adquirir satelites.")
print("")
print("(fix en frio: 30s a varios minutos con cielo despejado)")
PYFB
done
