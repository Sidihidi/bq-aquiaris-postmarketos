# 🎉 Módem H3 v3 — HS1 LOGRADO: el MD arranca y hace handshake (0717)

> **HITO: el módem MT6582 (MOLY) arranca por primera vez en mainline Linux y
> señaliza el primer handshake CCCI (HS1).** Reproducible: boot limpio + un
> `md_release` → `RCHNUM` del CCIF pasa de 0 a `0x1` a los ~100ms.

## Las DOS piezas que faltaban (esta sesión, capturando el arranque en LineageOS + RE del source)

1. **CCIF init (H3 v2)**: poner el mailbox AP en modo ARB (`CCIF_CON=0x1`) + `ACK=0xFF`
   antes de soltar el MD (`__ccif_v1_init` del stock). Necesario, no suficiente por sí solo.

2. **BANK0 remap (H3 v3) = LA CLAVE**: el MD arranca su boot ROM en **su dirección 0x0**
   (boot-slave Vector se escribe a 0x0). El stock hace TRES remaps, no uno
   (`ccci_platform.c` `ccci_ipo_h_platform_restore`:2538-2541):
   - `set_ap_smem_remap(0x40000000, smem)` → AP BANK4 (MCUSYS+0x200/0x204)
   - `set_md_smem_remap(0x40000000, smem)` → MD BANK4 (INFRACFG+0x308/0x30C)
   - **`set_md_rom_rw_mem_remap(0x00000000, md_mem)` → MD BANK0 (INFRACFG+0x300/0x304)** ← faltaba

   Nuestro H3 v1/v2 solo hacía BANK4 (la vista SMEM del MD en 0x40000000). Sin **BANK0**,
   la dir 0x0 del MD no apuntaba al firmware → al soltarlo, el MD ejecutaba memoria vacía
   → nunca hablaba por CCIF (RCHNUM=0). Con BANK0 mapeando MD-0x0 → firmware DRAM
   (0xb8000000), el MD ejecuta MOLY y hace HS1.

## Layout de memoria (confirmado del stock `mtk_ccci_helper.c`:475-476)

Carveout único de 24MB `modem-region@b8000000`:
- **Firmware (md_region_phy)** = base = **0xb8000000** ← BANK0 apunta aquí; el img se carga aquí
- **SMEM (smem_region_phy)** = base + MD1_MEM_SIZE(22MB) = **0xb9600000** ← BANK4 apunta aquí

(H3 v1/v2 conflaciaba ambos en 0xb8000000; corregido: BANK4 ahora → 0xb9600000.)

## Registros y fórmula BANK0 (implementado en `spm_md_remap`, mt6582-spm.c)

`set_md_rom_rw_mem_remap(md_id, src=0x0, des=0xb8000000)`:
```
des -= KERN_EMI_BASE(0x80000000)   -> 0x38000000
MAP0 = ((des>>24)|1)&0xFF  +  slots invalidos 7,8,9   -> 0x514f4d39
MAP1 = slots invalidos 10,11,12,13                    -> 0x59575553
write MAP0 -> INFRACFG(0x10001000)+0x300
write MAP1 -> INFRACFG(0x10001000)+0x304
```
Constantes: INVALID_ADDR=0x3E000000, INVALID_OFFSET=0x02000000. (BANK4 usa slots 0-6/14-20.)

## Evidencia (kernel #16, boot-md-test.img)

```
H3 load:  modem.img 5172580 B -> 0xb8000000; w0=0xe59ff018 (vector ARM de MOLY) OK
H3 v3 remap: BANK0(fw 0xb8000000)=514f4d39/59575553  BANK4(smem 0xb9600000) ...
H3 v2: CCIF init -> CON=0x1  (ARB)
H3 v2: *** HANDSHAKE! @t=100ms CON=0x1 BUSY=0x0 RCHNUM=0x00000001 RX0=0xffffffff ***
```
AP sobrevive (uptime sigue). Reproducible tras reboot. ⚠️ El MD arranca limpio UNA vez
por ciclo de power (un 2º `md_release` sin power-cycle no rehace HS1 — el MD ya está en
su estado post-boot esperando la respuesta).

## SIGUIENTE: HS2 (= M1 COMPLETO)

Tras HS1 el MD espera la **runtime data** del AP. Falta portar `set_md_runtime`
(`ccci_md_main.c:1373-1656`): escribe la geometría de shared-mem + `config_misc_info`
(magic "CCIF" + `get_md_mem_start_addr`) al SMEM (0xb9600000) y responde por CCIF
(ACK del canal + TX). El MD entonces avanza a `NORMAL_BOOT_ID` → stage 2 = **M1 completo**
(`/sys/kernel/ccci/boot` daría `md1:2`, como en Lineage). Es el grueso restante (~la
estructura `modem_runtime_t`), pero el arranque del MD — el mayor unknown — está RESUELTO.

## Ficheros
- `mt6582-spm-H1.c` (este dir) = tree #16, con H1+H2+H3(load/remap-BANK0+BANK4/CCIF-init/release).
- Triggers: `/sys/module/mt6582_spm/parameters/spm_md_{poweron,load,remap,release,dump}`.
- Fuente stock: `ccci_platform.c` (set_md_rom_rw_mem_remap:2474, remap caller:2538),
  `ccci_platform.h` (MD1_BANK0_MAP0/1:149), `mtk_ccci_helper.c` (layout:475).
- Contexto: H3v2-CCIF-INIT-0717.md, H3-BRINGUP-CCCI-SPEC-0713.md.
