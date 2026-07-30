p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# parametro para poder hacer A/B
a = "static uint spm_md_uart_ports = 6;"
n = """/*
 * H8e: ZERAR las regiones de control de RED — el assert de UPS
 * (ccci_uart_drv.c:2594).
 *
 * Localizado el inicializador en 0x225a del firmware:
 *     r2 = 0xf0873eb8  (la copia interna del runtime, la misma que mira L1)
 *     r0 = *(r2 + 0xb8)   = NetULCtrlShareMemBase[0]   (offset 184)
 *     r6 = *(r2 + 0xd8)   = NetDLCtrlShareMemBase[0]   (offset 216)
 *     str r0, [r3, #4]    -> tabla[puerto].campo_4 = NetULCtrlBase
 * Y el assert exige que *(campo_4 + 0) y *(campo_4 + 8) sean CERO para un canal
 * que no esta activo.  Declarabamos las 6 regiones (indices 46-48 y 54-56) pero
 * NUNCA las zerabamos -> basura del carveout -> assert.  Mismo patron que el
 * misc_info (ver el comentario del memset ahi).
 *
 * Nota: el fichero se llama ccci_uart_drv.c pero es el driver GENERICO de
 * puertos del CCCI; la tabla es de red, no de UART.  Por eso tocar los
 * descriptores TTY (H8c/H8d) no cambio nada.
 */
static uint spm_md_zero_net = 1;
module_param(spm_md_zero_net, uint, 0644);
MODULE_PARM_DESC(spm_md_zero_net, "1 = zerar las regiones NetUL/DLCtrl antes de arrancar el MD");

static uint spm_md_uart_ports = 6;"""
if "H8e: ZERAR las regiones" in s:
    print("= param ya aplicado")
elif a in s:
    s = s.replace(a, n, 1); print("+ param spm_md_zero_net")
else:
    print("! NO MATCH param"); raise SystemExit(1)

# zerar justo despues de declararlas
a2 = "\twritel(0x8000, smem + 58 * 4); writel(0x8000, smem + 59 * 4); writel(0x8000, smem + 60 * 4); /* DLCtrlSize[0-2] */"
n2 = a2 + """

	/* H8e: zerar las 6 regiones de control de red (el MD exige leer ceros ahi). */
	if (spm_md_zero_net) {
		static const u32 netoff[6] = { 0x120000, 0x130000, 0x140000,
					       0x150000, 0x160000, 0x170000 };
		int z;

		for (z = 0; z < 6; z++) {
			void __iomem *nr = ioremap(MD_SMEM_PHYS + netoff[z], 0x8000);
			u32 w;

			if (!nr) {
				dev_warn(s->dev, "H8e: no pude mapear net+0x%x\\n", netoff[z]);
				continue;
			}
			for (w = 0; w < 0x8000; w += 4)
				writel(0, nr + w);
			iounmap(nr);
		}
		wmb();
		dev_info(s->dev, "H8e: 6 regiones NetUL/DLCtrl zeradas (6 x 32KB)\\n");
	}"""
if "H8e: zerar las 6 regiones" in s:
    print("= zerado ya aplicado")
elif a2 in s:
    s = s.replace(a2, n2, 1); print("+ zerado de las 6 regiones de red")
else:
    print("! NO MATCH zerado"); raise SystemExit(1)
open(p, "w").write(s)
