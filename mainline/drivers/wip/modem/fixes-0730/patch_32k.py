p = "/home/cpcd/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c"
s = open(p).read()

# 1) parametro para barrer el support_mask en caliente
a = "static uint spm_fs_1010_mode = 9;"
n = """/*
 * H8b: support_mask del misc_info — el bloqueo de L1 (m12100.c:9064).
 *
 * Desensamblado (0730): el assert de la tarea L1 llama a una funcion en 0x6148
 * con (r0=1, r1=0, r2=0) y exige que devuelva 1 o 2.  Esa funcion hace:
 *     r1 = runtime[+0x108]        -> MiscInfoBase   (offset 264 = indice 66 ✓)
 *     if (*r1 != 'CCIF') return 0                   <- el prefijo SI lo escribimos
 *     return (*(r1+4) >> (r0*2)) & 3                <- support_mask, 2 BITS POR FEATURE
 * Con r0=1 mira los bits [3:2] = MISC_32K_LESS.  Escribiamos support_mask=0x1,
 * asi que esos bits valian 00 = FEATURE_NOT_EXIST -> devolvia 0 -> ASSERT.
 *
 * Enums del stock (ccci_common.h):
 *   estado : NOT_EXIST=0  NOT_SUPPORT=1  SUPPORT=2  PARTIALLY_SUPPORT=3
 *   feature: MISC_DMA_ADDR=0  MISC_32K_LESS=1  MISC_RAND_SEED=2  MD_COCLK=3
 * El MD necesita SABER si hay cristal de 32kHz; "no me consta" no le vale.
 * Candidatos (feature1 en bits[3:2]):
 *   0x5 = DMA_ADDR:1  32K_LESS:1(NOT_SUPPORT)   <- hay 32kHz externo
 *   0x9 = DMA_ADDR:1  32K_LESS:2(SUPPORT)       <- el sistema es "32k-less"
 *   0x6 / 0xa = idem con DMA_ADDR:2
 */
static uint spm_md_misc_mask = 0x5;
module_param(spm_md_misc_mask, uint, 0644);
MODULE_PARM_DESC(spm_md_misc_mask, "support_mask del misc_info: 2 bits por feature (acepta 0x...)");

static uint spm_fs_1010_mode = 9;"""
if "H8b: support_mask del misc_info" in s:
    print("= param ya aplicado")
elif a in s:
    s = s.replace(a, n, 1); print("+ param spm_md_misc_mask (default 0x5)")
else:
    print("! NO MATCH param"); raise SystemExit(1)

# 2) usarlo donde se rellena el misc_info
old = "\twritel(0x1, smem + 0x400 + 4);\t\t\t/* support_mask = FEATURE_SUPPORT<<MISC_DMA_ADDR */"
new = "\twritel(spm_md_misc_mask, smem + 0x400 + 4);\t/* H8b: 2 bits por feature */"
if "H8b: 2 bits por feature" in s:
    print("= escritura ya aplicada")
elif old in s:
    s = s.replace(old, new, 1); print("+ support_mask conmutable")
else:
    # buscar variante con espaciado distinto
    import re
    m = re.search(r"\twritel\(0x1, smem \+ 0x400 \+ 4\);[^\n]*\n", s)
    if m:
        s = s[:m.start()] + new + "\n" + s[m.end():]
        print("+ support_mask conmutable (variante)")
    else:
        print("! NO MATCH escritura"); raise SystemExit(1)
open(p, "w").write(s)
