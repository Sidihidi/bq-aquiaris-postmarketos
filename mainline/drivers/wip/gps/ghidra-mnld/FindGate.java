// Decompile functions referencing GPS gate strings + their callers.
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

public class FindGate extends GhidraScript {
    DecompInterface decomp;
    ConsoleTaskMonitor mon = new ConsoleTaskMonitor();
    StringBuilder sb = new StringBuilder();
    Set<String> seen = new HashSet<String>();

    String dc(Function f) {
        try {
            DecompileResults r = decomp.decompileFunction(f, 90, mon);
            if (r != null && r.decompileCompleted())
                return r.getDecompiledFunction().getC();
        } catch (Exception e) { return "// decomp exception: " + e; }
        return "// decomp failed";
    }

    void dumpFunc(Function f, String tag) {
        String k = f.getEntryPoint().toString();
        if (seen.contains(k)) return;
        seen.add(k);
        sb.append("\n--- " + tag + " " + f.getName() + " @ " + f.getEntryPoint() + " ---\n");
        sb.append(dc(f));
    }

    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        String[] targets = {"mtk_gps_sys_init", "mtk_gps_exit_proc", "launch_daemon_thread"};

        for (String t : targets) {
            sb.append("\n======================================================\n");
            sb.append("STRING: " + t + "\n");
            sb.append("======================================================\n");
            Address a = findBytes(currentProgram.getMinAddress(), t);
            if (a == null) { sb.append("  not found\n"); continue; }
            sb.append("  @ " + a + "\n");
            Set<Function> callerFuncs = new HashSet<Function>();
            // direct refs to the string bytes
            for (Reference r : getReferencesTo(a)) {
                Address fa = r.getFromAddress();
                Function f = fm.getFunctionContaining(fa);
                if (f != null) callerFuncs.add(f);
                else {
                    // literal-pool word: find who refs the pointer word
                    for (Reference r2 : getReferencesTo(fa)) {
                        Function f2 = fm.getFunctionContaining(r2.getFromAddress());
                        if (f2 != null) callerFuncs.add(f2);
                    }
                }
            }
            StringBuilder names = new StringBuilder();
            for (Function f : callerFuncs) names.append(f.getName() + " ");
            sb.append("  referenced by: " + names + "\n");
            for (Function f : callerFuncs) {
                dumpFunc(f, "FUNC");
                for (Function c : f.getCallingFunctions(mon))
                    dumpFunc(c, "CALLER(of " + f.getName() + ")");
            }
        }

        String out = "C:\\Users\\jferr\\AppData\\Local\\Temp\\claude\\C--Users-jferr-Desktop-pmos-krillin\\dec5d725-386e-4c12-8dd6-6902ab1f70f6\\scratchpad\\ghidra-gps\\decomp.txt";
        FileWriter fw = new FileWriter(out);
        fw.write(sb.toString());
        fw.close();
        println("WROTE " + sb.length() + " chars to " + out);
    }
}
