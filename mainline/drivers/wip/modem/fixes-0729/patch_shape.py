p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

anchor = """			if (m == 8) {		/* H6o: control -> no contestar nada */
				dev_info(s->dev, "H6 FS DIR op=%x %s -> SIN RESPUESTA (control)\\n",
					 op, path);
				return SPM_FS_NOREPLY;
			}"""

new = anchor + """

			/*
			 * H6t: respuesta con la FORMA que el MD acepta.
			 *
			 * Del desensamblado de fs_ccci.c (0729) salen DOS reglas, ambas
			 * confirmadas midiendo con el registro de excepcion limpio:
			 *   linea 528: buffer[+4] debe conservar el 'c' de la PETICION.
			 *              (GetDrive llega con c=2 y escribiamos 2 -> pasaba;
			 *               el 0x1010 llega con c=1 y escribiamos 2 -> petaba)
			 *   linea 547: buffer[+8] (longitud) NO puede exceder lo que el MD
			 *              reservo -> aqui midio 4; nosotros devolviamos 0x26.
			 * Asi que: NO tocar +4 y devolver una longitud corta.
			 *   9  = +8=4, +0xc=0, length=12
			 *   10 = clon exacto de la forma del GetDrive (+8=4, +0xc=0,
			 *        +0x10=0x54, length=16), que es la unica que el MD acepta.
			 */
			if (m == 9 || m == 10) {
				writel(4, fs + boff + 8);	/* longitud corta */
				writel(0, fs + boff + 0xc);
				if (m == 10) {
					writel(0x54, fs + boff + 0x10);
					length = 16;
				} else {
					length = 12;
				}
				dev_info(s->dev, "H6t op=%x %s -> forma corta (modo %u, +4 intacto)\\n",
					 op, path, m);
				break;
			}"""

if "H6t: respuesta con la FORMA" in s:
    print("ya aplicado")
elif anchor in s:
    open(p, "w").write(s.replace(anchor, new, 1))
    print("H6t: modos 9 y 10 anadidos")
else:
    print("NO MATCH"); raise SystemExit(1)
