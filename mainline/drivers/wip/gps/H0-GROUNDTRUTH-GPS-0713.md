# GPS — ground-truth del stock (LineageOS 13) — 0713

> Extraído con el móvil en LineageOS 13 stock, adb root. **Todo el stack GPS del stock extraído +
> capturado funcionando en vivo.** Complementa `FINDINGS-GPS-0707.md` / `FASE-A-ENLAZA-0709.md`:
> confirma la vía "correr la pila stock `libmnl` bajo libhybris" con datos concretos.

## Blobs extraídos (en la Pi `~/mainline/downstream/stock-gps-0713/`, NO en repo = propietarios)
| Blob | Ruta stock | Tamaño | Rol |
|---|---|---|---|
| **libmnl.so** | `/system/lib/` | 1 786 320 | **motor de posición** (cálculo host-based del fix) |
| **mnld** | `/system/xbin/` | 72 188 | daemon: lee `/dev/stpgps`, conduce libmnl, emite NMEA |
| mtk_agpsd | `/system/bin/` | 1 439 384 | AGPS (asistencia por red: NTP + EPO) |
| gps.default.so | `/system/lib/hw/` | 38 600 | HAL Android (glue al framework — probablemente NO se porta) |
| gps.conf | `/system/etc/` | 1 180 | AGPS: NTP_SERVER=*.pool.ntp.org (assist de tiempo) |
| agps_profiles_conf2.xml | `/system/etc/` | 10 614 | perfiles AGPS/SUPL |
+ runtime bionic (para libhybris): `libc.so libstdc++.so libm.so libcutils.so libnvram.so libc++.so
liblog.so libdl.so libhardware.so` + el **linker** bionic (`/system/bin/linker`).

## Dependencias (los shims a proveer) — CLAVE de la portabilidad
- **`libmnl.so` NEEDED = solo `libc.so`, `libstdc++.so`, `libm.so`.** Nada de libcutils/libhardware/KAL
  exótico → **el motor de posición es MUY portable** (libhybris con bionic libc/libstdc++/libm, o linkado
  directo). Símbolo externo notable: `mtk_gps_sys_agps_disaptcher_callback` (lo provee mnld/agpsd).
- **`mnld` NEEDED = libc, libcutils, libm, libmnl, libnvram, libstdc++.** Añade `libcutils` (utils Android)
  y `libnvram` (lee la **calibración GPS de NVRAM**). Es el driver del stack.
- `gps.default.so` NEEDED = liblog, libcutils, libhardware, libcurl, libc++, libdl, libc, libm = capa HAL
  de Android; para Linux se sustituye por nuestro consumidor (gpsd/geoclue leyendo el NMEA de mnld).

## Arquitectura confirmada (fds de mnld en runtime, pid 158)
`mnld` tiene abiertos: **`/dev/stpgps`** (canal STP del DSP = medidas crudas 0xAAF0) + **`/dev/gps`** +
varios `socket:[...]`. Flujo: `/dev/stpgps` (DSP) → mnld+libmnl (cálculo del fix en CPU) → **NMEA por
socket** a la app/HAL (`mtk_gps_sys_nmea_output_to_app`).

## PRUEBA EN VIVO (mnld corriendo, logcat) — el stack FUNCIONA
```
mnl_linux: mtk_gps_sys_nmea_output_to_app: $GPGGA,235943.999,3736.6702,N,00058.4496,W,0,0,,100.3,M,49.7,M,,*5A
                                           $GPGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30   (A,1 = sin fix)
                                           $GPGSV,1,1,0*49                              (0 satelites = interior)
                                           $GPRMC,...,V,3736.6702,N,00058.4496,W,...     (V = void, sin fix)
                                           $PMTKTSX1,2011,83400.404,...                  (medida timing cruda del DSP)
MNLD: send_cmd_ex: slf_send_cmd (8, 0x30)                                                (protocolo de comando al DSP)
```
Emite NMEA real (posición aprox. correcta de la última asistencia); sin fix por estar en interior
(0 satélites). **Confirma que mnld+libmnl calculan y emiten** — solo falta cielo despejado para el lock.

## Vía del port (de-riskeada con estos datos)
1. Correr **mnld + libmnl bajo libhybris** en pmOS, apuntando a **`/dev/stpgps`** (ya lo tenemos en
   mainline vía el STP del CONNSYS) + `/dev/gps`. libmnl solo pide libc/libstdc++/libm → factible.
2. `libnvram` → shim mínimo o extraer la NVRAM de calibración GPS del móvil (partición nvram).
3. Consumir el NMEA que emite mnld (socket) con **gpsd/geoclue** → Phosh/apps.
4. AGPS (mtk_agpsd) = opcional (acelera el primer fix con EPO+NTP); no bloquea el fix básico.
Sustituye la vía LineageOS entera por: **binarios stock + libhybris + /dev/stpgps mainline**.

## Siguiente (cuando toque GPS)
Reproducir el enlace de Fase A (`FASE-A-ENLAZA-0709.md`) pero ahora con los binarios REALES extraídos:
cargar `mnld`+`libmnl.so` bajo libhybris en pmOS, dev nodes `/dev/stpgps`+`/dev/gps`, y ver si emite NMEA.

*Ground-truth 2026-07-13 desde LineageOS stock (adb root). GPS = CONDITIONAL-GO, ahora con los blobs en mano.*
