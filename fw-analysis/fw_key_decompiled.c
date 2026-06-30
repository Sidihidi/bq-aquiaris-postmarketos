// ============================================================
// privacy_enc_check @ 0xf002a480
// ============================================================

bool FUN_ram_f002a480(void)

{
  int iVar1;
  
  iVar1 = iRam020a0068;
  if (*(int *)(iRam020a0068 + 0x14) != 1) {
    if (7 < *(uint *)(iRam020a0068 + 0x10)) {
      (*pcRam00014800)(0,0,0xf0063590,0x191,pcRam00014800);
    }
    if (2 < *(uint *)(iVar1 + 0x10)) {
      return *(uint *)(iVar1 + 0x10) != 5;
    }
  }
  return false;
}



// ============================================================
// privacy_port_control @ 0xf002a4cc
// ============================================================

bool FUN_ram_f002a4cc(int param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = *(int *)(param_1 + 0x20);
  if (iVar2 == 0) {
    if (*(char *)(param_1 + 0xd) == '\0') {
      return (bool)*(undefined1 *)(iRam020a0064 + 0x12e3);
    }
    if (*(char *)(param_1 + 0xd) == '\x01') {
      return *(char *)(iRam020a0064 + 0x197c) != '\0';
    }
  }
  else {
    uVar1 = (uint)*(byte *)(iVar2 + 0x14);
    iVar3 = *(int *)(uVar1 * 4 + 0x20a0068);
    if (*(char *)(param_1 + 0x28) == '\0') {
      if (*(char *)(iVar2 + 0x289) != '\0') {
        return true;
      }
      if ((uVar1 == 0) && (*(uint *)(iVar3 + 0x10) < 3)) {
        return *(char *)(iRam020a0064 + 0x12e3) != '\0';
      }
    }
    else if (((uVar1 != 0) || (2 < *(uint *)(iVar3 + 0x10))) && (*(char *)(iVar2 + 0x289) != '\0'))
    {
      return *(char *)(iVar2 + 0x211) == '\0';
    }
  }
  return false;
}



// ============================================================
// privacy_key_installed @ 0xf002a554
// ============================================================

bool FUN_ram_f002a554(int param_1)

{
  bool bVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 * 4 + 0x20a0068);
  if (*(char *)(iVar2 + 0x35c) == '\0') {
    bVar1 = false;
    if (*(int *)(iVar2 + 0x14) != 1) {
      if (7 < *(uint *)(iVar2 + 0x10)) {
        (*pcRam00014800)(0,0,0xf0063590,0x504,pcRam00014800);
      }
      bVar1 = 2 < *(uint *)(iVar2 + 0x10);
    }
  }
  else {
    bVar1 = true;
  }
  return bVar1;
}



// ============================================================
// hal_config_wtbl_enc @ 0xf004b6cc
// ============================================================

bool FUN_ram_f004b6cc(byte param_1)

{
  bool bVar1;
  uint uVar2;
  uint uVar3;
  undefined1 auStack_58 [32];
  undefined1 auStack_38 [12];
  undefined1 auStack_2c [12];
  undefined4 uStack_20;
  undefined2 uStack_1c;
  
  if (param_1 < 0x14) {
    (*(code *)0x6a0a0)(auStack_58,0);
    (*(code *)0x6a0a0)(auStack_38,0,10);
    (*(code *)0x6a0a0)(auStack_2c,0,10);
    uStack_20 = 0;
    uStack_1c = 0;
    uVar2 = (*pcRam00014e4c)(param_1,auStack_2c,&uStack_20,auStack_38,pcRam00014e4c);
    uVar3 = (*pcRam00014e78)(param_1,auStack_58,pcRam00014e78);
    bVar1 = (uVar3 & uVar2) == 0;
  }
  else {
    bVar1 = false;
  }
  return bVar1;
}



// ============================================================
// nic_privacy_arm_tx_enc @ 0xf004bb2c
// ============================================================

undefined4 FUN_ram_f004bb2c(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  undefined1 auStack_60 [32];
  undefined1 uStack_40;
  undefined1 uStack_3f;
  undefined1 auStack_34 [6];
  undefined1 uStack_2e;
  undefined4 uStack_28;
  undefined2 uStack_24;
  
  uVar4 = (uint)*(byte *)(param_1 + 0x14);
  iVar1 = *(int *)(uVar4 * 4 + 0x20a0068);
  if ((uVar4 == 0) && (*(int *)(param_1 + 0x10) == 0x41)) {
    iVar2 = (*(code *)0x6a074)(param_1 + 10,iRam020a0064 + 0xfb3,6);
    if (iVar2 != 0) {
      return 0;
    }
    (*pcRam00014d94)(0x36,pcRam00014d94);
    uVar3 = 0;
    goto LAB_ram_f004bc46;
  }
  uVar3 = *(uint *)(param_1 + 0x10);
  if (uVar3 == 0x11) {
    uVar4 = 8;
LAB_ram_f004bb98:
    uVar3 = 1;
  }
  else {
    if (uVar4 == 2) {
      uVar4 = 4;
      goto LAB_ram_f004bb98;
    }
    if (uVar3 == 0x42) {
LAB_ram_f004bbc8:
      uVar3 = 2;
      goto LAB_ram_f004bc46;
    }
    if ((uVar3 & 0x20) == 0) {
      if ((uVar4 == 1) && (uVar3 == 0x41)) goto LAB_ram_f004bbc8;
      if (uVar3 != 0x81) {
        return 0;
      }
      uVar4 = 0xb;
      uVar3 = 0xb;
    }
    else {
      uVar4 = 9;
      uVar3 = 2;
    }
  }
  for (uVar6 = uVar3; uVar6 <= uVar4; uVar6 = uVar6 + 1 & 0xff) {
    (*pcRam00014e54)(uVar6,auStack_34,&uStack_28,&uStack_40,pcRam00014e54);
    iVar2 = (*(code *)0x6a074)(param_1 + 10,auStack_34,6);
    uVar5 = uVar6;
    if (iVar2 == 0) goto LAB_ram_f004bc10;
  }
  uVar5 = 0x7f;
LAB_ram_f004bc10:
  if (uVar6 == uVar4 + 1) {
    while( true ) {
      if (uVar4 < uVar3) break;
      if (*(char *)(iRam020a0064 + (uVar3 + 0x417) * 8 + 4) == '\0') goto LAB_ram_f004bc46;
      uVar3 = uVar3 + 1 & 0xff;
    }
  }
  uVar3 = uVar5;
  if (uVar5 == 0x7f) {
    return 0;
  }
LAB_ram_f004bc46:
  if ((*(char *)(param_1 + 0x14) == '\0') && (iVar2 = FUN_ram_f002a480(), iVar2 != 0)) {
    FUN_ram_f004b6cc(uVar3);
  }
  (*(code *)0x6a0a0)(auStack_60,0,0x20);
  if (*(int *)(param_1 + 0x10) == 0x11) {
    if (*(int *)(iVar1 + 0x14) == 1) {
      *(undefined1 *)(param_1 + 0x289) = 0;
      (*pcRam00014e54)(0,auStack_34,&uStack_28,&uStack_40,pcRam00014e54);
      (*(code *)0x6a0a0)(&uStack_40,0,10);
    }
    else {
      *(undefined1 *)(param_1 + 0x289) = 1;
      (*pcRam00014e58)(0,auStack_60,&uStack_28,auStack_34,pcRam00014e58);
      (*pcRam00014e54)(0,auStack_34,&uStack_28,&uStack_40,pcRam00014e54);
    }
  }
  else {
    (*pcRam00014e58)(uVar3,auStack_60,pcRam00014e58);
    (*pcRam00014e54)(uVar3,auStack_34,&uStack_28,&uStack_40,pcRam00014e54);
    if (((*(int *)(param_1 + 0x10) == 0x41) && (*(int *)(iRam020a0064 + 0xf7c) == 0)) &&
       (*(char *)(iRam020a0064 + 0x12e3) != '\0')) {
      uStack_40 = 1;
    }
  }
  (*(code *)0x6a08c)(auStack_34,param_1 + 10,6);
  iVar2 = iRam020a0064;
  uStack_2e = 0;
  uStack_24 = 0;
  uStack_28 = 0;
  iVar1 = *(int *)(iVar1 + 0x10);
  if (((iVar1 == 0) || (iVar1 == 2)) &&
     ((*(char *)(param_1 + 0x14) == '\0' && (iVar1 = FUN_ram_f002a480(), iVar1 == 0)))) {
    uStack_3f = 1;
  }
  iVar1 = (*pcRam00014e4c)(uVar3,auStack_34,&uStack_28,&uStack_40,pcRam00014e4c);
  if (iVar1 != 0) {
    (*pcRam00014800)(0,0,0xf006486c,0x288,pcRam00014800);
    return 0;
  }
  iVar2 = iVar2 + (uVar3 + 0x417) * 8;
  *(undefined1 *)(iVar2 + 4) = 1;
  *(int *)(iVar2 + 8) = param_1;
  *(undefined1 *)(iVar2 + 5) = *(undefined1 *)(param_1 + 0x14);
  *(char *)(param_1 + 0x28a) = (char)uVar3;
  if ((*(char *)(param_1 + 0x14) == '\0') && (iVar1 = FUN_ram_f002a480(), iVar1 == 0)) {
    (*pcRam00014e78)(uVar3,auStack_60,pcRam00014e78);
  }
  return 1;
}



// ============================================================
// hal_wtbl_access @ 0xf004be5c
// ============================================================

void FUN_ram_f004be5c(void)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = 0;
  do {
    uRam6032ff14 = iVar1 << 0x18 | 0x30000;
    uRam6032ff10 = 0;
    uVar2 = 0;
    while ((uRam6032ff14 & 0x10000) != 0) {
      uVar2 = uVar2 + 1;
      (*pcRam0001480c)(4,pcRam0001480c);
      if (10000 < uVar2) {
        (*pcRam00014800)(0,0,0xf0064898,0x143,pcRam00014800);
      }
    }
    iVar1 = iVar1 + 1;
  } while (iVar1 != 0x20);
  return;
}



