p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

a = "static uint spm_md_misc_mask = 0x5;"
n = """/*
 * H8c: puertos TTY del CCCI — el bloqueo de la tarea UPS (ccci_uart_drv.c:2594).
 *
 * Desensamblado (0730) del assert en 0x230c: para un puerto cuyo bit NO esta en
 * la mascara de activos, el MD EXIGE que su descriptor de memoria compartida
 * este A CERO:
 *     r1 = tabla[puerto].descriptor
 *     if (*(r1+0) != 0) ASSERT      <- rx.read   (ya era 0)
 *     if (*(r1+8) != 0) ASSERT      <- rx.LENGTH (poniamos 16K)
 * H7r inicializaba rx.length/tx.length = 16K, lo que avanzo el assert de la
 * linea 793 a la 2594, pero el MD no quiere longitudes en puertos que el AP no
 * ha abierto.  Dos salidas posibles, ambas conmutables:
 *   spm_md_uart_ports = 0  -> no declarar ningun puerto
 *   spm_md_uart_len   = 0  -> declararlos con el descriptor entero a cero
 */
static uint spm_md_uart_ports = 6;
module_param(spm_md_uart_ports, uint, 0644);
MODULE_PARM_DESC(spm_md_uart_ports, "puertos TTY del CCCI a declarar (0..6)");
static uint spm_md_uart_len;		/* 0 = descriptor a cero (defecto nuevo) */
module_param(spm_md_uart_len, uint, 0644);
MODULE_PARM_DESC(spm_md_uart_len, "longitud declarada en rx/tx del TTY (0 = a cero)");

static uint spm_md_misc_mask = 0x5;"""
if "H8c: puertos TTY del CCCI" in s:
    print("= params ya aplicados")
elif a in s:
    s = s.replace(a, n, 1); print("+ params spm_md_uart_ports / spm_md_uart_len")
else:
    print("! NO MATCH params"); raise SystemExit(1)

# usar los params
s2 = s.replace("\twritel(CCCI_TTY_PORTS, smem + 12 * 4);\t\t\t\t/* UartPortNum */\n\tfor (i = 0; i < CCCI_TTY_PORTS; i++) {",
               "\twritel(spm_md_uart_ports, smem + 12 * 4);\t\t\t/* H8c: UartPortNum */\n\tfor (i = 0; i < spm_md_uart_ports && i < CCCI_TTY_PORTS; i++) {", 1)
if s2 == s:
    print("! NO MATCH bucle de puertos"); raise SystemExit(1)
s = s2
s = s.replace("\t\t\twritel(CCCI_TTY_BUF_SIZE, tty + 0x08);\t\t/* rx.length */",
              "\t\t\twritel(spm_md_uart_len, tty + 0x08);\t\t/* H8c: rx.length */", 1)
print("+ bucle y rx.length conmutables")
open(p, "w").write(s)

# tx.length (esta unas lineas mas abajo)
s = open(p).read()
import re
m = re.search(r"\t\t\twritel\(CCCI_TTY_BUF_SIZE, tty \+ 0x14\);[^\n]*\n", s)
if m:
    s = s[:m.start()] + "\t\t\twritel(spm_md_uart_len, tty + 0x14);\t\t/* H8c: tx.length */\n" + s[m.end():]
    open(p,"w").write(s); print("+ tx.length conmutable")
else:
    print("  (tx.length con otra forma, revisar)")
