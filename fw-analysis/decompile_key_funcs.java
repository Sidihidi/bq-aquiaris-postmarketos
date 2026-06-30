//@category Analysis
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.address.*;
import ghidra.app.cmd.function.CreateFunctionCmd;
import java.io.FileWriter;
import java.io.PrintWriter;

public class decompile_key_funcs extends GhidraScript {
    public void run() throws Exception {
        // Direcciones clave del handoff 30-06 (rango del dump f0020000-f004fffc)
        long[] addrs = {
            0xf002a480L, // privacy.c: comprueba enc_status > 2 (cifrado activo)
            0xf002a4ccL, // privacy.c: PORT CONTROL (decide si frame pasa o se descarta)
            0xf002a554L, // privacy.c: comprueba flag 0x35c (clave instalada)
            0xf004b6ccL, // hal.c: configura WTBL para cifrado (enc_status > 2)
            0xf004bb2cL, // nic_privacy.c: ARMA el TX cifrado (lee WTBL, instala clave HW)
            0xf004be5cL, // hal.c: acceso a WTBL
        };
        String[] names = {
            "privacy_enc_check", "privacy_port_control", "privacy_key_installed",
            "hal_config_wtbl_enc", "nic_privacy_arm_tx_enc", "hal_wtbl_access",
        };
        AddressSpace s = currentProgram.getAddressFactory().getDefaultAddressSpace();
        DecompInterface ifc = new DecompInterface();
        ifc.openProgram(currentProgram);
        PrintWriter out = new PrintWriter(new FileWriter("/tmp/fw_key_decompiled.c"));
        FunctionManager fm = currentProgram.getFunctionManager();
        for (int i = 0; i < addrs.length; i++) {
            long a = addrs[i];
            Address addr = s.getAddress(a);
            try { disassemble(addr); } catch (Exception e) {}
            Function f = fm.getFunctionAt(addr);
            if (f == null) {
                CreateFunctionCmd cmd = new CreateFunctionCmd(addr);
                cmd.applyTo(currentProgram, monitor);
                f = fm.getFunctionAt(addr);
            }
            out.println("// ============================================================");
            out.println("// " + names[i] + " @ 0x" + Long.toHexString(a));
            out.println("// ============================================================");
            if (f != null) {
                DecompileResults res = ifc.decompileFunction(f, 120, monitor);
                if (res != null && res.decompileCompleted() && res.getDecompiledFunction() != null) {
                    out.println(res.getDecompiledFunction().getC());
                } else {
                    out.println("// (decompile fallo)");
                }
            } else {
                out.println("// (no se pudo crear funcion en esta direccion)");
            }
            out.println();
            println("Procesado " + names[i] + " @ 0x" + Long.toHexString(a));
        }
        out.close();
        println("LISTO -> /tmp/fw_key_decompiled.c");
    }
}
