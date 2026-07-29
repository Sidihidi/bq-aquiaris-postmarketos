p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# helper justo antes de spm_fs_serve
a = "static u32 spm_fs_serve(struct mt6582_spm *s, void __iomem *fs, u32 idx)"
h = """/* H6s: escribe el campo +4 de la respuesta segun spm_fs_p0_mode. */
static void spm_fs_p0(void __iomem *fs, u32 boff, u32 val)
{
	switch (spm_fs_p0_mode) {
	case 1:				/* NO TOCAR: dejar lo que puso el MD */
		return;
	case 2:				/* cero */
		writel(0, fs + boff + 4);
		return;
	case 3:				/* eco del c de la peticion (=1 tipicamente) */
		writel(1, fs + boff + 4);
		return;
	default:
		writel(val, fs + boff + 4);
		return;
	}
}

static u32 spm_fs_serve(struct mt6582_spm *s, void __iomem *fs, u32 idx)"""
if "H6s: escribe el campo +4" in s:
    print("= helper ya aplicado")
else:
    s = s.replace(a, h, 1); print("+ helper spm_fs_p0")

# sustituir las escrituras a +4 dentro de spm_fs_serve
i = s.index("static u32 spm_fs_serve(struct mt6582_spm *s, void __iomem *fs, u32 idx)")
j = s.index("\n}", s.index("marca \"hecho\"", i))
body = s[i:j]
n = 0
for old, new in [
    ("writel(0x00000002, fs + boff + 4);\t/* p0 = drive type (FS_DRIVE_I_SYSTEM) */",
     "spm_fs_p0(fs, boff, 0x00000002);\t/* p0 = drive type (H6s) */"),
    ("writel(2, fs + boff + 4);\t\t/* found */",
     "spm_fs_p0(fs, boff, 2);\t\t\t/* found (H6s) */"),
]:
    if old in body:
        body = body.replace(old, new, 1); n += 1
if n:
    s = s[:i] + body + s[j:]
    print("+ %d escrituras a +4 pasan por el helper" % n)
else:
    print("! no se sustituyo ninguna escritura (revisar)")
open(p, "w").write(s)
