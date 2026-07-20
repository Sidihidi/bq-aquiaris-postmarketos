#!/bin/sh
# wrapper XNU/Darwin sobre Linux:
#  1) -arch X (Apple) -> -target X-apple-darwin11 + ld64 Mach-O
#  2) sin -arch = build de HOST -> forzar el ld de GNU (nuestro ld64 esta antes en PATH)
#  3) relajar el C de 2014 que clang-19 rechaza (C23)
COMPAT="-Wno-register -Wno-writable-strings -Wno-c++11-narrowing -Wno-implicit-int -Wno-implicit-function-declaration -Wno-int-conversion -Wno-deprecated-non-prototype -Wno-error"
args=""; target=""
while [ $# -gt 0 ]; do
  case "$1" in
    -arch)
      shift
      case "$1" in
        armv7|armv7s|armv6) target="armv7-apple-darwin11" ;;
        arm64|aarch64)      target="aarch64-apple-darwin11" ;;
        *)                  target="$1-apple-darwin11" ;;
      esac ;;
    *) args="$args \"$1\"" ;;
  esac
  shift
done
if [ -n "$target" ]; then
  set -- -target "$target"
else
  set -- -fuse-ld=/usr/bin/ld.bfd -include sys/sysmacros.h
fi
eval exec /usr/bin/clang-19 "$@" $COMPAT $args
