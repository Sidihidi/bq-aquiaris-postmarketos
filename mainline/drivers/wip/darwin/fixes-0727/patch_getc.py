p = "/home/cpcd/darwin-krillin/xnu/pexpert/arm/pe_mt6582.c"
s = open(p).read()
old = """    {
        unsigned int spin;
        for (spin = 0; spin < MT6582_UART_SPIN_MAX; spin++) {
            if (HwReg(gMT6582UartBase + UART_LSR) & UART_LSR_DR)
                break;
            barrier();
        }
        if (spin >= MT6582_UART_SPIN_MAX)
            return -1;
    }
    return (int) (HwReg(gMT6582UartBase + UART_RBR) & 0xff);"""
new = """    /*
     * M130: sondeo INSTANTANEO, sin espera.
     *
     * Antes esto giraba MT6582_UART_SPIN_MAX (100000) veces esperando a que
     * llegara una tecla, con un acceso MMIO al APB en cada vuelta (~10 ms por
     * llamada si nadie teclea, que es lo normal).  Y serial_keyboard_poll() lo
     * llama CADA 16 ms desde un hilo a PRIORIDAD 95, que expulsa a todo
     * userland (launchd va a 31): medido en M129, el 85% de las muestras del
     * tick caian aqui dentro.  Era el freno de todo el arranque.
     *
     * La espera acotada solo tiene sentido al TRANSMITIR (esperar THRE); al
     * leer hay que mirar una vez y volver.  serial_keyboard_poll ya llama en
     * bucle hasta que devolvemos -1 ("get a character if there is one").
     */
    if (!(HwReg(gMT6582UartBase + UART_LSR) & UART_LSR_DR))
        return -1;
    return (int) (HwReg(gMT6582UartBase + UART_RBR) & 0xff);"""
if old not in s:
    print("NO MATCH getc"); raise SystemExit(1)
open(p, "w").write(s.replace(old, new, 1))
print("mt6582_getc: sondeo instantaneo (M130)")
