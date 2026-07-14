#!/usr/bin/env python3
# mtk_hdr.py NAME infile outfile
# Recreado 0714 tras perder pkg/, del formato exacto de la imagen de boot del movil.
# Cabecera MTK de 512 bytes: magic(88 16 88 58) + size(LE u32 del payload) +
# nombre padded a 32 con nulls + relleno 0xff hasta 512, seguido del payload.
import sys, struct

name, infile, outfile = sys.argv[1], sys.argv[2], sys.argv[3]
data = open(infile, "rb").read()

hdr = struct.pack("<I", 0x58881688)              # magic -> bytes 88 16 88 58
hdr += struct.pack("<I", len(data))              # tamano del payload
nb = name.encode()
hdr += nb + b"\x00" * (32 - len(nb))             # nombre (KERNEL/ROOTFS) en 32 bytes
hdr += b"\xff" * (512 - len(hdr))                # relleno 0xff hasta 512

open(outfile, "wb").write(hdr + data)
print("mtk_hdr: name=%s payload=%d bytes (+512 hdr)" % (name, len(data)))
