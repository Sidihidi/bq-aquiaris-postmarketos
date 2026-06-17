# ROADMAP DE IMPLEMENTACION — BQ Aquaris E4.5 "krillin" (MT6582) en Linux MAINLINE
**Fecha: 2026-06-17** · Kernel objetivo 7.0.12 + Alpine 3.24.1 (armv7) · Síntesis de 12 informes de subsistema

> **Estado de partida (lo que YA funciona en mainline):** arranque del kernel; display legible (simplefb RGB565 + `mt6582-dispfix.c` reprograma el OVL); eMMC (mtk-sd); USB-gadget de red + SSH; I2C (`mediatek,mt6582-i2c`); GPIO por poke (devmem); táctil FT5336 (daemon userspace por I2C0@0x38, eventos en `/dev/input/event0`). PWRAP MT6582 **validado** (CID MT6323=0x2023). VGP1 del táctil se enciende por poke pwrap.
>
> **Idea-fuerza del roadmap:** el cuello de botella raíz de TODO el hardware "profundo" (display real, GPU, WiFi, audio, EINT) es la **ausencia de un clock driver (CCF) y de pinctrl/EINT para el MT6582**. La estrategia ganadora es la que ya usa el proyecto: **fixed-clock stubs + `clk_ignore_unused` + dejar que el LK encienda los bloques**, y atacar primero lo que NO depende de esas piezas. El **PMIC (pwrap+MFD+regulators)** es el habilitador transversal nº1; el **EINT/pinctrl** es el nº2.

---

## 1) Tabla-resumen de los 12 subsistemas

| # | Área | Desbloquea (resumen) | Dificultad | Prioridad | Estado en mainline |
|---|------|----------------------|:----------:|:---------:|--------------------|
| 1 | **gui-x11-fbdev** | GUI usable YA (Xorg sobre `/dev/fb0` + táctil) sin GPU ni DRM | baja | **9** | Userspace estable; todo empaquetado en Alpine armv7. Kernel ya provee fb0/event0 |
| 2 | **pmic-mt6323** | Reguladores por framework (no poke); BASE de batería/carga/audio/WiFi/poweroff | media | **8** | Slave MT6323 + MFD + regulator YA en mainline; **falta solo el compatible SoC `mt6582-pwrap`** |
| 3 | **drm-kms (simpledrm)** | `/dev/dri/card0` → prerequisito de Phosh/wlroots | baja | **8** | `simpledrm` en mainline; solo activar config + conservar dispfix. mediatek-drm NO cubre MT6582 |
| 4 | **eint-buttons (pinctrl/EINT)** | IRQ HW del táctil (edt-ft5x06 nativo), botones gpio-keys, wakeup, reset-gpios declarativos | alta | **8** | Framework v1 (pinctrl-mtk-common + mtk-eint) en mainline; **falta escribir `pinctrl-mt6582.c` + header de pines** |
| 5 | **bateria-porcentaje** | % de batería en la UI (UPower) por voltaje/OCV | media | **7** | IIO + generic-adc-battery + UPower en mainline; **falta compatible AUXADC + camino VBAT** |
| 6 | **sensores-misc** | Autorrotación, brillo auto, proximidad, brújula, vibrador, LED | baja | **6** | Drivers IIO ST/AMS + leds-mt6323 YA en mainline; **falta solo añadir nodos DT**. Magnetómetro = punto flojo |
| 7 | **carga-usb (MT6323)** | Que el teléfono se CARGUE por USB encendido | muy alta | **6** | NO existe driver de carga MT6323 en mainline; hay que escribirlo (riesgo HW Li-ion) |
| 8 | **wifi (CONSYS_6582)** | wlan0 nativo | muy alta | **5** | CERO driver mainline; CONSYS on-die; portar stack downstream gen2 (cientos de miles de líneas) |
| 9 | **gpu-mali-lima** | GL ES 2.0 acelerado → UI fluida | alta | **5** | LIMA(kernel)+Mesa maduros; **falta clocks/power-domain/IRQs del Mali + proveedor KMS** |
| 10 | **phosh-wayland** | Teléfono Linux "de verdad" (Phosh/GTK4 táctil) | muy alta | **4** | Userspace maduro (pmOS); depende en cascada de clk→pinctrl→KMS→panel→lima |
| 11 | **audio (AFE+MT6323)** | Sonido por altavoz/auricular, micro | muy alta | **3** | NADA del MT6582 en mainline; 3-4 drivers nuevos desde cero |
| 12 | **modem-3g (CCCI)** | Llamadas/SMS/datos 3G | muy alta | **2** | CERO en mainline; nadie lo ha logrado en NINGÚN MTK on-die. Camino real = Halium/downstream |

---

## 2) Grafo de DEPENDENCIAS

```
                       ┌─────────────────────────────────────────────────────────┐
                       │  RAÍZ (gaps de plataforma que bloquean lo "profundo")    │
                       │                                                         │
   [LK deja relojes ON]│   clock driver MT6582 (NO existe)  ──┐                  │
   + clk_ignore_unused │   pinctrl/EINT MT6582 (NO existe) ───┼── habilitan ──┐  │
   + fixed-clock stubs │                                      │               │  │
   = workaround usado  └──────────────────────────────────────┼───────────────┼──┘
                                                              │               │
   ┌──────────────────────────────────────────────────────────┘               │
   │                                                                          │
   ▼                                                                          ▼
 (2) PMIC pwrap+MFD+regulators MT6323  ───────────────┐            (4) pinctrl/EINT MT6582
   │  (solo necesita fixed-clock stubs, NO el CCF)     │              │
   ├──► (7) carga-usb (charger MT6323)                 │              ├──► táctil-IRQ (edt-ft5x06 nativo, requiere ADEMÁS arreglar APDMA i2c)
   ├──► (5) batería % (camino PMIC-AUXADC, Fase 2)     │              ├──► (6) vibrador / LED MT6323 (también necesita PMIC en DT)
   ├──► (11) audio (Avdd-supply del codec MT6323)      │              ├──► botones gpio-keys (Vol±); power-key = MT6323 PWRKEY (mt6397-keys)
   ├──► (8) WiFi (raíles VCN_* del MT6323)             │              └──► wakeup desde suspend, reset-gpios/enables declarativos
   ├──► (6) alimentación de sensores (LDO MT6323)      │
   └──► poweroff/shutdown limpio (mt6323-poweroff)     │
                                                       │
 (5) batería % (camino SoC-AUXADC, Fase 1) ◄───────────┘ (NO necesita PMIC; solo parche mt6577_auxadc + DT)

   GUI / GRÁFICOS
   ──────────────
 (1) X11+fbdev  ◄── ya tiene /dev/fb0 (simplefb) + /dev/input/event0  ──►  GUI usable SIN DRM ni GPU  [QUICK WIN]

 (3) simpledrm  ──► /dev/dri/card0  ──┬──► (10) phosh/phoc/wlroots con WLR_RENDERER=pixman (software, lento pero usable)
   │  (conserva dispfix; excluye FB_SIMPLE)            │
   │                                                   │
   └── (9) lima (Mali-400) ──► /dev/dri/renderD128 ────┴──► (10) Phosh ACELERADO (GLES2 vía kmsro)  ── pero "usable", no "snappy" (Mali-400 + 1GB)
          ▲
          └── necesita clocks del Mali (fixed-clock + gate MFG ON por LK) + power-domain MFG ON + 6 IRQs GIC

   FUERA DE RUTA REALISTA EN MAINLINE
   ──────────────────────────────────
 (12) modem-3g ──► depende de CCCI/CCIF + firmware MOLY + (audio para que la llamada suene) → camino real: Halium sobre kernel 3.10
```

**Lecturas clave del grafo:**
- **PMIC (2) es el hub.** Desbloquea 5 áreas (carga, batería-correcta, audio, WiFi, vibrador/sensores-power) y el poweroff. Su único gap es un compatible nuevo en `mtk-pmic-wrap.c` — **alta relación valor/esfuerzo**.
- **simpledrm (3) y X11 (1) son ortogonales:** X11 da GUI YA sobre fbdev; simpledrm es el carril hacia Phosh. **No se hacen a la vez** sobre el mismo FB.
- **lima (9) NO pinta solo:** necesita un proveedor KMS (simpledrm) como scanout vía `kmsro`.
- **EINT (4) y batería-Fase-1 (5)** son los dos caminos que **esquivan** el CCF (usan fixed-clock/polling), por eso son abordables antes que WiFi/audio/Phosh-acelerado.

---

## 3) FASES ordenadas por valor/esfuerzo (con SIGUIENTE PASO concreto)

### FASE 0 — Quick wins inmediatos (horas–días, riesgo ~nulo)

| Item | Siguiente paso concreto |
|------|-------------------------|
| **GUI X11 (1)** | En el teléfono: `apk add xorg-server xf86-video-fbdev xf86-input-evdev xinit xterm jwm ttf-dejavu eudev matchbox-keyboard`. Crear `/etc/X11/xorg.conf.d/10-fbdev.conf` (Driver `fbdev`, `Option "fbdev" "/dev/fb0"`, `Option "ShadowFB" "true"`, DefaultDepth 16) y `40-touch.conf` (evdev, MatchIsTouchscreen). Probar `startx`. **No** poner TransformationMatrix (ejes ya alineados). |
| **Extraer blobs WiFi (8)** | Montar `~/Downloads/BQ Aquaris E4.5...Lollipop/system.img` (si sparse: `simg2img`), copiar `/system/etc/firmware/WIFI_RAM_CODE*`, `WMT.cfg`, `*_patch*.bin` y los launchers a `artifacts/wifi-blobs/`. Anotar la MAC de `nvram.img`. Es 100% accionable hoy aunque el driver tarde meses. |
| **Dumpear particiones del modem (12)** | `dd` de `protect_f`/`protect_s`/`nvram`/`md1img` de la eMMC y **archivar** (calibración RF + IMEI irreemplazables). **NUNCA escribirlas.** Solo dump defensivo. |

### FASE 1 — Power + GUI usable (el corazón de la usabilidad)

| Item | Siguiente paso concreto |
|------|-------------------------|
| **PMIC pwrap (2)** — *primero* | En `mtk-pmic-wrap.c`: añadir `enum PWRAP_MT6582` + `mt6582_regs` **clonando `mt2701_regs`** (offsets WACS2_EN=0x94/INIT_DONE2=0x98/CMD=0x9C/RDATA=0xA0/VLDCLR=0xA4 ya validados por el fixup) + `struct pmic_wrapper_type pwrap_mt6582 {caps=0, init_soc_specific=NULL}` + entrada of_match. **Ruta de menor riesgo:** detectar `INIT_DONE2==1` y SALTAR reset/cipher (el LK ya inicializó). Test: compat temporal `mediatek,mt2701-pwrap` @0x1000d000 debe leer CID 0x2023 por el framework. |
| **PMIC clocks + DT (2)** | Declarar 2 `fixed-clock` stubs 26 MHz (`spi`,`wrap`) — igual que MSDC. Añadir nodo `pwrap@1000d000` con `pmic{compatible="mediatek,mt6323"; regulators{...}}`. Config: `CONFIG_MTK_PMIC_WRAP=y`, `CONFIG_MFD_MT6397=y`, `CONFIG_REGULATOR_MT6323=y` (hoy los tres `is not set`). |
| **PMIC migración táctil (2)** | Poner `vcc-supply=<&mt6323_vgp1_reg>` en el nodo del táctil; verificar VGP1 sube por refcount en `/sys/class/regulator`; **luego** borrar el `late_initcall` del poke de VGP1. Mantener el fixup del táctil hasta confirmar. |
| **Batería % (5)** — *Fase 0 del informe* | **Decidir de dónde sale VBAT antes de escribir código:** medir con multímetro y comparar (a) AUXADC del SoC @0x11001000 vs (b) AUXADC del MT6323 (canal 7, vía pwrap). El downstream usa el del PMIC → probable que el del SoC NO tenga VBAT. Esto decide Fase 1 (barata) vs Fase 2 (driver IIO nuevo). |
| **Batería % (5)** — *si SoC-AUXADC* | Parche de ~6 líneas: añadir compatible `mediatek,mt6582-auxadc` a `mt6577_auxadc.c` (reusar struct mt8173) + nodo `auxadc@11001000` (fixed-clock) + nodo `generic-adc-battery` con `io-channels` + curva OCV de `cust_battery_meter_table.h` del mt6582. UPower lo detecta solo. |
| **Carga USB (7)** — *prototipo* | **Antes de tocar kernel**, replicar el método VGP1: por poke pwrap sobre CHR_CON del MT6323, secuencia `RG_VCDT_HV_EN→RG_VBAT_CV_VTH(4.2V)→RG_CS_VTH(500mA)→RG_CSDAC_EN→RG_CHR_EN`. Extraer los offsets de bit de `upmu_hw.h`/`pmic_mt6323.c` del downstream del krillin. **Validar con USB power-meter + vigilar VBAT.** Empezar 500 mA / CV 4.2 V exactos. |

### FASE 2 — Conectividad + entrada (más valor de uso diario)

| Item | Siguiente paso concreto |
|------|-------------------------|
| **Sensores (6)** — *casi quick win* | Con el I2C ya vivo: `i2cget -y 0 0x1d 0x0F` y `0x6b`/`0x6a` (WHO_AM_I) para confirmar accel(LSM330)/gyro(L3GD20). Habilitar `CONFIG_IIO_ST_ACCEL/GYRO_*` + `CONFIG_TSL2772`. Añadir nodos hijos a `&i2c0` (`st,lsm330-accel`@0x1d, `st,l3gd20`@0x6b, `amstaos,tmd2772`@0x39) **sin `interrupts`** (polling). Instalar `iio-sensor-proxy`. Magnetómetro@0x30 al final (probar `memsic,mmc35240`; si es ST480 no hay driver). |
| **EINT/pinctrl (4)** | Extraer del downstream mt6582 (`mt_pin_info.h`/`cust_eint.h`) la tabla de 169 pines con su `eint_n` y **confirmar el EINT real del pin INT del FT5336** (la nota dice "117" pero puede ser nº de GPIO, no EINT). Generar `pinctrl-mtk-mt6582.h` (molde de mt8135) y `pinctrl-mt6582.c` (offsets GPIO@0x10005000 + eint_hw{ap_num=169,ports=6,port_mask=7,db_cnt=16}, EINT base 0x1000B000). `CONFIG_PINCTRL_MT6582=y`. Nodo `pinctrl@1000b000` + `gpio-keys` (Vol±; power va por MT6323 PWRKEY/mt6397-keys). |
| **Táctil nativo (4)** | **Bloqueado por el APDMA del i2c-mt65xx** (edt-ft5x06 lee 33 B de golpe; hoy solo van ≤8 B fiables). Arreglar el APDMA primero. Mientras tanto: dar al daemon ft5336 el fd del IRQ EINT (sustituye polling) como paso intermedio. |
| **WiFi (8)** | **Posponer en mainline** (depende de clk+pinctrl+PMIC-DT+SPM). Camino: portar `conn_soc`+`wlan/gen2` como módulo out-of-tree (shims de API 3.4→7.x) + nodo DT del CONSYS (4 regiones, direcciones de `mtk_wcn_consys_hw.c` del mt6582) + userspace (`wmt_loader`/launchers → `/dev/wmtWifi`). **Alternativa táctica:** dongle USB WiFi mainline (`mt7601u`/`rtl8188eu`) por OTG, o seguir con g_ether. |
| **Audio (11)** | **Posponer** (ROI bajo, 3-4 drivers nuevos, depende de PMIC-regulators + clocks). Cuando se aborde: MFD MT6323 → codec `sound/soc/codecs/mt6323.c` (hijo del pwrap, patrón mt6358) → plataforma AFE `mt6582/` (base 0x11220000) → machine driver. Validar incrementalmente solo DL1 (playback a altavoz). |

### FASE 3 — Aceleración → Phosh (ver §4 para el detalle)

| Item | Siguiente paso concreto |
|------|-------------------------|
| **simpledrm (3)** | `CONFIG_DRM_SIMPLEDRM=y` **y** `CONFIG_FB_SIMPLE` is not set (mutuamente excluyentes). **CLAVE:** re-anclar `mt6582-dispfix.c` en el Makefile de `obj-$(CONFIG_FB_SIMPLE)` a `obj-y`/Kconfig propio para que el fix del OVL siga compilando (si no → pantalla amarilla). Verificar `/dev/dri/card0`, `modetest`, y que la pantalla sigue legible. |
| **Compositor de prueba (10)** | Antes de Phosh entero: `weston` (drm-backend) o `cage` con `WLR_RENDERER=pixman` sobre card0 para validar que pinta + que el táctil emite eventos libinput. zram+swap obligatorio (1GB). |
| **lima (9)** | Nodo `gpu@13010000` (`arm,mali-400`,`arm,mali-utgard`; reg 0x10000; **6 IRQs GIC** `gp/gpmmu/pp0/ppmmu0/pp1/ppmmu1` — sacar números de `/proc/interrupts` del kernel 3.10 en vivo); `clocks` = 2 fixed-clock (`bus`,`core`) + asegurar por devmem que el LK deja MFG (gate `MT_CG_MFG_G3D`@0x13000000) **desgateado** y dominio SYS_MFG ON; **OMITIR `power-domains`** (opcional). Esperar en dmesg `gp - mali400 ... renderD128`. `apk add mesa-lima mesa-egl mesa-gbm`. |
| **Phosh acelerado (10)** | `phoc` con `WLR_RENDERER=gles2`; si hace falta, `WLR_DRM_DEVICES` apuntando a la card de simpledrm (scanout) dejando que wlroots use renderD128 (lima) por kmsro. Gestionar expectativas: **usable, no fluido.** |

### FASE 4 — Modem (fuera de alcance mainline realista)

| Item | Siguiente paso concreto |
|------|-------------------------|
| **modem-3g (12)** | **Declarar fuera de alcance en mainline.** Para un teléfono usable con voz/SMS: Halium (contenedor Android con rild MTK binario) + ofono-rilmodem **sobre el kernel downstream 3.10** (camino probado por Ubuntu Touch en el krillin). En mainline nadie ha hecho funcionar el baseband on-die de ningún MTK clásico. Lo único accionable hoy ya está en Fase 0 (dump defensivo de particiones). |

---

## 4) RUTA ESPECÍFICA A PHOSH (con realismo 1GB RAM + Mali-400)

Phosh es el **último eslabón**, no el primero. Hay **dos rutas**; recórrelas en este orden:

### RUTA A — Phosh por SOFTWARE (Pixman), AHORA — para validar el stack sin esperar GPU
1. **simpledrm en vez de simplefb.** Activar `CONFIG_DRM_SIMPLEDRM=y`, desactivar `CONFIG_FB_SIMPLE`, **conservar el fixup OVL RGB565** re-anclado en el Makefile. Resultado: `/dev/dri/card0` (1 CRTC + 1 plano primario, formato `r5g6b5`) que es lo que wlroots exige (simplefb NO da card0).
2. **Verificar input:** el FT5336 ya emite `/dev/input/event0`; comprobar con `libinput list-devices`/`debug-events`.
3. **phosh+phoc desde pmOS con `WLR_RENDERER=pixman`** y backend DRM sobre simpledrm. UI **usable-pero-lenta** sin tocar GPU. Sirve para depurar input, escalado HiDPI (540×960 → scale 1.5), squeekboard y rotación.
4. **zram + swap obligatorio** (1GB es el mínimo absoluto para Phosh). Vigilar `gsd-power` (fuga conocida, crece a 150-300 MB). Phosh idle ya es ajustado en 1GB pero arranca.

### RUTA B — Phosh ACELERADO (el objetivo del usuario)
**El "DRM-KMS atomic real" para MT6582 NO es viable a corto plazo** (mediatek-drm no cubre MT6582, su modelo ddp_comp/mutex asume HW tipo mt8173+ con CMDQ/GCE que el MT6582 no tiene; escribirlo es un proyecto de meses). **La decisión arquitectónica clave es desacoplar "aceleración GL" de "KMS del panel":**

```
   simpledrm  →  /dev/dri/card0  (scanout, dumb buffer, SIN page-flip/vblank HW → posible tearing)
       +
   lima       →  /dev/dri/renderD128  (render GLES2 sobre Mali-400)
       └────────── kmsro ──────────┘
                     ↓
   phoc (wlroots, WLR_RENDERER=gles2) compone por GPU a un dumb buffer de simpledrm
                     ↓
                  PHOSH
```

**Orden ESTRICTO de la Ruta B (de prerequisito a meta):**
1. **(prerequisito ideal) clock driver `clk-mt6582.c`** portando topología del downstream `mt_clkmgr` al CCF. **Es el trabajo más grande y arriesgado del proyecto** y el cuello de botella real. *Atajo del proyecto:* fixed-clock + dejar gate GPU ON por LK + `clk_ignore_unused` (coherente con MSDC).
2. **pinctrl/EINT (Fase 2)** para IRQs/GPIO limpios del panel.
3. **MFD MT6323 + regulators en DT (Fase 1)** para VGP1/backlight/DSI rails.
4. **lima** sobre el card0 de simpledrm (no meter el panel en KMS).
5. **phoc con GLES2 vía kmsro.**

**Realismo (gestionar expectativas):**
- **Mali-400 MP2 a 500 MHz + qHD + 1GB RAM es "super low end"** (lo dice el wiki de pmOS). Phosh acelerado será **funcional, NO snappy**: animaciones lentas.
- **simpledrm no tiene page-flip real** → puede haber tearing o caída a copia por CPU, reduciendo el beneficio esperado de la GPU.
- **Plan B si decepciona:** Phosh forzado a Pixman (a veces comparable en HW con poco ancho de banda de memoria), o un kiosko `cage`/`sway`.
- **No soporta Vulkan** (Utgard solo GLES2) — pero Phosh/GTK4 solo necesitan GLES2, así que **lima es funcionalmente suficiente**.
- Confirmar que el Mesa de Alpine 3.24 trae **gallium lima habilitado para armv7** (`mesa-dri-gallium`).

---

## 5) QUICK WINS destacados (máximo valor / mínimo esfuerzo)

1. **🥇 GUI X11 sobre fbdev (área 1, dificultad baja, prio 9).** GUI gráfica real con táctil **en horas**, solo instalando paquetes Alpine + 2 ficheros de config. **No necesita nada nuevo del kernel.** Es el mayor salto de usabilidad por hora invertida y desbloquea probar apps, teclado en pantalla (matchbox-keyboard) y navegador ligero ya.
2. **🥈 PMIC pwrap MT6582 (área 2, prio 8).** Un compatible nuevo (~clonar `mt2701_regs` + saltar init porque el LK ya inicializó) **desbloquea 5 subsistemas** (carga, batería-correcta, audio, WiFi, vibrador/sensores-power) + poweroff limpio. El MFD y el regulator MT6323 ya están en mainline; el trabajo es pequeño y muy apalancado.
3. **🥉 Sensores por I2C (área 6, dificultad baja, prio 6).** Accel/gyro/ALS son **plug-and-play**: chips ST/AMS con driver IIO upstream, sobre el I2C que ya funciona. Solo añadir nodos DT sin `interrupts` (polling) → autorrotación y brillo auto vía iio-sensor-proxy. (Vibrador/LED esperan al PMIC en DT.)
4. **Extraer firmware WiFi + dump de particiones del modem (Fase 0).** Baratísimo y **urgente como seguro**: los blobs y la calibración RF/IMEI son irreemplazables. Hazlo aunque los drivers tarden.
5. **Batería %: Fase 0 de medición (área 5).** Una tarde con multímetro + devmem **decide la arquitectura entera** (camino barato SoC-AUXADC vs caro PMIC-AUXADC) y evita escribir código en el camino equivocado. Si sale SoC-AUXADC, el parche es de ~6 líneas.

---

**Secuencia recomendada en una frase:** Fase 0 (X11 + dumps) → PMIC pwrap → batería% + carga (prototipo por poke) → sensores → EINT/pinctrl → simpledrm + Phosh-Pixman → lima + Phosh-acelerado; **WiFi y audio se atacan cuando exista la base PMIC/EINT; el modem queda para Halium sobre downstream, fuera del mainline.**

---

Archivos del repo relevantes para los "siguientes pasos": `/Users/cpcd/Desktop/pmos-krillin/mainline/config-mainline-krillin` (flags PMIC/DRM/LIMA confirmados: `CONFIG_MFD_MT6397`, `CONFIG_MTK_PMIC_WRAP`, `CONFIG_DRM_SIMPLEDRM` están `is not set`; `CONFIG_FB_SIMPLE=y`; `CONFIG_DRM_LIMA=m`), `/Users/cpcd/Desktop/pmos-krillin/mainline/dts/mt6582-bq-krillin.dts`, `/Users/cpcd/Desktop/pmos-krillin/mainline/drivers-video/` (mt6582-dispfix.c), `/Users/cpcd/Desktop/pmos-krillin/mainline/drivers-touch/`, `/Users/cpcd/Desktop/pmos-krillin/mainline/HITO-M3-DISPLAY-RESUELTO.md`.