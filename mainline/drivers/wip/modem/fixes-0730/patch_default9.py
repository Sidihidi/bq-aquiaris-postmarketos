p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()
old = "static uint spm_fs_1010_mode;"
new = """/*
 * H8a: modo 9 POR DEFECTO — es el que arranca el MD.
 *
 * Medido 0730 en HW: con el default anterior (0) el arranque se quedaba en 2
 * operaciones; con el modo 9 (respuesta de forma corta: 1 campo de 4 bytes, sin
 * tocar +4) el MD monta su NVRAM ENTERA y alcanza
 *   *** HS2 LOGRADO: NORMAL_BOOT_ID (stage 2 = M1 COMPLETO) ***
 * Con spm_fs_1010_val=0 (el default) hace 894 peticiones, contra las 883 del
 * ccci_fsd real (gd-boot-full.out) -> el mount es equivalente al de fabrica.
 * Con 0xffffffff tambien llega a HS2 pero solo con 476 (mount a medias).
 */
static uint spm_fs_1010_mode = 9;"""
if "H8a: modo 9 POR DEFECTO" in s:
    print("ya aplicado")
elif old in s:
    open(p, "w").write(s.replace(old, new, 1)); print("+ modo 9 por defecto")
else:
    print("NO MATCH"); raise SystemExit(1)
