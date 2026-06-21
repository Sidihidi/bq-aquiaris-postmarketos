# 🔀 Viabilidad de dual-boot Android + pmOS (+ mainline) en el BQ E4.5 (MT6582)

> Estudio 2026-06-16. Premisa confirmada: LK de KitKat arranca kernels custom;
> efuse secure-boot OFF; mtkclient/BROM como red de seguridad.

## TL;DR de viabilidad

| Pieza | ¿Viable? | Esfuerzo |
|---|---|---|
| **Mecanismo de arranque (2 slots)** | ✅ SÍ, GRATIS | nulo — el LK ya lo hace |
| **Reparto de almacenamiento** | ✅ Sí | **medio — es el trabajo real** |
| **Selección por software (sin teclas)** | ✅ Sí | bajo (bit RTC `reboot recovery`) |
| **3 SOs (Android+pmOS+mainline)** | ⚠️ Necesita menú en LK | alto (parchear LK / lk2nd / U-Boot) |

**Conclusión:** dual-boot de DOS sistemas es totalmente viable **sin tocar el
bootloader** — el LK del krillin ya arranca dos targets (`boot` y `recovery`). El
trabajo de verdad NO es el arranque, es **repartir el almacenamiento** (cada SO
necesita su raíz). Tres sistemas requerirían un bootloader con menú (otro nivel).

## 1. El mecanismo de arranque (lo fácil — gratis)

El LK del MT6582 arranca **dos imágenes** de forma nativa:
- **`boot`** (`0x1d80000`, 20 MB) → arranque normal.
- **`recovery`** (`0x3180000`, 20 MB) → con combo (mantener **Vol+** al encender) **o**
  un bit de modo en la RTC.

Eso ES un dual-boot de 2 slots sin parchear nada:
- `boot`    = SO A (p.ej. pmOS, o mainline)
- `recovery`= SO B (p.ej. Android KitKat)

**Cambiar de sistema:**
- **Por teclas**: mantener Vol+ al encender → arranca el de `recovery`.
- **Por software (headless, ideal para desarrollo)**: desde el SO activo,
  `reboot recovery` escribe el bit de modo en la RTC y al reiniciar arranca el
  otro slot. Sin tocar el teléfono.

> Ojo: ambos kernels deben ser "custom-friendly" → requiere el **LK de KitKat**
> (el de Lollipop verifica y haría bootloop). Ya es nuestra base.

## 2. El reto real: reparto del almacenamiento

Layout actual (scatter, offsets en `mmcblk0`):
```
p5 ANDROID/system  1.0 GB  (hoy: raíz de pmOS)
p6 CACHE           0.7 GB
p7 USRDATA         5.7 GB  (mayor parte libre)
```
Cada SO necesita su raíz/system. El conflicto: pmOS y Android stock **ambos
quieren p5**. Opciones:

**Opción A — recortar usrdata (recomendada).** Reparticionar p7 (5.7 GB) en dos:
- `p7a` data de Android (~2.5–3 GB)
- `p7b` raíz de pmOS (~2.5–3 GB)
Android usa p5(system)+p6(cache)+p7a(data); pmOS usa p7b. Requiere reescribir
el EBR/MBR (mtkclient puede; BROM recupera). **Es el diseño real a hacer.**

**Opción B — pmOS en una partición existente y Android recortado.** Más frágil
(Android espera tamaños concretos). Menos recomendable.

**Opción C — rootfs de pmOS en un fichero dentro de usrdata** (loop-mount desde
el initramfs). Evita reparticionar, pero el initramfs de pmOS hay que adaptarlo
para montar la raíz desde un archivo. Elegante; esfuerzo en el initramfs.

## 3. Pasos concretos (opción A)

1. **Backup golden** (ya lo tenemos como práctica) antes de reparticionar.
2. Diseñar un **scatter custom** con p7 dividido (o reescribir el EBR con mtkclient).
3. Flashear: Android KitKat (system p5 + boot.img stock → `recovery`), pmOS
   (rootfs → p7b, kernel pmOS → `boot`).
4. Ajustar el initramfs de pmOS para buscar su raíz en `p7b` (por label
   `pmOS_root` ya lo hace — solo cambia la partición destino).
5. Probar el cambio por `reboot recovery` (RTC) y por combo de teclas.

## 4. Para TRES sistemas (Android + pmOS + mainline)

Solo hay 2 slots de boot nativos. Para un tercero hace falta un **bootloader con
menú**:
- **Parchear el LK** (Capa 4a): añadir menú pmOS/mainline/Android. Riesgo acotado
  por BROM. Mejor relación esfuerzo/resultado.
- **lk2nd**: no portado a MT6582 (sería un subproyecto).
- **U-Boot mt6582**: `mediatek-mainline/u-boot` arranca por UART/SD pero sin
  display/USB/fastboot aún (se solaparía con M3).

**Recomendación**: empezar por dual-boot de 2 slots (Android KitKat en `recovery`
+ pmOS en `boot`), resolver el reparto (opción A o C), y dejar el 3er SO (mainline)
para cuando haya menú en el LK — o sencillamente alternar mainline↔pmOS en el slot
`boot` (que es como desarrollamos ahora).

## 5. Relación con mainline

Para el desarrollo de mainline NO necesitamos dual-boot: seguimos flasheando
mainline en `boot` y restaurando pmOS cuando toca (ciclo actual). El dual-boot es
para uso "producto" (tener Android usable + pmOS de juguete a la vez). Tiene
sentido montarlo cuando pmOS/mainline estén más maduros (display M3, etc.).
