#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    if (argc < 3) { printf("uso: %s <dev> <req_hex>\n", argv[0]); return 2; }
    int fd = open(argv[1], O_RDWR);
    if (fd < 0) { perror("open"); return 1; }
    unsigned long req = strtoul(argv[2], 0, 0);
    int ret = ioctl(fd, req, 0);
    printf("ioctl(%s,0x%lx)=%d errno-ok\n", argv[1], req, ret);
    return 0;
}
