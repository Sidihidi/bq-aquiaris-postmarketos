export XDG_RUNTIME_DIR=/run/user/1000
export WLR_RENDERER=pixman LIBSEAT_BACKEND=seatd
export XDG_DATA_DIRS=/usr/local/share:/usr/share
export PATH=/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin
exec dbus-run-session phoc -E "/usr/local/libexec/phosh"
