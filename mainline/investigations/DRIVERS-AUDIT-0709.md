# Auditoría de drivers — 2026-07-09 (sesión paralela "drivers")

> **Misión de esta sesión (Windows/Opus):** mientras la sesión principal trabaja el **SPM (M3 deep-sleep)**,
> esta sesión audita qué drivers están a medias / sin empezar y **los escribe/avanza SOLO EN CÓDIGO**
> (compilando en la Pi con un build dir propio `build-drv`, **sin tocar el móvil ni build-krillin**).
> La sesión principal integra y prueba en HW cuando le venga bien.

## Matriz de estado (síntesis de HITOS + PORT-STRATEGY-0707 + tree del repo)

### ✅ HECHOS (funcionan en HW)
| Subsistema | Evidencia |
|---|---|
| Boot SMP, MUSB, display DRM (mtk_drm), GPU lima, táctil, PMIC MT6323, carga FAN5405 | HITOS 1-19 |
| WiFi WPA2+DHCP completo | MILESTONE-WIFI-MAINLINE-COMPLETO-0704 |
| Audio playback (altavoz+auriculares, AFE+codec analógico) | MILESTONE-AUDIO-FUNCIONA-0706 |
| Batería VBAT hwmon kernel (mt6323-auxadc.c) | battery/VBAT-HWMON-0708 |
| Power key kernel (mtk-pmic-keys) + powerkey v8 userspace | input/, commits 0708 |
| Sensores: accel (auto-rotación ✅), gyro, luz | sensors/FINDINGS-SENSORS-0706 |
| Auto-brillo ALS + autosuspend híbrido + powermenu | userspace/, commits 0708 |
| LEDs + vibración | HANDOFF-LED-NOTIF-0706 |
| FM: enciende + sintoniza (chipid fix + FW patch v1) | fm/PARA-CASA-FM-SIGUIENTE |
| Multiboot (menupick) + s2idle M1/M2 | bootmenu/, spm/ |

### 🔨 EN CURSO — OTRA SESIÓN (no tocar desde aquí)
| Qué | Estado |
|---|---|
| **SPM M3** (deep-sleep: dormant/resume de CPU0) | HANDOFF-SPM-SOAK-DIAG-0709 — crash NO es el offline, es el sleep de CPU0; kernel #269 instrumentado |
| Maemo-Leste port | PLAN-UBOOT-MAEMO-LESTE-0708 |

### 🟡 A MEDIAS (esta sesión los avanza en código)
| Qué | Falta | Plan |
|---|---|---|
| **FM audio** (sintoniza, no suena) | `aif_ctrl_cb` no-op + ruteo codec (¿analógico line-in MT6323 o I2S→AFE?) | RE del downstream (cmb_stub + krillin audio cust) → implementar callback + secuencia codec + script pwrap de validación |
| **Proximidad** (lee raw, proxy no la expone) | eventos/umbral tsl2772 | baja prioridad, tras lo gordo |

### ✅ HECHO ESTA SESIÓN (0709) — código completo + compila, SIN probar HW
| Qué | Entregable (en el repo) | Estado |
|---|---|---|
| **Magnetómetro MMC3516x** | `mainline/sensors/mmc3516x/` (driver IIO ~400 L + INTEGRATION) | **`mmc3516x.ko` COMPILA** ✓ |
| **Thermal CPU** | `mainline/thermal/` (auxadc_thermal + parche 202 L + DTS + INTEGRATION) | **`auxadc_thermal.ko` COMPILA** ✓ |
| **Accdet jack** | `mainline/audio-accdet/` (mt6323-accdet ~450 L + INTEGRATION) | **`mt6323-accdet.ko` COMPILA** ✓ |
| **FM audio** | `mainline/wifi-consys/fm/FINDINGS-FM-I2S-ROUTE-0709.md` | **RE cerrado**: I2S interno PAD_CONN + receta de registros AFE exacta |
| **GPS Fase A** | `mainline/wifi-consys/gps/FASE-A-ENLAZA-0709.md` + `fase-a-shim/` | **✅ ENLAZA** (binario ARM estático: libmnl_6628 + glue AOSP + shims KAL) |

Los `.ko`/binarios de prueba están en `~/drivers-wip/` y `~/gps/fase-a/` de la Pi .38 (todo en **build-drv**, NUNCA build-krillin). Cada uno con su `INTEGRATION.md`/`FINDINGS` (DT + config + qué validar en HW).

### 🔴 FUERA (decisión estrategia-0707)
Módem (MOONSHOT, solo spike explícito) · Cámara (MOONSHOT/NO-GO, solo spike oportunista) · FM ya se decidió atacarlo pese al NO-GO y sintoniza (queda solo el audio) · BTCVSD (DEFER).

## Reglas de esta sesión paralela
1. **No toca el móvil** (la sesión SPM lo necesita para soaks) — solo código + compile en Pi.
2. **Build dir propio** `~/mainline/linux-7.0.12/build-drv` (modules_prepare); NUNCA build-krillin.
3. Drivers nuevos se compilan **out-of-tree** (`make M=`) → no ensucian el árbol compartido.
4. Entregables a `mainline/<subsys>/` del repo con doc de integración por driver.
5. Los ficheros compartidos calientes (mt6582-btif.c, dts) se entregan como **parche**, no editados.

*Sesión paralela drivers (Opus), 2026-07-09.*
