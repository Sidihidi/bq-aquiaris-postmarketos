p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = """	default:				/* READ / CLOSE / GETSIZE / metadata-mount:
		 * PENDIENTE de confirmar el op-code exacto del log del proxy en pmOS.
		 * Por ahora: exito minimo (deja avanzar los 42 ops de mount). */
		break;"""
new = """	default:
		/*
		 * H6y: RESPUESTA MINIMA VALIDA para cualquier op sin handler.
		 *
		 * Antes no se tocaba nada, asi que en +8 quedaba la longitud de la
		 * PETICION y el MD reventaba en la comprobacion de la linea 547
		 * (capacidad >= longitud).  Medido 0729 con el op 0x1007: esperado 4,
		 * devolviamos 0x24.
		 *
		 * Como la regla es capacidad >= longitud y la capacidad minima que
		 * hemos observado es 4, devolver UN campo de 4 bytes es seguro para
		 * cualquier op: si la capacidad fuese mayor, 4 sigue cabiendo.
		 */
		spm_fs_p0(fs, boff, 1);			/* 1 campo */
		writel(4, fs + boff + 8);		/* len_0 = 4 */
		writel(spm_fs_1010_val, fs + boff + 0xc);
		length = 12;
		fs_dbg(s->dev, "H6y op=%x sin handler -> respuesta minima\\n", op);
		break;"""
if "H6y: RESPUESTA MINIMA VALIDA" in s:
    print("ya aplicado")
elif old in s:
    open(p, "w").write(s.replace(old, new, 1)); print("H6y: default con respuesta minima valida")
else:
    print("NO MATCH"); raise SystemExit(1)
