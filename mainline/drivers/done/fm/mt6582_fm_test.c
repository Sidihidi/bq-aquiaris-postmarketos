// mt6582_fm_test.c — Módulo de test mínimo del FM del CONSYS (MT6627).
// Objetivo (M0): validar que func_on(FM) + STP canal 1 funcionan, ANTES de
// portear el stock completo de 12K LOC. Si esto funciona, el transporte está
// listo y el port del stock es solo fricción de Kbuild.
//
// Hace: func_on(FM), envía un comando de POWERUP del FM por STP, y loguea
// cualquier respuesta que llegue por el canal 1. No registra /dev/fm (eso
// lo hará el stock).

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

extern int mt6582_consys_func_on(u8 type);
extern int mt6582_consys_func_off(u8 type);
extern int mt6582_stp_fm_send(const u8 *data, u32 len);
extern int mt6582_stp_fm_register_rx(void (*cb)(const u8 *data, u32 len));

static struct dentry *fm_dbg_dir;
static char fm_rx_log[4096];
static size_t fm_rx_len;
static DEFINE_SPINLOCK(fm_log_lock);

static void fm_test_rx_cb(const u8 *data, u32 len)
{
	u32 i, n;
	spin_lock(&fm_log_lock);
	n = min_t(u32, len, sizeof(fm_rx_log) - fm_rx_len - 1);
	for (i = 0; i < n; i++)
		fm_rx_len += scnprintf(fm_rx_log + fm_rx_len,
				       sizeof(fm_rx_log) - fm_rx_len,
				       "%02x ", data[i]);
	fm_rx_len += scnprintf(fm_rx_log + fm_rx_len,
			       sizeof(fm_rx_log) - fm_rx_len, "\n");
	spin_unlock(&fm_log_lock);
	pr_info("mt6582_fm_test: RX %d bytes: %*ph\n", len, min_t(u32, len, 16), data);
}

/* Comando FM POWERUP del MT6627 (del stock mt6627_fm_lib.c / mt6627_fm_cmd.c).
 * Es el primer comando para encender el radio FM. Formato: opcode + params.
 * Opcode 0x01 = FM_POWERUP_CMD. */
static const u8 FM_POWERUP_CMD[] = {
	0x01,       /* opcode: POWERUP */
	0x00, 0x02, /* band: 0x0002 (US/Europe) */
	0x00, 0xD0, /* vol */
	0x00, 0x32, /* audio gain */
	0x00, 0x01, /* rds on */
};

static ssize_t fm_powerup_write(struct file *f, const char __user *u, size_t n, loff_t *o)
{
	int ret;

	pr_info("mt6582_fm_test: func_on(FM)...\n");
	ret = mt6582_consys_func_on(1); /* WMTDRV_TYPE_FM = 1 */
	if (ret) {
		pr_err("mt6582_fm_test: func_on(FM) failed: %d\n", ret);
		return ret;
	}
	pr_info("mt6582_fm_test: func_on(FM) OK, enviando POWERUP por STP-1\n");

	ret = mt6582_stp_fm_send(FM_POWERUP_CMD, sizeof(FM_POWERUP_CMD));
	pr_info("mt6582_fm_test: POWERUP enviado (%d bytes)\n", ret);
	return n;
}

static ssize_t fm_rx_read(struct file *f, char __user *u, size_t n, loff_t *o)
{
	size_t len;

	spin_lock(&fm_log_lock);
	len = min_t(size_t, n, fm_rx_len);
	if (copy_to_user(u, fm_rx_log, len)) {
		spin_unlock(&fm_log_lock);
		return -EFAULT;
	}
	fm_rx_len = 0; /* consume */
	spin_unlock(&fm_log_lock);
	return len;
}

static const struct file_operations fm_powerup_fops = {
	.owner = THIS_MODULE,
	.write = fm_powerup_write,
};

static const struct file_operations fm_rx_fops = {
	.owner = THIS_MODULE,
	.read = fm_rx_read,
};

static int __init mt6582_fm_test_init(void)
{
	fm_dbg_dir = debugfs_create_dir("mt6582_fm_test", NULL);
	debugfs_create_file("powerup", 0222, fm_dbg_dir, NULL, &fm_powerup_fops);
	debugfs_create_file("rx", 0444, fm_dbg_dir, NULL, &fm_rx_fops);

	/* Registrar callback de RX del canal FM */
	mt6582_stp_fm_register_rx(fm_test_rx_cb);

	pr_info("mt6582_fm_test: cargado. Usa: echo 1 > /sys/kernel/debug/mt6582_fm_test/powerup\n");
	return 0;
}

static void __exit mt6582_fm_test_exit(void)
{
	mt6582_stp_fm_register_rx(NULL);
	debugfs_remove_recursive(fm_dbg_dir);
	mt6582_consys_func_off(1);
	pr_info("mt6582_fm_test: descargado\n");
}

module_init(mt6582_fm_test_init);
module_exit(mt6582_fm_test_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MT6582 FM test (CONSYS MT6627) — M0 validación de transporte");
