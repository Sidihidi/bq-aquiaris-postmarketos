p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = """		if (rgu) iounmap(rgu); if (vec) iounmap(vec); if (key) iounmap(key);
		if (en) iounmap(en); if (ccif) iounmap(ccif);"""
new = """		if (rgu)
			iounmap(rgu);
		if (vec)
			iounmap(vec);
		if (key)
			iounmap(key);
		if (en)
			iounmap(en);
		if (ccif)
			iounmap(ccif);"""
assert old in s, "bloque de iounmap no encontrado"
open(p, "w").write(s.replace(old, new, 1))
print("OK: indentacion arreglada")
