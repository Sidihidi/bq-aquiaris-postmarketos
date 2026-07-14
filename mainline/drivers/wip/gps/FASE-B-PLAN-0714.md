# GPS Fase B — plan de ejecución (síntesis Fase A + H0) — 0714

> Sintetiza `FASE-A-ENLAZA-0709.md` (runner estático que ENLAZA) + `H0-GROUNDTRUTH-GPS-0713.md`
> (stack stock extraído + capturado emitiendo NMEA). Resuelve **qué vía** y fija la **ruta crítica**.
> Objetivo Fase B: **un fix real → geoclue → Phosh**. HW en `.38` (el móvil).

## Recordatorio que decide el diseño (no re-litigar)
GPS del krillin = **HOST-BASED (MSB)**. El DSP del combo solo emite **medidas crudas** (pseudorangos)
en 0xAAF0 por `/dev/stpgps`; **la POSICIÓN (PVT) la calcula `libmnl` en la CPU**. → **sin `libmnl`
corriendo NO hay fix.** Un bridge que solo parsea 0xAAF0 (el viejo `mtkgps_aaf0.c`) es un callejón para el fix.

## Las dos vías (ambas de-riesgadas; recomendación abajo)
| | Vía 1 — stock `mnld`+`libmnl.so` bajo bionic | Vía 2 — runner estático (glue + `libmnl_6628.a`) |
|---|---|---|
| Estado | binarios extraídos (H0), **capturados emitiendo NMEA** en Lineage | **ya enlaza** (Fase A), binario ARM estático musl |
| Config/TCXO/START | **lo hace `mnld` solo** (lee NVRAM por `libnvram`, emite el START) | **hay que re-implementarlo** en `init_cfg` a mano |
| ABI floats | **exacto** (binarios bionic sin recompilar) → sin riesgo | riesgo soft/hard-float (⚠️ Fase A §ABI) |
| Coste | montar runtime bionic (linker + libs, ya extraídas) en Alpine | ninguno extra, pero resolver config+ABI |

**RECOMENDACIÓN: Vía 1 primero.** Correr el `mnld` real evita re-implementar lo más frágil (lectura de
NVRAM/TCXO + construcción del START, que es exactamente donde se atascó el bridge en 0707) **y** elimina
el riesgo ABI. El runner estático (Vía 2) queda como **fallback ya construido** si el runtime bionic da guerra.

## Ruta crítica (común): la calibración TCXO de la NVRAM
Sin los TCXO reales (`hw_Clock_Freq` / `hw_Clock_Drift` / `u1ClockType`, campos de `init_cfg` en
`CFG_GPS_File.h`) **el DSP NO adquiere** (Fase A §SIGUIENTE.1; el `read` bloqueado de 0707 encaja).
- **Vía 1**: `mnld`+`libnvram` lo leen solos de la **partición `nvram`** — sub-tarea = tener esa NVRAM
  disponible en pmOS (extraer el registro GPS de la `nvram` del krillin; el H0 del módem ya sacó el
  *"set NV extraído (nvram+protect)"* — reusar). Localizar el item: buscar el record GPS/AGPS en
  `libnvram`/`CFG_GPS_File.h` del downstream (`~/mainline/downstream/.../gps/`).
- **Vía 2**: extraer esos 3 valores y hardcodearlos en `init_cfg` del runner.

## Loop de prueba en HW (pmOS; `/dev/stpgps` YA funciona por WMT)
0. **Restaurar pmOS**: el móvil quedó en Lineage tras el rescate → flashear
   `~/mainline/pkg/boot-menupick13-DSIFIX-GANADOR.img` (sector 83968) y arrancar pmOS del menú.
1. Desplegar Vía 1 (mnld + libmnl.so + libs bionic + linker en un prefix, p.ej. `/opt/gps-bionic/`) o
   Vía 2 (el runner estático en `/usr/local/bin/`).
2. Arrancar el radio: abrir `/dev/stpgps` (dispara `func_on[GPS]` del WMT). Confirmar `/dev/gps` presente.
3. Ejecutar mnld/runner con `init_cfg`: `dsp_port=/dev/stpgps`, `nmea_port=<pty>`, TCXO de NVRAM.
4. **Instrumentar el `write` al dsp_fd** (strace `-s 512 -e write` o log): confirmar que el **START burst
   0xAAF0 sale en NUESTRO HW** (esto es lo que 0707 no logró con el bridge stub).
5. ¿El DSP **responde**? Bytes de vuelta en `/dev/stpgps` = medidas crudas → libmnl las procesa.
6. **libmnl → NMEA** por el pty/socket. Si el NMEA sale con **números basura** = el riesgo ABI floats
   (solo puede pasar en Vía 2) → fix: sysroot softfp, o cambiar a Vía 1.
7. **Cadena de consumo (ya existe)**: NMEA → `gpsd -N -n <pty>` → geoclue → Phosh. Retirar `mtkgps_aaf0.c`.
8. **Fix real en EXTERIOR** (vista al cielo; primer fix frío puede tardar >30 s / necesitar EPO de AGPS).

## Criterio de éxito Fase B
`cgps` o geoclue muestran **lat/lon reales** (≈ posición real) y **Phosh ubica**. AGPS (`mtk_agpsd` + EPO/NTP)
es opcional (acelera el primer fix); no bloquea el fix básico.

## Riesgos y mitigaciones
- **Runtime bionic en Alpine/musl** (Vía 1): montar el linker bionic + libs extraídas; si full-libhybris es
  demasiado, cargar directo por el linker bionic (libmnl.so solo pide libc/libm/libstdc++ bionic — H0).
- **NVRAM GPS ausente/incorrecta** → DSP no adquiere: reusar el set NV que ya extrajo el H0 del módem.
- **Primer fix**: en interior NUNCA hay fix (0 satélites, visto en H0). Probar SIEMPRE con cielo despejado.

## Qué NO volver a hacer
- No perseguir el fix con el bridge 0xAAF0 solo (`mtkgps_aaf0.c`) — sin libmnl no calcula posición.
- No gastar el dual-boot LineageOS "para capturar el START" como si cerrara el GPS — el START lo genera
  `mnld` en NUESTRO HW con los TCXO reales (H0 §reframe).

*Plan Fase B 2026-07-14 (sesión Mac, síntesis). Ejecutable por la sesión con el móvil (.38). Vía 1 = mnld
stock bajo bionic (recomendada); Vía 2 = runner estático (fallback ya enlazado).*
