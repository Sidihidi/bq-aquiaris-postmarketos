/* ft5336_touch.c - driver de espacio de usuario para el touchscreen FT5336
 * del BQ Aquaris E4.5 (krillin / MT6582), mainline 7.0.12.
 *
 * Lee el bloque de toque del FT5336 por I2C0 (0x38) en polling y lo emite
 * como dispositivo de pantalla tactil (single-touch + MT-B 1 slot) por
 * /dev/uinput -> /dev/input.
 *
 *   ft5336_touch            -> daemon: crea el dispositivo uinput y reporta toques
 *   ft5336_touch --raw      -> imprime cada muestra (rc/status/evt/x/y), sin uinput
 *   ft5336_touch --once     -> imprime una muestra y sale (diagnostico)
 *
 * Cross-compilar (armhf static):
 *   arm-linux-gnueabihf-gcc -O2 -static -o ft5336_touch ft5336_touch.c
 *
 * HALLAZGOS validados en hardware:
 *  - El i2c-mt65xx del MT6582 (mainline) solo lee fiable <=8 bytes (FIFO=8);
 *    lecturas mas largas necesitan APDMA, que da timeout. Por eso leemos 7 bytes.
 *  - En reposo el chip entra en monitor y los regs leen 0xFF (=> sin dedo).
 *  - El dedo se detecta por el EVENT FLAG (bits 7-6 de XH), no por td_status (flaky).
 *  - Power VGP1: /etc/local.d/touch-power.start. Pantalla 540x960.
 *  - TODO multitouch: trocear en lecturas de <=8 bytes o arreglar el apdma del i2c.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/uinput.h>
#include <linux/input.h>

#define I2C_DEV  "/dev/i2c-0"
#define ADDR     0x38
#define RDLEN    7          /* mode,gest,status,t1: xh,xl,yh,yl  (<=8 = FIFO) */
#define SCR_X    540        /* qHD krillin */
#define SCR_Y    960
#define POLL_US  16000      /* ~60 Hz de muestreo */

static int i2c_fd = -1, ui_fd = -1;
static volatile int running = 1;
static void on_sig(int s){ (void)s; running = 0; }

/* lectura combinada (write reg + repeated-START + read) <=8 bytes */
static int ft_read(uint8_t reg, uint8_t *buf, int n)
{
    struct i2c_msg msgs[2] = {
        { .addr = ADDR, .flags = 0,        .len = 1, .buf = &reg },
        { .addr = ADDR, .flags = I2C_M_RD, .len = (uint16_t)n, .buf = buf },
    };
    struct i2c_rdwr_ioctl_data x = { .msgs = msgs, .nmsgs = 2 };
    return ioctl(i2c_fd, I2C_RDWR, &x);
}

static void emit(int type, int code, int val)
{
    struct input_event ev;
    memset(&ev, 0, sizeof ev);
    ev.type = type; ev.code = code; ev.value = val;
    if (write(ui_fd, &ev, sizeof ev) < 0) { /* ignora EAGAIN */ }
}

static int uinput_setup(void)
{
    ui_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (ui_fd < 0) { perror("open /dev/uinput"); return -1; }

    ioctl(ui_fd, UI_SET_EVBIT, EV_SYN);
    ioctl(ui_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(ui_fd, UI_SET_EVBIT, EV_ABS);
    ioctl(ui_fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(ui_fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);   /* es una pantalla */
    ioctl(ui_fd, UI_SET_ABSBIT, ABS_X);
    ioctl(ui_fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(ui_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
    ioctl(ui_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    ioctl(ui_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
    ioctl(ui_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);

    struct uinput_user_dev us;
    memset(&us, 0, sizeof us);
    strncpy(us.name, "ft5336", sizeof us.name - 1);
    us.id.bustype = BUS_I2C;
    us.id.vendor  = 0x5a;
    us.id.product = 0x14;
    us.id.version = 1;
    us.absmax[ABS_X] = SCR_X - 1;
    us.absmax[ABS_Y] = SCR_Y - 1;
    us.absmax[ABS_MT_POSITION_X] = SCR_X - 1;
    us.absmax[ABS_MT_POSITION_Y] = SCR_Y - 1;
    us.absmax[ABS_MT_SLOT] = 0;
    us.absmin[ABS_MT_TRACKING_ID] = 0;
    us.absmax[ABS_MT_TRACKING_ID] = 0xffff;
    if (write(ui_fd, &us, sizeof us) < 0) { perror("write uinput_user_dev"); return -1; }
    if (ioctl(ui_fd, UI_DEV_CREATE) < 0)  { perror("UI_DEV_CREATE");        return -1; }
    return 0;
}

int main(int argc, char **argv)
{
    int raw  = (argc > 1 && !strcmp(argv[1], "--raw"));
    int once = (argc > 1 && !strcmp(argv[1], "--once"));

    signal(SIGINT, on_sig); signal(SIGTERM, on_sig);

    i2c_fd = open(I2C_DEV, O_RDWR);
    if (i2c_fd < 0) { perror("open " I2C_DEV); return 1; }

    if (!raw && !once) {
        if (uinput_setup() < 0) {
            fprintf(stderr, "uinput no disponible (¿CONFIG_INPUT_UINPUT?). "
                            "Prueba: ft5336_touch --raw\n");
            return 1;
        }
        fprintf(stderr, "ft5336: dispositivo uinput creado (%dx%d). Toca la pantalla.\n",
                SCR_X, SCR_Y);
    }

    int wasdown = 0, next_tid = 0, sample = 0;
    uint8_t buf[RDLEN];

    do {
        int rc = ft_read(0x00, buf, RDLEN);
        if (rc < 0) {
            if (raw || once) { printf("rc=%d ERR(%s)\n", rc, strerror(errno)); fflush(stdout); }
            if (once) return 1;
            usleep(POLL_US); continue;
        }
        int xh = buf[3], xl = buf[4], yh = buf[5], yl = buf[6];
        int evt  = xh >> 6;                       /* 0=down 1=up 2=contact 3=none */
        int down = (evt == 0 || evt == 2);
        int x = ((xh & 0x0f) << 8) | xl;
        int y = ((yh & 0x0f) << 8) | yl;

        if (raw || once) {
            printf("#%d rc=%d st=0x%02x evt=%d down=%d x=%d y=%d\n",
                   sample++, rc, buf[2], evt, down, x, y);
            fflush(stdout);
            if (once) break;
            usleep(POLL_US); continue;
        }

        if (down) {
            emit(EV_ABS, ABS_MT_SLOT, 0);
            if (!wasdown) emit(EV_ABS, ABS_MT_TRACKING_ID, next_tid++ & 0xffff);
            emit(EV_ABS, ABS_MT_POSITION_X, x);
            emit(EV_ABS, ABS_MT_POSITION_Y, y);
            if (!wasdown) { emit(EV_KEY, BTN_TOUCH, 1); wasdown = 1; }
            emit(EV_ABS, ABS_X, x);
            emit(EV_ABS, ABS_Y, y);
            emit(EV_SYN, SYN_REPORT, 0);
        } else if (wasdown) {
            emit(EV_ABS, ABS_MT_SLOT, 0);
            emit(EV_ABS, ABS_MT_TRACKING_ID, -1);
            emit(EV_KEY, BTN_TOUCH, 0);
            emit(EV_SYN, SYN_REPORT, 0);
            wasdown = 0;
        }
        usleep(POLL_US);
    } while (running);

    if (ui_fd >= 0) { ioctl(ui_fd, UI_DEV_DESTROY); close(ui_fd); }
    close(i2c_fd);
    return 0;
}
