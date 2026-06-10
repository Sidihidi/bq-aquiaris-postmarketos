#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "uso: memwrite <addr_hex> <string>\n"); return 1; }
    unsigned long addr = strtoul(argv[1], 0, 16);
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }
    char *p = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);
    if (p == MAP_FAILED) { perror("mmap"); return 1; }
    strcpy(p, argv[2]);
    msync(p, 4096, MS_SYNC);
    printf("escrito '%s' en 0x%lx\n", argv[2], addr);
    return 0;
}
