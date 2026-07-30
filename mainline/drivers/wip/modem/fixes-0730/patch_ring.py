p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

old1 = """static u32 spm_fslog[512];
static int spm_fslog_n;"""
new1 = """/*
 * H8k: traza de ops del FS. Antes era un buffer lineal que se llenaba y dejaba
 * de grabar, asi que (a) el contador saturaba en 512 aunque se sirvieran ~900 y
 * (b) los "ultimos" que imprimia eran en realidad los del medio del arranque.
 * Ahora es un ANILLO (guarda los ULTIMOS 512) + un total sin tope.
 */
#define SPM_FSLOG_SZ 512
static u32 spm_fslog[SPM_FSLOG_SZ];
static int spm_fslog_n;			/* cuantos hay en el anillo (<= SZ) */
static int spm_fslog_w;			/* siguiente posicion de escritura */
static int spm_fs_total;		/* total servido en el ciclo, sin tope */"""
assert old1 in s, "declaraciones del fslog no encontradas"
s = s.replace(old1, new1, 1)

old2 = """	if (spm_fslog_n < 512)			/* H6n: traza compacta, se vuelca al final */
		spm_fslog[spm_fslog_n++] = op;"""
new2 = """	spm_fs_total++;				/* H6n/H8k: traza compacta, anillo */
	spm_fslog[spm_fslog_w] = op;
	spm_fslog_w = (spm_fslog_w + 1) % SPM_FSLOG_SZ;
	if (spm_fslog_n < SPM_FSLOG_SZ)
		spm_fslog_n++;"""
assert old2 in s, "sitio de incremento no encontrado"
s = s.replace(old2, new2, 1)

old3 = """			z0 = (spm_fslog_n > 48) ? spm_fslog_n - 48 : 0;
			for (z = z0; z < spm_fslog_n && o < 3100; z++)
				o += scnprintf(b + o, 3200 - o, "%04x ", spm_fslog[z]);
			b[o] = 0;
			dev_info(s->dev, "H6 FS ops servidos (%d), cola: %s\\n",
				 spm_fslog_n, b);
			spm_fslog_n = 0;"""
new3 = """			z0 = (spm_fslog_n < 48) ? spm_fslog_n : 48;
			for (z = z0; z > 0 && o < 3100; z--) {
				int k = (spm_fslog_w - z + SPM_FSLOG_SZ) % SPM_FSLOG_SZ;

				o += scnprintf(b + o, 3200 - o, "%04x ", spm_fslog[k]);
			}
			b[o] = 0;
			dev_info(s->dev, "H6 FS: %d ops servidos, ultimos %d: %s\\n",
				 spm_fs_total, z0, b);
			spm_fslog_n = 0;
			spm_fslog_w = 0;
			spm_fs_total = 0;"""
assert old3 in s, "bloque del volcado no encontrado"
s = s.replace(old3, new3, 1)

open(p, "w").write(s)
print("OK: anillo real + contador total")
