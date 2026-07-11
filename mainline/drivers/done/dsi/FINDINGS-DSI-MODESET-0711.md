# DSI/LCM — raíz del "negro tras modeset" (Maemo hildon / rotación / DPMS) — 0711

> ## ✅✅ RESUELTO EN HW (0711, #283 / boot-menupick13.img, fcfedf35) — la ROTACIÓN SOBREVIVE
> Test decisivo por SSH en pmOS (sin depender de Maemo): `wlr-randr --output DSI-1 --transform 90`
> → **dmesg LIMPIO** (cero `commit wait timed out`, cero `reset engine`) + `grim` capturó la
> pantalla **girada a horizontal con contenido visible** (reloj 17:52), y volvió a vertical OK.
> Boot pmOS con GUI = sin regresión (grim: lockscreen phosh). Maemo pendiente de confirmar por el
> usuario, pero era el MISMO modeset → esperado OK.
>
> **EL FIX (2 piezas, la del panel es la esencial):**
> 1. **Panel `panel-himax-hx8389.c`** (backup `.GANADOR-0711`, md5 1652efdf): bring-up del panel
>    UNA sola vez (primer `prepare`+`enable` con power+reset+DCS init) + flag `bool up`; luego
>    `disable`/`unprepare`/re-`prepare`/re-`enable` = **no-ops** → cero DCS re-enviados en modesets
>    → cero `mtk_dsi_reset_engine()` a mitad del atomic commit → el commit completa → panel vivo.
> 2. **`mtk_dsi.c`**: da IGUAL la variante — el freeze del panel **neutraliza** hasta el hack
>    `mode_changed` de Mac (su re-init llama `panel.enable`, que ahora es no-op → no re-manda DCS).
>    #283 se compiló con el mtk_dsi de Mac (6ec2d51d) y funcionó igual. → **el arreglo del panel
>    es independiente del mtk_dsi**; robusto.
>
> ⚠️ CARRERA: la sesión Mac SIGUE escribiendo `mtk_dsi.c` (cambió bec2560d→6ec2d51d durante mi
> build). Como el fix vive en el panel, no bloquea; pero para builds deterministas: **panel
> `.GANADOR-0711` es la fuente a preservar**. Imagen buena nueva = `boot-menupick13-DSIFIX-GANADOR.img`.
>
> ## MAEMO tras el fix (validado en HW por el usuario, 0711)
> El "Maemo no bootea" (logs kernel → negro) ERA el modeset del DSI. Con #283: dmesg de Maemo
> **LIMPIO** (cero commit-timeout), Xorg completa el modeset (`modeset(0): Output DSI-1 using
> initial mode 540x960`, DRI2 mediatek, sin (EE)), panel en HORIZONTAL con render (el usuario vio
> un cuadrado rosa = render parcial de Clutter). **PERO** la pantalla sigue negra porque
> **hildon-desktop NO pinta el escritorio** (capa distinta, NO el DSI):
> - hildon-desktop (pid vivo, 0% CPU = bloqueado, no crash) carga `libclutter-eglx-0.8` +
>   `libEGL_mesa`/`libGLESv2`; lima (mali400) OK.
> - `.xsession-errors`: `xrec_data_cb(): xrd == NULL` + `sound_init(): canberra Not available`.
> - Coincide con `HANDOFF-DRIVERS-BOOT6-0710.md` (sesión Mac): hildon inicializa xrecord/sound y
>   no completa el arranque del stage de Clutter → negro. **Territorio Mac (Clutter/GL), no DSI.**
> - Follow-up separado: depurar el render de Clutter 0.8 eglx sobre Mesa/lima en hildon-desktop.

> Sesión principal (casa). Diagnóstico sólido tras 3 builds fallidos. Restauré el panel a la base
> `panel-himax-hx8389.c.bak-pre-dsi-fix` y luego apliqué el fix ganador (arriba).

## ⚠️ COLISIÓN DE SESIONES detectada
La sesión Mac y la principal editaban el **mismo árbol DSI** a la vez:
- `mtk_dsi.c` (18:16) lleva parches Mac que **añaden `mtk_dsi_reset_engine()` y `mtk_dsi_stop()`**
  (diff vs `.bak-pre-reinit`). Backups: `.bak-pre-reinit` (07-10 22:52), `.bak-stop` (18:16).
- `boot-menupick-dsifix.img` es **blanco móvil**: era `d9a1346b` (validada por el usuario con GUI),
  la Mac la reconstruyó a `e48413db` (18:21) sin avisar.
- Mis 3 builds (#280/#281/#282) del panel se hicieron **encima del mtk_dsi de Mac en curso** → regresión.
- **Imagen BUENA confirmada por el usuario = `d9a1346b`** (guardada en el móvil `/root/bgood.img`).
  Restaurada al eMMC (sector 83968). Es la de daily; pmOS con GUI OK, Maemo negro tras rotación.

## Reproducción exacta (kernel #279, imagen dsifix d9a1346b, entrada Maemo)
Boot OK (se ven los logs del kernel = panel muestra). hildon-desktop arranca y hace su **rotación
inicial incondicional** (XRRSetCrtcConfig 960x540 shadow-fb) → un **modeset**. dmesg:
```
[ 51-55s]  [drm] polling dsi wait not busy timeout!   x3   (DCS del panel enable)
[   376s]  [CONNECTOR:34:DSI-1] commit wait timed out         (el modeset de hildon)
[390-394s] [drm] polling dsi wait not busy timeout!   x3
```
Backlight queda a 8/10 (por eso "negro con retro").

## RAÍZ (confirmada por comparación de builds)
1. Este IP DSI del mt6582 **no transfiere DCS de forma fiable desde el kernel**. En `.enable`
   (modo vídeo) los DCS del panel dan `polling dsi wait not busy timeout` (blando: los datos
   salen igual, el panel se ve). En LP/`.prepare` dan `failed to switch cmd mode -62` (duro).
2. `mtk_dsi_wait_for_idle()` y `mtk_dsi_wait_for_irq_done()`, al agotar el timeout, **llaman a
   `mtk_dsi_reset_engine(dsi)`**. Durante un modeset, los DCS del panel (`hx8389_enable`/
   `hx8389_disable`) corren DENTRO del atomic commit → el reset del engine rompe el vídeo →
   no llega frame-done → **`commit wait timed out`** → pipeline apagado → **negro**.
3. En el boot funciona porque el primer commit tolera el reset (no hay dependencia de flip-done
   previa); en el modeset de hildon, no.

## Qué probé (y por qué falló cada uno) — para no repetir
- **#280**: init DCS movido a `.prepare` + `prepare_prev_first` + `return accum_err`. → `-62`
  (DCS en LP fallan duro) y además el `return` del error **aborta todo el display** (ni menú). Negro.
- **#281**: quitar TODOS los DCS (bring-up solo power+reset). → Negro en boot mismo: el panel
  **crudo tras reset NO muestra vídeo**; necesita al menos `exit_sleep`+`display_on`.
- **#282** (idea correcta pero base equivocada): bring-up UNA vez (power+reset+DCS init) y luego
  `disable/unprepare/re-prepare/re-enable` = no-ops, para que el modeset no re-mande DCS. → `-62`
  en el PRIMER enable: partí de `.bak-pre-dsi-fix` sobre el mtk_dsi **con** reinit/stop de Mac →
  el DCS de boot que en `d9a1346b` era blando aquí es duro. (Sospecha: el `reset_engine`/`stop`
  añadidos o la falta de `prepare_prev_first` cambian el estado a modo-vídeo antes del DCS.)

## Camino recomendado (a coordinar — el DSI lo lleva Mac)
La idea de **#282 es la correcta** y hay que hacerla sobre el mtk_dsi definitivo:
- **Bring-up del panel UNA vez** (primer prepare+enable con los DCS que sí funcionan al boot) y
  **congelar** el estado: `disable`/`unprepare`/re-`prepare`/re-`enable` = no-ops → cero DCS en
  modesets → cero `reset_engine` a mitad de commit → el commit completa → sobrevive rotación/DPMS.
- Complemento en `mtk_dsi.c`: **no llamar `mtk_dsi_reset_engine()` dentro del path de commit**
  (o bajar el timeout de `wait_for_idle` de 2 s a ~20 ms para que un DCS fallido no coma el
  presupuesto de 15 s del commit). Esto lo debe decidir quien lleva mtk_dsi (Mac).
- Alternativa userspace (sin kernel, si urge Maemo): impedir la rotación inicial de hildon
  (config de hildon-desktop / transformación fija) → se salta el modeset asesino.

## REGLA reforzada
Ficheros DSI del árbol compartido (`mtk_dsi.c`, `panel-himax-hx8389.c`) = **un solo dueño a la vez**.
Marcar cambios staged con `/* STAGED <sesión> <fecha> */` y anotarlos aquí. La imagen buena de
daily es `d9a1346b` (en `/root/bgood.img` del móvil y como backup).

*Sesión principal, 2026-07-11.*
