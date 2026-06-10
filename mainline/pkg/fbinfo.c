#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
int main(void) {
    int fd = open("/dev/fb0", O_RDONLY);
    if (fd < 0) { perror("fb0"); return 1; }
    struct fb_fix_screeninfo fix;
    ioctl(fd, FBIOGET_FSCREENINFO, &fix);
    printf("smem_start=0x%lx smem_len=0x%x (%u KB)\n", fix.smem_start, fix.smem_len, fix.smem_len/1024);
    return 0;
}
