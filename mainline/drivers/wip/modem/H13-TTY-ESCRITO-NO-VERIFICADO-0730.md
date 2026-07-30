# H13 — `/dev/ttyCCCI0`: escrito, registra, y **cuelga al usarlo**

> Continúa de `H12-DIALOGO-AT-CON-EL-BASEBAND-0730.md`.
> Kernel **#89**, imagen `~/mainline/pkg/boot-modem-m1.img` (md5 `d7d170469c0251662299a2b4227ee8f4`).

**Estado honesto**: el tty existe, se registra y el hilo de servicio arranca, pero **abrirlo y
escribirlo cuelga el sistema**. Por eso queda **desactivado por defecto** (`spm_tty_enable=0`): el
arranque del módem y el canal AT por sysfs funcionan, y no deben quedar a merced de una pieza sin
verificar.

Lo que **sí** está verificado en este kernel (regresión completa, con el tty apagado):

```
*** HS2 LOGRADO: NORMAL_BOOT_ID ***
H6 FS: 1572 ops servidos
H12 TX 'ATI' -> MTK2 / MOLY.WR8.W1449.MD.WG.MP.V1 / OK
H7p EXCEPCION del MD: tipo=0 [INVALID]      (ninguna)
```

⇒ **la extracción del bucle a `spm_ccci_pass()` no rompió nada.** Eso era el riesgo principal del
cambio y está descartado.

---

## Lo que se implementó

- **`spm_ccci_pass()`**: el cuerpo del bucle de servicio del CCCI extraído a una función *sin tocar
  su contenido*, para que lo usen el bucle de arranque y el hilo. Devuelve 1 si en esa pasada llegó
  el boot-ready. **Verificado**: el arranque se comporta igual que antes.
- **kthread `mt6582-ccci`**: llama a `spm_ccci_pass()` en bucle tras el HS2. Hasta ahora el servicio
  moría con el bucle síncrono, así que un tty sólo habría funcionado mientras alguien lo mantenía
  vivo a mano.
- **`/dev/ttyCCCI0`**: `write` copia al `tx_buffer` del puerto 1 y toca el timbre en
  `CCCI_UART2_TX` (12); el RX del `lch 10` se empuja al `tty_port` y se ACKea por
  `CCCI_UART2_RX_ACK` (11). Mutex `spm_ccif_tx` porque ahora el CCIF lo tocan el hilo y el `write`.

## Los dos fallos encontrados (y uno que queda)

**H13b — empujar al `tty_port` antes de que existiera.** El registro estaba al *final* del bucle,
pero los timbres del `lch 10` llegan **durante** la fase post-HS2 ⇒ reinicio del móvil. Arreglado
registrando el tty antes de servir, más una guarda que drena y ACKea aunque el tty no esté listo (lo
importante es que el MD no se atasque).

**H13c — al `tty_port` le faltaba el `ops`.** `tty_port_init()` hace `memset(port, 0, ...)`, así que
`port->ops` queda a NULL, y `tty_port_open()` lo desreferencia sin comprobarlo. Medido en el móvil:

```
Unable to handle kernel NULL pointer dereference at virtual address 0000000c
PC is at tty_port_open+0xa8/0xf0     <- 0x0c = offset de ->activate
tty_port_open from tty_open+0x14c/0x634
```

Arreglado con un `tty_port_operations` vacío (sin `->activate` no hay init específica, y sin
`->carrier_raised` se considera que siempre hay portadora, que es lo correcto para un canal sin
líneas de módem reales).

**Lo que queda**: con las dos cosas arregladas, usar el nodo **sigue colgando el sistema** — y esta
vez es un **cuelgue duro**: sin `Oops`, sin registro en `pstore` (que está configurado), o sea que
reinicia el watchdog.

### Hipótesis principal para quien siga: `ioremap` dentro del camino caliente

`spm_tty_rx()` hace `ioremap`/`iounmap` del anillo **en cada evento**, y se le llama desde
`spm_ccci_pass()`, que durante la cascada de arranque corre en fase rápida (`spm_fs_fastpoll_us`,
~200 µs por pasada). En ARM de 32 bits el espacio de `vmalloc` es escaso y `ioremap` no es barato ni
libre de bloqueos: a esa frecuencia es un candidato serio a livelock, y encaja con el perfil
(cuelgue sin excepción, watchdog).

**Arreglo propuesto**: mapear el anillo **una sola vez** al registrar el tty y guardar el puntero,
en vez de mapear por evento. Lo mismo para `spm_tty_ring_write()`, que también mapea en cada
escritura. Es un cambio pequeño y hay que probarlo antes que cualquier otra teoría.

Segundo candidato, menor: el bloque de TX del FS dentro de `spm_ccci_pass()` **no** está bajo el
mutex `spm_ccif_tx`, así que el hilo y el `write` del tty pueden elegir el mismo canal físico de TX.
Eso corrompería mensajes, no colgaría el AP, pero conviene cerrarlo igualmente.

## Cómo experimentar

```sh
S=/sys/module/mt6582_spm/parameters
echo 1 > $S/spm_tty_enable      # ANTES del ciclo; por defecto 0
# ...ciclo del modem...
ls -l /dev/ttyCCCI0             # aparece tras el registro
```

⚠️ Con el tty activado, **el móvil se cuelga al abrir el nodo** hasta que se resuelva lo de arriba.

⚠️ Tras un cuelgue/oops, el arranque siguiente **no levanta `sshd`** — hay que arrancarlo a mano
desde la sesión gráfica (`doas rc-service sshd start`). El móvil sí sale en la red (responde a ping
y registra su nombre por DHCP), así que "ping sí, ssh no" **no** significa que se haya quedado en el
initramfs: significa justo esto.
