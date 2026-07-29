# H6h→H6o — Barrido del proxy FS del módem + experimento de control (2026-07-28)

> Continúa de `H6-*` (hasta H6g). Reconstruido a posteriori del árbol de la Pi `.123`
> (`~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c`), los parches `~/mainline/patch_*.py`
> y los scripts de flasheo — la sesión no se documentó en su momento.

## Contexto: por qué se volvió a pmOS

En casa se consiguió **shell de Darwin**, pero **no se le pueden meter datos**: el **RX del UART del
teléfono parece ser de 1.8 V** y la Pi transmite a 3.3 V, así que la entrada de teclado no entra.
Decisión: aparcar Darwin (queda con shell, solo salida) y volver a afinar pmOS.

Herramientas que quedaron de ese diagnóstico (en la Pi, `~/`):
- `uart-loopback.sh` — puentea pin 8 (TXD) ↔ pin 10 (RXD) de la Pi y valida **su propio TX** a
  921600. Sirve para separar "falla la Pi" de "falla el cable/nivel hacia el móvil".
- `uart-console.sh` — consola interactiva con `screen` sobre `/dev/ttyAMA0`; al salir reactiva sola
  la captura de fondo en `/tmp/uart.log`.

**Pendiente si se retoma Darwin**: conversor de nivel 3.3 V ↔ 1.8 V en la línea RX del móvil.
Darwin quedó en **M138** (build limpio: por el UART solo sale la consola, conservando todos los
fixes reales; M134 arregló el interworking ARM/Thumb al crear hilos y el bucle infinito de
`sleh_undef`; M135–M137 fueron sondas del receptor UART e inyección de pulsaciones sintéticas para
zanjar "¿voltaje o software?").

---

## Dónde estábamos en el módem (recordatorio)

El MD (radio) arranca MOLY, hace **HS1**, acepta el runtime y responde al **GetDrive** (`0x100e`).
Después pide ficheros de su EFS/NVRAM por el canal FS y **EXCEPCIONA** justo después de nuestra
respuesta, sin llegar a HS2.

**Ground-truth** (única respuesta real capturada del `ccci_fsd` de Lineage, `~/modem-fsd/gd-boot-full.out`):
```
REQ   c=1 len=0x24  'Z:\NVRAM\NVD_DATA'
RESP  c=2 len=0x36  'Z:\NVRAM\NVD_CORE\MT00A000'     <- ruta DISTINTA y más larga
```
O sea: el fsd real **RESUELVE** una ruta, no hace eco. Y responde en **~0.5 ms**.

---

## Lo que se hizo el 28-jul (kernels #32 → #37)

Todo en `mt6582-spm.c`, y todo **seleccionable en caliente por sysfs** para no recompilar/reflashear
en cada variante — esa es la idea de fondo de la sesión.

### H6j — selector del encoding del NOT-FOUND (kernel #32)
`/sys/module/mt6582_spm/parameters/spm_fs_nf_mode` (0..6):
`0=orig 1=limpio 2=cnt1 3=cnt2 4=ENOENT 5=fake-found 6=bit31`

**Sospecha de partida**: el not-found escribía `+4=0` pero **no tocaba `+8`**, que conservaba el
`len` de la petición (0x26) en memoria compartida → el MD leía status=0 con una longitud de path
fantasma. El modo 1 (limpio, `+8=0`) era el candidato.
Parche: `~/mainline/patch_nfmode.py`. Flasheo: `~/flash-nfmode.sh`.

### H6k — power-OFF del MD por MTCMOS (kernel #33)
`echo 1 > /sys/module/mt6582_spm/parameters/spm_md_poweroff`

Hasta entonces el driver solo tenía power-ON: un 2º intento en el mismo boot daba `BUSY=1` y ni
handshake, y el `reboot` por software cuelga en musb → **cada prueba exigía sacar la batería**.
Ahora: `poweroff → poweron → load → remap → release → hs2`.

Secuencia 1:1 con `spm_mtcmos_ctrl_mdsys1(STA_POWER_DOWN)` del stock
(`bq-src .../core/mt_spm_mtcmos.c:799`):
1. `TOPAXI_PROT_EN |= MD1_PROT` + esperar `PROT_STA1` (bloquea el bus del MD)
2. `SRAM_PDN` → 3. `PWR_ISO` → 4. `(~RST_B | CLK_DIS)` → 5. `~(PWR_ON|PWR_ON_S)`
6. esperar `PWR_STATUS`/`_S` a 0 (dominio realmente apagado)

Parche: `~/mainline/patch_mdoff.py`. Flasheo: `~/flash-mdoff.sh`. **Es la mejora de productividad
más grande de la sesión**: quita el power-cycle de cada iteración.

### H6m / H6n — quitar la latencia que metíamos nosotros
- **H6m**: el bucle de servicio sondeaba cada **25 ms**; el `ccci_fsd` real contesta en **~0.5 ms**.
  Si el MD tiene un *deadline* para el mount del FS, 25 ms lo revientan. Fase rápida de sondeo
  configurable: `spm_fs_fastpoll_us` (0 = busy-poll) y `spm_fs_fastpoll_iters`.
- **H6n**: los `dev_info` del camino FS costaban **20–37 ms por operación** (consola). Con
  `spm_fs_quiet=1` se acumulan los op-codes en `spm_fslog[]` y se vuelcan **después** del bucle,
  dejando el servicio a la velocidad del hardware.

Imágenes: `boot-modem-fast.img`, `boot-modem-quiet.img`.

### H6l — barrido del formato de respuesta del `0x1010` (GetFullPath)
`/sys/module/mt6582_spm/parameters/spm_fs_1010_mode` (0..7):
```
0 = actual        1 = eco + escribir +8=plen      2 = length = plen (sin cabecera)
3 = eco con c=1   4 = formato corto tipo GetDrive 5 = vacío limpio (c=0,+8=0)
6 = eco + NUL     7 = ★ REPRODUCIR EL GROUND-TRUTH byte a byte
```
**Motivo**: el MD excepciona tras el `0x1010` **tanto en not-found como en found** (probado creando
el fichero) ⇒ el fallo está en el **formato/semántica** de la respuesta, no en si existe o no.
Parche: `~/mainline/patch_1010mode.py`. Imagen: `boot-modem-1010.img`.

### H6o — EXPERIMENTO DE CONTROL: no responder nada (kernel #37, `boot-modem-ctl.img`)
`spm_fs_1010_mode = 8` → no toca el buffer, no pone el done-marker y no envía `FS_TX`.
Centinela `SPM_FS_NOREPLY` (0xFFFFFFFF) en el driver.

**Por qué**: quedaron refutadas las tres hipótesis anteriores —encoding del not-found (7 variantes),
formato de la respuesta (8 variantes, **incluida la del ground-truth byte a byte**) y la latencia
(25 ms → sub-ms)—. En los tres casos el resultado es idéntico: **HS ok, 2 operaciones FS, EXCP**.
La excepción llega **~7 ms después** de nuestra 2ª respuesta, así que la dispara la respuesta…
pero **su contenido da igual**.

Lectura del control:
- **Si el MD EXCEPCIONA IGUAL** → el disparador **no es nuestra respuesta**: hay que mirar otra capa
  (framing CCCI, canales que MOLY espera en paralelo, watchdog interno del MD…).
- **Si NO excepciona** (se queda esperando) → sí es nuestra respuesta lo que le sienta mal, y el
  problema está en el framing/contenido → toca RE del `ccci_fsd`.

Parche: `~/mainline/patch_noreply.py`. Imagen: `~/mainline/pkg/boot-modem-ctl.img` (kernel
`#37 SMP Wed Jul 29 00:16:41 BST 2026`).

---

## 📍 ESTADO AL CERRAR

- **Kernel #37 construido** con H6j+H6k+H6l+H6m+H6n+H6o; imagen `boot-modem-ctl.img` lista.
- **El experimento de control (modo 8) NO consta como ejecutado** — es lo primero que hay que hacer.
- El móvil no responde en la red ahora mismo (apagado o con otra imagen).

## ▶️ SIGUIENTE PASO CONCRETO

```sh
# 1) flashear (BROM: batería fuera/dentro + VOL-ABAJO + USB)
sudo systemctl stop ModemManager
sudo ~/mtkclient/venv/bin/python ~/mtkclient/mtk.py wo 0x2900000 0x1400000 \
     ~/mainline/pkg/boot-modem-ctl.img

# 2) en el móvil: control = no contestar al 0x1010
echo 8 > /sys/module/mt6582_spm/parameters/spm_fs_1010_mode
~/modem-test.sh          # (desde la Pi; lanza /root/mdtest.sh en el móvil)

# 3) reintentos SIN sacar la batería (H6k)
echo 1 > /sys/module/mt6582_spm/parameters/spm_md_poweroff
echo 1 > /sys/module/mt6582_spm/parameters/spm_md_poweron
```

Y según salga:
- **EXCP igual** → el disparador no es la respuesta FS. Mirar framing CCCI y qué otros canales
  espera MOLY en paralelo mientras monta el FS.
- **Sin EXCP** → es el contenido/framing de nuestra respuesta → RE del `ccci_fsd` de Lineage
  (ya tenemos `gd-boot-full.out` como ground-truth y los op-codes en `~/modem-fsd/fs-opcodes.md`).

## Artefactos (Pi `.123`)

| Qué | Ruta |
|---|---|
| Driver con todo aplicado | `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c` |
| Copia previa a H6j | `…/mt6582-spm.c.pre-nfmode-0728` |
| Parches | `~/mainline/patch_{nfmode,mdoff,1010mode,noreply}.py` |
| Imágenes | `~/mainline/pkg/boot-modem-{1010,fast,quiet,ctl}.img` |
| Flasheo | `~/flash-{nfmode,mdoff,modem-gd,diario}.sh` |
| Prueba | `~/modem-test.sh` → `/root/mdtest.sh` en el móvil |
| Ground-truth del fsd | `~/modem-fsd/gd-boot-full.out`, `~/modem-fsd/fs-opcodes.md` |
| Imagen diaria A/B (kernel #14) | `~/mainline/pkg/boot-menupick24-consys.img` (`~/flash-diario.sh`) |

**Nota de método**: `flash-diario.sh` existe para hacer A/B contra una imagen **conocida-buena**
(kernel #14, consys/GPS/WiFi) cuando algo raro pasa y hay que descartar que sea del kernel del módem.
