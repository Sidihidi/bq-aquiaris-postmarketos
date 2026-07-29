import re
F="/home/cpcd/modem-fsd/gd-boot-full.out"
OPS={0x1001:"OPEN",0x1002:"?02",0x1003:"?03",0x1004:"WRITE",0x1005:"CLOSE?",
     0x1009:"GetSize?",0x100e:"GetDrive",0x1010:"GetFullPath",0x1011:"GetFullPath2",
     0x1012:"OP_1012",0x1013:"?13"}
n=0
for ln in open(F):
    m=re.match(r"([\d.]+) idx=(\d+) op=([0-9a-f]{8}) \| (.*)",ln.strip())
    if not m: continue
    op=int(m.group(3),16); lo=op&0xffff; hi=op>>16
    if lo not in (0x1012,0x1011): continue
    w=[int(x,16) for x in m.group(4).split()]
    raw=b"".join(x.to_bytes(4,"little") for x in w[2:])
    s=raw[:max(0,w[1])].decode("utf-16-le","replace").split("\x00")[0]
    s="".join(c if 32<=ord(c)<127 else "." for c in s)
    print("%s %-4s op=%04x %-12s c=%-2d len=0x%02x  %s   [w2=%08x w3=%08x]"%(
        m.group(1),"RESP" if hi==0xffff else "REQ",lo,OPS.get(lo,"?"),w[0],w[1],s,w[2],w[3]))
    n+=1
    if n>=14: break
print("total lineas con 1011/1012 mostradas: %d"%n)
