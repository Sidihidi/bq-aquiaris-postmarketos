# Estudio de viabilidad — Módem 2G/3G del krillin (MT6582) en mainline

> Sesión Mac (Opus 4.8), 2026-07-07. Entregable: **go / no-go** para intentar llamadas/SMS/datos.
> **VEREDICTO: NO-GO como siguiente paso.** Es un moonshot de investigación de varios meses y baja
> probabilidad, no una consolidación. Detalle abajo.

## 1. Qué es el módem del MT6582 (arquitectura)
El MT6582 lleva un **módem integrado en un core SEPARADO** ("MD", modem processor), distinto del AP (los
4× Cortex-A7 donde corre Linux). No es un chip con interfaz AT por UART: es un co-procesador en el mismo SoC.
- **Firmware**: el MD ejecuta la baseband propietaria de MediaTek, **MOLY** (confirmado: el repo
  `Pablito2020/mediatek-modem` es el *source de MOLY* — su README es literalmente
  `CELLTEL82_WET_KK_GPRS_HSPA_MOLY.WR8.W1315.MD.WG.MP.V35`). Se carga desde la partición del módem.
- **Interfaz AP↔MD = CCCI** (Cross-Core Communication Interface, luego "eccci"): memoria compartida +
  mailboxes CCIF + interrupciones. El driver CCCI gestiona el arranque del MD, la carga del firmware, y
  multiplexa los canales lógicos (control, datos, IPC del RIL).
- **Userspace (Android)**: un **RIL propietario de MTK** (mtk-ril) habla con los chardevs del CCCI.
  NO es AT ni QMI ni MBIM estándar.

## 2. Qué haría falta para una llamada (la pila entera, desde cero)
1. **Portar el driver `eccci`/`dual_ccci/mt6582` a mainline 7.0.12** (hoy solo existe en Android 3.4/3.10):
   CCIF + memoria compartida + arranque del MD + carga del firmware MOLY + los canales lógicos. Es un port
   MÁS grande que el del WiFi, y el protocolo de memoria compartida está **sin documentar**.
2. **Que el MD arranque y registre** (el handshake CCCI) — hito intermedio, ya de por sí un proyecto.
3. **Exponer los canales RIL** al userspace.
4. **Puente RIL-MTK → ModemManager/oFono**: MTK no usa AT/QMI/MBIM, así que **no hay plugin en
   mainline** que lo entienda. Habría que escribir un traductor del RIL propietario, o portar mtk-ril y
   un shim. Este paso es **tan difícil como el driver** y no tiene framework destino limpio (a diferencia
   del WiFi, que tenía cfg80211).

## 3. Estado en mainline (lo decisivo)
- **CERO soporte en mainline** para esta clase de módem MTK integrado. Todo el código CCCI/eccci del
  MT6582 vive **solo en árboles Android downstream** (`drivers/misc/mediatek/{eccci,dual_ccci/mt6582}`) o
  en **investigación de RE/seguridad** (Comsecuris `mtk-baseband-sanctuary`, cyrozap
  `mediatek-lte-baseband-re`, el MOLY de Pablito).
- El **único** driver WWAN de MTK en mainline es **`t7xx`**, y es para módems **5G PCIe (T700)** —
  arquitectura completamente distinta, **no aplica** al módem shared-memory del MT6582.

## 4. Precedente (la señal más fuerte)
- **Ningún dispositivo MT65xx** con módem integrado tiene **llamadas/SMS funcionando en un kernel
  mainline** (pmOS ni otros). El módem es el pedazo NO resuelto de todos los ports mainline de MTK de esta
  era.
- Los esfuerzos que existen son research-grade y **nunca llegaron a llamar**: el MOLY de Pablito es
  source de firmware (no un driver funcional); la ruta OsmocomBB (baseband abierta) en MTK Fernvale
  (blog pmOS 2018) llegó a **"parpadear un LED"** y dejó las llamadas 2G como "trabajo futuro aspiracional".

## 5. Esfuerzo / probabilidad
- **Esfuerzo: MUY ALTO** (meses). Son ≥3 sub-proyectos duros encadenados (port eccci → boot MD+firmware →
  puente RIL↔ModemManager), cada uno contra protocolos propietarios sin documentar.
- **Probabilidad: BAJA.** Sin precedente de éxito en esta clase de SoC en mainline; múltiples incógnitas
  independientes; el paso RIL↔ModemManager no tiene destino framework limpio.
- Comparación: el port del WiFi —el hito más difícil del proyecto, semanas de equipo— tenía a favor un
  framework destino (cfg80211) y un core stock portable. El módem es más grande **y** sin ese apoyo.

## 6. VEREDICTO: NO-GO (por ahora)
No es una consolidación: es un proyecto de investigación de meses con final incierto, sobre un móvil que
**ya funciona para todo lo demás** (WiFi, audio, suspend, GUI, vídeo…). El coste/beneficio no sale.

### Si algún día se intenta (la ruta menos mala)
Replicar la estrategia del WiFi: portar `dual_ccci/mt6582` sobre una capa baja propia, con el **hito M1 =
el MD arranca + completa el handshake CCCI + responde** (sin RIL todavía). Si M1 se logra tras varias
semanas, RECIÉN entonces evaluar el infierno del RIL↔ModemManager. Puerta de decisión: si el MD no
arranca limpio en 3-4 semanas, abortar. **No entrar sin asumir explícitamente que es un moonshot.**

## 7. Recomendación de qué hacer en su lugar
Con la energía restante, más valor por esfuerzo:
- **Cerrar GPS** (casa ya lo diagnosticó: falta el `START_BURST` exacto de mnld) — feature real, tractable.
- **Pulido de sensores**: auto-brillo (ALS→backlight), proximidad en el proxy.
- Declarar las *phone-features* del proyecto "completas" y documentar el módem como límite conocido
  (como cámara): un BQ de 2014 con WiFi+audio+suspend+GUI+vídeo en mainline ya es un resultado enorme.

## Fuentes
- CCCI/eccci solo downstream: android.googlesource.com `.../dual_ccci/mt6582/`, `.../eccci/`
- MOLY firmware source: github.com/Pablito2020/mediatek-modem (README = versión MOLY MD)
- RE baseband: github.com/Comsecuris/mtk-baseband-sanctuary, github.com/cyrozap/mediatek-lte-baseband-re
- Único WWAN MTK mainline (no aplica): docs.kernel.org/networking/device_drivers/wwan/t7xx.html
- OsmocomBB en MTK (LED blink): postmarketos.org/blog/2018/04/14/lowlevel/
