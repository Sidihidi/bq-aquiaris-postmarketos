import re
f="/home/cpcd/mainline/linux-7.0.12/drivers/i2c/busses/i2c-mt65xx.c"
s=open(f).read()
if "mt6582_compat" not in s:
    anchor = s.index("static const struct mtk_i2c_compatible mt6589_compat")
    block = ("static const struct mtk_i2c_compatible mt6582_compat = {\n"
             "\t.regs = mt_i2c_regs_v1,\n"
             "\t.pmic_i2c = 0,\n"
             "\t.dcm = 1,\n"
             "\t.auto_restart = 1,\n"
             "\t.aux_len_reg = 0,\n"
             "\t.timing_adjust = 0,\n"
             "\t.dma_sync = 0,\n"
             "\t.ltiming_adjust = 0,\n"
             "\t.apdma_sync = 0,\n"
             "\t.max_dma_support = 32,\n"
             "};\n\n")
    s = s[:anchor] + block + s[anchor:]
    s = s.replace('{ .compatible = "mediatek,mt6577-i2c", .data = &mt6577_compat },',
                  '{ .compatible = "mediatek,mt6577-i2c", .data = &mt6577_compat },\n\t{ .compatible = "mediatek,mt6582-i2c", .data = &mt6582_compat },')
    open(f,"w").write(s)
    print("i2c-mt65xx.c: mt6582_compat (auto_restart=1) + of_match anadidos")
else:
    print("i2c-mt65xx.c ya tiene mt6582_compat")
d="/home/cpcd/mainline/linux-7.0.12/arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts"
ds=open(d).read()
if '"mediatek,mt6582-i2c"' not in ds:
    ds=ds.replace('compatible = "mediatek,mt6577-i2c";','compatible = "mediatek,mt6582-i2c";')
    open(d,"w").write(ds); print("DT: compatible -> mt6582-i2c")
else: print("DT ya usa mt6582-i2c")
