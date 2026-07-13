# 2026-07-13 — Rescate de brick (seccfg borrado) por BROM + hallazgo del build de pmOS

> Sesión Mac (Pi `.123`, la del árbol viejo del 0702). El móvil se quedó en bootloop
> ("logo BQ → se apaga") y se recuperó **entero por BROM (mtkclient)**. Termina arrancando
> **LineageOS sin problema** → la cadena de arranque está sana. **El build de pmOS de la `.123`
> NO arranca** (hay que depurarlo en casa contra el árbol `.38`).

## Qué pasó (causa raíz)
Trabajando en paralelo: (a) yo flasheé al **sector 83968** (`0x2900000`) un kernel con el `mt6323`
como `interrupt-controller`; (b) **en otra sesión se restauraba la NVRAM con SP Flash Tool**. El
bootloop resultó ser de **nivel LK** (logo→apaga, antes del kernel). Diagnóstico por BROM:
**el `seccfg` estaba a CEROS** — el SP Flash Tool lo borró. Un `seccfg` inválido → el LK no valida
la config de seguridad → apaga en bucle. (Ni mi kernel ni el `sec_ro` eran la causa del *bootloop*.)

## Recuperación por BROM (mtkclient en `.123`, `~/mtkclient`, venv) — TODO con `wo` (NUNCA `wf`)
**Traducción de offsets verificada** (3 anclajes): `offset_wo = physical_start_addr(scatter) + 0xB80000`.

| Partición | offset `wo` | fuente restaurada | md5 |
|---|---|---|---|
| MBR | `0xB80000` | `1.5-1.2_krillin/MBR` | `0da80476…` |
| EBR1 | `0xC00000` | `1.5-1.2_krillin/EBR1` | `7676d7af…` |
| SECCFG | `0x2880000` | `artifacts/golden/seccfg.img` (UNLOCK, `lock_state=3`) | `90c6e927…` |
| UBOOT (LK) | `0x28A0000` | ya estaba bien (KitKat 1.5.2, `88 16 88 58 "LK"`) | `f0f3a93e` |
| BOOTIMG | `0x2900000` | nuestro boot / stock según prueba | — |
| SEC_RO | `0x5100000` | `1.5-1.2_krillin/secro.img` | `4f307dfc…` |

Comando tipo: `sudo ~/mtkclient/venv/bin/python ~/mtkclient/mtk.py wo <off> <len> <img>`
(entrar BROM en el bucle de espera: batería fuera→dentro, mantener **Vol−**, conectar USB).

### Gotchas del rescate (para no repetir)
- **`w preloader` / lecturas por NOMBRE FALLAN** en este legacy (sin GPT): "Couldn't detect partition".
  → usar **offsets crudos `wo`/`ro`** siempre. (El preloader en boot1 no se pudo tocar por nombre;
  no hizo falta — el preloader original estaba bien.)
- **`pkill -f mtk.py` se auto-mata** (la propia orden ssh contiene "mtk.py") → matar por PID.
- **`USBError Resource busy`** = proceso mtkclient anterior colgado agarrando el USB → matar por PID + reintentar.
- **fastboot se colgó** a mitad de un `flash boot` tras muchos ciclos → `wo` por BROM fue más fiable (8-9 MB/s).
- **NO restaurar el `uboot.img` golden**: es Lollipop (verifica firma → bootloop). El bueno es
  `~/stock-1.5.2/lk.bin` (KitKat, `f0f3a93e`). NO tocar el preloader salvo necesidad (recuperable por BROM igual).

## Hallazgo clave: el build de pmOS de la `.123` NO arranca
Con la cadena ya sana, se probó: kernel **#232** (dts revertido) y un **build "limpio" recompilado** →
**ambos "logo→apaga"**. En cambio **el boot de LineageOS arranca perfecto** → la cadena/preloader/LK/tablas
están BIEN; **el problema es el propio pmOS de la `.123`** (kernel/DTB/initramfs). A depurar en casa:
comparar el árbol `.123` (viejo, 0702) contra el `.38`/el **kernel-patchset** (commit `5c947df`, delta
completo). Probable: el árbol `.123` arrastra algo roto del 0702 o un DTS/initramfs que no cuadra con esta SD.

## Estado final del móvil
- **Arranca LineageOS** (eMMC) sin problema. Recuperado, NO brickeado.
- pmOS-en-SD (`mmcblk1p1`) intacto en la tarjeta; falta un **boot.img de pmOS que arranque** (usar el del `.38`).
- Nota: esta sesión Mac corría sobre un checkout **desfasado 11 días** (0702). Sus "ports" de vibrador/LEDs/
  thermal/audio son **redundantes** (main ya los tiene mejor; vibración RESUELTA el 0706). Commits colgantes
  por si acaso: `e449def` (vibr/LEDs), `de37961` (thermal), `1db3ecd` (audio), `724a875` (EAPOL WiFi driver A),
  `0179cc1` (limpieza). No integrar sin revisar contra main.

## Para seguir en casa
1. Flashear un **boot.img de pmOS del árbol `.38`** (el que sí arranca) al sector 83968 y confirmar pmOS.
2. Si se quiere pmOS desde la `.123`: depurar por qué su kernel no arranca (dmesg por pstore/ramoops; comparar
   `.config` y DTS contra el patchset `5c947df`).
3. El firmware stock 1.5 (`~/Downloads/1.5-1.2_krillin` en el Mac) sirve de red de seguridad para MBR/EBR/secro/lk.
