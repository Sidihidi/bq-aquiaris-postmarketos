p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# 1) parametro nuevo, junto a los otros del bucle FS
old1 = """static uint spm_fs_fastpoll_iters = 10000;"""
new1 = """/*
 * H9a: iteraciones que se SIGUEN sirviendo despues del HS2. El bucle salia en
 * cuanto detectaba NORMAL_BOOT_ID, asi que el MD se quedaba sin servicio FS
 * justo al entrar en L1 (la capa de radio) y el registro de excepcion salia
 * limpio porque no le daba tiempo a llegar. 0 = comportamiento historico
 * (salir en el hito, deja la medida de M1 intacta).
 */
static uint spm_fs_post_hs2_iters;
module_param(spm_fs_post_hs2_iters, uint, 0644);
MODULE_PARM_DESC(spm_fs_post_hs2_iters,
		 "iteraciones que se siguen sirviendo tras el HS2 (0 = parar en el hito)");
static uint spm_fs_fastpoll_iters = 10000;"""
assert old1 in s
s = s.replace(old1, new1, 1)

# 2) el bucle: no salir en el HS2 si quedan iteraciones post
old2 = """		int done = 0, k;

		for (i = 0; i < (int)(spm_fs_fastpoll_iters + spm_fs_slow_iters) && !done; i++) {	/* fase rapida + 8s */"""
new2 = """		int done = 0, k, hs2_i = -1;

		for (i = 0; i < (int)(spm_fs_fastpoll_iters + spm_fs_slow_iters) &&
			    (!done || i - hs2_i < (int)spm_fs_post_hs2_iters); i++) {	/* fase rapida + 8s */"""
assert old2 in s
s = s.replace(old2, new2, 1)

# 3) el HS2 se anuncia UNA vez y anota en que iteracion fue
old3 = """				if (lch == 0 && d1 == 0 && rsv != 0x5555FFFF) {
					dev_info(s->dev, "*** HS2 LOGRADO: NORMAL_BOOT_ID (stage 2 = M1 COMPLETO) ***\\n");
					done = 1;
				}"""
new3 = """				if (lch == 0 && d1 == 0 && rsv != 0x5555FFFF) {
					if (!done) {
						dev_info(s->dev, "*** HS2 LOGRADO: NORMAL_BOOT_ID (stage 2 = M1 COMPLETO) ***\\n");
						hs2_i = i;	/* H9a: desde aqui cuentan las post-HS2 */
					}
					done = 1;
				}"""
assert old3 in s
s = s.replace(old3, new3, 1)

open(p, "w").write(s)
print("OK: spm_fs_post_hs2_iters anadido")
