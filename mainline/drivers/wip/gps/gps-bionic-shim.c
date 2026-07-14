/* gps-bionic-shim.c — LD_PRELOAD para correr el mnld/libmnlp stock (bionic) en pmOS mainline.
 *
 * Provee los xlog de MTK (__xlog_buf_printf/_vprintf) que mnld/libmnl referencian y que la
 * liblog de LineageOS no exporta (evita "cannot locate symbol").
 *
 * Por defecto los xlog se REENVÍAN a stderr (fd 2) para poder DEPURAR la narración del
 * motor (por qué no sale NMEA, estado del fix, medidas del DSP...). Si molesta el ruido,
 * exportar GPS_XLOG_QUIET=1 → no-op. (Se cargan en un proceso BIONIC, así que write/vsnprintf
 * se resuelven contra la libc de bionic en runtime aunque el shim sea -nostdlib.)
 *
 * (El interpositor de open() para /sys/class/gpsdrv se RETIRÓ: ahora el driver mainline
 *  `mt6582-gpsdrv.c` crea el `/sys/class/gpsdrv/gps/` + `/dev/gps` reales.)
 *
 * Compilar: arm-linux-gnueabihf-gcc -shared -nostdlib -fPIC -O1 -o libxlogshim.so gps-bionic-shim.c
 * Uso: env LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld
 * (requiere el driver mt6582-gpsdrv cargado + mkdir -p /data/gps_mnl)
 */

#include <stdarg.h>

/* resueltos en runtime contra la libc de bionic del proceso anfitrión */
extern long write(int, const void *, unsigned long);
extern int  vsnprintf(char *, unsigned long, const char *, va_list);
extern char *getenv(const char *);

static int xlog_quiet(void) { char *q = getenv("GPS_XLOG_QUIET"); return q && q[0] == '1'; }

int __xlog_buf_printf(int prio, const char *tag, const char *fmt, ...)
{
    if (xlog_quiet() || !fmt) return 0;
    char buf[600];
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf - 2, fmt, ap);
    va_end(ap);
    if (n < 0) return 0;
    if (n > (int)sizeof buf - 2) n = sizeof buf - 2;
    if (n == 0 || buf[n - 1] != '\n') buf[n++] = '\n';
    write(2, buf, n);
    return 0;
}
int __xlog_buf_vprintf(int prio, const char *tag, const char *fmt, va_list ap)
{
    if (xlog_quiet() || !fmt) return 0;
    char buf[600];
    int n = vsnprintf(buf, sizeof buf - 2, fmt, ap);
    if (n < 0) return 0;
    if (n > (int)sizeof buf - 2) n = sizeof buf - 2;
    if (n == 0 || buf[n - 1] != '\n') buf[n++] = '\n';
    write(2, buf, n);
    return 0;
}

/* Callback AGPS que libmnl.so referencia y que en el stock provee mnld/agpsd.
 * AGPS es OPCIONAL (no bloquea el fix básico) -> no-op para poder correr el
 * libmnlp stock (bionic) con la libmnl.so stock (ABI correcto). */
long mtk_gps_sys_agps_disaptcher_callback(long a, long b, long c, long d) { return 0; }
