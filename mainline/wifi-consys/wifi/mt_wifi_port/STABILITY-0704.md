# Punto 3 — Estabilidad del port (validado en HW #235, 2026-07-04)

## Teardown/disconnect + reconexión multi-ciclo: SIN CRASH
El bug que tumbaba al **driver A** (un scan/acceso sobre el MAC activo durante el teardown colgaba el
bus AHB/CONSYS -> WDT reset; semanas de RE con UART/pstore/netconsole) **NO ocurre en el port**: el
core stock hace su propio teardown limpio (`nicUpdateBss(DISCONNECTED)` + `REMOVE_STA_RECORD` +
`BSS_ACTIVATE deactivate`) y la reconexión en el mismo boot va a la primera.

Test: 3 ciclos `nmcli con up hola-test` (connect+DHCP) -> `nmcli con down` (teardown), vía NM/Phosh:
```
ciclo 1: UP connected IP 10.181.211.116 | DOWN wlan0 down
ciclo 2: UP connected IP 10.181.211.116 | DOWN wlan0 down
ciclo 3: UP connected IP 10.181.211.116 | DOWN wlan0 down
uptime 89090 -> 89116 monotonico = 0 crashes
```
La reconexion en el mismo boot (que el driver A NO lograba) funciona cada vez.

## Nota NM
`nmcli dev wifi connect` sobre un perfil "hola" viejo/incompleto da `key-mgmt property is missing`
(perfil roto de sesiones de test). Solución: perfil limpio
`nmcli con add type wifi con-name hola ssid hola wifi-sec.key-mgmt wpa-psk wifi-sec.psk <psk>`
o borrar los perfiles rotos. No es un problema del driver.

## Pendiente (estabilidad avanzada, no bloqueante)
- Roaming entre APs, suspend/resume, sesiones largas (horas), señal débil/reintentos.
- Confirmar TX-power/rate finos (el port hereda la RF-cal del boot; VCN33 always-on).

*El WiFi del krillin en mainline: conecta, DHCP, navega, y reconecta estable. Objetivo cumplido.*
