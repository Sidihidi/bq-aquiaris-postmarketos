# PLAN MAESTRO DE IMPLEMENTACIÓN — BQ Aquaris E4.5 "krillin" (MT6582) en Linux mainline 7.0.12

**Actualizado 2026-06-17 (tarde)** — sustituye a `ROADMAP-DRIVERS.md` (mañana), incorporando todo lo
logrado hoy: PMIC al DT, batería %, carga, **EINT (driver propio)**, **keypad (vol↓)** y **simpledrm**.

---

## 0. Filosofía del port y cuello de botella raíz

**El gap que bloquea todo lo "profundo": el MT6582 no tiene en mainline ni `clk` driver (CCF) ni
`pinctrl` completo.** La estrategia que funciona (y que seguimos) es:

1. **fixed-clock stubs + `clk_ignore_unused` + el LK deja los bloques encendidos.** Evita escribir el CCF.
2. **Portar el driver mainline** si existe (cambiar/añadir un `compatible`, nodo DT, config).
3. **Adaptar** el driver de un SoC hermano de la misma generación (MT8135 / MT8127 / MT6779) — los IP son casi idénticos.
4. **Daemon userspace como puente** cuando el driver de kernel está bloqueado (táctil, batería, carga).
5. **Crear desde 0** solo cuando no hay ninguna de las anteriores (audio, WiFi, CCF).

> Regla de oro demostrada hoy 3 veces: **el IP del MT6582 == el de su generación**. El i2c, el EINT y el
> keypad funcionaron con drivers mainline de MT6577/MT8127/MT6779 **sin tocar el driver**, solo el DT.

---

## 1. ✅ ESTADO ACTUAL — lo que YA funciona en hardware

| Subsistema | Cómo | Tipo de solución |
|---|---|---|
| Arranque SMP 4×A7, consola | DT mínimo + LK deja clocks | nativo |
| **Display** legible (color OK) | simplefb RGB565 + `mt6582-dispfix.c` (reprograma OVL) | driver propio mínimo |
| **eMMC** | `mtk-sd` (compat mt8135) | portado (DT) |
| **USB gadget + SSH** | `mt6582-musb.c` (driver propio) + g_ether | creado |
| **I2C0** | `i2c-mt65xx` (compat mt6582 propio) | adaptado |
| **GPIO** | `gpio_chip` dentro de `gpio-mt6582-eint.c` | creado (mínimo) |
| **EINT** (interrupciones ext.) | `gpio-mt6582-eint.c` (librería `mtk-eint`, 169 pines, GIC_SPI113) | **creado hoy** |
| **Táctil FT5336** | daemon userspace `ft5336_touch.c` (I2C0) → `/dev/input/event*` | puente userspace |
| **Keypad — Vol↓** | `mediatek,mt6779-keypad` (offsets idénticos) | **portado hoy (DT)** |
| **GUI X11 táctil** | Xorg fbdev + jwm + matchbox-keyboard (Alpine) | userspace |
| **PMIC MT6323** | pwrap (`mt6582-pwrap`) + MFD `mt6397` + 31 reguladores en DT | adaptado |
| **Batería %** | lector VBAT por AUXADC del MT6323 (userspace) | puente userspace |
| **Carga** | daemon `FAN5405` (charger externo, I2C) | puente userspace |
| **sshd, NAT, rootfs Alpine en eMMC** | OpenRC | userspace |

---

## 2. 🔄 A MEDIAS — empezado, falta completar

| Item | Qué hay | Qué falta | Bloqueante |
|---|---|---|---|
| **simpledrm** | Compilando ahora (`DRM_SIMPLEDRM=y`, `FB_SIMPLE` off, dispfix→obj-y) | Validar `/dev/dri/card0` + pantalla OK | — (en vuelo) |
| **EINT → pinctrl** | `gpio-mt6582-eint.c` da EINT + gpio_chip básico (get/set/dir) | **pinmux + pull-up/down** (= `pinctrl-mt6582.c`) | falta pinctrl |
| **Keypad — Vol↑** | Vol↓ funciona | configurar el pin de su columna (pull-up) | falta pinctrl |
| **Táctil — IRQ nativo** | daemon por polling funciona | `edt-ft5x06` por IRQ EINT117 | **APDMA del i2c** (lee 33B; hoy ≤8B) |
| **Táctil ↔ GUI** | event device correcto (ABS_MT + BTN_TOUCH) | que la GUI X11 lo consuma (config Xorg/calibración) | config userspace |
| **Batería/Carga** | funcionan por userspace | (opcional) pasarlos a framework kernel (IIO/power_supply) | nada urgente |

---

## 3. 📋 POR HACER — clasificado por tipo de trabajo

### 3A · PORTAR / ADAPTAR (el driver existe en mainline; trabajo = DT + config + a veces un `compatible`)

| Driver | Origen | Trabajo | Desbloquea | Esfuerzo |
|---|---|---|---|---|
| **`pinctrl-mt6582.c`** | molde `pinctrl-mt8135.c` + tabla de pines del downstream + **mi gpio/eint ya hecho** | generar `pinctrl-mtk-mt6582.h` (169 pines) y el `.c`; integra el gpio_chip+eint actuales | Vol↑, **botones limpios**, táctil-IRQ, `reset-gpios`/`vin-supply` declarativos, wakeup | **media-alta** ⭐ |
| **Sensores IIO** (accel/gyro/ALS) | drivers ST/AMS upstream | añadir nodos hijos a `&i2c0` (sin `interrupts`, polling) + configs IIO | autorrotación, brillo auto (iio-sensor-proxy) | **baja** ⭐ quick win |
| **LED / vibrador / poweroff MT6323** | `leds-mt6323`, `mt6323-poweroff` (ya en mainline) | nodos DT hijos del MFD (que ya tenemos) | notificación LED, vibración, apagado limpio | baja |
| **lima (GPU Mali-400)** | driver `lima` mainline (`DRM_LIMA=m` ya está) | nodo `gpu@13010000` (`arm,mali-400`) + 6 IRQs GIC + 2 fixed-clock + gate MFG ON por LK | GLES2 → Phosh acelerado (sobre card0 de simpledrm vía kmsro) | media |
| **simpledrm** | mainline (`drivers/gpu/drm/sysfb/`) | *(en progreso)* | `/dev/dri/card0` → Phosh | baja |

### 3B · ARREGLAR / COMPLETAR código existente

| Tarea | Qué | Desbloquea | Esfuerzo |
|---|---|---|---|
| **APDMA del `i2c-mt65xx`** | habilitar/portar el DMA para lecturas > 8 bytes (hoy FIFO≤8) | `edt-ft5x06` nativo (táctil multitouch por IRQ), retira el daemon | media |
| **pinctrl (ver 3A)** | es el "completar" del EINT actual | — | — |

### 3C · CREAR DESDE 0 (no existe nada en mainline)

| Driver | Realidad | Estrategia | Esfuerzo / viabilidad |
|---|---|---|---|
| **`clk-mt6582.c` (CCF)** | El cuello de botella raíz teórico | **Evitable**: fixed-clock stubs + `clk_ignore_unused` ya cubren todo. Solo escribirlo si algún driver exige gating real | alto / **opcional** |
| **WiFi `CONSYS_6582`** | CERO en mainline; stack on-die de cientos de miles de líneas | Portar `conn_soc`+`wlan/gen2` out-of-tree (shims 3.4→7.x) **o** atajo: **dongle USB WiFi** (`mt7601u`/`rtl8188eu`) por OTG | muy alto / **atajo recomendado** |
| **Audio (AFE MT6582 + codec MT6323)** | NADA en mainline; 3-4 drivers nuevos | codec `mt6323` (patrón mt6358) → AFE `mt6582` (0x11220000) → machine; validar solo DL1 (altavoz) | muy alto / bajo ROI |
| **Charger en kernel** | hoy resuelto por daemon FAN5405 (userspace) | (opcional) driver `power_supply` para el FAN5405 | bajo / opcional |

### 3D · FUERA DE ALCANCE mainline (decisión arquitectónica)

| Item | Por qué | Camino real |
|---|---|---|
| **Modem 3G (CCCI/MOLY)** | Nadie ha hecho funcionar el baseband on-die de ningún MTK clásico en mainline | Halium + rild binario sobre el **kernel downstream 3.10** (ya probado por Ubuntu Touch en krillin). En mainline: solo **dump defensivo** de `nvram`/`protect_*`/`md1img` (IMEI/calibración irreemplazables) |

---

## 4. Orden de implementación recomendado (actualizado post-hoy)

```
HECHO ──► PMIC ✅ · batería ✅ · carga ✅ · GUI-X11 ✅ · EINT ✅ · keypad-Vol↓ ✅

AHORA ──► (1) simpledrm  →  /dev/dri/card0            [EN PROGRESO]
              └─► (2) Phosh por software (pixman) sobre card0  → GUI MÓVIL de verdad (lenta pero usable)

LUEGO ──► (3) pinctrl-mt6582   ⭐ el gran habilitador que falta
              ├─► Vol↑ + power-key (MT6323 PWRKEY)
              ├─► táctil por IRQ (con APDMA) → edt-ft5x06 nativo, retira daemon
              └─► reset-gpios / vin-supply / wakeup declarativos

PARALELO ─► (4) sensores IIO (quick win: autorrotación/brillo)
            (5) LED/vibrador/poweroff MT6323 (nodos DT)

DESPUÉS ─► (6) lima (GPU)  →  Phosh ACELERADO (GLES2, "usable, no snappy")

CONECTIVIDAD ► (7) WiFi: primero DONGLE USB (rápido); CONSYS nativo = proyecto aparte
AUDIO ──────► (8) AFE+codec (cuando haya ganas; ROI bajo)
MODEM ──────► (9) fuera de mainline → Halium/downstream
```

**Próximo gran objetivo tras la GUI: `pinctrl-mt6582.c`.** Es el único "gap de plataforma" que de verdad
nos frena ahora (Vol↑, botones, táctil-IRQ, declaratividad del DT). Y ya tenemos hecho medio camino: el
`gpio_chip` + EINT del `gpio-mt6582-eint.c` se integran dentro del pinctrl. El CCF (clocks) lo seguimos
**esquivando** con fixed-clocks mientras no haga falta.

---

## 5. Resumen en una línea por categoría

- **PORTAR/ADAPTAR (fácil-medio):** pinctrl-mt6582 ⭐, sensores IIO, LED/vibrador/poweroff MT6323, lima, simpledrm.
- **ARREGLAR existente:** APDMA del i2c (→ táctil nativo).
- **CREAR desde 0 (duro):** WiFi CONSYS (→ atajo: dongle USB), audio AFE, (clk-mt6582 = opcional).
- **A MEDIAS:** simpledrm (validando), EINT→pinctrl, keypad Vol↑, táctil-IRQ, táctil↔GUI.
- **FUERA:** modem 3G (→ Halium sobre 3.10).
```
