/* gps-bionic-shim.c — LD_PRELOAD para correr el mnld stock (bionic) en pmOS mainline.
 *
 * Dos funciones:
 *  1) no-op de los xlog de MTK (__xlog_buf_printf/_vprintf) que mnld referencia y que
 *     la liblog de LineageOS no exporta (evita "cannot locate symbol").
 *  2) interpositor de open()/openat(): mnld abre /sys/class/gpsdrv/gps/status (interfaz
 *     sysfs del driver GPS DOWNSTREAM de MTK, ausente en mainline — aquí el power va por
 *     el open de /dev/stpgps). Lo redirige a un fichero real /data/gps_mnl/fakestatus para
 *     que mnld no muera; el resto de rutas pasan tal cual por la syscall openat.
 *
 * Compilar (glibc cross-tc vale, es -nostdlib puro syscall, sin deps de libc):
 *   arm-linux-gnueabihf-gcc -shared -nostdlib -fPIC -O1 -o libxlogshim.so gps-bionic-shim.c
 * Uso: env LD_PRELOAD=/system/lib/libxlogshim.so /system/xbin/mnld
 * (requiere: mkdir -p /data/gps_mnl && printf 1 > /data/gps_mnl/fakestatus)
 */
#include <stdarg.h>

/* openat(2) por syscall directa ARM EABI (__NR_openat=322, AT_FDCWD=-100). */
static long sys_openat(int d, const char *p, int f, int m)
{
	register long r0 __asm__("r0") = d;
	register long r1 __asm__("r1") = (long)p;
	register long r2 __asm__("r2") = f;
	register long r3 __asm__("r3") = m;
	register long r7 __asm__("r7") = 322;
	__asm__ volatile("svc #0" : "+r"(r0)
			 : "r"(r1), "r"(r2), "r"(r3), "r"(r7) : "memory");
	return r0;
}

static int pfx(const char *s, const char *p)
{
	while (*p) { if (*s++ != *p++) return 0; }
	return 1;
}

int openat(int d, const char *path, int flags, ...)
{
	va_list ap; va_start(ap, flags); int m = va_arg(ap, int); va_end(ap);
	if (path && pfx(path, "/sys/class/gpsdrv"))
		return (int)sys_openat(-100, "/data/gps_mnl/fakestatus", 2 /*O_RDWR*/, 0);
	return (int)sys_openat(d, path, flags, m);
}

int open(const char *path, int flags, ...)
{
	va_list ap; va_start(ap, flags); int m = va_arg(ap, int); va_end(ap);
	if (path && pfx(path, "/sys/class/gpsdrv"))
		return (int)sys_openat(-100, "/data/gps_mnl/fakestatus", 2, 0);
	return (int)sys_openat(-100, path, flags, m);
}

int __xlog_buf_printf(int a, const char *b, const char *c, ...) { return 0; }
int __xlog_buf_vprintf(int a, const char *b, const char *c, void *d) { return 0; }
