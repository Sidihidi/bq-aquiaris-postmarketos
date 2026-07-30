# Parches 0730 — cierre del hito M1

| Fichero | Qué es |
|---|---|
| `patch_default9.py` | **H8a — `spm_fs_1010_mode = 9` por defecto.** Con el default anterior (0) el arranque se quedaba en 2 operaciones; con el 9 el MD monta su NVRAM entera y alcanza `NORMAL_BOOT_ID`. Verificado en HW (kernel #67, 894 peticiones FS con defaults puros). |
| `patch_1004.py`, `patch_enum.py` | de la tanda H7 de casa (WRITE y enumeración) |
| `build-modem.sh` | script de build del kernel del módem. Vive en `~/mainline/build-modem.sh` en la Pi (antes estaba en `/tmp` y se perdía). Lleva la guarda del `abootimg`: salida **visible** (falla si se redirige a `/dev/null`) y comprobación de que la imagen existe antes de seguir. |

Se aplican sobre `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c` (árbol de la Pi `.123`,
que no es git → esto es la foto de los cambios).

## Auditoría (kernels #77-#79)

| Fichero | Qué es |
|---|---|
| `patch_audit.py` | **H8j** — elimina el `p0` muerto (el comentario obsoleto que causó H8i), renombra `p1`→`len0`, documenta la estructura de campos en el sitio donde se lee, documenta el `whence` del SEEK y arregla el `kstrtou32`. |
| `patch_indent.py` | `-Wmisleading-indentation` en el `iounmap` de `spm_md_release`. Cosmético, pero tapaba warnings reales. |
| `patch_trace.py` | **H8k** — saca el volcado de la traza del `if (!done)`: el contador de ops solo se imprimía al fallar. |
| `patch_ring.py` | **H8k** — `spm_fslog` pasa a anillo real + `spm_fs_total` sin tope. Antes saturaba en 512 y los "últimos 48" eran del medio del arranque. |

Resultado: el fichero compila **sin warnings** y la medida con defaults puros es
**900 ops FS + `NORMAL_BOOT_ID`** (kernel #79).
