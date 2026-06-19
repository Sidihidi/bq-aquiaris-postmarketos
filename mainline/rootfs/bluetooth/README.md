# Bluetooth en Phosh (mainline) — setup de userspace

El kernel registra `hci0` (driver `mt6582-btif.c`). Userspace:

1. **bluez**: `apk add bluez` (5.86). `rc-update add bluetooth default`.
2. **Bring-up al arranque**: `zz-consys-bt.start` → `/etc/local.d/` (+ `rc-update add local default`).
   Dispara el bring-up del CONSYS (patch + func_on BT + registra hci0) y reinicia bluetoothd.
   Manual: `echo 1 > /sys/kernel/debug/mt6582_btif/bringup`.
3. **GUI**:
   - **Toggle nativo de Phosh** (recomendado, móvil): barra superior → quick settings → Bluetooth.
     Aparece cuando hay hci0 + bluetoothd (gnome-bluetooth lo detecta por D-Bus).
   - **gnome-control-center** (`org.gnome.Settings`, instalado) → panel Bluetooth (para emparejar).
     Es la versión "escritorio" pero funciona. `phosh-mobile-settings` NO está en los repos de esta Alpine.

## Estado verificado (2026-06-19)
`Powered: yes`, `Discoverable: yes`, `Pairable: yes`. Escaneó y encontró "S24 Ultra de Juan".

## Pendiente fino
- bdaddr real de NVRAM (ahora derivada 00:00:46:65:82:01) vía comando HCI vendor MTK.
- Confirmar que el toggle de la barra de Phosh sale solo tras el autostart.
