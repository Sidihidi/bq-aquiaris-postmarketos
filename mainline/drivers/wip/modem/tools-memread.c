#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    if (argc < 3) { printf("uso: memread <addr_hex> <n_ints>\n"); return 2; }
    unsigned long addr = strtoul(argv[1], 0, 0);
    int n = atoi(argv[2]);
    int fd = open("/dev/mem", O_RDONLY);
    if (fd < 0) { perror("open /dev/mem"); return 1; }
    if (lseek(fd, addr, SEEK_SET) == (off_t)-1) { perror("lseek"); return 1; }
    unsigned int *buf = malloc(n*4);
    int r = read(fd, buf, n*4);
    if (r < 0) { perror("read"); return 1; }
    for (int i=0;i<r/4;i++) {
        printf("%08x ", buf[i]);
        if ((i&7)==7) printf("\n");
    }
    printf("\n(leidos %d bytes de 0x%lx)\n", r, addr);
    return 0;
}
