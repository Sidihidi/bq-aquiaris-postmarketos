p = "/home/cpcd/darwin-krillin/xnu/osfmk/arm/pmap.c"
s = open(p).read()

# 1) invalidar la I-cache tras escribir la PTE en pmap_enter_options
old = """    { extern void arm_dcache_wbinv_all(void); arm_dcache_wbinv_all(); }
    __asm__ volatile ("dsb\\n\\tisb" : : : "memory");"""
new = """    { extern void arm_dcache_wbinv_all(void); arm_dcache_wbinv_all(); }
    __asm__ volatile ("dsb\\n\\tisb" : : : "memory");

    /*
     * M132: invalidar tambien la CACHE DE INSTRUCCIONES.
     *
     * Arriba solo se limpiaba la D-cache.  Al traer una pagina de CODIGO por
     * demand paging, el contenido llega a RAM (eso lo cubre el wbinv de arriba)
     * pero la I-cache puede seguir con lineas VIEJAS de lo que hubiera antes en
     * esa direccion -> la CPU ejecuta basura.  Sintoma medido en M131: launchd
     * atascado en un bucle de "instruccion indefinida" en _tmcomp+0x2a
     * (libsystem_c) donde el binario tiene un `ldr.w r2,[r12,#4]` perfectamente
     * valido en Thumb-2: no era la instruccion, era lo que la CPU LEIA.
     *
     * XNU llama a pmap_sync_page_data_phys() justo para esto, pero en este port
     * era un stub vacio (ver mas abajo, tambien arreglado).
     */
    __asm__ volatile ("mcr p15, 0, %0, c7, c5, 0" :: "r"(0) : "memory");  /* ICIALLU */
    __asm__ volatile ("mcr p15, 0, %0, c7, c5, 6" :: "r"(0) : "memory");  /* BPIALL  */
    __asm__ volatile ("dsb\\n\\tisb" : : : "memory");"""
if old not in s:
    print("NO MATCH pmap_enter"); raise SystemExit(1)
s = s.replace(old, new, 1)

# 2) implementar los stubs de sincronizacion de pagina
old2 = """void pmap_sync_page_data_phys(__unused ppnum_t pa)
{
    Debugger("pmap_sync_page_data_phys");
    return;
}"""
new2 = """void pmap_sync_page_data_phys(ppnum_t pa)
{
    /*
     * M132: implementado (era un stub que solo llamaba a Debugger()).
     * XNU invoca esto tras cargar el contenido de una pagina para que el codigo
     * recien traido sea visible al fetch de instrucciones: hay que bajar los
     * datos hasta el punto de unificacion y tirar la I-cache.
     */
    extern unsigned long gPhysBase, gVirtBase;
    vm_offset_t va, end, l;

    va = (vm_offset_t) (((uint32_t) pa << PAGE_SHIFT) - gPhysBase) + gVirtBase;
    end = va + PAGE_SIZE;
    for (l = va & ~31UL; l < end; l += 32)
        __asm__ volatile ("mcr p15, 0, %0, c7, c11, 1" :: "r"(l) : "memory"); /* DCCMVAU */
    __asm__ volatile ("dsb" ::: "memory");
    __asm__ volatile ("mcr p15, 0, %0, c7, c5, 0" :: "r"(0) : "memory");      /* ICIALLU */
    __asm__ volatile ("mcr p15, 0, %0, c7, c5, 6" :: "r"(0) : "memory");      /* BPIALL  */
    __asm__ volatile ("dsb\\n\\tisb" ::: "memory");
    return;
}"""
if old2 not in s:
    print("NO MATCH sync_page_data"); raise SystemExit(1)
s = s.replace(old2, new2, 1)

old3 = """void pmap_sync_page_attributes_phys(ppnum_t pa)
{
    Debugger("pmap_sync_page_attributes_phys");
    return;
}"""
new3 = """void pmap_sync_page_attributes_phys(ppnum_t pa)
{
    /* M132: igual que pmap_sync_page_data_phys (era otro stub con Debugger). */
    pmap_sync_page_data_phys(pa);
    return;
}"""
if old3 not in s:
    print("NO MATCH sync_page_attributes"); raise SystemExit(1)
s = s.replace(old3, new3, 1)
open(p, "w").write(s)
print("pmap.c: I-cache invalidada + stubs implementados (M132)")
