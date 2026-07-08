/* fmradio.c — radio FM completa por /dev/fm (port stock MT6627/CONNSYS on-die).
 * uso: fmradio <freq_x10>       (1023 = 102.3 MHz; banda UE, rama 100 kHz)
 *
 * Hace TODO el lado chip: POWERUP + TUNE + volumen + unmute + **I2S ON**
 * (FM_IOCTL_I2S_SETTING {ON, MASTER, 32K} — la pieza que faltaba: el powerup
 * stock lleva el I2s_Setting COMENTADO, por eso el chip nunca emitia audio).
 * Mantiene el fd abierto hasta ser matado (al cerrarse, el driver apaga el FM).
 *
 * El lado SoC (AFE: 2ºI2S in -> ASRC -> GAIN1 -> DAC -> codec HP) lo enciende
 * el kcontrol del driver de audio:  amixer -c 0 cset name='FM Radio Route' 1
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
struct fm_i2s_setting { int32_t onoff; int32_t mode; int32_t sample; };

#define FM_IOC_MAGIC 0xf5
#define FM_IOCTL_POWERUP        _IOWR(FM_IOC_MAGIC, 0, struct fm_tune_parm*)
#define FM_IOCTL_TUNE           _IOWR(FM_IOC_MAGIC, 2, struct fm_tune_parm*)
#define FM_IOCTL_SETVOL         _IOWR(FM_IOC_MAGIC, 4, uint32_t*)
#define FM_IOCTL_MUTE           _IOWR(FM_IOC_MAGIC, 6, uint32_t*)
#define FM_IOCTL_GETRSSI        _IOWR(FM_IOC_MAGIC, 7, int32_t*)
#define FM_IOCTL_I2S_SETTING    _IOWR(FM_IOC_MAGIC, 33, struct fm_i2s_setting*)
/* RESTORE_SEARCH ejecuta mt6627_restore_search() = LOS DOS WRITES que encienden
 * la salida de audio I2S del chip (FM reg 0x9B=0xF9AB "I2S Tx mode" + host
 * 0x80101054=0x3f35 "I2S Rx mode"). Es la via que funciona: el ioctl 33
 * (I2S_SETTING) devuelve FM_ELINK en este port. */
#define FM_IOCTL_RESTORE_SEARCH _IOWR(FM_IOC_MAGIC, 46, int32_t)

/* enums del kernel (fm_interface.h): ON=0, MASTER=0, 32K=0 */
#define FM_I2S_ON      0
#define FM_I2S_MASTER  0
#define FM_I2S_32K     0

static volatile sig_atomic_t quit;
static void on_sig(int s) { (void)s; quit = 1; }

int main(int argc, char **argv)
{
	int freq = (argc > 1) ? atoi(argv[1]) : 1023;
	struct fm_tune_parm p;
	struct fm_i2s_setting i2s = { FM_I2S_ON, FM_I2S_MASTER, FM_I2S_32K };
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

	/* LA PIEZA CLAVE: encender la salida de audio I2S del chip */
	r = ioctl(fd, FM_IOCTL_I2S_SETTING, &i2s);
	printf("I2S_SETTING(ON,MASTER,32K) rc=%d %s\n", r, r ? strerror(errno) : "");
	{
		int32_t z = 0;
		r = ioctl(fd, FM_IOCTL_RESTORE_SEARCH, &z);
		printf("RESTORE_SEARCH (audio I2S on: 0x9B=0xF9AB + host 0x3f35) rc=%d %s\n",
		       r, r ? strerror(errno) : "");
	}

	printf("FM sonando (fd abierto). Activa el ruteo del SoC con:\n"
	       "  amixer -c 0 cset name='FM Radio Route' 1\n"
	       "Ctrl-C / kill para parar.\n");
	fflush(stdout);
	while (!quit)
		pause();

	close(fd);	/* el driver hace powerdown al cerrar */
	return 0;
}
