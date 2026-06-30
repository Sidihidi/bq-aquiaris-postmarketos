// ===== FUNCION ANTERIOR MAS CERCANA: FUN_ram_f00260bc @ 0xf00260bc =====

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00260bc(int param_1,int param_2,undefined1 param_3)

{
  undefined2 *puVar1;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x5cc,_DAT_ram_00014800);
  }
  if (param_2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x5cd,_DAT_ram_00014800);
  }
  puVar1 = *(undefined2 **)(param_1 + 8);
  (*(code *)&SUB_ram_0006a08c)(puVar1 + 4,param_2 + 10,6);
  *(undefined1 *)(puVar1 + 7) = *(undefined1 *)(param_2 + 8);
  *(undefined1 *)((int)puVar1 + 0xf) = *(undefined1 *)(param_2 + 0x14);
  *(undefined1 *)(puVar1 + 8) = *(undefined1 *)(param_2 + 0x2a8);
  *(byte *)((int)puVar1 + 0x11) = (byte)(*(uint *)(param_2 + 0x10) >> 6) & 1;
  *(undefined1 *)(puVar1 + 2) = 0x13;
  *puVar1 = 0x14;
  *(undefined1 *)((int)puVar1 + 5) = param_3;
  *(undefined1 *)(param_1 + 0x2f) = 1;
  *(undefined2 **)(param_1 + 0x28) = puVar1;
  puVar1[1] = 1;
  FUN_ram_f0039750(param_1);
  return;
}



// ===== CALLER (contiene 0xf0026470): FUN_ram_f0026470 @ 0xf0026470 =====

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0026470(void)

{
  bool bVar1;
  int iVar2;
  int unaff_s0;
  int unaff_s1;
  code *in_ta;
  
  iVar2 = (*in_ta)();
  if (iVar2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x66c,_DAT_ram_00014800);
  }
  FUN_ram_f00347e8();
  FUN_ram_f002c468();
  FUN_ram_f0033f8c();
  FUN_ram_f0041210();
  if (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(unaff_s0 + 0x14) * 4) + 0xa8) == 3) {
    *(undefined1 *)(unaff_s0 + 0x56) = 1;
    *(undefined4 *)(unaff_s0 + 0x80) = 0;
    *(undefined4 *)(unaff_s0 + 0x78) = 0;
  }
  FUN_ram_f003ae98();
  FUN_ram_f003e0d8();
  FUN_ram_f0021e04();
  bVar1 = *(char *)(unaff_s1 + 0x22) != '\0';
  if (bVar1) {
    FUN_ram_f00260bc();
  }
  return bVar1;
}


