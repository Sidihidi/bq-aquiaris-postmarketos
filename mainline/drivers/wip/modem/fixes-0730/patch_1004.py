#!/usr/bin/env python3
# H7d — arreglar el 1004 (WRITE): rellenar TODOS los campos que se declaran.
#
# BUG: en el `default` el contador se calcula con el override
#      nf = (op == spm_fs_ov_op) ? spm_fs_ov_nf : spm_fs_def_nf
# pero los campos se escribian mirando **spm_fs_def_nf**, no `nf`.  Con el
# override del 0x1004 (nf=2) se declaraban 2 campos y solo se escribia 1: el
# segundo conservaba la longitud de la PETICION (medido 0x358 = 856, el WRITE
# lleva datos) y el MD moria en la comprobacion de capacidad (fs_ccci.c:547).
#
# FIX: un solo bucle que escribe los `nf` campos declarados (len=4, dato 0) y
# calcula length = 4 + nf*8, que es exactamente lo que daba el codigo anterior
# para nf=1/2/3 (12 / 0x14 / 0x1c).  Ademas queda generalizado para nf>3, asi
# el barrido de ops nuevos ya no necesita tocar el codigo.
import sys

p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

old = """		{
			u32 nf = (op == spm_fs_ov_op) ? spm_fs_ov_nf : spm_fs_def_nf;

			if (nf != 0xff)
				spm_fs_p0(fs, boff, nf);	/* H7a/H7c */
		}
		writel(4, fs + boff + 8);		/* len_0 = 4 */
		writel(spm_fs_1010_val, fs + boff + 0xc);
		if (spm_fs_def_nf >= 2) {		/* 2o campo, tambien de 4 bytes */
			writel(4, fs + boff + 0x10);
			writel(0, fs + boff + 0x14);
			length = 0x14;
		} else {
			length = 12;
		}
		if (spm_fs_def_nf >= 3) {
			writel(4, fs + boff + 0x18);
			writel(0, fs + boff + 0x1c);
			length = 0x1c;
		}
		fs_dbg(s->dev, "H6y op=%x sin handler -> minima (nf=%u)\\n", op, spm_fs_def_nf);
		break;"""

new = """		{
			u32 nf = (op == spm_fs_ov_op) ? spm_fs_ov_nf : spm_fs_def_nf;
			u32 i;

			if (nf == 0xff)			/* 0xff = no tocar el contador */
				nf = spm_fs_def_nf;
			else
				spm_fs_p0(fs, boff, nf);	/* H7a/H7c */

			/*
			 * H7d: rellenar TODOS los campos DECLARADOS.
			 *
			 * Antes esto miraba spm_fs_def_nf en vez de `nf`, asi que con el
			 * override (0x1004 -> nf=2) se declaraban 2 campos y solo se
			 * escribia 1: el segundo se quedaba con la longitud de la
			 * PETICION (medido 0x358) y el MD moria en la comprobacion de
			 * capacidad (fs_ccci.c:547, error -4010).
			 *
			 * Cada campo son 8 bytes: [len=4][dato].  length = 4 (la palabra
			 * del contador) + nf*8, que reproduce 12/0x14/0x1c para nf=1/2/3.
			 */
			if (nf > 8)
				nf = 8;			/* tope de seguridad para el buffer */
			for (i = 0; i < nf; i++) {
				writel(4, fs + boff + 8 + i * 8);		/* len_i = 4 */
				writel(i ? 0 : spm_fs_1010_val,
				       fs + boff + 0xc + i * 8);	/* dato_i */
			}
			length = 4 + nf * 8;

			fs_dbg(s->dev, "H6y op=%x sin handler -> minima (nf=%u len=%u)\\n",
			       op, nf, length);
		}
		break;"""

if "H7d: rellenar TODOS los campos DECLARADOS" in s:
    print("ya aplicado")
elif old in s:
    open(p, "w").write(s.replace(old, new, 1))
    print("PATCH H7d OK: el default rellena los nf campos declarados")
else:
    print("NO MATCH — el bloque default no coincide"); sys.exit(1)
