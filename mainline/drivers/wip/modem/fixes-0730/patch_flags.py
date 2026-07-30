p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = """		char path[160];
		struct file *f;
		int h, oflag = O_RDWR;
		/* mapeo flags FS->oflag (Ghidra H6d) */
		if (p0 & 0x10000)  oflag = O_RDWR | O_CREAT;
		if (p0 & 0x20000)  oflag = O_RDWR | O_CREAT | O_TRUNC;"""
new = """		char path[160];
		struct file *f;
		int h, oflag = O_RDWR;
		u32 fl = 0;
		/*
		 * H8i: los FLAGS del OPEN vienen en el SEGUNDO CAMPO, detras del path
		 * — no en +4, que es el CONTADOR DE CAMPOS (vale 2 y por eso los bits
		 * nunca se activaban: jamas pasabamos O_CREAT).
		 *
		 * Medido 0730 volcando la peticion entera (H8h):
		 *   w0=00001001 w1=00000002 w2=0000001a  <- op, nfields, len_0
		 *   w3..w9 = "X:\\\\MP0D_000"             (26 bytes, align4 -> 28)
		 *   w10=00000004                        <- len_1
		 *   w11=01010400                        <- FLAGS  (bit 0x10000 = crear)
		 * Asi que el offset depende de la longitud del path:
		 *   flags_off = 0xc + align4(len_0) + 4
		 * Las constantes del mapeo (0x10000 / 0x20000) ya eran correctas; lo
		 * que estaba mal era de donde se leia el valor.
		 */
		{
			u32 plen0 = readl(fs + boff + 8);
			u32 foff = 0xc + ((plen0 + 3) & ~3u);

			if (plen0 <= 144 && readl(fs + boff + foff) == 4)
				fl = readl(fs + boff + foff + 4);
		}
		/* mapeo flags FS->oflag (Ghidra H6d) */
		if (fl & 0x10000)  oflag = O_RDWR | O_CREAT;
		if (fl & 0x20000)  oflag = O_RDWR | O_CREAT | O_TRUNC;"""
if "H8i: los FLAGS del OPEN" in s:
    print("ya aplicado")
elif old in s:
    open(p,"w").write(s.replace(old,new,1)); print("+ flags del OPEN leidos del 2o campo")
else:
    print("NO MATCH"); raise SystemExit(1)
