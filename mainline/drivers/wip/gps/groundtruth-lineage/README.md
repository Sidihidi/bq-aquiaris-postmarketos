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
