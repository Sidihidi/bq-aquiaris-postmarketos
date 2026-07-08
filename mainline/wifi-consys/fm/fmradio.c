/* fmradio.c — radio FM completa por /dev/fm (port stock MT6627/CONNSYS on-die).
 * uso: fmradio <freq_x10>       (1023 = 102.3 MHz; banda UE, rama 100 kHz)
 *
 * Hace el lado chip: POWERUP + TUNE + volumen + unmute. Con eso el DSP del FM
 * queda SONANDO y emitiendo audio digital por el I2S interno hacia el AFE
 * (FM_MAIN_CTRL 0x63 = TUNE activo; verificado por ground-truth 0708).
 *
 * ⚠️ NO llamar a FM_IOCTL_RESTORE_SEARCH (46): su primera accion es
 * mt6627_RampDown() = PARA el DSP de audio -> la salida I2S se queda en DC
 * (silencio). Fue la causa del "FM sintoniza pero no suena". El I2S del chip
 * NO necesita enable extra: sale activo del powerup+tune (LineageOS suena
 * con 0x9B=0x000b, sin el write 0xF9AB).
 *
 * Mantiene el fd abierto hasta ser matado (al cerrarse, el driver apaga el FM).
 *
 * El lado SoC (2º I2S in -> ASRC 32k->44.1k -> conexion directa I00/I01 ->
 * O03/O04 -> DAC -> codec HP) lo enciende el kcontrol del driver de audio:
 *   amixer -c 0 cset name='FM Radio Route' 1
 * Los auriculares son la antena: enchufalos.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

struct fm_tune_parm { uint8_t err; uint8_t band; uint8_t space; uint8_t hilo; uint16_t freq; };

#define FM_IOC_MAGIC 0xf5
#define FM_IOCTL_POWERUP        _IOWR(FM_IOC_MAGIC, 0, struct fm_tune_parm*)
#define FM_IOCTL_TUNE           _IOWR(FM_IOC_MAGIC, 2, struct fm_tune_parm*)
#define FM_IOCTL_SETVOL         _IOWR(FM_IOC_MAGIC, 4, uint32_t*)
#define FM_IOCTL_MUTE           _IOWR(FM_IOC_MAGIC, 6, uint32_t*)
#define FM_IOCTL_GETRSSI        _IOWR(FM_IOC_MAGIC, 7, int32_t*)

static volatile sig_atomic_t quit;
static void on_sig(int s) { (void)s; quit = 1; }

int main(int argc, char **argv)
{
	int freq = (argc > 1) ? atoi(argv[1]) : 1023;
	struct fm_tune_parm p;
	uint32_t vol = 15, mute = 0;
	int32_t rssi = 0;
	int fd, r;

	signal(SIGINT, on_sig);
	signal(SIGTERM, on_sig);

	fd = open("/dev/fm", O_RDWR);
	if (fd < 0) { printf("open /dev/fm: %s\n", strerror(errno)); return 1; }

	memset(&p, 0, sizeof(p));
	p.band = 1; p.space = 2; p.freq = (uint16_t)freq;
	r = ioctl(fd, FM_IOCTL_POWERUP, &p);
	printf("POWERUP rc=%d err=%d\n", r, p.err);
	if (r) { close(fd); return 1; }

	memset(&p, 0, sizeof(p));
	p.band = 1; p.space = 2; p.freq = (uint16_t)freq;
	r = ioctl(fd, FM_IOCTL_TUNE, &p);
	ioctl(fd, FM_IOCTL_GETRSSI, &rssi);
	printf("TUNE %d.%d MHz rc=%d err=%d RSSI=%d\n", freq / 10, freq % 10, r, p.err, rssi);

	ioctl(fd, FM_IOCTL_SETVOL, &vol);
	ioctl(fd, FM_IOCTL_MUTE, &mute);

	printf("FM sonando (fd abierto). Activa el ruteo del SoC con:\n"
	       "  amixer -c 0 cset name='FM Radio Route' 1\n"
	       "Ctrl-C / kill para parar.\n");
	fflush(stdout);
	while (!quit)
		pause();

	close(fd);	/* el driver hace powerdown al cerrar */
	return 0;
}
