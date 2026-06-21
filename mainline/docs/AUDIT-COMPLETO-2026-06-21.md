# AUDITORÍA EXHAUSTIVA — Port Linux mainline + postmarketOS al BQ Aquaris E4.5 ("krillin", MediaTek MT6582)

> Fecha de corte: **2026-06-21**. Kernel vivo: **mainline 7.0.12 `#92`** (build de 2026-06-20 23:35). Teléfono alcanzable y sano. Fuentes cruzadas: docs locales (`Desktop\pmos-krillin`), firmwares stock (`Desktop`), memoria `.claude`, Raspberry Pi de build (`cpcd@192.168.0.38`), teléfono en vivo (`root@172.16.42.1`), repo GitHub privado (`Sidihidi/bq-aquiaris-postmarketos`).

---

## 1. RESUMEN EJECUTIVO

El proyecto está **mucho más avanzado de lo que sugeriría un port MT6582 de 2014**. Existen **dos tracks**: (A) un port **postmarketOS sobre el kernel downstream 3.10** que funciona pero sin GPU, hoy en segundo plano; y (B) la **línea principal — Linux mainline 7.0.12 + Alpine 3.24.1 + Phosh acelerado por GPU (lima/Mali-400)** corriendo como sistema principal **desde la tarjeta SD** (dual-boot con el Android interno intacto). En mainline funcionan de forma sólida y verificada en hardware: **display DRM nativo (`mtk_drm`, panel HX8389), GPU lima, táctil FT5336, backlight, almacenamiento eMMC+SD, red USB+SSH, carga de batería (FAN5405) y toda la pila de sesión D-Bus/polkit/elogind/seatd**. La conectividad está a medio camino: **Bluetooth llega a `hci0 UP RUNNING` pero el RF no radia (RX/TX = 0 bytes, MAC placeholder)**; **WiFi descarga el firmware pero el MAC nunca afirma `WLAN_READY`**; **GPS tiene toda la cadena userspace viva (`/dev/stpgps`→bridge→gpsd→socket) pero le falta el motor del protocolo MTK `0xAAF0` (mnld)**. Quedan **sin empezar en mainline**: sensores/autorrotación (IIO vacío), audio (sin nodo ASoC), batería→UPower (sin driver `power_supply`, de ahí el icono a 0%), modem y suspend profundo. El conocimiento está **excepcionalmente bien documentado** (≈40 bitácoras HITO + memorias densas), y el código está respaldado en GitHub; el **punto débil crítico es que los dumps irrecuperables de hardware (`nvram.img`, `proinfo.img`, `seccfg.img`, `lk.img`) NO están respaldados remotamente** (excluidos por `.gitignore`).

### Tabla de subsistemas de un vistazo

| Subsistema | Estado | Mecanismo en mainline | Falta para cerrar |
|---|---|---|---|
| **Display (DRM mtk_drm)** | ✅ Funciona | `mediatek-drm` card1, OVL+RDMA+COLOR+DSI, panel HX8389, fb0 540x960x32 | Nada (cerrado) |
| **GPU (lima / Mali-400 MP2)** | ✅ Funciona | `lima` card0, gp+pp0+pp1, renderD128, power-domain MFG por poke SPM | Nada (Phosh acelerado) |
| **Táctil (FT5336)** | ✅ Funciona | i2c0@0x38, daemon userspace `ft5336_touch`→uinput event1 | Driver kernel EINT (mejora, no bloqueante) |
| **Almacenamiento (eMMC + SD)** | ✅ Funciona | `mtk-sd` mmc0(eMMC)+mmc1(SD), root en `mmcblk1p1` | Nada |
| **Red USB + SSH** | ✅ Funciona (con muleta) | `mt6582-musb` gadget, usb0 172.16.42.1, sshd | Limpiar estado OpenRC "crashed" (cosmético) |
| **Boot / estabilidad** | 🟡 Parcial | seedrng+haveged; sshd por bucle de reintento | Causa raíz del cuelgue runlevel `boot` (udev-settle?) |
| **Backlight / brillo** | ✅ Funciona | daemon `mt6582-backlight` → PWM_DUTY@0x1400a0a0+0xa0 | Driver kernel que escriba +0xa0 (limpieza) |
| **Pila energía (polkit/UPower/elogind/seatd)** | ✅ Funciona (parcial UPower) | dbus+polkit+seatd+elogind vivos, login1 responde | Registrar sesión Phosh ante login1; backend batería |
| **Bluetooth** | 🔴 Roto (RF) | `hci0` por BTIF/STP/WMT, BlueZ 5.86, discoverable | RF no radia (RX/TX=0); MAC real de NVRAM |
| **WiFi** | 🔴 Roto | `mt6582-wifi` HIF@0x180F0000, FW descargado | `WLAN_READY` no asserta; init que el scaffold salta |
| **GPS** | 🟡 Parcial | `/dev/stpgps`→bridge→gpsd→socket vivos | Motor `0xAAF0` (portar/correr mnld) |
| **Sensores / autorrotación** | 🔴 No empezado | — (IIO vacío, ningún driver de sensor) | Instanciar nodos accel/gyro/mag/ALS en DT |
| **Batería (UPower)** | 🔴 Roto a UI | VBAT real legible por script (`battery`), no por UPower | Driver `power_supply` kernel o shim UPower |
| **Audio** | 🔴 No empezado | — (sin tarjeta ALSA, `/proc/asound` no existe) | Nodo ASoC/AFE + codec MT6323 en DT |
| **Carga (FAN5405)** | ✅ Funciona | daemon `fan5405-charge` i2c0@0x6a, CV 4.2V/800mA | Reflejar en UPower (ver batería) |
| **Modem (3G/CCCI)** | 🔴 Fuera de alcance | — | Vía Halium sobre 3.10 (decisión arquitectónica) |
| **Suspend (S2R)** | 🔴 No empezado | — | s2idle (deep S2R descartado); requiere power-button antes |
| **pmOS 3.10 (track A)** | ✅ Funciona (limitado) | kernel LineageOS 3.10.107, Sxmo X11, WiFi | Sin GPU (lima exige ≥5.2); congelado |

Leyenda: ✅ Funciona · 🟡 Parcial · 🔴 Roto/No-empezado.

---

## 2. MAPA COMPLETO DE TODO

### 2.1 Local — `C:\Users\jferr\Desktop\pmos-krillin` (≈1213 ficheros / 57 dirs)

Carpeta de trabajo. El grueso son **~610 scripts `.py` + ~68 `.sh`** de automatización (paramiko/SSH desde este PC → Pi → teléfono) y **árboles fuente C de MediaTek** como referencia. Conteo: `.py` 610 · `.c` 219 · `.h` 212 · `.sh` 68 · `.md` 54 · `.dts/.dtsi` 9 · `.img` 7 · `.json` 3.

```
pmos-krillin/
├── [RAÍZ] ~520 ficheros: 54 .md (docs/HITOs), 3 .json, ~610 .py, ~68 .sh,
│          + C de drivers (mt6582-musb.c, mt6582-dispfix.c, gpio-mt6582-eint.c,
│            ft5336_touch.c, mt6582-btif*.c, mt6582-consys.c, mt6582-pmic-fixup.c,
│            tiny_init_*.c, pwrap_poke.c, devmem.c) + configs kernel + .img grandes
├── golden/                  Backup de particiones del teléfono (BINARIOS irreemplazables)
│     boot.img(20M) lk.img(384K) nvram.img(5M) proinfo.img(3M) recovery.img(20M) seccfg.img(128K)
├── backup-phosh/            Snapshot Phosh que arranca (#25): apk-installed.txt(736 pkgs),
│                            usrlocal.tar.gz(4.9M binarios phosh/phoc), launch_phosh.sh, gschema
├── osk-config/              Teclado en pantalla squeekboard: phosh-session.sh, sm.puri.OSK0.service
├── device-bq-krillin/       aport pmOS: APKBUILD + deviceinfo (offsets MTK boot.img)
├── linux-bq-krillin/        aport pmOS: APKBUILD (kernel 3.10.107 LineageOS)
├── mainline/                Artefactos mainline (subset; DESINCRONIZADO: el repo está más completo)
│     HITO-M2b-WIP.md, config-*, mt6582.dtsi.ref, *.dts.ref, drivers-musb/, dts/usb-node.dtsi
├── repo_dl/                 Espejo GitHub (2026-06-17 mañana): HITOs PMIC/batería/touch/EINT/GUI-X11
├── repo-sync/               Espejo GitHub MÁS RECIENTE (2026-06-19): HITOs WiFi-CONSYS/M3A/LIMA,
│                            PHOSH-BWRAP-FIX, SECUENCIA-ARRANQUE-CONSYS, GPS-CONSYS,
│                            CATCHUP_* (BT funciona, 4 radios on, dual-boot SD), CATCHUP_wifi-bringup.sh
├── wifi-work/               Encargo driver WiFi (diseño + scaffold + fuentes vendor)
│     BRIEF.md, WIFI-DESIGN.md, WIFI-ROADMAP.md
│     ├── our-drivers/       mt6582-btif.c, mt6582-consys.c (NUESTROS, funcionan)
│     ├── scaffold/          mt6582-wifi.c, mt6582-wifi-reg.h (esqueleto a integrar)
│     ├── wmt-soc/           stack WMT downstream (ref)
│     └── downstream-wlan/wlan/  driver 802.11 MTK COMPLETO (176k líneas, ref)
├── _connsoc_common/         Árbol fuente downstream conn_soc/common (stack CONSYS, ref)
├── _connsoc_wlan/           Árbol fuente downstream WLAN (otra copia, ref)
├── disp-research/           Fuentes DISP: ds_*=downstream 3.10, ml_*=mtk_drm mainline, panel_hx8389.c
├── research_mt6582_disp/    Fuentes DISP kernel 3.10 (ddp_bls/color/ovl/path)
├── research_mt6582_lk/      Fuentes DISP del bootloader LK (lcd_drv, disp_drv, mt_logo)
└── [imágenes grandes raíz]  pmos-root.img(586M), krillin_kernel.bin(3.7M),
                             config-mainline-krillin / config-PI-actual (~290K), uinput.ko(74K)
```

**Familias de scripts (raíz):** `build_v10..v25` / `build_{m2b,m3,disp,eint,i2c,keypad,quickwins}*` (compilar+ensamblar boot.img en la Pi); `flash_*` / `dd_v19..24` / `flash_boot_dd*` (flashear por fastboot o `dd` al sector 83968 con md5+rollback); `read_*` / `check_*` / `diag_*` / `recon_*` / `find_*` / `extract_*` (diagnóstico: leen el veredicto en pantalla o ramoops/devmem porque el UART no entrega datos); por subsistema: `recon_usb`/`usb_link_up`, `recon_mtkdrm`/`fix_colors`, `poke_touch_reset`/`run_evtest`, `poke_vgp1`/`validate_pwrap`/`val_backlight`, `build_btif{A,CD,STP}`/`dump_consys_regs`/`extract_consys_seq`/`gps_*`/`fm_invest`, `run_phosh2..9`/`fix_bwrap`/`fix_osk`/`diag_schemas`. **`push_m2b.py` contiene el PAT de GitHub en claro** (rotar).

### 2.2 Firmwares stock — `C:\Users\jferr\Desktop\`

Las 4 ROMs MTK comparten **exactamente la misma tabla de particiones** (`MT6582_Android_scatter.txt`, project=krillin, EMMC, block 0x20000). **Ninguna trae imagen de NVRAM ni de calibración.**

| Carpeta | Qué es | Scatter | Imágenes | NVRAM | Uso |
|---|---|---|---|---|---|
| **`1.5.2_krillin`** | Stock **KitKat 4.4.2** (BQ 1.5.2) + boot rooteado | Sí (offsets físicos reales) | Set MTK completo + `kernel_krillin.bin`, `boot-ROOTED.img`(md5 a0c5ee69), `ramdisk*.img` | **No** | **Restauración eMMC segura** (LK KitKat = NO bootloop); captura ADB-root |
| **`...Firmware 2.1.0 - Lollipop`** | Stock **Android 5.1** (BQ 2.1.0) | Sí (`physical_start_addr:0x0`) | Set completo + variantes `-verified` / `_bthdr` | **No** | Flasheo 5.1. ⚠️ **LK Lollipop = NO restaurar** (verifica firma → bootloop) |
| **`...2.0.1_20150623_..._MT6582`** | Stock **Android 5.0** (BQ 2.0.1) + drivers + SN tool | Sí (subcarpeta `Firmware\`) | Set completo + **`Driver\`** (VCOM/CDC) + **`SN Write Tool v2.1504`** (IMEI/BT/WiFi MAC) | **No** | Flasheo 5.0 + drivers Windows + recuperar IMEI/MAC |
| **`lineage-13.0-...-krillin`** | **LineageOS 13 / Android 6.0.1** (zip OTA) | No | zip recovery (`system/`, updater-script: system=p5, data=p7) + **firmware WiFi/BT/FM/GPS** | No | Instalar por TWRP; **fuente de blobs propietarios** |
| **`TWRPv3.0.2.0`** | Recovery TWRP 3.0.2.0 | Sí (solo recovery) | `recovery.img`(8.4M) | No | Puente SPFT→zip Lineage |
| **`SP_Flash_Tool_v5.1924_Win`** | Herramienta de flasheo | No | DA (`MTK_AllInOne_DA.bin`, `DA_PL*.bin`) | n/a | El programa que escribe la eMMC |

**Layout de particiones (scatter canónico 1.5.2):** PRELOADER 0x0 · MBR 0x1400000 · EBR1 0x1480000 · PRO_INFO 0x1500000 (PROTECTED) · **NVRAM 0x1800000/0x500000 (BINREGION, file=NONE)** · PROTECT_F 0x1d00000 · PROTECT_S 0x2700000 · SECCFG 0x3100000 · UBOOT/lk 0x3120000 · BOOTIMG 0x3180000 · RECOVERY 0x4580000 · SEC_RO 0x5980000 · LOGO 0x6000000 · ANDROID/system 0x6d00000/0x40000000 · CACHE 0x46d00000 · USRDATA 0x72900000.

**Conclusión NVRAM:** un flasheo normal **preserva** la NVRAM existente (es BINREGION/PROTECTED, SPFT no la toca) pero **no se puede "restaurar de fábrica"** desde estos archivos. Lo único de identidad es **SN Write Tool** (IMEI/BT/WiFi MAC, no calibración RF completa). Para respaldar NVRAM hay que hacer **Readback con SPFT o `dd`** — ese dump **sí existe** ya en `golden/nvram.img` (5M, datos reales confirmados).

### 2.3 Memoria `.claude` — `...\pes14pspmod\memory\` (31 ficheros)

El almacén aloja **tres proyectos sin relación**: (A) modding PES14 PSP — 11 memorias, (B) **krillin/MT6582 — 18 memorias**, (C) TomTom root — 1. Solo B es relevante aquí. Memorias del bloque B (todas en `MEMORY.md`):

| Memoria | Tema | Estado capturado |
|---|---|---|
| `project_pmos_bq_e45.md` | Proyecto global, batalla del bootloader | Recuperación total; **GOLDEN RULE: KitKat 1.5.2 (LK F0F3A93E) sí, Lollipop (7EB8C318) bootloop**; OpenRC siempre |
| `reference_mt6582_mainline_estado.md` | Estado consolidado #22→#36+ | Mainline 7.0.12+Alpine como sistema principal; Phosh arranca; backlight era "muro" |
| `reference_mt6582_disp_tint.md` | Tinte amarillo (RESUELTO) | Formato simplefb argb8888 vs OVL rgb565; bases DISP completas |
| `reference_mt6582_musb.md` | USB MUSB gadget (RESUELTO) | g_ether+SSH+rootfs Alpine; 3 bugs musb; MAC 0x11200000, IRQ SPI32 |
| `reference_mt6582_touch.md` | Táctil FT5336 (RESUELTO e2e) | i2c auto_restart=1, VGP1 por servicio, uinput sin reflashear |
| `reference_mt6582_apdma_i2c.md` | APDMA i2c (>8B cuelga) | DMA-ONLY; FIX dma_sync=1; prioridad BAJA |
| `reference_mt6582_sensors_rotation.md` | Sensores IIO | Chips REALES: LSM330/L3GD20/MMC3516X/TMD2772; drivers mainline; IRQ opcional |
| `reference_mt6582_drivers_roadmap.md` | Roadmap drivers post-Phosh | 4 quick wins; power-btn antes que suspend; lima no-prioritario |
| `reference_mt6582_disp_drm_roadmap.md` | Port mtk_drm | MT2701 gemelo; Fase 1 ✅, Fase 2 compilada; DSI+HX8389=muro |
| `reference_mt6582_backlight.md` | Backlight | PWM_DUTY +0xa0 (no +0xac); daemon+CLI `bl`; slider Phosh bancado |
| `reference_phosh_bwrap_fix.md` | Phosh SIGABRT | `chmod u-s /usr/bin/bwrap` |
| `reference_mt6582_wifi_consys.md` | CONSYS WiFi/BT/GPS/FM | combo @0x18070000; capas; fix 1-byte canal STP=4 |
| `reference_mt6582_wifi_hif.md` | WiFi data-path HIF | HIF@0x180F0000; WLAN_READY no asserta; FIX#1/#2 descartados en HW |
| `reference_mt6582_gps.md` | GPS /dev/stpgps | START_SEQ capturado; protocolo 0xAAF0; falta motor mnld |
| `reference_mt6582_flash_frompmos.md` | Flash sin fastboot | **boot REAL = sector 83968 (0x2900000)**, NO 0x1D80000 |
| `reference_mt6582_boot_stability.md` | Boot estabilizado | seedrng+haveged; bucle reintento sshd |

### 2.4 Raspberry Pi de build — `cpcd@192.168.0.38` (uptime 1d 21h, raíz 84%)

```
~/mainline/
├── linux-7.0.12/                  Árbol kernel mainline (NO es git; tarball editado). Build #92
│   ├── build-krillin/.config      290K, fechado 2026-06-20 23:35 = el #92 que corre el teléfono
│   ├── drivers/soc/mediatek/      mt6582-{mfg-power,consys,btif,wifi}.c (todos obj-y, built-in)
│   │        mt6582-pmic-fixup.c   (presente pero NO en Makefile → no se compila)
│   │        mtk-mmsys.c (+mt6582), mtk-pmic-wrap.c (71K, pwrap mt6582)
│   ├── drivers/pwm/pwm-mtk-disp.c  Backlight DISP (+.bak-pre-bl)
│   └── arch/arm/boot/dts/mediatek/mt6582-bq-krillin.dts  13.7K, pipeline DRM completo (10 .bak)
├── downstream/                    Árbol kernel 3.10 ubports/BQ (referencia de offsets)
├── pkg/                           ~50 boot-*.img (histórico) + assemble.sh / assemble-sd-gps.sh
│        flash_boot_dd*.sh (⚠️ usan OFF=0x1D80000 = a ceros; el bueno es 83968)
│        helpers C: devmem memdump memwrite pwrap_poke fbfmt fbinfo reboot2 tiny_init ft5336_touch
├── (junto al árbol) build-*.log, dmesg-mainline-v*.txt, ramoops-v*.bin, serial-v*.txt
~/golden/                          Dumps GOLDEN por BROM (datos reales confirmados vs /dev/zero):
│        boot.img(20M) recovery.img(20M) lk.img(384K) preloader.bin(124K) nvram.img(5M)
│        protect_f.img(10M) protect_s.img(10M) seccfg.img(128K) region.bin(109M)
│        proinfo.img(3M=TODO CEROS, normal) + .state(hwcode 0x6582, BROM abierto sin SBC/SLA)
~/wifi-fw/extracted/               WIFI_RAM_CODE_MT6582(160480B) +_E6/_MT6628, patch_e1_{0,1}, WMT_SOC.cfg
~/gps-fm-extract/                  bin/{6620_launcher,mtk_agpsd,wmt_loader}, lib/{libmnl.so 7.3M,...}
~/firmware-stock/                  boot.img(4.36M) + lk.bin(277K) stock
~/mtkclient/                       Flasher BROM (HEAD 2c9f4d7). Usar `wo`, NUNCA `wf`
~/ (otros)                         capa0-backup.sh, wifi-bringup/check/iter.sh, rnt.sh, edit_i2c.py,
                                   99gpsallow, mnld-{init,live}.strace, busybox/dropbear/box86 builds
```

**Avisos de coherencia con la memoria:** (1) `~/wifi-work` **no existe** en la Pi — es `~/wifi-fw` + `~/gps-fm-extract`; (2) los `flash_boot_dd*.sh` apuntan a **0x1D80000 (zeroed)**; el flasheo bueno usa el sector **83968**; (3) hay un **proyecto separado en el home** (controlador flash USB CBM2199ES/ChipsBank + trabajo CVE-2026-31431) — no relacionado con krillin.

### 2.5 Teléfono en vivo — `root@172.16.42.1` (mainline 7.0.12 #92, Alpine 3.24.1)

```
/etc/local.d/*.start    (orden por prefijo)
  touch-power.start      VGP1 + reset GPIO115 del táctil
  usb0.start / zy-usb-route.start   red USB
  zz-consys-bt.start     bring-up CONSYS BT (echo a /sys/kernel/debug/mt6582_btif/bringup)
  zz-sshd.start          BUCLE DE REINTENTO de sshd (clave del "regulero")
  zzy-charge.start       arranca fan5405-charge
  zzz-gps.start          bridge stpgps→gps0 + gpsd + socat
  zzz-touch-input.start  insmod uinput + reset + daemon ft5336
  zzzz-phosh.start       autoarranque Phosh (incl. chmod u-s /usr/bin/bwrap)
  zzzz-startx.start.off  X11 DESACTIVADO
/usr/local/bin/         battery, bl, charge-status, devmem, fan5405-charge, ft5336_touch,
                        mt6582-backlight.py, mt6582-bl-shim.py, mtk-gps-bridge, pwrap_poke,
                        phosh-session(.sh), launch_phosh.sh   ← TODO el HAL vive en userspace
/etc/init.d/           mt6582-backlight (OpenRC del daemon de brillo)
/etc/fstab             /dev/mmcblk1p1 / ext4 rw,relatime 0 1   ← root en la SD
lsmod                  solo uinput (todo lo demás built-in)
```

Estado vivo: DRM card1 mediatek + card0 lima + renderD128; Phosh (phoc 1092 + phosh 1313 + squeekboard 1603) bajo usuario **sxmo** (uid 1000); BT `hci0 UP RUNNING` BD `00:00:46:65:82:01` pero **RX/TX=0**; **sin `wlanX`**, **`/sys/bus/iio/devices` vacío**, **`/sys/class/power_supply` vacío**, **sin tarjeta ALSA**; `/dev/stpgps` presente; `sshd` PID 797 escuchando (OpenRC dice "crashed", cosmético).

### 2.6 Repo GitHub — `Sidihidi/bq-aquiaris-postmarketos` (privado, ~5.7M, push 2026-06-21 10:32)

```
[raíz]              README, README-PMOS-3.10, INDICE-GENERAL, PLAN-MAESTRO-IMPLEMENTACION,
                    ROADMAP-DRIVERS, ESTRATEGIA-CUSTOM-BOOTLOADER, GUIA-MTKCLIENT, .gitignore
aports/             device-bq-krillin/{APKBUILD,deviceinfo}, linux-bq-krillin/{APKBUILD,config armv7 81K}
artifacts/          stock-firmware/MT6582_Android_scatter.txt  (los .img .gitignored)
mainline/dts/       mt6582-bq-krillin.dts(13.7K), usb-node.dtsi (+variantes disp-drm/quickwins/consys)
mainline/drivers*/  mt6582-{btif,consys,dispfix,mfg-power,musb,pmic-fixup,wifi}.c, pwm-mtk-disp.c,
                    gpio-mt6582-eint.c, ft5336_touch.c, panel-himax-hx8389.c, mtk-mmsys.c, rdma,
                    mtk-gps-bridge.c, mt6582-wifi-reg.h
mainline/patches/   0001-mmc-pinctrl-optional, 0002/0003-pmic-wrap-mt6582, 0004-mfd-mt6397-irq-opt,
                    add-mt6582-i2c-compat.py
mainline/docs/+*.md ~40 HITO-* + ESTRATEGIA-DRIVERS + DISP-DRM-* (mapeo-completo.json 114K) + SECCFG_NEXT
mainline/userspace/ etc/{conf.d/sshd, init.d/{gpsd,mt6582-backlight}, local.d/*.start},
                    usr/local/bin/{bl, mt6582-backlight.py, mt6582-bl-shim.py,
                    mtk-gps-bridge(480K ARM), phosh-session.sh, launch_phosh.sh}
mainline/rootfs/    copia OpenRC (battery, bluetooth, charge/fan5405, sshd, touch, x11, phosh)
mainline/phosh/     usrlocal.tar.gz(4.9M, binario mayor del repo), apk-installed.txt(52K), gschema
mainline/init/+pkg/ initramfs (init-ssh, init-switchroot, tiny_init_*) + assemble/flash/mtk_hdr
mainline/wifi-consys/ m3a/captura/stock-consys-bringup.txt(110K traza OEM), WIFI-{DESIGN,ROADMAP},
                    GPS-CONSYS, HITO-WIFI-M3A(36K), SECUENCIA-ARRANQUE-CONSYS, wifi-{bringup,check,iter}.sh
mainline/modules/   uinput.ko(74K)
mainline/           config-mainline-krillin(290K), dmesg-mainline-v{3,5,6,7}.txt
legacy/             docs-superseded + ~50 .sh tempranos
scripts/            capa0-backup.sh
```

**`.gitignore` excluye:** `*.img`, `artifacts/*.img`, `*.zip`, `*.bin`, `.claude/`. Backup de **código/config/conocimiento = excelente**; backup de **datos de hardware irreemplazables = ausente** (los `golden/*.img` NO están en GitHub).

---

## 3. ESTADO REAL POR SUBSISTEMA

Para cada uno: (a) Android/OEM, (b) mainline (cómo funciona técnicamente), (c) qué falta, (d) dónde está documentado.

### 3.1 Display — ✅ FUNCIONA

**(a) Android/OEM:** el LK (bootloader KitKat) inicializa el panel **HX8389** por DSI vídeo y enciende VGP2 (LCD) + backlight; el kernel 3.10 usa `mtkfb` (downstream DISP `ddp_*`). cmdline `lcm=1-hx8389_qhd_dsi_vdo_truly`.

**(b) mainline:** **DRM nativo `mtk_drm`** (simplefb+dispfix RETIRADOS). Pipeline enlazado en dmesg: `mmsys@14000000` + `mutex@1400e000` + `ovl@14007000` + `rdma@14008000` (variante propia `mt6582-disp-rdma`) + `color@1400b000` + `dsi@1400c000` + `mipi_tx0@10010000` + panel `truly,hx8389-qhd` (reset GPIO112, power VGP2). `card1` driver mediatek-drm, conector `card1-DSI-1 connected`, `fb0 mediatekdrmfb`, modo **U:540x960p**, 32bpp. El port se hizo tomando **MT2701 como gemelo** (OVL/COLOR/MUTEX reusados; mmsys/RDMA/BLS = parches de pocas líneas: `RDMA MEM_START_ADDR 0x0f00→0x28`, `BLS_DEBUG=0`). Bases DISP: MMSYS 0x14000000, OVL 0x14007000, RDMA 0x14008000, BLS 0x1400A000, COLOR 0x1400B000, DSI 0x1400C000, MUTEX 0x1400E000. IRQ DISP: OVL=153, RDMA=152, COLOR=156, DSI=157, MUTEX=161 (SPI raw-32, LEVEL_LOW). Avisos benignos: `mipi-tx can't get nvmem_cell_get, ignore it` y ciclo de dependencia DSI↔panel resuelto.

**Histórico clave:** el tinte amarillo (M3) **no era gamma/panel**, era **FORMATO** (simplefb a8r8g8b8 vs OVL leyendo RGB565 → se perdía el azul). Fix intermedio: DT `format=r5g6b5; stride=1088` + reprogramar OVL clonando la config DISP leída en vivo de pmOS 3.10 (`OVL SRC_CON=0x8`, `MUTEX MOD=0x680/SOF=1/EN=0x303` = el commit real). Luego se sustituyó por mtk_drm completo.

**(c) Falta:** nada funcional. Cerrado.

**(d) Documentado:** `reference_mt6582_disp_tint.md`, `reference_mt6582_disp_drm_roadmap.md`; HITOs `HITO-M3-DISPLAY-RESUELTO.md`, `DISP-DRM-{ROADMAP,componentes,CONTINUACION}.md`, `DISP-DRM-mapeo-completo.json`. Fuentes: `disp-research/`, `research_mt6582_disp/`, `research_mt6582_lk/`, `mainline/disp-drm/code/`.

### 3.2 GPU (lima / Mali-400 MP2) — ✅ FUNCIONA

**(a) Android/OEM:** blob propietario Mali en Android 4.4/5/6.

**(b) mainline:** **`lima`** (`card0`), Mali-400 MP2 (gp + pp0 + pp1, L2 64K), `[drm] Initialized lima 1.1.0`, `renderD128` → **Phosh acelerado por GPU**. Nodo `gpu@13010000` (`arm,mali-400`, 6 IRQ). El bloqueante era encender el **power-domain MFG** (no hay scpsys mainline para MT6582): se resolvió con **`mt6582-mfg-power.c`** (`__init` que poke directo a **SPM 0x10006000, MFG_PWR_CON 0x214** MTCMOS antes del probe de lima, idempotente). `CONFIG_DRM_LIMA=y`.

**(c) Falta:** nada. (Antes de lima, weston/Phosh ya iban fluidos por software/Pixman, por eso la memoria marcaba lima como "no prioritario".)

**(d) Documentado:** `HITO-LIMA-MFG.md`, `HITO-WAYLAND.md`; driver `mt6582-mfg-power.c` (Pi + repo).

### 3.3 Táctil (FT5336) — ✅ FUNCIONA (e2e)

**(a) Android/OEM:** driver kernel `ft5x06`/`focaltech` por I2C0 @0x38, IRQ por EINT117; alimentado por **VGP1** del MT6323. El LK enciende VGP2 (LCD) pero **no VGP1**.

**(b) mainline:** **I2C funciona** con compat `mt6582-i2c` y **`auto_restart=1`** (el FT5x06 necesita repeated-START real). **Power VGP1** automático al boot vía servicio OpenRC `touch-power.start` (pwrap_poke reg **0x050A bit15** = MT6323 DIGLDO_CON7 + reset GPIO115). El driver real es **userspace `ft5336_touch`** (PID 917): lee i2c (≤8B = single-touch, FIFO=8) y emite por **uinput** (`/dev/input/event1`, `Name=ft5336 Vendor=005a Product=0014`, multitouch tipo B). `uinput.ko` se insmoda sin reflashear (`zzz-touch-input.start`). Existe también driver kernel `gpio-mt6582-eint.c` (169 pines, GIC_SPI113, EINT117 dispara IRQ) y `CONFIG_TOUCHSCREEN_EDT_FT5X06=y`, pero el camino productivo hoy es el daemon userspace. GPIO_BASE 0x10005000, PWRAP_BASE 0x1000D000 (WACS2_CMD +0x9C, RDATA +0xA0), CID MT6323=0x2023.

**(c) Falta (mejora, no bloqueante):** migrar a driver kernel `edt-ft5x06` con IRQ EINT (táctil por interrupción en vez de polling) y multitouch completo (>8B necesita APDMA, ver 3.x i2c). El wake/reset GPIO115 ya es fiable.

**(d) Documentado:** `reference_mt6582_touch.md`; HITOs `HITO-I2C-TOUCH.md`, `HITO-TOUCH-POWER-SERVICE.md`, `HITO-TOUCH-DRIVER.md`, `HITO-EINT-RESUELTO.md`, `CONTINUAR-TOUCH.md`.

### 3.4 Almacenamiento (eMMC + SD) — ✅ FUNCIONA

**(a) Android/OEM:** `mtk-msdc` sobre eMMC; layout Android (blkdevparts: p5=android/system 1G, p7=usrdata 5.6G).

**(b) mainline:** **`mtk-sd`** con dos controladores: **mmc0** (eMMC `mt8135-mmc`, no-removable) + **mmc1** (ranura **SD**, power VMCH/VMC del MT6323, nodo `mmc1@11240000`). **root real en la SD: `/dev/mmcblk1p1` (30G ext4, 7% uso)** → **dual-boot**: pmOS arranca de la SD, el Android interno (`mmcblk0`: p5 android, p7 usrdata) queda intacto. msdc bindea sin pinctrl ("relying on bootloader pin setup", normal). El bloqueante original era **`mtk-sd` leyendo CEROS por DMA** en la eMMC (M3 mainline), ya resuelto. `CONFIG_MMC_MTK=y`.

**(c) Falta:** nada.

**(d) Documentado:** `reference_mt6582_musb.md` (mapa particiones), `CONTINUAR_MAINLINE.md` (bug DMA ceros), `HITO-ROOTFS-ALPINE.md`, `CATCHUP_DUAL-BOOT-SD.md`, `DUAL-BOOT-ANALISIS.md`; parche `0001-mmc-mtk-sd-make-pinctrl-optional.patch`.

### 3.5 Red USB + SSH — ✅ FUNCIONA (con muleta cosmética)

**(a) Android/OEM:** MUSB en modo dispositivo, adb/RNDIS.

**(b) mainline:** driver **`mt6582-musb.c`** (gadget peripheral). g_ether (RNDIS/CDC) enumera; **usb0 = 172.16.42.1/24**, ruta por 172.16.42.2 (la Pi). 3 bugs resueltos: **L1INTM=0x7** (no 0x1ff, tormenta de IRQ), PHY pull-downs vía `usb_phy_recover()` completo, usb0 por ioctl. MAC 0x11200000, SIF/PHY 0x11210000, IRQ GIC_SPI 32; regs L1: USB_L1INTS=0xa0, L1INTM=0xa4 (TX=b0/RX=b1/USBCOM=b2). **NO tocar clocks** (rompe MSDC). SSH = OpenSSH (antes busybox+dropbear estáticos en initramfs); **sshd PID 797 escuchando** (estoy conectado por él). `/etc/conf.d/sshd` tiene `rc_need="!net"`.

**(c) Falta:** que OpenRC deje de marcar `sshd [crashed]` — es **cosmético** (el bucle de `zz-sshd.start` lanza sshd fuera del pidfile de OpenRC, así que pierde el rastro del pid). El acceso funciona.

**(d) Documentado:** `reference_mt6582_musb.md`, `reference_mt6582_boot_stability.md`; HITOs `HITO-M2b-USB-RESUELTO.md`, `HITO-M2d-SSH-RESUELTO.md`.

### 3.6 Boot / estabilidad — 🟡 PARCIAL (funciona por muleta)

**(a) Android/OEM:** preloader → LK → kernel. LK **KitKat 1.5.2 = permisivo** (no verifica firma); LK **Lollipop = verificador** → bootloop con kernels propios. **GOLDEN RULE.** `fastboot oem unlock` es persistente y suficiente (battery IN para fastboot). LK ignora cmdline → bootargs por DT.

**(b) mainline:** boot SMP 4×A7, initramfs (`root=/dev/ram`) que pivota a `mmcblk1p1`, `panic=15`, ramoops en `0xbf300000`. Estabilización (2026-06-21): cuelgue intermitente (~1/4, gadget enumeraba pero `usb0 carrier=0`, requería power-cycle) → **seedrng (sysinit+boot) + haveged + quitar avahi → 7/7 reinicios limpios (14-19s)**. La entropía (crng 236s) era **síntoma**; el cuelgue real es en el runlevel `boot` de OpenRC (~8s, sospechoso udev/udev-settle, **no confirmado**). Regresión "regulero": boot llega a Phosh pero sshd no arranca por timing temprano → **bucle de reintento en background en `zz-sshd.start`** (hasta 12 intentos). `localmount=started` ahora (la cascada que tumbaba dbus/bt/haveged **no está activa**). Error de boot benigno: `mt6577-uart 11002000.serial probe failed -2` (earlycon ya tomó el UART) y "clock skew" (sin RTC).

**(c) Falta:** diagnosticar la **causa raíz** del cuelgue del runlevel `boot` (udev-settle) y la regresión de sshd, para eliminar la muleta del bucle. Captura para recaídas: `/sys/fs/pstore/console-ramoops-0` + `rc_log_path=/dev/kmsg`.

**(d) Documentado:** `reference_mt6582_boot_stability.md`, `project_pmos_bq_e45.md` (GOLDEN RULE, bootloader), `reference_mt6582_flash_frompmos.md` (sector 83968), `SECCFG_NEXT.md` (muro del unlock persistente seccfg).

### 3.7 Backlight / brillo — ✅ FUNCIONA

**(a) Android/OEM:** el LK enciende el backlight; el kernel 3.10 lo controla por el bloque DISP-PWM/BLS.

**(b) mainline:** **HARDWARE 100% resuelto** — el brillo se controla por **PWM_DUTY @0x1400a000+0xa0 (10-bit, max 0x3ff)**, NO el **+0xac** que escribía erróneamente `pwm-mtk-disp` (mt2701), razón por la que el slider "no hacía nada". Hoy lo gobierna un **daemon userspace `mt6582-backlight.py`** (PID 742, OpenRC) que lee `/run/mt6582-bl-pct` y escribe `+0xa0`; verificado vivo: `+0xa0 = 0x332` (818/1023 ≈ 80%) coincide con el 80% pedido. CLI **`bl 0-100`** funciona. Existe además **`mt6582-bl-shim.py`** que expone `org.gnome.SettingsDaemon.Power.Screen` en el bus de sesión para que **el slider de Phosh** escriba el porcentaje (puente D-Bus→fifo→registro). `BLS_EN @+0x00 = 0x10001`. El DT también declara `pwm-backlight` (11 niveles, `brightness=8/10`). Phosh intenta su ruta nativa y falla limpiamente (`Setting backlight on DSI-1 failed: EISDIR`), inofensivo pero ensucia el log.

**(c) Falta (limpieza):** que el **driver kernel `pwm-mtk-disp` escriba +0xa0** (en lugar del daemon userspace) y exponga `/sys/class/backlight` real, para que el slider de Phosh sea nativo. Variante `mediatek,mt6582-disp-pwm` "SIN BLS_DEBUG" ya en árbol.

**(d) Documentado:** `reference_mt6582_backlight.md`; HITO `HITO-QUICKWINS.md` (diagnóstico inicial).

### 3.8 Pila de energía (D-Bus / polkit / UPower / elogind / seatd) — ✅ FUNCIONA (UPower parcial)

**(a) Android/OEM:** N/A (gestión Android propia).

**(b) mainline:** todos vivos — `dbus-daemon --system` (702), **polkit** (762/763), **seatd** (970), **elogind-daemon** (1425), **upowerd** (1411). Ping D-Bus a **login1** y a **PolicyKit1** → ambos responden `method return`. **Caveat:** `loginctl` = "No sessions" porque **Phosh se lanza por `su - sxmo` + nohup, no por login1/PAM** → la sesión gráfica no se registra ante login1/elogind (funciona, pero la integración de lock/idle/energía queda fuera del loop). El fix `bwrap` (Phosh moría con SIGABRT en glycin/bwrap setuid) = **`chmod u-s /usr/bin/bwrap`** (user namespaces), persistido en `zzzz-phosh.start`.

**(c) Falta:** registrar la sesión Phosh ante login1 (sesión elogind/PAM real) — históricamente **bancado por el usuario** por riesgo de romper la GUI. Sin esto, gsd-power y el lock no se integran. Ligado a batería→UPower (3.13).

**(d) Documentado:** `reference_phosh_bwrap_fix.md`, `reference_mt6582_backlight.md` (la sesión elogind/PAM bancada), `HITO-PHOSH-ARRANCA.md`.

### 3.9 Bluetooth — 🔴 ROTO a nivel RF

**(a) Android/OEM:** combo **CONSYS @0x18070000** (WiFi+BT+GPS+FM). BT por capas **BTIF → STP → WMT → radio**; nodo Android `/dev/stpbt`. Firmware/patch del stock (`mt6572_82_patch_e1_{0,1}`, `WMT_SOC.cfg`); la captura OEM en KitKat rooteado confirma la secuencia.

**(b) mainline:** driver **`mt6582-btif.c`** (BTIF DMA base **0x1100c000**, APDMA TX **0x11000780**/RX **0x11000800**) que registra **`hci0` real** para BlueZ. Bring-up por `zz-consys-bt.start` (`echo a /sys/kernel/debug/mt6582_btif/bringup`); CONSYS encendido por `mt6582-consys.c` (VCN18/28/33 + MTCMOS SPM `CONN_PWR_CON 0x280`, chip-id `0x18070008` = 0x6582). Índices STP: BT=0 FM=1 GPS=2 WIFI=3 WMT=4. **VERIFICADO:** `hci0 UP RUNNING PSCAN ISCAN`, `bluetoothctl`: **Powered/Discoverable/Pairable yes**, nombre `krillin-bq`, BlueZ 5.86. dmesg: "CANAL OK: CONSYS responde GEN_HCR". El bug que bloqueaba el bring-up (bootloop) se resolvió: era **tormenta de IRQ no clock** (bus vivo LSR=0x60) + faltaban **EMI compartida + OSC 26M (AP2CONN_OSC_EN)** para que el MCU ejecute + **fix 1-byte del header STP (canal WMT=4 → h[1]=0x40, no 0x00=BT)** + el TX-DMA no expulsaba cola parcial <8B sin FLUSH.

**PERO el RF no radia:** `hciconfig` muestra **RX bytes:0 / TX bytes:0** (acl/sco=0), `Discovering: no`. Solo hay tráfico de **control local** (events:59/commands:56). Aunque BlueZ se anuncie discoverable, **ningún dispositivo externo lo ve**. **MAC placeholder `00:00:46:65:82:01`** (derivada del chip-id 6582, no de NVRAM real). El "S24 Ultra detectado" del 06-19 indica que en ese momento sí cursó inquiry; ahora el HCI está arriba pero el contador de bytes en cero apunta a la **capa física CONSYS-BT que no cursa tráfico por aire**, no a BlueZ ni a la config.

**(c) Falta:** (1) hacer que el **RF radie de verdad** — investigar por qué la PHY/MAC del BT no transmite (¿calibración RF de NVRAM? ¿secuencia de habilitación de antena/PA? ¿el mismo patrón de init incompleto que en WiFi?); (2) inyectar una **BD_ADDR real** (de `golden/nvram.img` o vía SN Write Tool) para identidad estable.

**(d) Documentado:** `reference_mt6582_wifi_consys.md`; HITOs `HITO-WIFI-CONSYS.md`, `HITO-WIFI-M3A.md`, `CATCHUP_HITO-WIFI-M3A.md` (BT funcionando), `SECUENCIA-ARRANQUE-CONSYS.md`, `CATCHUP_README.md` (setup BT userspace), `CATCHUP_PLAN-B-CAPTURA-ANDROID.md`. El **detalle "RX/TX=0, RF no radia, MAC placeholder" está documentado por primera vez en este informe** (la auditoría del teléfono); las memorias decían "BT funciona" sin capturar la regresión RF.

### 3.10 WiFi — 🔴 ROTO (bring-up incompleto)

**(a) Android/OEM:** el **data-path WiFi NO va por BTIF/STP** sino por un bloque **HIF AHB @0x180F0000** (regs estilo-SDIO WCIR/WHLPCR/WTDR0/WRDR0) + **PDMA @0x11000180**; STP/WMT solo para `func_on(WIFI)`+reset. Driver vendor full-MAC (el firmware hace la FSM 802.11). Firmware correcto = **`WIFI_RAM_CODE_MT6582` (160480B)**. La captura OEM confirma que el OEM usa **exactamente nuestro firmware + patches** y su WiFi funciona.

**(b) mainline:** driver **`mt6582-wifi.c` (SCAFFOLD Fase 0)** + `mt6582-wifi-reg.h`. Secuencia: probe → `consys_ready` → `func_on(WIFI)` → leer WCIR (chip-id 0x6582) → descargar `WIFI_RAM_CODE` por WTDR0 → WIFI_START → poll WLAN_READY. Vivo: CONSYS ok, regulador **`vcn33_wifi` a 3.3V**, HIF mapeado, **FW se descarga** — pero se queda en *"Fase 0: cfg80211 = TODO"* y **`WLAN_READY` (WCIR bit21) NUNCA se afirma** (la MAC del WiFi no arranca, `mbox D2HRM0R=0`). **No hay interfaz `wlanX`** (`/sys/class/net`: solo lo/sit0/usb0), sin cfg80211/mac80211. Regs: HIF_DRV_BASE 0x180F0000, WCIR=0x00, WHLPCR=0x04, WHIER=0x14 (default 0xffffff0f), D2HRM0R=0x40, HSTCR(MCR=0x58)=avanza FIFO.

**Descartado en HW:** firmware, patch, chip-id, VCN33 enable + modo-HW (`ANALDO_CON17=0xd000` verificado). **FIX#1 (timing VCN33: ciclar off→on dentro de HifAhbProbe)** y **FIX#2 (modo-HW bit14)** probados y **descartados**. **Sospechoso restante:** los pasos de init que el scaffold **SALTA** respecto a `wlanAdapterStart` del OEM — **`nicTxInitResetResource` / `nicTxInitialize` / `nicRxInitialize`**.

**(c) Falta:** portar los pasos de init de TX/RX que faltan (`nicTx/RxInitialize`) para que el MAC afirme `WLAN_READY`; luego Fase 1 (cmd/event+scan), Fase 2 (connect/WPA2), Fase 3 (data path PDMA+IRQ). Roadmap estimado ~17-27 sesiones, sin muro conocido. Desplegar el blob como `mediatek/mt6582/WIFI_RAM_CODE` (renombrar `WIFI_RAM_CODE_MT6582`).

**(d) Documentado:** `reference_mt6582_wifi_hif.md` (la más densa del bloque), `reference_mt6582_wifi_consys.md`; `wifi-work/{BRIEF,WIFI-DESIGN,WIFI-ROADMAP}.md`; `mainline/wifi-consys/`. Fuentes vendor: `wifi-work/downstream-wlan/`, `_connsoc_wlan/`.

### 3.11 GPS — 🟡 PARCIAL (infra viva, sin motor)

**(a) Android/OEM:** GPS por CONSYS **canal STP 2** → `/dev/stpgps` (tubo crudo). Todo el protocolo **MT3332 vive en el daemon Android `mnld`**: binario MTK **0xAAF0** (sync `AA F0` + len + tipo + payload) + NMEA out + socket PMTK/AGPS. Captura Lineage-13 (strace -p, posición real Murcia). Power downstream `/sys/class/gpsdrv/gps/{pwrctl,state}`. Fix Lineage = `mnld` real + `location_providers_allowed +gps` (`99gpsallow`). Stack extraído en `~/gps-fm-extract/` (`mtk_agpsd`, `libmnl.so 7.3M`, `6620_launcher`).

**(b) mainline:** **`/dev/stpgps` existe** (func_on(GPS) ok, BT intacto). Cadena userspace **toda arriba**: **`mtk-gps-bridge`** (PID 875, expone pty `/dev/gps0`) → **`gpsd -N -n /dev/gps0`** (896) → **`socat`** exponiendo `/var/run/gps-share.sock` (908). gpsd responde JSON (VERSION/DEVICES/WATCH, 38400). Validado con NMEA enlatada (`where-am-i` dio lat/lon). **PERO** `cat /dev/gps0` y `gpspipe -r` **no devuelven NMEA real** — solo el handshake de gpsd, **sin sentencias de posición**: falta el motor que hable **0xAAF0** para arrancar el chip. geoclue **no corre**.

**(c) Falta:** el **motor 0xAAF0** — portar/correr `mnld` (o reimplementar la `START_SEQ` ya capturada) que genere los comandos binarios MTK; arrancar geoclue. Trazas de referencia en `mnld-{init,live}.strace`.

**(d) Documentado:** `reference_mt6582_gps.md` (START_SEQ capturado); `GPS-CONSYS.md`, `mainline/wifi-consys/gps/mtk-gps-bridge.c`.

### 3.12 Sensores / autorrotación — 🔴 NO EMPEZADO

**(a) Android/OEM:** chips reales (del defconfig downstream + cust): **accel ST LSM330** i2c0@0x1D (WAI 0x40), **gyro L3GD20** @0x6B, **mag MEMSIC MMC3516X** @0x30, **ALS/prox AMS TMD2772** @0x39. Drivers Android propietarios + HAL.

**(b) mainline:** **NADA instanciado.** `/sys/bus/iio/devices/` está **VACÍO**; ningún driver de sensor cargado (dmesg solo `i2c_dev`); `iio-sensor-proxy` no corre. Sin accel → **sin autorrotación ni brillo automático**. Pese a que **todos los chips tienen driver mainline** (`st_accel` `st,lsm330-accel`, `st_gyro`, `tsl2772` `amstaos,tmd2772`; mag `mmc35240` PARCIAL/regmap≠) y la memoria deja Kconfig+nodos DT listos para pegar, **el DT del kernel #92 no los declara**.

**(c) Falta:** añadir los nodos hijos de `&i2c0` (SIN `interrupts` → polling; el `st_accel` acepta `if(irq>0)`), Kconfig de los drivers IIO, `mount-matrix dir7 = 0,-1,0, -1,0,0, 0,0,-1`, y `apk add iio-sensor-proxy` (3.9-r2 ya en Alpine 3.24 armhf). **Depende del APDMA i2c** (lecturas 6B≤8B funcionan; `dma_sync=1` lo robustece). Accel = ALTA prioridad para Phosh. Es un **quick-win** según el roadmap, **todavía sin hacer**.

**(d) Documentado:** `reference_mt6582_sensors_rotation.md` (muy completo), `reference_mt6582_drivers_roadmap.md`. **No hay HITO** porque no se ha ejecutado.

### 3.13 Batería (UPower) — 🔴 ROTO a la UI

**(a) Android/OEM:** fuel-gauge por AUXADC del MT6323 + driver `power_supply` Android.

**(b) mainline:** **`/sys/class/power_supply/` está VACÍO** — no hay driver `power_supply` (ni fuel-gauge ni charger expuesto al kernel); no hay nodo de batería en el DT (`mt6323-rtc/led/keys/pwrc` fallan con "Failed to locate of_node"). Por eso `upower -d` solo tiene `DisplayDevice` sintético → `percentage: 0%`, `icon-name battery-missing-symbolic`; como `upowerd` corre, **Phosh muestra el icono pero a 0%/missing** (log: `on_battery_clicked: assertion 'group' failed`). **El nivel real SÍ se lee** pero solo por userspace fuera de UPower: el script **`/usr/local/bin/battery`** (AUXADC canal 7 del MT6323, BATSNS `ADC0 0x0714`, vía `pwrap_poke`) devolvió en vivo **VBAT 3997 mV (~79%)**. Ese dato **nunca llega a UPower**.

**(c) Falta:** un **driver/shim que cree `/sys/class/power_supply/*`** (driver kernel fuel-gauge MT6323, o un backend custom de UPower / un daemon que escriba un `power_supply` por uevent). Esto exige el nodo en DT. Curva OCV→% real pendiente. Es **quick-win #4** del roadmap (UPower exige el nodo creado por el KERNEL; validación previa con `CONFIG_TEST_POWER_SUPPLY`).

**(d) Documentado:** `reference_mt6582_drivers_roadmap.md`; HITO `HITO-BATERIA-WIP.md` (VBAT por pwrap). **El diagnóstico "icono 0% = UPower sin power_supply, VBAT real 79%" se captura en la auditoría del teléfono.**

### 3.14 Carga (FAN5405) — ✅ FUNCIONA

**(a) Android/OEM:** charger FAN5405 gestionado por el kernel/HAL Android.

**(b) mainline:** **daemon userspace `fan5405-charge`** (PID 866, I2C0 @0x6a, lanzado por `zzy-charge.start`) re-escribe **CV 4.2V (OREG=35) / IINLIM 800mA (=2)** cada 10s y **patea el watchdog** (~22s) para que no revierta a 100mA. Vivo: `charge-status` → **STAT=CARGANDO**, CON0=0xd0/CON1=0xb8/CON2=0x8c. El teléfono **carga** mientras corre mainline (motivado por el HITO-GUI-X11, donde se quedó sin batería 2 veces).

**(c) Falta:** reflejar el estado en UPower (ligado a 3.13). Idealmente, driver kernel `power_supply` para el charger.

**(d) Documentado:** `HITO-CARGA-FAN5405.md`.

### 3.15 PMIC MT6323 — ✅ FUNCIONA (base de varios subsistemas)

**(a) Android/OEM:** PMIC MT6323 vía pwrap; reguladores VGP1/VGP2/VCN/VMC.

**(b) mainline:** pwrap **`mtk-pmic-wrap`** (soporte mt6582, caps=0) + MFD **`mt6397`** + **31 reguladores** (`REGULATOR_MT6323=y`, `MFD_MT6397=y`): VGP1@2.8V (táctil), VGP2 (LCD), VCN18/28/**VCN33_WIFI**, VMCH/VMC (SD). Chip ID **0x2023**. 3 fixes de driver. `mt6582-pmic-fixup.c` (encendía VGP1 por late_initcall) **reemplazado por el regulador VGP1 `always-on` del DTS** (ya no se compila). pwrap por DT con `skip-init/no-reset` y `no-irq` (parches 0002/0003); `mfd-mt6397 irq optional` (parche 0004). **Lección de oro mtkclient: `wo` NUNCA `wf` (wf escribe en sector 0).**

**(c) Falta:** exponer subdispositivos del MT6323 que el DT no declara: **PWRKEY (power-button), fuel-gauge/charger (power_supply), RTC, audio codec** — cada uno desbloquea un subsistema (3.13, 3.16, suspend).

**(d) Documentado:** `HITO-PMIC-MT6323.md`; parches `0002/0003/0004`.

### 3.16 Audio — 🔴 NO EMPEZADO

**(a) Android/OEM:** AFE MediaTek + codec del MT6323 (ASoC downstream).

**(b) mainline:** **NADA.** `aplay -l` → "no soundcards found", `/proc/asound/` **no existe**, dmesg sin `asoc/afe/codec/i2s/snd`. `pulseaudio` (PID 1400) corre **sin sink**; `pactl` ni instalado. El AFE/MT6323-audio **no está en el DT**.

**(c) Falta:** todo — nodo ASoC/AFE + codec MT6323 en DT y driver. No hay base de partida.

**(d) Documentado:** **NO DOCUMENTADO** (ninguna memoria ni HITO cubre audio). Hueco total.

### 3.17 Modem (3G / CCCI) — 🔴 FUERA DE ALCANCE

**(a) Android/OEM:** stack **CCCI/CCMNI** (md1img), RIL propietario.

**(b) mainline:** no abordado. Decisión arquitectónica: **fuera de alcance** del port mainline puro → vía **Halium sobre el kernel 3.10** si se quisiera telefonía. Dump defensivo de `md1img` recomendado (Fase 0) pero **no consta hecho**.

**(c) Falta:** todo; estrategia Halium no iniciada.

**(d) Documentado:** `reference_mt6582_drivers_roadmap.md` (lo marca fuera de alcance), `project_pmos_bq_e45.md` (modem=CCCI).

### 3.18 Suspend (S2R) — 🔴 NO EMPEZADO

**(a) Android/OEM:** suspend-to-RAM profundo con SPM.

**(b) mainline:** no abordado. Decisión: **deep S2R descartado → s2idle**. **Requiere primero el power-button** (EINT = único wakeup de s2idle).

**(c) Falta:** power-button (MT6323 PWRKEY → EINT25) y luego s2idle.

**(d) Documentado:** `reference_mt6582_drivers_roadmap.md` (s2idle, orden power-btn→suspend).

### 3.19 Track A — pmOS sobre kernel 3.10 — ✅ FUNCIONA (limitado, congelado)

**(a)=(b):** kernel **LineageOS 3.10.107** (`Pablito2020/android_kernel_bq_krillin` @lineage-15.0, commit a1ad4ab, forzado gcc6, parche `linux3.4-ARM-8933-1`). OpenRC (systemd NO funciona en 3.10). Funcional probado en HW: consola, SSH-USB, **Sxmo X11**, táctil (tipo-A→evdev), **WiFi** (downstream). Receta de build, flasheo por `nc`/debug-shell, lecciones (ext4 features, mtkfb decouple) en `README-PMOS-3.10.md`.

**(c) Falta / techo:** **sin GPU** (Mali+lima exige kernel ≥5.2). Es el camino para **modem vía Halium** si se quisiera. Hoy **en segundo plano** frente a mainline.

**(d) Documentado:** `README.md`/`README-PMOS-3.10.md`, aports `device-bq-krillin`/`linux-bq-krillin`, `CONTINUAR_AQUI.md`.

---

## 4. DOCUMENTADO vs NO DOCUMENTADO

**Excepcionalmente bien documentado (memoria + ≈40 HITOs + roadmaps, todo respaldado en GitHub):**
- Display (tinte + mtk_drm completo, mapeo registro-a-registro), GPU/lima, táctil (i2c+power+EINT+driver), USB+SSH, almacenamiento+dual-boot, PMIC MT6323, backlight (registro +0xa0 exacto), carga FAN5405, boot/estabilidad, Phosh (arranque, bwrap, OSK), bloqueador del bootloader (GOLDEN RULE, seccfg), flasheo (sector 83968).
- Conectividad: BT/CONSYS bring-up (capas, fix 1-byte, secuencia OEM capturada), WiFi HIF (diseño full-MAC, regs, FIX descartados, sospechoso `nicTx/RxInitialize`), GPS (protocolo 0xAAF0, START_SEQ capturada, cadena userspace).
- Sensores (chips reales, drivers mainline, DT/Kconfig listos) y batería→UPower (mecanismo, VBAT por AUXADC) **documentados como plan, no como hecho**.

**Huecos de documentación (capturados aquí por primera vez o ausentes):**
- **Audio: SIN documentar** — ninguna memoria/HITO. Hueco total.
- **Regresión RF de Bluetooth** (HCI up pero **RX/TX=0**, MAC placeholder): las memorias dicen "BT funciona"; el **detalle de que el RF no radia no estaba capturado** hasta la auditoría del teléfono.
- **Batería a la UI**: que el icono marca **0% por falta de `power_supply` kernel** mientras el VBAT real es 79% — diagnóstico nuevo de la auditoría.
- **Modem**: solo marcado "fuera de alcance"; sin dump `md1img` ni plan Halium concreto.
- **Suspend**: solo mencionado (s2idle); sin trabajo.
- **`power-button` (PWRKEY→EINT25)**: planificado (quick-win) pero sin HITO de ejecución.
- **Desincronizaciones de fuentes**: (1) `~/wifi-work` no existe en la Pi (es `~/wifi-fw`+`~/gps-fm-extract`); (2) `flash_boot_dd*.sh` apuntan a offset 0x1D80000 (zeroed) en vez de 83968; (3) `mainline/` local casi vacío vs el repo (checkout desincronizado); (4) el kernel en la Pi **no está bajo git** (solo `.bak-*` + backups GitHub); (5) `reference_eboot_diff.md` en disco pero no en el índice (proyecto PES, irrelevante aquí).

**Riesgo de respaldo (crítico):** `golden/{nvram,proinfo,seccfg,lk,boot,recovery}.img` **NO están en GitHub** (`.gitignore` excluye `*.img`/`*.bin`). Son los datos que el hardware **no regenera** (calibración RF, IMEI, secure-boot). El comentario del `.gitignore` ("se regeneran con la doc") es **falso** para estos. Además, **`push_m2b.py` tiene el PAT de GitHub en claro**.

---

## 5. TODOs PRIORIZADOS POR SUBSISTEMA

**P0 — Respaldo y seguridad (antes de seguir tocando HW):**
1. Subir `golden/*.img` (nvram, proinfo, seccfg, lk, boot, recovery, protect_f/s) a **almacenamiento externo/cifrado** (Git LFS o release privado), NO a git normal. Son irreemplazables.
2. **Rotar/eliminar el PAT** en claro de `C:\Users\jferr\Desktop\pmos-krillin\push_m2b.py`.
3. Poner el **kernel de la Pi bajo git** (hoy solo `.bak-*`).
4. Corregir `flash_boot_dd*.sh` para usar el sector **83968** (no 0x1D80000) y/o documentar que el flasheo bueno va por el método del sector real.
5. Dump defensivo pendiente de `md1img` (modem) en Fase 0.

**P1 — Conectividad (estrategia del usuario: GPS/BT/WiFi al 100% primero):**
6. **WiFi:** portar `nicTxInitResetResource` / `nicTxInitialize` / `nicRxInitialize` (los pasos que el scaffold salta) → objetivo `WLAN_READY=1`; luego cmd/event+scan. Desplegar blob como `mediatek/mt6582/WIFI_RAM_CODE`.
7. **Bluetooth RF:** diagnosticar por qué **RX/TX=0** (RF no radia) — revisar calibración RF de NVRAM, secuencia PA/antena, y si comparte la misma carencia de init que WiFi; inyectar **BD_ADDR real** (de `golden/nvram.img` / SN Write Tool).
8. **GPS:** portar/correr el **motor 0xAAF0** (mnld o reimplementar la START_SEQ capturada) para emitir NMEA real; arrancar geoclue.

**P2 — Botones/menús de Phosh usable sin SSH:**
9. **power-button:** MT6323 PWRKEY → EINT25 (quick-win; habilita también el wakeup de suspend). Añadir Vol↑ (falta pull-up/pinctrl; Vol↓ ya da KEY_VOLUMEDOWN).
10. **Batería→UPower:** driver/shim `power_supply` kernel (nodo MT6323 fuel-gauge en DT) → icono real en Phosh; curva OCV→%.
11. **Backlight nativo:** que `pwm-mtk-disp` escriba **+0xa0** y exponga `/sys/class/backlight` real (eliminar el daemon userspace / el shim D-Bus).
12. **Sesión Phosh ante login1** (elogind/PAM real) para integrar lock/idle/energía — con cuidado (riesgo de romper GUI; estaba bancado).
13. Limpieza: bucle sshd → diagnosticar causa raíz del cuelgue runlevel `boot` (udev-settle); silenciar ruido de logs (backlight EISDIR, battery assertion).

**P3 — Sensores y resto:**
14. **Sensores/autorrotación:** instanciar `&i2c0` accel LSM330 (+gyro/mag/ALS) en DT (polling, sin IRQ), Kconfig IIO, mount-matrix dir7, `apk add iio-sensor-proxy`. Robustecer con **APDMA i2c `dma_sync=1`** (FIX#1).
15. **DISP-DRM Fase 2/3:** flashear la Fase 2 (OVL+RDMA+COLOR ya compilada) si aún no está integrada en #92, y consolidar DSI+HX8389 (mayormente ya en mtk_drm vivo).
16. **Audio:** investigar AFE/codec MT6323 ASoC (desde cero; sin documentación).
17. **Suspend (s2idle)** tras power-button. **Modem (Halium/3.10)** solo si se decide telefonía.

---

## 6. HOJA DE RUTA CONSOLIDADA (respeta la estrategia del usuario)

**Fase 0 — Blindaje (inmediato, antes de tocar HW):** TODOs P0 (backup `golden/*` externo, rotar PAT, kernel a git, fix offset flash, dump md1img). Sin esto, un error de flasheo puede costar la NVRAM/IMEI sin retorno.

**Fase 1 — Conectividad al 100% (prioridad declarada del usuario):**
1. **WiFi** → cerrar `WLAN_READY` (init TX/RX que faltan) → scan → connect WPA2 → data-path PDMA. Es el de mayor recorrido (~17-27 sesiones) pero sin muro conocido.
2. **Bluetooth RF** → pasar de "HCI up sin radiar" a transmitir por aire (NVRAM/PA + BD_ADDR real). Probablemente comparte raíz con WiFi (init/calibración CONSYS).
3. **GPS** → motor 0xAAF0 (mnld) → NMEA real → geoclue. Es el más cercano a cerrar (toda la infra userspace ya vive).

**Fase 2 — Phosh usable sin SSH (botones/menús):**
4. power-button (PWRKEY→EINT25) + Vol↑.
5. batería→UPower (driver power_supply) → icono real.
6. backlight nativo + slider Phosh nativo.
7. sesión login1/elogind real (con cautela).
8. limpieza de boot/sshd/logs.

**Fase 3 — Sensores y refinamiento:**
9. sensores IIO (accel→autorrotación, ALS→brillo auto) + APDMA i2c dma_sync=1.
10. consolidar DISP-DRM, investigar audio, s2idle, (modem opcional vía Halium).

**Techo realista declarado:** "PC de bolsillo Linux con GUI móvil + WiFi", no reemplazo total de smartphone (modem/audio/suspend son los últimos y algunos fuera de alcance).

---

## 7. LAGUNAS / DESCONOCIDOS (a verificar)

1. **Bluetooth RF — causa exacta del RX/TX=0:** no se ha determinado si es (i) falta de **calibración RF de NVRAM**, (ii) secuencia de habilitación de **PA/antena** ausente, o (iii) el mismo patrón de **init incompleto** que tumba el WiFi. La discrepancia entre "detectó S24 Ultra el 06-19" y "RX/TX=0 el 06-21" sugiere una **regresión** entre builds (¿el fix VCN33 / cambios consys posteriores?) — no verificado.
2. **WiFi — ¿bastan `nicTx/RxInitialize`?** Es la hipótesis principal pero **no probada**; podría haber más pasos del `wlanAdapterStart` OEM que el scaffold omite.
3. **Boot — causa raíz del cuelgue runlevel `boot`:** sospechoso `udev`/`udev-settle`, **no confirmado**; el bucle de reintento de sshd y seedrng son mitigaciones, no diagnóstico.
4. **Estado del WiFi bring-up en el arranque actual:** la auditoría del teléfono encontró que el bring-up WiFi se dispara por debugfs y **no estaba activado en el arranque #92** — no se pudo ver más allá de "descarga FW" en esta sesión.
5. **¿Está la Fase 2 del DISP-DRM (OVL+RDMA+COLOR) integrada en #92?** El DRM mtk vivo bindea OVL+RDMA+COLOR+DSI, lo que sugiere que sí; pero la memoria decía "Fase 2 compilada, no flasheada" — **conviene confirmar** que #92 incluye todo (parece que sí, pero el origen del histórico es anterior).
6. **`proinfo.img` a ceros:** el dump (golden y Pi) está **TODO CEROS**. Se asume "partición vacía de fábrica" (normal), pero **no se ha verificado** que no contuviera datos antes del incidente "Format All" que perdió IMEI/NVRAM.
7. **NVRAM de `golden/nvram.img` (5M):** tiene datos reales (desde ~4M), pero **no se ha verificado** que contenga la **calibración RF / BD_ADDR / MAC WiFi originales** utilizables — clave para arreglar BT/WiFi RF. Habría que inspeccionarla.
8. **Audio:** **cero información** — no se sabe el codec exacto, el binding ASoC ni si el AFE del MT6582 tiene soporte mainline. Todo por investigar.
9. **Modem:** sin dump `md1img`, sin evaluación de viabilidad Halium concreta para este equipo.
10. **`sshd [crashed]` vs PID vivo:** se asume cosmético (pid fuera del pidfile de OpenRC) y se confirmó el listener vivo, pero **no se auditó** si hay reinicios silenciosos del listener bajo carga.
11. **`mt_wifi.tgz` y ficheros `*-LIVE.{c,dts}`** de `wifi-work`: el repo no los incluye y **no se confirmó** que estén respaldados en otro sitio.
12. **Salud de la Pi:** raíz al **84% (4.5G libres)** — no es laguna técnica del port, pero **vigilar**: un build grande podría llenar el disco.