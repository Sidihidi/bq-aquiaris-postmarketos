p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# 1) parametro
a1 = "static uint spm_fs_p0_mode;"
n1 = """/*
 * H6v: valor de 4 bytes que devolvemos en el UNICO campo de la respuesta al 0x1010.
 *
 * El desensamblado del bucle de fs_ccci.c (0729) demostro que el buffer de
 * respuesta es una LISTA DE CAMPOS con longitud:
 *      +0 = 0xffff0000|op | +4 = c = NUMERO DE CAMPOS | +8 = len_0 | +0xc = datos_0 | ...
 * y que para cada campo el MD tiene reservada una CAPACIDAD fija (fp[i]) sobre la
 * que hace memcpy: exige capacidad >= len.  El 0x1010 llega con c=1 y capacidad 4
 * -> la respuesta es UN campo de 4 bytes, no un path.  Devolvemos 0 y el MD acaba
 * recorriendo la NVRAM en circulo hasta el op 0x1012 (que no existe en el arranque
 * real).  Este parametro permite barrer ese valor sin recompilar:
 *   echo 0xfffff057 > /sys/module/mt6582_spm/parameters/spm_fs_1010_val
 * (acepta hex con prefijo 0x; candidatos: 0, 1, 0xffffffff y la serie de codigos
 *  del modulo FS 0xfffff055..0xfffff05d = -4011..-4003)
 */
static uint spm_fs_1010_val;
module_param(spm_fs_1010_val, uint, 0644);
MODULE_PARM_DESC(spm_fs_1010_val, "valor de 4 bytes del campo de respuesta del 0x1010 (acepta 0x...)");

static uint spm_fs_p0_mode;"""
if "H6v: valor de 4 bytes" in s:
    print("= param: ya aplicado")
elif a1 in s:
    s = s.replace(a1, n1, 1); print("+ param spm_fs_1010_val")
else:
    print("! NO MATCH param"); raise SystemExit(1)

# 2) usarlo en la forma corta de los modos 9/10
a2 = """			if (m == 9 || m == 10) {
				writel(4, fs + boff + 8);	/* longitud corta */
				writel(0, fs + boff + 0xc);"""
n2 = """			if (m == 9 || m == 10) {
				writel(4, fs + boff + 8);	/* len_0 = 4 (capacidad del MD) */
				writel(spm_fs_1010_val, fs + boff + 0xc);	/* H6v */"""
if "spm_fs_1010_val, fs + boff + 0xc" in s:
    print("= modos 9/10: ya aplicado")
elif a2 in s:
    s = s.replace(a2, n2, 1); print("+ modos 9/10 usan el valor")
else:
    print("! NO MATCH modos 9/10"); raise SystemExit(1)

# 3) y en la rama "sin resolucion" de 11/12/13
a3 = """				if (!out) {		/* sin resolucion -> forma corta (modo 9) */
					writel(4, fs + boff + 8);
					writel(0, fs + boff + 0xc);"""
n3 = """				if (!out) {		/* sin resolucion -> forma corta (modo 9) */
					writel(4, fs + boff + 8);
					writel(spm_fs_1010_val, fs + boff + 0xc);	/* H6v */"""
if a3 in s:
    s = s.replace(a3, n3, 1); print("+ rama sin-resolucion usa el valor")

open(p, "w").write(s)
