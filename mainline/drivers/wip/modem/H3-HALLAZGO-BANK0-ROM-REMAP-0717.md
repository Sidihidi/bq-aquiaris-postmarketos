# Módem H3 — LA PIEZA QUE FALTABA: el ROM-remap (BANK0). Ground-truth LineageOS — 0717

> Capturado del arranque REAL del MD en LineageOS 13 (adb, dmesg del boot a los ~16s, buffer
> aún desde 0s). **El H3 nunca llegó a HS1 porque le faltaba el remap de la ROM del MD (BANK0):
> el MD arranca ejecutando desde su dirección 0x0, y H3 solo mapeó el BANK4 (SMEM @0x40000000),
> dejando 0x0 sin apuntar al firmware.** Con el BANK0 añadido, el MD debería ejecutar MOLY y
> hacer HS1 → luego HS2 con el runtime.

## El mapa de memoria del MD (de la captura, carveout Lineage @0xBC000000)

```
[ccci/ctl] SmemStart(0xBD600000):SmemSize(0x00200000)                 <- SMEM 2MB, SEPARADO de la ROM
[ccci_plat] MD1 Smem remap:[3d600000]->[40000000](4543413d:4d4b4947)  <- BANK4: MD 0x40000000 -> SMEM
[ccci_plat] MD1 ROM  remap:[3c000000]->[00000000](53514f3d:5b595755)  <- BANK0: MD 0x00000000 -> ROM(fw)  ★
[ccci_plat] md1: md_rom<P:0xbc000000>, md_smem<P:0xbd600000>
```

**Son DOS regiones separadas y TRES remaps** (H3 solo hizo los dos de SMEM):
| remap | registro (infracfg=0x10001000 / mcusys=0x10200000) | mapea | H3? |
|---|---|---|---|
| `set_ap_smem_remap`  | `AP_BANK4_MAP0/1`  = `0x10200200/204` | AP ve SMEM  | ✅ |
| `set_md_smem_remap`  | `MD1_BANK4_MAP0/1` = `0x10001308/30C` | MD 0x40000000 → SMEM | ✅ |
| **`set_md_rom_rw_mem_remap`** | **`MD1_BANK0_MAP0/1` = `0x10001300/304`** | **MD 0x00000000 → ROM(firmware)** | ❌ **FALTABA** |

## La fórmula del remap (idéntica para los 3, cambia el slot INVALID)

```c
/* KERN_EMI_BASE=0x80000000, INVALID_ADDR[MD1]=0x3E000000, INVALID_OFFSET=0x02000000 */
des -= KERN_EMI_BASE;              /* AP-phys -> valor de 26 bits */
map0 = ((des>>24)|0x1)&0xFF                                               /* slot0 = destino real */
     + ((((INVALID_ADDR + INVALID_OFFSET*A)>>16)|1<<8)  & 0xFF00)        /* slots invalidos */
     + ((((INVALID_ADDR + INVALID_OFFSET*B)>>8) |1<<16) & 0xFF0000)
     + ((((INVALID_ADDR + INVALID_OFFSET*C)>>0) |1<<24) & 0xFF000000);
/* slots por banco:  BANK4(smem)=0..6 ; BANK0(rom)=7..13 ; AP_BANK4=14..20 */
```

**Verificado con la captura**: ROM des=0xBC000000 → 0x3C000000, byte0=(0x3C|1)=0x3D → MAP0=0x53514f3**d** ✓.

### Valores para NUESTRO carveout (ROM @0xB8000000, SMEM @0xB9600000)
- **BANK0 (ROM, MD 0x0 → 0xB8000000)**: des=0x38000000, byte0=0x39 →
  `MD1_BANK0_MAP0(0x10001300)=0x53514f39`, `MD1_BANK0_MAP1(0x10001304)=0x5b595755`
- **BANK4 (SMEM, MD 0x40000000 → 0xB9600000)**: des=0x39600000, byte0=0x39 →
  `MD1_BANK4_MAP0(0x10001308)=0x45434139`, `MD1_BANK4_MAP1(0x1030C)=0x4d4b4947`
  (⚠️ H3 usaba des=0xB8000000 para BANK4 = apuntaba a la ROM, no al SMEM separado — corregir)
- AP_BANK4 (0x10200200/204): igual que el MD_BANK4 pero desde mcusys.

**Nota carveout**: ROM 22MB @0xB8000000 (la img MOLY 5.17MB cabe) + SMEM 2MB @0xB9600000 = 24MB
(coincide con el `modem-region@b8000000` ya reservado en H2b). SMEM va DESPUÉS de la región ROM.

## Secuencia completa del stock (de la captura, en orden)
1. (init) los 3 remaps: BANK0 ROM, BANK4 SMEM, AP_BANK4.
2. `Clear MD1 region protect` + `Clear MPU protect MD1 ROM region` + `R/W region` → **MPU OFF** en
   bring-up (H3 ya lo dejaba off, correcto).
3. cargar firmware a la ROM (0xB8000000), `check_md_header` (GFH al final, warning de tamaño = benigno).
4. `ungate_md1`: md_power_on (MTCMOS, = nuestro H1 1:1) → WDT off → boot-slave Key/Vector/En.
5. **HS1**: `receive MD_INIT_START_BOOT` — el MD ejecuta MOLY y habla por CCIF (lo que nunca llegó).
6. `set runtime data: size=280` → escribe `modem_runtime_t` (280B) al SMEM runtime-region + envía
   `MD_INIT_START_BOOT` de vuelta por CCIF.
7. **HS2**: `receive NORMAL_BOOT_ID` (~100ms tras HS1) → **MD LISTO (M1 completo)**.

## El runtime para HS2 (capturado + derivado)
`modem_runtime_t` = **280 bytes**, todos int32 en orden fijo (struct en `ccci_md.h`). Campos clave
capturados de Lineage:
- `Prefix`/`Postfix` = `0x46494343` ("CCIF")
- `Platform_L`/`Platform_H` = **"MT6582E1"** (`0x3536544d` / `0x31453238`)  ← nota: "MT65"+"82E1"
- `DriverVersion` = **`0x20121001`**
- `BootChannel` = CCCI_CONTROL_RX ; `BootingStartID` = **`0x0`** (NORMAL_BOOT_ID, sin mdlogger)
- Sub-regiones ShareMem (Exce/Misc/Pcm/Mdlog/Sys/Fs/…): base = `smem_sub_phy − md_2_ap_phy_addr_offset`,
  con `md_2_ap_phy_addr_offset = (smem_base & 0xFE000000) − 0x40000000`. Layout SECUENCIAL desde la base
  del SMEM: runtime_data(280) → exp(2K) → md_ex_exp_info(sizeof) → misc_info(1K) → [round 0x1000] → pcm…
  La mayoría pueden ir a 0 si su size=0; las esenciales: Exce (512B+), Misc (config_misc_info: "CCIF"
  + `get_md_mem_start_addr`), Sys.
- `CheckSum` (ver `platform_set_runtime_data`), `Postfix`.

## Próximo paso (implementación)
1. **Añadir el BANK0 ROM-remap** (2 writes a 0x10001300/304) + corregir el BANK4 a la base del SMEM
   separado (0xB9600000) en el módulo H3 → **debería dar HS1** (el bloqueo real).
2. Con HS1, implementar la respuesta HS2: montar el layout del SMEM, rellenar `modem_runtime_t` (280B,
   valores de arriba), `config_misc_info`, y enviar el `ccci_msg_t`(id=MD_INIT_START_BOOT) por CCIF
   (`ccci_message_send` → CCIF TXCHDATA + trigger). → **NORMAL_BOOT_ID = M1 completo**.

Captura cruda: `lineage-md-boot.log` (scratchpad). Firmware/H0-H2 sin cambios. Método: LineageOS vivo
(flash `lineage13-boot.img` por dd desde pmOS; volver por `fastboot flash boot`), adb + dmesg del boot
capturado a los ~16s (el buffer aún alcanza 0s). El dump DEBUG del runtime está off; los INF (Platform/
DriverVersion/BootingStartID) sí salen. `svc power stayon true` evita que el USB se suspenda.

*2026-07-17, sesión Windows (Fable 5). El ground-truth de Lineage destapó el ROM-remap que faltaba.*
