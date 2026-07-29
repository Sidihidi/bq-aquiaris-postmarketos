p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
a = "static uint spm_fs_def_nf = 1;"
n = """/*
 * H7b: contador de campos PROPIO del op 0x1012.
 * Barrido del kernel #49: el 0x1007 SOLO acepta 1 campo y el 0x1012 rechaza
 * tanto 1 como 4 -> el contador es POR OP, no global.
 */
static uint spm_fs_1012_nf = 2;
module_param(spm_fs_1012_nf, uint, 0644);
MODULE_PARM_DESC(spm_fs_1012_nf, "campos de la respuesta del op 0x1012 (0xff = no tocar)");

static uint spm_fs_def_nf = 1;"""
if "spm_fs_1012_nf = 2;" in s:
    print("ya declarado")
else:
    open(p, "w").write(s.replace(a, n, 1)); print("+ declaracion anadida")
