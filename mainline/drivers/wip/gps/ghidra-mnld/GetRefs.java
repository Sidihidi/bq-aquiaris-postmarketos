// Dump every string/data reference from the function at 0x140fc (mtk_gps_sys_init),
// in instruction order, so we can read the property KEY and the strcmp CONSTANT.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.mem.MemoryAccessException;
import java.io.FileWriter;

public class GetRefs extends GhidraScript {

    String readCString(Address a) {
        try {
            StringBuilder s = new StringBuilder();
            for (int i = 0; i < 120; i++) {
                byte b = getByte(a.add(i));
                if (b == 0) break;
                if (b >= 0x20 && b < 0x7f) s.append((char) b);
                else s.append("\\x" + String.format("%02x", b & 0xff));
            }
            return s.toString();
        } catch (Exception e) { return "<err>"; }
    }

    void dumpFunc(long entry, StringBuilder sb) throws Exception {
        Address fe = toAddr(entry);
        Function f = getFunctionContaining(fe);
        if (f == null) { sb.append("no func at " + fe + "\n"); return; }
        sb.append("\n### FUNC " + f.getName() + " @ " + f.getEntryPoint() + " ###\n");
        Listing lst = currentProgram.getListing();
        InstructionIterator it = lst.getInstructions(f.getBody(), true);
        while (it.hasNext()) {
            Instruction ins = it.next();
            for (Reference r : ins.getReferencesFrom()) {
                Address to = r.getToAddress();
                if (to == null) continue;
                // read as string
                String sval = readCString(to);
                String mn = ins.getMnemonicString();
                // only show if it looks like text or a call
                if (sval.length() > 1 && sval.matches("[\\x20-\\x7e\\\\]+.*")) {
                    sb.append(String.format("  %s  %-6s -> %s : \"%s\"\n",
                        ins.getAddress(), mn, to, sval));
                }
            }
        }
    }

    public void run() throws Exception {
        StringBuilder sb = new StringBuilder();
        long[] funcs = {0x140fcL, 0x14ef4L};
        for (long fn : funcs) dumpFunc(fn, sb);
        String out = "C:\\Users\\jferr\\AppData\\Local\\Temp\\claude\\C--Users-jferr-Desktop-pmos-krillin\\dec5d725-386e-4c12-8dd6-6902ab1f70f6\\scratchpad\\ghidra-gps\\refs.txt";
        FileWriter fw = new FileWriter(out);
        fw.write(sb.toString());
        fw.close();
        println("WROTE refs to " + out);
    }
}
