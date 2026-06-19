// SPDX-License-Identifier: GPL-2.0
/*
 * mtk-gps-bridge.c — reemplazo mínimo de `mnld` para el GPS del CONSYS MT6582.
 *
 * Arranca el GPS por /dev/stpgps (canal STP 2) y reenvía el NMEA a un PTY que
 * gpsd consume:  /dev/stpgps  <->  /dev/gps0 (pty)  ->  gpsd  ->  geoclue/Phosh.
 *
 * El kernel (mt6582-btif.c) ya hace func_on(GPS) al abrir /dev/stpgps; este
 * daemon solo (1) escribe la secuencia de arranque capturada de mnld en Android
 * y (2) bombea bytes en ambos sentidos.
 *
 * Compilar:  gcc -O2 -Wall -o mtk-gps-bridge mtk-gps-bridge.c   (musl/Alpine OK)
 * Uso:       ./mtk-gps-bridge   (luego: gpsd -N -n /dev/gps0 ; cgps)
 */
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

/* ====== SECUENCIA DE ARRANQUE ======
 * Rellenar con los write() que mnld envía a /dev/stpgps (ver GPS-CONSYS.md, Stage 2).
 * Vacío = solo bombear: sirve para ver si el GPS escupe algo tras func_on(GPS).
 * Si la captura muestra varios write() seguidos, ponerlos como tramas separadas. */
static const unsigned char START_SEQ[] = {
	/* TODO: pegar aquí los bytes del/los write() de arranque capturados de mnld. */
};

static volatile sig_atomic_t run = 1;
static void on_sig(int s) { (void)s; run = 0; }

static int open_pty(char *link)
{
	int m = posix_openpt(O_RDWR | O_NOCTTY);
	if (m < 0 || grantpt(m) || unlockpt(m)) {
		perror("pty");
		return -1;
	}
	const char *sl = ptsname(m);
	if (!sl) { perror("ptsname"); return -1; }
	unlink(link);
	if (symlink(sl, link))
		fprintf(stderr, "aviso: no pude symlink %s -> %s (%s)\n", link, sl, strerror(errno));
	else
		fprintf(stderr, "pty: %s -> %s (apunta gpsd aquí)\n", link, sl);
	return m;
}

int main(int argc, char **argv)
{
	const char *dev = (argc > 1) ? argv[1] : "/dev/stpgps";
	const char *link = (argc > 2) ? argv[2] : "/dev/gps0";
	unsigned char buf[2048];
	ssize_t n;

	signal(SIGINT, on_sig);
	signal(SIGTERM, on_sig);

	int fd = open(dev, O_RDWR);
	if (fd < 0) { fprintf(stderr, "open %s: %s\n", dev, strerror(errno)); return 1; }
	fprintf(stderr, "abierto %s (kernel hace func_on(GPS) + bring-up)\n", dev);

	int pty = open_pty((char *)link);
	if (pty < 0) { close(fd); return 1; }

	if (sizeof(START_SEQ) > 0) {
		if (write(fd, START_SEQ, sizeof(START_SEQ)) != (ssize_t)sizeof(START_SEQ))
			fprintf(stderr, "aviso: START_SEQ no se escribió entera (%s)\n", strerror(errno));
		else
			fprintf(stderr, "START_SEQ enviada (%zu bytes) — esperando NMEA...\n", sizeof(START_SEQ));
	} else {
		fprintf(stderr, "START_SEQ vacía: solo bombeo (¿emite el GPS tras func_on?)\n");
	}

	struct pollfd p[2] = { { fd, POLLIN, 0 }, { pty, POLLIN, 0 } };
	while (run) {
		if (poll(p, 2, 1000) < 0) { if (errno == EINTR) continue; break; }
		if (p[0].revents & POLLIN) {		/* GPS -> gpsd (NMEA) */
			n = read(fd, buf, sizeof(buf));
			if (n > 0 && write(pty, buf, n) < 0) { /* gpsd no leyó */ }
		}
		if (p[1].revents & POLLIN) {		/* gpsd -> GPS (comandos $PMTK, opcional) */
			n = read(pty, buf, sizeof(buf));
			if (n > 0 && write(fd, buf, n) < 0)
				fprintf(stderr, "write a %s: %s\n", dev, strerror(errno));
		}
	}
	fprintf(stderr, "saliendo\n");
	close(pty);
	close(fd);
	return 0;
}
