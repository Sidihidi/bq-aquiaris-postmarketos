# Módem H5 — bucle de servicio CCCI OK; HS2 gated por el FS/NVRAM del MD (0719)

> Continuación de [H4-HS2-DATAABORT-RESUELTO-0718.md] (data abort = solape de regiones, RESUELTO).
> Añadido el **bucle de servicio CCCI** (ACK de los canales del MD). Resultado: el MD arranca sin
> abortar, inicializa, y **pide montar su filesystem "Z:\" (EFS/NVRAM)** — y bloquea HS2 esperando
> la respuesta del AP. El módem está BOOTEANDO de verdad; el siguiente hito es el proxy FS.

## Lo añadido: bucle de servicio CCCI (kernel #25)
El `spm_md_hs2` era one-shot. Ahora, tras enviar runtime+MD_INIT_START_BOOT, entra en un bucle 8s:
lee `RCHNUM` (0x10), y por cada canal con dato: lee `RXCHDATA[ch]` (0x180+ch*16), lo loguea, y hace
**`CCIF_ACK(0x14)=(1<<ch)`** (= `__ccif_v1_ack` del stock) para LIBERAR el canal TX del MD (sin ACK,
el canal queda ocupado y el MD no puede mandar el siguiente → se bloquea). Busca el boot-ready =
mensaje de control (lch=0) con id=NORMAL_BOOT_ID(0) y rsv!=MD_INIT_CHK_ID (para distinguirlo del HS1).

## Resultado (kernel #25)
```
CCCI RX ch1: d0=4160e000 id=00000020 lch=0000000e rsv=00000000
H4 HS2: sin boot-ready tras 8s. RCHNUM=00000000
```
- El MD manda UN mensaje: canal lógico **14 = CCCI_FS_RX** (filesystem), data0 = 0x4160e000 = la base
  de la región FS que le dimos, id/len=0x20.
- Lo ACKeamos → RCHNUM vuelve a 0 → pero el MD NO manda más (ni boot-ready ni retry): **espera una
  RESPUESTA al mensaje FS**, no solo el ACK.

## Qué pide el MD (leído de la región FS @0xB960E000 por devmem)
```
+0x00: 0x0000100e 0x00000002 0x00000008   (cabecera fs_stream: ops/len/idx)
+0x0c: 5a 00 3a 00 5c 00 00 00 = "Z:\\" (UTF-16LE, null-term)  + flags 0x04 0x03
```
El MD abre **"Z:\"** = la raíz de su EFS (Embedded File System / NVRAM). MOLY guarda su NVRAM
(calibración RF, IMEI, config) en ficheros "Z:\..." que en Android **proxya un daemon userspace
`ccci_fsd`** (abre `/dev/ccci_fs`, lee la petición del MD de la shared-mem FS, hace el file-op, escribe
la respuesta y manda un mensaje FS de vuelta). Sin ese proxy, el MD bloquea el boot en el mount del EFS.

## Estado del boot del MD (traza completa lograda)
1. Power/remap/release ✅ → 2. HS1 (MD_INIT_START_BOOT) ✅ → 3. runtime ACEPTADO ✅ (sin abort, fix del
solape) → 4. MD arranca, inicializa → 5. **pide montar "Z:\" (EFS/NVRAM)** ⏳ → [HS2 bloqueado aquí].
El módem procesa MOLY hasta la fase de filesystem = MUY avanzado (primer MT6582 con el MD aquí en mainline).

## Siguiente hito: proxy FS (EFS/NVRAM)
Responder los mensajes FS del MD. Opciones:
- **Mínimo para HS2**: implementar en el kernel una respuesta mínima al open de "Z:\" (código de
  resultado + mensaje FS de vuelta) y ver si el MD avanza a NORMAL_BOOT_ID. RE necesario: los `fs_ops`
  (open/read/write/close), el formato de respuesta en la shared-mem, y el mensaje FS de ack.
  Fuente: `ccci_fs_main.c` (kernel) + el protocolo que espera `ccci_fsd` (userspace).
- **Completo**: portar/emular `ccci_fsd` con un backing store para "Z:\" = la NVRAM real del modem
  (que YA tenemos extraída — ver [[krillin-device-identity]] para los IMEIs, y la partición nvram).
  Esto da NVRAM real = calibración RF correcta = imprescindible para señal.

## HW / coordinación
Móvil restaurado a diario (#14). Fix + bucle en la COPIA LOCAL `spm-sweep`; el `spm-H1.c` compartido
NO tocado. Driver de test con el bucle: mismo que `mt6582-spm-H1.c.dataabort-fix-0718` + el bucle de
servicio (writels del bucle documentados arriba). El Mac debería integrar ambos.

**★INTEGRADO (Mac, 0720)**: ambos ya en el `mt6582-spm-H1.c` compartido del repo (data-abort fix
byte-idéntico + bucle de servicio CCCI `< 8 && rch`/`msleep(25)`/ACK-al-final = la versión tested de
casa). Compila RC=0 en la Pi (`drivers/soc/mediatek/mt6582-spm.o`). Confirmado que la Pi ya tenía esta
misma versión en el árbol; el commit solo la sube al repo (antes tenía el `0x2000` buggy + poll one-shot).
Siguiente = proxy FS (responder el open de "Z:\\").

*2026-07-19, sesión Windows (Fable 5). Data abort resuelto + servicio CCCI; HS2 gated por el EFS/NVRAM
del MD. Siguiente: proxy FS.*
