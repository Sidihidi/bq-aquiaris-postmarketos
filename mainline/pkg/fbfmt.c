#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
int main(void) {
    int fd = open("/dev/fb0", O_RDONLY);
    if (fd < 0) { perror("fb0"); return 1; }
    struct fb_var_screeninfo v;
    struct fb_fix_screeninfo f;
    ioctl(fd, FBIOGET_VSCREENINFO, &v);
    ioctl(fd, FBIOGET_FSCREENINFO, &f);
    printf("bpp=%u\n", v.bits_per_pixel);
    printf("red:    off=%u len=%u\n", v.red.offset, v.red.length);
    printf("green:  off=%u len=%u\n", v.green.offset, v.green.length);
    printf("blue:   off=%u len=%u\n", v.blue.offset, v.blue.length);
    printf("transp: off=%u len=%u\n", v.transp.offset, v.transp.length);
    printf("xres=%u yres=%u xres_virtual=%u line_length=%u\n",
           v.xres, v.yres, v.xres_virtual, f.line_length);
    // sugerir formato simplefb
    const char *fmt = "?";
    if (v.bits_per_pixel==32) {
        if (v.red.offset==16 && v.blue.offset==0) fmt="a8r8g8b8";
        else if (v.red.offset==0 && v.blue.offset==16) fmt="a8b8g8r8";
        else if (v.red.offset==16 && v.transp.length==0) fmt="x8r8g8b8";
        else if (v.red.offset==0 && v.transp.length==0) fmt="x8b8g8r8";
    } else if (v.bits_per_pixel==16) fmt="r5g6b5";
    printf("=> simplefb format = %s\n", fmt);
    return 0;
}
