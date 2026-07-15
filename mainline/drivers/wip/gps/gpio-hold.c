/* gpio-hold <line> <val> : pone la linea GPIO a <val> y la MANTIENE (kernel pinctrl
 * gestiona modo/dir) hasta que se mate el proceso. Para /dev/gpiochip0 del mt6582. */
#include <linux/gpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "uso: gpio-hold <line> <0|1>\n"); return 2; }
    int line = atoi(argv[1]), val = atoi(argv[2]);
    int fd = open("/dev/gpiochip0", O_RDWR);
    if (fd < 0) { perror("open gpiochip0"); return 1; }
    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = line;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.default_values[0] = val;
    strncpy(req.consumer_label, "gps-lna", sizeof(req.consumer_label)-1);
    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) { perror("GET_LINEHANDLE"); return 1; }
    printf("GPIO %d = %d (mantenido; kill para soltar)\n", line, val);
    fflush(stdout);
    pause();
    return 0;
}
