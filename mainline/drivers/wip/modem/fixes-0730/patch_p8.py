p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = "#define CCCI_TTY_PORTS\t\t6\t\t/* CCCI_TTY_PORT_COUNT */"
new = """/* H8d: el runtime declara UartShareMemBase[8] ("Current UART_MAX_PORT_NUM is 8"
 * en ccci_md.h), no 6.  El assert de UPS (ccci_uart_drv.c:2594) indexa los
 * puertos como (canal_CCCI - 37), asi que puede haber mas de 6.  Se sube el tope
 * a 8 y el numero declarado se barre con spm_md_uart_ports. */
#define CCCI_TTY_PORTS\t\t8"""
if "H8d: el runtime declara" in s:
    print("ya aplicado")
elif old in s:
    open(p,"w").write(s.replace(old,new,1)); print("+ tope de puertos a 8")
else:
    print("NO MATCH"); raise SystemExit(1)
