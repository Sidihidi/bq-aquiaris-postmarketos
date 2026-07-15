# RESPUESTA al handoff `CONTINUAR-GPS-RF-0715.md` — la init RF del GPS que falta

> Pregunta de casa: el DSP busca pero 0 satélites (`$GPGSV,1,1,0`) — falta la **secuencia de encendido
> RF del GPS del stock**, que "no teníamos (downstream solo firmware, sin source)".
> **Ahora SÍ la tenemos**: cloné la fuente GPL de BQ (`github.com/bq/aquaris-E4.5`, rama `aquaris-E4.5`) y
> encontré exactamente qué hace el stock con el GPS **después** de `func_on(GPS)`. Fuente guardada en
> `stock-ref/` (`wmt_func.c`, `wmt_ctrl.c`, `wmt_plat_alps.c`, `WMT_SOC.cfg-krillin`).

## Lo que hace el stock al encender el GPS (y pmOS NO)
Cadena: `wmt_func_gps_on()` → `wmt_func_gps_pre_on()` → **`wmt_func_gps_pre_ctrl(FUNC_ON)`** →
**LUEGO** `wmt_func_gps_ctrl(FUNC_ON)` (= la WMT `func_ctrl_cmd(GPS,ON)` que pmOS ya manda).
El paso que FALTA en pmOS es todo lo de `wmt_func_gps_pre_ctrl`, que ocurre **ANTES** del func_on:

Config del krillin (`stock-ref/WMT_SOC.cfg-krillin`): `wmt_gps_lna_enable=0`, `wmt_gps_lna_pin=0`,
`co_clock_flag=0`, `coex_wmt_ant_mode=1`.

1. **GPS_SYNC** (`WMT_CTRL_GPS_SYNC_SET`, 1): mux del pin GPS_SYNC (1PPS/co-clock). `co_clock_flag=0` →
   probablemente NO crítico para adquisición, pero lo hace igual.
2. **GPS_LNA** ← **EL CANDIDATO Nº1**. Como `wmt_gps_lna_enable=0` = "host pin used for gps lna":
   ```
   wmt_ctrl(WMT_CTRL_GPS_LNA_SET, 1)
     -> wmt_ctrl_gps_lna_set()  (wmt_ctrl.c:1020)
       -> wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_OUT_H)
         -> wmt_plat_gps_lna_ctrl(PIN_STA_OUT_H)  (wmt_plat_alps.c:653)
           -> mt_set_gpio_out(GPIO_GPS_LNA_PIN, GPIO_OUT_ONE)   // ← LNA de la antena a HIGH
   ```
   **pmOS nunca toca este GPIO** → el LNA de la antena GPS no se alimenta → 0 señal al correlador → 0 sat.
   (En init/deinit el stock lo pone como GPIO output y a LOW; en GPS-ON lo pone HIGH.)

**VCN28 NO es el hueco**: el bloque `GPS_PALDO`/`PALDO_ON` de `wmt_func_gps_on` se SALTA en krillin
(requiere `co_clock_type != 0`), y de todas formas nuestro `mt6582-consys.c` ya hace `regulator_enable(vcn28)`
en el bring-up. Así que la alimentación analógica del LNA está. **Lo que falta es el GPIO enable del LNA.**

## Cómo sacar el NÚMERO de GPIO del GPS_LNA (2 vías)
El número está en el DCT binario del krillin (`codegen.dws`, tiene la entrada `GPIO_GPS_LNA_PIN`) pero no en
texto. La vía fiable (ground-truth del proyecto):
- **Arrancar LineageOS** y `dmesg | grep -iE "GPS_LNA|GPS_SYNC"` →
  `wmt_plat_alps.c` imprime **`GPS_LNA(GPIO<N>)`** (o `GPS_LNA(not defined)` si fuera LDO-only).
  Eso da el número exacto Y confirma que es GPIO-controlado.
- (alt) descomprimir el `codegen.dws` con la herramienta DCT de MTK.

## Fix (rápido de probar)
### A) Test SIN recompilar (userspace, lo más rápido)
Con mnld corriendo (0 sat), exportar el GPIO del LNA y ponerlo a 1, y ver si aparecen satélites:
```sh
# N = el número de dmesg de LineageOS
echo N > /sys/class/gpio/export 2>/dev/null
echo out > /sys/class/gpio/gpioN/direction
echo 1   > /sys/class/gpio/gpioN/value
# esperar 30-60s con cielo, y:  gpspipe -w | grep -E 'GPGSV|nSat'
```
(si no hay /sys/class/gpio para ese pin, usar el mt_gpio debug del kernel de pmOS.)

### B) Fix real en el driver `drivers/soc/mediatek/mt6582-btif.c`
En `__bringup`, **tras** `func_on(b, STP_TYPE_GPS, "GPS")` (~línea 387), añadir el enable del LNA:
```c
/* Stock (wmt_func_gps_pre_ctrl): host GPS_LNA GPIO a HIGH al encender el GPS.
 * Sin esto el LNA de la antena no recibe -> 0 satelites. GPIO_GPS_LNA de LineageOS dmesg. */
gpio_request(GPS_LNA_GPIO, "gps-lna");
gpio_direction_output(GPS_LNA_GPIO, 1);   /* PIN_STA_OUT_H */
```
Mejor: declararlo en el DTS (`gps-lna-gpios = <&eint N GPIO_ACTIVE_HIGH>`) y cogerlo con `devm_gpiod_get`,
subir en func_on(GPS) y bajar en func_off. (Y opcional: mux del GPS_SYNC — ver `wmt_plat_gps_sync_ctrl`.)

## Confianza / siguiente paso
- **Alta** en que el GPS_LNA GPIO es la init que falta (encaja: DSP vivo + busca + 0 sat = frontend RF sin
  alimentar; el stock lo hace, pmOS no; VCN28 ya está). Pero **confirmar el nº de GPIO con LineageOS dmesg**
  antes de cablearlo (podría ser "not defined" → entonces revisar GPS_SYNC / un cal RF, ver abajo).
- Si el LNA no lo arregla: el `RF-CAL` genérico que ya se manda (`stp_send WMT {01,14,01,00,01}`) podría
  necesitar parámetros específicos de GPS — pero probar el LNA primero.

*Mac (Fable 5), 2026-07-15. Fuente GPL de BQ clonada; init RF del GPS identificada = GPS_LNA GPIO high.
Ref en `stock-ref/`. Falta el nº de GPIO (LineageOS dmesg) para cablearlo.*

---
## ✅ CONFIRMADO 0715 (Mac): GPS_LNA = **GPIO 47** + fix probado (drives high)
Número sacado del log de arranque de **LineageOS** guardado (`~/android-cap/lineage/kmsg-boot-clean.log`):
```
wmt_plat_dump_pin_conf: GPS_LNA(GPIO-2147483601)   -> 0x8000002F = 0x80000000|47 -> GPIO 47
                        GPS_SYNC(not defined)      -> el krillin NO usa GPS_SYNC
```
→ **la ÚNICA init RF que falta = conducir el GPIO 47 a HIGH.** (`downstream` recuperado en
`~/mainline/downstream/bq-src/`; DrvGen x86 en `mediatek/dct/` + `qemu-i386` en la Pi por si hiciera falta.)

### Probado en HW (userspace, sin recompilar)
- Herramienta `gpio-hold.c` (cross-compilada armhf → `/data/gpio-hold` del móvil): usa `/dev/gpiochip0`
  (el pinctrl mainline gestiona modo/dir) y MANTIENE la línea. `gpio-hold 47 1` →
  **`gpio-47 (|gps-lna) out hi` ✓** (el pinctrl lo aceptó, sin conflicto).
- Con LNA ON + cadena GPS corriendo. **En INTERIOR sigue 0 sat / 1 payload** (lógico: sin cielo no hay señal
  que amplificar). **Validar = móvil al cielo** con GPIO 47 high → `gpspipe -w | grep -E 'GPGSV|nSat'`.

### Test rápido (móvil, con cielo)
```sh
setsid /data/gpio-hold 47 1 &            # enciende el LNA (mantener corriendo)
/etc/local.d/zzz-gps.start               # arranca la cadena
gpspipe -w | grep -E 'GPGSV|nSat|mode'   # con cielo: satelites > 0 si el LNA lo arregla
```

### Fix PERMANENTE en el driver (`drivers/soc/mediatek/mt6582-btif.c`)
DTS (nodo del consys/btif): `gps-lna-gpios = <&eint 47 GPIO_ACTIVE_HIGH>;`
Driver: en probe `b->gps_lna = devm_gpiod_get_optional(b->dev,"gps-lna",GPIOD_OUT_LOW);`
y **tras** `func_on(b, STP_TYPE_GPS, "GPS")` (~L388): `gpiod_set_value_cansleep(b->gps_lna, 1);`
(alt legacy, gpiochip0 base=0: `gpio_request(47,"gps-lna"); gpio_direction_output(47,1);`)
En `func_off(GPS)` bajarlo. El stock también hace `PIN_STA_INIT` del LNA en el power-on del CONSYS
(`mtk_wcn_consys_hw_gpio_ctrl`), pero drive-high en GPS-on es lo que enciende el amplificador.
