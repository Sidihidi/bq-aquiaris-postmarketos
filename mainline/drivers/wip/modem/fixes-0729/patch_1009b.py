p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = """		writel(8, fs + boff + 0x10);		/* tamaño = 8 bytes */
		writel((u32)sz, fs + boff + 0x14);
		writel((u32)(sz >> 32), fs + boff + 0x18);
		length = 0x18;"""
new = """		/* H6z-b: la capacidad del 2o campo es 4, no 8 (medido: assert 547 con
		 * params 4 y 8) -> el tamaño va en 32 bits. */
		writel(4, fs + boff + 0x10);		/* len_1 = 4 */
		writel((u32)sz, fs + boff + 0x14);	/* tamaño (32b) */
		length = 0x14;"""
if "H6z-b" in s:
    print("ya aplicado")
elif old in s:
    open(p, "w").write(s.replace(old, new, 1)); print("H6z-b: tamaño en 32 bits")
else:
    print("NO MATCH"); raise SystemExit(1)
