# H14 — cómo ver el pánico: netconsole **no sirve**, la UART sí (falta el cable)

> Continúa de `H13...`. El problema de fondo del tramo del tty es que el reinicio es **mudo**: sin
> `oops` en `dmesg` y sin nada en `pstore`. Cada hipótesis costaba compilar, flashear, reiniciar y a
> veces un encendido manual, y devolvía **un solo bit**. Esto es el intento de abrir un canal.

## netconsole: se registra pero no emite

El kernel lo trae listo (`CONFIG_NETCONSOLE=y`, `CONFIG_NETCONSOLE_DYNAMIC=y`,
`CONFIG_NETCONSOLE_EXTENDED_LOG=y`), así que se configura en caliente por configfs:

```sh
N=/sys/kernel/config/netconsole/krillin
mkdir $N
echo usb0              > $N/dev_name
echo 172.16.42.1       > $N/local_ip
echo 6665              > $N/local_port
echo 172.16.42.2       > $N/remote_ip      # la Pi
echo 6666              > $N/remote_port
echo 56:a7:6b:31:f3:5b > $N/remote_mac     # MAC de usb0 en la Pi
echo 1                 > $N/enabled
```

Y **funciona en apariencia**: `dmesg` dice `netconsole: network logging started` y aparece en
`/proc/consoles` como `netcon_ext0 -W- (E  N   )`, o sea habilitada.

**Pero no sale un solo paquete.** Verificado con `tcpdump` en la Pi, en las **dos** interfaces:

| transporte | resultado |
|---|---|
| `usb0` (gadget USB, 172.16.42.x) | `0 packets captured` |
| `wlan0` (nuestro driver WiFi) | `0 packets captured` |

⇒ `netpoll` no funciona ni sobre el gadget USB (`u_ether`) ni sobre el driver WiFi portado. No es un
problema de configuración ni del receptor (se probó con `nc -u -l` y con `tcpdump` directo).

⚠️ Detalle que despista: al configurar imprime `local ethernet address '00:00:00:00:00:00'`, pero eso
es sólo el valor de configuración — `usb0` sí tiene MAC (`e2:66:75:8b:91:e7`). No es la causa.

## La UART sí está viva — sólo falta enchufarla

Mirando `/proc/consoles`:

```
mtk8250              -W- (ECB p  )     <- la consola UART, ACTIVA (E=enabled, C=preferida, B=boot)
netcon_ext0          -W- (E  N   )
ramoops-1            -W- (E   p a)
tty0                 -WU (E   p  )
```

**La consola serie está registrada y habilitada**, así que un pánico se imprimiría por ella. El
cmdline lleva `printk.disable_uart=1`, pero ese parámetro **no existe en este kernel** (no hay
`/sys/module/printk/parameters/disable_uart`), así que es inerte — de ahí que `mtk8250` siga activa.

Lo único que falta es **el cable**, que existe y ya se usó en toda la saga de Darwin:

- móvil ↔ Pi, **921600 8N1**, la Pi lo lee en `/dev/ttyAMA0`
- helpers ya escritos: `~/uart-console.sh`, `~/uart-loopback.sh`
- la UART del krillin es de **3,3 V**: no hace falta divisor (ver la memoria de Darwin)
- sólo se necesita **RX en la Pi** (ver la salida del móvil); el problema conocido de que el móvil no
  reciba lo tecleado no afecta aquí

Comprobación de que está conectado:

```sh
stty -F /dev/ttyAMA0 921600 raw -echo
timeout 15 cat /dev/ttyAMA0 > /tmp/uart.log &
# en el movil:
printf '<1>HOLA-POR-LA-UART\n' > /dev/kmsg
```

Hoy esto da fichero vacío ⇒ **cable desconectado**.

## ▶️ Qué hacer al retomar

1. **Conectar el cable UART** entre el móvil y la Pi y verificar con la prueba de arriba.
2. Con la UART capturando, repetir el escenario del tty (`spm_tty_enable=1`, `spm_tty_push=1`,
   lector + escritura). El pánico saldrá por serie aunque el sistema se muera.
3. Sólo entonces volver a tocar `spm_tty_rx`. Seis hipótesis plausibles ya han caído sin datos; la
   séptima no merece la pena a ciegas.

Alternativa si el cable no está a mano: **cambiar de enfoque** y exponer el canal como **chardev**
(`/dev/ccci-at`) en vez de tty, que elimina de golpe toda la interacción con el `tty_port` — que es
donde está el fallo.
