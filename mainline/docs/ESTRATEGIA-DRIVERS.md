I now have full alignment with the actual repo state. I have everything needed to produce the strategy.

# Estrategia de implementación — BQ Aquaris E4.5 "krillin" (MT6582) + Phosh en mainline 7.0.12

> **Premisa real (verificada hoy):** Phosh 0.55 **ya arranca** sobre `simpledrm`/`card0`, con táctil (daemon), display RGB565, eMMC, USB+SSH, I2C, EINT, keypad-Vol↓ y PMIC/batería/carga por userspace. El bloqueo de schema (`org.gnome.shell.keybindings`) ya está resuelto. El renderer **pixman (software) va fluido**. Por tanto la estrategia NO es "hacer arrancar Phosh", sino **convertir un Phosh que arranca en un teléfono usable**: backlight, batería en la barra, botón de encendido, sensores, suspend. Todo lo demás es secundario.

---

## 1. Tabla-resumen de subsistemas

| # | Subsistema | Tipo | Esfuerzo | Viabilidad | Impacto Phosh | Veredicto |
|---|---|---|---|---|---|---|
| 1 | **backlight** (DISP_PWM/BLS) | portar | bajo | alta | **alto** | ⭐ Quick win #1 |
| 2 | **power-button** (MT6323 PWRKEY→EINT25) | adaptar | bajo | alta | **alto** | ⭐ Quick win #2 |
| 3 | **sensors-rotation** (accel/ALS IIO) | adaptar | bajo | alta | **alto** | ⭐ Quick win #3 |
| 4 | **battery-upower** (módulo power_supply puente) | crear-desde-0 | medio | alta | **alto** | Núcleo Fase 1 |
| 5 | **pinctrl-mt6582** (pinmux+pull, habilitador) | adaptar | alto | alta | medio | Habilitador transversal |
| 6 | **suspend-pm** (s2idle + wakeup EINT) | adaptar | medio | media | alto | Fase 1.5 (tras power-btn) |
| 7 | **wifi** (dongle USB-OTG) | adaptar | medio | alta | **crítico** | Fase 2 conectividad |
| 8 | **apdma-i2c** (lecturas >8B) | arreglar | bajo | media | bajo | Mejora calidad (no bloquea) |
| 9 | **audio** (AFE+codec desde 0) | portar | muy-alto | media | bajo | Fase 3, último |

**Lectura de la tabla:** los tres de impacto alto **y** esfuerzo bajo (1, 2, 3) son la prioridad absoluta. El #4 (batería visible) es impacto alto pero esfuerzo medio — entra justo después. El #5 (pinctrl) es el único de esfuerzo alto que aún así conviene hacer pronto **porque desbloquea a varios**. El #9 (audio) es el peor ratio coste/beneficio del roadmap.

---

## 2. Grafo de dependencias — qué desbloquea qué

```
                         ┌─────────────────────────────────────────────┐
   YA RESUELTO (base) ──▶│ pwrap+MFD MT6323 │ EINT(&eint,GIC113) │ I2C0 │ simpledrm/card0 │ FAN5405 daemon │
                         └──────┬──────────────────┬───────────┬─────────────────┬──────────────────────────┘
                                │                  │           │                 │
            ┌───────────────────┘      ┌───────────┘    ┌──────┴──────┐    ┌─────┴───────────┐
            ▼                          ▼                ▼             ▼    ▼                 ▼
   ┌──────────────────┐      ┌──────────────────┐  ┌─────────┐  ┌──────────────┐   ┌──────────────────┐
   │ #4 battery-upower│      │ #2 power-button  │  │ #3 accel│  │ #3 ALS (tsl) │   │ #1 backlight     │
   │ (power_supply +  │      │ (mt6397-irq sobre│  │ /gyro/  │  │ in_illumin.  │   │ (pwm-mtk-disp    │
   │  daemon VBAT)    │      │  EINT25, revert  │  │ mag IIO │  │              │   │  mt2701 + BLS)   │
   └────────┬─────────┘      │  patch 0004)     │  └────┬────┘  └──────┬───────┘   └────────┬─────────┘
            │                └────────┬─────────┘       │              │                    │
            │                         │                 │              │                    │
            ▼                         ▼                 ▼              ▼                    ▼
        UPower ──▶ Phosh:        KEY_POWER ──▶ Phosh:  iio-sensor-proxy ──▶ Phosh:      /sys/class/backlight
        icono %, "cargando",     bloquear/desbloquear   autorrotación      brillo-auto   ──▶ slider de brillo
        avisos batería baja      + menú apagar          (accel)            (ALS escribe
                                                                            backlight)
            │                         │
            │                         ▼
            │              ┌────────────────────────┐
            └─────────────▶│ #6 suspend-pm (s2idle) │   ← NECESITA wakeup; el power-button (EINT) ES el wakeup source
                           │  wakeup = EINT power-key│      (la IRQ del PMIC/RTC está apagada → el botón es la vía)
                           └────────────────────────┘

   ════════════════════ HABILITADOR TRANSVERSAL ════════════════════
   ┌──────────────────────────────────────────────────────────────────────────────┐
   │ #5 pinctrl-mt6582  (dueño de 0x10005000 + 0x1000B000; FUSIONA gpio-mt6582-eint)│
   │   ├─▶ Vol↑ (pull-up de su columna del keypad)        [hoy falta SOLO esto]     │
   │   ├─▶ táctil por IRQ: edt-ft5x06 nativo  ──(requiere)──▶ #8 apdma-i2c (33B)     │
   │   ├─▶ reset-gpios / vin-supply / wakeup DECLARATIVOS (limpieza DT)             │
   │   └─▶ bias-pull-up/down que pedirán WiFi-nativo y audio en el futuro           │
   └──────────────────────────────────────────────────────────────────────────────┘

   #7 wifi(dongle) ──(requiere)──▶ host/OTG en mt6582-musb + VBUS por FAN5405 ──▶ NetworkManager ──▶ Phosh WiFi
   #9 audio ── independiente (cuelga del MFD/pwrap ya hecho) ── ROI bajo, al final
```

**Puntos clave del grafo:**

- **Los 4 de impacto alto (1,2,3,4) son INDEPENDIENTES entre sí** y todos cuelgan de base ya resuelta (PMIC, EINT, I2C, simpledrm). Se pueden atacar en paralelo o en cualquier orden. Ninguno depende de pinctrl.
- **pinctrl NO bloquea nada de la Fase 1.** Es habilitador de *calidad* (Vol↑, táctil-IRQ, declaratividad), no de funciones nuevas visibles. Encaja como tarea transversal de "limpieza/upstream" que se hace cuando los quick wins estén.
- **power-button es prerrequisito de facto de suspend-pm**: con la IRQ del PMIC/RTC deshabilitada (polling), el único wakeup source viable es el EINT del botón. Hacer #2 primero te regala el wakeup de #6.
- **apdma-i2c solo importa si se quiere táctil nativo** (edt-ft5x06 lee 33B de golpe). Como el daemon ya funciona, apdma es opcional y va detrás de pinctrl.
- **wifi por dongle depende solo de host-mode en musb + VBUS** (FAN5405, ya controlado por I2C), NO de pinctrl ni de IDDIG.

---

## 3. Orden por fases

### **FASE 0 — Backup y red de seguridad** (antes de tocar nada)
*Objetivo: poder volver al estado-bueno-conocido sin reflashear a ciegas.*

1. **Snapshot del estado Phosh que YA arranca** (ya parcialmente hecho: `backup-phosh/`, `apk-installed.txt`, `usrlocal.tar.gz`, `zz-phosh-shell.gschema.xml`, `launch_phosh.sh`). Verificar que el tar restaura en limpio.
2. **Backup del `boot.img` actual** (zImage+dtb+initramfs que arranca) a la Pi `cpcd@192.168.0.123` y a la Mac. Etiquetar como "kernel #23 + Phosh OK".
3. **Backup defensivo de particiones irreemplazables** (`nvram`, `protect_f`/`protect_s`, `md1img`) — IMEI/calibración del modem. Aunque el modem esté fuera de alcance, estos datos **no se regeneran**.
4. **Regla de flasheo confirmada:** `mtkclient` usar `wo` **nunca** `wf`; cargar módulos con `insmod` sin reflashear siempre que se pueda (el musb no aguanta transferir el zImage de 13.7 MB y reinicia). NO restaurar LK Lollipop.

### **FASE 1 — Phosh usable de verdad** (lo crítico; aquí está el 80% del valor)
*Objetivo: que un humano pueda usar el teléfono sin SSH — ver brillo, batería, apagar pantalla, autorrotar.*

| Orden | Tarea | Por qué primero | Entregable visible |
|---|---|---|---|
| 1.1 | **#1 backlight** (PWM_MTK_DISP=y + nodo pwm@1400a000 mt2701 + 2 fixed-clock + pwm-backlight) | El BLS ya está encendido por dispfix; es casi solo DT+config. Sin brillo controlable la pantalla está al 100% siempre (consumo + incómodo). | Slider de brillo de Phosh funciona |
| 1.2 | **#2 power-button** (DT: mt6323 interrupt-parent=&eint, interrupts=<25 LEVEL_HIGH>; mt6323keys power{116}; revertir patch 0004; INPUT_MTK_PMIC_KEYS=y) | Es la pieza más "de móvil" y de mayor visibilidad; sin botón de encendido no hay bloqueo de pantalla. Bajo esfuerzo (DT + revertir un patch). | KEY_POWER → bloquear/desbloquear + menú apagar |
| 1.3 | **#4 battery-upower** (test_power.ko como atajo de validación → módulo `krillin-battery` propio + unificar daemon VBAT+FAN5405 + curva OCV + `apk add upower`) | Icono de batería y "cargando" son señal de usabilidad nº1 en un móvil. El daemon ya lee VBAT (3719 mV verificado) y la carga; solo falta el puente power_supply. | Icono %, animación de carga, avisos batería baja |
| 1.4 | **#3 sensors (accel + ALS)** (IIO_ST_ACCEL_3AXIS=m, TSL2772=m; nodos accel@1d + light@39 a &i2c0 SIN interrupts; `apk add iio-sensor-proxy`) | Autorrotación es lo que hace que se "sienta" un teléfono. Esfuerzo bajísimo (DT + config + paquete). Las lecturas son de 6B ≤ 8B → seguras para el i2c DMA-only actual. | Autorrotación + brillo automático |

> **Hito de salida de Fase 1 = "Phosh usable mínimo viable"** (ver §5). Tras estas 4 tareas el teléfono se usa con la mano, sin terminal.

### **FASE 1.5 — Energía** (cierre natural de Fase 1; depende de power-button)
| Orden | Tarea | Nota |
|---|---|---|
| 1.5 | **#6 suspend-pm (s2idle)** | Hacer **solo s2idle**, NO suspend-to-RAM (S2R necesitaría reimplementar el SPM+PCM = riesgo de brick). Wakeup = EINT del power-key (regalo de 1.2). SIEMPRE armar `rtcwake -s N` o botón antes de cada `echo freeze`. Callback resume crítico: **reprogramar el OVL a RGB565** (si no, vuelve el tinte amarillo). Ahorro modesto (sin DDR self-refresh), pero baja el drenaje en bolsillo. |

### **FASE 1.7 — Habilitador transversal** (cuando los quick wins estén; antes de conectividad si se quiere upstream limpio)
| Orden | Tarea | Nota |
|---|---|---|
| 1.7 | **#5 pinctrl-mt6582** (clonar pinctrl-mt8135 + pinctrl-mtk-common v1; **usar offsets compactos validados en HW**: dir=0x000, ies=0x100, pullen=0x200, pullsel=0x300, dout=0x400, din=0x500, pinmux=0x600, mode 3-bit/5-por-reg; **NO** los de mt8135 0x800/0xA00/0xC00) | **Riesgo nº1: fusionar/retirar `gpio-mt6582-eint.c`** — el common-v1 trae su propio gpio_chip + llama mtk_eint_do_init; no pueden coexistir dos dueños de 0x10005000/0x1000B000. Validar que NO se rompe el reset GPIO115 ni el EINT117 del táctil (hoy funcionan). **Entregable estrella: Vol↑** (pull-up de su columna). |
| 1.7b | **#8 apdma-i2c** (`dma_sync=1` + `apdma_sync=1` en mt6582_compat; 2-4 líneas) | Solo tras pinctrl, y solo si se quiere táctil nativo. Primer intento barato; si falla por coherencia de cache (A7 no-coherente), instrumentar pdmabase. **Plan B cero-riesgo: mantener el daemon ft5336** que ya va. NO bloquea Phosh. |

> **Atajo pragmático si solo se quiere Vol↑ ya:** poke puntual (pullen.set + pullsel-up + mode=KPD) sobre la columna de Vol↑ desde un servicio OpenRC (como se hizo con VGP1 del táctil). Entrega Vol↑ en horas; el pinctrl completo es la vía correcta/upstream.

### **FASE 2 — Conectividad** (lo que falta para que sea autónomo)
| Orden | Tarea | Nota |
|---|---|---|
| 2.1 | **#7 wifi por dongle USB-OTG** | (a) comando `vbus on` en el daemon FAN5405 (OPA_MODE boost) + **medir 5 V con multímetro**; (b) extender mt6582-musb a OTG con `set_mode host` inline (DEVCTL SESSION + PHY 0x6c a host) y **forzar host por sysfs** (sin IDDIG al principio); (c) AR9271/ath9k_htc (1ª opción por fiabilidad) o rtl8188eu (consumo); (d) NetworkManager → panel WiFi de Phosh. **Aviso: en host el FAN5405 está en boost, NO carga** → 1 solo puerto, sin carga+dongle simultáneos. |
| — | WiFi CONSYS nativo | **Fuera de alcance** (cientos de miles de líneas, FW propietario, cero mainline). El dongle es la respuesta. |

### **FASE 3 — Extras / aceleración** (mejoras; ninguna es bloqueante)
| Orden | Tarea | Nota |
|---|---|---|
| 3.1 | **LED/vibrador/poweroff MT6323** (nodos DT hijos del MFD: leds-mt6323, mt6323-poweroff) | Esfuerzo bajo, drivers ya en mainline. Notificación LED + apagado limpio. Buen relleno. |
| 3.2 | **GPU lima (Mali-400)** | **Ver §6 — recomendación honesta: opcional, no prioritario** porque pixman ya va fluido. |
| 3.3 | **#9 audio** (AFE mt6582 desde 0 + codec MT6323) | **Último.** 2 drivers nuevos (~700+~500 líneas) + machine + DT. Validar solo DL1→altavoz (80% del valor). ROI bajo; Phosh es 100% usable sin audio. |
| — | **Modem 3G** | **Fuera de alcance** (ver §7). |

---

## 4. Quick wins (máximo valor / mínimo esfuerzo)

Ordenados por ratio. **Estos cuatro convierten "arranca" en "se usa" en pocos días:**

1. **backlight** — *bajo esfuerzo, impacto alto.* El BLS ya está vivo (dispfix). Casi solo `CONFIG_PWM_MTK_DISP=y` + 2 nodos DT. **Validación de 1 minuto sin tocar kernel:** `devmem2 0x1400A0A0` (PWM_DUTY, 0..1023) para confirmar que el duty controla el brillo antes de cablear el driver. Riesgo único conocido: si el brillo no responde por con1=0xAC, patch de ~10 líneas con compatible `mt6582-disp-pwm` y con1=0xA0.
2. **power-button** — *bajo esfuerzo, impacto alto.* Es **DT + revertir un patch**. El EINT ya funciona (verificado con EINT117). Clave conceptual: el pwrkey NO necesita la IRQ del pwrap (sigue en polling); llega por EINT25. El storm previo era del pwrap, no del PMIC.
3. **sensores accel + ALS** — *bajísimo esfuerzo, impacto alto.* DT (2 nodos sin `interrupts`) + 2 configs IIO + `apk add iio-sensor-proxy`. Drivers mainline puros (st_accel, tsl2772). Lecturas de 6B caben en el margen seguro del i2c DMA-only.
4. **battery → UPower vía test_power.ko** — *medio-bajo esfuerzo para validar TODA la cadena de UI hoy.* `CONFIG_TEST_POWER_SUPPLY=m` + el daemon escribe en `/sys/module/test_power/parameters/{battery_capacity,battery_status,ac_online}` → ya aparece el icono. El módulo propio es luego un rename/limpieza.

> **Hack de mayor valor inmediato:** los 4 quick wins son casi todos **DT + Kconfig + un `apk add`**, sin escribir drivers nuevos. Un solo ciclo de recompilación de kernel (backlight + power-button + accel/ALS configs en el mismo `.config`) los habilita a los tres de golpe; UPower se valida en paralelo con test_power sin recompilar.

---

## 5. "Phosh usable mínimo viable" vs "teléfono completo"

### Phosh usable mínimo viable (MVP) — **fin de Fase 1**
Lo que un humano necesita para usar el teléfono **con la mano, sin SSH**:
- ✅ Pantalla legible + táctil + GUI Phosh (**YA**)
- ✅ **Brillo** controlable (backlight) — sin esto la pantalla quema batería al 100%
- ✅ **Batería visible** (%, cargando, avisos) — saber cuánta carga queda
- ✅ **Botón de encendido** — bloquear/desbloquear/apagar (lo más básico de un móvil)
- ✅ **Autorrotación** — que se sienta un teléfono
- ◐ **Vol↑** (deseable; Vol↓ ya va) — vía pinctrl o poke
- ◐ **s2idle** (deseable) — no agotar batería en el bolsillo

**Veredicto MVP:** alcanzable con los 4 quick wins + (opcional) Vol↑ y s2idle. **No requiere WiFi, audio, GPU ni táctil-nativo.** Es un teléfono-consola-GUI usable offline.

### Teléfono completo — Fases 2-3
Añade lo que lo hace **autónomo y multimedia**:
- **WiFi** (dongle USB) — conectividad real → de "juguete offline" a "usable de verdad". **Es el salto de mayor impacto tras el MVP.**
- **pinctrl completo** — Vol↑ limpio, táctil-IRQ (jubila el daemon), declaratividad/upstream
- **LED/vibrador/poweroff** — feedback físico
- **Audio** — tono/altavoz (ROI bajo)
- **GPU lima** — aceleración (ver §6: opcional)
- ❌ **Modem/llamadas/datos celulares** — fuera de alcance (§7)

> **Honestidad sobre el techo:** ni con todo esto es un daily-driver telefónico (sin modem no hay llamadas/SMS/datos). El objetivo realista es **"PC de bolsillo Linux con GUI móvil + WiFi"**, no "reemplazo de smartphone". El proyecto lo sabe y es la decisión correcta.

---

## 6. Recomendación honesta: lima/GPU dado que pixman ya va fluido

**Recomendación: NO priorizar lima. Dejarlo como Fase 3 opcional, casi "nice-to-have de curiosidad".**

Razones técnicas:
1. **El criterio de éxito ya está cumplido por software.** El informe y el hito confirman que weston/sxmo/**Phosh corren "superfluidos" con pixman** en este qHD (540×960). Si el objetivo es "Phosh lo más usable posible", el cuello de botella **no es el rendimiento de pintado** — ya es fluido. lima resolvería un problema que no existe hoy.
2. **Coste real medio, no bajo.** Requiere nodo `gpu@13010000` (arm,mali-400) + 6 IRQs del GIC + 2 fixed-clocks + asegurar el gate MFG encendido por LK + montar el camino **kmsro** (lima renderiza, simpledrm hace el scanout). Es más frágil que un DT de 2 nodos.
3. **Riesgo de regresión y consumo.** Mali-400 sin gobernador de frecuencia (no hay CCF) corre a clock fijo → puede **consumir más** sin mejorar la experiencia percibida. En un SoC de 1 GB sin power management fino, encender la GPU para algo que ya va fluido es mal negocio energético.
4. **Beneficio marginal concreto.** GLES2 ayudaría a: vídeo/animaciones pesadas, apps GTK4 con mucho compositing, y a **descargar la CPU** (lo único con valor real — liberar los A7 para otras tareas). Pero a 540×960 con pixman ya fluido, el usuario no lo notará en la UI de Phosh.

**Cuándo SÍ valdría la pena (revisitarlo):**
- Si tras medir (Fase 0/1.5) se ve que el **compositing de pixman dispara el consumo de CPU** y eso domina el drenaje de batería → lima podría *bajar* consumo total descargando los A7. Hay que **medirlo**, no asumirlo.
- Si se quiere correr apps concretas que exigen GLES2 (algún juego, mapas con GL).

**Conclusión:** lima es el clásico "porque mola técnicamente", no "porque Phosh lo necesite". Con pixman fluido, **el tiempo rinde mucho más en backlight/batería/power-button/WiFi**. Ponerlo detrás de todo lo demás y solo si una medición de consumo lo justifica.

---

## 7. Qué queda fuera de alcance y por qué

| Fuera de alcance | Por qué | Camino real (si alguna vez se quiere) |
|---|---|---|
| **Modem 3G (CCCI/MOLY, voz/SMS/datos celulares)** | Nadie ha hecho funcionar el **baseband on-die de ningún MTK clásico en mainline**. Depende del subsistema CCCI (inter-core con el modem) + firmware MOLY propietario + RIL binario. Es un proyecto en sí mismo, no un driver. | **Halium + rild binario sobre el kernel downstream 3.10** (Ubuntu Touch ya lo hizo en krillin). En mainline: **solo dump defensivo** de `nvram`/`protect_*`/`md1img` (IMEI/calibración irreemplazables) — hacerlo en Fase 0. |
| **WiFi/BT CONSYS nativo (on-die MT6628-like)** | Stack de cientos de miles de líneas out-of-tree + FW propietario, cero soporte mainline. Coste desproporcionado para 1 GB/armhf. | **Atajo = dongle USB** (Fase 2). El CONSYS nativo es "proyecto aparte" indefinido. |
| **Suspend-to-RAM real (S2R "deep")** | Lo hace el bloque HW **SPM** gobernado por un blob PCM (28×u32) que **no existe en mainline** para MT6582. Sin datasheet del SPM ni del controlador DRAM, alto **riesgo de brick/no-resume**. | **s2idle** (Fase 1.5) cubre el ahorro alcanzable sin firmware. Revisitar S2R solo si s2idle resulta insuficiente. |
| **clk-mt6582 (CCF) completo** | El "cuello de botella raíz" teórico, pero **evitable**: fixed-clock stubs + `clk_ignore_unused` + el LK deja los bloques encendidos ya cubren todo. | Escribirlo **solo** si algún driver futuro exige gating/`clk_set_rate` real (hoy ninguno lo hace). |
| **Voz de llamada por audio** | Aunque se portara el audio (AFE+codec), la ruta de voz depende del **CCCI inexistente** (el modem alimenta el audio de llamada). | Ligado al modem → mismo veredicto. El audio de media (altavoz/auricular) sí es alcanzable (Fase 3). |

---

## TL;DR ejecutivo

- **El trabajo ya hecho es enorme:** Phosh arranca, fluido, con táctil/display/USB/I2C/EINT/PMIC. El objetivo cambió de "arrancar" a "**usar sin SSH**".
- **Haz primero los 4 quick wins** (backlight → power-button → batería/UPower → accel+ALS): casi todos son **DT + Kconfig + `apk add`**, un solo ciclo de kernel, y entregan el **MVP usable**.
- **power-button antes que suspend** (el EINT del botón ES el wakeup que la IRQ-PMIC-apagada te niega).
- **pinctrl es habilitador, no bloqueante** — hazlo cuando el MVP esté, para Vol↑ + táctil-IRQ + upstream. Riesgo: fusionar/retirar `gpio-mt6582-eint.c`.
- **WiFi por dongle USB-OTG** es el mayor salto post-MVP (Fase 2). CONSYS nativo, fuera.
- **lima: NO ahora.** Pixman ya va fluido; lima solo se justifica si una medición demuestra que el compositing CPU domina el consumo. **El tiempo rinde más en batería/WiFi.**
- **Fuera de alcance:** modem (→ Halium/3.10), S2R real (→ s2idle), CCF (→ stubs). **Backup defensivo de nvram/protect_*/md1img en Fase 0** pase lo que pase.

**Archivos clave para ejecutar:** DTS a editar `C:\Users\jferr\Desktop\pmos-krillin\mt6582-bq-krillin-final.dts`; toggles `C:\Users\jferr\Desktop\pmos-krillin\config-mainline-krillin`; driver display (enciende BLS) `mt6582-dispfix.c`; gpio/eint a fusionar en pinctrl `gpio-mt6582-eint.c`; daemon táctil `ft5336_touch.c`; musb a extender a OTG `mt6582-musb.c` (+ secuencia PHY `usb20_phy_downstream.c`); hitos de batería/carga/PMIC en `C:\Users\jferr\Desktop\pmos-krillin\repo_dl\HITO-{BATERIA-WIP,CARGA-FAN5405,PMIC-MT6323}.md`.