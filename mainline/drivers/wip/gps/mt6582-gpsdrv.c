// SPDX-License-Identifier: GPL-2.0
/*
 * mt6582-gpsdrv — port mainline del `gps.c` (mt3326_gps) del downstream MTK.
 *
 * Provee la interfaz que el `mnld` stock (bajo bionic) EXIGE para arrancar una
 * sesión GPS, y que en mainline faltaba (por eso mnld se quedaba idle):
 *   - /sys/class/gpsdrv/gps/{pwrctl,suspend,status,state,pwrsave,rdelay}
 *   - chardev /dev/gps
 *
 * Semántica IDÉNTICA al downstream (contra el que se escribió el `libmnl` cerrado):
 *   - pwrctl/state/pwrsave/suspend/rdelay = "echo-registers" (mnld escribe un valor,
 *     lo lee de vuelta). pwrctl RST(2) deja pwrctl=ON(1). status = log de reinicios.
 *   - /dev/gps = buffer loopback en memoria (write->buf->read); en el downstream
 *     TAMPOCO va al STP (el dato real del DSP va por /dev/stpgps).
 *   - El "power" real del combo va por WMT/STP (ya lo hace el open de /dev/stpgps al
 *     boot: func_on[GPS]) -> aquí mt6582_gps_power es un stub.
 *
 * Driver virtual (sin HW propio): se crea en module_init, sin nodo DT.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/sched/signal.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define GPS_DEVNAME "mt3326-gps"

enum { GPS_PWRCTL_UNSUPPORTED = 0xFF, GPS_PWRCTL_OFF = 0, GPS_PWRCTL_ON = 1,
       GPS_PWRCTL_RST = 2, GPS_PWRCTL_OFF_FORCE = 3, GPS_PWRCTL_RST_FORCE = 4,
       GPS_PWRCTL_MAX = 5 };
enum { GPS_PWR_RESUME = 0, GPS_PWR_SUSPEND = 1 };
enum { GPS_STATE_UNSUPPORTED = 0xFF, GPS_STATE_OFF = 0, GPS_STATE_INIT = 1,
       GPS_STATE_START = 2, GPS_STATE_STOP = 3, GPS_STATE_DEC_FREQ = 4,
       GPS_STATE_SLEEP = 5, GPS_STATE_MAX = 6 };
enum { GPS_PWRSAVE_UNSUPPORTED = 0xFF, GPS_PWRSAVE_DEC_FREQ = 0,
       GPS_PWRSAVE_SLEEP = 1, GPS_PWRSAVE_OFF = 2, GPS_PWRSAVE_MAX = 3 };

struct gps_data {
	int dat_len, dat_pos;
	char dat_buf[4096];
	struct semaphore sem;
	wait_queue_head_t read_wait;
};

struct gps_sta_itm {
	u8 year, month, day, hour, minute, sec, count, reason;
};
struct gps_sta_obj { int index; struct gps_sta_itm items[32]; };

struct gps_drv_obj {
	u8 pwrctl, suspend, state, pwrsave;
	int rdelay;
	struct kobject *kobj;
	struct mutex sem;
	struct gps_sta_obj status;
};

static struct gps_data gps_private;
static struct gps_dev { struct class *cls; struct device *dev; dev_t devno; struct cdev chdev; } g_dev;
static struct gps_drv_obj *g_obj;
static char * const str_reason[] = { "none", "init", "monitor", "wakeup", "TTFF", "force", "unknown" };

/* power del combo = WMT/STP (ya ON por /dev/stpgps); aquí no-op */
static void mt6582_gps_power(unsigned int on, unsigned int force)
{
	pr_debug("gpsdrv: power %s (force=%u) [no-op: combo via STP]\n", on ? "on" : "off", force);
}
static void mt6582_gps_reset(int delay, int force)
{
	mt6582_gps_power(1, 0); mdelay(delay);
	mt6582_gps_power(0, force); mdelay(delay);
	mt6582_gps_power(1, 0);
}

/* ---- set_* (fieles al downstream) ---- */
static int gps_set_pwrctl(struct gps_drv_obj *o, u8 pwrctl)
{
	int err = 0;
	mutex_lock(&o->sem);
	if (pwrctl == GPS_PWRCTL_ON || pwrctl == GPS_PWRCTL_OFF) {
		o->pwrctl = pwrctl; mt6582_gps_power(pwrctl, 0);
	} else if (pwrctl == GPS_PWRCTL_OFF_FORCE) {
		o->pwrctl = pwrctl; mt6582_gps_power(0, 1);
	} else if (pwrctl == GPS_PWRCTL_RST) {
		mt6582_gps_reset(o->rdelay, 0); o->pwrctl = GPS_PWRCTL_ON;
	} else if (pwrctl == GPS_PWRCTL_RST_FORCE) {
		mt6582_gps_reset(o->rdelay, 1); o->pwrctl = GPS_PWRCTL_ON;
	} else {
		err = -1;
	}
	mutex_unlock(&o->sem);
	return err;
}
static int gps_set_state(struct gps_drv_obj *o, u8 state)
{
	int err = 0;
	mutex_lock(&o->sem);
	if (state < GPS_STATE_MAX) o->state = state; else err = -1;
	mutex_unlock(&o->sem);
	return err;
}
static int gps_set_pwrsave(struct gps_drv_obj *o, u8 pwrsave)
{
	int err = 0;
	mutex_lock(&o->sem);
	if (pwrsave < GPS_PWRSAVE_MAX) o->pwrsave = pwrsave; else err = -1;
	mutex_unlock(&o->sem);
	return err;
}
static int gps_set_suspend(struct gps_drv_obj *o, u8 suspend)
{
	mutex_lock(&o->sem);
	if (o->suspend != suspend)
		sysfs_notify(o->kobj, NULL, "suspend");
	o->suspend = suspend;
	mutex_unlock(&o->sem);
	return 0;
}
static int gps_set_status(struct gps_drv_obj *o, const char *buf, size_t count)
{
	int year, mon, day, hour, minute, sec, cnt, reason, idx, num;
	int err = 0;
	mutex_lock(&o->sem);
	if (sscanf(buf, "(%d/%d/%d %d:%d:%d) - %d/%d", &year, &mon, &day,
		   &hour, &minute, &sec, &cnt, &reason) == 8) {
		num = ARRAY_SIZE(o->status.items);
		idx = o->status.index % num;
		o->status.items[idx] = (struct gps_sta_itm){ year, mon, day, hour,
			minute, sec, cnt, reason };
		o->status.index++;
	} else {
		err = -1;
	}
	mutex_unlock(&o->sem);
	return err;
}

/* ---- sysfs show/store ---- */
#define OBJ(dev) ((struct gps_drv_obj *)dev_get_drvdata(dev))
static ssize_t pwrctl_show(struct device *d, struct device_attribute *a, char *b)
{ struct gps_drv_obj *o = OBJ(d); ssize_t r; mutex_lock(&o->sem); r = sysfs_emit(b, "%d\n", o->pwrctl); mutex_unlock(&o->sem); return r; }
static ssize_t pwrctl_store(struct device *d, struct device_attribute *a, const char *b, size_t c)
{ struct gps_drv_obj *o = OBJ(d); if (c == 1 || (c == 2 && b[1] == '\n')) gps_set_pwrctl(o, b[0] - '0'); return c; }

static ssize_t suspend_show(struct device *d, struct device_attribute *a, char *b)
{ struct gps_drv_obj *o = OBJ(d); ssize_t r; mutex_lock(&o->sem); r = sysfs_emit(b, "%d\n", o->suspend); mutex_unlock(&o->sem); return r; }
static ssize_t suspend_store(struct device *d, struct device_attribute *a, const char *b, size_t c)
{ struct gps_drv_obj *o = OBJ(d); if (c == 1 || (c == 2 && b[1] == '\n')) gps_set_suspend(o, b[0] - '0'); return c; }

static ssize_t state_show(struct device *d, struct device_attribute *a, char *b)
{ struct gps_drv_obj *o = OBJ(d); ssize_t r; mutex_lock(&o->sem); r = sysfs_emit(b, "%d\n", o->state); mutex_unlock(&o->sem); return r; }
static ssize_t state_store(struct device *d, struct device_attribute *a, const char *b, size_t c)
{ struct gps_drv_obj *o = OBJ(d); if (c == 1 || (c == 2 && b[1] == '\n')) gps_set_state(o, b[0] - '0'); return c; }

static ssize_t pwrsave_show(struct device *d, struct device_attribute *a, char *b)
{ struct gps_drv_obj *o = OBJ(d); ssize_t r; mutex_lock(&o->sem); r = sysfs_emit(b, "%d\n", o->pwrsave); mutex_unlock(&o->sem); return r; }
static ssize_t pwrsave_store(struct device *d, struct device_attribute *a, const char *b, size_t c)
{ struct gps_drv_obj *o = OBJ(d); if (c == 1 || (c == 2 && b[1] == '\n')) gps_set_pwrsave(o, b[0] - '0'); return c; }

static ssize_t rdelay_show(struct device *d, struct device_attribute *a, char *b)
{ struct gps_drv_obj *o = OBJ(d); ssize_t r; mutex_lock(&o->sem); r = sysfs_emit(b, "%d\n", o->rdelay); mutex_unlock(&o->sem); return r; }
static ssize_t rdelay_store(struct device *d, struct device_attribute *a, const char *b, size_t c)
{ struct gps_drv_obj *o = OBJ(d); int v; if (kstrtoint(b, 10, &v) == 0) { mutex_lock(&o->sem); o->rdelay = v; mutex_unlock(&o->sem); } return c; }

static ssize_t status_show(struct device *d, struct device_attribute *a, char *b)
{
	struct gps_drv_obj *o = OBJ(d);
	int idx, num, len = 0, cnt;
	char *reason;
	mutex_lock(&o->sem);
	num = ARRAY_SIZE(o->status.items);
	for (idx = 0; idx < num; idx++) {
		struct gps_sta_itm *it = &o->status.items[idx];
		if (it->month == 0) continue;
		reason = str_reason[min_t(int, it->reason, ARRAY_SIZE(str_reason) - 1)];
		cnt = scnprintf(b + len, PAGE_SIZE - len,
				"[%d] %.4d/%.2d/%.2d %.2d:%.2d:%.2d - %d, %s\n", idx,
				it->year + 1900, it->month, it->day, it->hour,
				it->minute, it->sec, it->count, reason);
		len += cnt;
	}
	mutex_unlock(&o->sem);
	return len;
}
static ssize_t status_store(struct device *d, struct device_attribute *a, const char *b, size_t c)
{ gps_set_status(OBJ(d), b, c); return c; }

static DEVICE_ATTR_RW(pwrctl);
static DEVICE_ATTR_RW(suspend);
static DEVICE_ATTR_RW(status);
static DEVICE_ATTR_RW(state);
static DEVICE_ATTR_RW(pwrsave);
static DEVICE_ATTR_RW(rdelay);
static struct device_attribute *gps_attr_list[] = {
	&dev_attr_pwrctl, &dev_attr_suspend, &dev_attr_status,
	&dev_attr_state, &dev_attr_pwrsave, &dev_attr_rdelay,
};

/* ---- chardev /dev/gps (loopback, fiel al downstream) ---- */
static int gps_open(struct inode *i, struct file *f)
{ f->private_data = &gps_private; return nonseekable_open(i, f); }
static int gps_release(struct inode *i, struct file *f) { f->private_data = NULL; return 0; }
static long gps_ioctl(struct file *f, unsigned int cmd, unsigned long arg) { return -ENOIOCTLCMD; }

static ssize_t gps_read(struct file *f, char __user *buf, size_t count, loff_t *ppos)
{
	struct gps_data *dev = f->private_data;
	ssize_t ret; int copy_len;
	if (!dev) return -EINVAL;
	if (signal_pending(current)) return -ERESTARTSYS;
	if (down_interruptible(&dev->sem)) return -ERESTARTSYS;
	if (dev->dat_len == 0) {
		up(&dev->sem);
		if (f->f_flags & O_NONBLOCK) return -EAGAIN;
		ret = wait_event_interruptible(dev->read_wait, dev->dat_len > 0);
		if (ret) return ret;
		if (down_interruptible(&dev->sem)) return -ERESTARTSYS;
	}
	copy_len = min_t(int, dev->dat_len, count);
	if (copy_to_user(buf, dev->dat_buf + dev->dat_pos, copy_len)) {
		ret = -EFAULT;
	} else {
		if (dev->dat_len > copy_len + dev->dat_pos)
			dev->dat_pos += copy_len;
		else { dev->dat_len = 0; dev->dat_pos = 0; }
		ret = copy_len;
	}
	up(&dev->sem);
	return ret;
}
static ssize_t gps_write(struct file *f, const char __user *buf, size_t count, loff_t *ppos)
{
	struct gps_data *dev = f->private_data;
	size_t copy_size;
	if (!dev) return -EINVAL;
	if (!count) return 0;
	if (signal_pending(current)) return -ERESTARTSYS;
	if (down_interruptible(&dev->sem)) return -ERESTARTSYS;
	copy_size = min(count, sizeof(dev->dat_buf));
	if (copy_from_user(dev->dat_buf, buf, copy_size)) { up(&dev->sem); return -EFAULT; }
	dev->dat_len = copy_size; dev->dat_pos = 0;
	up(&dev->sem);
	wake_up_interruptible(&dev->read_wait);
	return copy_size;
}
static __poll_t gps_poll(struct file *f, poll_table *wait)
{
	struct gps_data *dev = f->private_data;
	__poll_t mask;
	if (!dev) return 0;
	down(&dev->sem);
	poll_wait(f, &dev->read_wait, wait);
	mask = EPOLLOUT | EPOLLWRNORM;
	if (dev->dat_len != 0) mask |= EPOLLIN | EPOLLRDNORM;
	up(&dev->sem);
	return mask;
}
static const struct file_operations gps_fops = {
	.owner = THIS_MODULE, .open = gps_open, .release = gps_release,
	.read = gps_read, .write = gps_write, .poll = gps_poll,
	.unlocked_ioctl = gps_ioctl,
};

/* ---- init/exit ---- */
static int __init mt6582_gpsdrv_init(void)
{
	int err, i;

	sema_init(&gps_private.sem, 1);
	init_waitqueue_head(&gps_private.read_wait);

	g_obj = kzalloc(sizeof(*g_obj), GFP_KERNEL);
	if (!g_obj) return -ENOMEM;
	g_obj->pwrctl = GPS_PWRCTL_OFF;
	g_obj->suspend = GPS_PWR_RESUME;
	g_obj->state = GPS_STATE_UNSUPPORTED;
	g_obj->pwrsave = GPS_PWRSAVE_UNSUPPORTED;
	g_obj->rdelay = 50;
	mutex_init(&g_obj->sem);

	err = alloc_chrdev_region(&g_dev.devno, 0, 1, GPS_DEVNAME);
	if (err) goto err_obj;
	cdev_init(&g_dev.chdev, &gps_fops);
	g_dev.chdev.owner = THIS_MODULE;
	err = cdev_add(&g_dev.chdev, g_dev.devno, 1);
	if (err) goto err_region;

	g_dev.cls = class_create("gpsdrv");
	if (IS_ERR(g_dev.cls)) { err = PTR_ERR(g_dev.cls); goto err_cdev; }
	g_dev.dev = device_create(g_dev.cls, NULL, g_dev.devno, g_obj, "gps");
	if (IS_ERR(g_dev.dev)) { err = PTR_ERR(g_dev.dev); goto err_class; }
	g_obj->kobj = &g_dev.dev->kobj;

	for (i = 0; i < ARRAY_SIZE(gps_attr_list); i++)
		device_create_file(g_dev.dev, gps_attr_list[i]);

	pr_info("mt6582-gpsdrv: /sys/class/gpsdrv/gps + /dev/gps listos (major %d)\n",
		MAJOR(g_dev.devno));
	return 0;

err_class:
	class_destroy(g_dev.cls);
err_cdev:
	cdev_del(&g_dev.chdev);
err_region:
	unregister_chrdev_region(g_dev.devno, 1);
err_obj:
	kfree(g_obj);
	return err;
}
static void __exit mt6582_gpsdrv_exit(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(gps_attr_list); i++)
		device_remove_file(g_dev.dev, gps_attr_list[i]);
	device_destroy(g_dev.cls, g_dev.devno);
	class_destroy(g_dev.cls);
	cdev_del(&g_dev.chdev);
	unregister_chrdev_region(g_dev.devno, 1);
	kfree(g_obj);
}
module_init(mt6582_gpsdrv_init);
module_exit(mt6582_gpsdrv_exit);
MODULE_DESCRIPTION("MT6582 GPS driver (gpsdrv sysfs + /dev/gps) — port de mt3326_gps para mnld");
MODULE_LICENSE("GPL");
