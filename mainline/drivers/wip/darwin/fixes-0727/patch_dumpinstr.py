p = "/home/cpcd/darwin-krillin/xnu/osfmk/arm/trap.c"
s = open(p).read()
old = """        if (nundef < 6) {
            nundef++;
            PE_early_puts("INSTR ILEGAL pc=");
            PE_early_puthex(state->pc);
            PE_early_puts(" cpsr=");
            PE_early_puthex(state->cpsr);
            PE_early_puts("\\n");
        }"""
new = """        if (nundef < 8) {
            nundef++;
            PE_early_puts("INSTR ILEGAL pc=");
            PE_early_puthex(state->pc);
            PE_early_puts(" cpsr=");
            PE_early_puthex(state->cpsr);
            /*
             * M133: volcar los BYTES que hay de verdad en memoria alrededor del
             * pc.  M132 dejo a dos procesos colgados en el MISMO punto de
             * libsystem_c (_tmcomp+0x2a), donde el binario tiene un
             * `ldr.w r2,[r12,#4]` (f8dc2004) perfectamente valido en Thumb-2.
             * Si lo que leemos aqui NO es f8dc2004, el contenido de la pagina
             * es incorrecto (paginacion/caches) y la instruccion no tiene la
             * culpa.  Si SI coincide, el problema es que la CPU rechaza una
             * instruccion legal -> mirar enrutado de excepciones y CP15.
             */
            {
                unsigned int w[3];
                user_addr_t base = (user_addr_t) ((state->pc & ~1u) - 4);
                if (copyin(base, (char *) w, sizeof(w)) == 0) {
                    PE_early_puts(" mem[-4]="); PE_early_puthex(w[0]);
                    PE_early_puts(" [0]=");     PE_early_puthex(w[1]);
                    PE_early_puts(" [+4]=");    PE_early_puthex(w[2]);
                } else {
                    PE_early_puts(" (copyin FALLO)");
                }
            }
            PE_early_puts("\\n");
        }"""
if old not in s:
    print("NO MATCH"); raise SystemExit(1)
open(p, "w").write(s.replace(old, new, 1))
print("sleh_undef: volcado de bytes reales (M133)")
