# Ground-truth de LineageOS con GPS ACTIVO (13 sats en GPSTest) — 0715

Capturado por adb (Pi) con LineageOS corriendo y GPSTest viendo **13 satélites** (la RF FUNCIONA
en este hardware con el kernel stock 3.10).

- `gpio-stock-gpson.txt` — dump de `/sys/devices/virtual/misc/mtgpio/pin` (169 pines).
  Formato: `PIN: [MODE][PULL_SEL][DIN][DOUT][PULLEN][DIR][IES]`.
  **Pin 47 = `0011011`** → MODE=0(GPIO) DIN=1 DOUT=1 DIR=out = **LNA HIGH, IGUAL que pmOS** ✓
  → el LNA NO es el delta. El delta está en OTRO pin (¿switch/frontend de antena?).
- `dmesg-stock.txt` — dmesg del stock (la parte del boot ya rotada; sin líneas wmt-func).
- PMIC LDOs stock con GPS on: VCN28=1, VCN_1V8=1, VTCXO=1, VA=1 → **idéntico a pmOS** ✓
  → la alimentación NO es el delta.

## Siguiente paso (pmOS #297 re-flasheado)
Dump del pinmux mainline (`/sys/kernel/debug/pinctrl/*/pinmux-pins` + pinconf) y **diff pin a pin**
contra `gpio-stock-gpson.txt`. Los pines donde stock≠pmOS (especialmente GPIO-out o modos especiales)
= candidatos al control del frontend RF del GPS. Cross-check de nombres: el DCT del krillin
(`codegen.dws` en la fuente GPL, records secuenciales por pin con varname con prefijo de longitud).

## RESULTADO del diff (0715 tarde): GPIOs IDÉNTICOS → el delta NO es hardware
Dump de pmOS con GPS on vía módulo `ioremap(0x10005000)` (la base GPIO real; `0x1000b000` = EINT, NO GPIO
— error inicial). `gpio-pmos-gpson.txt`. Diff MODE/DIR/DOUT de los 169 pines stock vs pmOS:
- **LNA (47), enables out-high (74,81,82,83,93,114,115), out-low (118,120,121): TODOS coinciden.**
- Único delta limpio: **pin 122** (stock MODE=GPIO, pmOS MODE=alt1) pero **out-LOW en ambos** → no controla RF.
- 141-166 = bus MSDC (formato con drive-strength, artefacto). DCT del krillin: NO hay GPIO "ANT_SW/RF" — el
  único control RF del GPS es el LNA (47), que está OK.
- **PMIC LDOs (VCN28/VCN_1V8/VTCXO/VA): idénticos.**

### Conclusión: config de HW (pines+rails) = idéntica al stock, pero GPS en ruido 0xCA.
→ El delta es **DINÁMICO / firmware / calibración**, NO configuración de pines. Candidatos que quedan:
1. **Firmware/patch del CONSYS**: verificar que `mt6572_82_patch_e1_{0,1}_hdr.bin` (los que descarga el btif)
   son idénticos a los del stock (`/system/etc/firmware/` o `/vendor/firmware/` de LineageOS). Si difieren,
   el RF se comporta distinto.
2. **NVRAM de calibración RF**: el WIFI NVRAM (crystal trim byte 0x6D) está VACÍO en pmOS; el stock lo tiene
   de fábrica (en protect_f/s, cifrado). Extraer de LineageOS (`/data/nvram/APCFG/APRDEB/WIFI`) y aplicar el
   `crystal_triming_set`. (Aunque el Doppler search deberia cubrir el offset... verificar.)
3. **Comandos que el HAL/mnld del stock manda al DSP** (`/dev/stpgps` write burst): snoopear en LineageOS qué
   escribe mnld al arrancar la sesión vs pmOS. Usamos el MISMO libmnlp, pero el HAL (`gps.default.so`,
   ausente en pmOS) puede mandar comandos de config RF/referencia que activan la adquisición.
