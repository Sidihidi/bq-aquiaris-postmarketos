# PLAN MAESTRO DE PORTS — BQ Aquaris E4.5 (krillin / MT6582) mainline 7.0.12

> Auditoría y plan de ports para todos los drivers que necesitan trabajo.
> Basado en análisis del downstream MTK 3.10 + estado actual del proyecto + 8 hipótesis refutadas.
> Pi de build: `cpcd@192.168.0.38`. Móvil: `root@172.16.42.1`. Kernel: `~/mainline/linux-7.0.12`.
> Downstream: `~/mainline/downstream/drivers/misc/mediatek/`.

---

## RESUMEN EJECUTIVO

| # | Driver | Esfuerzo | Prioridad | Estado actual | Próximo paso |
|---|---|---|---|---|---|
| 1 | **WiFi (mt_wifi stock)** | 6-9 sem | 🔴 CRÍTICO | Scaffold 2.6K LOC, WPA2 roto | Port (plan en HANDOFF-MTWIFI-PORT-PLAN-0702.md) |
| 2 | **Audio codec MT6582** | 2-3 sem | 🟡 ALTO | Sin implementar | Port ALSA del downstream |
| 3 | **GPS (gpsdrv init)** | 1-2 días | 🟡 ALTO | Bridge userspace, chip no streamea | Capturar START_SEQ de mnld en LineageOS |
| 4 | **Touch kernel (edt-ft5x06)** | 2-3 días | 🟠 MEDIO | Userspace polling, funciona | Migrar a driver kernel con EINT117 |
| 5 | **STP core (resync/CRC)** | 1 sem | 🟠 MEDIO | BTIF sin CRC, BT funciona | Portear stp_core.c del downstream |
| 6 | **BTCVSD (audio BT)** | 1-2 sem | 🟠 MEDIO | Sin SCO | Portear btcvsd del downstream |
| 7 | **Vibrador + LEDs** | 2-3 días | 🟢 BAJO | Sin implementar | Driver trivial PWM/GPIO/MT6323 |
| 8 | **Accdet (jack detect)** | 1 sem | 🟢 BAJO | Sin implementar | Portear accdet del downstream |
| 9 | **Thermal** | 1 sem | 🟢 BAJO | Sin implementar | Portear thermal del downstream |
| 10 | **FM Radio** | 4-6 sem | 🔴 BAJO* | Sin implementar | Port V4L2 masivo, baja prioridad |

(*Baja prioridad = mucho trabajo para poca utilidad)

---

## QUICK-WINS MAINLINE (0702, sesión Mac) — drivers ya en mainline, solo DT+config

| Subsistema | Driver mainline | Estado | Test HW |
|---|---|---|---|
| **Botón de encendido** | `mtk-pmic-keys` ("mediatek,mt6323-keys") | ✅ CODE-COMPLETE | `evtest` → KEY_POWER (116) |
| **RTC** | `rtc-mt6397` ("mediatek,mt6323-rtc") | ✅ CODE-COMPLETE | `hwclock -r`, `/dev/rtc0` |
| **Vibrador** | `regulator-haptic` (ldo_vibr 2.8V) | ✅ CODE-COMPLETE (driver 7) | `fftest`, feedbackd |
| **LED RGB + botones** | `leds-mt6323` (ISINK0-3) | ✅ CODE-COMPLETE (driver 7) | `echo 255 > /sys/class/leds/*/brightness` |
| **AUXADC (SoC)** | `mt6577_auxadc` ("mediatek,mt6582-auxadc") | ✅ validado por devmem (driver 9) | `iio_info` |

El MFD `mt6397-core` ya registra los hijos mt6323 (rtc/keys/led/regulator/pwrc); solo faltaban el
nodo DT y el símbolo Kconfig de cada uno. Config: RTC_DRV_MT6397, KEYBOARD_MTK_PMIC, LEDS_MT6323,
INPUT_REGULATOR_HAPTIC, MEDIATEK_MT6577_AUXADC, GENERIC_ADC_THERMAL. DTS canónico: `mainline/dts/`.
**Todos compilan (zImage+DTB). Falta 1 flash coordinado con la sesión WiFi para probarlos juntos.**

---

## DRIVER 1: WiFi (mt_wifi stock port) — 🔴 CRÍTICO

**Plan completo:** `HANDOFF-MTWIFI-PORT-PLAN-0702.md` (189 líneas, 6 fases, 2 críticos adversariales).

### Resumen
- **Stock**: 63 .c + 87 .h, 110K LOC (combo/drv_wlan/mt6628/)
- **Nuestro**: mt6582-wifi.c (2189 lín) — scaffold, WiFi abierto funciona, WPA2 DHCP roto
- **Causa raíz confirmada**: 8 hipótesis refutadas, diferencia 100% dinámica con LineageOS
- **Estrategia**: híbrido estricto (3 capas): nuestro HIF + KAL shims nuevos + core stock intacto

### Las 10 diferencias fatales que requiere el port
1. **Frames 802.11 a mano** vs SAA FSM del stock (1953 LOC)
2. **EAPOL por data-path** vs cmd-queue como SECURITY_FRAME
3. **Sin sec_fsm** (1187 LOC) — deferred PTK hasta TX-DONE del 4/4
4. **Sin wlanLoadManufactureData** — init-blob NVRAM/EFUSE nunca enviado
5. **Sin qmActivateStaRec** — queue manager (4756 LOC)
6. **Sin cnm_timer** (493 LOC) — timers del core
7. **Sin enhance-block read-clear** correcto (88B struct)
8. **SET_BSS_INFO directo** sin OID wrapper (sin serialización)
9. **Sin FSM AIS** (4439 LOC) — coordinación de estados
10. **Sin privacy.c + rsn.c** (4584 LOC) — parser RSN + activación de cifrado

### Fases del port
```
Fase 0: árbol + Kconfig + dieta core (2-3 días)
Fase 1: contrato KAL + shims que compilan (1-1.5 sem)
Fase 2: backend HIF = nuestro driver (1 sem)
Fase 3: probe + boot del FW por path STOCK (1-1.5 sem)
Fase 4: cfg80211 scan = HITO M1 (1 sem)
Fase 5: connect + 4-way + claves = DHCP (1.5-2 sem)
Fase 6: teardown, estabilidad, limpieza (1-2 sem)
```

### M1 (primer hito)
`insmod → wlanAdapterStart completa → iw dev wlan0 scan lista APs`

### Criterio de éxito final
`nmcli con up "hola" → DHCP → IP → ping 8.8.8.8 → WPA2 navega`

### Riesgos principales
1. **Teardown landmine**: `SET_BSS_INFO(DISCONNECTED)` cuelga este FW (fix #197: teardown ligero)
2. **kalIoctl sin timeout + wiphy_lock**: deadlock si FW cuelga en un OID
3. **Enhance-block semantics**: debe ser bit-identico al que el stock espera
4. **RX aggregation**: burst-reads multi-paquete que nuestro driver nunca ejercitó
5. **CFG_PRIVACY_MIGRATION**: dos paths de privacy/keys, decidir cual emite

### Entorno
- Stock driver: `~/mainline/downstream/drivers/misc/mediatek/combo/drv_wlan/mt6628/wlan/`
- Nuestro driver: `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-wifi.c`
- FW descompilado: `fw-analysis/` en el repo (633 funciones NDS32 mapeadas)
- Test WPA2: `wpa_supplicant -i wlan0 -c /tmp/wpa.conf -D nl80211 -B; udhcpc -i wlan0`
- AP test: SSID "hola", WPA2-PSK, pass "kakatua1", BSSID e6:92:82:f4:ce:44, ch=10 (2457)

---

## DRIVER 2: Audio codec MT6582 — 🟡 ALTO — 📋 MAPEADO + Fase A esqueleto (0702, sesión Mac)

**Mapa completo verificado en `mainline/audio/HANDOFF-AUDIO-PORT-0702.md`**: AFE @0x11220000 (SoC) +
codec ANA en el PMIC vía pwrap + amp externo de altavoz por **GPIO118** (secuencia 3× toggle + 40ms).
Sin codec mainline → escribir de cero con patrón mt2701-afe-pcm. **Estimación honesta: 6-8 semanas.**
Fase A empezada: `sound/soc/mediatek/mt6582/` (esqueleto compilable, `CONFIG_SND_SOC_MT6582=m`,
sin nodo DT → no probea → riesgo cero). Fuentes canónicas en `mainline/audio/`.
SIGUIENTE: validar reloj del AFE en HW (devmem 0x11220000) → PCM DL1 + IRQ (A.2).

## DRIVER 2 (plan original): Audio codec MT6582 — 🟡 ALTO

### Estado actual
Sin implementar. El krillin tiene un codec de audio integrado en el MT6582 SoC
(accsesible por I2C + ALSA). No hay sonido (altavoz, auricular, micrófono).

### Stock downstream
```
sound/ — 4 ficheros .c
drivers/misc/mediatek/sound/ — codec MT6582
```

### Arquitectura del audio en el MT6582
- **Codec analógico**: MT6582 tiene un codec I2S integrado
- **Digital audio interface**: I2S/PCM hacia el codec
- **Control de volumen/ganancia**: registros MMIO del SoC
- **ALSA SoC machine driver**: conecta el codec con el DAI

### Plan de porteo

#### Fase A: Identificar el codec (1 día)
```sh
# buscar en el downstream qué codec usa el krillin
grep -rn "krillin\|mt6582.*codec\|mt6582.*audio\|mtk_codec" ~/mainline/downstream/sound/
# buscar el machine driver
find ~/mainline/downstream -name "*.c" | xargs grep -l "snd_soc" | grep mt6582
# buscar el DAI link
grep -rn "dai_link\|snd_soc_dai" ~/mainline/downstream/sound/ | grep 6582
```

#### Fase B: Portear el codec driver (1 sem)
- Copiar el codec driver del downstream (`sound/soc/codecs/mtk*.c`)
- Adaptar a ALSA SoC moderno (7.0.12): `snd_soc_component` en vez de `snd_soc_codec`
- Mapear registros MMIO del codec (probablemente en el rango `0x1121xxxx`)

#### Fase C: Portear el machine driver (1 sem)
- Crear el DAI link: CPU DAI (I2S del MT6582) ↔ codec DAI
- Configurar el routing de audio: `dapm_widgets` + `dapm_routes`
- Habilitar el clock de audio (probablemente un fixed-clock del DT)

#### Fase D: DT bindings (2-3 días)
```dts
&i2s {
    codec {
        compatible = "mediatek,mt6582-codec";
        /* registros del codec */
    };
};
```

#### Fase E: Driver de plataforma (3-5 días)
- DMA de audio: el MT6582 usa un DMA de audio interno
- `snd_pcm_hardware` con period/buffer sizes
- `snd_dmaengine_pcm_open` o DMA custom

### Riesgos
- El codec puede no ser un chip separado sino interno al SoC → registros MMIO compartidos
- El clock de audio puede requerir el CCF (Common Clock Framework) del MT6582 que no tenemos
- ALSA SoC cambió mucho entre 3.10 y 7.0.12 (snd_soc_codec → snd_soc_component)

### Criterio de éxito
`aplay /usr/share/sounds/test.wav` → suena por el altavoz

---

## DRIVER 3: GPS (gpsdrv init) — 🟡 ALTO

### Estado actual
- `/dev/stpgps` funciona (btif expone el canal STP 2)
- `mtkgps_aaf0.c` (293 lín userspace) decodifica el protocolo 0xAAF0
- **El chip NO streamea datos GPS** pese al START_BURST

### Causa probable
LineageOS arranca el GPS vía `/sys/class/gpsdrv/gps/` (driver `gps.c` del stock,
1107 LOC). Nuestro driver no tiene ese chardev sysfs. Falta replicar la secuencia
de init que mnld envía por ahí.

### Plan de porteo

#### Paso 1: Capturar el START_SEQ de mnld en LineageOS (1 día)
```sh
# arrancar LineageOS
adb root; adb shell setenforce 0
# strace mnld mientras se inicia el GPS
adb shell "strace -p $(pidof mnld) -s 1024 -e write -f" &
adb shell "am broadcast -a android.location.GPS_ENABLED_CHANGE"
# el write() a /dev/stpgps o /sys/class/gpsdrv/gps/ es el START_SEQ
```

#### Paso 2: Replicar en el bridge (1 día)
- Pegar el START_SEQ capturado en `mtkgps_aaf0.c` (array `START_BURST`)
- O si va por `/sys/class/gpsdrv/gps/`: portear `gps.c` (1.1K LOC) del downstream

#### Paso 3: Test con cielo despejado (1 día)
```sh
mtkgps_aaf0 /dev/stpgps /dev/gps0 &
gpsd -N -n /dev/gps0
cgps  # debe mostrar lat/lon
```

### Stock driver
```c
// ~/mainline/downstream/drivers/misc/mediatek/gps/gps.c (1107 LOC)
// chardev /dev/gpsdrv con sysfs /sys/class/gpsdrv/gps/
// controla el power on/off del GPS y el reset
```

### Riesgos
- El START_SEQ puede variar entre boots (timestamps/efemérides)
- Puede necesitar primer fix en frío (>30s con cielo despejado)
- La antena GPS del krillin puede ser débil

### Criterio de éxito
`cgps` muestra lat/lon reales (37.xxxx N, -0.xxxx W)

---

## DRIVER 4: Touch kernel (edt-ft5x06) — 🟠 MEDIO

### Estado actual
- `ft5336_touch.c` (173 lín) — daemon userspace, polling I2C 60Hz, single-touch
- EINT117 disponible pero sin usar
- `edt-ft5x06` ya compilado en el kernel (`CONFIG_TOUCHSCREEN_EDT_FT5X06=y`)
- Falta el nodo DT con `reset-gpios` + `interrupts` + `vin-supply`

### Plan de porteo

#### Paso 1: Crear el nodo DT (1 día)
```dts
&i2c0 {
    touchscreen@38 {
        compatible = "edt,edt-ft5x06";
        reg = <0x38>;
        interrupt-parent = <&eint>;
        interrupts = <117 IRQ_TYPE_EDGE_FALLING>;
        reset-gpios = <&gpio 115 GPIO_ACTIVE_LOW>;
        vin-supply = <&mt6323_vgp1_reg>;  /* VGP1 ya en DT */
        touchscreen-size-x = <540>;
        touchscreen-size-y = <960>;
    };
};
```

#### Paso 2: Retirar el daemon userspace (1 hora)
```sh
mv /etc/local.d/touch-power.start /etc/local.d/touch-power.start.off
pkill ft5336_touch
# el nodo DT con vin-supply=&vgp1 enciende VGP1 automáticamente
```

#### Paso 3: Verificar (1 día)
```sh
evtest /dev/input/eventX  # debe mostrar el edt-ft5x06
# multitouch real (hasta 5 dedos)
# IRQ-driven (0 CPU en reposo)
```

### Riesgos
- El I2C-mt65xx solo lee fiable ≤8 bytes (FIFO=8); el edt-ft5x06 necesita
  lecturas más largas para multitouch → hay que trocear o arreglar APDMA
- El reset GPIO115 puede no funcionar (readbacks `0x5`/`0xd`, hito 9)

### Criterio de éxito
`evtest` muestra multitouch (5 dedos), 0 CPU en reposo

---

## DRIVER 5: STP core (resync/CRC) — 🟠 MEDIO — 📋 ANALIZADO (0702): handoff con parche listo

**Hallazgo:** nuestro `stp_send` manda checksum=0 y CRC=0 y **BT funciona** → el FW por BTIF NO valida
CRC/checksum. La única mejora de valor es el **resync RX** (hoy `stp_pop_frame` lee el header a ciegas
→ un byte espurio desincroniza para siempre). NO portar stp_core.c entero (arrastra psm/btm ~1500 LOC
inútiles aquí). Parche de ~15 LOC (resync por sync-byte + patrón 4×0x7f del FW) listo para pegar +
plan de test en `mainline/wifi-consys/HANDOFF-STP-CORE-PORT-0702.md`. **No integrado** (arriesga BT y
no testeable ahora); hacerlo cuando el móvil esté libre.

## DRIVER 5 (plan original): STP core (resync/CRC) — 🟠 MEDIO

### Estado actual
- `mt6582-btif.c` (668 lín) — BTIF DMA simplificado, STP sin CRC
- BT funciona (hci0, emparejado con S24)
- Si un byte espurio llega por el BTIF, el parser STP se desincroniza

### Stock downstream
```c
// combo/common/core/stp_core.c (~5K LOC)
// - CRC16 por frame STP
// - Resync automático (escanea SYNC bytes)
// - Retransmisiones con timeout
// - Colas separadas por tipo (BT=0, FM=1, GPS=2, WIFI=3, WMT=4)
// - Flow control con el FW
```

### Plan de porteo

#### Fase A: Extraer stp_core.c del downstream (2 días)
- Copiar `stp_core.c` + headers
- Adaptar a 7.0.12: timers, spinlocks, workqueue
- El backend BTIF ya funciona (nuestro mt6582-btif.c)

#### Fase B: Integrar con btif (2-3 días)
- Reemplazar el parser STP simplificado del btif por el stp_core
- El stp_core recibe bytes crudos del BTIF y entrega frames completos con CRC

#### Fase C: Test de estrés (2 días)
- Enviar BT ACL masivo (transfer de fichero grande)
- Desconectar/reconectar BT repetidamente
- GPS + BT simultáneo

### Riesgos
- El stp_core depende de `wmt_lib` para algunas funciones → puede arrastrar dependencias
- Los timeouts del STP pueden no cuadrar con el FW de 2014

### Criterio de éxito
Transfer BT de 10MB sin desync; GPS + BT simultáneo sin pérdida de frames

---

## DRIVER 6: BTCVSD (audio BT SCO) — 🟠 MEDIO

### Estado actual
Sin implementar. El krillin tiene un BTCVSD (Bluetooth Codec Voice over SD)
que maneja el audio SCO por Bluetooth (auriculares BT, manos libres).

### Stock downstream
```
drivers/misc/mediatek/btcvsd/ — ~2K LOC
```

### Plan de porteo

#### Fase A: Entender la arquitectura (1 día)
El BTCVSD captura el audio del modem BT y lo enruta al codec:
- Recibe frames SCO del STP (canal BT)
- Los convierte a PCM
- Los entrega al ALSA como captura/playback

#### Fase B: Portear btcvsd (1 sem)
- Copiar el driver btcvsd del downstream
- Adaptar a ALSA SoC moderno (7.0.12)
- Integrar con el driver de audio (Driver 2)

#### Fase C: Test (2-3 días)
- Emparejar auriculares BT
- Reproducir audio por auriculares
- Llamar (si hubiera módem)

### Riesgos
- Depende del Driver 2 (Audio codec) para el playback
- Depende del STP core (Driver 5) para recibir los frames SCO

### Criterio de éxito
Audio por auriculares Bluetooth (A2DP + HFP)

---

## DRIVER 7: Vibrador + LEDs — 🟢 BAJO — ✅ CODE-COMPLETE (0702, sesión Mac; falta test HW)

### Implementado (0 código nuevo: todo drivers MAINLINE + DT)
- **Vibrador** = `regulator-haptic` (mainline) sobre `ldo_vibr` del MT6323 a **2.8 V**
  (cust_vibrator.c krillin: vib_vol=0x5). Input device con FF_RUMBLE → feedbackd/Phosh directo.
- **LED RGB + botones** = `leds-mt6323` (mainline, "mediatek,mt6323-led") con la tabla del
  downstream verificada: **green=ISINK0, red=ISINK1, blue=ISINK2, button-backlight=ISINK3**.
- Config: `CONFIG_LEDS_MT6323=y` + `CONFIG_INPUT_REGULATOR_HAPTIC=y` (deps MFD_MT6397 ya =y).
- DT: nodos `&pmic { leds {...} }` + `/ { vibrator {...} }` en `mt6582-bq-krillin.dts`
  (copia canónica versionada en `mainline/dts/`). DTB compila; zImage compila.
- NOTA: los borradores `mainline/drivers/mt6323-isink-led.c` y `mt6582-vibrator.c` quedan
  SUPERSEDIDOS por los drivers mainline (no compilados ni cableados).

### Test HW pendiente (cuando el móvil esté libre — coordina con la sesión WiFi antes de flashear)
```sh
# LEDs
echo 255 > /sys/class/leds/red:indicator/brightness      # y green/blue
echo 255 > /sys/class/leds/white:button-backlight/brightness
# Vibrador (input FF): fftest /dev/input/eventN (device "regulator-haptic"), o tocar en Phosh
```

### Estado actual (pre-port)
Sin implementar.

### Hardware
- **Vibrador**: motor DC controlado por GPIO o LDO del MT6323
- **LED de notificación**: LED RGB o simple controlado por PWM/MT6323

### Plan de porteo

#### Vibrador (1 día)
```dts
vibrator {
    compatible = "gpio-vibrator";
    enable-gpios = <&gpio XX GPIO_ACTIVE_HIGH>;
};
```
O si va por el MT6323:
```dts
vibrator {
    compatible = "mediatek,mt6323-vibrator";
    /* reg del MT6323 para el vibrador */
};
```

#### LEDs (1-2 días)
```dts
led-controller {
    compatible = "mediatek,mt6323-led";
    /* registros del MT6323 para LEDs */
};
```

### Stock downstream
```
drivers/misc/mediatek/vibrator/ — 3 .c, 520 LOC
drivers/misc/mediatek/leds/ — 2 .c, 1824 LOC
```

### Criterio de éxito
- `echo 1 > /sys/class/leds/vibrator/brightness` → vibra
- LED de notificación parpadea en Phosh

---

## DRIVER 8: Accdet (jack detection) — 🟢 BAJO

### Estado actual
Sin implementar. El krillin detecta la inserción de auriculares por un
GPIO o EINT del accdet del MT6582.

### Stock downstream
```
drivers/misc/mediatek/accdet/ — 2 .c
```

### Plan de porteo

#### Fase A: Identificar el GPIO/EINT (1 día)
- Buscar en el downstream el GPIO del accdet
- Verificar si es un EINT o un ADC del MT6323

#### Fase B: Driver simple (3-5 días)
- Si es EINT: nodo DT con `interrupts` + `extcon` driver
- Si es ADC: leer el AUXADC del MT6323 en polling

```dts
accdet {
    compatible = "mediatek,mt6582-accdet";
    interrupt-parent = <&eint>;
    interrupts = <XX IRQ_TYPE_EDGE_BOTH>;
    io-channels = <&auxadc YY>;
};
```

### Criterio de éxito
`cat /sys/class/extcon/extcon0/state` cambia al insertar auriculares

---

## DRIVER 9: Thermal — 🟢 BAJO — 🟡 INFRA LISTA (0702, sesión Mac): AUXADC validado; zona pendiente de un rail

### Hecho (0702)
- **AUXADC del SoC @0x11001000 VALIDADO por devmem** en el móvil: conversión real OK
  (toggle CON1 → DAT bit12=ready), layout idéntico a mt6577 (CON1=0x04, DAT0=0x14+4*ch),
  gate de reloj PERI bit24 ya abierto de fábrica (PERI_PDN0_STA=0x0 → sin riesgo de cuelgue de bus).
- DT: nodo `auxadc: adc@11001000` compatible `"mediatek,mt6582-auxadc","mediatek,mt2701-auxadc"`
  (bind directo al driver mainline `mt6577_auxadc`, 0 código) + fixed-clock. Compila (zImage+DTB).
- Config: `CONFIG_MEDIATEK_MT6577_AUXADC=y` + `CONFIG_GENERIC_ADC_THERMAL=y`.

### Bloqueado: canal 0 (NTC batería) lee 0 mV
El NTC (10K, pull-up 121K a 2.8V — mtk_ts_battery2.c) está a masa: **el rail de 2.8V del pull-up
está apagado en nuestro boot**. TODO: encontrar el rail (candidatos: VTCXO/VIO28/VGP del MT6323 —
probar encendiéndolos uno a uno y re-leyendo ch0 con `sh /tmp/scan.sh` o devmem).

### Zona térmica lista para pegar cuando el rail esté (generic-adc-thermal, tabla ya convertida a mV)
```dts
tbat_sensor: battery-thermal-sensor {
	compatible = "generic-adc-thermal";
	#thermal-sensor-cells = <0>;
	io-channels = <&auxadc 0>;
	io-channel-names = "sensor-channel";
	temperature-lookup-table = <
			( -20000) 1010
			( -15000)  860
			( -10000)  728
			(  -5000)  613
			       0  514
			    5000  431
			   10000  361
			   15000  303
			   20000  254
			   25000  214
			   30000  180
			   35000  152
			   40000  129
			   45000  109
			   50000   93
			   55000   79
			   60000   68 >;
};
```
(trip inicial tipo "hot", NO "critical" — con el rail caído leería 60°C y un critical apagaría el móvil)

### Pendiente aparte: sensor on-die de CPU (controlador thermal dedicado + calibración efuse
del downstream mtk_ts_cpu.c) — trabajo medio, evaluar tras STP/audio.

## DRIVER 9 (plan original): Thermal — 🟢 BAJO

### Estado actual
Sin implementar. El MT6582 tiene sensores de temperatura internos
 que pueden usarse para throttle del CPU.

### Stock downstream
```
drivers/misc/mediatek/thermal/ — 29 .c
```

### Plan de porteo

#### Fase A: Identificar el sensor (1 día)
- El MT6582 tiene un sensor de temperatura on-chip
- Puede leerse por AUXADC del MT6323 o por registro MMIO del SoC

#### Fase B: Driver thermal zone (3-5 días)
```dts
thermal-zones {
    cpu_thermal: cpu_thermal {
        polling-delay-passive = <1000>;
        polling-delay = <5000>;
        thermal-sensors = <&temp_sensor>;
        trips { /* throttle points */ };
        cooling-maps { /* cpufreq scaling */ };
    };
};
```

### Riesgos
- El sensor de temperatura puede requerir calibración de fábrica (NVRAM)
- El thermal framework de 7.0.12 cambió desde 3.10

### Criterio de éxito
`cat /sys/class/thermal/thermal_zone0/temp` devuelve temperatura real (~35-45°C idle)

---

## DRIVER 10: FM Radio — 🔴 BAJA PRIORIDAD

### Estado actual
Sin implementar.

### Stock downstream
```
drivers/misc/mediatek/fmradio/ — 37 .c, 29K LOC
```

### Plan de porteo

#### Resumen
El FM radio del CONSYS es una función más del WMT (tipo=1). Funciona por STP.
El driver downstream expone una interfaz V4L2 (frequency set/get, seek, RDS).

#### Fases
1. `func_on(FM)` en el btif (1 día)
2. Exponer `/dev/radio0` con V4L2 ioctl básico (1 sem)
3. Portear el driver completo del downstream con RDS (4-5 sem)

### Criterio de éxito
`fmtools -f 95.8` → suena radio FM por el altavoz

### Por qué es baja prioridad
29K LOC de driver para una funcionalidad que casi nadie usa en 2026.
Hacerlo solo después de que TODO lo demás funcione.

---

## ORDEN DE EJECUCIÓN RECOMENDADO

```
FASE 1 (CRÍTICA):
  Driver 1: WiFi stock port              6-9 sem
    ↓
FASE 2 (ALTA):
  Driver 2: Audio codec                  2-3 sem
  Driver 3: GPS (START_SEQ)              1-2 días
    ↓ (paralelo)
FASE 3 (MEDIA):
  Driver 4: Touch kernel (edt-ft5x06)    2-3 días
  Driver 5: STP core (resync/CRC)        1 sem
  Driver 6: BTCVSD (audio BT)            1-2 sem
    ↓ (paralelo)
FASE 4 (BAJA):
  Driver 7: Vibrador + LEDs              2-3 días
  Driver 8: Accdet                       1 sem
  Driver 9: Thermal                      1 sem
    ↓
FASE 5 (FUTURO):
  Driver 10: FM Radio                    4-6 sem
```

### Dependencias entre drivers
```
WiFi (1) ── independiente de todo
Audio (2) ── independiente (pero BTCVSD lo necesita)
GPS (3) ── independiente (usa btif)
Touch (4) ── independiente
STP (5) ── independiente (mejora BT/GPS)
BTCVSD (6) ── depende de Audio (2) + STP (5)
Vibrator/LEDs (7) ── independiente
Accdet (8) ── depende de Audio (2) para switching de ruta
Thermal (9) ── independiente
FM (10) ── depende de STP (5)
```

---

## ESTADO ACTUAL DEL DISPOSITIVO (jul-2026)

| Subsistema | Estado | Driver |
|---|---|---|
| Boot + SMP + eMMC + ext4 | ✅ | mainline |
| Display DRM + Phosh + lima | ✅ | mainline + adaptaciones |
| USB gadget (usb0 + SSH) | ✅ | mt6582-musb.c (nuestro) |
| GPIO + EINT (169 pines) | ✅ | gpio-mt6582-eint.c (nuestro) |
| PMIC MT6323 + reguladores | ✅ | mainline + patches |
| Keypad (Vol-) | ✅ | mt6779-keypad (mainline) |
| Watchdog (mtk-wdt) | ✅ | mainline |
| Bluetooth (hci0) | ✅ | mt6582-btif.c (nuestro) |
| CONSYS power-on + RF-cal | ✅ | mt6582-consys.c (nuestro) |
| **WiFi ABIERTO** | ✅ | mt6582-wifi.c (nuestro) |
| Backlight slider (Phosh) | ✅ | gsd-power + daemon |
| **WiFi WPA2** | ❌ DHCP falla | → **PORT Driver 1** |
| **Audio** | ❌ sin sonido | → **PORT Driver 2** |
| **GPS** | 🟡 no streamea | → **Driver 3** |
| **Touch multitouch** | 🟡 single-touch | → **Driver 4** |
| **Audio BT (SCO)** | ❌ | → **PORT Driver 6** |
| **Vibrador + LEDs** | ❌ | → **Driver 7** |
| **Accdet** | ❌ | → **Driver 8** |
| **Thermal** | ❌ | → **Driver 9** |
| **FM Radio** | ❌ | → **Driver 10** |
| Cámara | ❌ | ISP propietario (muy difícil) |
| Módem 3G | ❌ | CCCI propietario (muy difícil) |
| Vol+ | ❌ | Necesita pinctrl-mt6582 |

---

## REGLAS DE ORO (todos los drivers)

- `fastboot flash boot` NUNCA `fastboot boot`. mtkclient `wo` NUNCA `wf`.
- NO flashear preloader. NO restaurar LK Lollipop (bueno = KitKat 1.5.2).
- `dd` desde pmOS funciona; desde Android NO (verity).
- Reboot: `echo 1 > /proc/sys/kernel/sysrq; echo b > /proc/sysrq-trigger`.
- Si sshd no arranca → power-cycle físico.
- pstore: `/sys/fs/pstore/console-ramoops-0` — leer antes de reiniciar tras crash.
- NO `iw dev wlan0 scan` estando conectado WPA2 (crashea con driver actual).
- Co-autor de commits: `Co-Authored-By: Claude <noreply@anthropic.com>`.

---

## ENTORNO

- **Pi de casa**: `cpcd@192.168.0.38` (sudo NOPASSWD). Kernel: `~/mainline/linux-7.0.12`.
  Build dir: `build-krillin`. Build cmd: `make O=build-krillin ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 zImage dtbs`.
- **Pi de sesión Mac**: `cpcd@192.168.0.123` (otra distinta — no confundir).
- **Móvil**: `root@172.16.42.1` (USB gadget). Reconectar: `sudo ip addr replace 172.16.42.2/24 dev usb0`.
- **Build+flash WiFi**: `~/wifi-iter-w.sh` (build zImage + dd sector 83968 + reboot).
- **Empaquetar boot.img**: `~/mainline/pkg/assemble.sh` o manual (`mtk_hdr.py` + `abootimg`).
- **Downstream**: `~/mainline/downstream/drivers/misc/mediatek/`.
- **pstore**: `/sys/fs/pstore/console-ramoops-0`.
- **Dual-boot pmOS ↔ LineageOS**: `fastboot flash boot boot-btifDMA-sd.img` / `lineage13-boot.img`.

*Co-autor: opencode (glm-5.2).*
