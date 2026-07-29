p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

old = """static int spm_md_smem_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_smem_scan(gspm);
}"""

new = """/*
 * H6q: volcado hex+ASCII de una zona concreta de la SMEM.
 *
 * HALLAZGO (0729, kernel #38): el barrido de texto encontro "fs_ccci.c" en
 * +0x8d8 -> firma de un ASSERT de MOLY (escribe fichero y linea al petar).  O
 * sea que el MD SI EXCEPCIONA, y lo hace en su capa de FS/CCCI.  Falta leer el
 * registro entero (linea, codigo, contexto), que vive alrededor de +0x800.
 *
 *   echo 1   > .../spm_md_smem    -> barrido de cadenas (como antes)
 *   echo 800 > .../spm_md_smem    -> volcado hex+ASCII de 512B desde +0x800
 */
static int spm_md_smem_hex(struct mt6582_spm *s, u32 off)
{
	void __iomem *sm;
	u32 i, j;

	off &= ~0xFu;
	sm = ioremap(MD_SMEM_PHYS + off, 0x200);
	if (!sm) {
		dev_err(s->dev, "H6q: no pude mapear SMEM+0x%x\\n", off);
		return -ENOMEM;
	}
	dev_info(s->dev, "H6q SMEM +0x%06x (512B):\\n", off);
	for (i = 0; i < 0x200; i += 16) {
		char asc[17];

		for (j = 0; j < 16; j++) {
			u8 c = readb(sm + i + j);

			asc[j] = (c >= 0x20 && c < 0x7f) ? c : '.';
		}
		asc[16] = 0;
		dev_info(s->dev, "  +%04x: %08x %08x %08x %08x  |%s|\\n", off + i,
			 readl(sm + i), readl(sm + i + 4),
			 readl(sm + i + 8), readl(sm + i + 12), asc);
	}
	iounmap(sm);
	return 0;
}

static int spm_md_smem_set(const char *val, const struct kernel_param *kp)
{
	u32 off = 0;

	if (!gspm)
		return -ENODEV;
	if (val)
		kstrtou32(strim((char *)val), 16, &off);	/* el valor se lee en HEX */
	if (off >= 0x100)
		return spm_md_smem_hex(gspm, off);
	return spm_md_smem_scan(gspm);
}"""

if "H6q: volcado hex+ASCII" in s:
    print("ya aplicado")
elif old in s:
    s = s.replace(old, new, 1)
    # cadenas mas cortas: capturar cosas tipo "EXCP", "ASSERT"
    s = s.replace("#define SPM_SMEM_MINRUN\t8", "#define SPM_SMEM_MINRUN\t5", 1)
    s = s.replace("#define SPM_SMEM_MAXHIT\t48", "#define SPM_SMEM_MAXHIT\t80", 1)
    open(p, "w").write(s)
    print("H6q: visor hex + cadenas mas cortas (MINRUN 5)")
else:
    print("NO MATCH"); raise SystemExit(1)
