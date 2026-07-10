# Maemo Leste: saga de la pantalla negra — diagnóstico completo (2026-07-10)

> Estado al cierre de la sesión: **cadena de 5 fixes apilados** en la p3; el último (no-rotación)
> queda **staged pendiente de probar**. La tesis: **Maemo funciona entero pero era invisible** —
> la sesión corre y el primer modeset (DPMS o rotación) mata el panel para siempre.

## La limitación de fondo (kernel): el DSI NO sobrevive a un modeset completo
Evidencia (console-ramoops del boot de Maemo):
```
WARNING: drm_crtc_wait_one_vblank ... [drm] vblank wait timed out on crtc 0
  mtk_crtc_atomic_disable <- drm_atomic_connector_commit_dpms   (DPMS off de Xorg)
[drm] polling dsi wait not busy timeout!   (x3)
WARNING: drm_atomic_helper_wait_for_vblanks ... vblank wait timed out
  drm_mode_setcrtc   (X intentando re-encender)
```
- Cualquier **disable→enable del CRTC** (DPMS off, rotación RandR, cambio de modo) cuelga el DSI
  (`polling dsi wait not busy timeout`) y el panel no vuelve.
- Causa probable: **el init del panel (hx8389, video mode) solo lo hace el LK** — el driver mainline
  hereda el panel encendido y no sabe reinicializarlo tras un stop del DSI.
- **Afecta también a pmOS**: suspend/resume y apagado de pantalla harán lo mismo en Phosh.
  → TAREA KERNEL pendiente: portar la secuencia de init del LCM del downstream al driver DSI/panel.
- Los "plane flips" normales (X arrancando con el mismo modo que fbcon) NO disparan el bug.

## Cadena de fixes en la p3 (orden cronológico del debug)
| # | Bug | Evidencia | Fix | Estado |
|---|---|---|---|---|
| 1 | Sesión X abortaba (`no session managers... aborting`) | `.xsession-errors` | `/etc/X11/Xsession.d/49maemo-forcestartup` (STARTUP=Xsession-post; 50determine-startup corre antes que 98) | ✅ probado |
| 2 | hildon-desktop dpkg half-configured | dpkg status | chroot `dpkg --configure -a` (+policy-rc.d exit 101) | ✅ |
| 3 | Clutter muere: `xcb_dri3_pixmap_from_buffer failed / BadAlloc` | hd-desktop.log | **`DefaultDepth 24`** en xorg.conf (X iba a 16bpp/RGB565 vs DRM XR24 32bpp; glamor Mali400 estaba OK) | ✅ probado (log: `Depth 24, bpp 32`) |
| 4 | DPMS off de Xorg → DSI muerto | ramoops (arriba) | xorg.conf: `Option "DPMS" "false"` + Blank/Standby/Suspend/OffTime 0 + elogind `HandlePowerKey=ignore` | staged |
| 5 | **hildon-desktop ROTA la pantalla a 960x540** (paisaje) → modeset → DSI muerto | Xorg.log rootless: `Allocate new frame buffer 960x540` y muere ahí | gconf `/apps/osso/hildon-desktop/ui_can_rotate=false` (`gconftool-2 --direct` en chroot) + activar `/etc/hildon-desktop.env` (=.leste: `COGL_RENDERER=egl_xlib`, `COGL_DRIVER=gles2`) | **staged, SIN probar** |

## Trampas de forensics aprendidas (importante para iterar)
- **El X de Maemo corre rootless** → su log real es `~user/.local/share/xorg/Xorg.0.log`,
  NO `/var/log/Xorg.0.log` (ese es un fósil de cuando corría como root; mtime 1970).
- El stderr de hildon-desktop bajo dsmetool es invisible → wrapper `hd-launch.sh` que redirige a
  `/var/log/hd-desktop.log` (instalado; a veces queda vacío: maemo-invoker/launcher se lleva el output).
- `21hildon-desktop-wait` bloquea la sesión eternamente esperando `desktop-started.stamp`.
- rsyslog NO corre en Maemo → syslog vacío; el RTC arranca en 2073 → mtimes/timestamps engañosos.
- Power-cycle abrupto pierde escrituras recientes (ext4) → logs del último instante pueden faltar.
  → instalar `kmsg-dump` (init.d en rcS.d: `dmesg --follow >> /var/log/kmsg-live.log` + sync 5s).

## Cómo iterar (ciclo)
1. Arrancar Maemo (menú → entrada 2). Observar pantalla.
2. Volver a pmOS, montar `mmcblk1p3` ro y leer: `home/user/.local/share/xorg/Xorg.0.log`,
   `var/log/hd-desktop.log`, `var/log/kmsg-live.log`, ramoops (`/sys/fs/pstore/console-ramoops-0`
   — ¡solo guarda el ÚLTIMO boot, leerlo antes de otro reboot!).

## Ronda 4 (0710 tarde): la rotación inicial es INCONDICIONAL — el bloqueo es de KERNEL
- Con `ui_can_rotate=false` + env COGL activado, el Xorg rootless muestra otra vez
  `Allocate new frame buffer 960x540` a los ~50 s → **hildon-desktop hace el XRRSetCrtcConfig
  a paisaje SIEMPRE al arrancar** (la clave gconf solo gobierna la rotación dinámica de UI).
- `hd-desktop.log` salió con 35 KB de espacios en blanco (basura de terminal) — sin valor.
- La sesión corre entera (mis-xrecord/mis-sound de hildon en `.xsession-errors`). **Maemo está
  VIVO y a ciegas**: el único bloqueador real es que el modeset de la rotación mata el panel.
- `kmsg-dump` INSTALADO (`/etc/init.d/kmsg-dump` + `rcS.d/S03`): el próximo boot de Maemo dejará
  el kernel log completo y persistente en `/var/log/kmsg-live.log` (sync cada 5 s; el anterior
  rota a `.old`). Ahí se verán los WARN del DSI sin depender del ramoops.

## PLAN PARA CASA (por prioridad)
1. **KERNEL (fix de fondo, sirve a Maemo Y pmOS)**: hacer que el pipeline DRM sobreviva a un
   modeset completo. Portar la secuencia de init del LCM `hx8389_qhd_dsi_vdo_truly` del downstream
   al driver de panel mainline + revisar `mtk_dsi` stop/start (el `polling dsi wait not busy
   timeout` tras `mtk_crtc_atomic_disable`). Desbloquea: rotación, DPMS, apagado de pantalla y
   suspend/resume en ambos SO. Verificación rápida sin Maemo: desde pmOS,
   `xrandr --rotate left` (o modetest con ciclo off/on) debe sobrevivir.
2. **Userspace (workaround si 1 se atasca)**: parchear el hildon-desktop de Leste (es open
   source, paquete `hildon-desktop`) para saltarse el `XRRSetCrtcConfig` inicial, o shim
   LD_PRELOAD que haga no-op `XRRSetCrtcConfig`/`XRRSetScreenSize` (cross-compilar armhf en la
   Pi). Con el panel vertical y sin rotación se vería el escritorio (quizá con layout raro).
3. Tras ver Hildon: quitar el env de DEBUG de `20hildon-desktop` (CLUTTER_DEBUG/EGL_LOG_LEVEL/
   LIBGL_DEBUG) y el wrapper si molesta; revisar `21hildon-desktop-wait` (stamp) y el táctil.

*Sesión Mac (Fable 5), 2026-07-10.*
