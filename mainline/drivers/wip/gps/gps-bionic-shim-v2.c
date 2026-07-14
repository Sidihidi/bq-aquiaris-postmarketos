/* gps-bionic-shim-v2.c — LD_PRELOAD para correr mnld/libmnlp stock (bionic) en pmOS.
 * v2 (0714 casa): añade el chipid CORRECTO `persist.mtk.wcn.combo.chipid` (lo lee chip_detector)
 * y usa dlsym en runtime para __system_property_add/_area_init (evita el "symbol not found" por
 * versión @@LIBC al relocar el shim -nostdlib).
 * Compilar: arm-linux-gnueabihf-gcc -shared -nostdlib -fPIC -O1 -o libxlogshim.so gps-bionic-shim-v2.c -ldl
 * Uso: env GPS_SET_CHIPID=1 LD_LIBRARY_PATH=/system/lib LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld
 */
#include <stdarg.h>

extern long write(int, const void *, unsigned long);
extern int  vsnprintf(char *, unsigned long, const char *, va_list);
extern char *getenv(const char *);
extern void *dlopen(const char *, int);
extern void *dlsym(void *, const char *);
#define RTLD_NOW 2

static int xlog_quiet(void) { char *q = getenv("GPS_XLOG_QUIET"); return q && q[0] == '1'; }

int __xlog_buf_printf(int prio, const char *tag, const char *fmt, ...)
{
    if (xlog_quiet() || !fmt) return 0;
    char buf[600]; va_list ap; va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf - 2, fmt, ap); va_end(ap);
    if (n < 0) return 0;
    if (n > (int)sizeof buf - 2) n = sizeof buf - 2;
    if (n == 0 || buf[n - 1] != '\n') buf[n++] = '\n';
    write(2, buf, n); return 0;
}
int __xlog_buf_vprintf(int prio, const char *tag, const char *fmt, va_list ap)
{
    if (xlog_quiet() || !fmt) return 0;
    char buf[600];
    int n = vsnprintf(buf, sizeof buf - 2, fmt, ap);
    if (n < 0) return 0;
    if (n > (int)sizeof buf - 2) n = sizeof buf - 2;
    if (n == 0 || buf[n - 1] != '\n') buf[n++] = '\n';
    write(2, buf, n); return 0;
}

/* AGPS callback (opcional) -> no-op para linkar con la libmnl.so stock (ABI correcto). */
long mtk_gps_sys_agps_disaptcher_callback(long a, long b, long c, long d) { return 0; }

/* Poblar el ÁREA de properties de bionic con el chipid que chip_detector necesita.
 * chip_detector lee `persist.mtk.wcn.combo.chipid` (visto en strings del mnld). */
typedef int (*ai_t)(void);
typedef int (*pa_t)(const char *, unsigned, const char *, unsigned);

__attribute__((constructor))
static void gps_shim_setup_props(void)
{
    char *e = getenv("GPS_SET_CHIPID");
    if (!(e && e[0] == '1')) return;
    void *h = dlopen("libc.so", RTLD_NOW);
    if (!h) return;
    ai_t area_init = (ai_t)dlsym(h, "__system_property_area_init");
    pa_t prop_add  = (pa_t)dlsym(h, "__system_property_add");
    if (!prop_add) return;
    if (area_init) area_init();
    /* combo.chipid (28) + radio.chipid (29) + mnl.prop, valor "0x6582" (6). */
    prop_add("persist.mtk.wcn.combo.chipid", 28, "0x6582", 6);
    prop_add("persist.radio.mediatek.chipid", 29, "0x6582", 6);
}
