/*
 * plat-mt6582/fb.c — consola de framebuffer para el BQ Aquaris E4.5 (krillin).
 * El LK del MTK deja el panel hx8389 (qHD 540x960, DSI modo video, se auto-refresca)
 * con su framebuffer en 0xBF400000, stride 2176 = 544*4 (32bpp XRGB8888). GenericBooter
 * corre bare-metal -> escritura fisica directa = pixeles en pantalla, sin tocar el DSI.
 */
#include "genboot.h"

#define FONT_WIDTH  8
#define FB_BASE     0xBF400000
#define STRIDE_PX   544          /* stride 2176 bytes / 4 */
#define WIDTH       540          /* visible */
#define HEIGHT      960
#define BG_COLOR    0x00000000
#define FG_COLOR    0x00FFFFFF   /* blanco */

#include "consolefont.c"

int mt6582_console_row = 0;
int mt6582_console_col = 0;

static inline void fb_setPixel(int x, int y, unsigned int color)
{
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
        return;
    volatile unsigned int *fb = (volatile unsigned int *)FB_BASE;
    fb[y * STRIDE_PX + x] = color;
}

static void fb_drawRect(int bx, int by, int w, int h, unsigned int color)
{
    for (int y = by; y < by + h; y++)
        for (int x = bx; x < bx + w; x++)
            fb_setPixel(x, y, color);
}

static int fb_drawChar(char c, int row, int col)
{
    int index = ((int)c) * FONT_WIDTH;
    for (int rr = 0; rr < FONT_WIDTH; rr++) {
        int myrow = console_font[index + rr];
        for (int cc = 0; cc < FONT_WIDTH; cc++) {
            if ((myrow >> (FONT_WIDTH - 1 - cc)) & 0x1)
                fb_setPixel(col + cc, row + rr, FG_COLOR);
        }
    }
    return console_font_widths[(int)c] + 1;
}

void mt6582_console_init(void)
{
    mt6582_console_col = 0;
    mt6582_console_row = 0;
    fb_drawRect(0, 0, WIDTH, HEIGHT, BG_COLOR);
}

void mt6582_console_putchar(char c)
{
    if (c == 0x0A) {
        mt6582_console_row += (FONT_WIDTH + 1);
        mt6582_console_col = 0;
        return;
    }
    mt6582_console_col += fb_drawChar(c, mt6582_console_row, mt6582_console_col);
    if (mt6582_console_col + FONT_WIDTH > WIDTH) {
        mt6582_console_row += (FONT_WIDTH + 1);
        mt6582_console_col = 0;
    }
    if (mt6582_console_row + FONT_WIDTH > HEIGHT)   /* wrap arriba (simple) */
        mt6582_console_row = 0;
}
