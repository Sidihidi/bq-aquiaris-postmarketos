/* M3 v3 — dump ATOMICO legible + clonar pmOS en RGB565.
 * Cambios vs v2:
 *  - dump con snprintf: 1 write por linea, nivel <0> (KERN_EMERG, siempre sale),
 *    HECHO tras sleep(15) para que el ruido de initcalls del kernel ya no
 *    sobreescriba el ramoops circular (console-size 0x80000).
 *  - idle TOTAL (sleep 3600 sin imprimir) -> el dump queda intacto al final.
 *  - FIX en RGB565 (CLRFMT=1) igual que pmOS; el DT de simplefb se pasa a
 *    r5g6b5/stride1088 para que el buffer COINCIDA con lo que lee el OVL.
 * Tabla CLRFMT MT6582 (de rebuild310.log): 1=rgb565 2=argb8888 3=pargb8888. */
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>

static int kfd;
static void kline(const char*fmt,...){ char b[200]; int n; va_list a; va_start(a,fmt);
    n=vsnprintf(b,sizeof b,fmt,a); va_end(a); if(kfd>=0&&n>0) write(kfd,b,n); }
#define R(base,off) (base[(off)/4])

int main(void){
    mkdir("/dev",0755); mkdir("/proc",0755);
    mount("proc","/proc","proc",0,""); mount("devtmpfs","/dev","devtmpfs",0,"");
    kfd=open("/dev/kmsg",O_WRONLY);
    sleep(15);                       /* esperar a que el kernel deje de spamear */
    kline("<0>M3v3: start\n");
    int mfd=open("/dev/mem",O_RDWR|O_SYNC);
    if(mfd<0){ for(;;){kline("<0>M3v3: no /dev/mem\n");sleep(60);} }
    volatile uint32_t *ovl =mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x14007000);
    volatile uint32_t *rdma=mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x14008000);
    volatile uint32_t *bls =mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x1400A000);
    volatile uint32_t *col =mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x1400B000);
    volatile uint32_t *mtx =mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x1400E000);
    if(ovl==MAP_FAILED){ for(;;){kline("<0>M3v3: mmap OVL fail\n");sleep(60);} }

    int fb=-1;
    for(int i=0;i<4;i++) if(R(ovl,0x40+i*0x20)==0xBF400000) fb=i;
    kline("<0>M3v3 ANTES SRC_CON=%08x fblayer=%d\n",(unsigned)R(ovl,0x2C),fb);
    for(int i=0;i<4;i++)
        kline("<0> L%d CON=%08x KEY=%08x ADDR=%08x PITCH=%08x\n", i,
            (unsigned)R(ovl,0x30+i*0x20),(unsigned)R(ovl,0x34+i*0x20),
            (unsigned)R(ovl,0x40+i*0x20),(unsigned)R(ovl,0x44+i*0x20));
    kline("<0>M3v3 COLOR=%08x BLS=%08x MUTEX_EN=%08x MUTEX_MOD=%08x\n",
        (unsigned)(col!=MAP_FAILED?R(col,0x400):0),(unsigned)(bls!=MAP_FAILED?R(bls,0):0),
        (unsigned)(mtx!=MAP_FAILED?R(mtx,0):0),(unsigned)(mtx!=MAP_FAILED?R(mtx,0x2C):0));

    /* ---- FIX: clonar pmOS en L3 (RGB565), el DT ya pone simplefb en r5g6b5 ---- */
    R(ovl,0x90)=0x400010ff;   /* L3 CON  (CLRFMT=1 rgb565) */
    R(ovl,0x94)=0xff000000;   /* L3 KEY */
    R(ovl,0x98)=0x03c0021c;   /* L3 SRC_SIZE 960x540 */
    R(ovl,0x9C)=0x00000000;   /* L3 OFFSET */
    R(ovl,0xA0)=0xBF400000;   /* L3 ADDR */
    R(ovl,0xA4)=0x00000440;   /* L3 PITCH 1088 (=544*2) */
    R(ovl,0x28)=0x00000000;   /* ROI_BGCLR */
    R(ovl,0xC0)=0x03ff0000;   /* OVL RDMA0_CON */
    R(ovl,0x2C)=0x00000008;   /* SRC_CON: solo L3 */
    if(rdma!=MAP_FAILED){ R(rdma,0x14)=0x0000021c; R(rdma,0x18)=0x000003c0; R(rdma,0x00)=0x0000003f; }
    if(col!=MAP_FAILED)  R(col,0x400)=0x20000000;
    if(bls!=MAP_FAILED){ R(bls,0x00)=0x00010001; R(bls,0x10)=0x00100007; }
    if(mtx!=MAP_FAILED){ R(mtx,0x2C)=0x00000680; R(mtx,0x30)=0x00000001;
                         R(mtx,0x00)=0x00000000; R(mtx,0x00)=0x00000303; }
    msync((void*)ovl,0x1000,MS_SYNC);

    kline("<0>M3v3 DESPUES SRC_CON=%08x L3CON=%08x COLOR=%08x MUTEX_EN=%08x\n",
        (unsigned)R(ovl,0x2C),(unsigned)R(ovl,0x90),
        (unsigned)(col!=MAP_FAILED?R(col,0x400):0),(unsigned)(mtx!=MAP_FAILED?R(mtx,0):0));
    kline("<0>M3v3: aplicado. pantalla: LEGIBLE / AMARILLO / BASURA?\n");
    for(;;) sleep(3600);   /* idle TOTAL: preserva el ramoops */
    return 0;
}
