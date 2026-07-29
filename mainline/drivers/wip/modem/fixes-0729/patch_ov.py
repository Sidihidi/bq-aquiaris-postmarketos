p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# 1) params de override generico
a = "static uint spm_fs_1012_nf = 2;"
n = """/*
 * H7c: override GENERICO del contador de campos para UN op cualquiera.
 * Cada op nuevo que aparece pide su propio numero de campos (medido: 1007->1,
 * 1012->3), asi que en vez de un parametro por op, dos parametros que apuntan
 * a cualquiera:
 *   echo 0x1004 > spm_fs_ov_op ; echo 2 > spm_fs_ov_nf
 */
static uint spm_fs_ov_op = 0x1004;
module_param(spm_fs_ov_op, uint, 0644);
MODULE_PARM_DESC(spm_fs_ov_op, "op al que aplicar el override del contador de campos");
static uint spm_fs_ov_nf = 1;
module_param(spm_fs_ov_nf, uint, 0644);
MODULE_PARM_DESC(spm_fs_ov_nf, "campos de la respuesta para spm_fs_ov_op (0xff = no tocar)");

static uint spm_fs_1012_nf = 3;"""
if "H7c: override GENERICO" in s:
    print("= params ya aplicados")
else:
    s = s.replace(a, n, 1); print("+ params de override generico (1012_nf por defecto = 3)")

# 2) quitar el placeholder del 1004 para que caiga al default
old4 = """	case 0x1004: {				/* WRITE: p0/p1 = handle, un word = len */
		/* placeholder: aceptar (la NVRAM del boot es READ; WRITE se afina en HW) */
		length = 0;
		break;
	}
"""
if old4 in s:
    s = s.replace(old4, "", 1); print("+ placeholder del 1004 eliminado (cae al default)")

# 3) el default aplica el override
olddef = "\t\tif (spm_fs_def_nf != 0xff)\n\t\t\tspm_fs_p0(fs, boff, spm_fs_def_nf);\t/* H7a: barrido */"
newdef = """\t\t{
\t\t\tu32 nf = (op == spm_fs_ov_op) ? spm_fs_ov_nf : spm_fs_def_nf;

\t\t\tif (nf != 0xff)
\t\t\t\tspm_fs_p0(fs, boff, nf);\t/* H7a/H7c */
\t\t}"""
if "H7a/H7c" in s:
    print("= default ya aplicado")
elif olddef in s:
    s = s.replace(olddef, newdef, 1); print("+ default aplica el override")
else:
    print("! NO MATCH default")
open(p, "w").write(s)
