# Estado al cerrar (2026-07-31) y por dónde seguir

## Dónde estamos

El módem del krillin **arranca, no excepciona, habla AT por un tty de Linux y tiene la radio
encendida midiendo señal**. En una sesión pasó de reventar en `nvram_io.c` a esto:

| hito | evidencia |
|---|---|
| **M1** — el MD arranca | `*** HS2 LOGRADO: NORMAL_BOOT_ID ***`, 1572 ops FS servidas |
| **L4 superado** | cero excepciones (`spm_md_ex` → `tipo=0 INVALID`) tras 4 min de servicio |
| **Canal AT** | `/dev/ttyCCCI0`: `ATI` → `MOLY.WR8.W1449.MD.WG.MP.V1` |
| **Radio ON** | `AT+CFUN=1` → `+CFUN: 1`; **`+CSQ: 29`** (≈ −55 dBm, no `99`) |

Ese `+CSQ` con nivel concreto es la señal de que **el frontend de RF y la calibración que servimos
desde la NVRAM funcionan**: los `CALIBRAT` que tanto costó servir bien (el `0x1010`, el `READ`
desplazado, el registro 161) están cumpliendo su función real.

Kernel **#102**. Driver en `mt6582-spm-H9.c` (snapshot en este directorio; el árbol vivo está en la
Pi, que **no es git**).

## ⛔ Bloqueo actual: no hay SIM que quepa

El teléfono usa **micro-SIM** y la SIM personal del usuario es **nano-SIM**. Sin tarjeta:

```
AT+CPIN?  ->  ERROR
AT+CREG?  ->  +CREG: 0,0
AT+COPS?  ->  +COPS: 0
```

Todo eso es **lo esperado sin SIM**, no un fallo. Se desbloquea con un **adaptador nano→micro**
(un par de euros) o cualquier micro-SIM, aunque sea de prepago sin saldo: para `+CPIN: READY` y ver
si engancha en red basta con que la tarjeta exista.

⚠️ Cuando haya SIM, ojo: este móvil es **dual-SIM**. Si el módem no la ve con la tarjeta puesta,
tocará mirar los comandos MTK del slot antes de dar por malo el bring-up.

## ⚠️ La imagen flasheada ahora mismo es de DEPURACIÓN

`boot-uart-nousb.img` lleva el nodo `usb@11200000` **desactivado** (dtb `ganador-h2b-nousb.dtb`) para
que la UART sobreviva. Consecuencia: **no hay red por USB**, sólo WiFi. Para volver a la normal hay
que reempaquetar con `ganador-h2b.dtb`.

## Lo que hace falta antes de oFono: que el módem arranque SOLO

Hoy el bring-up es **manual y efímero**: hay que escribir en seis ficheros de sysfs
(`poweroff → poweron → load → remap → release → hs2`) y encima poner `spm_tty_enable=1` **antes**.
Nada de eso sobrevive a un reinicio.

Para que oFono o ModemManager sirvan de algo, el driver tiene que:

1. **Arrancar el MD solo** (en el `probe`, o en un `init` de OpenRC que haga la secuencia).
2. **Mantener el hilo de servicio vivo** desde ese momento, no sólo tras un `hs2` manual.
3. **Dejar `/dev/ttyCCCI0` siempre disponible** — ya se registra en el `probe` (H13l), eso está.
4. Quitar el andamiaje de depuración del camino normal (`spm_tty_debug`, `spm_fs_dumpresp`, los
   volcados) o dejarlo apagado por defecto, que ya lo está.

Eso es trabajo de driver, no necesita SIM, y es el prerrequisito real de **M3**.

## Parámetros que hay que fijar hoy (recordatorio)

```sh
S=/sys/module/mt6582_spm/parameters
echo 1 > $S/spm_tty_enable        # ANTES del ciclo; por defecto 0
echo 1 > $S/spm_fs_quiet
echo 1500 > $S/spm_fs_slow_iters
echo 20000 > $S/spm_fs_post_hs2_iters
echo 1 > $S/spm_md_smem_clr
echo 1 > $S/spm_md_poweroff; sleep 1; echo 1 > $S/spm_md_poweron; sleep 1
for p in spm_md_load spm_md_remap spm_md_release spm_md_hs2; do echo 1 > $S/$p; done
```

Scripts en el móvil: `/root/rxtest2.sh` (ciclo + lector + escritor), `/root/attest3.sh` (diálogo AT),
`/root/radio.sh` (encender radio y consultar estado).

## Herramientas de diagnóstico que han demostrado su valor

- **`spm_md_ex`** — lee y decodifica el registro de excepción del MD (fichero, línea, código). Fue lo
  que desatascó el frente del módem.
- **UART a 921600 en `/dev/ttyAMA0` de la Pi** — imprescindible para fallos mudos. Ver `H14` para las
  tres trampas (el `musb` mata la serie; el LK ignora el cmdline del boot image; `cat` bufferiza).
- **`spm_tty_dump`**, **`spm_fs_trace`**, `sysrq-W`.

**La regla que resume las dos veces que nos atascamos**: cuando un fallo es **mudo**, abrir un canal
de diagnóstico **antes** de formular hipótesis. Las dos veces que se ignoró costó horas de
compilar-flashear-reiniciar para descartar cosas que no eran.
