#!/bin/sh
COMPAT="-fno-stack-protector -fno-builtin -Wno-cast-align -Wno-register -Wno-writable-strings -Wno-c++11-narrowing -Wno-implicit-int -Wno-implicit-function-declaration -Wno-int-conversion -Wno-deprecated-non-prototype -Wno-error"
args=""; target=""; save=""
for a in "$@"; do
  case "$a" in *bcopyinout*) save="-save-temps=obj";; esac
done
while [ $# -gt 0 ]; do
  case "$1" in
    -arch) shift
      case "$1" in
        armv7|armv7s|armv6) target="armv7-apple-darwin11" ;;
        arm64|aarch64) target="aarch64-apple-darwin11" ;;
        *) target="$1-apple-darwin11" ;;
      esac ;;
    *) args="$args \"$1\"" ;;
  esac
  shift
done
if [ -n "$target" ]; then set -- -target "$target" -mllvm -disable-loop-idiom-all $save
else set -- -fuse-ld=/usr/bin/ld.bfd -include sys/sysmacros.h; fi
eval exec /usr/bin/clang-19 "$@" $args $COMPAT
