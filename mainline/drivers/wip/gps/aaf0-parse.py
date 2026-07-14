#!/usr/bin/env python3
"""
aaf0-parse.py — VÍA B: parser del stream binario 0xAAF0 del motor GPS (libmnlp).

Contexto (ver RECETA-BIONIC-VIA1-0714.md, STATUS noche-4d):
  El motor `libmnlp_mt6582` escribe sus reportes a `dae_snd_fd`, que standalone
  queda en fd 0 (default). Truco de captura: hacer fd 0 ESCRIBIBLE antes del execv.

CAPTURA (en el móvil, /tmp/launchB.py):
    import os
    os.chdir('/tmp')
    report = os.open('/tmp/report.bin', os.O_RDWR|os.O_CREAT|os.O_TRUNC, 0o644)
    os.dup2(report, 0)                 # fd 0 = report.bin (ESCRIBIBLE) -> reportes 0xAAF0
    r, w = os.pipe()
    nmea = os.open('/tmp/nmea21.bin', os.O_WRONLY|os.O_CREAT|os.O_TRUNC, 0o644)
    os.dup2(r, 20); os.dup2(nmea, 21)
    for fd in (0, 20, 21): os.set_inheritable(fd, True)
    os.set_inheritable(w, True)
    os.environ['LD_LIBRARY_PATH'] = '/system/lib'
    os.environ['LD_PRELOAD'] = '/system/lib/libxlogshim.so'
    os.environ['GPS_XLOG_QUIET'] = '1'
    os.execv('/system/xbin/libmnlp_mt6582', ['libmnlp', '20', '21', '0'])
  (requiere /tmp/sc/EPO.DAT: `mkdir -p /tmp/sc; dd if=/dev/zero of=/tmp/sc/EPO.DAT bs=1 count=2304`)

FORMATO DE FRAME (observado en HW, mismo framing que el DSP /dev/stpgps):
    AA F0 | len(2 LE) | payload[len] | AA 0F
    payload[0:2] = tipo (LE).  Tipos vistos en INTERIOR (sin fix):
      0xFE05 len=6   -> status/heartbeat (05 fe 04 00 0d 01)
      0xFE08 len=110 -> reporte de medidas/estado (con datos DUMMY en interior:
                        "HIJKLMNO"=48..4F secuencial + ceros = 0 satelites)

⚠️ INTERIOR = INUTIL para decodificar: sin fix los campos de posicion van a cero/dummy.
   Para construir el decodificador 0xAAF0->NMEA hace falta un `report.bin` con FIX REAL
   (movil con vista al cielo). Con coords conocidas se localizan los `double LLH[4]`
   (lat/lon IEEE-754, 8 bytes) dentro del frame y se mapea el resto (time, sats, HDOP).

Uso:  python3 aaf0-parse.py report.bin
"""
import sys
import struct
import collections


def parse(data):
    SYNC, TRAIL = b'\xaa\xf0', b'\xaa\x0f'
    i, frames = 0, []
    while i < len(data) - 6:
        if data[i:i + 2] != SYNC:
            i += 1
            continue
        ln = struct.unpack('<H', data[i + 2:i + 4])[0]
        end = i + 4 + ln
        if end + 2 > len(data) or data[end:end + 2] != TRAIL:
            i += 1
            continue
        payload = data[i + 4:end]
        ftype = struct.unpack('<H', payload[0:2])[0] if len(payload) >= 2 else -1
        frames.append((ftype, ln, payload))
        i = end + 2
    return frames


def find_doubles(payload, lo=-180.0, hi=180.0):
    """Busca secuencias de 8 bytes que parezcan un double plausible (lat/lon)."""
    hits = []
    for off in range(0, len(payload) - 8 + 1):
        for endian in ('<', '>'):
            try:
                v = struct.unpack(endian + 'd', payload[off:off + 8])[0]
            except struct.error:
                continue
            if v == v and abs(v) > 1e-6 and lo <= v <= hi:  # no NaN, no 0, en rango
                hits.append((off, endian, v))
    return hits


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else 'report.bin'
    data = open(path, 'rb').read()
    frames = parse(data)
    print("fichero %s: %d bytes, %d frames 0xAAF0" % (path, len(data), len(frames)))

    inv = collections.Counter((f[0], f[1]) for f in frames)
    print("=== inventario por (tipo, len) ===")
    for (t, ln), c in sorted(inv.items(), key=lambda x: -x[1]):
        print("  tipo 0x%04X  len=%-4d  x%d" % (t, ln, c))

    print("=== ejemplo de cada tipo (payload hex) ===")
    seen = set()
    for t, ln, pl in frames:
        if t in seen:
            continue
        seen.add(t)
        print("  0x%04X: %s" % (t, pl.hex()))

    # Cuando haya un fix real: buscar candidatos a lat/lon en los frames grandes.
    print("=== candidatos a double lat/lon (solo utiles CON FIX real) ===")
    for t, ln, pl in frames:
        if ln < 32:
            continue
        for off, endian, v in find_doubles(pl):
            print("  frame 0x%04X off=%d %s -> %.6f" % (t, off, endian, v))


if __name__ == '__main__':
    main()
