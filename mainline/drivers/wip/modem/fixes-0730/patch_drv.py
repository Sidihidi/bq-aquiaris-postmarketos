p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# --- 1) traduccion de ruta consciente del DISCO ---
old = """static void spm_fs_path(void __iomem *src, int plen, char *out, int outsz)
{
	int i, o;
	strscpy(out, "/data/nvram/md", outsz);
	o = strlen(out);
	/* saltar "Z:" (2 chars UTF-16 = 4 bytes) */
	for (i = 4; i + 1 < plen && o < outsz - 1; i += 2) {
		u8 c = readb(src + i);		/* low byte del char UTF-16 */
		if (c == '\\\\')
			c = '/';
		out[o++] = c;
	}
	out[o] = 0;
}"""
new = """/*
 * H8f: la traduccion de ruta era ciega al DISCO.
 *
 * Medido 0730 con spm_fs_quiet=0: tras montar la NVRAM el MD pide ficheros de
 * OTROS volumenes, no solo de Z:
 *     path=[003a0058 005c005c 0050004d]  ->  "X:\\\\MP0D_000"
 *     path=[003a0058 005c005c 00540053]  ->  "X:\\\\ST33A004"
 *     path=[003a0059 005c005c 00540053]  ->  "Y:\\\\ST33B004"
 * Descartabamos los 2 primeros caracteres y lo metiamos TODO en /data/nvram/md,
 * asi que (a) confundiamos tres volumenes en un solo directorio y (b) la barra
 * doble producia rutas con "//" (de ahi el "/data/nvram/md//MP0D_000" del log).
 *
 * Ahora: Z: -> /data/nvram/md (como antes, la NVRAM), y cualquier otro disco a
 * su propio subdirectorio (/data/nvram/md/X, .../Y).  Ademas se colapsan las
 * barras repetidas.
 */
static void spm_fs_path(void __iomem *src, int plen, char *out, int outsz)
{
	int i, o;
	u8 drive = readb(src);			/* letra de volumen: 'Z', 'X', 'Y'... */

	strscpy(out, "/data/nvram/md", outsz);
	o = strlen(out);
	if (drive != 'Z' && drive != 'z' && o < outsz - 3) {
		out[o++] = '/';
		out[o++] = drive;
	}
	/* saltar "<letra>:" (2 chars UTF-16 = 4 bytes) */
	for (i = 4; i + 1 < plen && o < outsz - 1; i += 2) {
		u8 c = readb(src + i);		/* low byte del char UTF-16 */

		if (c == '\\\\')
			c = '/';
		if (c == '/' && o > 0 && out[o - 1] == '/')
			continue;		/* colapsar barras repetidas */
		out[o++] = c;
	}
	out[o] = 0;
}"""
if "H8f: la traduccion de ruta" in s:
    print("= path: ya aplicado")
elif old in s:
    s = s.replace(old, new, 1); print("+ traduccion consciente del disco + colapso de barras")
else:
    print("! NO MATCH path"); raise SystemExit(1)

# --- 2) el mensaje contradictorio del final ---
old2 = """			dev_info(s->dev, "H4 HS2: sin boot-ready tras 8s. RCHNUM=%08x BUSY=%08x\\n","""
new2 = """			/* H8g: antes esto se imprimia SIEMPRE, incluso tras un HS2
			 * correcto, dejando dos mensajes contradictorios en el log
			 * ("HS2 LOGRADO" y "sin boot-ready").  El stock decide la
			 * etapa 2 con la misma condicion que nosotros
			 * (ccci_md_main.c:2140: msg.id == NORMAL_BOOT_ID en etapa 1),
			 * asi que el HS2 era bueno y el que sobraba era este aviso. */
			dev_info(s->dev, done
				 ? "H4 HS2: fin del bucle (etapa 2 alcanzada). RCHNUM=%08x BUSY=%08x\\n"
				 : "H4 HS2: sin boot-ready. RCHNUM=%08x BUSY=%08x\\n","""
if "H8g: antes esto se imprimia" in s:
    print("= mensaje: ya aplicado")
elif old2 in s:
    s = s.replace(old2, new2, 1); print("+ mensaje del final corregido")
else:
    print("! NO MATCH mensaje"); raise SystemExit(1)
open(p, "w").write(s)
