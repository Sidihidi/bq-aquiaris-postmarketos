# Herramientas — krillin (pmOS mainline)

Scripts de **diagnóstico y recuperación**. Se ejecutan **en la Pi** (`cpcd@192.168.0.123`); hacen SSH al teléfono por USB (`root@172.16.42.1`).

| Script | Qué hace | Uso |
|---|---|---|
| `krillin-diag.sh` | Diagnóstico completo: boot, GUI (phoc/phosh), CONSYS (BT/WiFi/GPS), memoria, errores dmesg, pstore | `bash krillin-diag.sh` |
| `krillin-recover.sh` | Recuperar el teléfono | `bash krillin-recover.sh {softreboot\|forcereboot\|fastboot <img>\|brom}` |
| `krillin-logs.sh` | Captura dmesg + rc.log + phosh-boot.log + pstore a `~/krillin-logs/<fecha>/` | `bash krillin-logs.sh` |

## Recetas rápidas
- **¿Qué le pasa al teléfono?** → `bash krillin-diag.sh`
- **No responde / apagado colgado** → `bash krillin-recover.sh forcereboot` (salta el apagado de OpenRC). Si ni eso, power-cycle.
- **Recuperar pmOS booteable** → `bash krillin-recover.sh fastboot boot-btifDMA-sd.img` (teléfono en fastboot).
- **Ladrillo total** → `bash krillin-recover.sh brom` + [GUIA-MTKCLIENT.md](../../GUIA-MTKCLIENT.md). **NUNCA flashear el preloader.**

## Notas técnicas (lecciones aprendidas)
- **Reconexión**: todos hacen `ip addr replace 172.16.42.2/24 dev usb0` antes del SSH (la interfaz gadget se cae).
- **`pidof`, no `pgrep -x`**: en BusyBox, `sshd`/`phoc`/`phosh` reescriben su título de proceso → `pgrep -x` da 0 aunque estén vivos. `pidof` acierta.
- **`reboot -f`**: fuerza el reinicio saltando el apagado de OpenRC (que a veces se cuelga en *"Stopping System Message Bus"*).
- **NO supervisores en background** en `local.d`: pelean con el apagado. Si hace falta supervisión, OpenRC `supervise-daemon`.
