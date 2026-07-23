# Darwin/XNU en el krillin — TRASPASO para seguir en casa (0723 noche)

> Continúa a `ESTADO-Y-HALLAZGOS-0722.md` y `SIGUIENTES-PASOS-LAUNCHD-0723.md`
> (que tiene el detalle técnico build-a-build). Este es el resumen de dónde
> estamos y qué hacer al reanudar.

---

## 1. ESTADO EN UNA FRASE

XNU arranca Mach + IOKit + BSD completos, monta raíz HFS+, **y el bug de MMU que
bloqueaba `/sbin/launchd` está RESUELTO**. La última imagen (**M102**) debería
hacer que `copyout` complete y el proceso 1 arranque. **Falta confirmarlo en HW**
(pendiente de flashear M102 con más batería).

- Última imagen: `~/mainline/pkg/boot-xnu-M102-uartttbr1.img`
  md5 `a2e21fa1ba9d0d72a9728bc9bc313097`
- Árbol: Pi de build `~/darwin-krillin/`. Imágenes en `~/mainline/pkg/`.

---

## 2. LO PRIMERO AL REANUDAR: flashear M102 y leer el UART

```sh
# arrancar captura UART (un SOLO lector)
sudo ~/uart-capture.sh
# ver en vivo (desde el Mac o la propia Pi):
tail -f /tmp/uart.log | tr -c '[:print:]\n' '.'

# flashear (BROM: bateria fuera/dentro + Vol- + USB de datos; luego cable UART):
sudo systemctl stop ModemManager
sudo ~/mtkclient/venv/bin/python ~/mtkclient/mtk.py \
     wo 0x2900000 0x1400000 ~/mainline/pkg/boot-xnu-M102-uartttbr1.img
```

**Éxito = ver en el UART:**
```
LP 1 copyout nombre ok
LP 2 args -s ok
LP 3 copyout argv ok
LP 4 security token ok
*** LI0: execve /sbin/launchd (proceso 1) ***
*** LI1: launchd EN EJECUCION ***
SYS pid=0x00000001 <syscall> ...   <- launchd ejecutando codigo de usuario
```
Si sale eso: **launchd corre**. Hito enorme (kernel de Apple ejecutando espacio
de usuario en un MT6582). A partir de ahí: ¿qué hace launchd? (ver §5).

---

## 3. EL MURO QUE SE RESOLVIÓ (para entender qué se tocó)

Eran **dos bugs encadenados**:

### Bug real (M102): el "bucle de copyout" era el UART, no copyout
El mapeo de usuario funcionaba desde M94 (verificado por HW: `L1[0]=0x82fc6001`,
`PTE=0x82fc51bb` AP=RW, `ATS1CPR` y `ATS1CPW` ambos ÉXITO). Capturando el 1er y
2o fallo (M101):
- 1er fallo: `dfar=0x1000` (copyout) → vm_fault mapea → **reintenta y FUNCIONA**.
- 2o fallo: `dfar=0x11002014` = **registro LSR del UART**, desde `fbcon_uart_putc`.

`fbcon_uart_putc` accedía al UART por la física `0x11002000`, identity-mapeada en
**TTBR0** (por-pmap, sólo en el pmap del kernel). Al correr el proceso de USUARIO
(TTBR0=user), el UART desaparece → fallo mudo = "bucle azul". El framebuffer
(0xBF400000) nunca falló porque está en **TTBR1** (compartido).

**Fix**: mapear el UART también en TTBR1 (VA `0xF1000000` → PA `0x11000000`) en
`arm_vm_init`, y `fbcon_uart_putc` usa `0xF1002000`.

> **REGLA NUEVA DEL PORT**: todo MMIO accedido desde contexto de USUARIO
> (consola, etc.) debe estar en **TTBR1** (VA ≥ 0x40000000), no en identity TTBR0,
> o desaparece bajo el pmap de usuario.

### Bug secundario (M94): caminatas de tabla de usuario cacheables
`arm_context_switch` (cpufunc-v7.s) ponía TTBR0 de usuario con `|0x5b` (WB+S).
Cambiado a no-cacheable. NO era EL muro pero es correcto y se deja.
El kernel usa `|0x18` en `set_mmu_ttb` y por eso siempre funcionó.

### Fix previo, real y necesario (M62): alineación de la L1 de usuario
`pmap_create` reservaba la L1 con `pmap_grab_page` (4KB). El HW toma la base de
TTBR0 alineada a 16KB. Ahora `cpm_allocate(0x4000,&pg,0,3,...)` = 16KB alineada.

---

## 4. LIMPIEZA PENDIENTE en el árbol (hay MUCHO andamiaje de debug)

Los ficheros tienen instrumentación temporal de los ~18 builds del muro. Al
confirmar que M102 funciona, **limpiar y dejar sólo los fixes definitivos**:

- `osfmk/arm/trap.c` (`sleh_abort`): quitar `fbmark_quarters`, `rawuart_*`,
  `g_abort_fault`, `PE_fb_hexline` calls, el paint AZUL de entrada (M84), los
  bloques de comentarios M88/M93/M96/M99/M101/M102. Dejar el `case DATA_ABORT`
  limpio: `vm_fault` + retorno. **OJO**: el `PE_early_puts` del "FALLO" está en
  `if(0)` (cuelga en abort) — dejarlo desactivado o borrarlo.
- `osfmk/vm/vm_fault.c`: quitar los `if (g_abort_fault) fbmark_quarters(...)`.
- `osfmk/arm/pmap.c`: la limpieza `arm_dcache_wbinv_all()` tras la PTE (M96) NO
  hace falta (no era coherencia) — quitarla, es LENTÍSIMA (flush total en cada
  pmap_enter). Mantener el `DSB` es inofensivo. **El fix real aquí es sólo el
  M62 (alineación L1 en pmap_create).**
- `osfmk/arm/cpufunc-v7.s` (`arm_context_switch`): el cambio a no-cacheable se
  queda.
- `osfmk/arm/arm_vm_init.c`: el mapeo UART en 0xF1000000 (TTBR1) se queda.
- `pexpert/arm/common/pe_bringup.c`: `fbcon_uart_putc` → 0xF1002000 se queda.
  `PE_fb_hexline` se puede dejar (útil) o quitar. `fbcon_quiet=0` (M67, verboso)
  — decidir si dejar verboso o volver a silenciar.
- `bsd/kern/kern_exec.c`: sondas `LP0..LP4`, `LI0/LI1`, `REGS/L1DUMP/KSTACK/
  ATS1CPR/VEC` en `load_init_program` — quitar cuando arranque.
- `bsd/dev/arm/unix_syscalls.c`: traza `SYS pid=` — quitar o dejar tras flag.

**Antes de limpiar, hacer commit del estado que funciona** (o un backup del
árbol) — hay 18 builds de trabajo sin commitear.

---

## 5. SIGUIENTE FRONTERA (si launchd arranca)

launchd ejecuta pero probablemente choque pronto: necesita leer sus plists de
`/System/Library/LaunchDaemons`, abrir `/dev/console`, forkear... con el ramdisk
recortado (sólo dyld+libSystem+launchd+sh). Pasos:
1. Ver qué syscalls hace launchd (la traza `SYS pid=1` lo muestra).
2. Cablear `/dev/console` a la consola del pexpert para ver su salida.
3. Probablemente falten libs/plists en el ramdisk recortado → añadir del
   `ramdisk.dmg` original (`~/darwin-krillin/ramdisk/`, 25MB, tiene el userland
   completo; recortar a lo que pida).
4. Un `/bin/sh` interactivo necesitaría driver de teclado/entrada (no existe).

---

## 6. INFRA (recordatorio)

- **UART por micro-USB del krillin**: verde=D+ (TX movil)→RX Pi(GPIO15/pin10),
  blanco=D-→TX Pi(GPIO14/pin8), negro=GND, **rojo AISLADO**. Pi5: `/dev/ttyAMA0`
  (NO serial0), `enable_uart=1` en config.txt (ya puesto). UN solo lector.
  921600 8N1. Se captura preloader+LK+GenericBooter+XNU.
- **PE_early_puts / UART CUELGAN en contexto de abort** (nested abort). En
  `sleh_abort` sólo vale **framebuffer directo** a 0xBF400000 (TTBR1) — para
  volcar valores usar `PE_fb_hexline` (dibuja hex con pixel directo).
- **Batería**: Darwin NO carga (sin driver FAN5405). LK corta a **3450mV**
  (`Can not Boot Linux Kernel`, bucle `mt_power_off`). El cable UART NO carga
  (VBUS aislado). Cargar: cargador externo de batería, o USB de datos real
  (modo carga apagado del LK). **Si deja de arrancar de golpe, mirar VBAT en el
  UART antes de sospechar del build.**
- Build: `cd ~/darwin-krillin && ./build-xnu.sh && ./package-xnu.sh` (los
  `Error 139` de symbolset son no-fatales, el kernel enlaza igual). Cada imagen
  con nombre único. `nm` de Mach-O: usar el de la toolchain Darwin
  (`/opt/Developer/Toolchains/DarwinARM.toolchain/usr/bin/nm`), el de GNU no lee
  Mach-O y el `mach_kernel-mt6582` empaquetado está stripped (usar el de
  `BUILD/obj/DEBUG_ARM_MT6582/mach_kernel`).

---

## 7. CONSTANTES ÚTILES (medidas)

- `gPhysBase=0x81000000  gVirtBase=0x80000000  phys_to_virt(p)=p-0x01000000`
- fb: `0xBF400000` RGB565 16bpp stride 1088 (TTBR1). UART fis `0x11002000`,
  ahora también en `0xF1002000` (TTBR1).
- `TTBCR N=2` (TTBR0=user 0-0x3FFFFFFF, TTBR1=kernel 0x40000000+).
- TTBR0 usuario ahora `0x82fb4000` (no-cacheable tras M94).
- Mapeo de 0x1000 tras vm_fault: `L1[0]=0x82fc6001` → L2 → `PTE=0x82fc51bb`
  (PA 0x82fc5000, AP=RW).
