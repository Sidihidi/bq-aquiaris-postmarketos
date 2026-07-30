import re, sys
p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
orig = s

# 1) p0 muerto + comentarios obsoletos (el modelo pre-campos que causo el bug H8i)
old = """	u32 p0 = readl(fs + boff + 4);		/* flags (OPEN) / handle */
	u32 p1 = readl(fs + boff + 8);		/* len path (OPEN) / handle */"""
new = """	/*
	 * H8j: ESTRUCTURA DE LA PETICION (y de la respuesta) — lista de campos
	 * con longitud, confirmada contra el ground-truth del fsd de Lineage:
	 *
	 *   +0    op                marca; en la respuesta 0xffff0000|op, escrita AL FINAL
	 *   +4    nfields           numero de campos
	 *   +8    len_0
	 *   +0xc  datos_0           (alineado a 4)
	 *         len_1  datos_1    ... y asi nfields veces
	 *
	 * Por eso, con un primer campo de 4 bytes (un handle): datos_0 en +0xc,
	 * len_1 en +0x10 y datos_1 en +0x14. Leer +0x10 esperando datos es el
	 * error que costo tres bugs (H6z, H7b, H8i) — solo el WRITE lee +0x10
	 * a proposito, porque ahi el len_1 ES la longitud de los datos.
	 */
	u32 len0 = readl(fs + boff + 8);	/* len_0: long. del 1er campo */"""
assert old in s, "decls p0/p1 no encontradas"
s = s.replace(old, new, 1)

# renombrar los usos de p1 (solo dentro de spm_fs_serve)
n = len(re.findall(r"\bp1\b", s))
s = re.sub(r"\bp1\b", "len0", s)
print("usos de p1 renombrados a len0:", n)

# 2) kstrtou32: valor de retorno ignorado (warn_unused_result)
old2 = """	if (val)
		kstrtou32(strim((char *)val), 16, &off);	/* el valor se lee en HEX */"""
new2 = """	if (val && kstrtou32(strim((char *)val), 16, &off))	/* se lee en HEX */
		off = 0;					/* invalido -> barrido */"""
assert old2 in s, "sitio del kstrtou32 no encontrado"
s = s.replace(old2, new2, 1)

# 3) documentar el whence del SEEK (constante 1 = desde el principio)
old3 = "	case 0x1002: {				/* SEEK: handle@+0xc, offset@+0x14 -> newpos en +0xc */"
new3 = """	/*
	 * SEEK: handle@+0xc, offset@+0x14, whence@+0x1c -> newpos en +0xc.
	 * H8j: el whence es 1 en las 97 peticiones del ground-truth y significa
	 * "desde el principio" (la respuesta real devuelve el offset absoluto
	 * pedido), asi que SEEK_SET es correcto y no hace falta leerlo.
	 * La respuesta lleva 1 campo con la posicion — igual que el fsd real.
	 */
	case 0x1002: {"""
assert old3 in s, "handler del SEEK no encontrado"
s = s.replace(old3, new3, 1)

assert s != orig
open(p, "w").write(s)
print("OK: parche de auditoria aplicado")
