/* M2c — init de verificacion de la CONSOLIDACION.
 * Ya NO toca el display (el fix lo hace el kernel: late_initcall krillin_dispfix).
 * Solo levanta usb0 (red por USB). Si la pantalla sale BIEN con este init, la
 * consolidacion del display en el kernel funciona. */
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
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

static void setup_usb0(void){
    int s=socket(AF_INET,SOCK_DGRAM,0);
    if(s<0){ kline("<0>M2c: socket fail\n"); return; }
    struct ifreq ifr; struct sockaddr_in *sin=(struct sockaddr_in*)&ifr.ifr_addr;
    int i, have=0;
    for(i=0;i<40;i++){
        memset(&ifr,0,sizeof ifr); strncpy(ifr.ifr_name,"usb0",IFNAMSIZ);
        if(ioctl(s,SIOCGIFINDEX,&ifr)==0){ have=1; break; }
        sleep(1);
    }
    if(!have){ kline("<0>M2c: usb0 NO aparecio\n"); close(s); return; }
    memset(&ifr,0,sizeof ifr); strncpy(ifr.ifr_name,"usb0",IFNAMSIZ);
    sin->sin_family=AF_INET; inet_pton(AF_INET,"172.16.42.1",&sin->sin_addr);
    if(ioctl(s,SIOCSIFADDR,&ifr)<0) kline("<0>M2c: SIOCSIFADDR fail\n");
    memset(&ifr,0,sizeof ifr); strncpy(ifr.ifr_name,"usb0",IFNAMSIZ);
    sin->sin_family=AF_INET; inet_pton(AF_INET,"255.255.255.0",&sin->sin_addr);
    if(ioctl(s,SIOCSIFNETMASK,&ifr)<0) kline("<0>M2c: netmask fail\n");
    memset(&ifr,0,sizeof ifr); strncpy(ifr.ifr_name,"usb0",IFNAMSIZ);
    ifr.ifr_flags=IFF_UP|IFF_RUNNING|IFF_BROADCAST|IFF_MULTICAST;
    if(ioctl(s,SIOCSIFFLAGS,&ifr)<0) kline("<0>M2c: SIOCSIFFLAGS fail\n");
    kline("<0>M2c: usb0 = 172.16.42.1/24 UP\n");
    close(s);
}

int main(void){
    mkdir("/dev",0755); mkdir("/proc",0755); mkdir("/sys",0755);
    mount("proc","/proc","proc",0,""); mount("devtmpfs","/dev","devtmpfs",0,"");
    mount("sysfs","/sys","sysfs",0,"");
    kfd=open("/dev/kmsg",O_WRONLY);
    kline("<0>M2c: start (display = kernel late_initcall; init solo red)\n");
    setup_usb0();
    kline("<0>M2c: listo. pantalla debe verse BIEN sin que el init la toque\n");
    for(;;) sleep(3600);
    return 0;
}
