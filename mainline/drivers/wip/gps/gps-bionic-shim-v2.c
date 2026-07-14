/* gps-bionic-shim-v2.c — LD_PRELOAD para correr mnld/libmnl stock (bionic) en pmOS (Alpine/musl).
 *
 * Historia:
 *  v2 (0714 casa): chipid `persist.mtk.wcn.combo.chipid` (lo lee chip_detector) vía dlsym de
 *    __system_property_add/_area_init (evita "symbol not found" @@LIBC al relocar -nostdlib).
 *  v3 (0714 casa, Ghidra sobre mnld): RESUELVE el gate final. RE del binario mnld (72KB, Thumb-2):
 *    mtk_gps_sys_init (FUN_000140fc) hace un bucle 20x/500ms: property_get("service.nvram_init")
 *    y sólo rompe si == "Ready"; si no, devuelve -1 y el motor NUNCA abre /dev/stpgps
 *    (FUN_00014ef4 abre el DSP sólo tras ese éxito). En Android lo pone el nvram_daemon; en pmOS
 *    no existe. Con la prop = "Ready" el gate pasa a la 1a iteración → mnld ABRE /dev/stpgps y
 *    corre el motor (mtk_gps_sys_nmea_output_to_app dispara). Ver ghidra-gps/decomp.txt + refs.txt.
 *  v3 robustez (0714): la inyección va por LAZY-INIT en la 1a llamada real a __xlog_buf_printf (el
 *    constructor -nostdlib no corría fiable / corría antes de environ). CAS atómico (1 hilo, mnld
 *    es multihilo → evita doble area_init → SIGSEGV). unlink("/dev/__properties__") antes de
 *    area_init: bionic lo mapea READ-ONLY para lectores; un backing stale de una run previa hace
 *    que el area_init de ESTA run lo mapee r/o → prop_add falla con SIGSEGV. Auto-reparable.
 *
 * Compilar: arm-linux-gnueabihf-gcc -shared -nostdlib -fPIC -O1 -o libxlogshim.so gps-bionic-shim-v2.c -ldl
 * Uso: env GPS_SET_CHIPID=1 LD_LIBRARY_PATH=/system/lib LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld
 */
#include <stdarg.h>

extern long write(int, const void *, unsigned long);
extern int  vsnprintf(char *, unsigned long, const char *, va_list);
extern char *getenv(const char *);
extern void *dlopen(const char *, int);
extern void *dlsym(void *, const char *);
extern int   unlink(const char *);
#define RTLD_NOW 2

typedef int (*ai_t)(void);
typedef int (*pa_t)(const char *, unsigned, const char *, unsigned);
typedef int (*pg_t)(const char *, char *);

static void wr(const char *s) { int n = 0; while (s[n]) n++; if (n) write(2, s, n); }

/* Poblar el ÁREA de properties de bionic. Lazy + atómico: se ejecuta 1 sola vez, en el proceso mnld. */
static volatile int g_props_done = 0;
static int g_diag = 0;
static void ensure_props(void)
{
    if (g_props_done) return;
    char *e = getenv("GPS_SET_CHIPID");
    if (!g_diag) { g_diag = 1; wr("[shim] GPS_SET_CHIPID=["); wr(e ? e : "(null)"); wr("]\n"); }
    if (!(e && e[0] == '1')) return;                              /* env aún no listo: reintenta */
    if (!__sync_bool_compare_and_swap(&g_props_done, 0, 1)) return; /* otro hilo ganó la carrera */
    void *h = dlopen("libc.so", RTLD_NOW);
    if (!h) { wr("[shim] dlopen libc.so FAILED\n"); return; }
    ai_t area_init = (ai_t)dlsym(h, "__system_property_area_init");
    pa_t prop_add  = (pa_t)dlsym(h, "__system_property_add");
    pg_t prop_get  = (pg_t)dlsym(h, "__system_property_get");
    if (!prop_add) { wr("[shim] no __system_property_add\n"); return; }
    unlink("/dev/__properties__");   /* backing fresco/escribible (evita SIGSEGV en re-runs) */
    if (area_init) area_init();
    prop_add("persist.mtk.wcn.combo.chipid", 28, "0x6582", 6);   /* chip_detector */
    prop_add("persist.radio.mediatek.chipid", 29, "0x6582", 6);
    prop_add("service.nvram_init", 18, "Ready", 5);              /* gate de mtk_gps_sys_init */
    if (prop_get) {
        char v[92];
        const char *keys[3] = {"persist.mtk.wcn.combo.chipid", "service.nvram_init", "persist.radio.mediatek.chipid"};
        int i;
        for (i = 0; i < 3; i++) { v[0] = 0; prop_get(keys[i], v);
            wr("[shim] "); wr(keys[i]); wr("=\""); wr(v); wr("\"\n"); }
    }
}

static int xlog_quiet(void) { char *q = getenv("GPS_XLOG_QUIET"); return q && q[0] == '1'; }

/* Firma bionic/MTK usada por mnld+libmnl (deja correr el motor; el log es sólo debug). */
int __xlog_buf_printf(int prio, const char *tag, const char *fmt, ...)
{
    ensure_props();
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
    ensure_props();
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
