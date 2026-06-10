#!/usr/bin/env python3
"""Prepend MTK 512-byte header: magic 0x58881688, size LE, label[32], 0xFF pad."""
import struct, sys
label, src, dst = sys.argv[1], sys.argv[2], sys.argv[3]
data = open(src, 'rb').read()
hdr = struct.pack('<II', 0x58881688, len(data)) + label.encode().ljust(32, b'\0')
hdr = hdr.ljust(512, b'\xff')
open(dst, 'wb').write(hdr + data)
print(f"{dst}: {len(data)} bytes payload + MTK hdr '{label}'")
