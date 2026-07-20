#!/bin/sh
# stub de xcrun para builds de XNU en Linux: el build solo usa
#   xcrun --show-sdk-path      -> raiz del SDK
#   xcrun [-sdk X] -find TOOL  -> ruta de la herramienta
TOOLCHAIN=/opt/Developer/Toolchains/DarwinARM.toolchain/usr/bin
while [ $# -gt 0 ]; do
  case "$1" in
    --show-sdk-path) echo "/"; exit 0 ;;
    --sdk|-sdk) shift ;;
    --find|-find)
        shift
        [ -x "$TOOLCHAIN/$1" ] && { echo "$TOOLCHAIN/$1"; exit 0; }
        command -v "$1" 2>/dev/null && exit 0
        echo "/usr/bin/$1"; exit 0 ;;
    -verbose|-log) ;;
    *) ;;
  esac
  shift
done
exit 0
