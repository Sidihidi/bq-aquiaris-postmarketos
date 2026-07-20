/* Poller pasivo: mmap /dev/ccci_fs, detecta cuando un buffer pasa a RESPUESTA
 * (op high16 == 0xffff) y vuelca la respuesta completa (24 words). Coexiste con ccci_fsd. */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#define BUFSZ 16388
#define NBUF  5
int main(int argc, char**argv){
  long iters = argc>1 ? atol(argv[1]) : 30000000L;
  unsigned long mapsz = ((unsigned long)BUFSZ*NBUF + 0xfff) & ~0xfffUL;
  int fd=-1, tries; volatile uint8_t *m = MAP_FAILED;
  for(tries=0; tries<600; tries++){
    if(fd<0) fd = open("/dev/ccci_fs", O_RDWR);
    if(fd>=0 && m==MAP_FAILED) m = mmap(0, mapsz, PROT_READ, MAP_SHARED, fd, 0);
    if(fd>=0 && m!=MAP_FAILED) break;
    usleep(100000);
  }
  if(m==MAP_FAILED){ fprintf(stderr,"give up\n"); return 2; }
  fprintf(stderr,"ready\n");
  uint32_t last[NBUF]; int b,w; for(b=0;b<NBUF;b++) last[b]=0xdeadbeef;
  struct timeval tv; long i;
  for(i=0;i<iters;i++){ usleep(15);
    for(b=0;b<NBUF;b++){
      volatile uint32_t *p=(volatile uint32_t*)(m+(unsigned long)b*BUFSZ);
      uint32_t op=p[0];
      if(op!=last[b]){ gettimeofday(&tv,0);
        printf("%ld.%06ld idx=%d op=%08x |", (long)tv.tv_sec%100000,(long)tv.tv_usec,b,op);
        for(w=1;w<24;w++) printf(" %08x", p[w]);
        printf("\n"); fflush(stdout); last[b]=op; }
    }
  }
  return 0;
}
