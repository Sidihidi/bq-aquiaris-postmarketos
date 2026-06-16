#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
/* poke del PMIC MT6323 via pwrap (secuencia wacs2) del MT6582.
 * uso: pwrap_poke r <adr_hex>            (lee reg PMIC)
 *      pwrap_poke w <adr_hex> <data_hex> (escribe reg PMIC) */
#define PWRAP_BASE   0x1000D000UL
#define WACS2_CMD    0x9C
#define WACS2_RDATA  0xA0
#define WACS2_VLDCLR 0xA4
static volatile uint8_t *base;
static uint32_t rd(int o){ return *(volatile uint32_t*)(base+o); }
static void wr(int o,uint32_t v){ *(volatile uint32_t*)(base+o)=v; }
static int wait_fsm(uint32_t want){
    int i; for(i=0;i<200000;i++){ if(((rd(WACS2_RDATA)>>16)&0x7)==want) return 0; }
    return -1;
}
static int pwrap_read(uint32_t adr,uint32_t*out){
    if(wait_fsm(0)) return -1;                 /* idle */
    wr(WACS2_CMD,(adr>>1)<<16);
    if(wait_fsm(6)) return -2;                 /* wfvldclr */
    *out = rd(WACS2_RDATA)&0xffff;
    wr(WACS2_VLDCLR,1);
    return 0;
}
static int pwrap_write(uint32_t adr,uint32_t d){
    if(wait_fsm(0)) return -1;
    wr(WACS2_CMD,(1u<<31)|((adr>>1)<<16)|(d&0xffff));
    return 0;
}
int main(int argc,char**argv){
    int fd=open("/dev/mem",O_RDWR|O_SYNC);
    if(fd<0){perror("/dev/mem");return 1;}
    base=mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,PWRAP_BASE);
    if(base==MAP_FAILED){perror("mmap");return 1;}
    if(argc>=3 && argv[1][0]=='r'){
        uint32_t a=strtoul(argv[2],0,16),v; int e=pwrap_read(a,&v);
        if(e) printf("PMIC[0x%04x] read ERROR %d (FSM=0x%x)\n",a,e,(rd(WACS2_RDATA)>>16)&0x7);
        else  printf("PMIC[0x%04x] = 0x%04x\n",a,v);
    } else if(argc>=4 && argv[1][0]=='w'){
        uint32_t a=strtoul(argv[2],0,16),d=strtoul(argv[3],0,16); int e=pwrap_write(a,d);
        printf("PMIC[0x%04x] <- 0x%04x : %s\n",a,d,e?"ERROR":"ok");
    } else printf("uso: pwrap_poke r <adr> | w <adr> <data>\n");
    return 0;
}
