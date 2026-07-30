#!/usr/bin/env python3
# H7j — 0x1010 = FindFirst y 0x1011 = FindNext con ENUMERACION REAL de la NVRAM.
#
# El diff contra el ground-truth (883 peticiones) mostro que divergimos en la 6a op:
#   GT:      100e 1010 1001 1005 1010 | 1009 1002 1003 1005 ...
#   NUESTRO: 100e 1010 1001 1005 1010 | 1010 1007 1010 1007 ...
# El 5o op es 1010 'Z:\NVRAM\NVD_DATA' y el fsd real contesta con una RUTA
# ('Z:\NVRAM\NVD_CORE\MT00A000'), tras lo cual el MD hace GetSize+READ.  Y en la
# secuencia real abundan los tripletes 1011 1009 1005 => el MD ENUMERA su NVRAM:
#   1010 = FindFirst   1011 = FindNext
#
# Aqui se implementa de verdad: se recorren los subdirectorios de la NVRAM en orden
# alfabetico (CALIBRAT, IMPORTNT, NVD_CORE, NVD_DATA, NVD_IMEI — el fsd devolvio
# NVD_CORE\MT00A000, que es el primer fichero del primer directorio con contenido)
# y se devuelven las rutas completas una a una, en UTF-16, con el formato de campos.
#
# spm_fs_enum=0 restaura el comportamiento anterior (devolver spm_fs_1010_val).
# spm_fs_find_nf = numero de campos de la respuesta (la tabla del Mac midio 1; el
# ground-truth muestra 2) -> ajustable en caliente para probar ambos.
import sys

p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
ok = True

# ---- 1) motor de enumeracion, justo antes de spm_fs_path ---------------------
anchor1 = "static void spm_fs_path(void __iomem *src, int plen, char *out, int outsz)"

engine = '''/* ===== H7j: enumeracion REAL de la NVRAM (FindFirst / FindNext) ===== */
#define FS_ENUM_MAX	96
#define FS_ENUM_PLEN	72

static char g_enum[FS_ENUM_MAX][FS_ENUM_PLEN];	/* rutas "Z:\\\\NVRAM\\\\DIR\\\\FICHERO" */
static int  g_enum_n;				/* cuantas hay */
static int  g_enum_i;				/* cursor del FindNext */

static uint spm_fs_enum = 1;
module_param(spm_fs_enum, uint, 0644);
MODULE_PARM_DESC(spm_fs_enum, "1 = 1010/1011 enumeran la NVRAM de verdad; 0 = comportamiento anterior");

static uint spm_fs_find_nf = 1;
module_param(spm_fs_find_nf, uint, 0644);
MODULE_PARM_DESC(spm_fs_find_nf, "campos de la respuesta de FindFirst/FindNext (1 o 2)");

struct spm_enum_ctx {
	struct dir_context ctx;
	const char *sub;
};

static bool spm_enum_filldir(struct dir_context *ctx, const char *name, int nlen,
			     loff_t off, u64 ino, unsigned int dt)
{
	struct spm_enum_ctx *c = container_of(ctx, struct spm_enum_ctx, ctx);

	if (nlen == 1 && name[0] == '.')
		return true;
	if (nlen == 2 && name[0] == '.' && name[1] == '.')
		return true;
	if (dt == DT_DIR)
		return true;
	if (g_enum_n < FS_ENUM_MAX)
		scnprintf(g_enum[g_enum_n++], FS_ENUM_PLEN,
			  "Z:\\\\NVRAM\\\\%s\\\\%.*s", c->sub, nlen, name);
	return true;
}

/* Orden alfabetico de subdirectorios: es el que sigue el fsd real (su FindFirst
 * devolvio NVD_CORE\\MT00A000, el primer fichero del primer subdir con contenido). */
static void spm_fs_enum_build(struct mt6582_spm *s)
{
	static const char * const subs[] = {
		"CALIBRAT", "IMPORTNT", "NVD_CORE", "NVD_DATA", "NVD_IMEI"
	};
	char path[80];
	int k;

	g_enum_n = 0;
	g_enum_i = 0;
	for (k = 0; k < ARRAY_SIZE(subs); k++) {
		struct spm_enum_ctx c = {
			.ctx.actor = spm_enum_filldir,
			.ctx.pos = 0,
			.sub = subs[k],
		};
		struct file *d;

		scnprintf(path, sizeof(path), "/data/nvram/md/NVRAM/%s", subs[k]);
		d = filp_open(path, O_RDONLY | O_DIRECTORY, 0);
		if (IS_ERR(d))
			continue;
		iterate_dir(d, &c.ctx);
		filp_close(d, NULL);
	}
	dev_info(s->dev, "H7j: enumeracion NVRAM = %d ficheros%s%s\\n", g_enum_n,
		 g_enum_n ? ", primero: " : "", g_enum_n ? g_enum[0] : "");
}

/* Escribe una ruta ASCII como UTF-16 en el buffer de respuesta y deja los campos.
 * Devuelve la 'length' (4 del contador + 4 del len + los bytes del path). */
static u32 spm_fs_reply_path(void __iomem *fs, u32 boff, const char *ap, u32 nf)
{
	u32 n = strlen(ap), i;

	for (i = 0; i < n; i++)
		writew((u16)(u8)ap[i], fs + boff + 0xc + i * 2);
	writew(0, fs + boff + 0xc + n * 2);	/* terminador */
	spm_fs_p0(fs, boff, nf);
	writel((n + 1) * 2, fs + boff + 8);	/* len del campo 0 = bytes UTF-16 */
	return 8 + (n + 1) * 2;
}

'''

if "H7j: enumeracion REAL" in s:
    print("  = motor: ya aplicado")
elif anchor1 in s:
    s = s.replace(anchor1, engine + anchor1, 1); print("  + motor de enumeracion")
else:
    print("  ! ancla spm_fs_path: NO MATCH"); ok = False

# ---- 2) engancharlo en el case 0x1010/0x1011 --------------------------------
anchor2 = """		/* H6l: formatos alternativos de respuesta, seleccionables en caliente.
		 * El modo 0 conserva el comportamiento actual (found/not-found real). */"""

hook = """		/*
		 * H7j: FindFirst (0x1010) / FindNext (0x1011) de VERDAD.
		 * El fsd real responde con la RUTA COMPLETA del fichero encontrado y el
		 * MD sigue con GetSize+READ; devolver un error (lo que haciamos) le mete
		 * en un bucle de recuperacion que nunca llega a NORMAL_BOOT_ID.
		 */
		if (spm_fs_enum) {
			if (op == 0x1010)
				spm_fs_enum_build(s);	/* FindFirst: (re)construir y cursor a 0 */
			else if (g_enum_i < g_enum_n)
				g_enum_i++;		/* FindNext: avanzar */

			if (g_enum_i < g_enum_n) {
				length = spm_fs_reply_path(fs, boff,
							   g_enum[g_enum_i],
							   spm_fs_find_nf);
				fs_dbg(s->dev, "H7j FIND op=%x [%d/%d] -> %s\\n",
				       op, g_enum_i, g_enum_n, g_enum[g_enum_i]);
			} else {			/* agotado: no hay mas ficheros */
				spm_fs_p0(fs, boff, spm_fs_find_nf);
				writel(4, fs + boff + 8);
				writel(spm_fs_1010_val, fs + boff + 0xc);
				length = 12;
				fs_dbg(s->dev, "H7j FIND op=%x -> AGOTADO (%d)\\n", op, g_enum_n);
			}
			break;
		}

""" + anchor2

if "H7j: FindFirst (0x1010) / FindNext" in s:
    print("  = enganche: ya aplicado")
elif anchor2 in s:
    s = s.replace(anchor2, hook, 1); print("  + enganche en el case 0x1010/0x1011")
else:
    print("  ! ancla del case 1010: NO MATCH"); ok = False

if ok:
    open(p, "w").write(s)
print("PATCH H7j " + ("OK" if ok else "INCOMPLETO"))
sys.exit(0 if ok else 1)
