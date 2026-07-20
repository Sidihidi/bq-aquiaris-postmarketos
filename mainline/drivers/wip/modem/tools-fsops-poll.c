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
  long iters = argc>1 ? atol(argv[1]) : 400000000L;
  unsigned long mapsz = ((unsigned long)BUFSZ*NBUF + 0xfff) & ~0xfffUL;
  int fd = open("/dev/ccci_fs", O_RDWR);
  if(fd<0){ perror("open /dev/ccci_fs"); return 1; }
  volatile uint8_t *m = mmap(0, mapsz, PROT_READ, MAP_SHARED, fd, 0);
  if(m==MAP_FAILED){ perror("mmap"); return 2; }
  uint32_t last[NBUF]; int b; for(b=0;b<NBUF;b++) last[b]=0xdeadbeef;
  struct timeval tv; long i;
  for(i=0;i<iters;i++){
    for(b=0;b<NBUF;b++){
      volatile uint32_t *p=(volatile uint32_t*)(m+(unsigned long)b*BUFSZ);
      uint32_t op=p[0];
      if(op!=last[b]){
        gettimeofday(&tv,0);
        printf("%ld.%06ld idx=%d op=%08x p0=%08x p1=%08x p2=%08x p3=%08x\n",
               (long)tv.tv_sec%100000,(long)tv.tv_usec,b,op,p[1],p[2],p[3],p[4]);
        fflush(stdout); last[b]=op;
      }
    }
  }
  return 0;
}
