p = "/home/cpcd/darwin-krillin/xnu/osfmk/arm/trap.c"
s = open(p).read()

FIX = """                    /*
                     * ==== M128: EL FIX ====
                     * Invalidar la TLB antes de reintentar la instruccion.
                     *
                     * La rama de KERNEL ya lo hacia desde REALVIEW/0723; la de
                     * USUARIO se quedo sin ello, y ese era el bucle infinito:
                     * dyld escribe en 0x94000 -> fallo de PERMISOS (FS=0xF,
                     * WnR=1) -> vm_fault lo resuelve y pmap_enter deja la PTE
                     * en RW (medido en M127: L2=0x846861bb, AP=RW) -> se
                     * reintenta la instruccion CON LA ENTRADA VIEJA DE SOLO
                     * LECTURA TODAVIA EN LA TLB -> vuelve a fallar -> para
                     * siempre (M126: NF y NFR crecian igual = misma pagina).
                     *
                     * Los primeros ~50 fallos SI progresaban porque eran de
                     * traduccion (FS=0x7) sobre paginas sin entrada previa en
                     * la TLB: no habia nada obsoleto que estorbara.  El bucle
                     * arranca justo con el primer fallo de PERMISOS.
                     *
                     * TLBIMVA usa VA|ASID; como aqui pasamos la VA con ASID 0
                     * y el proceso corre con otro (CTXID=2 en M127), anadimos
                     * TLBIALL, que barre la TLB entera. Igual que en kernel.
                     */
                    {
                        uint32_t _va = (uint32_t) (%%ADDR%%) & ~0xFFFu;
                        __asm__ volatile ("dsb" ::: "memory");
                        __asm__ volatile ("mcr p15, 0, %0, c8, c7, 1" :: "r"(_va) : "memory"); /* TLBIMVA */
                        __asm__ volatile ("mcr p15, 0, %0, c8, c7, 0" :: "r"(0) : "memory");   /* TLBIALL */
                        __asm__ volatile ("dsb\\n\\tisb" ::: "memory");
                    }
"""

# 1) DATA abort de usuario (el bucle medido)
old_data = """                } else {
                    /*
                     * Retry execution of instruction.
                     */
                    ml_set_interrupts_enabled(TRUE);
                    return;
                }
                break;
            }
        default:
            exception_type = EXC_BREAKPOINT;"""
new_data = """                } else {
""" + FIX.replace("%%ADDR%%", "dfar") + """                    /*
                     * Retry execution of instruction.
                     */
                    ml_set_interrupts_enabled(TRUE);
                    return;
                }
                break;
            }
        default:
            exception_type = EXC_BREAKPOINT;"""
if old_data not in s:
    print("NO MATCH data abort usuario"); raise SystemExit(1)
s = s.replace(old_data, new_data, 1)
print("fix aplicado: data abort de usuario")

# 2) PREFETCH abort de usuario (mismo razonamiento, por simetria)
import re
idx = s.find("usermode prefetch abort")
tail = s[idx:]
old_pf = """                } else {
                    /*
                     * Retry execution of instruction.
                     */
                    ml_set_interrupts_enabled(TRUE);
                    return;
                }"""
if old_pf in tail:
    new_pf = """                } else {
""" + FIX.replace("%%ADDR%%", "arm_ctx->pc") + """                    /*
                     * Retry execution of instruction.
                     */
                    ml_set_interrupts_enabled(TRUE);
                    return;
                }"""
    s = s[:idx] + tail.replace(old_pf, new_pf, 1)
    print("fix aplicado: prefetch abort de usuario")
else:
    print("AVISO: prefetch de usuario con otra forma, no tocado")

open(p, "w").write(s)
