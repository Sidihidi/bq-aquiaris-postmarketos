import struct
d=open("/home/cpcd/modem-h0/modem_1_wg_n.img","rb").read()
TARGETS={0x3db0:"0xfffff057 (codigo de NUESTRA excepcion)",
         0x3da8:"puntero a fs_ccci.c",
         0x3bf0:"puntero a fs_ccci.c (pool 2)",
         0x3e00:"puntero a fs_ccci.c (pool 3)"}
print("=== instrucciones que cargan esos literales (LDR PC-relativo) ===")
for pc in range(0x3000,0x3e10,2):
    hw=struct.unpack_from("<H",d,pc)[0]
    tgt=None; desc=""
    if 0x4800<=hw<0x5000:                       # LDR Rt,[PC,#imm8*4]  (16 bits)
        rt=(hw>>8)&7; imm=(hw&0xff)*4
        tgt=((pc+4)&~3)+imm; desc="LDR r%d,[pc,#%d]"%(rt,imm)
    elif hw==0xf8df:                            # LDR.W Rt,[PC,#imm12] (32 bits)
        w2=struct.unpack_from("<H",d,pc+2)[0]
        rt=(w2>>12)&0xf; imm=w2&0xfff
        tgt=((pc+4)&~3)+imm; desc="LDR.W r%d,[pc,#%d]"%(rt,imm)
    if tgt in TARGETS:
        print("  0x%06x  %-22s -> 0x%06x  %s"%(pc,desc,tgt,TARGETS[tgt]))
