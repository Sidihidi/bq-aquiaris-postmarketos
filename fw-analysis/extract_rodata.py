#!/usr/bin/env python3
# Convierte rodata.txt (addr=val) a binario y extrae strings de modulo wifi/*.c
import struct, re
from collections import OrderedDict

out = bytearray()
rows = []
with open("/tmp/rodata.txt") as f:
    for line in f:
        line = line.strip()
        if "=" not in line:
            continue
        addr_s, val_s = line.split("=", 1)
        addr = int(addr_s, 16)
        val = int(val_s, 16)
        rows.append((addr, val))
        out += struct.pack("<I", val)

with open("/tmp/rodata.bin", "wb") as f:
    f.write(out)
print("rodata: %d palabras -> %d bytes (0x%x - 0x%x)" % (len(rows), len(out), rows[0][0], rows[-1][0]))

# Base address de la rodata dumpada
base = rows[0][0]   # 0xf0063000

# Extraer strings: buscar secuencias ascii imprimibles >= 4 chars
data = out
strings = []  # (offset_in_bin, string)
cur = bytearray()
cur_start = None
for i, b in enumerate(data):
    if 0x20 <= b < 0x7f:
        if cur_start is None:
            cur_start = i
        cur.append(b)
    else:
        if cur_start is not None and len(cur) >= 4:
            strings.append((cur_start, bytes(cur).decode("ascii", "replace")))
        cur = bytearray()
        cur_start = None
if cur_start is not None and len(cur) >= 4:
    strings.append((cur_start, bytes(cur).decode("ascii", "replace")))

# Filtrar strings que parezcan rutas de modulo wifi/
mod_strings = {}   # addr -> string
with open("/tmp/rodata_strings.txt", "w") as f:
    for off, s in strings:
        addr = base + off
        f.write("0x%08x  %s\n" % (addr, s))
        if re.search(r"wifi/.*\.c", s) or "wifi/" in s or s.endswith(".c"):
            mod_strings[addr] = s

print("Strings totales: %d" % len(strings))
print("Strings de modulo (.c / wifi/): %d" % len(mod_strings))
with open("/tmp/mod_strings_resolved.txt", "w") as f:
    for a in sorted(mod_strings):
        f.write("0x%x = %s\n" % (a, mod_strings[a]))
print("-> /tmp/mod_strings_resolved.txt")
