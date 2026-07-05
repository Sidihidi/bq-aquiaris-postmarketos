# Parches de Phosh (build propio del krillin)

Phosh 0.55 se construye **en el móvil** en `/root/build/phosh` (meson/ninja ya configurado en
`_build`). Estos parches están aplicados sobre ese árbol.

## 0001-monitor-no-dpms-off.patch
`phosh_monitor_set_power_save_mode` deja de mandar el DPMS real a phoc
(`zwlr_output_power_v1_set_mode`) y solo actualiza el estado interno + notify.

**Por qué**: en este hardware la "pantalla apagada" la implementa el daemon del backlight
(PWM deshabilitado, `/run/mt6582-screen-off`), NO el DPMS del panel. Si phosh apaga el output de
verdad, no hay forma externa de despertarlo sin romper el lockscreen (medido: `wlr-randr --on` tira
el shield; `wlopm` no puede — phosh posee el control output-power en exclusiva; el táctil inyectado
no despierta el fade). Con el parche, toda la máquina de estados de phosh (screensaver
active/lockscreen) funciona igual, pero el panel sigue renderizando → al volver la luz se ve el
lockscreen, bloqueado, como un móvil normal.

**Aplicar y reconstruir** (en el móvil):
```
cd /root/build/phosh
patch -p1 < 0001-monitor-no-dpms-off.patch    # (ya aplicado; el .bak-pre-noblank-0705 es el original)
ninja -C _build && ninja -C _build install     # ~30s incremental
pkill -x phoc                                  # launch_phosh relanza la sesion con el nuevo binario
```

**Interacción con el resto**: `mt6582-powerkey` bloquea con el método D-Bus
`org.gnome.ScreenSaver.Lock` (NO `SetActive true`, que respeta `lock-enabled` y solo hace blank)
y apaga la luz con el flag del backlight. Si algún día phosh se actualiza/reinstala sin este
parche, el síntoma será: 2º toque = pantalla iluminada en negro que no vuelve (el fade DPMS
de phosh es de sentido único desde fuera).
