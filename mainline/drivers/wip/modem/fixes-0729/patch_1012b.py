p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
a = "\tdefault:\n\t\t/*\n\t\t * H6y: RESPUESTA MINIMA VALIDA"
n = """\tcase 0x1012: {
\t\t/* H7b: mismo esquema que la respuesta generica pero con su PROPIO
\t\t * contador de campos.  Barrido del kernel #49: el 0x1007 solo acepta 1
\t\t * campo y el 0x1012 rechaza 1 y 4 -> el contador es POR OP. */
\t\tu32 k, nf = (spm_fs_1012_nf == 0xff) ? 1 : spm_fs_1012_nf;

\t\tif (spm_fs_1012_nf != 0xff)
\t\t\tspm_fs_p0(fs, boff, spm_fs_1012_nf);
\t\tlength = 4;
\t\tfor (k = 0; k < nf && k < 6; k++) {
\t\t\twritel(4, fs + boff + 8 + k * 8);
\t\t\twritel(k ? 0 : spm_fs_1010_val, fs + boff + 0xc + k * 8);
\t\t\tlength = 4 + (k + 1) * 8;
\t\t}
\t\tfs_dbg(s->dev, "H7b op=1012 -> nf=%u len=%u\\n", spm_fs_1012_nf, length);
\t\tbreak;
\t}
\tdefault:
\t\t/*
\t\t * H6y: RESPUESTA MINIMA VALIDA"""
if "H7b op=1012" in s:
    print("ya aplicado")
elif a in s:
    open(p, "w").write(s.replace(a, n, 1)); print("+ case 0x1012 insertado")
else:
    print("NO MATCH"); raise SystemExit(1)
