#!/bin/sh
# Secuencia EXACTA de encendido del codec (RE del HAL audio.primary.mt6582.so).
# Clave vs poke plano: el ORDEN + usleep(10ms) de asentamiento del bias entre CON4 y CON6.
PP=/usr/local/bin/pwrap_poke
DM=/usr/local/bin/devmem
pw() { $PP w "$1" "$2" >/dev/null 2>&1; }
pr() { $PP r "$1" 2>/dev/null | sed 's/.*= //'; }
rmw() { c=$(pr "$1"); pw "$1" $(printf '0x%04x' $(( ( (0x${c#0x} & (0xffff ^ $2)) | ($3 & $2) ) & 0xffff )) ); }  # reg mask val
dm() { $DM "$1" "$2" >/dev/null 2>&1; }

# --- SoC AFE (devmem): deltas que nuestro driver no pone ---
dm 0x11220018 0x8000000d   # AFE_I2S_CON
dm 0x11220010 0x00013043   # AFE_DAC_CON0 (completo)
dm 0x11220014 0x00009909   # AFE_DAC_CON1
dm 0x11220028 0x00410040   # AFE_CONN2
dm 0x11220138 0x03f87200   # ADDA_NEWIF_CFG0
dm 0x1122013C 0x03117180   # ADDA_NEWIF_CFG1

# --- PMIC §2: AFE/downlink digital (orden del HAL) ---
rmw 0x010c 0x0100 0x0100   # reloj del bloque AFE del PMIC   <-- NUEVO (nunca lo ponia)
pw  0x4024 0x7330          # ABB_AFE_PMIC_NEWIF_CFG0 (freq ADDA=7)
rmw 0x4002 0x000f 0x0009   # ABB_AFE_CON1 (sample-rate)
pw  0x4000 0x0001          # ABB_AFE_CON0 = AFE_ON
pw  0x4006 0x0253          # CON3 (del lado digital, del ground-truth)
pw  0x4008 0x0274          # CON4
pw  0x4014 0x0001          # CON10
pw  0x4016 0x0303          # CON11

# --- PMIC §3: secuencia ANALOGICA de auriculares (=altavoz+GPIO118), ORDEN LITERAL ---
pw  0x070c 0xf7f2          # AUDTOP_CON6: bias/LDO/refgen del HP (pre-depop)
rmw 0x0700 0xf000 0x7000   # AUDTOP_CON0 bits[15:12]
pw  0x070a 0x0014          # AUDTOP_CON5
pw  0x0708 0x007c          # AUDTOP_CON4 = 0x7c
sleep 0.02                 # >= 10ms ASENTAMIENTO DEL BIAS (LO QUE FALTABA)
pw  0x070c 0xf5ba          # AUDTOP_CON6 = 0xf5ba: enciende drivers HP L/R
pw  0x070a 0x2214          # AUDTOP_CON5 = 0x2214
# ganancia HP (subir volumen)
rmw 0x070a 0x7000 0x4000   # HP-L gain
rmw 0x070a 0x0700 0x0400   # HP-R gain
echo "aplicado. AUDTOP_CON4=$(pr 0x0708) CON6=$(pr 0x070c) ABB_CON0=$(pr 0x4000) AFEclk(0x10c)=$(pr 0x010c)"
