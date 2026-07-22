/*
 * Copyright 2013, winocm. <winocm@icloud.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 *   If you are going to use this software in any form that does not involve
 *   releasing the source to this project or improving it, let me know beforehand.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * PE bringup (semihosting)
 */

#include <stdarg.h>
#include <kern/debug.h>
#include <mach/mach_types.h>
#include <pexpert/pexpert.h>
#include <pexpert/machine/protos.h>
#include <pexpert/machine/boot.h>
#include <machine/machine_routines.h>
#include "semihost.h"

/*
 * M6 (krillin/MT6582): consola de framebuffer para XNU.
 *
 * PE_early_puts original usa SEMIHOSTING (escribe a un debugger JTAG por SVC/BKPT),
 * que en el movil sin depurador NO imprime nada. Lo re-cableamos a una consola de
 * framebuffer directa, portada 1:1 del GenericBooter (plat-mt6582/fb.c), YA PROBADA
 * en este hardware (sus mensajes de arranque salian en pantalla en M3).
 *
 * El LK deja el panel hx8389 (qHD 540x960, DSI video, auto-refresco) con su fb en el
 * FISICO 0xBF400000, stride 2176 = 544*4 (32bpp XRGB8888). En modo virtual de XNU ese
 * fisico esta IDENTITY-MAPPED por el fix de M4, asi que la escritura directa funciona
 * desde arm_init. Texto BLANCO sobre NEGRO -> legible con cualquier orden de canal.
 */
#define FB_FONT_WIDTH  8
#define FB_BASE        0xBF400000
/*
 * El framebuffer del krillin es RGB565 (16 BITS por pixel), stride 1088 bytes
 * = 544 pixeles.  Antes lo tratabamos como 32bpp con stride 2176: cada palabra
 * de 32 bits pintaba DOS pixeles reales y cada fila nuestra ocupaba DOS filas
 * reales, asi que todo salia al doble de tamaño y cada linea de mas de ~34
 * caracteres derramaba su mitad derecha sobre la fila siguiente.  Ese era el
 * "solape" que arrastrabamos desde el principio.
 */
#define FB_STRIDE_PX   544          /* pixeles de 16 bits (1088 bytes) */
/*
 * COORDENADAS LOGICAS, no reales.
 *
 * El panel es RGB565 (16 bits) pero escribimos palabras de 32, que es la
 * configuracion con la que el arranque llega lejos; escribiendo de 16 en 16 el
 * kernel muere antes de A3 por razones aun sin identificar.  Con escrituras de
 * 32 bits, cada pixel logico nuestro cubre DOS pixeles reales en horizontal y
 * cada fila logica DOS filas reales.  Declarando 270x480 logicos cubrimos
 * exactamente los 540x960 reales del panel: nada se sale por abajo (antes se
 * perdia la mitad de la traza) y ninguna linea invade la fila siguiente.
 *
 * El texto sale al doble: 33 columnas x 28 lineas.  Los marcadores estan
 * pensados para caber en ese ancho.
 */
#define FB_WIDTH       270          /* 540 reales / 2 */
#define FB_HEIGHT      480          /* 960 reales / 2 */
#define FB_BG_COLOR    0x00000000
#define FB_FG_COLOR    0x00FFFFFF
#define FB_FONT_H      16           /* iso_font es 8x16 */
#define FB_SCALE       fbcon_scale  /* escala en runtime */
#define FB_LINE_H      ((FB_FONT_H + 1) * FB_SCALE)       /* alto de linea (1px de aire) */
#define FB_CH          (FB_FONT_H * FB_SCALE)             /* alto del glifo escalado */
#define FB_CW          (FB_FONT_WIDTH * FB_SCALE)         /* ancho del glifo escalado */
#define FB_ADV         (FB_FONT_WIDTH * FB_SCALE)         /* la ISO 8x16 ya trae su propio aire lateral */
#define FB_TAB_PX      (FB_ADV * 4)                       /* parada de tabulador, en pixeles */

/*
 * Fuente ISO 8x16 de XNU (osfmk/console/iso_font.c), la misma que usa la
 * consola vc_.  Sustituye a la 8x8 de antes: al doble de alto los glifos
 * tienen DETALLE real, mientras que ampliar una 8x8 por 3 solo produce
 * bloques de 3x3 escalonados — mas grande pero no mas legible.
 *
 * OJO: los bits van LSB PRIMERO (bit 0 = pixel izquierdo), al reves que la
 * 8x8.  Lo fija video_console.c: 'unsigned char mask = 1' y mask <<= 1.
 */
extern unsigned char iso_font[];

/*
 * Escala 1 = 60 columnas x 53 lineas en el panel de 540x960, con glifos de
 * 16px de alto: mas pequeño que la 8x8 a escala 3 y bastante mas legible.
 */
static int fbcon_scale = 1;
static int fbcon_quiet = 1;
static int fbcon_inited = 0;

static inline void fbcon_setPixel(int x, int y, unsigned int color)
{
    volatile unsigned int *fb = (volatile unsigned int *)FB_BASE;
    if (x < 0 || y < 0 || x >= FB_WIDTH || y >= FB_HEIGHT)
        return;
    fb[y * FB_STRIDE_PX + x] = color;
}

static void fbcon_drawRect(int bx, int by, int w, int h, unsigned int color)
{
    int x, y;
    for (y = by; y < by + h; y++)
        for (x = bx; x < bx + w; x++)
            fbcon_setPixel(x, y, color);
}

static int fbcon_drawChar(char c, int row, int col)
{
    int index = ((int)(unsigned char) c) * FB_FONT_H;
    int rr, cc, sx, sy;
    for (rr = 0; rr < FB_FONT_H; rr++) {
        int myrow = iso_font[index + rr];
        for (cc = 0; cc < FB_FONT_WIDTH; cc++) {
            if ((myrow >> cc) & 0x1)
                for (sy = 0; sy < FB_SCALE; sy++)
                    for (sx = 0; sx < FB_SCALE; sx++)
                        fbcon_setPixel(col + cc * FB_SCALE + sx,
                                       row + rr * FB_SCALE + sy, FB_FG_COLOR);
        }
    }
    /* La ISO es monoespaciada: avance fijo, 1px de aire entre caracteres. */
    return FB_ADV;
}

/*
 * La consola se apoya en un BUFFER DE TEXTO en DRAM, no en los pixeles.
 *
 * La version anterior hacia el scroll moviendo ~2MB de pixeles dentro del
 * framebuffer, que esta mapeado como memoria device.  En hardware eso NO
 * desplazaba nada: al llegar abajo el cursor volvia arriba y el texto nuevo se
 * pintaba sobre el viejo (el "solape" que arrastrabamos).  Guardando el texto
 * en RAM, el scroll es un memmove de ~6KB en memoria normal y la pantalla se
 * REDIBUJA entera desde el buffer.  Deja de depender de como se comporte el
 * framebuffer y es reproducible fuera del movil.
 */
#define FB_MAXROWS 64
#define FB_MAXCOLS 96

static char fbcon_buf[FB_MAXROWS][FB_MAXCOLS];
static int fbcon_rows, fbcon_cols;   /* tamaño util en caracteres */
static int fbcon_r, fbcon_c;         /* cursor, en CARACTERES */
static int fbcon_heal;               /* cuenta para el repintado periodico */

/* Borra el buffer ENTERO, incluidas las columnas fuera de pantalla que
 * fbcon_drawRect() no toca (stride 544 px frente a 540 visibles). */
static void fbcon_clear_all(void)
{
    volatile unsigned int *fb = (volatile unsigned int *)FB_BASE;
    int i, total = 960 * FB_STRIDE_PX;   /* area COMPLETA, igual que M16 */

    for (i = 0; i < total; i++)
        fb[i] = FB_BG_COLOR;
}

static void fbcon_redraw(void)
{
    int r, c, x;

    fbcon_clear_all();
    for (r = 0; r < fbcon_rows; r++) {
        x = 0;
        for (c = 0; c < fbcon_cols && fbcon_buf[r][c]; c++)
            x += fbcon_drawChar(fbcon_buf[r][c], r * FB_LINE_H, x);
    }
}

#define XNU_KRILLIN_BUILD "M32-iokit"

static void fbcon_init(void)
{
    int r, c;

    fbcon_rows = FB_HEIGHT / FB_LINE_H;
    fbcon_cols = FB_WIDTH / FB_ADV;
    if (fbcon_rows > FB_MAXROWS)
        fbcon_rows = FB_MAXROWS;
    if (fbcon_cols > FB_MAXCOLS)
        fbcon_cols = FB_MAXCOLS;

    for (r = 0; r < FB_MAXROWS; r++)
        for (c = 0; c < FB_MAXCOLS; c++)
            fbcon_buf[r][c] = 0;

    fbcon_r = fbcon_c = 0;
    fbcon_clear_all();

    /* Barra verde de 4px: prueba visual de que XNU LLEGO hasta aqui.  Si la
     * pantalla sale de un color plano sin esta barra, el arranque murio antes
     * de la consola y lo que se ve lo dejo el booter. */
    {
        int x, y;
        for (y = 0; y < 4; y++)
            for (x = 0; x < FB_WIDTH; x++)
                fbcon_setPixel(x, y, 0x07E0);
    }
    fbcon_inited = 1;            /* antes del banner: corta la recursion */
    PE_early_puts("=== XNU krillin build " XNU_KRILLIN_BUILD " ===\n");
}

static void fbcon_newline(void)
{
    int r, c, keep;

    fbcon_c = 0;
    fbcon_r++;

    /*
     * Auto-reparacion.  Hay algo mas en XNU que repinta el framebuffer entero
     * y sepulta la traza (el dialogo de panico era uno, pero no el unico).  En
     * vez de perseguir cada pintor, repintamos NOSOTROS periodicamente desde
     * el buffer de texto: quien pinte encima queda tapado en el siguiente
     * ciclo y lo que acaba viendose es siempre nuestra consola.
     */
    if (fbcon_r < fbcon_rows)
        return;

    /* pantalla llena: se descarta la mitad de arriba y se redibuja */
    keep = fbcon_rows / 2;
    for (r = 0; r + keep < fbcon_rows; r++)
        for (c = 0; c < fbcon_cols; c++)
            fbcon_buf[r][c] = fbcon_buf[r + keep][c];
    for (; r < fbcon_rows; r++)
        for (c = 0; c < fbcon_cols; c++)
            fbcon_buf[r][c] = 0;

    fbcon_r = fbcon_rows - keep;
    fbcon_redraw();
}

static void fbcon_putchar(char c)
{
    unsigned char u = (unsigned char) c;

    if (!fbcon_inited)
        fbcon_init();

    if (u == 0x0A) {                 /* LF */
        fbcon_newline();
        return;
    }
    if (u == 0x0D) {                 /* CR */
        fbcon_c = 0;
        return;
    }
    if (u == 0x09) {                 /* TAB: espacios hasta la parada de 4 */
        int stop = ((fbcon_c / 4) + 1) * 4;
        while (fbcon_c < stop && fbcon_c < fbcon_cols) {
            fbcon_buf[fbcon_r][fbcon_c] = ' ';
            fbcon_c++;
        }
        return;
    }
    if (u < 0x20 || u > 0x7E)        /* no imprimible */
        return;

    if (fbcon_c >= fbcon_cols)
        fbcon_newline();

    fbcon_buf[fbcon_r][fbcon_c] = c;
    fbcon_drawChar(c, fbcon_r * FB_LINE_H, fbcon_c * FB_ADV);
    fbcon_c++;
}

/**
 * PE_fb_calibrate
 *
 * Patron de calibracion.  NO asume formato: escribe por OFFSET DE BYTE crudo y
 * deja que la pantalla nos diga como se interpretan esos bytes.
 *
 * Cuatro bandas consecutivas de 100 filas de 2176 bytes:
 *   1. u32 0x00FF0000  -> ROJA  si el framebuffer es XRGB8888 de 32 bits
 *   2. u32 0x000000FF  -> AZUL  si es XRGB8888
 *   3. u16 0xF800      -> ROJA  si el framebuffer es RGB565 de 16 bits
 *   4. u16 0x001F      -> AZUL  si es RGB565
 *
 * Cual de las bandas sale roja identifica la PROFUNDIDAD.  Y la altura total
 * de las cuatro identifica el STRIDE: ~40%% de la pantalla si son 2176 bytes
 * por fila, ~83%% si son 1088.
 */
void PE_fb_calibrate(void)
{
    volatile unsigned int   *w = (volatile unsigned int *) FB_BASE;
    volatile unsigned short *h = (volatile unsigned short *) FB_BASE;
    unsigned int band = 100 * 2176;      /* bytes por banda */
    unsigned int i;

    for (i = 0; i < (960 * 2176) / 4; i++)          /* todo a negro */
        w[i] = 0;

    for (i = 0;          i < band;     i += 4) w[i / 4] = 0x00FF0000;
    for (i = band;       i < 2 * band; i += 4) w[i / 4] = 0x000000FF;
    for (i = 2 * band;   i < 3 * band; i += 2) h[i / 2] = 0xF800;
    for (i = 3 * band;   i < 4 * band; i += 2) h[i / 2] = 0x001F;
}

/**
 * PE_fb_cache_flush
 *
 * Limpia+invalida la D-cache sobre el rango del framebuffer (DCCIMVAC, hasta
 * el punto de coherencia).
 *
 * Las primeras lineas del arranque se pintan con el framebuffer aun mapeado
 * como memoria NORMAL CACHEABLE por el bootloader, asi que quedan sucias en la
 * D-cache.  arm_vm_init lo remapea despues como memoria device, pero esas
 * lineas sucias siguen ahi y se vuelcan MAS TARDE, segun la cache se llena y
 * las va expulsando, reescribiendo pantalla ya actualizada: son los rayajos
 * diagonales que ensuciaban la traza, y por eso iban a mas segun avanzaba el
 * arranque.  Expulsandolas a mano ANTES del cambio de tablas, y con el mapeo
 * device ya instalado, el residuo no puede reaparecer.
 *
 * flush_dcache() esta declarada en arm/pmap.h pero NO implementada para ARM en
 * este arbol (solo existe la version x86_64), de ahi el ensamblador.
 */
void PE_fb_cache_flush(void)
{
    unsigned int addr;
    unsigned int end = FB_BASE + (unsigned int)(960 * FB_STRIDE_PX * 4);

    for (addr = FB_BASE; addr < end; addr += 32)
        __asm__ volatile ("mcr p15, 0, %0, c7, c14, 1" : : "r"(addr) : "memory");

    __asm__ volatile ("dsb" : : : "memory");
}

/**
 * PE_fb_refresh
 *
 * Repinta la pantalla ENTERA desde el buffer de texto.
 *
 * Las primeras lineas del arranque se escriben con el framebuffer aun mapeado
 * como memoria normal CACHEABLE (las tablas del bootloader).  Esas escrituras
 * se quedan sucias en la D-cache; cuando mas tarde se vuelcan, reaparecen
 * encima de la pantalla actual y producen el texto viejo rayado que veiamos.
 * Llamando a esto una vez las cachés estan configuradas, la pantalla se
 * reconstruye desde RAM y el residuo desaparece.
 */
void PE_fb_refresh(void)
{
    if (fbcon_inited)
        fbcon_redraw();
}

/**
 * PE_early_puts
 *
 * M6: re-cableado a la consola de framebuffer del krillin (arriba). El texto de XNU
 * (empezando por "arm_init: starting up") aparece en pantalla.
 */
/**
 * PE_fb_putc
 *
 * Un solo caracter. Es lo que se engancha a PE_kputc para que kprintf(),
 * kdb_printf() y panic() de XNU salgan por ESTA consola (el krillin no tiene
 * UART, y la consola vc_ pelearia por el mismo framebuffer).
 */
extern const char *panicstr;    /* != NULL en cuanto XNU entra en panic() */

void PE_fb_putc(char c)
{
    /*
     * Este es el destino de kprintf()/kdb_printf()/panic() de XNU.
     *
     * El kprintf normal es DEMASIADO verboso para 30 columnas: llena la
     * pantalla de ruido y sepulta los marcadores del arranque, que es lo unico
     * que podemos leer en una foto.  Se silencia por defecto.
     *
     * Dos excepciones: el boot-arg -xnu-verbose, y un panic() en curso, donde
     * el mensaje es justo lo que estamos buscando (ahi encogemos la letra para
     * que quepa entero).
     */
    if (panicstr == (const char *) 0 && fbcon_quiet)
        return;
    fbcon_putchar(c);
}

/* Llamado desde el pexpert si aparece el boot-arg -xnu-verbose. */
void PE_fb_verbose(void)
{
    fbcon_quiet = 0;
}

void PE_early_puts(char *s)
{
    while (*s != '\0') {
        fbcon_putchar(*s);
        s++;
    }
    return;
}
