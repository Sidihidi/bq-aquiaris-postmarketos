p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = """					fs_dbg(s->dev, "H6 FS REQ idx=%u op=%08x [%08x %08x] path=[%08x %08x %08x]\\n",
						 idx, fs_ops, readl(fs + boff + 4),
						 readl(fs + boff + 8), readl(fs + boff + 12),
						 readl(fs + boff + 16), readl(fs + boff + 20));"""
new = """					fs_dbg(s->dev, "H6 FS REQ idx=%u op=%08x [%08x %08x] path=[%08x %08x %08x]\\n",
						 idx, fs_ops, readl(fs + boff + 4),
						 readl(fs + boff + 8), readl(fs + boff + 12),
						 readl(fs + boff + 16), readl(fs + boff + 20));
					/*
					 * H8h: volcado COMPLETO de la peticion del OPEN.
					 * El handler leia los flags de +4, que es el CONTADOR DE
					 * CAMPOS (vale 2) -> nunca activaba O_CREAT.  Los flags
					 * deben venir en el 2o campo, detras del path:
					 *   +8 = len_0 | +0xc = path | +0xc+align4(len_0) = len_1
					 * Se volcan 14 palabras para localizarlo con datos, no
					 * por deduccion.
					 */
					if ((fs_ops & 0xffff) == 0x1001 && !spm_fs_quiet) {
						char rb[200];
						int rn = 0, rw;

						for (rw = 0; rw < 14 && rn < 190; rw++)
							rn += scnprintf(rb + rn, 200 - rn, "%08x ",
									readl(fs + boff + rw * 4));
						rb[rn] = 0;
						dev_info(s->dev, "H8h OPEN req: %s\\n", rb);
					}"""
if "H8h: volcado COMPLETO" in s:
    print("ya aplicado")
elif old in s:
    open(p,"w").write(s.replace(old,new,1)); print("+ volcado completo de la peticion del OPEN")
else:
    print("NO MATCH"); raise SystemExit(1)
