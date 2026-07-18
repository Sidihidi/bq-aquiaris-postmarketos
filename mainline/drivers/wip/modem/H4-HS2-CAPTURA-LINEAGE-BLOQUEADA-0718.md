# Módem H4 — captura del runtime de Lineage: BLOQUEADA por el build + reorientación (0718)

> Objetivo: leer el `modem_runtime_t` COMPLETO que Lineage escribe (con las bases/tamaños de
> ShareMem) para diffear contra el nuestro. **Bloqueado en este build de Lineage por 4 muros.**
> Además, análisis: **el data abort probablemente NO es un campo del runtime** (ya damos todas
> las regiones no-cero del source). Reorientación al final.

## Cómo se dispara el dump (encontrado)
- `ccci_dump_runtime_data` (ccci_md_main.c) vuelca todos los campos. Los de ShareMem son
  `CCCI_DBG_MSG`; Platform/DriverVersion/BootingStartID son `CCCI_MSG_INF`.
- Filtro de log: **`/sys/ccci/filter`** (NO `/sys/kernel/ccci/`), comando
  `echo "-l=0 0x20111111 0xffffffff" > /sys/ccci/filter` (key fija 0x20111111, mask=todo).
  Verificado: `msg mask: ffffffff`.
- Trigger: ioctls en `/dev/ccci_ioctl0` (`ccci_vir_chr_ioctl`). Números (CCCI_IOC_MAGIC='C'):
  `SEND_RUN_TIME_DATA=0x4307`, `DO_MD_RST=0x4306`, `FORCE_MD_ASSERT=0x4304`.

## Los 4 muros (todos reales, verificados en HW)
1. **`SEND_RUN_TIME_DATA` (0x4307) restringido**: "Set runtime by invalid user" — el ioctl solo
   lo acepta del proceso md_init registrado. No se puede forzar el dump directo.
2. **`CCCI_DBG_MSG` NO cae en dmesg**: aunque KERN_DEBUG (`<7>`) SÍ se retiene en dmesg (probado
   con `/dev/kmsg`), las líneas ShareMem —que van JUSTO tras BootingStartID (que sí sale)— no
   aparecen. En este build el macro usa la variante **xlog VERBOSE → logcat**, no printk→dmesg.
   Y en logcat tampoco aparecieron (el dump del kernel no llega al logcat de userspace).
3. **`/dev/mem` compilado fuera** (`mknod /dev/mem c 1 1` crea el nodo pero `open` da ENXIO =
   CONFIG_DEVMEM=n). No se puede leer la SMEM de Lineage (0xBD600000) directamente.
4. **`FORCE_MD_ASSERT` no re-arranca fiable**: la 1ª vez re-handshakeó (MD_INIT_START_BOOT→
   NORMAL_BOOT_ID con el dump INF), pero las siguientes solo loguean la excepción sin re-boot ni
   re-dump. + spam del MD ([FH] freqhopping, ~300 líneas/s) rota el buffer en ~2s.

## Herramientas construidas (reutilizables, `tools-*.c` en este dir)
- **`tools-ccci-ioctl.c`**: caller de ioctl estático ARM (`arm-linux-gnueabihf-gcc -static`;
  corre en Android). `ccci-ioctl <dev> <req_hex>`.
- **`tools-memread.c`**: lector de `/dev/mem` con lseek (bloqueado aquí por CONFIG_DEVMEM=n, pero
  útil si otro build lo trae). Compilar igual y `adb push` a `/data/local/tmp/`.

## 🔎 Reorientación (lo importante)
**El data abort casi seguro NO es un campo del runtime que falte.** Ya proveemos TODAS las
sub-regiones que el source marca no-cero (Exce, MDExExpInfo, Misc, IPC, PCM, RPC, FS) y la
estructura del runtime coincide 1:1 con la derivación del source. El MD acepta el runtime, arranca
y aborta muy temprano igual. Un dump de Lineage probablemente CONFIRMARÍA lo que ya tenemos, no
revelaría un campo nuevo.

**Mejor siguiente dirección — la DIFERENCIA de bring-up, no el runtime:** nuestro bring-up es
MÍNIMO (load→remap→release→runtime por sysfs). El driver ccci de Lineage hace un bring-up COMPLETO
con pasos que nos saltamos y que el MD puede exigir:
- **MPU de la EMI** (`emi_mpu_set_region_protection` / "Clear MPU protect MD1 ROM/R/W region" —
  visto en la captura del ROM-remap). Lo dejamos OFF; quizá el MD exige la región configurada
  (permisos AP+MD), no ausente.
- **Clocks/EINT/GPIO** del MD que el ccci de Lineage toca antes del release.
- El orden/sincronía exactos de `md_env_setup_before_boot` → `set_ccci_runtime` → `let_md_go`.
Diffear la secuencia COMPLETA de Lineage (dmesg del boot, ya parcialmente capturado en
`lineage-md-boot-0717.log`) contra nuestros 5 pasos. Alternativa: desensamblar el early-boot de
MOLY (qué dirección desreferencia = la `0x7003425c` del EX, en espacio-MD).

## Estado
HS1 ✅, runtime ACEPTADO ✅, data abort temprano = frontera. Móvil restaurado a la imagen de diario
(#14, boot-menupick24-consys). Driver y docs en el repo. Captura de Lineage cerrada como vía (muros
del build); la vía viva es el diff de la secuencia de bring-up o el RE de MOLY.

*2026-07-18, sesión Windows (Fable 5). Captura del runtime de Lineage agotada; reorientado al
bring-up completo vs el mínimo.*
