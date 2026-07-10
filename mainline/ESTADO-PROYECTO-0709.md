# ESTADO REAL DEL PROYECTO — sincronización de sesiones (2026-07-09, sesión principal/casa)

> Documento de coordinación. Corrige datos rancios de `DRIVERS-AUDIT-0709.md` y define carriles.
> Regla de oro: **UN solo móvil y UN solo build-krillin** — solo la sesión principal flashea/itera en HW.

## 1. Estado real por subsistema

### ✅ FUNCIONA EN HARDWARE (validado por el usuario o por test en HW)
| Subsistema | Detalle | Desde |
|---|---|---|
| Boot SMP 4 cores, display DRM, GPU lima, táctil, MUSB/usb0 | base estable | HITOS |
| WiFi WPA2+DHCP+GUI + SSH nativo | hueco conocido: RX GTK TKIP (BSS mixto) | 0704 |
| Audio completo (aplay/mpv/PulseAudio, altavoz+auriculares) | AFE+codec en driver kernel | 0706 |
| **Radio FM COMPLETA: sintoniza Y SUENA + app GUI** | ⚠️ CORRIGE al audit: NO está "a medias" — RESUELTA 0708 (RESTORE_SEARCH=RampDown + conexión directa CONN1 b16/CONN2 b1; app GTK4 "Radio FM" con seek/presets/volumen) | 0708 |
| Batería VBAT por driver kernel (hwmon mt6323-auxadc) | pwrap_poke retirado del camino periódico | 0708 |
| Botón power v8 (helper socket + IRQ-delta + rescate VOL−) | wedge del INT del PMIC diagnosticado y mitigado | 0708-09 |
| Auto-rotación + auto-brillo ALS + vibración + LEDs | — | 0706-08 |
| Suspend s2idle (freeze) + autosuspend HÍBRIDO (20 min idle ligero → s2idle) | daily validado | 0706-09 |
| **SPM M1+M2+M3**: deep sleep vía PCM con CPU0 dormant | 4/5 ciclos limpios; ver §2 | 0709 |

### 🔨 EN CURSO — SESIÓN PRINCIPAL (única que toca móvil/build-krillin)
1. **SPM**: ✅ raíz del BUG_ON del offline ARREGLADA (#273: `v7_exit_coherency_flush(louis)` en
   `mtk_cpu_die` — el core se aparcaba en WFI dentro de la coherencia SMP; commit 79697d0).
   QUEDA: **cuelgue post-resume raro** (~1/7 ciclos, 1-3s tras `suspend exit`, sin oops = bus lockup).
   **Experimento en vuelo** (0709 noche): soak AISLADO (WiFi down + daemons de polling parados) —
   última lectura 6/6 limpios; el resultado final está en `/root/soakiso.log` del móvil (leer al
   volver a estar disponible). Si sale 10/10 → bisecar WiFi vs daemons; el script restaura todo solo.
   Después: **M4** (infra_pdn = suspend profundo completo).
2. **Estabilización del boot** (siguiente hito tras el SMP): el boot se ha desestabilizado con las
   últimas iteraciones (sshd flaky, arranques parciales). Plan: auditar orden/serialización de local.d,
   el `init-menupick` del multiboot (nuevo actor en la cadena), cadena de entropía tras resets duros,
   y medir N boots con timestamps. Objetivo: secuencia determinista y supervisada.

### 🧪 CARRIL SESIÓN PARALELA "drivers" (código+compile SOLO; entrega parche+guía; NO móvil, NO build-krillin)
| Entregable | Estado | Validación HW (sesión principal) |
|---|---|---|
| Magnetómetro MMC3516x (IIO nuevo) | COMPILA, en repo | pendiente de integrar+flash |
| Thermal MT6582 (auxadc_thermal + entrada mt6582) | COMPILA, parche 202 líneas | pendiente |
| Accdet jack+botones (mt6323-accdet) | COMPILA | pendiente |
| GPS Fase A (runner libmnl + shims KAL, meta: que enlace) | por empezar | — |
| ~~FM audio~~ | **QUITAR DEL CARRIL: ya resuelto 0708** | — |
| Proximidad en iio-sensor-proxy (umbral tsl2772) | baja prioridad | — |

Verificación técnica de los 3 drivers entregados: **en curso** (agente contrastando contra el
downstream); resultados se anexarán como `VERIFICACION-DRIVERS-PARALELA-0709.md`.

### 🔬 EXPERIMENTOS (no bloquean el carril principal)
- **Maemo-Leste**: rootfs construido + menú multiboot de 3 entradas (pmOS/Android/Maemo).
  ⚠️ El `init-menupick.sh` entra en la cadena de boot → candidato en la auditoría de estabilización.
- Bootloader custom: estudio GO (sesión Mac); fastbootd/Android13 NO-GO.

### 🔴 FUERA (sin cambios)
Módem (moonshot, solo spike explícito) · Cámara (NO-GO) · BTCVSD (defer).

## 2. SPM — estado técnico exacto (para cualquier sesión que lo retome)
- **Funciona**: PCM v35rc1 cargado, wake por EINT (botón/RTC) y PCM_TIMER, dormant de CPU0 con
  resume por BootROM→0x10001800→cpu_resume, GIC salvado/restaurado (cpu_cluster_pm), 4 cores de
  vuelta (MTCMOS en cpu_kill/boot_secondary + ventana del bit31 solo-durante-el-ciclo).
- **Falla**: intermitente, el OFFLINE consecutivo de los 3 cores durante `mem` (no el sleep). Con el
  blindaje ya NO crashea: aborta el suspend y sigue vivo.
- **Config actual del móvil (#272)**: `spm_cpu_pdn=0` forzado al boot (mem=M2); daily=freeze (s2idle).
- **Misterio abierto**: algo pone `spm_cpu_pdn=1` al boot (no es cmdline del kernel ni del bootimg ni
  local.d ni /etc/init.d — neutralizado por zzz-spm-safe.start; candidato: ¿modprobe.d? ¿initramfs?).
- Docs: `spm/HANDOFF-SPM-SOAK-DIAG-0709.md` (método de captura no_console_suspend + guardián pstore).

## 3. Reglas de sincronización (las 3 sesiones)
1. **Móvil y build-krillin = SOLO sesión principal.** Paralela usa `build-drv` out-of-tree; Mac avisa antes de flashear.
2. **git pull antes de editar, commit+push después** — el repo es la verdad. Ficheros calientes
   (platsmp.c, mt6582-spm.c, dts, btif) se entregan como PARCHE si no eres la sesión principal.
3. **Antes de trabajar un subsistema: leer este doc + HITOS.md** (evita repetir lo ya resuelto — caso FM audio).
4. Los descubrimientos que REFUTAN algo previo se marcan explícitamente (⚠️ CORRIGE...) y actualizan este doc.
5. El pstore-guardián (`00-pstore-save.start`) conserva las trazas de crash en `/root/pstore-logs/` — leer antes de reflashear.

*Sesión principal (Fable 5, casa), 2026-07-09.*
