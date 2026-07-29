p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

anchor = """			if (m == 9 || m == 10) {"""

new = """			/*
			 * H6u: RESOLUCION DE RUTAS (lo que hace el ccci_fsd real).
			 *
			 * Ground-truth (gd-boot-full.out, unica respuesta 1010 capturada):
			 *    REQ  1010 c=1 len=0x24 "Z:\\NVRAM\\NVD_DATA"
			 *    RESP 1010 c=2 len=0x36 "Z:\\NVRAM\\NVD_CORE\\MT00A000"
			 * O sea: el fsd NO hace eco, TRADUCE el nombre logico a un fichero
			 * fisico.  Contestando "vacio" (modos 9/10) el MD recorre toda la
			 * NVRAM y acaba en el op 0x1012, que NO existe en el arranque real
			 * -> se va por una rama que el original nunca pisa.
			 *
			 *   11 = replicar el ground-truth (mapeo fijo), +4 INTACTO
			 *   12 = idem pero escribiendo c=2 en +4 (como el fsd real).  OJO:
			 *        escribir 2 nos hacia petar en la linea 528, asi que la
			 *        expectativa del MD depende de su estado -> hay que medirlo.
			 *   13 = generico: si la ruta pedida es un DIRECTORIO que existe,
			 *        devolver el primer fichero que contenga.
			 */
			if (m == 11 || m == 12 || m == 13) {
				static const char *gt = "Z:\\\\NVRAM\\\\NVD_CORE\\\\MT00A000";
				char res[128];
				const char *out = NULL;
				int n;

				if (m == 13) {
					/* primer fichero dentro del directorio pedido */
					struct file *d = filp_open(path, O_RDONLY | O_DIRECTORY, 0);

					if (!IS_ERR(d)) {
						filp_close(d, NULL);
						/* sin readdir en este contexto: usamos el mapeo
						 * conocido si la ruta acaba en NVD_DATA */
						if (strstr(path, "NVD_DATA"))
							out = gt;
					}
				} else if (strstr(path, "NVD_DATA")) {
					out = gt;
				}

				if (!out) {		/* sin resolucion -> forma corta (modo 9) */
					writel(4, fs + boff + 8);
					writel(0, fs + boff + 0xc);
					length = 12;
					dev_info(s->dev, "H6u op=%x %s -> sin resolucion (corta)\\n",
						 op, path);
					break;
				}

				/* escribir la ruta resuelta en UTF-16LE a partir de +0xc */
				n = 0;
				while (out[n] && n < 120) {
					writew((u16) out[n], fs + boff + 0xc + n * 2);
					n++;
				}
				writew(0, fs + boff + 0xc + n * 2);	/* NUL */
				writel((n + 1) * 2, fs + boff + 8);	/* len en BYTES, con NUL */
				if (m == 12)
					writel(2, fs + boff + 4);	/* como el fsd real */
				length = 8 + (n + 1) * 2;
				scnprintf(res, sizeof(res), "%s", out);
				dev_info(s->dev, "H6u op=%x %s -> RESUELTO '%s' len=%d (modo %u)\\n",
					 op, path, res, (n + 1) * 2, m);
				break;
			}

			if (m == 9 || m == 10) {"""

if "H6u: RESOLUCION DE RUTAS" in s:
    print("ya aplicado")
elif anchor in s:
    open(p, "w").write(s.replace(anchor, new, 1))
    print("H6u: modos 11/12/13 (resolucion de rutas) anadidos")
else:
    print("NO MATCH"); raise SystemExit(1)
