# Parches de la sesión 0729 — proxy FS del módem

Se aplican en orden sobre `~/mainline/linux-7.0.12/drivers/soc/mediatek/mt6582-spm.c`
(el árbol de la Pi `.123`, que **no es git**, así que esto es la foto de los cambios).

Contexto y razonamiento: `../ESTADO-Y-TRASPASO-0729.md`.

| Orden | Parche | Qué hace |
|---|---|---|
| 1 | `patch_smem.py` | H6p — volcado/búsqueda de texto en la SMEM (encontró `fs_ccci.c` = el MD SÍ excepciona) |
| 2 | `patch_smem2.py` | H6q — visor hex interactivo de la SMEM (`echo <offset_hex> > spm_md_smem`) |
| 3 | `patch_p0.py` + `patch_p0b.py` | H6r/H6s — **`spm_md_smem_clr`** (limpiar el registro; sin esto se comparan medidas rancias) y control de `+4` |
| 4 | `patch_shape.py` | H6t — respuesta de forma corta (modos 9/10): no tocar `+4` + longitud corta → **13 ops** |
| 5 | `patch_resolve.py` | H6u — resolución de rutas (modos 11/12/13). Refutó que el `1010` deba devolver un path |
| 6 | `patch_val.py` | H6v — `spm_fs_1010_val`: barrido del valor devuelto. Con bit31 el MD entra en la cascada |
| 7 | `patch_fix3.py` | H6w — quita la marca de error con bit31, abre directorios, enruta las 17 escrituras a `+4` |
| 8 | `patch_nf.py` | H6x — `+4` = nº de campos de la RESPUESTA, decidido por cada handler |
| 9 | `patch_def.py` | H6y — respuesta mínima válida para ops sin handler → **de 7 a 21 ops** |
| 10 | `patch_1009.py` + `patch_1009b.py` | H6z — `1009` con 2 campos y tamaño en 32 bits → **de 21 a 30 ops** |
| 11 | `patch_defnf.py` | H7a — contador de campos genérico conmutable |
| 12 | `patch_1012b.py` + `patch_decl.py` | H7b — `case 0x1012` con su propio contador (=3) → **32 ops** |
| 13 | `patch_ov.py` | H7c — override genérico por op (`spm_fs_ov_op`/`spm_fs_ov_nf`), reutilizable |

Herramientas de RE (no modifican el kernel):
- `re4.py` — localiza los `LDR` PC-relativos que cargan un literal → encuentra la función culpable.
- `dec12.py` — decodifica el ground-truth del `ccci_fsd` (`gd-boot-full.out`): op, `c`, `len` y rutas UTF-16.
