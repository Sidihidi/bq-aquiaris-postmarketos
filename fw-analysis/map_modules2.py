#!/usr/bin/env python3
# Mapeo completo FW NDS32 -> modulo .c (con diccionario extraido de la rodata)
import re
from collections import defaultdict

MODULE_STRINGS = {
    0xf00630d4: "wifi/mgmt/bcm.c",
    0xf0063108: "wifi/mgmt/bss.c",
    0xf0063138: "wifi/mgmt/cnm.c",
    0xf0063148: "wifi/mgmt/cnm_mem.c",
    0xf006315c: "wifi/mgmt/cnm_timer.c",
    0xf00631dc: "wifi/mgmt/hem.c",
    0xf0063240: "wifi/mgmt/hem_cmd.c",
    0xf0063464: "wifi/mgmt/hem_mbox.c",
    0xf006347c: "wifi/mgmt/hem_pwr.c",
    0xf00634e8: "wifi/mgmt/mqm.c",
    0xf0063574: "wifi/mgmt/p2p_fsm.c",
    0xf0063590: "wifi/mgmt/privacy.c",
    0xf00635bc: "wifi/mgmt/pwr_mgt_fsm.c",
    0xf0063654: "wifi/mgmt/pwr_apsd.c",
    0xf006366c: "wifi/mgmt/pm_timer.c",
    0xf0063684: "wifi/mgmt/rate.c",
    0xf00636b8: "wifi/mgmt/scan.c",
    0xf00636ec: "wifi/mgmt/scan_fsm.c",
    0xf006380c: "wifi/mgmt/txm.c",
    0xf00638a4: "wifi/mgmt/rxm.c",
    0xf006393c: "wifi/mgmt/rlm.c",
    0xf0063950: "wifi/mgmt/rlm_domain.c",
    0xf0063968: "wifi/mgmt/rlm_protection.c",
    0xf0063994: "wifi/mgmt/rlm_statistics.c",
    0xf00639c8: "wifi/mgmt/rlm_ar.c",
    0xf0063af0: "wifi/mgmt/tdls.c",
    0xf0063b04: "wifi/mgmt/stats.c",
    0xf0063b38: "wifi/mgmt/mt6582/roaming_fsm.c",
    0xf0063c1c: "wifi/mgmt/mt6582/hem_efuse.c",
    0xf0063c54: "wifi/mgmt/mt6582/rlm_phy.c",
    0xf00643ac: "wifi/mgmt/mt6582/rlm_sensor.c",
    0xf00643cc: "wifi/mgmt/mt6582/rlm_test.c",
    0xf006440c: "wifi/mgmt/mt6582/rlm_txpwr.c",
    0xf0064818: "wifi/Service/wifi_init.c",
    0xf0064834: "wifi/nic/nic.c",
    0xf0064844: "wifi/nic/nic_pm.c",
    0xf0064858: "wifi/nic/nic_dma.c",
    0xf006486c: "wifi/nic/nic_privacy.c",
    0xf0064898: "wifi/nic/HAL/mt6582/hal.c",
}

src = open("/tmp/fw_all.c").read()
parts = re.split(r"// ===== (\S+) @ (0x[0-9a-f]+) =====", src)

assert_re = re.compile(r"\(\*[^)]*00014800\)\s*\(\s*0\s*,\s*0\s*,\s*(0x[0-9a-f]+)\s*,\s*(0x[0-9a-f]+)")
plain_re = re.compile(r"\bASSERT\w*\s*\(\s*(0x[0-9a-f]+)\s*,\s*(0x[0-9a-f]+)", re.I)

functions = []
i = 1
while i < len(parts):
    name = parts[i]
    addr = int(parts[i+1], 16)
    body = parts[i+2] if i+2 < len(parts) else ""
    mods = defaultdict(list)
    for m in assert_re.finditer(body):
        mods[int(m.group(1), 16)].append(int(m.group(2), 16))
    for m in plain_re.finditer(body):
        mods[int(m.group(1), 16)].append(int(m.group(2), 16))
    functions.append((name, addr, mods, body))
    i += 3

def mod_name(a):
    return MODULE_STRINGS.get(a, "MOD_unknown_0x%x" % a)

with open("/tmp/fw_key_functions.txt", "w") as out:
    out.write("# Mapeo FW NDS32 (633 funcs) -> modulo .c + lineas ASSERT\n")
    by_mod = defaultdict(list)
    for name, addr, mods, body in functions:
        if mods:
            for ma, lines in mods.items():
                by_mod[mod_name(ma)].append((addr, name, lines))
        else:
            by_mod["(sin ASSERT)"].append((addr, name, []))
    for mod in sorted(by_mod.keys()):
        out.write("\n========== %s (%d funcs) ==========\n" % (mod, len(by_mod[mod])))
        for addr, name, lines in sorted(by_mod[mod]):
            ls = ",".join("0x%x" % l for l in sorted(set(lines))) if lines else ""
            out.write("  0x%08x  %-22s  %s\n" % (addr, name, ls))

# labeled C
with open("/tmp/fw_all_labeled.c", "w") as out:
    for name, addr, mods, body in functions:
        if mods:
            label = " | ".join("%s:%s" % (mod_name(ma), ",".join("0x%x" % l for l in sorted(set(lines))))
                                for ma, lines in mods.items())
        else:
            label = "(sin ASSERT)"
        out.write("// >>> MOD: %s <<<\n" % label)
        out.write("// ===== %s @ 0x%x =====\n" % (name, addr))
        out.write(body.rstrip() + "\n\n")

mapped = sum(1 for _,_,m,_ in functions if m)
print("Funciones totales: %d" % len(functions))
print("Funciones con modulo identificado: %d (%d%%)" % (mapped, 100*mapped//len(functions)))
mods_count = defaultdict(int)
for _,_,mods,_ in functions:
    for ma in mods:
        mods_count[mod_name(ma)] += 1
print("\nTop modulos por nº de funciones:")
for m, c in sorted(mods_count.items(), key=lambda x:-x[1])[:12]:
    print("  %3d  %s" % (c, m))
print("\nOutput: /tmp/fw_key_functions.txt + /tmp/fw_all_labeled.c")
