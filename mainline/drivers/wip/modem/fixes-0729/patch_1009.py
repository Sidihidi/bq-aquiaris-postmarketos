p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = """		spm_fs_p0(fs, boff, 1);	/* H6w FIX C: +4 = numero de campos */
		writel(4, fs + boff + 8);
		writel(hnd, fs + boff + 0xc);
		writel(4, fs + boff + 0x10);
		writel((u32)sz, fs + boff + 0x14);
		writel((u32)(sz >> 32), fs + boff + 0x18);
		length = 0x18;"""
new = """		/*
		 * H6z: este handler escribe DOS campos (handle y tamaño) pero declaraba
		 * UNO en +4, y ademas ponia len=4 para un tamaño de 64 bits.  Medido
		 * 0729 (kernel #46): el MD asserta en la linea 528 con valor 1 leido.
		 *   +4    = 2      numero de campos
		 *   +8    = 4      len_0  -> +0xc  = handle
		 *   +0x10 = 8      len_1  -> +0x14 = tamaño (64b, dos palabras)
		 */
		spm_fs_p0(fs, boff, 2);			/* 2 campos */
		writel(4, fs + boff + 8);
		writel(hnd, fs + boff + 0xc);
		writel(8, fs + boff + 0x10);		/* tamaño = 8 bytes */
		writel((u32)sz, fs + boff + 0x14);
		writel((u32)(sz >> 32), fs + boff + 0x18);
		length = 0x18;"""
if "H6z: este handler escribe DOS campos" in s:
    print("ya aplicado")
elif old in s:
    open(p, "w").write(s.replace(old, new, 1)); print("H6z: 1009 declara 2 campos y len 8")
else:
    print("NO MATCH"); raise SystemExit(1)
