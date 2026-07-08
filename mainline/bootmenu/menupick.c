/* menupick.c — selector de SO visual para el krillin (MT6582).
 * Dibuja un menú en el framebuffer (DRM ya cargado por el kernel) y lee
 * los botones de volumen (Vol+/- para navegar) y power (para seleccionar).
 * Devuelve por stdout el indice del SO elegido (0, 1, 2...) para que el
 * init script haga switch_root o kexec.
 *
 * HW confirmado:
 *   - /dev/fb0: 540x960, 32bpp (BGRA) — el DRM mediatek-drm expone fb0
 *   - /dev/input/event0: mt6779-keypad (Vol+ = KEY_VOLUMEUP=115, Vol- = KEY_VOLUMEDOWN=114)
 *   - /dev/input/event1: mtk-pmic-keys (Power = KEY_POWER=116)
 *
 * Compilar estático: musl-gcc -static -O2 -o menupick menupick.c
 * (o gcc -static en la Pi que es armhf nativo)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <time.h>

#define MAX_ENTRIES 8

/* colores BGRA (32bpp) */
#define COLOR_BG    0xFF1A1A2E   /* azul oscuro */
#define COLOR_SEL   0xFFE94560   /* rojo/rosa */
#define COLOR_TEXT  0xFFFFFFFF    /* blanco */
#define COLOR_SUB   0xFFA0A0B0   /* gris */
#define COLOR_BAR   0xFF16213E   /* barra inferior */

static int fb_fd, fb_w, fb_h, fb_bpp, fb_stride;
static unsigned char *fb_mem;
static int fb_mem_size;

/* fuentes bitmap simples (5x7) — solo mayusculas, digitos, signos basicos */
#include "font5x7.h"

static void fb_init(void)
{
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;

	fb_fd = open("/dev/fb0", O_RDWR);
	if (fb_fd < 0) { perror("fb0"); exit(1); }
	ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
	ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
	fb_w = vinfo.xres;
	fb_h = vinfo.yres;
	fb_bpp = vinfo.bits_per_pixel;
	fb_stride = finfo.line_length;
	fb_mem_size = fb_stride * fb_h;
	fb_mem = mmap(NULL, fb_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if (fb_mem == MAP_FAILED) { perror("mmap fb"); exit(1); }
}

static inline void put_pixel(int x, int y, unsigned int color)
{
	if (x < 0 || x >= fb_w || y < 0 || y >= fb_h) return;
	unsigned int *p = (unsigned int *)(fb_mem + y * fb_stride + x * (fb_bpp / 8));
	*p = color;
}

static void fill_rect(int x, int y, int w, int h, unsigned int color)
{
	int i, j;
	for (j = y; j < y + h; j++)
		for (i = x; i < x + w; i++)
			put_pixel(i, j, color);
}

static void clear_screen(unsigned int color)
{
	fill_rect(0, 0, fb_w, fb_h, color);
}

/* dibuja un caracter 5x7 en (x,y), scale = multiplicador */
static void draw_char(int x, int y, char c, unsigned int color, int scale)
{
	const unsigned char *glyph;
	int col, row;

	if (c < 32 || c > 127) c = '?';
	glyph = &font5x7_data[(c - 32) * 5];

	for (col = 0; col < 5; col++) {
		unsigned char bits = glyph[col];
		for (row = 0; row < 7; row++) {
			if (bits & (1 << row))
				fill_rect(x + col * scale, y + row * scale, scale, scale, color);
		}
	}
}

static void draw_text(int x, int y, const char *s, unsigned int color, int scale)
{
	int i;
	for (i = 0; s[i]; i++)
		draw_char(x + i * 6 * scale, y, s[i], color, scale);
}

static int text_width(const char *s, int scale)
{
	return strlen(s) * 6 * scale - 1 * scale;
}

static void draw_centered(int y, const char *s, unsigned int color, int scale)
{
	int w = text_width(s, scale);
	int x = (fb_w - w) / 2;
	if (x < 10) x = 10;
	draw_text(x, y, s, color, scale);
}

/* Lee un input event (bloqueante). Devuelve el keycode o -1. */
static int read_key(int fd)
{
	struct input_event ev;
	int n;
	while (1) {
		n = read(fd, &ev, sizeof(ev));
		if (n != sizeof(ev)) return -1;
		if (ev.type == EV_KEY && ev.value == 1) /* press */
			return ev.code;
	}
}

int main(int argc, char **argv)
{
	char *entries[MAX_ENTRIES];
	char *subtitles[MAX_ENTRIES];
	int n_entries = 0;
	int sel = 0;
	int i;
	int fd_keypad, fd_power;
	int timeout_sec = 10; /* auto-boot tras 10s */
	time_t last_input;

	/* Parsear argumentos: menupick "titulo" "entry1" "entry2" ... */
	if (argc < 3) {
		fprintf(stderr, "uso: menupick \"titulo\" \"entry1\" \"entry2\" ...\n");
		return 0;
	}
	const char *title = argv[1];
	for (i = 2; i < argc && n_entries < MAX_ENTRIES; i++) {
		entries[n_entries] = argv[i];
		subtitles[n_entries] = NULL;
		n_entries++;
	}

	fb_init();
	clear_screen(COLOR_BG);

	/* Abrir input devices */
	fd_keypad = open("/dev/input/event0", O_RDONLY);
	fd_power = open("/dev/input/event1", O_RDONLY);
	if (fd_keypad < 0 || fd_power < 0) {
		/* sin botones: auto-boot la primera entrada */
		fprintf(stderr, "no input devices -> auto-boot 0\n");
		printf("0\n");
		return 0;
	}

	/* Hacer los fds non-blocking para el timeout */
	fcntl(fd_keypad, F_SETFL, O_NONBLOCK);
	fcntl(fd_power, F_SETFL, O_NONBLOCK);

	last_input = time(NULL);

redraw:
	/* Dibujar el menú */
	clear_screen(COLOR_BG);

	/* Titulo */
	draw_centered(60, title, COLOR_TEXT, 4);

	/* Linea separadora */
	fill_rect(30, 130, fb_w - 60, 2, COLOR_SUB);

	/* Entradas */
	int entry_y = 170;
	int entry_h = 80;
	for (i = 0; i < n_entries; i++) {
		int y = entry_y + i * entry_h;
		if (i == sel) {
			/* resaltar entrada seleccionada */
			fill_rect(20, y - 5, fb_w - 40, entry_h - 10, COLOR_SEL);
			draw_centered(y + 15, entries[i], COLOR_TEXT, 3);
		} else {
			draw_centered(y + 15, entries[i], COLOR_SUB, 3);
		}
	}

	/* Barra inferior con instrucciones */
	fill_rect(0, fb_h - 50, fb_w, 50, COLOR_BAR);
	draw_centered(fb_h - 38, "Vol+/- navegar   Power seleccionar", COLOR_SUB, 2);

	/* Auto-boot countdown */
	if (timeout_sec > 0) {
		int remaining = timeout_sec - (int)(time(NULL) - last_input);
		if (remaining < 0) remaining = 0;
		char buf[32];
		snprintf(buf, sizeof(buf), "Auto-boot en %ds", remaining);
		draw_text(fb_w - text_width(buf, 2) - 20, 10, buf, COLOR_SUB, 2);
	}

	/* Esperar input (con timeout) */
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 200000000 }; /* 200ms */
	while (1) {
		/* Check timeout */
		if (timeout_sec > 0 && (time(NULL) - last_input) >= timeout_sec) {
			printf("%d\n", sel);
			return 0;
		}

		/* Leer Vol+/- (keypad) */
		int key = read_key(fd_keypad);
		if (key == 115) { /* Vol+ */
			sel = (sel - 1 + n_entries) % n_entries;
			last_input = time(NULL);
			goto redraw;
		}
		if (key == 114) { /* Vol- */
			sel = (sel + 1) % n_entries;
			last_input = time(NULL);
			goto redraw;
		}

		/* Leer Power */
		key = read_key(fd_power);
		if (key == 116) { /* Power = seleccionar */
			printf("%d\n", sel);
			/* Limpiar pantalla antes de switch_root */
			clear_screen(COLOR_BG);
			munmap(fb_mem, fb_mem_size);
			close(fb_fd);
			close(fd_keypad);
			close(fd_power);
			return 0;
		}

		nanosleep(&ts, NULL);

		/* Redibujar countdown cada segundo */
		static time_t last_draw = 0;
		if (time(NULL) != last_draw) {
			last_draw = time(NULL);
			goto redraw;
		}
	}

	return 0;
}
