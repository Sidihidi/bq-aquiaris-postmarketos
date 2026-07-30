p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = """		if (!done)
			{	int z, z0; static char b[3200]; int o = 0;
				/* H7h: imprimir la COLA (donde se para), no la cabeza:
				 * el volcado se truncaba y ocultaba el punto de parada. */
				z0 = (spm_fslog_n > 48) ? spm_fslog_n - 48 : 0;
				for (z = z0; z < spm_fslog_n && o < 3100; z++)
					o += scnprintf(b + o, 3200 - o, "%04x ", spm_fslog[z]);
				b[o] = 0;
				dev_info(s->dev, "H6 FS ops servidos (%d): %s\\n", spm_fslog_n, b);
				spm_fslog_n = 0;
			}"""
new = """		{	int z, z0; static char b[3200]; int o = 0;
			/* H7h: imprimir la COLA (donde se para), no la cabeza:
			 * el volcado se truncaba y ocultaba el punto de parada.
			 * H8k: se imprime SIEMPRE, no solo al fallar. El if (!done)
			 * que introdujo H8g escondia el contador de operaciones
			 * justo cuando el arranque salia bien, que es cuando mas
			 * interesa (es la metrica de progreso del hito). */
			z0 = (spm_fslog_n > 48) ? spm_fslog_n - 48 : 0;
			for (z = z0; z < spm_fslog_n && o < 3100; z++)
				o += scnprintf(b + o, 3200 - o, "%04x ", spm_fslog[z]);
			b[o] = 0;
			dev_info(s->dev, "H6 FS ops servidos (%d), cola: %s\\n",
				 spm_fslog_n, b);
			spm_fslog_n = 0;
		}"""
assert old in s, "bloque del volcado no encontrado"
open(p, "w").write(s.replace(old, new, 1))
print("OK: volcado de traza ahora se imprime siempre")
