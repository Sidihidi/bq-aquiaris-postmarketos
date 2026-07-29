p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# parametro
a = "static uint spm_fs_1010_val;"
n = """/*
 * H7a: numero de campos de la respuesta generica (ops sin handler).
 * El 0x1012 asserta en la linea 528 con valor 1 leido -> espera otro contador.
 * Como 0x1012 no existe en el arranque real, se barre: 0,1,2,3.
 * 0xff = no tocar +4 (dejar el de la peticion).
 */
static uint spm_fs_def_nf = 1;
module_param(spm_fs_def_nf, uint, 0644);
MODULE_PARM_DESC(spm_fs_def_nf, "campos de la respuesta generica (0..3; 0xff = no tocar)");

static uint spm_fs_1010_val;"""
if "H7a: numero de campos" in s:
    print("= param ya aplicado")
else:
    s = s.replace(a, n, 1); print("+ param spm_fs_def_nf")

# usarlo en el default
old = """		spm_fs_p0(fs, boff, 1);			/* 1 campo */
		writel(4, fs + boff + 8);		/* len_0 = 4 */
		writel(spm_fs_1010_val, fs + boff + 0xc);
		length = 12;
		fs_dbg(s->dev, "H6y op=%x sin handler -> respuesta minima\\n", op);"""
new = """		if (spm_fs_def_nf != 0xff)
			spm_fs_p0(fs, boff, spm_fs_def_nf);	/* H7a: barrido */
		writel(4, fs + boff + 8);		/* len_0 = 4 */
		writel(spm_fs_1010_val, fs + boff + 0xc);
		if (spm_fs_def_nf >= 2) {		/* 2o campo, tambien de 4 bytes */
			writel(4, fs + boff + 0x10);
			writel(0, fs + boff + 0x14);
			length = 0x14;
		} else {
			length = 12;
		}
		if (spm_fs_def_nf >= 3) {
			writel(4, fs + boff + 0x18);
			writel(0, fs + boff + 0x1c);
			length = 0x1c;
		}
		fs_dbg(s->dev, "H6y op=%x sin handler -> minima (nf=%u)\\n", op, spm_fs_def_nf);"""
if "H7a: barrido" in s:
    print("= default ya aplicado")
elif old in s:
    s = s.replace(old, new, 1); print("+ default con nf conmutable")
else:
    print("! NO MATCH default"); raise SystemExit(1)
open(p, "w").write(s)
