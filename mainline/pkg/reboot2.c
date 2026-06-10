#include <unistd.h>
#include <sys/syscall.h>
#include <linux/reboot.h>
#include <stdio.h>
int main(int argc, char **argv) {
    const char *arg = argc > 1 ? argv[1] : "bootloader";
    sync();
    long r = syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
                     LINUX_REBOOT_CMD_RESTART2, arg);
    perror("reboot2");
    return (int)r;
}
