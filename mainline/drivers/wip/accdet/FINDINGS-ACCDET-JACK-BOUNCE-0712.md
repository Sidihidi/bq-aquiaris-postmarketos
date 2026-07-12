# Accdet — el jack NO se mantiene detectado (bounce de settle) — 0712

> Intentando validar los **botones inline** (play/pause, vol±) en HW descubrí que el jack **no se
> mantiene insertado**: se detecta y se suelta ~20ms después, en bucle. Por eso no llega ningún
> botón (el input nunca queda en estado "insertado") y el auto-routing a auriculares tampoco iría.

## Evidencia (evtest sobre `/dev/input/event2` = mt6323-accdet)
Device correcto y con las capacidades bien declaradas: `KEY_PLAYPAUSE (164)`, `KEY_VOLUMEUP (115)`,
`KEY_VOLUMEDOWN (114)`, `SW_HEADPHONE_INSERT (2)`, `SW_MICROPHONE_INSERT (4)`. micbias 2.8V lista.

Al enchufar (firme, dejándolo puesto), dmesg repite:
```
mt6323-accdet: jack: headphone (AB=3)
mt6323-accdet: jack: removed        <- ~20ms despues
mt6323-accdet: jack: headphone (AB=3)
mt6323-accdet: jack: removed        <- ~20ms despues (se repite)
```
Y evtest solo vio `SW_HEADPHONE_INSERT value 1` seguido inmediato de `value 0`. **0 pulsaciones**
de botones (los conteos de KEY_* que parecían >0 eran las líneas de *capacidades* de evtest, no eventos).

## Diagnóstico
- El **timing constante de ~20ms** entre insert→removed apunta a un **settle/debounce del driver**, no
  a rebote mecánico. El accdet detecta la inserción, tras un settle re-lee el comparador (AB) y decide
  "removed". La lectura del AB durante el settle es inestable (ya se sospechaba en el handoff Mac:
  "AB=0 puede ser settle 250ms").
- **AB=3** ahora vs **AB=0** en la validación previa (Mac, 5 ciclos OK) = con estos auriculares el
  comparador cae en un umbral distinto/marginal. La detección no es robusta al tipo de jack.
- `SW_MICROPHONE_INSERT` quedó en 0 → estos auriculares parecen de 3 polos (sin micro/botones); aun
  así el problema PRIMARIO es que ni el headphone plano se mantiene.

## Fix probable (ya scopeado por la sesión paralela, `VERIFICACION-DRIVERS-PARALELA-0709.md`)
Los fixes pendientes de accdet encajan justo con esto: **RSV=0x5A20 (switch del auxadc)**, **PWM boost**,
**IDLE_EN=0x70**. La estabilidad de la lectura del comparador durante el settle depende de esos
registros. Aplicarlos + re-tunear el tiempo de debounce debería mantener el jack. Es **cambio de driver
in-kernel** (`mt6323-accdet.c`) → build+flash, no un quick-win.

## Estado
- Jack detección: **INESTABLE** (detecta pero no mantiene; bounce de ~20ms). Refuta el "accdet OK".
- Botones inline: **NO validables** hasta que el jack se mantenga. Wiring del driver correcto (caps OK).
- Necesita: aplicar los fixes de registro scopeados + re-tunear debounce, luego re-test con un headset
  de 4 polos (con micro/botón).

*Sesión principal (Fable 5), 2026-07-12.*
