# H12 — diálogo AT con el baseband: le mandamos `AT` y contesta `OK`

> Continúa de `H11-EL-MODEM-HABLA-AT-0730.md`.
> Kernel **#85**, imagen `~/mainline/pkg/boot-modem-m1.img` (md5 `a291a09f6955d9330351421de50490bd`).

**El módem responde.** Ida y vuelta completa por el canal AT del CCCI:

```
H12 RX drenado: read 0 -> 26, ACK por ch11
H12 TX 'AT' (3 B) tx.write 0 -> 3, timbre ch12 -> 0
H11 TTY1 UART2/modem-AT rx[r=26 w=32] tx[r=3 w=3]
H11   +001a: 0d 0a 4f 4b 0d 0a      |..OK..|
```

El `tx[r=3 w=3]` es la mitad que se suele pasar por alto: el MD **consumió** nuestros 3 bytes
(avanzó `tx.read` hasta donde habíamos dejado `tx.write`). No es que apareciera un `OK` por
casualidad: hubo lectura, proceso y respuesta.

Y con preguntas de verdad:

```
ATI      ->  MTK2
             MOLY.WR8.W1449.MD.WG.MP.V1
             OK

AT+CGMR  ->  +CGMR: MOLY.WR8.W1449.MD.WG.MP.V1, 2015/02/11 17:50
             OK
```

Se identifica **exactamente** con el firmware que le cargamos
(`~/mainline/downstream/stock-firmware-0713/modem.img`), con su fecha de compilación. No queda
margen de duda sobre qué está corriendo.

---

## Cómo se manda

Del stock (`ccci_tty.c` + las macros de `ccci_layer.h`), los datos **siempre** viajan por el anillo
de memoria compartida y el mensaje CCCI es sólo el **timbre**:

| acción | mensaje CCCI | antes |
|---|---|---|
| ACK de recepción | **mailbox** `{0xFFFFFFFF, id}` en `CCCI_UART2_RX_ACK` (11) | poner `rx.read` |
| enviar | **stream** `{addr=0, len}` en `CCCI_UART2_TX` (12) | copiar a `tx_buffer` y avanzar `tx.write` |

El `id` del mailbox es `tty_instance->channel`, que para el puerto del módem vale **1** — y encaja
con el `rsv=1` que traen los timbres que manda el MD por `lch 10`.

Un mensaje CCCI son cuatro palabras `{data0, data1, canal_lógico, reserved}` escritas en un canal
físico TX libre (`BUSY` en `ccif+0x04`), y luego `TCHNUM` (`ccif+0x0c`) para dispararlo — el mismo
camino que ya usaba la respuesta del FS.

**Importante**: hay que **drenar el RX y ACKear** antes o después de escribir; si no se consume, el
anillo se llena y el MD deja de emitir.

Uso:

```sh
echo AT      > /sys/module/mt6582_spm/parameters/spm_tty_at   # se le anade el CR
echo ATI     > /sys/module/mt6582_spm/parameters/spm_tty_at
echo 1       > /sys/module/mt6582_spm/parameters/spm_tty_dump # ver la respuesta
```

`spm_tty_at` drena+ACKea, escribe y toca el timbre; `spm_tty_dump` enseña los anillos.

## Detalle de compilación

`strlcpy` ya no existe en este kernel (7.0.12) — `snprintf`. Es el tipo de cosa que sólo aparece al
compilar; conviene mirar los warnings/errores del build antes de dar por bueno un parche.

## ▶️ Siguiente

El canal está probado en los dos sentidos, así que lo que falta es **plumbing**, no protocolo:

1. **Exponerlo como un tty de Linux** (`/dev/ttyCCCI0` o similar): un `tty_driver` cuyo `write`
   copie a `tx_buffer` + timbre, y una notificación de RX que empuje al `tty_port` cuando llegue el
   mensaje del `lch 10`. Hoy el RX se drena a mano desde `spm_tty_at`/`spm_tty_dump`.
2. Con el tty, **oFono o ModemManager** hablan solos: registro en red, SMS, llamadas → **M3**.
3. En paralelo, las tres `CCMNI` para datos IP → **M2**.

Ojo a un detalle de arquitectura: hoy todo esto cuelga del bucle de servicio de `spm_md_hs2`, que es
síncrono y termina. Para un tty de verdad hace falta que el servicio del CCCI viva en un **hilo o en
la interrupción** del CCIF, no en una llamada bloqueante desde sysfs.
