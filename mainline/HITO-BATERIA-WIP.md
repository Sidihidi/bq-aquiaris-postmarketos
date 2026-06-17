# 🏆 Batería % (lectura de VBAT) en mainline — RESUELTO (2026-06-17)

**RESUELTO:** lector userspace `/usr/local/bin/battery` (repo `mainline/rootfs/battery`)
por `pwrap_poke`, SIN recompilar kernel. Verificado: **VBAT 3719 mV (~33%)**, estable.
Las direcciones que faltaban (de `upmu_common.c` downstream):
- **OUT+RDY de BATSNS = `AUXADC_ADC0` = `0x0714`** (bits0-14 valor, bit15 ready). ← el HITO escaneó la zona CON (0x742+) y por eso no lo encontró; está en la zona ADC (0x0714-0x073A).
- **VBUF_EN = `AUXADC_CON11` = `0x0758` bit4**.
- request canal 7 = **CON22 `0x076E` bit7** (ya venía set en vivo, 0x00a0).
- El `PMIC_IMM_PollingAuxadcChannel()` solo limpia ADC_DECI_GDLY (CON19 0x0768) — no hacía falta.

Pendiente (no bloqueante): curva OCV→% real (hoy piecewise interina). El **voltaje es exacto**.

---
(Notas de investigación originales abajo.)

## De dónde sale VBAT
`battery_meter_hal.c`: `PMIC_IMM_GetOneChannelValue(VBAT_CHANNEL_NUMBER=7)` → **AUXADC del
PMIC MT6323, canal 7 = BATSNS**. NO el AUXADC del SoC (0x11001000). Leíble por pwrap
(I2C/pwrap @0x1000D000), SIN recompilar kernel — si se inicializa el AUXADC.

## Secuencia (de pmic_mt6323.c:210 PMIC_IMM_GetOneChannelValue)
Mapa de canales: 0=BATON2,1=CH6,2=THRSENSE2,3=THRSENSE1,4=VCDT,5=BATON1,6=ISENSE,
**7=BATSNS(VBAT)**,8=ACCDET,9-16=audio.
1. `PMIC_IMM_PollingAuxadcChannel()` — **paso de init/poll del AUXADC (PENDIENTE de extraer)**.
2. `upmu_set_rg_vbuf_en(1)` — VBUF_EN = **bit4** de un reg AUXADC (dir exacta pendiente).
3. CON22 (`0x076E`, RQST_LIST mask 0x1FF): leer, **clear bit7**, escribir; leer, **set bit7**, escribir → pide canal 7. (Nota: en vivo CON22=0x00a0 ya tiene bit7+bit5 set.)
4. `udelay(300)`; poll ready = **ADC_RDY_BATSNS = bit15** del reg OUT.
5. Leer OUT = **ADC_OUT_BATSNS = bits0-14 (mask 0x7FFF)** del mismo reg (dir exacta pendiente).
6. **VBAT_mV = raw × 4 × 1800 / 32768** (r_val=4, VOLTAGE_FULL_RANGE=1800, ADC_PRECISE=32768).
   → 4.2V ≈ raw 19117; 3.7V ≈ raw 16839.

## Registros (upmu_hw.h, PMIC_REG_BASE=0 → dir = offset)
- AUXADC_CON0=0x0742 ... CON22(RQST)=0x076E, CON23(RQST_RSV)=0x0770 ... CON27=0x0778.
- ADC_OUT_BATSNS: mask 0x7FFF shift 0; ADC_RDY_BATSNS: mask 0x1 shift 15 (mismo reg, **dir pendiente**).
- RG_VBUF_EN: mask 0x1 shift 4 (**dir pendiente**); RG_AP_RQST_LIST: mask 0x1FF shift 0 (= CON22).
- Escaneo live AUXADC 0x742-0x778 (CON22=0xa0; 0x762=0xfd12, 0x764=0xfb77 con bit15 pero valores ~32000 → NO son VBAT plausible → confirma que el AUXADC no está dando lecturas válidas sin init).

## Lo que FALTA
1. Extraer `PMIC_IMM_PollingAuxadcChannel()` (init del AUXADC) de pmic_mt6323.c.
2. Direcciones exactas de los regs OUT(BATSNS) y VBUF_EN (empírico: tras init+request, ver qué reg da bit15+~19000; o de los accesores upmu_*).
3. Curva OCV (voltaje→%): cust_battery_meter_table.h tiene la R-table; falta la ZCV (voltaje→%). Interim: piecewise (3.40V=0, 3.70=30, 3.85=60, 4.00=80, 4.20=100%).
4. Implementar `/usr/local/bin/battery` (pwrap_poke) + verificar lectura plausible (3.5-4.2V).

## Recomendación
Dos caminos: **(A)** completar el lector userspace por pwrap (extraer la init del AUXADC —
medio día de reverse). **(B)** llevar el MT6323 al DeviceTree (pwrap mt6582 compat + mfd +
driver ADC) — el "hub" del ROADMAP que ADEMÁS desbloquea audio y power de WiFi; más limpio
y reutilizable. Probablemente (B) cuando ataquemos el PMIC. La CARGA ya funciona (HITO-CARGA-FAN5405);
esto es solo para MOSTRAR el %.
