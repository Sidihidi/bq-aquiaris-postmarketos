#!/bin/sh -eu
# deploy-vbat-hwmon.sh — integra el driver hwmon mt6323-auxadc (VBAT por regmap
# del mt6397) en el arbol del kernel de la Pi y deja el config listo.
#
# Uso (en la Pi, con mt6323-auxadc.c al lado):
#   sh deploy-vbat-hwmon.sh [ruta-arbol]     # por defecto ~/mainline/linux-7.0.12
# Despues: ~/build-audio.sh  (config build-krillin, todo =y)  -> empaquetar/flashear
# como siempre (mtk_hdr + abootimg; wifi-iter-w.sh si el movil esta en userspace).
#
# Idempotente: se puede re-ejecutar sin duplicar lineas.

KDIR=${1:-$HOME/mainline/linux-7.0.12}
HERE=$(cd "$(dirname "$0")" && pwd)

[ -f "$HERE/mt6323-auxadc.c" ] || { echo "ERROR: falta $HERE/mt6323-auxadc.c"; exit 1; }
[ -d "$KDIR/drivers/hwmon" ] || { echo "ERROR: $KDIR no parece un arbol de kernel"; exit 1; }

# 1) fuente del driver
cp "$HERE/mt6323-auxadc.c" "$KDIR/drivers/hwmon/mt6323-auxadc.c"
echo "[ok] drivers/hwmon/mt6323-auxadc.c"

# 2) Makefile
if ! grep -q 'mt6323-auxadc' "$KDIR/drivers/hwmon/Makefile"; then
  printf 'obj-$(CONFIG_SENSORS_MT6323_AUXADC)\t+= mt6323-auxadc.o\n' >> "$KDIR/drivers/hwmon/Makefile"
  echo "[ok] Makefile: linea obj- anadida"
else
  echo "[ya] Makefile"
fi

# 3) Kconfig — insertar justo despues de 'if HWMON' (inequivoco; el final tiene
#    varios endif anidados y meterlo alli arriesga caer dentro de un 'if ACPI').
if ! grep -q 'SENSORS_MT6323_AUXADC' "$KDIR/drivers/hwmon/Kconfig"; then
  tmp=$(mktemp)
  awk '{print} /^if HWMON$/ && !done {
    print "";
    print "config SENSORS_MT6323_AUXADC";
    print "\ttristate \"MediaTek MT6323 PMIC AUXADC (VBAT)\"";
    print "\tdepends on MFD_MT6397";
    print "\thelp";
    print "\t  VBAT (BATSNS, canal 7 del AUXADC del PMIC MT6323) como hwmon,";
    print "\t  leido por el regmap del mt6397 (pwrap con lock). BQ Aquaris E4.5.";
    done=1
  }' "$KDIR/drivers/hwmon/Kconfig" > "$tmp" && cat "$tmp" > "$KDIR/drivers/hwmon/Kconfig" && rm -f "$tmp"
  grep -q 'SENSORS_MT6323_AUXADC' "$KDIR/drivers/hwmon/Kconfig" \
    || { echo "ERROR: no encontre 'if HWMON' en drivers/hwmon/Kconfig — anadir a mano"; exit 1; }
  echo "[ok] Kconfig"
else
  echo "[ya] Kconfig"
fi

# 4) DTS del krillin — anadir el nodo pmic-vbat al final (root reabierto, como
#    hacen los demas bloques del dts). Busca el dts vivo del arbol.
DTS=$(find "$KDIR/arch/arm/boot/dts" -name 'mt6582-bq-krillin.dts' | head -1)
[ -n "$DTS" ] || { echo "ERROR: mt6582-bq-krillin.dts no esta en el arbol"; exit 1; }
if ! grep -q 'mt6323-auxadc' "$DTS"; then
  cat >> "$DTS" <<'EOF'

/* ---- VBAT por AUXADC del MT6323 — driver hwmon mt6323-auxadc.c ----
 * Reemplaza el pwrap_poke periodico de /usr/local/bin/battery (MMIO al pwrap
 * SIN lock contra el driver del kernel; sospechoso del wedge del bloque de
 * INT del PMIC del 0708). Misma receta (CON11 bit4 VBUF, CON22 bit7 ch7,
 * ADC0 rdy/raw) pero via el regmap del mt6397 = serializado con keys/rtc/
 * regulator/codec. Regmap por phandle, patron identico al nodo de audio. */
/ {
	pmic_vbat: pmic-vbat {
		compatible = "mediatek,mt6323-auxadc";
		mediatek,pmic = <&pmic>;
	};
};
EOF
  echo "[ok] DTS: nodo pmic-vbat anadido a $DTS"
else
  echo "[ya] DTS"
fi

# 5) config: activar los simbolos alli donde viva el .config del build
#    (build-audio.sh / config build-krillin). scripts/config es del propio arbol.
enable_in() {
  cfg=$1
  [ -f "$cfg" ] || return 1
  "$KDIR/scripts/config" --file "$cfg" -e HWMON -e SENSORS_MT6323_AUXADC
  echo "[ok] config: HWMON=y SENSORS_MT6323_AUXADC=y en $cfg"
}
found=0
for c in "$KDIR/build-krillin/.config" "$KDIR/.config" "$KDIR"/build*/.config; do
  enable_in "$c" && found=1 || true
done
if [ $found -eq 0 ]; then
  echo "[!!] no encontre .config — activa a mano en el config de build-audio.sh:"
  echo "     CONFIG_HWMON=y / CONFIG_SENSORS_MT6323_AUXADC=y (y 'make olddefconfig')"
fi

echo
echo "Siguiente: ~/build-audio.sh  ->  empaquetar+flashear  ->  deploy del script"
echo "'battery' al movil (ver VBAT-HWMON-0708.md, transferir por base64)."
