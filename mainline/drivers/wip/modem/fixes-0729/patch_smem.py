p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

anchor = """static int spm_md_dump_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_dump(gspm);
}"""

new = """/*
 * H6p: volcado de la MEMORIA COMPARTIDA del MD, buscando texto.
 *
 * HECHO (0729): el experimento de control H6o (modo 8, no contestar nada al
 * 0x1010) da EXACTAMENTE el mismo resultado que contestar: handshake OK, 2 ops
 * FS (100e + 1010) y silencio total (RCHNUM=0 BUSY=0 tras 8s).  O sea que
 * NUESTRA RESPUESTA NO ES EL DISPARADOR.  Ademas nuestra respuesta al GetDrive
 * es identica byte a byte a la del ccci_fsd real ([02][04][00][54]).
 *
 * Y el ground-truth (gd-boot-full.out) demuestra que el MD de Lineage pide EL
 * MISMO fichero (Z:\\FAT3149C88D.log, op 1010) y SIGUE ADELANTE al 1001 OPEN
 * "Z:\\NVRAM" 458us despues.  El nuestro se para justo ahi.
 *
 * Falta saber si el MD esta EXCEPCIONADO o simplemente ESPERANDO algo.  MOLY
 * escribe sus excepciones en la memoria compartida con texto legible (nombre de
 * fichero del assert, motivo...).  /dev/mem esta bloqueado por STRICT_DEVMEM,
 * asi que lo volcamos desde el kernel: se barre la SMEM buscando cadenas ASCII
 * imprimibles de >=8 caracteres.  Si aparece un assert de MOLY, dice
 * EXACTAMENTE de que se queja.
 *
 *   echo 1 > /sys/module/mt6582_spm/parameters/spm_md_smem
 */
#define SPM_SMEM_SCAN	0x200000	/* barrer los primeros 2MB */
#define SPM_SMEM_MINRUN	8		/* longitud minima de cadena */
#define SPM_SMEM_MAXHIT	48		/* tope de cadenas impresas */

static int spm_md_smem_scan(struct mt6582_spm *s)
{
	void __iomem *sm;
	char buf[96];
	u32 off, run = 0;
	int hits = 0;

	sm = ioremap(MD_SMEM_PHYS, SPM_SMEM_SCAN);
	if (!sm) {
		dev_err(s->dev, "H6p: no pude mapear la SMEM\\n");
		return -ENOMEM;
	}

	dev_info(s->dev, "H6p SMEM @0x%08x, primeros 64B:\\n", MD_SMEM_PHYS);
	for (off = 0; off < 64; off += 16)
		dev_info(s->dev, "  +%04x: %08x %08x %08x %08x\\n", off,
			 readl(sm + off), readl(sm + off + 4),
			 readl(sm + off + 8), readl(sm + off + 12));

	dev_info(s->dev, "H6p: buscando texto en %d KB...\\n", SPM_SMEM_SCAN / 1024);
	for (off = 0; off < SPM_SMEM_SCAN && hits < SPM_SMEM_MAXHIT; off++) {
		u8 c = readb(sm + off);

		if (c >= 0x20 && c < 0x7f) {
			if (run < sizeof(buf) - 1)
				buf[run] = c;
			run++;
			continue;
		}
		if (run >= SPM_SMEM_MINRUN) {
			buf[min_t(u32, run, sizeof(buf) - 1)] = 0;
			dev_info(s->dev, "H6p +0x%06x: %s\\n", off - run, buf);
			hits++;
		}
		run = 0;
	}
	dev_info(s->dev, "H6p: %d cadenas encontradas\\n", hits);

	iounmap(sm);
	return 0;
}

static int spm_md_smem_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_smem_scan(gspm);
}
static const struct kernel_param_ops spm_md_smem_ops = { .set = spm_md_smem_set };
module_param_cb(spm_md_smem, &spm_md_smem_ops, NULL, 0200);
MODULE_PARM_DESC(spm_md_smem, "vuelca/busca texto en la memoria compartida del MD (excepciones de MOLY)");

static int spm_md_dump_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_dump(gspm);
}"""

if "H6p: volcado de la MEMORIA COMPARTIDA" in s:
    print("ya aplicado")
elif anchor in s:
    open(p, "w").write(s.replace(anchor, new, 1))
    print("H6p: volcado de SMEM anadido")
else:
    print("NO MATCH"); raise SystemExit(1)
