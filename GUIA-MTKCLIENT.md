# 🔧 Guía mtkclient para el BQ Aquaris E4.5 (krillin / MT6582)

> Procedimiento concreto para la Pi (`cpcd@192.168.0.123`), donde mtkclient YA
> está instalado en `~/mtkclient` (con venv y `mt6582_payload.bin`). mtkclient es
> el reemplazo libre de SP Flash Tool: habla con el BootROM/Preloader del SoC, así
> que funciona **pase lo que pase** con el sistema instalado.

> ⚠️ **CORRECCIÓN 2026-06-17 (importante):** en este equipo (DA legacy) los comandos
> `w <particion>` y `wf` **NO sirven**: `w` necesita la GPT (que no se lee) y **`wf`
> IGNORA `--offset` y escribe en el SECTOR 0** (machaca MBR/proinfo). **Para escribir
> una partición usar `wo <offset_hex> <length_hex> <fichero>`** (= `writeflash(addr)`).
> Offsets HEX: boot=`0x2900000`, uboot(LK)=`0x28A0000`, seccfg=`0x2880000`. Ej.:
> `sudo ~/mtkclient/venv/bin/python ~/mtkclient/mtk.py wo 0x2900000 0xD18800 boot.img`.
> Antes: `sudo systemctl stop ModemManager` y `~/mtkclient/hwparam.json` de `cpcd` (no root).
> Ver `mainline/HITO-PMIC-MT6323.md`. (Los ejemplos `w boot`/`w lk` de abajo quedan obsoletos.)

## 0. Atajos

```sh
MTK="~/mtkclient/venv/bin/python ~/mtkclient/mtk.py"   # entrypoint con venv
```
Comandos clave (verificados en la versión instalada):
`r` leer partición · `rl` leer todas a un dir · `rf` flash entero · `w` escribir
partición · `wl` escribir dir · `printgpt`/`gpt` tabla · `dumppreloader` ·
`gettargetconfig` (sbc/daa = estado secure boot) · `da seccfg {lock,unlock}`.

## 1. Preparar (una vez)
```sh
sudo cp ~/mtkclient/mtkclient/Library/*.rules /etc/udev/rules.d/ 2>/dev/null
sudo udevadm control --reload-rules && sudo udevadm trigger
# si el módulo cdc_acm acapara el puerto preloader, opcional: sudo rmmod qcaux 2>/dev/null
```

## 2. Entrar en modo BROM/Preloader (MT6582)
⚠️ **CLAVE en la Raspberry Pi 5 (todo xhci/RP1):** ejecutar mtkclient de forma
**INTERACTIVA** en una terminal (NO en background/automático) y reconectar el
móvil **justo cuando lo pida**. Si no, da `Couldn't get device configuration` en
bucle (no es hardware: es que nadie reconecta en la señal).
1. Teléfono **apagado** / batería fuera.
2. Lanzar el comando mtkclient → imprime
   `Waiting for PreLoader VCOM, please reconnect mobile/iot device to brom mode`.
3. EN ESE MOMENTO: enchufar el USB **manteniendo Vol−** (probar Vol+ si no).
   → `Device detected :)` / `Preloader - Detected regular mode!` / `BROM mode detected`.
4. Comprobado: enumera `0e8d:0003` (brom) o `2000` (preloader); ambos valen si
   reconectas en la señal. (cdc_acm/option/usbserial blacklisted + ModemManager
   parado ayuda, pero lo decisivo es el reconnect-on-cue.)

## 3. Confirmar que el secure boot está APAGADO (efuse sin quemar)
```sh
$MTK gettargetconfig
# Esperado en krillin: SBC disabled / DAA disabled  → boot.img sin firmar arranca.
# (Si saliera SBC enabled, habría que usar exploits kamakiri — NO debería ser el caso.)
```

## 4. ⭐ Backup golden — PROCEDIMIENTO REAL (probado 2026-06-16)
En el krillin, mtkclient **no puede leer la GPT/particiones** (bug del MBR
MT6582 → "Couldn't get gpt"). Así que `rl`/`r <nombre>` no sirven; cae a volcar
el **flash entero** (área de usuario, 7.5 GB). Pero **las particiones críticas
están todas en los primeros ~90 MB**, así que:

```sh
# 1) lanzar el volcado (cae a flash completo) y CORTAR con Ctrl+C tras ~700 MB:
$MTK rl ~/golden --skip usrdata,userdata,cache,android,system,data
#    → "No partition table detected, reading flash instead to ~/golden/flash.bin"
#    deja que pase de ~0x6000000 (96 MB) y pulsa Ctrl+C. (~700 MB sobra.)

# 2) recortar las particiones del flash.bin con dd. ⚠️ CALIBRACIÓN DE OFFSETS:
#    los offsets del /proc/dumchar_info NO coinciden con el volcado:
#    real_offset = dumchar_offset + 0xB80000  (verificado por firmas)
cd ~/golden
c() { dd if=flash.bin of="$1.img" skip="$2" count="$3" iflag=skip_bytes,count_bytes bs=4M status=none; }
c proinfo   13107200  3145728     # dumchar 0x100000  +0xB80000
c nvram     16252928  5242880     # dumchar 0x400000
c protect_f 21495808  10485760    # dumchar 0x900000
c protect_s 31981568  10485760    # dumchar 0x1300000
c seccfg    42467328  131072      # dumchar 0x1d00000  → AND_SECCFG
c uboot     42598400  393216      # dumchar 0x1d20000  → 88168858 "LK"
c boot      42991616  20971520    # dumchar 0x1d80000  → ANDROID!
c recovery  63963136  20971520    # dumchar 0x3180000  → ANDROID!
rm -f flash.bin   # recuperar los 700 MB
```
Verificar firmas: `head -c8 boot.img|od -c` → `ANDROID!`; seccfg → `AND_SECCFG`.
**El `preloader` NO está en este volcado** (vive en eMMC boot0, no en el área de
usuario) — pero es la partición que NUNCA se toca, y tenemos `lk.bin` stock.

**Guardar `~/golden/` fuera de la Pi** (scp al Mac → `artifacts/golden/`).
🔒 **NO subir `nvram`/`proinfo` a GitHub** (llevan MAC/IMEI únicos; el `.gitignore`
ya excluye `*.img`).

> Nota de scripting: la aritmética `$(())` y las llaves `{}` se rompen al pasar
> por zsh(Mac)→ssh→bash. Usar heredoc `<<'EOF'` + offsets en **decimal** +
> `iflag=skip_bytes,count_bytes` (nada de cálculo de bloques).

## 5. Desbloquear el bootloader (seccfg)
```sh
$MTK da seccfg unlock
# si "Couldn't detect existing seccfg partition": escribir un seccfg unlocked a
# mano (offset 0x1d00000) o restaurar uno unlocked conocido. El krillin SÍ tiene
# seccfg. Fallback con el LK arrancado: fastboot oem unlock.
```

## 6. Flashear un kernel (lo que sustituye al ciclo stock→TWRP→ROM)
```sh
# restaurar pmOS estable:
$MTK w boot ~/pmos-artifacts/boot-pstore.img
# o probar un mainline:
$MTK w boot ~/mainline/pkg/boot-mainline-v25.img
$MTK reset       # reiniciar
```
Con esto, un kernel que no arranca (como la v25) se arregla con **un comando**,
sin Android stock de por medio. Eso es lo que faltó en el softbrick.

## 7. Recuperación total (si algo se tuerce)
```sh
$MTK w boot ~/golden/boot.bin          # restaurar boot
$MTK w lk   ~/golden/lk.bin            # restaurar bootloader (BROM lo permite)
$MTK w seccfg ~/golden/seccfg.bin      # restaurar estado de lock
# preloader SOLO si imprescindible: $MTK w preloader ~/golden/preloader.bin
```
Mientras BootROM responda (siempre, salvo quemar el efuse del propio BROM —
algo que NO hacemos), el teléfono se recupera de todo.

## Mapa de particiones (de /proc/dumchar_info — offsets en bytes)
```
preloader  0x00000000  0x01400000   ← NO tocar salvo emergencia
seccfg     0x01d00000  0x00020000   ← lock/unlock
uboot(lk)  0x01d20000  0x00060000   ← bootloader LK
bootimg    0x01d80000  0x01400000   ← kernel (pmOS/mainline)
recovery   0x03180000  0x01400000   ← 2º slot de boot (Android/dual-boot)
android p5 0x05900000  0x40000000   ← root de pmOS
cache  p6  0x45900000  0x2bc00000
usrdata p7 0x71500000  hasta el fin  ← 5.7 GB (futura 2ª raíz / expansión)
```

## Checklist de la sesión de arranque (cuando el teléfono esté en BROM)
1. [ ] `gettargetconfig` → confirmar SBC disabled.
2. [ ] Backup golden (`~/golden/`) + scp fuera + subir los pequeños al repo.
3. [ ] `da seccfg unlock`.
4. [ ] `w boot boot-pstore.img` → recuperar pmOS estable. Verificar Sxmo.
5. [ ] (luego) retomar mainline v26 / M2b USB, o montar el dual-boot de 2 slots.

## Fuente
- mtkclient: https://github.com/bkerler/mtkclient
