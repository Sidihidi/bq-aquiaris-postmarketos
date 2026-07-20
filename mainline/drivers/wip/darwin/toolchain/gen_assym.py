#!/usr/bin/env python3
# Convierte genassym.o (con marcadores @DEFINITION# de clang/ARM) en assym.s
import sys, re
for line in open(sys.argv[1], errors="replace"):
    if "DEFINITION#" not in line:
        continue
    body = line.split("DEFINITION#", 1)[1].rstrip("\n")
    # body: "#define NAME \t#VALUE"  (o con $VALUE)
    body = body.replace("$", "")
    m = re.match(r"(#define\s+\w+)\s+#?(\S+)", body)
    if not m:
        continue
    name_def, val = m.group(1), m.group(2)
    print(f"{name_def}\t{val}")                       # #define NAME  VALUE
    nm = name_def.split()[1]
    print(f"#define {nm}_NUM\t{val}")                 # #define NAME_NUM VALUE
