#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <dirent.h>

static int cfd;
static void say(const char *s) { write(cfd, s, strlen(s)); }

static void cat(const char *path) {
    char buf[2048];
    int fd = open(path, O_RDONLY);
    if (fd < 0) { say("(no se pudo leer "); say(path); say(")\n"); return; }
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0) write(cfd, buf, n);
    close(fd);
}

int main(void) {
    cfd = open("/dev/console", O_WRONLY);
    if (cfd < 0) cfd = 1;
    say("\n*** MAINLINE v5: prueba de eMMC (M2) ***\n");
    mkdir("/mnt", 0755);
    int montado = 0;
    for (;;) {
        say("\n--- /proc/partitions ---\n");
        cat("/proc/partitions");
        if (!montado) {
            if (mount("/dev/mmcblk0p5", "/mnt", "ext4", MS_RDONLY, "") == 0) {
                montado = 1;
                say("\n>>> EMMC OK: mmcblk0p5 (pmOS root) MONTADA ext4 <<<\n>>> contenido: ");
                DIR *d = opendir("/mnt");
                if (d) {
                    struct dirent *e; int i = 0;
                    while ((e = readdir(d)) && i < 14) {
                        say(e->d_name); say(" ");
                        i++;
                    }
                    closedir(d);
                }
                say("<<<\n*** M2 CONSEGUIDO ***\n");
            } else {
                say("\n(aun sin montar mmcblk0p5...)\n");
            }
        } else {
            say("*** M2 CONSEGUIDO: eMMC funcionando en mainline ***\n");
        }
        sleep(8);
    }
}
