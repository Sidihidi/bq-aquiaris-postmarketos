# Parches 0730 — cierre del hito M1

| Fichero | Qué es |
|---|---|
| `patch_default9.py` | **H8a — `spm_fs_1010_mode = 9` por defecto.** Con el default anterior (0) el arranque se quedaba en 2 operaciones; con el 9 el MD monta su NVRAM entera y alcanza `NORMAL_BOOT_ID`. Verificado en HW (kernel #67, 894 peticiones FS con defaults puros). |
| `patch_1004.py`, `patch_enum.py` | de la tanda H7 de casa (WRITE y enumeración) |
| `build-modem.sh` | script de build del kernel del módem. Vive en `~/mainline/build-modem.sh` en la Pi (antes estaba en `/tmp` y se perdía). Lleva la guarda del `abootimg`: salida **visible** (falla si se redirige a `/dev/null`) y comprobación de que la imagen existe antes de seguir. |

Se aplican sobre `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c` (árbol de la Pi `.123`,
que no es git → esto es la foto de los cambios).
