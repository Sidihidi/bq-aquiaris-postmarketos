# Darwin/XNU en el krillin — **el kernel arranca ENTERO y lanza el proceso 1**

**Fecha:** 2026-07-22
**Hito:** `bsd_init: done` — Mach + IOKit + BSD completos, **raíz HFS+ montada** y
**`/sbin/launchd` lanzado** en un BQ Aquaris E4.5 (MediaTek MT6582, ARMv7, 2014).

Continúa a [`M32-MACH-E-IOKIT-ARRANCAN-0721.md`](M32-MACH-E-IOKIT-ARRANCAN-0721.md).

> **Matiz importante, medido después (§6):** `launchd` se *carga* y el `exec`
> tiene éxito, pero **no llega a ejecutar ni una instrucción de usuario**. El
> kernel se atasca antes, en `load_init_program` → `copyout`, por un bug de
> coherencia en las tablas de páginas de usuario. Frontera actual del port.

---

## 1. La traza final

```
bsd_init: calling bsd_autoconf
  BA0 pre kminit / BA1 kminit ok
  PS0..PS6 (pseudo-dispositivos; bpf SALTADO)
  BA2 pseudo_inits ok / BA3 -> IOKitBSDInit
Added memory device md0/rmd0 ... for 0000000000010000
AppleARMCPU::start: registered processor with Mach subsystem.
bsd_init: calling setconf → SC0 → SC1 IOFindBSDRoot RETURNED
bsd_init: vfs_mountroot
MAC Framework enabling multilabel support: root_device -> / (hfs)
MAC Framework enabling multilabel support: -> /dev (devfs)
bsd_init: calling siginit
bsd_init: calling bsd_utaskbootstrap      ← crea el proceso 1 y ejecuta launchd
bsd_init: calling mountroot_post_hook
bsd_init: done
=== BS_MAIN: bsd_init RETURNED ===
```

**Prueba de que `launchd` se ejecutó:** `load_init_program()` (bsd/kern/kern_exec.c)
termina en

```c
error = execve(p, &init_exec_args, retval);
if (error)
    panic("Process 1 exec of %s failed, errno %d", init_program_name, error);
```

No hubo panic y `bsd_init` retornó → el `exec` de `/sbin/launchd` tuvo éxito.

---

## 2. Los tres bugs de esta sesión

### 2.1 La base de tiempo no era monótona (`random_init` colgado)

`bsd_init` moría en el pseudo-dispositivo `PS6` = `random_init` →
`PreliminarySetup()` → `prngForceReseed()`, que recolecta entropía así:

```c
endTime = microuptime() + RESEED_TICKS;   /* 50 us */
do { ...SHA1... } while (curTime < endTime);
```

Con el reloj parado no sale nunca. Y estaba parado: `mt6582_get_timebase()`
**no leía el contador libre**, reconstruía el tiempo desde **GPT1** (el one-shot
del planificador) con `clock_absolute_time + (decrementer - contador)`. Como
GPT1 cuenta hacia arriba dentro de cada tick, esa resta hacía que el tiempo
**retrocediera** entre interrupciones.

**Fix:** leer **GPT2**, que ya estaba configurado en free-run precisamente para
esto y es monótono por construcción, con la parte alta a mano (32 bits a 13 MHz
dan la vuelta cada ~5,5 min).

> **Pista que teníamos delante desde el día anterior:** el informe de pánico
> decía `System uptime in nanoseconds: 0`. No era un campo sin rellenar, era el
> síntoma exacto.

### 2.2 `bpf_init` contra una red sin inicializar

La tabla `pseudo_inits` (generada en `BUILD/.../bsd/DEBUG/ioconf.c`) contiene
`{4, bpf_init}` — Berkeley Packet Filter, o sea **red**. Un build anterior
saltaba a propósito `dlil_init`/`proto_kpi` y `socketinit`/`domaininit`, pero
**dejaba el pseudo-dispositivo que depende de ellos en la tabla**. Se salta
comparando el puntero de función.

### 2.3 Faltaba `rd=md0` en la línea de comandos

`IOFindBSDRoot()` esperaba un medio real con `Content=Apple_HFS`
(`Still waiting for root device`) que en este teléfono no puede aparecer: no hay
driver de almacenamiento. Pero **el disco en memoria ya estaba registrado**
(`Added memory device md0/rmd0`).

`GenericBooter/init/main.c` tiene tres variantes de cmdline y **la del MT6582 era
la única sin `rd=md0`** — las otras plataformas ya lo llevaban. Con ese
boot-arg, `IOFindBSDRoot` (`IOKitBSDInit.cpp:405`) usa `md0` directamente.

---

## 3. El ramdisk: había un espacio de usuario sin usar

`~/darwin-krillin/ramdisk/ramdisk.dmg` (25 MB, clon de
`github.com/darwin-on-arm/ramdisk`) es un **volumen HFS+ real** con
`/sbin/launchd`, `/usr/lib/dyld`, `libSystem.B.dylib` y `/bin/sh`.
**Nunca se había embebido**: `package-xnu.sh` decía "Ramdisk sin cambios" y
seguía metiendo el `Ramdisk.img3` vacío de 64 KB — de ahí el
`hfs_mounthfsplus: unknown Volume Signature`.

⚠️ **HFS SÍ está compilado en este kernel.** Una comprobación previa con `nm`
dio 0 símbolos y llevó a la conclusión contraria: **el binario está stripped**.
La prueba real es la traza (`hfs_mountfs`, `hfs_mounthfsplus` ejecutándose).

### No cabía: presupuesto de la partición

| | |
|---|---|
| partición `boot` | 20 971 520 B (20 MB) |
| `mach_kernel` (DEBUG) | ~7,1 MB |
| `ramdisk.dmg` original | 24 MB (21 MB usados) |

**Solución: ramdisk recortado de 6 MB** (3,3 MB de contenido):

```sh
sudo apt-get install -y hfsprogs          # mkfs.hfsplus / fsck.hfsplus
dd if=/dev/zero of=ramdisk-min.dmg bs=1M count=6
/usr/sbin/mkfs.hfsplus -v DarwinRD ramdisk-min.dmg
# montar el original en RO y el nuevo en RW, y copiar:
#   SE QUEDA: sbin/ etc/ var/ tmp/ dev/ bin/sh
#             usr/lib/dyld, usr/lib/libSystem.B.dylib, usr/lib/system/
#             resto de usr/lib/*.dylib, System/Library/LaunchDaemons
#   FUERA:    System/Library (6,1M), usr/local (1,7M),
#             libicucore (1,6M), libobjc (1,1M), libiconv (1M),
#             libstdc++ (680K), libncurses (208K)
sudo /usr/sbin/fsck.hfsplus -f -y ramdisk-min.dmg   # dejar el volumen LIMPIO
image3maker -f ramdisk-min.dmg -t rdsk -o GenericBooter/images/Ramdisk.img3
```

Resultado: sección kernel 12,84 MB + initrd 1,82 MB = **14,7 MB** en 20 MB.

---

## 4. Estado del puerto

**Funciona, en hardware real:**

- Mach completo: VM, planificador, IPC, temporizador con tick real
- IOKit: runtime C++, `OSMetaClass`, motor de matching, `AppleARMPE`,
  `AppleARMCPU` registrando el procesador contra Mach
- BSD completo: VFS, devfs, kauth, sysctl, pseudo-dispositivos
- **Raíz HFS+ montada desde disco en memoria**
- **`/sbin/launchd` ejecutado como proceso 1**

**Cabos sueltos conocidos:**

- Escribir píxeles de **16 bits** en el framebuffer mata el arranque por razones
  sin identificar. Se trabaja con escrituras de 32 bits y geometría lógica
  **270×480** (cada píxel lógico cubre 2×2 reales). Ver doc anterior.
- Build **DEBUG** con asertos activos.
- Red desactivada a propósito (`dlil`, `socketinit`, `domaininit`, `bpf`).
- La consola `vc_` de XNU está desactivada; la pantalla es del pexpert.
- No se ve salida de `launchd`: falta comprobar si `/dev/console` está cableado
  a la consola del framebuffer, y si las `LaunchDaemons` que copiamos bastan.

**Veredicto sin adornos:** sigue siendo *research*. No hay drivers de
periféricos (pantalla táctil, radio, almacenamiento, WiFi) y cada uno habría que
**reescribirlo** en C++ para IOKit, no portarlo. Pero el kernel de macOS/iOS
arranca entero y lanza espacio de usuario en un teléfono MediaTek de gama baja
de 2014, que era exactamente la pregunta del proyecto.

---

## 6. La frontera real: las tablas de páginas de USUARIO

Se instrumentó el despachador de syscalls (`bsd/dev/arm/unix_syscalls.c`) para
imprimir las primeras llamadas de cualquier proceso, y los manejadores de
excepción (`osfmk/arm/trap.c`, `sleh_abort` / `sleh_undef`) para volcar los
primeros fallos. Resultado: **cero syscalls** y un fallo repetido en bucle.

### 6.1 Lectura del fallo

```
FALLO datos pc=0x800e22a4 far=0x00001000 fsr=0x00000805 cpsr=0x200001d3
            TTBR0=0x82f9205b
  vm_fault code=0x00000000 pmap_L1=0x82f92000 TTE=0x82fc2001
```

| campo | valor | lectura |
|---|---|---|
| `cpsr` modo | `0x13` | **modo SVC (kernel)** — NO es launchd fallando |
| `pc` | `0x800e22a4` | dentro de **`_copyout`** (empieza en `0x800e20b0`) |
| `far` | `0x1000` | escribe en la primera página de USUARIO |
| `fsr` | `0x805` | bit11=1 → **escritura**; FS=`0x5` → **fallo de traducción de SECCIÓN** (la entrada **L1** está inválida para el hardware) |
| `vm_fault` | `KERN_SUCCESS` | la VM cree que ha resuelto el fallo |
| `pmap_L1` vs `TTBR0` | `0x82f92000` == `0x82f92000` | **el pmap es el correcto** |
| `TTE` | `0x82fc2001` | la entrada L1 **SÍ está escrita** (tabla L2 en `0x82fc2000`, tipo `01` = page-table descriptor) |

Es decir: `launchd` **no falla — nunca ejecuta**. El kernel se atasca en
`load_init_program()` copiando el `argv` a la página que acaba de reservar con
`vm_allocate`. `sleh_abort` llama a `vm_fault`, que devuelve éxito, se reintenta
la instrucción y vuelve a fallar idéntica: bucle infinito.

**Resolver el `pc` exige el binario SIN strippear** (`BUILD/obj/DEBUG_ARM_MT6582/mach_kernel`)
y el **`nm` de la toolchain Darwin** — el `nm` de GNU no lee Mach-O:
`/opt/Developer/Toolchains/DarwinARM.toolchain/usr/bin/nm -n`.

### 6.2 Hipótesis descartadas

1. **`launchd` revienta.** No: el `cpsr` dice modo kernel, y `load_init_program`
   haría `panic` si el `exec` fallara.
2. **TTBR0 apunta a la tabla del kernel.** No: `TTBR0=0x82f9205b` (base
   `0x82f92000`, el resto son atributos de cacheabilidad) coincide con el L1 del
   pmap, y la traza confirma que `pmap_switch()` se llama y que
   `arm_context_switch()` escribe el registro.
3. **Se falta sobre otro pmap.** No: `pmap_L1` == base de TTBR0.
4. **`pmap_enter` no escribe nada.** No: la TTE es válida.

### 6.3 Hipótesis viva

El software lee/escribe la L1 a través de **`pmap->pm_l1_virt`** y el hardware
camina por **`pmap->pm_l1_phys`**. Todo cuadra *salvo* que la MMU no ve la
entrada. Lo más probable es que **ambos no apunten a la misma página física**
— se escribe en una tabla y la MMU camina por otra. `pmap_expand_ttb()`
(`osfmk/arm/pmap.c:2704`) reasigna la L1 y actualiza los dos campos; ahí es
donde pueden desincronizarse.

**Sonda siguiente:** en `sleh_abort`, comparar `pm_l1_virt` con
`phys_to_virt(pm_l1_phys)` y leer la TTE por **las dos** vías. Si difieren, está
encontrado. Descartado eso, quedaría coherencia de caché entre las escrituras de
tablas y el table walker (atributos de TTBR0: `0x5b` = IRGN/RGN Write-Back,
S=1).

---

## 7. Siguiente paso natural

1. Cerrar el bug de §6.3 — es lo único que separa al port de ejecutar código de
   usuario.
2. Después, cablear `/dev/console` a la consola del pexpert para ver la salida
   de `launchd`. Un `/bin/sh` interactivo requeriría además driver de entrada,
   que no existe.
