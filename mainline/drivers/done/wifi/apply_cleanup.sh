#!/bin/bash
# Aplica SOLO la limpieza segura en el arbol del kernel de la Pi:
#  - 04 btif .remove()  (parche completo, inocuo en built-in)
#  - 05 consys .remove() (parche completo, inocuo en built-in)
#  - 06 musb: SOLO dev_info->dev_dbg + ioremap->devm_ioremap  (SIN multipoint=false)
# (el 07 wifi .remove va en el driver local, se sube en el build)
T=~/mainline/linux-7.0.12
cd "$T" || { echo "no tree"; exit 1; }

echo "=== backups .bak-cleanup (no sobrescribe si ya existe) ==="
cp -n drivers/soc/mediatek/mt6582-btif.c   drivers/soc/mediatek/mt6582-btif.c.bak-cleanup
cp -n drivers/soc/mediatek/mt6582-consys.c drivers/soc/mediatek/mt6582-consys.c.bak-cleanup
cp -n drivers/usb/musb/mt6582-musb.c       drivers/usb/musb/mt6582-musb.c.bak-cleanup

tr -d '\r' < /tmp/04-btif-add-remove.patch   > /tmp/04.lf.patch
tr -d '\r' < /tmp/05-consys-add-remove.patch > /tmp/05.lf.patch

echo "=== 04 btif .remove ==="
if patch -p1 --dry-run < /tmp/04.lf.patch >/dev/null 2>&1; then
    patch -p1 < /tmp/04.lf.patch && echo "04 OK"
else
    echo "!!! 04 NO aplica limpio (¿ya aplicado o contexto cambiado?)"; patch -p1 --dry-run < /tmp/04.lf.patch 2>&1 | tail -4
fi

echo "=== 05 consys .remove ==="
if patch -p1 --dry-run < /tmp/05.lf.patch >/dev/null 2>&1; then
    patch -p1 < /tmp/05.lf.patch && echo "05 OK"
else
    echo "!!! 05 NO aplica limpio"; patch -p1 --dry-run < /tmp/05.lf.patch 2>&1 | tail -4
fi

echo "=== 06 musb (SOLO dev_dbg + devm_ioremap, SIN multipoint) ==="
M=drivers/usb/musb/mt6582-musb.c
sed -i 's|dev_info(musb->controller, "mt6582-musb: enable|dev_dbg(musb->controller, "mt6582-musb: enable|' "$M"
sed -i 's|= ioremap(0x10209000, 0x1000)|= devm_ioremap(dev, 0x10209000, 0x1000)|' "$M"
sed -i 's|= ioremap(0x10003000, 0x1000)|= devm_ioremap(dev, 0x10003000, 0x1000)|' "$M"

echo "=== VERIFICACION ==="
echo -n "btif .remove (>=1): ";   grep -c 'mt6582_btif_remove'   drivers/soc/mediatek/mt6582-btif.c
echo -n "consys .remove (>=1): "; grep -c 'mt6582_consys_remove' drivers/soc/mediatek/mt6582-consys.c
echo -n "musb dev_dbg (=1): ";    grep -c 'dev_dbg(musb->controller, "mt6582-musb: enable' "$M"
echo -n "musb devm_ioremap (=2): "; grep -c 'devm_ioremap(dev, 0x10' "$M"
echo -n "musb multipoint (DEBE ser true): "; grep -o 'multipoint[[:space:]]*=[^,;]*' "$M"
