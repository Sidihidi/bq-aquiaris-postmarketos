#!/usr/bin/env bash
# Capa 0 — backup GOLDEN del krillin vía mtkclient/BROM. SOLO LECTURA: no escribe
# nada en el teléfono. Diagnóstico + volcado de particiones críticas.
#
# USO (en la Pi): lanzar ESTE script y DESPUÉS conectar el teléfono en modo BROM
#   (apagado o batería fuera → enchufar USB manteniendo Vol- ; probar Vol+ si no).
# Cada comando mtkclient espera al device ("Waiting for device...").
set -u
MTK="$HOME/mtkclient/venv/bin/python $HOME/mtkclient/mtk.py"
OUT="$HOME/golden"; mkdir -p "$OUT"; cd "$OUT" || exit 1

echo "=== 1) Secure boot (sbc/daa) — debe salir DISABLED en krillin ==="
$MTK gettargetconfig 2>&1 | tee gettargetconfig.txt | grep -iE "sbc|daa|secure|config|disabled|enabled" || true

echo "=== 2) Tabla de particiones real (nombres/offsets que ve mtkclient) ==="
$MTK printgpt 2>&1 | tee printgpt.txt | tail -45

echo "=== 3) Backup de las críticas (por nombre; los fallos se ignoran y se siguen) ==="
for p in preloader seccfg uboot lk nvram proinfo protect1 protect2 protect_f protect_s boot recovery; do
  echo "--- $p ---"
  $MTK r "$p" "$p.bin" 2>&1 | tail -2
done

echo "=== 4) Resumen del golden ==="
ls -la "$OUT"/*.bin 2>/dev/null || echo "(no se volcó ninguna .bin — revisar conexión BROM)"
echo
echo ">>> Si faltan críticas (preloader/seccfg/lk) por el bug del MBR del MT6582,"
echo ">>> usar el fallback robusto (una sola conexión, sin parsear particiones):"
echo ">>>   revisar args con:  \$MTK ro --help"
echo ">>>   leer el frente del eMMC (preloader..recovery, ~89 MB) a un solo fichero."
echo
echo "NOTA: este script NO escribe nada. El unlock (da seccfg unlock) y el flash"
echo "      (w boot ...) son pasos MANUALES posteriores — ver GUIA-MTKCLIENT.md §5/§6."
