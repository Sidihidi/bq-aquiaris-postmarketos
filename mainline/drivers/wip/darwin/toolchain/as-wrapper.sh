#!/bin/sh
# el as de cctools necesita backends no instalados; usamos el ensamblador
# integrado de clang, que si sabe generar Mach-O armv7
exec /opt/Developer/Toolchains/DarwinARM.toolchain/usr/bin/clang -c -x assembler "$@"
