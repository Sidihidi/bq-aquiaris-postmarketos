p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# ---- 1) limpiar el registro de excepcion de la SMEM antes de cada ciclo ----
anchor = "static int spm_md_smem_scan(struct mt6582_spm *s)"
new = """/*
 * H6r: LIMPIAR el area de excepcion de la SMEM.
 *
 * FALLO DE METODO detectado el 0729: la SMEM NO se limpia entre ciclos, asi que
 * el registro de excepcion que leiamos podia ser el de la corrida ANTERIOR.  Por
 * eso el modo 8 (sin responder) parecia dar el mismo registro que el modo 0: era
 * el mismo registro, rancio.  Sin esto ninguna comparacion vale.
 */
static int spm_md_smem_clear(struct mt6582_spm *s)
{
	void __iomem *sm;
	u32 off;

	sm = ioremap(MD_SMEM_PHYS + 0x800, 0x200);
	if (!sm)
		return -ENOMEM;
	for (off = 0; off < 0x200; off += 4)
		writel(0, sm + off);
	wmb();
	iounmap(sm);
	dev_info(s->dev, "H6r: area de excepcion de la SMEM (+0x800..+0xa00) limpiada\\n");
	return 0;
}

static int spm_md_smem_scan(struct mt6582_spm *s)"""
if "H6r: LIMPIAR el area de excepcion" in s:
    print("  = clear: ya aplicado")
elif anchor in s:
    s = s.replace(anchor, new, 1); print("  + spm_md_smem_clear")
else:
    print("  ! NO MATCH clear"); raise SystemExit(1)

# ---- 2) llamarlo desde el power-on (arranque limpio de cada ciclo) ----
a2 = "static int spm_md_dump_set(const char *val, const struct kernel_param *kp)"
n2 = """static int spm_md_smem_clr_set(const char *val, const struct kernel_param *kp)
{
	if (!gspm)
		return -ENODEV;
	return spm_md_smem_clear(gspm);
}
static const struct kernel_param_ops spm_md_smem_clr_ops = { .set = spm_md_smem_clr_set };
module_param_cb(spm_md_smem_clr, &spm_md_smem_clr_ops, NULL, 0200);
MODULE_PARM_DESC(spm_md_smem_clr, "H6r: limpia el registro de excepcion de la SMEM (hacerlo ANTES de cada ciclo)");

static int spm_md_dump_set(const char *val, const struct kernel_param *kp)"""
if "spm_md_smem_clr" in s:
    print("  = param clear: ya aplicado")
else:
    s = s.replace(a2, n2, 1); print("  + param spm_md_smem_clr")

# ---- 3) control de lo que escribimos en +4 (la hipotesis) ----
a3 = "static uint spm_fs_1010_mode;"
n3 = """/*
 * H6s: que escribimos en el campo +4 de la respuesta.
 *
 * HIPOTESIS: el MD valida el buffer de respuesta con DOS comprobaciones
 * (fs_ccci.c, desensamblado):
 *    linea 520:  buffer[+0] == 0xffff0000|op     <- ESTA LA PASAMOS
 *    linea 528:  buffer[+4] == r6                <- AQUI PETA (leido=2)
 * Nosotros escribimos 2 en +4 (drive-type en el GetDrive, "found" en el 0x1010).
 * El MD pone 1 ahi en su peticion (campo c).  Puede que espere que NO se toque.
 *   0 = comportamiento actual
 *   1 = NO TOCAR +4 (dejar el valor que puso el MD)   <- candidato principal
 *   2 = escribir 0
 *   3 = escribir 1 (eco del c de la peticion)
 */
static uint spm_fs_p0_mode;
module_param(spm_fs_p0_mode, uint, 0644);
MODULE_PARM_DESC(spm_fs_p0_mode, "campo +4 de la respuesta: 0=actual 1=no-tocar 2=cero 3=eco del c");

static uint spm_fs_1010_mode;"""
if "spm_fs_p0_mode" in s:
    print("  = p0_mode: ya aplicado")
else:
    s = s.replace(a3, n3, 1); print("  + param spm_fs_p0_mode")

open(p, "w").write(s)
