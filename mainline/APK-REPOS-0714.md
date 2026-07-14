# Repos apk del krillin + repo pmOS (0714)

## Qué es este sistema (importante)
El rootfs del krillin **NO es postmarketOS** — es **Alpine Linux v3.24 puro + Phosh** (arch **`armhf`**).
No hay paquetes `postmarketos-*` ni claves de pmOS de fábrica; solo los repos de Alpine:
```
http://dl-cdn.alpinelinux.org/alpine/v3.24/main
http://dl-cdn.alpinelinux.org/alpine/v3.24/community
```

## Repo pmOS añadido (etiquetado @pmos)
Para poder instalar paquetes de postmarketOS se añadió su repo, **etiquetado `@pmos`** (así NO afecta al
`apk` normal — solo se usa con `apk add pkg@pmos`), + su clave de firma:
```
# /etc/apk/repositories
@pmos https://mirror.postmarketos.org/postmarketos/master
```
- **Clave**: `build.postmarketos.org.rsa.pub` en `/etc/apk/keys/` (sacada del paquete `postmarketos-keys`).
- **Por qué `master` (edge) y no una release estable**: pmOS **dejó de construir `armhf` en las releases
  estables** (v26.06 etc. solo traen armv7/aarch64). El ÚNICO repo pmOS con armhf es `master` (edge). ⚠️ Es
  edge sobre una base estable v3.24 → **no hacer `apk upgrade`** (mezclaría edge en el sistema). El `@pmos`
  lo hace opt-in por paquete, y `apk add --simulate pkg@pmos` muestra el impacto antes.

## ⚠️ Hallazgo: armhf no tiene apps GUI modernas
**Las apps de mapas/GUI (gnome-maps, pure-maps, foxtrotgps) NO existen para armhf** — Alpine las descartó de
community, y el repo pmOS armhf (master) tiene solo **260 paquetes de INFRAESTRUCTURA** (mutter-mobile,
libcamera, device tooling...), **no apps**. armhf es arquitectura legacy que los repos de apps han abandonado.
Para GUI de mapas habría que: (a) rebuild del rootfs en arch **`armv7`** (entonces las releases pmOS estables
sí lo traen), o (b) compilar desde aports. **Para PROBAR el GPS no hace falta**: `xgps` (satélites, gráfico),
`Where am I?` (geoclue), `cgps` (terminal) ya funcionan — ver `drivers/wip/gps/INTEGRACION-GPS-0714.md`.

## Paquetes pmOS ÚTILES para el proyecto (no apps)
El repo @pmos sí da cosas útiles para el desarrollo del krillin:
- **`akms`** — Alpine Kernel Module System: build/instalación de módulos out-of-tree (gpsdrv, batería, etc.)
  de forma persistente y reproducible (en vez de insmod a mano).
- **`libcamera` / `libcamera-tools`** — para el trabajo de cámara (OV8865).
- **`usb-moded`, `mutter-mobile`** — infra móvil.
Instalar con `apk add akms@pmos` (revisar antes con `--simulate` que no arrastre libs de edge).

## Revertir (si se quiere sistema 100% Alpine estable)
Quitar la línea `@pmos ...` de `/etc/apk/repositories` (backup en `/etc/apk/repositories.pre-pmos`) +
`rm /etc/apk/keys/build.postmarketos.org.rsa.pub` + `apk update`.

*Casa (Opus 4.8), 2026-07-14.*
