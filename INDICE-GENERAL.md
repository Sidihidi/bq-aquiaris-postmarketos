# BQ Aquaris E4.5 (MT6582 "krillin") · Mainline Linux + Phosh — ÍNDICE GENERAL

> **Punto de entrada para retomar el proyecto desde cualquier ordenador.**
> Repo: `Sidihidi/bq-aquiaris-postmarketos`. Última actualización: 2026-06-18.

## QUÉ ES ESTO
Port de **Linux mainline 7.0.12** (Alpine 3.24 armhf, eMMC p7) a un BQ Aquaris E4.5,
teléfono MediaTek MT6582 de 2014. Estado: **Phosh 0.55 arranca con apps y teclado**,
táctil funcional. Trabajando ahora en el **driver de display DRM (`mtk_drm`)** para
recuperar el control de **backlight** y vsync.

## ESTADO ACTUAL (resumen)
| Subsistema        | Estado |
|-------------------|--------|
| Boot mainline + Alpine en eMMC p7 | ✅ |
| Display (simplefb + hack `dispfix` rgb565) | ✅ pinta bien |
| Táctil FT5336 (ft5336_touch userspace) | ✅ |
| **Phosh 0.55 + apps + OSK (squeekboard)** | ✅ |
| Red USB (g_ether) + SSH | ✅ |
| **Backlight** | ❌ muro → requiere el driver DRM |
| DISP DRM driver (`mtk_drm`) | 🔨 **Fase 1 OK, Fase 2 compilada** (ver abajo) |
| WiFi / BT / sensores / GPU / cámara / batería | ⬜ pendientes |

## TRABAJO EN CURSO → empezar por aquí
**`mainline/disp-drm/DISP-DRM-CONTINUACION.md`** ← estado exacto, cambios de código,
cómo flashear, y el próximo paso (flashear `boot-disp2.img` y leer dmesg, luego Fase 3 = DSI+panel).
Código modificado en `mainline/disp-drm/code/` (mtk-mmsys.c, mtk_disp_rdma.c, el DT).

## MAPA DEL REPO
- `mainline/disp-drm/` — **driver DISP DRM** (lo de ahora): CONTINUACION + ROADMAP + componentes + code/
- `mainline/phosh/` — hito Phosh arranca: schema GNOME, launcher, lista de apks, `usrlocal.tar.gz`
- `mainline/quickwins/` — OSK (squeekboard .service + wrapper), DTS, notas de las "quick wins"
- `mainline/estrategia/` — roadmap de los 9 drivers pendientes (prioridades)
- `m2b/` (si existe) — red USB musb, rootfs Alpine, recetas de flasheo

## ACCESO / ENTORNO
- **Pi de build (donde está TODO el trabajo real)**: `ssh cpcd@192.168.0.38` (pass `cpcdupct`, sudo NOPASSWD).
  Kernel: `~/mainline/linux-7.0.12` (build dir `build-krillin`). boot.img/backups: `~/mainline/pkg/`.
  La IP oscila entre `.38` y `.123` (mirar el router si falla).
- **Teléfono desde la Pi**: `ssh root@172.16.42.1` (Pi en usb0 = 172.16.42.2/24).
- **Recuperar boot bueno**: flashear `~/mainline/pkg/boot-simpledrm.img` (#25, simplefb+dispfix).
- *(Los scripts de automatización son `.py` con paramiko en el PC de origen; desde otro PC se pueden
  recrear o usar `ssh`/`scp` directo a la Pi — el trabajo vive en la Pi, no en el PC.)*

## ⚠️ REGLAS DE SEGURIDAD (CRÍTICO — no romper el teléfono)
1. Softbrick → SP Flash Tool **"Download Only"**, **NUNCA "Format All"** (borra IMEI).
2. **NUNCA** restaurar el LK de **Lollipop** (verifica firma → bootloop). El bueno es **KitKat 1.5.2** (`~/stock-1.5.2/lk.bin`).
3. `mtkclient`: **`wo <off> <len> <img>`**, **NUNCA `wf`** (machaca el MBR/sector 0).
4. Fastboot: **`fastboot flash boot`**, **NUNCA `fastboot boot`** (el LK no lo soporta → carga el de eMMC).
5. Entrar en fastboot = manual: apagar (Power ~10s) → **Power + Vol↑** (pantalla negra, normal).

## CONTEXTO ACUMULADO
La memoria persistente del asistente (en el PC de origen, `~/.claude/.../memory/`) tiene el detalle
fino de cada subsistema (`reference_mt6582_*.md`). No se sincroniza entre PCs, por eso este repo
documenta lo necesario para continuar. Si retomas con el asistente en otro PC, dile que **lea este
índice y `mainline/disp-drm/DISP-DRM-CONTINUACION.md`** para ponerse al día.
