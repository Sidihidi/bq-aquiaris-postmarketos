p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# defecto: cada handler escribe SU numero de campos
s = s.replace("static uint spm_fs_p0_mode = 1;\t/* H6w: por defecto NO tocar +4 */",
              """/*
 * H6x: +4 = NUMERO DE CAMPOS DE LA RESPUESTA (no el 'c' de la peticion).
 *
 * Medido 0729 (kernel #44): poner "no tocar" por defecto rompio el OPEN, que
 * antes funcionaba escribiendo 1.  La peticion del OPEN trae c=2 pero su
 * respuesta lleva UN solo campo (el handle) -> +4 = 1.  El GetDrive coincide en
 * 2 por casualidad (peticion c=2, respuesta 2 campos).  El 0x1010 pide 1 campo,
 * y por eso "no tocar" (dejaba el 1 de la peticion) tambien funcionaba.
 * Asi que cada handler declara los suyos:  0 = handler, 1 = no tocar, ...
 */
static uint spm_fs_p0_mode;""", 1)

# el 1010 en forma corta lleva UN campo -> declararlo explicitamente
old = """			if (m == 9 || m == 10) {
				writel(4, fs + boff + 8);	/* len_0 = 4 (capacidad del MD) */"""
new = """			if (m == 9 || m == 10) {
				spm_fs_p0(fs, boff, 1);		/* H6x: 1 campo en la respuesta */
				writel(4, fs + boff + 8);	/* len_0 = 4 (capacidad del MD) */"""
if "H6x: 1 campo en la respuesta" in s:
    print("= 1010: ya aplicado")
elif old in s:
    s = s.replace(old, new, 1); print("+ 1010 forma corta declara 1 campo")
else:
    print("! NO MATCH 1010"); raise SystemExit(1)

# y la rama sin-resolucion de 11/12/13, igual
old2 = """				if (!out) {		/* sin resolucion -> forma corta (modo 9) */
					writel(4, fs + boff + 8);"""
new2 = """				if (!out) {		/* sin resolucion -> forma corta (modo 9) */
					spm_fs_p0(fs, boff, 1);	/* H6x: 1 campo */
					writel(4, fs + boff + 8);"""
if old2 in s:
    s = s.replace(old2, new2, 1); print("+ rama sin-resolucion declara 1 campo")

# el error del OPEN tambien devuelve 1 campo
old3 = """			dev_warn(s->dev, "H6 FS OPEN %s fallo (err en payload)\\n", path);
			writel(4, fs + boff + 8);"""
new3 = """			dev_warn(s->dev, "H6 FS OPEN %s fallo (err en payload)\\n", path);
			spm_fs_p0(fs, boff, 1);			/* H6x: 1 campo */
			writel(4, fs + boff + 8);"""
if old3 in s:
    s = s.replace(old3, new3, 1); print("+ error del OPEN declara 1 campo")

open(p, "w").write(s)
