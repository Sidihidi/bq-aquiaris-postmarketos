# 🔧 Guía mtkclient para el BQ Aquaris E4.5 (krillin / MT6582)

> Procedimiento concreto para la Pi (`cpcd@192.168.0.123`), donde mtkclient YA
> está instalado en `~/mtkclient` (con venv y `mt6582_payload.bin`). mtkclient es
> el reemplazo libre de SP Flash Tool: habla con el BootROM/Preloader del SoC, así
> que funciona **pase lo que pase** con el sistema instalado.

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
El SoC enumera como `0e8d:2000` (preloader) o `0e8d:0003` (brom). Para cazarlo:
1. Teléfono **apagado** (o batería fuera).
2. En la Pi, lanzar el comando mtkclient **primero** (queda esperando "Waiting for device").
3. Conectar el USB **manteniendo Vol−** (algunos krillin: Vol+; probar ambos).
   Con batería fuera, a veces basta enchufar. mtkclient imprime
   `Device detected :)` / `Preloader - Detected regular mode!`.

## 3. Confirmar que el secure boot está APAGADO (efuse sin quemar)
```sh
$MTK gettargetconfig
# Esperado en krillin: SBC disabled / DAA disabled  → boot.img sin firmar arranca.
# (Si saliera SBC enabled, habría que usar exploits kamakiri — NO debería ser el caso.)
```

## 4. ⭐ Backup golden (HACER ANTES DE TOCAR NADA)
```sh
mkdir -p ~/golden && cd ~/golden
# críticos e irremplazables (por nombre):
$MTK r preloader preloader.bin
$MTK r seccfg    seccfg.bin
$MTK r lk        lk.bin          # = uboot; ojo: a veces la partición se llama 'uboot'
$MTK r nvram     nvram.bin
$MTK r proinfo   proinfo.bin
$MTK r protect1  protect1.bin ; $MTK r protect2 protect2.bin
$MTK r boot      boot.bin ; $MTK r recovery recovery.bin
# y/o el flash entero (lento pero total):  $MTK rf full_flash.bin
$MTK printgpt    # ver nombres/offsets reales de las particiones
```
**Guardar `~/golden/` fuera del teléfono** (scp a Mac, y/o subir los pequeños al
repo: preloader/seccfg/nvram/proinfo/lk son KB–MB).

> ⚠️ **Gotcha MT6582**: la tabla MBR reporta tamaños imposibles (data ~2 PB,
> entrada extendida 0xFFFFFFFF). `rl` (leer TODAS) puede fallar en `userdata`.
> Workaround: leer por nombre las que importan, o por offset/longitud con `ro`/`rs`
> usando el mapa del `/proc/dumchar_info` (abajo). Para escribir kernels nunca
> tocamos userdata, así que no nos afecta.

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
