#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "uso: memdump <addr_hex> <size_hex>\n"); return 1; }
    unsigned long addr = strtoul(argv[1], 0, 16), size = strtoul(argv[2], 0, 16);
    int fd = open("/dev/mem", O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    void *p = mmap(0, size, PROT_READ, MAP_SHARED, fd, addr);
    if (p == MAP_FAILED) { perror("mmap"); return 1; }
    fwrite(p, 1, size, stdout);
    return 0;
}
