#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
/* devmem <addr_hex> [val_hex] : lee/escribe un u32 fisico (mmap pagina+offset) */
int main(int argc, char **argv){
    if(argc<2){ fprintf(stderr,"uso: devmem <addr_hex> [val_hex]\n"); return 1; }
    unsigned long addr = strtoul(argv[1],0,16);
    unsigned long page = addr & ~0xFFFUL, off = addr & 0xFFF;
    int fd = open("/dev/mem", O_RDWR|O_SYNC);
    if(fd<0){ perror("/dev/mem"); return 1; }
    volatile uint32_t *m = mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,page);
    if(m==MAP_FAILED){ perror("mmap"); return 1; }
    if(argc>=3){
        uint32_t v=(uint32_t)strtoul(argv[2],0,16);
        m[off/4]=v;
        printf("0x%08lx <- 0x%08x (rb 0x%08x)\n", addr, v, m[off/4]);
    } else {
        printf("0x%08lx = 0x%08x\n", addr, m[off/4]);
    }
    return 0;
}
