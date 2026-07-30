# H11 — el módem habla: `+EPOK: 1` y `+EIND: 128` en el canal AT

> Continúa de `H10b-READ-DESPLAZADO-L4-SUPERADO-0730.md`.
> Kernel **#84**, imagen `~/mainline/pkg/boot-modem-m1.img` (md5 `37ca5de286e3672ae943ee7f5a8809db`).

Tras superar L4 el MD avisa por varios canales nuevos. Identificados en
`bq-src/…/dual_ccci/include/ccci_ch.h` y volcados sus anillos, resulta que **el baseband ya está
emitiendo comandos AT**:

```
H11 TTY1 UART2/modem-AT rx[r=0 w=26 len=16384] tx[r=0 w=0 len=16384]
H11   +0000: 0d 0a 2b 45 50 4f 4b 3a 20 31 0d 0a 0d 0a 2b 45  |..+EPOK: 1....+E|
H11   +0010: 49 4e 44 3a 20 31 32 38 0d 0a                    |IND: 128..|
```

Es decir, `\r\n+EPOK: 1\r\n` y `\r\n+EIND: 128\r\n`: las notificaciones no solicitadas con las que
MOLY anuncia que ha arrancado y está listo — justo lo que un RIL espera antes de mandar nada.
Lo mismo, byte a byte, en el anillo de `IPC_UART`.

---

## Los canales, identificados

| `lch` | constante | puerto TTY | struct del stock | qué es |
|---|---|---|---|---|
| 4 | `CCCI_PCM_RX` | — | — | voz |
| **10** | **`CCCI_UART2_RX`** | **1** (`uart2_shared_mem`) | `ccci_tty_modem` | **puerto del módem: comandos AT** |
| 23 / 27 / 31 | `CCCI_CCMNI1/2/3_TX_ACK` | — | — | las tres interfaces de datos |
| **38** | **`CCCI_IPC_UART_RX`** | **5** (`uart3_shared_mem`) | `ccci_tty_ipc` | puerto IPC |

El mapeo puerto→canal sale de `ccci_tty.c`: `ccci_uart_setup(0,…)` = UART1 (meta),
`(1,…)` = UART2 (modem), `(5,…)` = IPC_UART.

**Los canales TTY no llevan datos en el mensaje** (por eso llegan con `d0=ffffffff`): el mensaje es
un timbre y el contenido está en los anillos de memoria compartida que declaramos en el runtime.
Layout de `shared_mem_tty_t`:

```
+0x00 rx.read  +0x04 rx.write  +0x08 rx.length      rx_buffer @ +0x18     (MD -> AP)
+0x0c tx.read  +0x10 tx.write  +0x14 tx.length      tx_buffer @ +0x4018   (AP -> MD)
```

Herramienta nueva: **`spm_tty_dump`** (`echo 1 > /sys/module/mt6582_spm/parameters/spm_tty_dump`)
vuelca punteros y contenido pendiente de cada puerto declarado.

## Detalle de método

El volcado recorría al principio `CCCI_TTY_PORTS` (8, que es el **tope del array** del runtime tras
H8d) en vez de los `spm_md_uart_ports` **declarados** (6). Los puertos 6 y 7 salían con basura de
SMEM sin inicializar y nombre `(null)`, que se lee como si algo fuera mal cuando el MD ni los conoce.
Acotado en H11c — merece la pena porque una herramienta de diagnóstico que enseña ruido cuesta más
tiempo del que ahorra.

## ▶️ Siguiente

El camino está abierto y es el del RIL:

1. **Consumir** lo que el MD ha escrito: avanzar `rx.read` hasta `rx.write` y devolver el ACK por el
   canal correspondiente (`CCCI_UART2_RX_ACK` = 11, `CCCI_IPC_UART_RX_ACK` = 39), o el MD dejará de
   emitir cuando se le llene el anillo.
2. **Escribir** en `tx_buffer`, avanzar `tx.write` y tocar el timbre en `CCCI_UART2_TX` (12): con eso
   se le puede mandar un `AT` y ver si contesta `OK`. Ése es el primer diálogo real con la radio.
3. Con el canal AT vivo, lo siguiente es exponerlo como un tty en Linux para que hable un RIL
   (oFono/ModemManager) — territorio de **M2/M3**.

Ojo al `tx.length`: los anillos se inicializan con `rx.length = tx.length = 16K` y ambos punteros a
0; el productor avanza `write` y el consumidor `read`, con vuelta al llegar a `length`.
