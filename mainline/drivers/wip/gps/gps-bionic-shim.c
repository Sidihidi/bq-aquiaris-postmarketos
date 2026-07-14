/* gps-bionic-shim.c — LD_PRELOAD para correr el mnld stock (bionic) en pmOS mainline.
 *
 * No-op de los xlog de MTK (__xlog_buf_printf/_vprintf) que mnld referencia y que la
 * liblog de LineageOS no exporta (evita "cannot locate symbol").
 *
 * (El interpositor de open() para /sys/class/gpsdrv se RETIRÓ: ahora el driver mainline
 *  `mt6582-gpsdrv.c` crea el `/sys/class/gpsdrv/gps/` + `/dev/gps` reales.)
 *
 * Compilar: arm-linux-gnueabihf-gcc -shared -nostdlib -fPIC -O1 -o libxlogshim.so gps-bionic-shim.c
 * Uso: env LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld
 * (requiere el driver mt6582-gpsdrv cargado + mkdir -p /data/gps_mnl)
 */

int __xlog_buf_printf(int a, const char *b, const char *c, ...) { return 0; }
int __xlog_buf_vprintf(int a, const char *b, const char *c, void *d) { return 0; }

/* Callback AGPS que libmnl.so referencia y que en el stock provee mnld/agpsd.
 * AGPS es OPCIONAL (no bloquea el fix básico) -> no-op para poder correr el
 * libmnlp stock (bionic) con la libmnl.so stock (ABI correcto). */
long mtk_gps_sys_agps_disaptcher_callback(long a, long b, long c, long d) { return 0; }
