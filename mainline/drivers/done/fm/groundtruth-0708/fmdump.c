/* fmdump — vuelca los 256 regs del chip FM via FM_IOCTL_RW_REG (no toca nada) */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
struct fm_ctl_parm { uint8_t err; uint8_t addr; uint16_t val; uint16_t rw_flag; };
#define FM_IOC_MAGIC 0xf5
#define FM_IOCTL_RW_REG _IOWR(FM_IOC_MAGIC, 12, struct fm_ctl_parm*)
int main(void) {
    int fd = open("/dev/fm", O_RDWR);
    if (fd < 0) { printf("open /dev/fm: %s\n", strerror(errno)); return 1; }
    for (int a = 0; a < 256; a++) {
        struct fm_ctl_parm p; memset(&p, 0, sizeof(p));
        p.addr = (uint8_t)a; p.rw_flag = 1;
        int r = ioctl(fd, FM_IOCTL_RW_REG, &p);
        if (r == 0) printf("%02x=%04x\n", a, p.val);
        else printf("%02x=ERR(%d)\n", a, r);
    }
    close(fd);
    return 0;
}
