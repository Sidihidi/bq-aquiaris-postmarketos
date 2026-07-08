/* fmtest.c — prueba mínima del /dev/fm del port stock MT6627.
 * uso: fmtest [freq_x10]   (980 = 98.0 MHz por defecto). freq = MHz*10 (rama 100kHz). */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct fm_tune_parm { uint8_t err; uint8_t band; uint8_t space; uint8_t hilo; uint16_t freq; };

#define FM_IOC_MAGIC 0xf5
#define FM_IOCTL_POWERUP           _IOWR(FM_IOC_MAGIC, 0, struct fm_tune_parm*)
#define FM_IOCTL_POWERDOWN         _IOWR(FM_IOC_MAGIC, 1, int32_t*)
#define FM_IOCTL_TUNE              _IOWR(FM_IOC_MAGIC, 2, struct fm_tune_parm*)
#define FM_IOCTL_SETVOL            _IOWR(FM_IOC_MAGIC, 4, uint32_t*)
#define FM_IOCTL_GETRSSI           _IOWR(FM_IOC_MAGIC, 7, int32_t*)
#define FM_IOCTL_GETCHIPID         _IOWR(FM_IOC_MAGIC, 10, uint16_t*)
#define FM_IOCTL_IS_FM_POWERED_UP  _IOWR(FM_IOC_MAGIC, 24, uint32_t*)

int main(int argc, char **argv)
{
	int freq = (argc > 1) ? atoi(argv[1]) : 980;	/* 98.0 MHz */
	struct fm_tune_parm p;
	uint16_t chip = 0;
	uint32_t up = 0, vol = 8;
	int32_t rssi = 0;
	int r, fd;

	fd = open("/dev/fm", O_RDWR);
	if (fd < 0) { printf("open /dev/fm: %s\n", strerror(errno)); return 1; }
	printf("/dev/fm abierto (fd=%d)\n", fd);

	r = ioctl(fd, FM_IOCTL_GETCHIPID, &chip);
	printf("GETCHIPID rc=%d chip=0x%04x %s\n", r, chip, r ? strerror(errno) : "");

	memset(&p, 0, sizeof(p));
	p.band = 1; p.space = 2; p.hilo = 0; p.freq = (uint16_t)freq;	/* UE band, 100kHz */
	printf(">> POWERUP band=%d space=%d freq=%d (%d.%d MHz)\n", p.band, p.space, p.freq, freq/10, freq%10);
	r = ioctl(fd, FM_IOCTL_POWERUP, &p);
	printf("   POWERUP rc=%d err=%d freq_out=%d %s\n", r, p.err, p.freq, r ? strerror(errno) : "");

	up = 0; ioctl(fd, FM_IOCTL_IS_FM_POWERED_UP, &up);
	printf("   IS_POWERED_UP=%u\n", up);

	ioctl(fd, FM_IOCTL_SETVOL, &vol);
	rssi = 0; r = ioctl(fd, FM_IOCTL_GETRSSI, &rssi);
	printf("   RSSI(tras powerup)=%d rc=%d\n", rssi, r);

	memset(&p, 0, sizeof(p));
	p.band = 1; p.space = 2; p.hilo = 0; p.freq = (uint16_t)freq;
	r = ioctl(fd, FM_IOCTL_TUNE, &p);
	printf(">> TUNE %d rc=%d err=%d freq_out=%d %s\n", freq, r, p.err, p.freq, r ? strerror(errno) : "");
	rssi = 0; ioctl(fd, FM_IOCTL_GETRSSI, &rssi);
	printf("   RSSI(tras tune)=%d\n", rssi);

	printf("== FM dejado ENCENDIDO en %d.%d MHz. Audio = analogico al jack: enchufa cascos ==\n", freq/10, freq%10);
	close(fd);
	return 0;
}
