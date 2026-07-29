import re
p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# ---- FIX A: nunca poner el bit31 en la marca ----
old_a = """			dev_warn(s->dev, "H6 FS OPEN %s fallo\\n", path);
			writel(fs_ops | 0x80000000, fs + boff + 0);	/* error: bit31 */
			return 0;"""
new_a = """			/*
			 * H6w FIX A: NO marcar el error con el bit31.
			 * Medido 0729: la comprobacion de la linea 520 de fs_ccci.c exige
			 * buffer[+0] == 0xffff0000|op SIEMPRE.  Con el bit31 el MD leia
			 * 0x80001001 donde esperaba 0xffff1001 -> assert inmediato (params
			 * del registro: esperado ffff1001, recibido 80001001).  El error se
			 * senaliza en el PAYLOAD, no en la marca.
			 */
			dev_warn(s->dev, "H6 FS OPEN %s fallo (err en payload)\\n", path);
			writel(4, fs + boff + 8);
			writel(spm_fs_1010_val, fs + boff + 0xc);
			length = 12;
			break;"""
if "H6w FIX A" in s:
    print("= FIX A: ya aplicado")
elif old_a in s:
    s = s.replace(old_a, new_a, 1); print("+ FIX A: marca sin bit31")
else:
    print("! NO MATCH FIX A"); raise SystemExit(1)

# ---- FIX B: abrir directorios ----
old_b = """		f = filp_open(path, oflag, 0660);"""
new_b = """		f = filp_open(path, oflag, 0660);
		/*
		 * H6w FIX B: el MD hace OPEN sobre DIRECTORIOS ("Z:\\\\NVRAM"), y con
		 * O_RDWR eso falla con EISDIR.  Reintentar en solo lectura.
		 */
		if (IS_ERR(f))
			f = filp_open(path, O_RDONLY | O_DIRECTORY, 0);
		if (IS_ERR(f))
			f = filp_open(path, O_RDONLY, 0);"""
if "H6w FIX B" in s:
    print("= FIX B: ya aplicado")
elif old_b in s:
    s = s.replace(old_b, new_b, 1); print("+ FIX B: OPEN de directorios")
else:
    print("! NO MATCH FIX B"); raise SystemExit(1)

# ---- FIX C: NUNCA tocar +4 (es el numero de campos) ----
i = s.index("static u32 spm_fs_serve(")
j = s.index("\n\treturn length;\n}", i)
body = s[i:j]
n = len(re.findall(r"writel\(([^;]+?), fs \+ boff \+ 4\);", body))
body = re.sub(r"writel\(([^;]+?), fs \+ boff \+ 4\);",
              r"spm_fs_p0(fs, boff, \1);\t/* H6w FIX C: +4 = numero de campos */",
              body)
s = s[:i] + body + s[j:]
print("+ FIX C: %d escrituras directas a +4 enrutadas" % n)

# y el defecto pasa a "no tocar"
s = s.replace("static uint spm_fs_p0_mode;",
              "static uint spm_fs_p0_mode = 1;\t/* H6w: por defecto NO tocar +4 */", 1)
open(p, "w").write(s)
print("+ p0_mode por defecto = 1 (no tocar)")
