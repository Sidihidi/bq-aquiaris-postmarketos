# 🔓 EMPIEZA AQUÍ — Desbloqueo persistente (seccfg) del BQ E4.5

> El único muro que queda para volver a pmOS/mainline. Todo lo demás ya funciona
> (port pmOS, mainline arranca SMP, mtk-sd diagnosticado). Lee también
> CONTINUAR_MAINLINE.md y la memoria del proyecto.

## El problema en una frase

El `fastboot oem unlock` de este lk 2.0.1 **no es persistente** (el lk falla al
escribir `seccfg`: string `write seccfg fail`). Al reiniciar, el lk arranca
**bloqueado** y **verifica la firma de `boot` Y de `recovery`** → rechaza todo
kernel no firmado por BQ (pmOS, mainline, LineageOS, TWRP) → bootloop. Solo
arrancan imágenes firmadas (Android stock, recovery stock).

## El plan (próxima sesión, con presupuesto)

1. **Investigar el formato `seccfg` del MT6582** (agente de investigación):
   - Fuente del lk MT6582: buscar `sec_cfg.h` / `seccfg` en repos lk de mt6582
     (l33tnoob/MT65x2_kernel_lk ya descargado en research_mt6582_lk/, buscar la
     struct `SEC_CFG` y los valores de `lock_state`: LKS_DEFAULT/MP/UNLOCK/LOCK).
   - Objetivo: saber qué bytes poner en el `seccfg` (offset user 0x1d00000) para
     estado **UNLOCK persistente** que el lk respete y NO verifique boot.
   - Dump actual guardado: cpcd:/tmp/seccfg.bin (header AND_SECCFG_v + MMMM, ver=3,
     size=0x1860, lock_state cerca de offset 0x14-0x1c).

2. **Conseguir un entorno con acceso al block device** (para escribir seccfg):
   - Catch-22: TWRP (no firmado) no arranca locked. Pero `fastboot oem unlock`
     desbloquea EN ESA SESIÓN y permite flashear. Secuencia: oem unlock (confirmar
     Vol+ en pantalla) → `fastboot flash recovery twrp` → `fastboot oem
     reboot-recovery` SIN que se re-bloquee (probar si el unlock dura hasta el
     reboot-recovery; la 1ª vez TWRP SÍ arrancó así).
   - Alternativa: arrancar TWRP/Linux por BROM con mtkclient (`mtk da boot` o
     payload) sin tocar el lock — investigar si mtkclient puede bootear una imagen.

3. **Escribir el seccfg unlocked** desde TWRP/adb:
   `dd if=seccfg-unlocked.bin of=/dev/block/mmcblk0 bs=512 seek=59392` (user
   offset 0x1d00000). Verificar y reiniciar. Si el lk respeta el estado → boot
   custom arranca → pmOS/mainline vuelven.

4. **Alternativa si seccfg es intratable**: conseguir un **lk parcheado** que no
   verifique (el que el teléfono tenía bajo LineageOS pre-SPFT). Buscar en foros
   krillin/MT6582 un `lk.bin` "no-verify" / engineering, o parchear el lk stock
   (NOP la llamada a `sec_verify`). Flashear a UBOOT por fastboot(unlock)/BROM.

## Recursos ya en sitio

- Pi compilación: `cpcd@192.168.0.38` (pwd cpcdupct). mtkclient en ~/mtkclient.
- Pi pmOS: `juan@192.168.0.112` (clave id_ed25519_rpi). pmaports + pmbootstrap.
- En cpcd: ~/boot-pmos.img, ~/pmos-root.img (rootfs lista, ext4 stripped),
  ~/mainline/pkg/boot-mainline-v25.img, ~/twrp-recovery.img,
  ~/firmware-stock/{boot.img, lk.bin}, /tmp/seccfg.bin (dump).
- Firmware stock + scatter: C:\Users\jferr\Desktop\BQ_Aquaris_E4.5_2.0.1_...\Firmware
- LineageOS 13: C:\Users\jferr\Desktop\lineage-13.0-...-krillin (carpeta)
- TWRP: C:\Users\jferr\Desktop\TWRPv3.0.2.0\...\recovery.img

## Para dejar el teléfono USABLE ahora (Android)

Boot tiene un kernel custom (bootloop) y está re-bloqueado. Restaurar con
**SP Flash Tool** (Download Only + scatter, marca al menos BOOTIMG=boot.img) →
Android stock arranca (su boot está firmado). El recovery stock ya está puesto.
