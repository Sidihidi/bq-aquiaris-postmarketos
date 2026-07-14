// Decompile the DSP init chain + find the reader thread (pthread_create) + who opens/reads /dev/stpgps.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.symbol.Reference;
import ghidra.util.task.ConsoleTaskMonitor;
import java.io.FileWriter;
import java.util.*;

public class FindReader extends GhidraScript {
    DecompInterface decomp;
    ConsoleTaskMonitor mon = new ConsoleTaskMonitor();
    StringBuilder sb = new StringBuilder();
    Set<String> seen = new HashSet<String>();
    FunctionManager fm;

    String dc(Function f) {
        try {
            DecompileResults r = decomp.decompileFunction(f, 90, mon);
            if (r != null && r.decompileCompleted()) return r.getDecompiledFunction().getC();
        } catch (Exception e) { return "// exc: " + e; }
        return "// decomp failed";
    }
    void dump(Function f, String tag) {
        if (f == null) return;
        String k = f.getEntryPoint().toString();
        if (seen.contains(k)) return;
        seen.add(k);
        sb.append("\n===== " + tag + " " + f.getName() + " @ " + f.getEntryPoint() + " =====\n");
        sb.append(dc(f));
    }
    void dumpAt(long addr, String tag) { dump(fm.getFunctionContaining(toAddr(addr)), tag); }

    // decompile every function that references the given symbol name (imported thunk)
    void dumpCallersOf(String sym, String tag) {
        java.util.Iterator<ghidra.program.model.symbol.Symbol> it =
            currentProgram.getSymbolTable().getSymbols(sym);
        while (it.hasNext()) {
            ghidra.program.model.symbol.Symbol s = it.next();
            for (Reference r : getReferencesTo(s.getAddress())) {
                Function f = fm.getFunctionContaining(r.getFromAddress());
                if (f != null) dump(f, tag + "(->" + sym + ")");
            }
        }
    }
    // decompile functions referencing a string literal
    void dumpRefsToString(String str, String tag) {
        Address a = findBytes(currentProgram.getMinAddress(), str);
        if (a == null) { sb.append("// string not found: " + str + "\n"); return; }
        for (Reference r : getReferencesTo(a)) {
            Function f = fm.getFunctionContaining(r.getFromAddress());
            if (f != null) dump(f, tag);
            else for (Reference r2 : getReferencesTo(r.getFromAddress())) {
                Function f2 = fm.getFunctionContaining(r2.getFromAddress());
                if (f2 != null) dump(f2, tag);
            }
        }
    }

    public void run() throws Exception {
        fm = currentProgram.getFunctionManager();
        decomp = new DecompInterface();
        decomp.openProgram(currentProgram);

        // init chain tras mtk_gps_sys_init (de FUN_000184b0)
        dumpAt(0x184b0L, "ORCH");
        dumpAt(0x17048L, "INIT1");
        dumpAt(0x18458L, "INIT2");
        dumpAt(0x17e64L, "POST");
        dumpAt(0x14ef4L, "OPEN_DSP(FUN_00014ef4)");

        // el/los hilos lectores + quien los crea
        dumpCallersOf("pthread_create", "PTHREAD");
        // quien abre/lee /dev/stpgps y /dev/gps
        dumpRefsToString("/dev/stpgps", "STPGPS_REF");

        String out = "C:\\Users\\jferr\\AppData\\Local\\Temp\\claude\\C--Users-jferr-Desktop-pmos-krillin\\dec5d725-386e-4c12-8dd6-6902ab1f70f6\\scratchpad\\ghidra-gps\\reader.txt";
        FileWriter fw = new FileWriter(out); fw.write(sb.toString()); fw.close();
        println("WROTE " + sb.length() + " chars to " + out);
    }
}
