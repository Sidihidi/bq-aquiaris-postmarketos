#!/system/bin/sh
/system/bin/fsresp 4000000 > /data/nvram/gd.out 2>/data/nvram/gd.err &
exec /system/bin/ccci_fsd.real "$@"
