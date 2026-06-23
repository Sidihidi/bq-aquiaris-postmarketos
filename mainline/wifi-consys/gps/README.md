# GPS bridge (CONSYS MT6582 → NMEA)

Puente de userspace que reemplaza a `mnld` para el GPS del CONSYS. El kernel
(`../m3a/mt6582-btif.c`) ya hace `func_on(GPS)` al abrir `/dev/stpgps` (canal
STP 2); este daemon escribe la secuencia de arranque y bombea el NMEA por un PTY
que consume gpsd:
`/dev/stpgps ↔ /dev/gps0 (pty) → gpsd → geoclue/gps-share → Phosh`.

**Estado: ⚠️ decodificado, falta el START_SEQ.** El protocolo `0xAAF0` está
decodificado y la cadena gpsd→geoclue→Phosh validada, pero `START_SEQ[]` en el
bridge está **vacío** (TODO): hay que capturar de `mnld` en Android los `write()`
de arranque que despiertan al GPS para que escupa NMEA y gpsd dé fix. Depende de
que `../m3a/` haya hecho el bring-up del CONSYS (por eso el start corre *tras*
`zz-consys-bt.start`, sin carrera).

## Ficheros

- **`mtk-gps-bridge.c`** — el bridge (C, musl/Alpine OK): abre `/dev/stpgps`,
  envía `START_SEQ`, bombea bytes en ambos sentidos a un pty `/dev/gps0`.
  Compilar: `gcc -O2 -Wall -o mtk-gps-bridge mtk-gps-bridge.c`. **Aquí va el
  START_SEQ** una vez capturado.
- **`zzz-gps.start`** — servicio OpenRC (`/etc/local.d/`): lanza el bridge + gpsd
  + el socket de gps-share para geoclue. Corre tras `zz-consys-bt.start`.

## Documentación detallada

- **`GPS-CONSYS.md`** — análisis del protocolo `0xAAF0`, la cadena
  gpsd→geoclue→Phosh y el procedimiento de captura del START_SEQ (Stage 2).
