# HANDOFF — Port del AUDIO MT6582 a mainline (mapa verificado + plan por fases) (Mac, 2026-07-02)

## Arquitectura (verificada contra el downstream)
- **AFE** (Audio Front End, digital+DMA+I2S): bloque del SoC @ **0x11220000** (64 KB)
  ✓ verificado `AudDrv_Afe.h:408` + ioremap en `AudDrv_Kernel.c:1024`.
  Ficheros: `downstream/drivers/misc/mediatek/sound/mt6582/AudDrv_{Afe,Ana,Clk,Kernel}.c`.
- **ANA** (codec analógico): registros **dentro del PMIC MT6323 vía pwrap** (Ana_Set_Reg → pwrap).
  Speaker amp: `SPK_CON0..12`; DAC/HP: `AUDTOP_CON0..9`; ADDA: `ABB_AFE_CON0..11` (base 0x4000).
- **Amplificador EXTERNO de altavoz**: enable por **GPIO118** ✓ verificado
  (`krillin/dct/dct/cust_gpio_usage.h:295 GPIO_SPEAKER_EN_PIN=GPIO118`), secuencia de wake
  3× toggle HIGH→LOW + 40 ms (`krillin/sound/yusu_android_speaker.c`).
- **Secuencia power-on ANA** ✓ verificada (`mt6582/AudDrv_Kernel.c:278-286 power_init`):
  `clksq_en_aud(1)` → AUDTOP_CON0=0x0002/0x000F → CON1=0x0020/0x00F0 → CON5=0x1114 (DAC bias 50%)
  → CON6=0x37A2→0x37E2 (depop+HP driver).
- ⚠ IRQ del AFE: el informe del agente dice GIC_SPI 104 — **NO verificado aún** (buscar el
  request_irq en AudDrv_Kernel.c antes de la Fase A real).

## Registros AFE clave (playback DL1)
`AUDIOAFE_TOP_CON0`(0x0000: bit2 AFE_EN, bit6 I2S_EN) · `AFE_I2S_CON`(0x0018) ·
`AFE_CONN0-4`(0x0020-30: mixer/routing) · `AFE_DL1_BASE/CUR/END`(0x0040/44/48: buffer DMA) ·
`AFE_ADDA_TOP_CON0`(0x0120) · `AFE_IRQ_MCU_CON`(0x03A0) + `AFE_IRQ_CNT1`(0x03AC) ·
`AFE_MEMIF_PBUF_SIZE`(0x03D8).

## Qué hay en mainline
- `sound/soc/mediatek/mt2701-afe-pcm.c` = plantilla moderna (snd_soc_component) — patrón ~30%
  reutilizable; el DMA downstream es custom por IRQ de sample-count (NO snd_dmaengine).
- Codec MT6582/6323: **NO existe en mainline** → escribir `mt6582-codec.c` de cero
  (regmap sobre pwrap; DAPM widgets SPK/HP/MIC).

## Riesgos (por orden)
1. **Clocks**: sin CCF para mt6582 → depender de `clk_ignore_unused` + relojes que deja LK
   (mismo patrón que ya usamos en display/kpd). Verificar en HW que el AFE tiene reloj
   (devmem a 0x11220000 tras boot: si lee sin colgar y TOP_CON0 responde, hay reloj) ANTES de
   escribir el grueso — lección del WiFi/AUXADC.
2. **DMA/IRQ custom** (underruns) — portar el handler downstream tal cual, no dmaengine.
3. **Regmap-pwrap para el ANA** — o pwrap_read/write directo (como hace nuestro DTS/PMIC ya).
4. Timings de depop/ADDA (delays exactos del downstream).

## Fases (estimación honesta: 6-8 semanas total)
- **A** (10-12d): platform AFE mínimo — `sound/soc/mediatek/mt6582/`: mt6582-afe-pcm.c (probe,
  ioremap, component, PCM ops DL1, IRQ), mt6582-afe-regs.h, Kconfig/Makefile. ESQUELETO YA CREADO
  (compila; sin nodo DT → no probea → riesgo cero).
- **B** (8-10d): codec ANA (speaker path: power_init + SPK_CON + GPIO118 amp).
- **C** (7-10d): machine driver + DAPM + DT.
- **D** (6-8d): captura (VUL + ADDA UL + mic).
- Primer hito audible (fin C): `aplay test.wav` → altavoz.

## Validación temprana en HW (barata, ANTES de escribir el grueso — pedir hueco de móvil)
```sh
devmem 11220000            # AUDIOAFE_TOP_CON0: ¿lee sin colgar? → hay reloj/power
devmem 11220000 <val>      # ¿se puede poner AFE_EN? releer
# y el clock-gate de AUDIO en PERI/INFRA si existe (mapear bit en mt_clkmgr como hicimos con AUXADC)
```

*Sesión Mac, 2026-07-02. Mapa por subagente Explore (afc49e8) con spot-checks manuales de los datos
load-bearing (base AFE ✓, GPIO118 ✓, power_init ✓; IRQ pendiente). Siguiente: validar reloj AFE en
HW y completar Fase A.*

---

## ✅ VALIDACIÓN HW COMPLETADA (2026-07-05) — riesgo nº1 (clocks) DESPEJADO

Ejecutada en el móvil (kernel #236, uptime estable):
- **IRQ CONFIRMADO**: `AudDrv_Kernel.c:127 → HW_AFE_MCU_IRQ_LINE (104+32)` = **GIC_SPI 104** (el
  dato del agente era correcto).
- **Cadena de relojes mapeada**: gate `MT_CG_INFRA_AUDIO` = **bit 5 de INFRA_PDN_STA @0x10001048**
  (SET 0x1040 / CLR 0x1044); gates locales en `AUDIO_TOP_CON0 @0x11220000`: PDN_AFE=bit2,
  PDN_I2S=bit6 (headers del downstream: `AudDrv_Afe.h:594-596`, `mt_clkmgr.h:145`).
- **Medido en HW**: `INFRA_PDN_STA = 0x0` → **INFRA_AUDIO ya viene ENCENDIDO del LK** (mismo patrón
  que display/AUXADC; `clk_ignore_unused` lo preserva). `AUDIO_TOP_CON0 = 0x80394038` → **PDN_AFE=0
  y PDN_I2S=0: el AFE está alimentado y sin gatear de fábrica**. Lecturas Y escrituras al bloque
  responden sin colgar el bus (I2S_CON=0x0, IRQ_MCU_CON=0x0, MEMIF_PBUF=0xff0000ff — valores sanos).
- **Implicación**: la Fase A puede hacer probe+ioremap+register-bang sin bring-up de clocks propio
  (basta un `clocks`-menos DT node o fixed-clock dummy). El esqueleto
  `sound/soc/mediatek/mt6582/` del árbol de la Pi es el punto de partida.

**SIGUIENTE (Fase A.2)**: completar `mt6582-afe-pcm.c`: PCM DL1 (BASE/CUR/END @0x40/44/48),
IRQ MCU (CON @0x3A0, CNT1 @0x3AC, GIC_SPI 104), formato del buffer, `snd_pcm_hardware`, nodo DT
`audio@11220000`. Criterio: `/proc/asound/cards` muestra la tarjeta y `aplay` corre sin XRUN
(aún sin sonido audible hasta Fase B, el codec ANA).
