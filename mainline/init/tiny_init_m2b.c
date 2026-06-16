/* M2b — display fix (clonar pmOS RGB565, igual que m3v3) + levantar usb0.
 * El gadget g_ether (built-in) ya enumera con el driver musb v16; aqui solo
 * configuramos la interfaz del lado telefono (ioctl, no hay /sbin/ip):
 *   usb0 = 172.16.42.1/24, UP.  Host (Pi) = 172.16.42.2/24.
 * Asi el carrier sube y hay ping/red por USB en mainline. */
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int kfd;
static void kline(const char*fmt,...){ char b[200]; int n; va_list a; va_start(a,fmt);
    n=vsnprintf(b,sizeof b,fmt,a); va_end(a); if(kfd>=0&&n>0) write(kfd,b,n); }
#define R(base,off) (base[(off)/4])

static void setup_usb0(void){
    int s=socket(AF_INET,SOCK_DGRAM,0);
    if(s<0){ kline("<0>M2b: socket fail\n"); return; }
    struct ifreq ifr; struct sockaddr_in *sin=(struct sockaddr_in*)&ifr.ifr_addr;
    int i, have=0;
    for(i=0;i<40;i++){                       /* esperar a que el gadget cree usb0 */
        memset(&ifr,0,sizeof ifr); strncpy(ifr.ifr_name,"usb0",IFNAMSIZ);
        if(ioctl(s,SIOCGIFINDEX,&ifr)==0){ have=1; break; }
        sleep(1);
    }
    if(!have){ kline("<0>M2b: usb0 NO aparecio (gadget no enumerado?)\n"); close(s); return; }
    kline("<0>M2b: usb0 presente tras %ds, configurando IP\n", i);
    memset(&ifr,0,sizeof ifr); strncpy(ifr.ifr_name,"usb0",IFNAMSIZ);
    sin->sin_family=AF_INET; inet_pton(AF_INET,"172.16.42.1",&sin->sin_addr);
    if(ioctl(s,SIOCSIFADDR,&ifr)<0) kline("<0>M2b: SIOCSIFADDR fail\n");
    memset(&ifr,0,sizeof ifr); strncpy(ifr.ifr_name,"usb0",IFNAMSIZ);
    sin->sin_family=AF_INET; inet_pton(AF_INET,"255.255.255.0",&sin->sin_addr);
    if(ioctl(s,SIOCSIFNETMASK,&ifr)<0) kline("<0>M2b: netmask fail\n");
    memset(&ifr,0,sizeof ifr); strncpy(ifr.ifr_name,"usb0",IFNAMSIZ);
    ifr.ifr_flags=IFF_UP|IFF_RUNNING|IFF_BROADCAST|IFF_MULTICAST;
    if(ioctl(s,SIOCSIFFLAGS,&ifr)<0) kline("<0>M2b: SIOCSIFFLAGS fail\n");
    kline("<0>M2b: usb0 = 172.16.42.1/24 UP  (ping desde Pi 172.16.42.2)\n");
    close(s);
}

int main(void){
    mkdir("/dev",0755); mkdir("/proc",0755); mkdir("/sys",0755);
    mount("proc","/proc","proc",0,""); mount("devtmpfs","/dev","devtmpfs",0,"");
    mount("sysfs","/sys","sysfs",0,"");
    kfd=open("/dev/kmsg",O_WRONLY);
    sleep(12);
    kline("<0>M2b: start\n");
    /* ---- display fix (clonar pmOS RGB565, identico a m3v3) ---- */
    int mfd=open("/dev/mem",O_RDWR|O_SYNC);
    if(mfd>=0){
        volatile uint32_t *ovl =mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x14007000);
        volatile uint32_t *rdma=mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x14008000);
        volatile uint32_t *bls =mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x1400A000);
        volatile uint32_t *col =mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x1400B000);
        volatile uint32_t *mtx =mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,mfd,0x1400E000);
        if(ovl!=MAP_FAILED){
            R(ovl,0x90)=0x400010ff; R(ovl,0x94)=0xff000000; R(ovl,0x98)=0x03c0021c;
            R(ovl,0x9C)=0; R(ovl,0xA0)=0xBF400000; R(ovl,0xA4)=0x00000440;
            R(ovl,0x28)=0; R(ovl,0xC0)=0x03ff0000; R(ovl,0x2C)=0x00000008;
            if(rdma!=MAP_FAILED){ R(rdma,0x14)=0x21c; R(rdma,0x18)=0x3c0; R(rdma,0x00)=0x3f; }
            if(col!=MAP_FAILED)  R(col,0x400)=0x20000000;
            if(bls!=MAP_FAILED){ R(bls,0x00)=0x00010001; R(bls,0x10)=0x00100007; }
            if(mtx!=MAP_FAILED){ R(mtx,0x2C)=0x680; R(mtx,0x30)=1; R(mtx,0x00)=0; R(mtx,0x00)=0x303; }
            msync((void*)ovl,0x1000,MS_SYNC);
            kline("<0>M2b: display fix aplicado\n");
        }
    } else kline("<0>M2b: no /dev/mem (display sin fix)\n");
    /* ---- red por USB ---- */
    setup_usb0();
    kline("<0>M2b: listo. Pi: ip addr add 172.16.42.2/24 dev usb0; ping 172.16.42.1\n");
    for(;;) sleep(3600);
    return 0;
}
