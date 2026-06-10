#include <unistd.h>
#include <fcntl.h>
int main(void) {
    int fd = open("/dev/console", O_WRONLY);
    if (fd < 0) fd = 1;
    for (;;) {
        write(fd, "\n*** MAINLINE v3 VIVO en BQ Aquaris E4.5 (ramoops activo) ***\n", 62);
        sleep(5);
    }
}
