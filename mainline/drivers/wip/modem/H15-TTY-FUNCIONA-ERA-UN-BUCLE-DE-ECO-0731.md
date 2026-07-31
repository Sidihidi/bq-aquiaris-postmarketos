# H15 — `/dev/ttyCCCI0` FUNCIONA: era un bucle de eco (lo cazó la UART)

> Cierra la serie H13. Kernel **#102**.

```
$ printf 'AT\r'      > /dev/ttyCCCI0   ->  OK
$ printf 'ATI\r'     > /dev/ttyCCCI0   ->  MTK2
                                           MOLY.WR8.W1449.MD.WG.MP.V1
                                           OK
$ printf 'AT+CGMR\r' > /dev/ttyCCCI0   ->  +CGMR: MOLY.WR8.W1449.MD.WG.MP.V1, 2015/02/11 17:50
                                           OK
$ printf 'AT+CFUN?\r'> /dev/ttyCCCI0   ->  +CFUN: 0
                                           OK
```

**Un tty de Linux hablando con el baseband.** A partir de aquí oFono o ModemManager pueden
conectarse al nodo directamente. `+CFUN: 0` = radio apagada, que es lo esperado: nunca la
encendimos.

---

## La causa: el puerto nacía con ECO

Con la UART capturando (ver `H14`), la secuencia apareció entera y no deja lugar a dudas:

```
260.418898  H13f write: 3 bytes -> 3                  <- nuestro 'AT\r'
260.454149  H13g rx: timbre rr=0 rw=32                <- el MD contesta (32 B)
260.455146  H13q rx: 32 pendientes -> 32 entregados   <- los empujamos al tty
260.455203  H13f write_room                           <- Y LA CAPA DE LINEA ESCRIBE
260.456734  H13f write: 2 bytes -> 2
            ... avalancha infinita de escrituras de 1-2 bytes
```

La capa de línea hacía **eco** de todo lo que llegaba del módem; se lo devolvíamos al MD; el MD
respondía —y trae `ATE1`, su propio eco— y volvía a entrar. La realimentación saturaba el CCIF hasta
tumbar el sistema, **sin dejar nada en `dmesg` ni en `pstore`**.

**El fallo estaba en el registro, no en el camino de RX:**

```c
spm_tty_drv->init_termios = tty_std_termios;
spm_tty_drv->init_termios.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
/* ...y nada mas: c_lflag heredaba ECHO e ICANON de tty_std_termios */
```

Fix (H13r): el puerto nace **crudo** — `c_iflag = c_oflag = c_lflag = 0`. Es un puerto de módem;
quien quiera línea canónica que la pida con `stty`.

## Por qué costó tanto, y la lección

Se probaron **seis** hipótesis razonables antes: `ioremap` por evento, falta de `tty_wakeup`, el
registro dentro del sysfs, el hilo, el desbordamiento de punteros del anillo, el ACK por CCCI, y el
patrón de empuje comparado con `hvc_console`. **Dos de ellas eran fallos reales** (el `ioremap` en el
camino caliente causaba un cuelgue duro; sin `tty_wakeup` la escritura no volvía) y están
arregladas. Pero **ninguna era ésta**, porque el fallo no estaba en ninguna de las piezas que se
iban descartando: estaba en cómo nacía el puerto.

**La lección es de método, y es la misma que ya funcionó con el MD**: cuando un fallo es mudo, lo
primero es **abrir un canal de diagnóstico**, no formular hipótesis. Con el registro de excepción del
MD (`spm_md_ex`) el frente del módem se desatascó en una tarde; aquí, en cuanto la UART estuvo
capturando, la causa apareció **en la primera ejecución**. Las horas anteriores fueron descartes a
ciegas, cada uno con su compilación, su flasheo y su reinicio.

## Cómo se consiguió capturar (resumen, ver H14 para el detalle)

- **netconsole NO sirve** en este móvil: se registra pero no emite un solo paquete, ni por `usb0` ni
  por `wlan0` (`netpoll` no funciona sobre ninguno de los dos). Comprobado con `tcpdump`.
- **La consola serie sí está activa** (`/proc/consoles` → `mtk8250 -W- (ECB p )`).
- ⚠️ Pero **la UART muere en cuanto el driver `musb` enciende el PHY del USB** (t≈1.18 s): sus pines
  están multiplexados con el USB. Se ve clarísimo: el log se corta siempre en
  `mt6582-musb: PHY poweron START (v16)`.
- Solución: un **dtb aparte** con el nodo `usb@11200000` en `status = "disabled"`
  (`ganador-h2b-nousb.dtb`), sin tocar el DTS compartido con la otra sesión. Con eso la UART sobrevive
  todo el arranque y toda la sesión. El control va por WiFi.
- ⚠️ El cmdline del boot image **lo ignora el LK**: no se pueden pasar parámetros al kernel por ahí
  (se intentó `initcall_blacklist=mt6582_musb_driver_init` y no llegó). Por eso hizo falta el dtb.
- ⚠️ `cat > fichero` **bufferiza**: la captura se perdía. Usar `stdbuf -o0 cat`.

## Artefactos

| Qué | Ruta |
|---|---|
| Imagen de depuración (USB desactivado, UART viva) | Pi: `~/mainline/pkg/boot-uart-nousb.img` (md5 `1cb5419a…`) |
| dtb con el USB desactivado | Pi: `~/mainline/pkg/ganador-h2b-nousb.dtb` |
| Captura de la UART | Pi: `~/uart.log` (921600, `/dev/ttyAMA0`) |

⚠️ **La imagen flasheada ahora mismo tiene el USB DESACTIVADO** (es la de depuración). Para volver a
tener red por USB hay que reempaquetar con `ganador-h2b.dtb`.

## ▶️ Siguiente

1. `AT+CFUN=1` para encender la radio y ver si registra en red (con SIM).
2. Apuntar **oFono o ModemManager** al nodo — es lo que faltaba para **M3**.
3. Las tres `CCMNI` para datos IP → **M2**.
