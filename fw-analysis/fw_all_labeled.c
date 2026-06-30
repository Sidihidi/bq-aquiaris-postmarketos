// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020000 @ 0xf0020000 =====


bool FUN_ram_f0020000(void)

{
  return DAT_ram_f0064fed != '\0';
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020010 @ 0xf0020010 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0020010(uint param_1)

{
  _DAT_ram_60120084 = (param_1 & 0x3f) << 0x18 | _DAT_ram_60120084 & 0xc0ffffff;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002003c @ 0xf002003c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002003c(uint param_1)

{
  _DAT_ram_60120088 = (param_1 & 0x3f) << 0x18 | _DAT_ram_60120088 & 0xc0ffffff;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020068 @ 0xf0020068 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0020068(uint param_1)

{
  _DAT_ram_60120088 = (param_1 & 0x3f) << 0x10 | _DAT_ram_60120088 & 0xffc0ffff;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020094 @ 0xf0020094 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0020094(uint param_1)

{
  _DAT_ram_60120088 = (param_1 & 0x3f) << 8 | _DAT_ram_60120088 & 0xffffc0ff;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00200b8 @ 0xf00200b8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f00200b8(char param_1)

{
  if (param_1 == '\0') {
    _DAT_ram_60120088 = _DAT_ram_60120088 & 0xffffff7f;
  }
  else {
    _DAT_ram_60120088 = _DAT_ram_60120088 | 0x80;
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00200e0 @ 0xf00200e0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f00200e0(uint param_1)

{
  _DAT_ram_60120088 = param_1 & 0xf | _DAT_ram_60120088 & 0xfffffff0;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00200fc @ 0xf00200fc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f00200fc(char param_1)

{
  if (param_1 == '\0') {
    _DAT_ram_601200a8 = _DAT_ram_601200a8 & 0xffff7fff;
  }
  else {
    _DAT_ram_601200a8 = _DAT_ram_601200a8 | 0x8000;
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020124 @ 0xf0020124 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0020124(void)

{
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  
  if (DAT_ram_f0064fd5 == '\0') {
    local_10 = 2;
  }
  else {
    local_10 = 1;
  }
  local_18 = 2;
  local_8c = 2;
  local_88 = 2;
  local_84 = 2;
  local_80 = 2;
  local_7c = 2;
  local_78 = 2;
  local_74 = 2;
  local_68 = 2;
  local_64 = 2;
  local_60 = 2;
  local_5c = 2;
  local_58 = 2;
  local_70 = 2;
  local_6c = 2;
  local_54 = 2;
  local_50 = 2;
  local_4c = 2;
  local_48 = 2;
  local_44 = 2;
  local_40 = 2;
  local_3c = 2;
  local_30 = 2;
  local_2c = 2;
  local_28 = 2;
  local_24 = 2;
  local_20 = 2;
  local_c = 2;
  local_14 = 1;
  local_1c = 1;
  (*_DAT_ram_00014ce0)(&local_8c,_DAT_ram_00014ce0);
  (*_DAT_ram_00014ce4)(0x12,_DAT_ram_00014ce4);
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00201d0 @ 0xf00201d0 =====


void FUN_ram_f00201d0(char param_1)

{
  DAT_ram_f0064fe7 = param_1 == '\0';
  DAT_ram_f0064ffc = param_1 == '\0';
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00201ec @ 0xf00201ec =====


void FUN_ram_f00201ec(void)

{
  DAT_ram_f0064ffc = 1;
  DAT_ram_f0064fe7 = DAT_ram_f0064fed == '\0';
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020218 @ 0xf0020218 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0020218(char param_1)

{
  undefined1 uVar1;
  
  if (DAT_ram_f0064fb0 != '\0') {
    param_1 = '\0';
  }
  if (DAT_ram_f0064ffa != param_1) {
    DAT_ram_f0064ffa = param_1;
    if (param_1 == '\0') {
      (*_DAT_ram_00014814)(2,0xf0063024,_DAT_ram_00014814);
      uVar1 = 0;
    }
    else {
      (*_DAT_ram_00014814)(2,0xf0063000,_DAT_ram_00014814);
      if (2 < *(uint *)(_DAT_ram_020a0064 + 0x380)) {
        return;
      }
      uVar1 = 1;
    }
    (*pcRam00000000)(uVar1,0,pcRam00000000);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020298 @ 0xf0020298 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0020298(void)

{
  byte *pbVar1;
  undefined4 uVar2;
  
  if (DAT_ram_f0064ff7 == '\0') {
    return;
  }
  if (((ushort)(_DAT_ram_f0065064 - 0x8fcU) < 300) && (DAT_ram_f0064fb1 != 8)) {
    if ((ushort)(_DAT_ram_f0065064 - 0x8fcU) < 100) {
      if (DAT_ram_f0064fb4 == '\x01') {
        DAT_ram_f0064fb2 = DAT_ram_f0064fff;
        pbVar1 = &DAT_ram_f0065000;
      }
      else {
        DAT_ram_f0064fb2 = DAT_ram_f0065001;
        pbVar1 = &DAT_ram_f0065002;
      }
    }
    else {
      if (99 < (ushort)(_DAT_ram_f0065064 - 0x9c4U)) goto LAB_ram_f0020368;
      DAT_ram_f0064fb2 = DAT_ram_f0064fff;
      pbVar1 = &DAT_ram_f0065000;
    }
    DAT_ram_f0064fb3 = *pbVar1;
  }
  else {
    if (DAT_ram_f0064ff0 == '\x05') {
      DAT_ram_f0064fb2 = 10;
    }
    else {
      DAT_ram_f0064fb2 = 0xe;
    }
    DAT_ram_f0064fb3 = 1;
  }
LAB_ram_f0020368:
  DAT_ram_f0064ff3 = DAT_ram_f0064fb3 <= DAT_ram_f0064ffe && DAT_ram_f0064ffe <= DAT_ram_f0064fb2;
  if (_DAT_ram_f0064f2c == 5000000) {
    DAT_ram_f0064fe7 = 0;
    DAT_ram_f0064ffc = 0;
  }
  else {
    if (DAT_ram_f0064fb1 == 2) {
      FUN_ram_f00201d0();
      uVar2 = 0xf0063068;
    }
    else if (DAT_ram_f0064fb1 < 3) {
      if (DAT_ram_f0064fb1 != 1) goto LAB_ram_f0020468;
      DAT_ram_f0064fe7 = 0;
      DAT_ram_f0064ffc = 0;
      uVar2 = 0xf0063050;
    }
    else if (DAT_ram_f0064fb1 == 4) {
      FUN_ram_f00201ec();
      uVar2 = 0xf0063080;
    }
    else {
      if (DAT_ram_f0064fb1 != 8) goto LAB_ram_f0020468;
      DAT_ram_f0064fe7 = 0;
      DAT_ram_f0064ffc = 0;
      uVar2 = 0xf0063098;
    }
    (*_DAT_ram_00014814)(2,uVar2,_DAT_ram_00014814);
  }
LAB_ram_f0020468:
  FUN_ram_f0020218(DAT_ram_f0064fe7);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020480 @ 0xf0020480 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0020480(uint param_1,undefined1 param_2)

{
  if (_DAT_ram_f0064f24 != param_1) {
    _DAT_ram_f0064f24 = param_1;
    (*_DAT_ram_00014ce8)(0x60,_DAT_ram_00014ce8);
    (*_DAT_ram_00014d14)(5,_DAT_ram_00014d14);
    (*_DAT_ram_00014d10)(1,_DAT_ram_00014d10);
    (*_DAT_ram_00014d04)(param_1 & 0xff,param_2,_DAT_ram_00014d04);
    (*_DAT_ram_00014cec)(1,1,1,_DAT_ram_00014cec);
    (*_DAT_ram_00014cf8)(0,4,0x20,4,_DAT_ram_00014cf8);
    (*_DAT_ram_00014d00)(0,0,_DAT_ram_00014d00);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020508 @ 0xf0020508 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0020508(undefined1 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((DAT_ram_f0064fec == '\0') && (_DAT_ram_f006513c == 0x24ba58)) {
    uVar1 = 0x280;
  }
  else {
    uVar1 = 0x500;
  }
  (*_DAT_ram_00014d0c)(uVar1,_DAT_ram_00014d0c);
  (*_DAT_ram_00014cd8)(0,0,_DAT_ram_00014cd8);
  iVar2 = (*_DAT_ram_00014878)(_DAT_ram_00014878);
  if (*(byte *)(iVar2 + 8) < 7) {
    uVar1 = 0x28;
  }
  else {
    uVar1 = 0x44;
  }
  (*_DAT_ram_00014cf0)(uVar1,_DAT_ram_00014cf0);
  if (((DAT_ram_f0064fe0 != '\0') || (DAT_ram_f0064fe1 != '\0')) && (DAT_ram_f0064fa9 == '\0')) {
    (*_DAT_ram_00014cf0)(0x272,_DAT_ram_00014cf0);
  }
  (*_DAT_ram_00014cf4)(param_1,_DAT_ram_00014cf4);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00205c4 @ 0xf00205c4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00205c4(undefined1 param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  undefined1 uVar3;
  
  if (DAT_ram_f0064fec == '\0') {
    if (DAT_ram_f0064fd5 == '\0') {
      uVar2 = 0x500;
    }
    else {
      uVar2 = 0xa00;
    }
  }
  else {
    uVar2 = 0x1380;
  }
  (*_DAT_ram_00014d0c)(uVar2,_DAT_ram_00014d0c);
  if (DAT_ram_f0064fe0 == '\0') {
    if (DAT_ram_f0064fe1 == '\0') {
      uVar3 = 1;
      uVar1 = uVar3;
    }
    else {
      uVar3 = 0;
      uVar1 = 1;
    }
  }
  else if (DAT_ram_f0064fe1 == '\0') {
    uVar3 = 1;
    uVar1 = 0;
  }
  else {
    uVar3 = 0;
    uVar1 = uVar3;
  }
  (*_DAT_ram_00014cd8)(uVar1,uVar3,_DAT_ram_00014cd8);
  (*_DAT_ram_00014cf0)(3,_DAT_ram_00014cf0);
  (*_DAT_ram_00014cf4)(param_1,_DAT_ram_00014cf4);
  if (*(char *)(_DAT_ram_020a0064 + 0x1002) == '\0') {
    (*_DAT_ram_00014cfc)(1,1,_DAT_ram_00014cfc);
    if ((*(char *)(_DAT_ram_020a0064 + 0x1392) != '\0') &&
       (*(char *)(_DAT_ram_020a0064 + 0x1310) != '\0')) {
      (*_DAT_ram_00014cfc)(1,0,_DAT_ram_00014cfc);
    }
    if (*(char *)(_DAT_ram_020a0064 + 0x1b42) == '\0') {
      return;
    }
    if (*(char *)(_DAT_ram_020a0064 + 0x1ac0) == '\0') {
      return;
    }
    uVar2 = 1;
  }
  else {
    (*_DAT_ram_00014cfc)(0,1,_DAT_ram_00014cfc);
    if ((*(char *)(_DAT_ram_020a0064 + 0x1392) != '\0') &&
       (*(char *)(_DAT_ram_020a0064 + 0x1310) != '\0')) {
      (*_DAT_ram_00014cfc)(0,0,_DAT_ram_00014cfc);
    }
    if (*(char *)(_DAT_ram_020a0064 + 0x1b42) == '\0') {
      return;
    }
    if (*(char *)(_DAT_ram_020a0064 + 0x1ac0) == '\0') {
      return;
    }
    uVar2 = 0;
  }
  (*_DAT_ram_00014cfc)(uVar2,0,_DAT_ram_00014cfc);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020704 @ 0xf0020704 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0020704(void)

{
  return _DAT_ram_f0065080;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020710 @ 0xf0020710 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0020710(void)

{
  undefined4 uVar1;
  
  if (DAT_ram_f0064fed == '\0') {
    return;
  }
  if (DAT_ram_f0064fe0 != '\0') {
    if (DAT_ram_f0064fe3 != '\0') {
      if (DAT_ram_f0064fe1 == '\0') goto LAB_ram_f00207b6;
      if (DAT_ram_f0064fe4 == '\0') goto LAB_ram_f002099e;
      uVar1 = 0xf00630b0;
LAB_ram_f0020920:
      (*_DAT_ram_00014814)(2,uVar1,_DAT_ram_00014814);
      goto LAB_ram_f0020a62;
    }
    if (DAT_ram_f0064fe1 != '\0') {
      if (DAT_ram_f0064fe4 == '\0') {
        uVar1 = 1;
      }
      else {
        uVar1 = 2;
      }
      FUN_ram_f0020508(uVar1,FUN_ram_f0020508);
      return;
    }
    if ((DAT_ram_f0064fe4 != '\0') || (DAT_ram_f0064faf != '\0')) {
      if ((DAT_ram_f0064fe4 == '\0') && (DAT_ram_f0064faf == '\0')) goto LAB_ram_f00207b6;
LAB_ram_f002097a:
      uVar1 = 2;
      goto LAB_ram_f00209a0;
    }
LAB_ram_f0020956:
    uVar1 = 1;
    goto LAB_ram_f00209a0;
  }
LAB_ram_f00207b6:
  if (DAT_ram_f0064fa9 == '\0') {
    if (DAT_ram_f0064fe0 == '\0') goto LAB_ram_f0020858;
    if ((((DAT_ram_f0064fe3 == '\0') || (DAT_ram_f0064fe1 != '\0')) || (DAT_ram_f0064fe4 != '\0'))
       || (DAT_ram_f0064faf != '\0')) goto LAB_ram_f002081c;
LAB_ram_f0020a54:
    uVar1 = 6;
LAB_ram_f0020a56:
    FUN_ram_f00205c4(uVar1);
    return;
  }
  if (DAT_ram_f0064fe0 == '\0') {
LAB_ram_f0020858:
    if (((DAT_ram_f0064fe1 != '\0') && (DAT_ram_f0064fe4 == '\0')) && (DAT_ram_f0064fe0 == '\0')) {
      if ((DAT_ram_f0064fe3 == '\0') && (DAT_ram_f0064fae == '\0')) {
        uVar1 = 4;
        goto LAB_ram_f00209a0;
      }
      if ((DAT_ram_f0064fe3 != '\0') || (DAT_ram_f0064fae != '\0')) goto LAB_ram_f002099e;
    }
    if (DAT_ram_f0064fa9 == '\0') {
      if (DAT_ram_f0064fe1 != '\0') {
        if ((((DAT_ram_f0064fe4 == '\0') || (DAT_ram_f0064fe0 != '\0')) ||
            (DAT_ram_f0064fe3 != '\0')) || (DAT_ram_f0064fae != '\0')) goto LAB_ram_f00208ee;
LAB_ram_f0020a24:
        uVar1 = 5;
        goto LAB_ram_f0020a56;
      }
LAB_ram_f00209ac:
      if (DAT_ram_f0064fe0 == '\0') goto LAB_ram_f00209ae;
      goto LAB_ram_f0020a28;
    }
    if (DAT_ram_f0064fe1 != '\0') {
      if (((DAT_ram_f0064fe4 != '\0') && (DAT_ram_f0064fe0 == '\0')) &&
         ((DAT_ram_f0064fe3 == '\0' && (DAT_ram_f0064fae == '\0')))) goto LAB_ram_f002097a;
LAB_ram_f00208ee:
      if (((DAT_ram_f0064fe4 != '\0') && (DAT_ram_f0064fe0 == '\0')) &&
         ((DAT_ram_f0064fe3 != '\0' || (DAT_ram_f0064fae != '\0')))) {
        uVar1 = 0xf00630c0;
        goto LAB_ram_f0020920;
      }
      if (DAT_ram_f0064fa9 == '\0') goto LAB_ram_f00209ac;
    }
    if (DAT_ram_f0064fe0 != '\0') goto LAB_ram_f0020a62;
    if (DAT_ram_f0064fe3 == '\0') {
      if (((DAT_ram_f0064fae == '\0') && (DAT_ram_f0064fe1 == '\0')) &&
         ((DAT_ram_f0064fe4 == '\0' && (DAT_ram_f0064faf == '\0')))) goto LAB_ram_f0020956;
      if (((DAT_ram_f0064fae == '\0') && (DAT_ram_f0064fe1 == '\0')) &&
         ((DAT_ram_f0064fe4 != '\0' || (DAT_ram_f0064faf != '\0')))) goto LAB_ram_f002097a;
      if (DAT_ram_f0064fae == '\0') goto LAB_ram_f00209ae;
    }
    if (((DAT_ram_f0064fe1 != '\0') || (DAT_ram_f0064fe4 != '\0')) || (DAT_ram_f0064faf != '\0')) {
LAB_ram_f00209ae:
      if ((((DAT_ram_f0064fe3 != '\0') || (DAT_ram_f0064fae != '\0')) &&
          ((DAT_ram_f0064fe1 == '\0' && ((DAT_ram_f0064fe4 != '\0' || (DAT_ram_f0064faf != '\0')))))
          ) || (DAT_ram_f0064fa9 != '\0')) goto LAB_ram_f0020a62;
      if ((DAT_ram_f0064fe0 == '\0') && (DAT_ram_f0064fe3 == '\0')) {
        if ((DAT_ram_f0064fae == '\0') &&
           (((DAT_ram_f0064fe1 == '\0' && (DAT_ram_f0064fe4 == '\0')) && (DAT_ram_f0064faf == '\0'))
           )) {
          uVar1 = 7;
          goto LAB_ram_f0020a56;
        }
        if (DAT_ram_f0064fae != '\0') goto LAB_ram_f0020a28;
        if (DAT_ram_f0064fe1 != '\0') goto LAB_ram_f0020a62;
        if ((DAT_ram_f0064fe4 != '\0') || (DAT_ram_f0064faf != '\0')) goto LAB_ram_f0020a24;
      }
      else {
LAB_ram_f0020a28:
        if (DAT_ram_f0064fe1 != '\0') goto LAB_ram_f0020a62;
      }
      if (((DAT_ram_f0064fe4 != '\0') || ((DAT_ram_f0064faf != '\0' || (DAT_ram_f0064fe0 != '\0'))))
         || ((DAT_ram_f0064fe3 == '\0' && (DAT_ram_f0064fae == '\0')))) {
LAB_ram_f0020a62:
        (*_DAT_ram_00014cf4)(0,_DAT_ram_00014cf4);
        return;
      }
      goto LAB_ram_f0020a54;
    }
  }
  else if (((DAT_ram_f0064fe3 == '\0') || (DAT_ram_f0064fe1 != '\0')) ||
          ((DAT_ram_f0064fe4 != '\0' || (DAT_ram_f0064faf != '\0')))) {
LAB_ram_f002081c:
    if (((DAT_ram_f0064fe3 != '\0') && (DAT_ram_f0064fe1 == '\0')) &&
       ((DAT_ram_f0064fe4 != '\0' || (DAT_ram_f0064faf != '\0')))) {
      uVar1 = 0xf00630b8;
      goto LAB_ram_f0020920;
    }
    goto LAB_ram_f0020858;
  }
LAB_ram_f002099e:
  uVar1 = 3;
LAB_ram_f00209a0:
  FUN_ram_f0020508(uVar1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020a78 @ 0xf0020a78 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0020a78(int param_1,int param_2)

{
  ushort uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  
  uVar3 = *(uint *)(param_1 + 0x40);
  uVar2 = (uVar3 & 0x7ffffff) >> 0x14;
  if ((uVar3 & 0x7ffff) >> 0x11 < 2) {
    uVar1 = (ushort)*(byte *)(uVar2 + 0xf0063aa8);
  }
  else {
    if ((int)uVar3 < 0) {
      iVar4 = -0xff9c540;
    }
    else {
      iVar4 = -0xff9c548;
    }
    uVar1 = (ushort)*(byte *)(iVar4 + uVar2);
    if ((uVar3 & 0x18000000) == 0x8000000) {
      uVar1 = uVar1 * 2;
    }
  }
  *(ushort *)(param_2 + 0x342) = uVar1;
  DAT_ram_f0064fab = 1;
  if ((DAT_ram_f0064fec == '\0') && (uVar1 < 0x24)) {
    DAT_ram_f0064fac = 1;
  }
  else if (0x5f < uVar1) {
    _DAT_ram_f0065078 = _DAT_ram_f0065078 + 1;
  }
  _DAT_ram_f006507c = _DAT_ram_f006507c + 1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020b14 @ 0xf0020b14 =====


undefined4 FUN_ram_f0020b14(void)

{
  undefined4 uVar1;
  
  uVar1 = 2;
  if (DAT_ram_f0064fa9 == '\0') {
    uVar1 = 8;
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020b28 @ 0xf0020b28 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0020b28(char param_1)

{
  undefined1 uVar1;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  
  if (DAT_ram_f0064fe8 == param_1) {
    return 1;
  }
  if (param_1 == '\0') {
    local_8c = 2;
  }
  else {
    local_8c = 0;
  }
  local_70 = 2;
  local_c = 2;
  local_18 = 2;
  if (param_1 == '\0') {
    if (DAT_ram_f0064fd5 == '\0') {
      local_10 = 2;
    }
    else {
      local_10 = 1;
    }
    local_14 = 1;
    local_1c = 1;
    if (DAT_ram_f0064fe9 != '\0') {
      uVar1 = 1;
      goto LAB_ram_f0020bf6;
    }
  }
  else {
    local_10 = 0;
    local_14 = 0;
    local_1c = 0;
  }
  uVar1 = 0;
LAB_ram_f0020bf6:
  DAT_ram_f0064fe8 = param_1;
  local_88 = local_8c;
  local_84 = local_8c;
  local_80 = local_8c;
  local_7c = local_8c;
  local_78 = local_8c;
  local_74 = local_8c;
  local_6c = local_8c;
  local_68 = local_8c;
  local_64 = local_8c;
  local_60 = local_8c;
  local_5c = local_8c;
  local_58 = local_8c;
  local_54 = local_8c;
  local_50 = local_8c;
  local_4c = local_8c;
  local_48 = local_8c;
  local_44 = local_8c;
  local_40 = local_8c;
  local_3c = local_8c;
  local_30 = local_8c;
  local_2c = local_8c;
  local_28 = local_8c;
  local_24 = local_8c;
  local_20 = local_8c;
  (*_DAT_ram_00014d24)(uVar1,_DAT_ram_00014d24);
  (*_DAT_ram_00014ce0)(&local_8c,_DAT_ram_00014ce0);
  FUN_ram_f0020710();
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020c1c @ 0xf0020c1c =====


/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0020c1c(undefined1 param_1)

{
  char cVar1;
  char cVar2;
  undefined1 uVar3;
  int *piVar4;
  int iVar5;
  
  piVar4 = (int *)&DAT_ram_020a0068;
  do {
    iVar5 = *piVar4;
    piVar4 = piVar4 + 1;
    if (*(char *)(iVar5 + 0x344) == '\0') {
      cVar1 = *(char *)(iVar5 + 0x18);
      cVar2 = DAT_ram_f0064fe3;
joined_r0xf0020c5e:
      if ((cVar1 != '\0') && (cVar2 != '\0')) {
        (*_DAT_ram_0001487c)(0,_DAT_ram_0001487c);
        return;
      }
    }
    else if (*(char *)(iVar5 + 0x344) == '\x01') {
      cVar1 = *(char *)(iVar5 + 0x18);
      cVar2 = DAT_ram_f0064fe4;
      goto joined_r0xf0020c5e;
    }
    if (piVar4 == (int *)&DAT_ram_020a0074) {
      uVar3 = param_1;
      if (DAT_ram_f0064fe5 == '\0') {
        uVar3 = 0;
      }
      (*_DAT_ram_0001487c)(uVar3,_DAT_ram_0001487c);
      DAT_ram_f0064fb0 = param_1;
      FUN_ram_f0020218(DAT_ram_f0064fe7);
      return;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020cb8 @ 0xf0020cb8 =====


void FUN_ram_f0020cb8(int param_1)

{
  if (param_1 == 0) {
    DAT_ram_f0064fae = 0;
  }
  else {
    DAT_ram_f0064faf = 0;
  }
  FUN_ram_f0020710();
  FUN_ram_f0020c1c(0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020cf4 @ 0xf0020cf4 =====


void FUN_ram_f0020cf4(int param_1)

{
  undefined1 *puVar1;
  undefined4 uVar2;
  
  if (param_1 == 0) {
    puVar1 = &DAT_ram_f0064fae;
  }
  else {
    puVar1 = &DAT_ram_f0064faf;
  }
  *puVar1 = 1;
  FUN_ram_f0020710();
  if ((DAT_ram_f0064fae == '\0') || (DAT_ram_f0064faf == '\0')) {
    uVar2 = 0;
  }
  else {
    uVar2 = 1;
  }
  FUN_ram_f0020c1c(uVar2);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020d4c @ 0xf0020d4c =====


undefined4 FUN_ram_f0020d4c(int param_1)

{
  if (param_1 == 0) {
    DAT_ram_f0064fe3 = 0;
  }
  else {
    DAT_ram_f0064fe4 = 0;
  }
  FUN_ram_f0020c1c(0);
  FUN_ram_f0020710();
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020d88 @ 0xf0020d88 =====


undefined4 FUN_ram_f0020d88(int param_1)

{
  undefined1 *puVar1;
  
  if (param_1 == 0) {
    puVar1 = &DAT_ram_f0064fe3;
  }
  else {
    puVar1 = &DAT_ram_f0064fe4;
  }
  *puVar1 = 1;
  FUN_ram_f0020c1c(0);
  FUN_ram_f0020710();
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020dc8 @ 0xf0020dc8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0020dc8(char param_1)

{
  undefined4 uVar1;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  
  local_8c = 1;
  if (DAT_ram_f0064fe8 == '\x01') {
    uVar1 = 0;
  }
  else if (DAT_ram_f0064fea == param_1) {
    uVar1 = 1;
  }
  else {
    if (param_1 == '\0') {
      uVar1 = 0x104000;
      local_8c = 2;
    }
    else {
      uVar1 = 0x104001;
    }
    DAT_ram_f0064fea = param_1;
    local_88 = local_8c;
    local_84 = local_8c;
    local_80 = local_8c;
    local_7c = local_8c;
    local_78 = local_8c;
    local_74 = local_8c;
    local_6c = local_8c;
    local_68 = local_8c;
    local_64 = local_8c;
    local_60 = local_8c;
    local_5c = local_8c;
    local_58 = local_8c;
    local_54 = local_8c;
    local_50 = local_8c;
    local_4c = local_8c;
    local_48 = local_8c;
    local_44 = local_8c;
    local_40 = local_8c;
    local_3c = local_8c;
    local_30 = local_8c;
    local_2c = local_8c;
    local_28 = local_8c;
    local_24 = local_8c;
    local_20 = local_8c;
    (*(code *)&SUB_ram_0006aa24)(0,0x6f,uVar1,&local_8c,local_8c,&SUB_ram_0006aa24);
    local_70 = 2;
    local_c = 2;
    local_18 = 2;
    local_14 = 1;
    local_1c = 1;
    if (DAT_ram_f0064feb == '\0') {
      if (DAT_ram_f0064fd5 == '\0') {
        local_10 = 2;
      }
      else {
        local_10 = 1;
      }
    }
    else {
      local_10 = 1;
    }
    (*_DAT_ram_00014ce0)(&local_8c,_DAT_ram_00014ce0);
    FUN_ram_f0020710();
    uVar1 = 1;
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020ed0 @ 0xf0020ed0 =====


undefined4 FUN_ram_f0020ed0(undefined4 *param_1)

{
  (*(code *)&SUB_ram_0006aa24)(0,0x6f,*param_1);
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0020ef4 @ 0xf0020ef4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0020ef4(int param_1)

{
  undefined1 *puVar1;
  undefined1 uVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  
  if (*(char *)(param_1 + 0x344) == '\0') {
    if ((*(int *)(param_1 + 0xc) - 1U < 2) || (uVar2 = 0, *(int *)(param_1 + 0xc) == 4)) {
      uVar2 = 1;
    }
    puVar1 = &DAT_ram_f0064fe0;
  }
  else {
    if (*(char *)(param_1 + 0x344) != '\x01') goto LAB_ram_f0020f38;
    if ((*(int *)(param_1 + 0xc) - 1U < 2) || (*(int *)(param_1 + 0xc) == 4)) {
      uVar2 = 1;
    }
    else {
      uVar2 = 0;
    }
    puVar1 = &DAT_ram_f0064fe1;
  }
  *puVar1 = uVar2;
LAB_ram_f0020f38:
  piVar5 = (int *)&DAT_ram_020a0068;
  piVar4 = piVar5;
  do {
    iVar3 = *piVar4;
    piVar4 = piVar4 + 1;
    if ((*(char *)(iVar3 + 0x18) != '\0') &&
       ((iVar3 = *(int *)(iVar3 + 0x324), iVar3 == 1 || (iVar3 == 3)))) {
      DAT_ram_f0064fe2 = 1;
      goto LAB_ram_f0020f9c;
    }
  } while (piVar4 != (int *)&DAT_ram_020a0074);
  DAT_ram_f0064fe2 = 0;
LAB_ram_f0020f9c:
  do {
    piVar4 = (int *)*piVar5;
    if (piVar4 != (int *)0x0) {
      if ((char)piVar4[6] == '\0') {
        FUN_ram_f0020c1c(1);
        if ((char)piVar4[0xd1] == '\0') {
          DAT_ram_f0064fe0 = 0;
LAB_ram_f0020fe6:
          DAT_ram_f0064fe3 = 0;
        }
        else if ((char)piVar4[0xd1] == '\x01') {
          DAT_ram_f0064fe1 = 0;
          DAT_ram_f0064fe4 = 0;
        }
      }
      else if ((char)piVar4[0xd1] == '\0') {
        DAT_ram_f0064fe3 = 0;
        if (*piVar4 == 1) {
          DAT_ram_f0064fae = 1;
          goto LAB_ram_f0020fe6;
        }
        DAT_ram_f0064fae = 0;
      }
      else if ((char)piVar4[0xd1] == '\x01') {
        if (*piVar4 == 1) {
          DAT_ram_f0064faf = 1;
          DAT_ram_f0064fe4 = 0;
        }
        else {
          DAT_ram_f0064fe4 = 0;
          DAT_ram_f0064faf = 0;
        }
        break;
      }
    }
    piVar5 = piVar5 + 1;
  } while (piVar5 != (int *)&DAT_ram_020a0074);
  DAT_ram_f0064fe5 = 1;
  DAT_ram_f0064fe6 = 0;
  iVar3 = FUN_ram_f003ab54(_DAT_ram_020a0068);
  if (((iVar3 != 0) || (iVar3 = FUN_ram_f003ab54(_DAT_ram_020a006c), iVar3 != 0)) ||
     (iVar3 = FUN_ram_f003ab54(_DAT_ram_020a0070), iVar3 != 0)) {
    DAT_ram_f0064fe6 = 1;
  }
  FUN_ram_f0020710();
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0021088 @ 0xf0021088 =====


void FUN_ram_f0021088(void)

{
  char cVar1;
  uint uVar2;
  byte bVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  int *piVar10;
  bool bVar11;
  
  DAT_ram_f0065184 = 2;
  piVar10 = (int *)&DAT_ram_f0065198;
  iVar9 = 0;
  bVar11 = true;
  do {
    iVar4 = *(int *)(&DAT_ram_020a0068 + iVar9);
    iVar8 = *(int *)(iVar4 + 0x2c8);
    iVar6 = *(int *)(iVar4 + 0x324);
    bVar3 = *(byte *)(iVar4 + 0x2cc);
    iVar4 = FUN_ram_f003ab54(iVar4);
    if ((iVar4 == 0) || (iVar8 != 0x24ba58)) {
      iVar6 = 0;
      iVar4 = 0;
    }
    else {
      if (iVar6 == 1) {
        cVar1 = '\x02';
LAB_ram_f0021100:
        bVar3 = bVar3 + cVar1;
        uVar7 = 0x2a;
      }
      else {
        if (iVar6 == 3) {
          cVar1 = -2;
          goto LAB_ram_f0021100;
        }
        uVar7 = 0x16;
      }
      uVar5 = (uint)bVar3;
      if (uVar5 == 0xe) {
        uVar2 = 0x9b4;
      }
      else {
        uVar2 = (uVar5 * 5000 + 0x24ba58) / 1000;
      }
      if (uVar5 == 0xe) {
        uVar5 = 0x9b4;
      }
      else {
        uVar5 = (uVar5 * 5000 + 0x24ba58) / 1000;
      }
      iVar6 = uVar5 + (uVar7 >> 1);
      iVar4 = uVar2 - (uVar7 >> 1);
    }
    if ((*piVar10 != iVar6) || (*(int *)(iVar9 + -0xff9ae78) != iVar4)) {
      *piVar10 = iVar6;
      *(int *)(iVar9 + -0xff9ae78) = iVar4;
      bVar11 = false;
    }
    iVar9 = iVar9 + 4;
    piVar10 = piVar10 + 1;
    if (iVar9 == 8) {
      if (!bVar11) {
        (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x40000);
      }
      return;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0021198 @ 0xf0021198 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0021198(void)

{
  bool bVar1;
  int iVar2;
  ushort uVar3;
  int iVar4;
  
  if (DAT_ram_f0064fab == '\0') {
    return;
  }
  if (DAT_ram_f0064fac == '\0') {
    iVar4 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    bVar1 = false;
    if (iVar4 - _DAT_ram_f0064f28 < 0) goto LAB_ram_f00211e6;
    _DAT_ram_f0064f28 = iVar4 + 0x8000;
    _DAT_ram_f0065078 = 0;
    _DAT_ram_f006507c = 0;
  }
  bVar1 = true;
LAB_ram_f00211e6:
  DAT_ram_f0064fab = 0;
  if (bVar1) {
    iVar4 = 0;
    uVar3 = 0x82;
    do {
      iVar2 = FUN_ram_f0025d90(iVar4);
      iVar4 = iVar4 + 1;
      if ((*(char *)(iVar2 + 9) != '\0') && (*(ushort *)(iVar2 + 0x342) < uVar3)) {
        uVar3 = *(ushort *)(iVar2 + 0x342);
      }
    } while (iVar4 != 0x14);
    if ((DAT_ram_f0064fec == '\0') && (uVar3 < 0x24)) {
      DAT_ram_f0064fec = '\x01';
      FUN_ram_f0020710();
    }
    if ((DAT_ram_f0064fec != '\0') && (0x30 < uVar3)) {
      DAT_ram_f0064fec = '\0';
      FUN_ram_f0020710();
    }
    DAT_ram_f0064fac = '\0';
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0021344 @ 0xf0021344 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0021344(uint *param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int iStack_1c;
  int iStack_18;
  undefined1 local_11 [5];
  
  (*_DAT_ram_000149bc)(&iStack_18,_DAT_ram_000149bc);
  if (iStack_18 == 0) {
    uVar1 = 0x4000000;
  }
  else {
    *param_1 = 0x4000080;
    (*_DAT_ram_000149a4)(&iStack_1c,_DAT_ram_000149a4);
    if (iStack_1c != 1) goto LAB_ram_f002138a;
    uVar1 = *param_1 | 0x40;
  }
  *param_1 = uVar1;
LAB_ram_f002138a:
  (*_DAT_ram_00014a60)(local_11,_DAT_ram_00014a60);
  iVar2 = FUN_ram_f003ab1c(local_11[0]);
  if (iVar2 == 0x24ba58) {
    uVar3 = *param_1 | 0x10;
    *param_1 = uVar3;
    uVar1 = FUN_ram_f003aaf8(local_11[0]);
    uVar3 = uVar1 & 0xf | uVar3;
  }
  else {
    uVar3 = *param_1 | 0x20;
  }
  *param_1 = uVar3;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00213e0 @ 0xf00213e0 =====


undefined4 FUN_ram_f00213e0(uint *param_1)

{
  if ((*param_1 & 0xffff0000) == 0x4000000) {
    FUN_ram_f0021344();
  }
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002140c @ 0xf002140c =====


void FUN_ram_f002140c(undefined4 param_1)

{
  FUN_ram_f00213e0();
  FUN_ram_f0020ed0(param_1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0021430 @ 0xf0021430 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0021430(void)

{
  uint auStack_c [2];
  
  if ((_DAT_ram_f0064f2c != 5000000) || (_DAT_ram_f006513c != 5000000)) {
    auStack_c[0] = 0x4000000;
    _DAT_ram_f0064f2c = _DAT_ram_f006513c;
    DAT_ram_f0064ffe = DAT_ram_f0065039;
    if (_DAT_ram_f0065140 == 1) {
      auStack_c[0] = 0x40000c0;
    }
    else if (_DAT_ram_f0065140 == 3) {
      auStack_c[0] = 0x4000080;
    }
    if (_DAT_ram_f006513c == 0x24ba58) {
      auStack_c[0] = auStack_c[0] | 0x10 | DAT_ram_f0065039 & 0xf;
    }
    else {
      auStack_c[0] = auStack_c[0] | 0x20;
    }
    FUN_ram_f002140c(auStack_c);
    if (DAT_ram_f0064ff7 != '\0') {
      FUN_ram_f0021088();
      FUN_ram_f0020298();
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00214f0 @ 0xf00214f0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00214f0(void)

{
  (*_DAT_ram_00014984)(0,8,_DAT_ram_00014984);
  (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x40000000);
  (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x80000);
  FUN_ram_f0044e24(0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0021538 @ 0xf0021538 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0021538(undefined4 param_1)

{
  _DAT_ram_800500a0 = _DAT_ram_800500a0 | 2;
  _DAT_ram_80000400 = 0x1e1d00;
  _DAT_ram_80050074 = _DAT_ram_80050074 & 0xff0000 | 0x66006666;
  _DAT_ram_80050084 = 0x17100000;
  _DAT_ram_80050088 = 0x1d1c;
  _DAT_ram_8005008c = 0x1615;
  _DAT_ram_60000014 = 0x1f;
  _DAT_ram_60000018 = 0x4040404;
  _DAT_ram_60000060 = 0xf0f0f0f0;
  FUN_ram_f0020010(0x19);
  FUN_ram_f00200e0(3);
  FUN_ram_f0020094(0x19);
  FUN_ram_f0020068(0x10);
  FUN_ram_f002003c(10);
  FUN_ram_f00200fc(0);
  (*_DAT_ram_0001487c)(0,_DAT_ram_0001487c);
  DAT_ram_f0064fa8 = 8;
  _DAT_ram_f0064f9e = 0xff;
  _DAT_ram_f0064f24 = 10;
  DAT_ram_f0064fe4 = 0;
  DAT_ram_f0064fe9 = 0;
  _DAT_ram_f0065084 = 0;
  _DAT_ram_f0065088 = 0;
  _DAT_ram_f006505a = 0;
  _DAT_ram_f0064f9c = 0xff;
  DAT_ram_f0064fea = 0;
  DAT_ram_f0064feb = 0;
  DAT_ram_f0064fa9 = 0;
  DAT_ram_f0064fef = 0;
  DAT_ram_f0064faa = 0;
  DAT_ram_f0064fe0 = 0;
  DAT_ram_f0064fe1 = 0;
  DAT_ram_f0064fe2 = 0;
  DAT_ram_f0064fae = 1;
  DAT_ram_f0064faf = 1;
  DAT_ram_f0064fe3 = 0;
  _DAT_ram_f0065180 = 0;
  DAT_ram_f0064ff0 = 0;
  DAT_ram_f0064ff1 = 0;
  DAT_ram_f0064ff2 = 0;
  DAT_ram_f0064ff3 = 0;
  DAT_ram_f0064fb1 = 1;
  _DAT_ram_f006505c = 0;
  _DAT_ram_f006505e = 0;
  _DAT_ram_f0065060 = 0;
  _DAT_ram_f0065062 = 0;
  DAT_ram_f0064ff4 = 0;
  DAT_ram_f0064ff5 = 0;
  DAT_ram_f0064fb2 = 0;
  DAT_ram_f0064fb3 = 0;
  DAT_ram_f0065184 = 2;
  _DAT_ram_f0065064 = 0;
  _DAT_ram_f006508c = 0;
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f0065198,0,0x10);
  (*(code *)&SUB_ram_0006a0a0)(0xf0065188,0,0x10);
  _DAT_ram_f0064fa0 = 0x69;
  DAT_ram_f0064fb0 = 1;
  _DAT_ram_f0064f2c = 0x24ba58;
  DAT_ram_f0064fed = 0;
  DAT_ram_f0064ff6 = 0;
  DAT_ram_f0064ff7 = 0;
  DAT_ram_f0064ff8 = 0;
  DAT_ram_f0064ff9 = 0;
  DAT_ram_f0064ffa = 0;
  DAT_ram_f0064fe7 = 0;
  DAT_ram_f0064ffb = 0;
  DAT_ram_f0064ffc = 0;
  DAT_ram_f0064ffd = 0;
  _DAT_ram_f0065066 = 0;
  DAT_ram_f0064ffe = 0;
  DAT_ram_f0064fec = 0;
  DAT_ram_f0064fe5 = 0;
  FUN_ram_f0020124(param_1);
  FUN_ram_f0020480(0,0);
  (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x80000000);
  (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x400001f);
  (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x10000001);
  (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x10000);
  DAT_ram_f0064fd5 = 0;
  FUN_ram_f00266a0(0xf006d794,&LAB_ram_f0021274,0);
  (*_DAT_ram_00014814)(2,0xf00630c8,_DAT_ram_00014814);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0021984 @ 0xf0021984 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0021984(uint *param_1)

{
  undefined1 uVar1;
  bool bVar2;
  undefined2 uVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  uint uVar7;
  undefined1 *puVar8;
  
  iVar4 = (*_DAT_ram_00014830)(param_1,_DAT_ram_00014830);
  if ((_DAT_ram_f0065084 == *param_1) && ((uint)(iVar4 - _DAT_ram_f0065088) < 0x8000)) {
    _DAT_ram_f0065088 = iVar4;
    return 1;
  }
  _DAT_ram_f0065084 = *param_1;
  uVar5 = *param_1;
  uVar7 = uVar5 & 0xffff0000;
  uVar1 = (undefined1)uVar5;
  _DAT_ram_f0065088 = iVar4;
  if (uVar7 == 0x100000) {
    puVar8 = &DAT_ram_f0064fb1;
  }
  else {
    if (uVar7 < 0x100001) {
      if (uVar7 == 0x50000) {
        if ((uVar5 & 0xffff) == 0xff) {
          return 1;
        }
        DAT_ram_f0065046 = (char)(uVar5 & 0xffff) * '\x02';
        return 1;
      }
      if (0x50000 < uVar7) {
        if (uVar7 == 0x60000) {
          DAT_ram_f0064fb4 = 1;
          DAT_ram_f0064fff = *(undefined1 *)((int)param_1 + 1);
          DAT_ram_f0065000 = uVar1;
          return 1;
        }
        if (uVar7 != 0x70000) {
          return 1;
        }
        DAT_ram_f0064fb4 = 0;
        DAT_ram_f0065001 = *(undefined1 *)((int)param_1 + 1);
        DAT_ram_f0065002 = uVar1;
        return 1;
      }
      if (uVar7 != 0x10000) {
        return 1;
      }
      if ((DAT_ram_f0064fad != '\0') &&
         (DAT_ram_f0064ff7 = (byte)(uVar5 >> 0xf) & 1, DAT_ram_f0064ffc = DAT_ram_f0064ff7,
         (*param_1 & 0x10) == 0)) {
        DAT_ram_f0064ffc = 0;
      }
      bVar2 = (*param_1 & 2) == 0;
      if (bVar2) {
        (*_DAT_ram_00014d1c)(0,_DAT_ram_00014d1c);
      }
      else {
        (*_DAT_ram_00014d1c)(1,_DAT_ram_00014d1c);
      }
      FUN_ram_f00200b8(!bVar2);
      bVar2 = (*param_1 & 4) == 0;
      if (bVar2) {
        (*_DAT_ram_00014cf4)(0,_DAT_ram_00014cf4);
        (*_DAT_ram_00014d24)(0,_DAT_ram_00014d24);
      }
      else {
        FUN_ram_f0020710();
        (*_DAT_ram_00014d24)(1,_DAT_ram_00014d24);
      }
      DAT_ram_f0064fe9 = !bVar2;
      if ((*param_1 & 0x40) != 0) {
        DAT_ram_f0064fed = 1;
        if (DAT_ram_f0064fd5 == '\0') {
          DAT_ram_f0064fed = 1;
          return 1;
        }
        FUN_ram_f002672c(0xf006d794,4000);
        return 1;
      }
      (*_DAT_ram_00014d1c)(0,_DAT_ram_00014d1c);
      FUN_ram_f00200b8(0);
      (*_DAT_ram_00014cf4)(0,_DAT_ram_00014cf4);
      (*_DAT_ram_00014d24)(0,_DAT_ram_00014d24);
      DAT_ram_f0064fed = 0;
      if (DAT_ram_f0064fd5 == '\0') {
        DAT_ram_f0064fed = 0;
        return 1;
      }
      FUN_ram_f00266d8(0xf006d794);
      return 1;
    }
    if (uVar7 != 0x800000) {
      if (0x800000 < uVar7) {
        if (uVar7 == 0x8000000) {
          FUN_ram_f0049bac();
          return 1;
        }
        if (uVar7 != 0x10000000) {
          return 1;
        }
        if ((uVar5 & 1) == 0) {
          DAT_ram_f0064fa9 = 0;
          if (DAT_ram_f0065003 != '\0') {
            FUN_ram_f003e080(0);
            DAT_ram_f0065003 = '\0';
          }
          if (DAT_ram_f0064ff6 != '\0') {
            FUN_ram_f0028f00(0);
            DAT_ram_f0064ff6 = '\0';
          }
          if ((*param_1 & 4) != 0) {
            DAT_ram_f0064faa = (*param_1 & 2) != 0;
            if ((bool)DAT_ram_f0064faa) {
              uVar6 = 0;
            }
            else {
              uVar6 = 3;
            }
            DAT_ram_f0064fef = !(bool)DAT_ram_f0064faa;
            (*_DAT_ram_00014d08)(uVar6,_DAT_ram_00014d08);
          }
        }
        else {
          DAT_ram_f0064fa9 = 1;
          if (DAT_ram_f0065003 == '\0') {
            FUN_ram_f003e080(1);
            DAT_ram_f0065003 = '\x01';
          }
          if (DAT_ram_f0064ff6 == '\0') {
            FUN_ram_f0028f00();
            DAT_ram_f0064ff6 = '\x01';
          }
        }
        FUN_ram_f0020710();
        return 1;
      }
      uVar3 = (short)uVar5;
      if (uVar7 != 0x200000) {
        return 1;
      }
      goto LAB_ram_f0021c84;
    }
    puVar8 = &DAT_ram_f0064ff0;
  }
  *puVar8 = uVar1;
  uVar3 = _DAT_ram_f0065064;
LAB_ram_f0021c84:
  _DAT_ram_f0065064 = uVar3;
  FUN_ram_f0020298();
  return 1;
}

// >>> MOD: wifi/mgmt/bss.c:0xe8a,0xe96 <<<
// ===== FUN_ram_f0021cc4 @ 0xf0021cc4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0021cc4(uint param_1)

{
  char cVar1;
  int iVar2;
  
  if (2 < param_1) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0xe8a,_DAT_ram_00014800);
  }
  iVar2 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (*(int *)(iVar2 + 0xc) == 2) {
    cVar1 = *(char *)(iVar2 + 0x73);
    if (cVar1 == '\0') {
      if (*(char *)(iVar2 + 0x72) == '\0') {
        (*_DAT_ram_00014800)(0,0,0xf0063108,0xe96,_DAT_ram_00014800);
      }
      cVar1 = *(char *)(iVar2 + 0x72);
    }
    *(char *)(iVar2 + 0x73) = cVar1 + -1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0021d30 @ 0xf0021d30 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0021d30(int *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  if (*param_1 == 1) {
    if ((char)param_1[0xd1] == '\0') {
      (*_DAT_ram_00014adc)(0,3,_DAT_ram_00014adc);
      (*_DAT_ram_00014aec)(0xfe,0xfe,0xff,0xff,_DAT_ram_00014aec);
      (*_DAT_ram_00014ae4)(0,1,_DAT_ram_00014ae4);
      uVar1 = 0;
      uVar2 = uVar1;
    }
    else {
      (*_DAT_ram_00014adc)(1,3,_DAT_ram_00014adc);
      (*_DAT_ram_00014aec)(0xff,0xff,0xfe,0xfe,_DAT_ram_00014aec);
      (*_DAT_ram_00014ae4)(1,1,_DAT_ram_00014ae4);
      uVar1 = 1;
      uVar2 = 0;
    }
  }
  else {
    if (*param_1 != 0) {
      return;
    }
    if ((char)param_1[0xd1] == '\0') {
      (*_DAT_ram_00014adc)(0,1,_DAT_ram_00014adc);
      (*_DAT_ram_00014ae4)(0,1,_DAT_ram_00014ae4);
      uVar1 = 0;
      uVar2 = 1;
    }
    else {
      (*_DAT_ram_00014adc)(1,1,_DAT_ram_00014adc);
      (*_DAT_ram_00014ae4)(1,1,_DAT_ram_00014ae4);
      uVar1 = 1;
      uVar2 = uVar1;
    }
  }
  (*_DAT_ram_00014a58)(uVar1,uVar2,_DAT_ram_00014a58);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0021e04 @ 0xf0021e04 =====


void FUN_ram_f0021e04(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  
  if ((((param_1 != (undefined4 *)0x0) && (*(char *)((int)param_1 + 9) != '\0')) &&
      (*(byte *)(param_1 + 5) < 3)) &&
     (iVar2 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 5) * 4),
     *(int *)(iVar2 + 0xc) == 2)) {
    puVar5 = (undefined4 *)(iVar2 + 0x5c);
    puVar3 = *(undefined4 **)(iVar2 + 0x5c);
    if (*(char *)((int)param_1 + 0x15) == '\x02') {
      if (puVar3 != puVar5) {
        for (; puVar3 != puVar5; puVar3 = (undefined4 *)*puVar3) {
          if (puVar3 == param_1) {
            return;
          }
        }
      }
      puVar3 = *(undefined4 **)(iVar2 + 0x60);
      *(undefined4 **)(iVar2 + 0x60) = param_1;
      *param_1 = puVar5;
      param_1[1] = puVar3;
      *puVar3 = param_1;
      iVar1 = *(int *)(iVar2 + 100) + 1;
    }
    else {
      if (puVar3 == puVar5) {
        return;
      }
      do {
        puVar4 = puVar3;
        if (puVar4 == puVar5) {
          return;
        }
        puVar3 = (undefined4 *)*puVar4;
      } while (puVar4 != param_1);
      puVar5 = (undefined4 *)puVar4[1];
      puVar3[1] = puVar5;
      *puVar5 = puVar3;
      puVar4[1] = 0;
      *puVar4 = 0;
      iVar1 = *(int *)(iVar2 + 100) + -1;
    }
    *(int *)(iVar2 + 100) = iVar1;
  }
  return;
}

// >>> MOD: wifi/mgmt/bss.c:0x9c9,0x9cb <<<
// ===== FUN_ram_f0021f24 @ 0xf0021f24 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0021f24(undefined2 *param_1,int param_2,undefined4 *param_3,undefined2 *param_4)

{
  undefined2 uVar1;
  undefined4 local_20;
  undefined2 local_1c;
  
  local_20 = _DAT_ram_f0063130;
  local_1c = (undefined2)_DAT_ram_f0063134;
  if (param_1 == (undefined2 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x9c9,_DAT_ram_00014800);
  }
  if (param_4 == (undefined2 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x9cb,_DAT_ram_00014800);
  }
  if (param_3 == (undefined4 *)0x0) {
    uVar1 = 0x80;
    param_3 = &local_20;
  }
  else {
    uVar1 = 0x50;
  }
  *param_1 = uVar1;
  (*(code *)&SUB_ram_0006a08c)(param_1 + 2,param_3,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 5,param_2 + 0x51,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 8,param_2 + 0x4b,6);
  param_1[0xb] = 0;
  param_1[0x10] = *(undefined2 *)(param_2 + 0x6a);
  param_1[0x11] = _DAT_ram_f0065076;
  *param_4 = 0xc;
  return;
}

// >>> MOD: wifi/mgmt/bss.c:0x489,0x48a,0x48b,0x4ae <<<
// ===== FUN_ram_f0021fe4 @ 0xf0021fe4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0021fe4(undefined2 *param_1,int param_2,byte param_3,char param_4)

{
  uint uVar1;
  undefined *puVar2;
  ushort uVar3;
  int iVar4;
  
  if (param_1 == (undefined2 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x489,_DAT_ram_00014800);
  }
  if (param_2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x48a,_DAT_ram_00014800);
  }
  if (2 < *(byte *)(param_2 + 0x14)) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x48b,_DAT_ram_00014800);
  }
  uVar1 = *(uint *)(param_2 + 0x10);
  iVar4 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_2 + 0x14) * 4);
  if ((uVar1 & 0x40) == 0) {
    if ((uVar1 & 0x20) == 0) {
      if ((uVar1 & 0x100) == 0) {
        if ((uVar1 & 0x80) == 0) {
          (*_DAT_ram_00014800)(0,0,0xf0063108,0x4ae,_DAT_ram_00014800);
          return;
        }
        if ((iVar4 != 0) && (*(int *)(iVar4 + 0xa8) == 1)) {
          puVar2 = (undefined *)0x10c8;
          goto LAB_ram_f00220c2;
        }
      }
      puVar2 = (undefined *)0xc8;
    }
    else {
      puVar2 = &dma_2dset;
    }
  }
  else if (*(char *)(param_2 + 0x37) == '\0') {
    puVar2 = &bpmtc;
  }
  else {
    puVar2 = (undefined *)0x11c8;
  }
LAB_ram_f00220c2:
  *param_1 = puVar2;
  (*(code *)&SUB_ram_0006a08c)(param_1 + 2,param_2 + 10,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 5,iVar4 + 0x51,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 8,iVar4 + 0x4b,6);
  param_1[0xb] = 0;
  uVar3 = param_3 & 7;
  if (param_4 != '\0') {
    uVar3 = uVar3 | 0x10;
  }
  param_1[0xc] = uVar3;
  return;
}

// >>> MOD: wifi/mgmt/bss.c:0x430,0x431,0x432,0x455 <<<
// ===== FUN_ram_f0022114 @ 0xf0022114 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0022114(undefined2 *param_1,int param_2)

{
  uint uVar1;
  undefined *puVar2;
  int iVar3;
  
  if (param_1 == (undefined2 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x430,_DAT_ram_00014800);
  }
  if (param_2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x431,_DAT_ram_00014800);
  }
  if (2 < *(byte *)(param_2 + 0x14)) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x432,_DAT_ram_00014800);
  }
  uVar1 = *(uint *)(param_2 + 0x10);
  iVar3 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_2 + 0x14) * 4);
  if ((uVar1 & 0x40) == 0) {
    if ((uVar1 & 0x20) == 0) {
      if ((uVar1 & 0x100) == 0) {
        if ((uVar1 & 0x80) == 0) {
          (*_DAT_ram_00014800)(0,0,0xf0063108,0x455,_DAT_ram_00014800);
          return;
        }
        if ((iVar3 != 0) && (*(int *)(iVar3 + 0xa8) == 1)) {
          puVar2 = (undefined *)0x1048;
          goto LAB_ram_f00221ea;
        }
      }
      puVar2 = (undefined *)0x48;
    }
    else {
      puVar2 = (undefined *)0x248;
    }
  }
  else if (*(char *)(param_2 + 0x37) == '\0') {
    puVar2 = &hsmp_saddr;
  }
  else {
    puVar2 = (undefined *)0x1148;
  }
LAB_ram_f00221ea:
  *param_1 = puVar2;
  (*(code *)&SUB_ram_0006a08c)(param_1 + 2,param_2 + 10,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 5,iVar3 + 0x51,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 8,iVar3 + 0x4b,6);
  param_1[0xb] = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0022800 @ 0xf0022800 =====


undefined4
FUN_ram_f0022800(int param_1,undefined1 param_2,undefined1 param_3,char param_4,undefined4 param_5)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  
  uVar2 = FUN_ram_f0025b3c();
  if ((uVar2 < 5) || (iVar3 = FUN_ram_f0025c48(0x22), iVar3 == 0)) {
    uVar4 = 0xc000009a;
  }
  else {
    FUN_ram_f0020c1c(0);
    FUN_ram_f0021fe4(*(int *)(iVar3 + 8) + 8,param_1,param_2,param_3);
    FUN_ram_f0034540(iVar3,param_1,*(undefined1 *)(param_1 + 0x14),*(int *)(iVar3 + 8) + 8,0x1a,
                     *(int *)(iVar3 + 8) + 0x22,0,param_5,0,0,0,0,0);
    if (param_4 == '\0') {
      bVar1 = *(byte *)(iVar3 + 0x24) & 0x7f;
    }
    else {
      bVar1 = *(byte *)(iVar3 + 0x24) | 0x80;
    }
    *(byte *)(iVar3 + 0x24) = bVar1;
    FUN_ram_f0037b58(iVar3);
    uVar4 = 0;
  }
  return uVar4;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00228c0 @ 0xf00228c0 =====


undefined4 FUN_ram_f00228c0(int param_1,char param_2,undefined4 param_3)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  
  uVar2 = FUN_ram_f0025b3c();
  if ((uVar2 < 5) || (iVar3 = FUN_ram_f0025c48(0x20), iVar3 == 0)) {
    uVar4 = 0xc000009a;
  }
  else {
    FUN_ram_f0020c1c(0);
    FUN_ram_f0022114(*(int *)(iVar3 + 8) + 8,param_1);
    FUN_ram_f0034540(iVar3,param_1,(uint)*(byte *)(param_1 + 0x14),*(int *)(iVar3 + 8) + 8,0x18,
                     *(int *)(iVar3 + 8) + 0x20,0,param_3,0,1,
                     *(undefined1 *)
                      (*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x349),
                     0xf,0);
    if (param_2 == '\0') {
      bVar1 = *(byte *)(iVar3 + 0x24) & 0x7f;
    }
    else {
      bVar1 = *(byte *)(iVar3 + 0x24) | 0x80;
    }
    *(byte *)(iVar3 + 0x24) = bVar1;
    FUN_ram_f0037b58(iVar3);
    uVar4 = 0;
  }
  return uVar4;
}

// >>> MOD: wifi/mgmt/bss.c:0x82b,0x837 <<<
// ===== FUN_ram_f0022990 @ 0xf0022990 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0022990(undefined4 param_1,uint param_2)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  byte bVar4;
  undefined4 *puVar5;
  uint uVar6;
  undefined4 *puVar7;
  uint uVar8;
  uint uVar9;
  undefined4 uVar10;
  
  iVar2 = _DAT_ram_020a0064;
  puVar7 = *(undefined4 **)(_DAT_ram_020a006c + 0x5c);
  puVar5 = (undefined4 *)(_DAT_ram_020a006c + 0x5c);
  if (puVar7 == puVar5) {
    uVar10 = 1;
  }
  else {
    uVar10 = 1;
    for (; puVar7 != puVar5; puVar7 = (undefined4 *)*puVar7) {
      if (*(char *)(puVar7 + 0xe) == '\0') {
        if (0x14 < *(ushort *)((int)puVar7 + 0x1a)) {
          (*_DAT_ram_00014800)(0,0,0xf0063108,0x82b,_DAT_ram_00014800);
        }
        uVar1 = *(ushort *)((int)puVar7 + 0x1a);
        if (uVar1 < 0x15) {
          iVar3 = (uint)(uVar1 >> 3) + iVar2;
          *(byte *)(iVar3 + 0x1f54) = ~(byte)(1 << (uVar1 & 7)) & *(byte *)(iVar3 + 0x1f54);
          uVar9 = *(uint *)(iVar2 + 0x1f64);
          uVar8 = *(uint *)(iVar2 + 0x1f68);
          uVar6 = (uint)*(ushort *)((int)puVar7 + 0x1a);
          if (uVar9 == uVar8) {
            if (uVar9 == uVar6) {
              *(undefined4 *)(iVar2 + 0x1f68) = 0;
              *(undefined4 *)(iVar2 + 0x1f64) = 0;
            }
          }
          else if (uVar9 == uVar6) {
            do {
              uVar9 = uVar9 + 1;
              if (uVar8 < uVar9) goto LAB_ram_f0022b8c;
            } while ((*(byte *)(((int)uVar9 >> 3) + iVar2 + 0x1f54) >> (uVar9 & 7) & 1) == 0);
LAB_ram_f0022b50:
            *(uint *)(iVar2 + 0x1f64) = uVar9;
          }
          else if (uVar8 == uVar6) {
            do {
              uVar8 = uVar8 - 1;
              if (uVar8 < uVar9) goto LAB_ram_f0022b8c;
            } while ((*(byte *)((int)uVar8 / 8 + iVar2 + 0x1f54) >> ((int)uVar8 % 8 & 0x1fU) & 1) ==
                     0);
LAB_ram_f0022b80:
            *(uint *)(iVar2 + 0x1f68) = uVar8;
          }
        }
      }
      else {
        if ((*(char *)((int)puVar7 + 0x2aa) == '\0') || (*(byte *)((int)puVar7 + 0x52) == 0xf)) {
          bVar4 = 0xf;
        }
        else {
          bVar4 = ~*(byte *)((int)puVar7 + 0x52) & 0xf;
        }
        if (*(ushort *)((int)puVar7 + 0x1a) < 0x15) {
          iVar3 = FUN_ram_f0034424(puVar7,bVar4,1);
          uVar1 = *(ushort *)((int)puVar7 + 0x1a);
          if (iVar3 == 0) {
            iVar3 = (uint)(uVar1 >> 3) + iVar2;
            *(byte *)(iVar3 + 0x1f54) = ~(byte)(1 << (uVar1 & 7)) & *(byte *)(iVar3 + 0x1f54);
            uVar9 = *(uint *)(iVar2 + 0x1f64);
            uVar8 = *(uint *)(iVar2 + 0x1f68);
            uVar6 = (uint)*(ushort *)((int)puVar7 + 0x1a);
            if (uVar9 == uVar8) {
              if (uVar9 == uVar6) {
                *(undefined4 *)(iVar2 + 0x1f64) = 0;
                *(undefined4 *)(iVar2 + 0x1f68) = 0;
              }
            }
            else {
              if (uVar9 == uVar6) {
                do {
                  uVar9 = uVar9 + 1;
                  if (uVar8 < uVar9) goto LAB_ram_f0022b8c;
                } while ((*(byte *)(((int)uVar9 >> 3) + iVar2 + 0x1f54) >> (uVar9 & 7) & 1) == 0);
                goto LAB_ram_f0022b50;
              }
              if (uVar8 == uVar6) {
                do {
                  uVar8 = uVar8 - 1;
                  if (uVar8 < uVar9) goto LAB_ram_f0022b8c;
                } while ((*(byte *)((int)uVar8 / 8 + iVar2 + 0x1f54) >> ((int)uVar8 % 8 & 0x1fU) & 1
                         ) == 0);
                goto LAB_ram_f0022b80;
              }
            }
          }
          else {
            iVar3 = (uint)(uVar1 >> 3) + iVar2;
            *(byte *)(iVar3 + 0x1f54) = (byte)(1 << (uVar1 & 7)) | *(byte *)(iVar3 + 0x1f54);
            if (((uint)*(ushort *)((int)puVar7 + 0x1a) < *(uint *)(iVar2 + 0x1f64)) ||
               (*(uint *)(iVar2 + 0x1f64) == 0)) {
              *(uint *)(iVar2 + 0x1f64) = (uint)*(ushort *)((int)puVar7 + 0x1a);
            }
            if ((*(uint *)(iVar2 + 0x1f68) < (uint)*(ushort *)((int)puVar7 + 0x1a)) ||
               (*(uint *)(iVar2 + 0x1f68) == 0)) {
              *(uint *)(iVar2 + 0x1f68) = (uint)*(ushort *)((int)puVar7 + 0x1a);
            }
            uVar10 = 0;
          }
        }
      }
LAB_ram_f0022b8c:
    }
    if (param_2 < 4) {
      (*(code *)&SUB_ram_0006a08c)(param_1,iVar2 + 0x1f54,param_2);
    }
    else {
      (*_DAT_ram_00014800)(0,0,0xf0063108,0x837,_DAT_ram_00014800);
    }
  }
  return uVar10;
}

// >>> MOD: wifi/mgmt/bss.c:0x895,0x89d,0x8a6,0x8c9 <<<
// ===== FUN_ram_f0022bd0 @ 0xf0022bd0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0022bd0(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  byte bVar2;
  int iVar3;
  byte bVar4;
  uint uVar5;
  uint uVar6;
  undefined1 *puVar7;
  byte *pbVar8;
  uint uStack_30;
  undefined1 auStack_24 [3];
  char local_21 [5];
  
  if (*(char *)(param_2 + 0x19) != DAT_ram_f0064fd2) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0x895,_DAT_ram_00014800);
  }
  if ((DAT_ram_f0064fd3 == '\0') && (param_3 == 0)) {
    pbVar8 = (byte *)(*(int *)(param_1 + 0x18) + _DAT_ram_f0065090);
    if (pbVar8 == (byte *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf0063108,0x8a6,_DAT_ram_00014800);
    }
    uVar5 = _DAT_ram_f0065090 & 0xffff;
    uVar1 = *(ushort *)(param_1 + 0x12);
    *(undefined2 *)(param_1 + 0x12) = _DAT_ram_f0065090;
    uVar5 = uVar1 - uVar5 & 0xffff;
  }
  else {
    pbVar8 = (byte *)(*(int *)(param_1 + 0x18) + (uint)*(ushort *)(param_1 + 0x12));
    if (pbVar8 == (byte *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf0063108,0x89d,_DAT_ram_00014800);
    }
    (*(code *)&SUB_ram_0006a08c)(pbVar8,&DAT_ram_f0070238,_DAT_ram_020a0094);
    _DAT_ram_f0065090 = (uint)*(ushort *)(param_1 + 0x12);
    uVar5 = (uint)_DAT_ram_020a0094;
  }
  uStack_30 = 0;
  puVar7 = &DAT_ram_f0070238;
  do {
    if (uVar5 <= uStack_30) {
      *(short *)(param_1 + 0x12) = (short)uVar5 + *(short *)(param_1 + 0x12);
      return;
    }
    while (bVar2 = *pbVar8, bVar2 != 0x2a) {
      if (bVar2 < 0x2b) {
        if (bVar2 == 0) {
          if ((*(char *)(param_2 + 0x29) != '\0') && (param_3 != 0)) {
            bVar2 = puVar7[1];
            uVar6 = (uint)_DAT_ram_020a0094;
            pbVar8[1] = *(byte *)(param_2 + 0x2a);
            bVar4 = *(byte *)(param_2 + 0x2a);
            if (bVar4 != 0) {
              if (0x20 < bVar4) {
                bVar4 = 0x20;
              }
              (*(code *)&SUB_ram_0006a08c)(pbVar8 + 2,param_2 + 0x2b,bVar4);
            }
            (*(code *)&SUB_ram_0006a08c)
                      (pbVar8 + pbVar8[1] + 2,puVar7 + bVar2 + 2,
                       (uVar6 + 0x238) - (int)(puVar7 + bVar2 + 2) & 0xffff);
            uVar5 = uVar5 + pbVar8[1] & 0xffff;
          }
          goto LAB_ram_f0022fb0;
        }
        if (bVar2 != 5) goto LAB_ram_f0022fb0;
        if (*(int *)(param_2 + 0xc) != 2) {
          (*_DAT_ram_00014800)(0,0,0xf0063108,0x8c9,_DAT_ram_00014800);
        }
        if (param_3 == 0) {
          pbVar8[2] = *(byte *)(param_2 + 0x73);
          if (*(char *)(param_2 + 0x73) == '\0') {
            bVar2 = FUN_ram_f00343a8(*(undefined1 *)(param_2 + 0x19));
            pbVar8[4] = bVar2;
          }
          else {
            pbVar8[4] = 0;
          }
          iVar3 = FUN_ram_f0022990(auStack_24,3);
          if (iVar3 == 0) {
            (*(code *)&SUB_ram_0006a08c)(pbVar8 + 5,auStack_24,3);
            uVar6 = 6;
          }
          else {
            pbVar8[5] = 0;
            uVar6 = 4;
          }
          bVar2 = pbVar8[1];
          if (uVar6 != bVar2) {
            bVar4 = puVar7[1];
            pbVar8[1] = (byte)uVar6;
            uVar5 = (uVar5 - bVar2) + uVar6 & 0xffff;
            (*(code *)&SUB_ram_0006a08c)
                      (pbVar8 + uVar6 + 2,puVar7 + bVar4 + 2,
                       &DAT_ram_f0070238 + ((uint)_DAT_ram_020a0094 - (int)(puVar7 + bVar4 + 2)));
          }
          goto LAB_ram_f0022fb0;
        }
        (*(code *)&SUB_ram_0006a08c)
                  (pbVar8,puVar7 + (byte)puVar7[1] + 2,
                   (uint)(&DAT_ram_f0070238 +
                         ((uint)_DAT_ram_020a0094 - (int)(puVar7 + (byte)puVar7[1] + 2))) & 0xffff);
        bVar2 = puVar7[1];
      }
      else {
        if (bVar2 == 0x3d) {
          if (*(int *)(param_2 + 0xc) == 2) {
            if (*(int *)(param_2 + 0x2d4) == 3) {
              pbVar8[4] = 3;
            }
            else if (*(int *)(param_2 + 0x2e4) == 1) {
              pbVar8[4] = 1;
            }
            else {
              pbVar8[4] = (byte)*(int *)(param_2 + 0x2d4);
            }
            pbVar8[5] = 0;
            if (*(int *)(param_2 + 0x2d8) != 0) {
              pbVar8[5] = (byte)((ushort)*(undefined2 *)(pbVar8 + 4) >> 8);
              pbVar8[4] = (byte)*(undefined2 *)(pbVar8 + 4) | 4;
            }
          }
          goto LAB_ram_f0022fb0;
        }
        if (bVar2 != 0xdd) goto LAB_ram_f0022fb0;
        local_21[0] = '\0';
        iVar3 = FUN_ram_f002a1ac(pbVar8,local_21);
        if (((iVar3 == 0) || (param_3 == 0)) || (local_21[0] != '\t')) goto LAB_ram_f0022fb0;
        (*(code *)&SUB_ram_0006a08c)
                  (pbVar8,puVar7 + (byte)puVar7[1] + 2,
                   (uint)(&DAT_ram_f0070238 +
                         ((uint)_DAT_ram_020a0094 - (int)(puVar7 + (byte)puVar7[1] + 2))) & 0xffff);
        bVar2 = puVar7[1];
      }
      uVar5 = (-2 - (uint)bVar2) + uVar5 & 0xffff;
      puVar7 = puVar7 + bVar2 + 2;
    }
    if (*(int *)(param_2 + 0xc) == 2) {
      if (*(char *)(param_2 + 0x2e0) == '\0') {
        bVar2 = 0;
      }
      else {
        bVar2 = 2;
      }
      pbVar8[2] = bVar2;
      if (*(char *)(param_2 + 0x2d2) != '\0') {
        pbVar8[2] = 3;
      }
      if (*(char *)(param_2 + 0x82) == '\0') {
        pbVar8[2] = pbVar8[2] | 4;
      }
    }
LAB_ram_f0022fb0:
    uStack_30 = uStack_30 + pbVar8[1] + 2 & 0xffff;
    puVar7 = puVar7 + (byte)puVar7[1] + 2;
    pbVar8 = pbVar8 + pbVar8[1] + 2;
  } while( true );
}

// >>> MOD: wifi/mgmt/bss.c:0xa17 <<<
// ===== FUN_ram_f0022fe8 @ 0xf0022fe8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0022fe8(uint param_1,int param_2)

{
  short sVar1;
  uint uVar2;
  int iVar3;
  char cVar4;
  int iVar5;
  undefined4 uVar6;
  undefined2 local_22 [3];
  
  if (2 < param_1) {
    (*_DAT_ram_00014800)(0,0,0xf0063108,0xa17,_DAT_ram_00014800);
  }
  sVar1 = _DAT_ram_020a0094;
  iVar5 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (param_2 == 0) {
    iVar3 = *(int *)(iVar5 + 0x7c);
  }
  else {
    uVar2 = FUN_ram_f0025b3c();
    if ((uVar2 < 8) || (iVar3 = FUN_ram_f0025c48(sVar1 + 0x2c), iVar3 == 0)) {
      return 0xc000009a;
    }
  }
  if (iVar3 == 0) {
    return 0xc0000001;
  }
  if ((DAT_ram_f0064fd3 == '\0') && (param_2 == 0)) {
    *(undefined1 *)(iVar3 + 0x54) = 0;
    *(undefined1 *)(iVar3 + 0x29) = 0;
    goto LAB_ram_f0023136;
  }
  FUN_ram_f0021f24(*(int *)(iVar3 + 8) + 8,iVar5,param_2,local_22);
  if ((param_1 == 1) &&
     ((*(char *)(_DAT_ram_020a0064 + 0x1a84) != '\0' &&
      (*(char *)(_DAT_ram_020a0064 + 0x1aa2) != '\0')))) {
    if (_DAT_ram_f006513c == 0x24ba58) {
      if (*(int *)(iVar3 + 0x20) == 0) {
        cVar4 = *(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar3 + 0xd) * 4) + 0x349);
      }
      else {
        cVar4 = *(char *)(*(int *)(iVar3 + 0x20) + 0x24);
      }
      if (cVar4 == '\0') {
        cVar4 = '\x01';
        uVar6 = 1;
        goto LAB_ram_f00230e8;
      }
    }
    else {
      cVar4 = 'k';
    }
    uVar6 = 1;
  }
  else {
    cVar4 = 'k';
    uVar6 = 0;
  }
LAB_ram_f00230e8:
  FUN_ram_f0034540(iVar3,0,param_1 & 0xff,*(int *)(iVar3 + 8) + 8,0x18,*(int *)(iVar3 + 8) + 0x20,
                   local_22[0],0,0,uVar6,cVar4,0,0);
LAB_ram_f0023136:
  FUN_ram_f0022bd0(iVar3,iVar5,param_2);
  if (param_2 == 0) {
    DAT_ram_f0064fd3 = '\0';
  }
  FUN_ram_f0037b58(iVar3);
  return 0;
}

// >>> MOD: wifi/mgmt/bss.c:0x360 <<<
// ===== FUN_ram_f002365c @ 0xf002365c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002365c(void)

{
  int iVar1;
  undefined4 *puVar2;
  char cVar3;
  undefined4 *puStack_2c;
  undefined4 local_28;
  undefined2 local_24;
  
  cVar3 = '\0';
  puStack_2c = (undefined4 *)&DAT_ram_020a0068;
  do {
    puVar2 = (undefined4 *)*puStack_2c;
    local_28 = 0;
    local_24 = 0;
    puVar2[3] = 0;
    *(char *)((int)puVar2 + 0x19) = cVar3;
    *puVar2 = 1;
    puVar2[1] = 1;
    *(undefined1 *)(puVar2 + 6) = 0;
    *(undefined1 *)(puVar2 + 10) = 0;
    (*(code *)&SUB_ram_0006a08c)((int)puVar2 + 0x4b,&local_28,6);
    puVar2[0x18] = puVar2 + 0x17;
    puVar2[0xd5] = puVar2 + 0xd4;
    puVar2[0x17] = puVar2 + 0x17;
    puVar2[0x19] = 0;
    *(undefined1 *)((int)puVar2 + 0x7b) = 0;
    *(undefined1 *)((int)puVar2 + 0x349) = 0;
    puVar2[0xd4] = puVar2 + 0xd4;
    puVar2[0xd6] = 0;
    if (cVar3 == '\0') {
      *(undefined1 *)((int)puVar2 + 0x347) = 0;
      *(undefined1 *)(puVar2 + 0xd1) = 0;
      *(undefined1 *)((int)puVar2 + 0x345) = 0;
      *(undefined1 *)((int)puVar2 + 0x346) = 0;
      iVar1 = _DAT_ram_f0064f94;
      puVar2[0x1f] = _DAT_ram_f0064f94;
      if (iVar1 == 0) {
        (*_DAT_ram_00014800)(0,0,0xf0063108,0x360,_DAT_ram_00014800);
      }
      (*(code *)&SUB_ram_0006a0a0)(puVar2[0x1f],0,0x70);
      *(undefined4 *)(puVar2[0x1f] + 8) = _DAT_ram_0209fef4;
      *(undefined1 *)(puVar2[0x1f] + 0xc) = 4;
    }
    else {
      *(undefined1 *)((int)puVar2 + 0x347) = 0;
      *(undefined1 *)(puVar2 + 0xd1) = 1;
      *(undefined1 *)((int)puVar2 + 0x345) = 1;
      *(undefined1 *)((int)puVar2 + 0x346) = 1;
    }
    cVar3 = cVar3 + '\x01';
    puStack_2c = puStack_2c + 1;
  } while (cVar3 != '\x03');
  if (_DAT_ram_f00650ac != 0) {
    FUN_ram_f002460c();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023778 @ 0xf0023778 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f0023778(void)

{
  _DAT_ram_f0065094 = _DAT_ram_f0065094 + 1;
  if (_DAT_ram_f0065094 == -1) {
    _DAT_ram_f0065094 = 1;
  }
  return _DAT_ram_f0065094;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023798 @ 0xf0023798 =====


undefined4 FUN_ram_f0023798(void)

{
  return 1;
}

// >>> MOD: wifi/mgmt/cnm.c:0x3e4 <<<
// ===== FUN_ram_f002379c @ 0xf002379c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * FUN_ram_f002379c(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = *(undefined4 **)(_DAT_ram_f00650a0 + 0x48);
  puVar2 = (undefined4 *)(_DAT_ram_f00650a0 + 0x48);
  while( true ) {
    if (puVar1 == puVar2) {
      return (undefined4 *)0x0;
    }
    if (puVar1 == (undefined4 *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf0063138,0x3e4,_DAT_ram_00014800);
    }
    if (*(int *)(_DAT_ram_f00650a0 + (uint)*(byte *)(puVar1 + 3) * 0x18) == 0) break;
    puVar1 = (undefined4 *)*puVar1;
  }
  return puVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00237f4 @ 0xf00237f4 =====


void FUN_ram_f00237f4(void)

{
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00237f8 @ 0xf00237f8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00237f8(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 800);
  if (iVar1 != 0) {
    if (param_1 == 1) {
      if ((*(char *)(iVar1 + 0x4c) != '\0') &&
         ((*(int *)(iVar1 + 0x50) != 0 || (*(char *)(_DAT_ram_020a0064 + 0x1ef4) == '\0')))) {
        *(undefined1 *)(iVar1 + 0x4c) = 0;
      }
    }
    else if (((param_1 == 2) && (*(char *)(iVar1 + 0x4d) != '\0')) &&
            ((1 < *(uint *)(iVar1 + 0x5c) ||
             (*(char *)(_DAT_ram_020a0064 + *(uint *)(iVar1 + 0x5c) * 0x14 + 0x1f2c) == '\0')))) {
      *(undefined1 *)(iVar1 + 0x4d) = 0;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023860 @ 0xf0023860 =====


undefined4 FUN_ram_f0023860(void)

{
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023864 @ 0xf0023864 =====


undefined4 FUN_ram_f0023864(void)

{
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023868 @ 0xf0023868 =====


undefined4 FUN_ram_f0023868(void)

{
  return 2000;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023870 @ 0xf0023870 =====


undefined4
FUN_ram_f0023870(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 *in_stack_00000004;
  undefined4 *in_stack_00000008;
  undefined4 *in_stack_0000000c;
  
  *in_stack_00000004 = param_2;
  *in_stack_00000008 = param_3;
  *in_stack_0000000c = param_4;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023880 @ 0xf0023880 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0023880(int param_1)

{
  uint uVar1;
  int iVar2;
  uint uStack_c;
  
  (*_DAT_ram_00014a90)(&uStack_c,_DAT_ram_00014a90);
  uVar1 = 0;
  uStack_c = uStack_c | 0x810;
  do {
    iVar2 = param_1 + uVar1;
    if (*(byte *)(param_1 + 8) <= uVar1) goto LAB_ram_f00238f2;
    uVar1 = uVar1 + 1 & 0xff;
    if (DAT_ram_f0064fd4 == '\x01') {
      uStack_c = uStack_c & 0xffffffef;
    }
  } while ((1 < *(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar2 + 9) * 4) + 0xc) - 1U)
          && (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar2 + 9) * 4) + 0xc) != 4));
  uStack_c = uStack_c & 0xfffff7ef;
LAB_ram_f00238f2:
  (*_DAT_ram_00014af0)(uStack_c,_DAT_ram_00014af0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023930 @ 0xf0023930 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0023930(int param_1)

{
  char cVar1;
  int iVar2;
  char *pcVar3;
  uint uVar4;
  int local_28 [3];
  
  iVar2 = (*_DAT_ram_00014c14)(param_1,_DAT_ram_00014c14);
  iVar2 = *(int *)(param_1 + 0x10) - iVar2;
  if (iVar2 < 0x92e) {
    iVar2 = 0x546;
  }
  else {
    iVar2 = iVar2 + -1000;
  }
  pcVar3 = (char *)(param_1 + 9);
  uVar4 = 0;
  while (uVar4 < *(byte *)(param_1 + 8)) {
    uVar4 = uVar4 + 1;
    if (*pcVar3 == '\x01') {
      (*_DAT_ram_00014bf0)
                (*(undefined1 *)(_DAT_ram_020a006c + 0x346),local_28,&DAT_ram_00014bf0,
                 _DAT_ram_00014bf0);
      *(int *)(param_1 + 0x54) = local_28[0] + iVar2;
      *(int *)(param_1 + 0x58) = (*(int *)(param_1 + 0x14) + 1000) - *(int *)(param_1 + 0x10);
    }
    cVar1 = *pcVar3;
    pcVar3 = pcVar3 + 1;
    if (cVar1 == '\x02') {
      (*_DAT_ram_00014bf0)
                (*(undefined1 *)(_DAT_ram_020a0070 + 0x346),local_28,&DAT_ram_00014bf0,
                 _DAT_ram_00014bf0);
      *(int *)(param_1 + 100) = local_28[0] + iVar2;
      *(int *)(param_1 + 0x68) = (*(int *)(param_1 + 0x14) + 1000) - *(int *)(param_1 + 0x10);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0xa1d <<<
// ===== FUN_ram_f00239e8 @ 0xf00239e8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f00239e8(int param_1)

{
  int iVar1;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063138,0xa1d,_DAT_ram_00014800);
  }
  if (*(uint *)(_DAT_ram_f00650a0 + 0x264) < 2) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(param_1 + 0x14) - *(int *)(param_1 + 0x10);
    iVar1 = iVar1 * (uint)(-1 < iVar1) + 0x1ebf18;
  }
  return iVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023a38 @ 0xf0023a38 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0023a38(int param_1)

{
  int iVar1;
  
  (*_DAT_ram_00014bcc)(0,_DAT_ram_00014bcc);
  _DAT_ram_60310000 = 0x100000;
  iVar1 = (*_DAT_ram_00014c14)(_DAT_ram_00014c14);
  param_1 = param_1 - iVar1;
  (*_DAT_ram_00014b74)
            (0,(int)((param_1 * (uint)(0x8c9 < param_1) | (uint)(param_1 < 0x8ca) * 0x8ca) + 0x3ff)
               >> 10 & 0xffff,_DAT_ram_00014b74);
  (*_DAT_ram_00014bcc)(1,_DAT_ram_00014bcc);
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0xd7a <<<
// ===== FUN_ram_f0023a90 @ 0xf0023a90 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0023a90(int param_1,char param_2)

{
  bool bVar1;
  char cVar2;
  uint uVar3;
  int *piVar4;
  int iVar5;
  char cVar6;
  undefined4 local_18;
  undefined2 local_14;
  
  local_18 = 0;
  local_14 = 0;
  if (param_2 == '\0') {
    (*_DAT_ram_00014dac)(*(undefined1 *)(param_1 + 0x345),0,param_1 + 0x51,_DAT_ram_00014dac);
    (*_DAT_ram_00014da4)(*(undefined1 *)(param_1 + 0x344),&local_18,_DAT_ram_00014da4);
  }
  else {
    piVar4 = (int *)&DAT_ram_020a0068;
    cVar2 = '\0';
    do {
      iVar5 = *piVar4;
      cVar6 = cVar2 + '\x01';
      piVar4 = piVar4 + 1;
      if (((*(char *)(iVar5 + 0x18) != '\0') && (*(char *)(param_1 + 0x19) != cVar2)) &&
         (*(char *)(param_1 + 0x346) == *(char *)(iVar5 + 0x346))) {
        bVar1 = *(char *)(param_1 + 0x346) != '\x01';
        *(bool *)(param_1 + 0x345) = bVar1;
        *(bool *)(param_1 + 0x346) = bVar1;
        *(bool *)(param_1 + 0x344) = bVar1;
        break;
      }
      cVar2 = cVar6;
    } while (cVar6 != '\x03');
    uVar3 = *(uint *)(param_1 + 0x344) & 0xffffff;
    if ((uVar3 != 0) && (uVar3 != 0x10101)) {
      (*_DAT_ram_00014800)(0,0,0xf0063138,0xd7a,_DAT_ram_00014800);
    }
    (*_DAT_ram_00014dac)(*(undefined1 *)(param_1 + 0x345),1,param_1 + 0x51,_DAT_ram_00014dac);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023b48 @ 0xf0023b48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f0023b48(char param_1,int param_2)

{
  int iVar1;
  
  if (param_1 == '\0') {
    if (param_2 == 0x24ba58) {
      iVar1 = (int)DAT_ram_f006d7ac;
    }
    else {
      iVar1 = (int)DAT_ram_f006d7b0;
    }
  }
  else if (param_1 == '\x01') {
    if (*(char *)(_DAT_ram_020a0064 + 0x1a84) == '\0') {
      if (param_2 == 0x24ba58) {
        iVar1 = (int)DAT_ram_f006d7ae;
      }
      else {
        iVar1 = (int)DAT_ram_f006d7b2;
      }
    }
    else if (param_2 == 0x24ba58) {
      iVar1 = (int)DAT_ram_f006d7ad;
    }
    else {
      iVar1 = (int)DAT_ram_f006d7b1;
    }
  }
  else if (param_2 == 0x24ba58) {
    iVar1 = (int)DAT_ram_f006d7af;
  }
  else {
    iVar1 = (int)DAT_ram_f006d7b3;
  }
  return iVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0023bdc @ 0xf0023bdc =====


void FUN_ram_f0023bdc(int param_1)

{
  char cVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  
  if (param_1 != 0) {
    uVar4 = 0;
    iVar3 = 0;
    while( true ) {
      cVar1 = (char)iVar3;
      if (*(byte *)(param_1 + 8) <= uVar4) break;
      iVar2 = FUN_ram_f0023b48(*(undefined1 *)(param_1 + uVar4 + 9),*(undefined4 *)(param_1 + 0x6c))
      ;
      if (uVar4 != 0) {
        if (DAT_ram_f006d7b4 == '\0') {
          iVar2 = (int)(char)((char)iVar2 * (iVar3 <= iVar2) | cVar1 * (iVar2 < iVar3));
        }
        else {
          iVar2 = (int)(char)((char)iVar2 * (iVar2 < iVar3) | cVar1 * (iVar3 <= iVar2));
        }
      }
      uVar4 = uVar4 + 1 & 0xff;
      iVar3 = iVar2;
    }
    *(char *)(param_1 + 0x90) = cVar1;
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0xb68 <<<
// ===== FUN_ram_f0023c3c @ 0xf0023c3c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0023c3c(int param_1,byte param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint extraout_a1;
  int iVar4;
  int iVar5;
  uint uVar6;
  
  uVar6 = (uint)param_2;
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063138,0xb68,_DAT_ram_00014800);
    uVar6 = extraout_a1;
  }
  iVar1 = param_1 + 0x1c;
  iVar2 = (*_DAT_ram_00014c14)(iVar1,uVar6,_DAT_ram_00014c14);
  iVar4 = *(int *)(param_1 + 0x10);
  iVar3 = FUN_ram_f00239e8(param_1);
  FUN_ram_f00266d8(iVar1);
  FUN_ram_f00266d8(param_1 + 0x34);
  *(undefined1 *)(_DAT_ram_f00650a0 + 0x9d) = 0;
  for (uVar6 = 0; uVar6 < *(byte *)(param_1 + 8); uVar6 = uVar6 + 1 & 0xff) {
    iVar5 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + uVar6 + 9) * 4);
    if (*(uint *)(iVar5 + 0xc) < 2) {
      if ((*(int *)(_DAT_ram_f00650a0 + 0x264) == 1) && (param_2 != 0)) {
        FUN_ram_f0030278(*(undefined1 *)(iVar5 + 0x19));
        FUN_ram_f002be74(*(undefined1 *)(iVar5 + 0x19),0,0);
      }
      else if ((15000 - iVar4) + iVar2 < 0) {
        FUN_ram_f0030278(*(undefined1 *)(iVar5 + 0x19),1);
        iVar5 = (*(int *)(param_1 + 0x10) + -0x36b1) - iVar2;
        FUN_ram_f002672c(iVar1,iVar5 / 1000,1000,iVar5 % 1000);
      }
      else {
        FUN_ram_f002fd54(*(undefined1 *)(iVar5 + 0x19));
        FUN_ram_f002be74(*(undefined1 *)(iVar5 + 0x19),_DAT_ram_f0065098 - 1U < 2,
                         iVar3 + 0x3ffU >> 10 & 0xffff);
        if (*(int *)(_DAT_ram_020a0064 + (*(byte *)(iVar5 + 0x19) + 0x3d6) * 4 + 4) == 1) {
          *(undefined1 *)(_DAT_ram_f00650a0 + 0x9d) = 1;
        }
      }
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0xdcf <<<
// ===== FUN_ram_f0023d9c @ 0xf0023d9c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0023d9c(byte param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  
  if ((1 < *(uint *)(_DAT_ram_f00650a0 + 0x264)) && (_DAT_ram_f0065098 != 2)) {
    iVar2 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
    if ((_DAT_ram_f00650a0 + 0x25c == iVar2) || (iVar2 == 0)) {
      (*_DAT_ram_00014800)(0,0,0xf0063138,0xdcf,_DAT_ram_00014800);
      iVar2 = 0;
    }
    iVar4 = *(int *)(*(int *)(&DAT_ram_020a0068 + (uint)param_1 * 4) + 800);
    if ((iVar4 == iVar2) && (*(int *)(iVar2 + 0x10) - *(int *)(iVar2 + 0x14) < 0)) {
      puVar1 = (undefined4 *)(_DAT_ram_f00650a0 + 0x25c);
      for (puVar3 = *(undefined4 **)(_DAT_ram_f00650a0 + 0x25c); puVar3 != puVar1;
          puVar3 = (undefined4 *)*puVar3) {
        puVar3[4] = puVar3[4] + param_2;
        puVar3[5] = puVar3[5] + param_2;
      }
      FUN_ram_f0023a38(*(undefined4 *)(iVar4 + 0x10));
      FUN_ram_f0023c3c(iVar4,0);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0x5fa <<<
// ===== FUN_ram_f0023e50 @ 0xf0023e50 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0023e50(int param_1,int param_2,uint param_3)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  int aiStack_28 [3];
  
  if ((*(int *)(_DAT_ram_f00650a0 + 0x264) == 2) && (_DAT_ram_f0065098 != 2)) {
    piVar3 = *(int **)(_DAT_ram_f00650a0 + 0x25c);
    if (((int *)(_DAT_ram_f00650a0 + 0x25c) == piVar3) || (piVar3 == (int *)0x0)) {
      (*_DAT_ram_00014800)(0,0,0xf0063138,0x5fa,_DAT_ram_00014800);
      piVar3 = (int *)0x0;
    }
    piVar4 = *(int **)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 800);
    if (piVar4 == piVar3) {
      iVar1 = (*_DAT_ram_00014c14)(_DAT_ram_00014c14);
      (*_DAT_ram_00014bf0)
                (*(undefined1 *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x346),aiStack_28,
                 _DAT_ram_00014bf0);
      if ((piVar3[4] - piVar3[5] < 0) && ((uint)(param_2 - aiStack_28[0]) < 0x124f81)) {
        iVar2 = *piVar3;
        if (_DAT_ram_f00650a0 + 0x25c == iVar2) {
          iVar2 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
        }
        iVar1 = iVar1 + (param_2 - aiStack_28[0]);
        if (param_3 < 350000) {
          param_3 = 350000;
        }
        iVar5 = param_3 + iVar1 + -5000;
        piVar4[4] = iVar1;
        *(int *)(iVar2 + 0x14) = iVar1 + 5000;
        *(int *)(iVar2 + 0x10) = iVar5;
        piVar4[5] = iVar5 + 5000;
        FUN_ram_f0023a38(piVar4[4]);
        FUN_ram_f0023c3c(piVar4,0);
      }
    }
  }
  return 1;
}

// >>> MOD: wifi/mgmt/cnm.c:0xdf0 <<<
// ===== FUN_ram_f0024024 @ 0xf0024024 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0024024(int param_1)

{
  int iVar1;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063138,0xdf0,_DAT_ram_00014800);
  }
  if (*(char *)(param_1 + 0x4c) != '\0') {
    FUN_ram_f002c974(1,*(undefined4 *)(param_1 + 0x50));
    *(undefined1 *)(param_1 + 0x4c) = 0;
  }
  iVar1 = FUN_ram_f002cfa8(1,*(undefined4 *)(param_1 + 0x54),*(undefined4 *)(param_1 + 0x58),0,1);
  *(int *)(param_1 + 0x50) = iVar1;
  if (iVar1 != -1) {
    *(undefined1 *)(param_1 + 0x4c) = 1;
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0xe1e <<<
// ===== FUN_ram_f0024080 @ 0xf0024080 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0024080(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063138,0xe1e,_DAT_ram_00014800);
  }
  if (*(char *)(param_1 + 0x4d) != '\0') {
    FUN_ram_f002c640(2,*(undefined4 *)(param_1 + 0x5c));
    FUN_ram_f0023798(*(undefined4 *)(param_1 + 0x60));
    *(undefined1 *)(param_1 + 0x4d) = 0;
  }
  iVar1 = FUN_ram_f002c750(2,*(undefined4 *)(param_1 + 100),*(undefined4 *)(param_1 + 0x68),0);
  *(int *)(param_1 + 0x5c) = iVar1;
  if (iVar1 != -1) {
    uVar2 = FUN_ram_f0023778(*(undefined4 *)(param_1 + 100),*(undefined4 *)(param_1 + 0x68));
    *(undefined1 *)(param_1 + 0x4d) = 1;
    *(undefined4 *)(param_1 + 0x60) = uVar2;
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0xb40 <<<
// ===== FUN_ram_f00240f8 @ 0xf00240f8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00240f8(int param_1)

{
  uint uVar1;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063138,0xb40,_DAT_ram_00014800);
  }
  for (uVar1 = 0; uVar1 < *(byte *)(param_1 + 8); uVar1 = uVar1 + 1 & 0xff) {
    if (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + uVar1 + 9) * 4) + 0xc) == 2)
    {
      FUN_ram_f0024024(param_1);
    }
    else if (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + uVar1 + 9) * 4) + 0xc)
             == 4) {
      FUN_ram_f0024080(param_1);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0x989 <<<
// ===== FUN_ram_f0024168 @ 0xf0024168 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0024168(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int local_18 [2];
  
  if (param_2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063138,0x989,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0xc) == 2) {
    (*_DAT_ram_00014bf0)(*(undefined1 *)(param_1 + 0x346),local_18,_DAT_ram_00014bf0);
    iVar1 = (*_DAT_ram_00014c14)(_DAT_ram_00014c14);
    local_18[0] = local_18[0] + 0x546;
    iVar1 = (*(int *)(param_2 + 0x14) + -0x546) - iVar1;
    *(int *)(param_2 + 0x54) = local_18[0];
    *(int *)(param_2 + 0x58) = iVar1;
    if (iVar1 == 0) {
      *(undefined4 *)(param_2 + 0x58) = 1;
    }
    FUN_ram_f0024024(param_2);
  }
  else if (*(int *)(param_1 + 0xc) == 4) {
    (*_DAT_ram_00014bf0)(*(undefined1 *)(param_1 + 0x346),local_18,_DAT_ram_00014bf0);
    iVar1 = (*_DAT_ram_00014c14)(_DAT_ram_00014c14);
    local_18[0] = local_18[0] + 0x546;
    iVar1 = (*(int *)(param_2 + 0x14) + -0x546) - iVar1;
    *(int *)(param_2 + 100) = local_18[0];
    *(int *)(param_2 + 0x68) = iVar1;
    if (iVar1 == 0) {
      *(undefined4 *)(param_2 + 0x68) = 1;
    }
    FUN_ram_f0024080(param_2);
    (*_DAT_ram_0001480c)(0x21,_DAT_ram_0001480c);
    (*_DAT_ram_0001480c)(0x21,_DAT_ram_0001480c);
  }
  else {
    iVar1 = FUN_ram_f00239e8(param_2);
    iVar2 = 0;
    FUN_ram_f002fd54(*(undefined1 *)(param_1 + 0x19));
    FUN_ram_f002be74(*(undefined1 *)(param_1 + 0x19),_DAT_ram_f0065098 - 1U < 2,
                     iVar1 + 0x3ffU >> 10 & 0xffff);
    do {
      (*_DAT_ram_0001480c)(0x21,_DAT_ram_0001480c);
      iVar2 = iVar2 + 1;
    } while (iVar2 != 10);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00242a0 @ 0xf00242a0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_ram_f00242a0(int param_1)

{
  byte bVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  
  if (((*(byte *)(_DAT_ram_f0065118 + 0x10c) & 1) == 0) &&
     (bVar1 = *(byte *)(_DAT_ram_f0065118 + 0x10c) >> 1, uVar5 = bVar1 & 1, (bVar1 & 1) == 0)) {
    uVar4 = 350000;
    for (; uVar5 < *(byte *)(param_1 + 8); uVar5 = uVar5 + 1 & 0xff) {
      iVar2 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + uVar5 + 9) * 4);
      uVar3 = ((uint)*(byte *)(iVar2 + 0x72) * (uint)*(ushort *)(iVar2 + 0x6a) + 0x96) * 0x400;
      if (*(int *)(iVar2 + 0xc) == 2) {
        if (uVar3 < 820000) {
          uVar3 = 820000;
        }
LAB_ram_f0024360:
        if (1200000 < uVar3) {
          return 1200000;
        }
      }
      else {
        if (679999 < uVar3) goto LAB_ram_f0024360;
        iVar2 = FUN_ram_f002b1e4(*(undefined1 *)(iVar2 + 0x19));
        if (iVar2 == 0) {
          uVar3 = 680000;
        }
      }
      if (uVar4 < uVar3) {
        uVar4 = uVar3;
      }
    }
  }
  else if (*(char *)(param_1 + 9) == '\x01') {
    uVar4 = 150000;
  }
  else {
    uVar4 = 30000;
  }
  return uVar4;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0024394 @ 0xf0024394 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0024394(void)

{
  if (*(char *)(_DAT_ram_f00650a0 + 0x268) != '\0') {
    FUN_ram_f004571c(*(undefined4 *)(_DAT_ram_f00650a0 + 0x26c),
                     *(undefined1 *)(_DAT_ram_f00650a0 + 0x270),
                     *(undefined4 *)(_DAT_ram_f00650a0 + 0x274),0);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00243c8 @ 0xf00243c8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00243c8(undefined4 param_1,undefined1 param_2,undefined4 param_3,undefined1 param_4,
                     undefined1 param_5,undefined4 param_6)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = _DAT_ram_f00650a0;
  puVar2 = (undefined4 *)(_DAT_ram_f00650a0 + 0x26c);
  *(undefined1 *)(_DAT_ram_f00650a0 + 0x268) = 1;
  *puVar2 = param_1;
  *(undefined1 *)(iVar1 + 0x270) = param_2;
  *(undefined4 *)(iVar1 + 0x274) = param_3;
  *(undefined1 *)(iVar1 + 0x27c) = param_4;
  *(undefined1 *)(iVar1 + 0x27d) = param_5;
  *(undefined4 *)(iVar1 + 0x278) = param_6;
  if ((*(int *)(_DAT_ram_f00650a0 + 0x264) == 0) && (_DAT_ram_f0065098 == 0)) {
    FUN_ram_f0024394();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0024414 @ 0xf0024414 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0024414(void)

{
  int iVar1;
  
  iVar1 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
  if ((_DAT_ram_f00650a0 + 0x25c != iVar1) && (iVar1 != 0)) {
    FUN_ram_f003af48(iVar1);
    FUN_ram_f0023880(iVar1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002444c @ 0xf002444c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002444c(void)

{
  _DAT_ram_f0065098 = 0;
  _DAT_ram_f006509c = 0;
  _DAT_ram_f00650a0 = _DAT_ram_020a0064 + 0xf8;
  (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_f00650a0,0,0x280);
  *(int *)(_DAT_ram_f00650a0 + 0x48) = _DAT_ram_f00650a0 + 0x48;
  *(int *)(_DAT_ram_f00650a0 + 0x4c) = _DAT_ram_f00650a0 + 0x48;
  *(undefined4 *)(_DAT_ram_f00650a0 + 0x50) = 0;
  *(int *)(_DAT_ram_f00650a0 + 0x25c) = _DAT_ram_f00650a0 + 0x25c;
  *(int *)(_DAT_ram_f00650a0 + 0x260) = _DAT_ram_f00650a0 + 0x25c;
  *(undefined4 *)(_DAT_ram_f00650a0 + 0x264) = 0;
  FUN_ram_f00266a0(_DAT_ram_f00650a0 + 0x58,&LAB_ram_f0024854,0);
  FUN_ram_f00266a0(_DAT_ram_f00650a0 + 0x70,&LAB_ram_f0025170,0);
  FUN_ram_f00266a0(_DAT_ram_f00650a0,&LAB_ram_f0025138,0);
  FUN_ram_f00266a0(_DAT_ram_f00650a0 + 0x18,&LAB_ram_f0025138,1);
  FUN_ram_f00266a0(_DAT_ram_f00650a0 + 0x30,&LAB_ram_f0025138,2);
  (*_DAT_ram_00014988)(0x100000,0,_DAT_ram_00014988);
  _DAT_ram_f0065068 = 0;
  _DAT_ram_f006506a = 0;
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006d7ac,0,0x20);
  DAT_ram_f0065004 = 0;
  (*(code *)&SUB_ram_0006a0a0)(0xf006d7b8,0x3f,0xe);
  (*(code *)&SUB_ram_0006a0a0)(0xf006d7c6,0x3f,4);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002460c @ 0xf002460c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002460c(undefined4 param_1)

{
  int iVar1;
  int *piVar2;
  
  (*(code *)&SUB_ram_0006a08c)(_DAT_ram_020a0064 + 0x2088,param_1,6);
  (*(code *)&SUB_ram_0006a08c)(_DAT_ram_020a0064 + 0x208e,_DAT_ram_020a0064 + 0x2088,6);
  piVar2 = (int *)&DAT_ram_020a0068;
  *(byte *)(_DAT_ram_020a0064 + 0x208e) = *(byte *)(_DAT_ram_020a0064 + 0x208e) ^ 2;
  (*(code *)&SUB_ram_0006a08c)(_DAT_ram_020a0064 + 0x2094,_DAT_ram_020a0064 + 0x208e,6);
  (*(code *)&SUB_ram_0006a08c)(_DAT_ram_020a0068 + 0x51,_DAT_ram_020a0064 + 0x2088,6);
  (*(code *)&SUB_ram_0006a08c)(_DAT_ram_020a006c + 0x51,_DAT_ram_020a0064 + 0x2094,6);
  (*(code *)&SUB_ram_0006a08c)(_DAT_ram_020a0070 + 0x51,_DAT_ram_020a0064 + 0x2094,6);
  do {
    iVar1 = *piVar2;
    piVar2 = piVar2 + 1;
    if (*(char *)(iVar1 + 0x18) != '\0') {
      (*_DAT_ram_00014dac)(*(undefined1 *)(iVar1 + 0x345),1,iVar1 + 0x51,_DAT_ram_00014dac);
    }
  } while (piVar2 != (int *)&DAT_ram_020a0074);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00247b8 @ 0xf00247b8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00247b8(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined1 uVar2;
  undefined2 *puVar3;
  
  iVar1 = FUN_ram_f0025c48(0x14);
  if (iVar1 == 0) {
    FUN_ram_f002672c(_DAT_ram_f00650a0 + 0x58,10);
  }
  else {
    puVar3 = *(undefined2 **)(iVar1 + 8);
    *(undefined1 *)(puVar3 + 2) = 0x18;
    *puVar3 = 0x14;
    *(undefined1 *)((int)puVar3 + 5) = 0;
    *(undefined1 *)(puVar3 + 4) = *(undefined1 *)(param_1 + 0xc);
    uVar2 = *(undefined1 *)(param_1 + 0xd);
    *(undefined1 *)(puVar3 + 5) = 0;
    *(undefined1 *)((int)puVar3 + 9) = uVar2;
    *(undefined1 *)((int)puVar3 + 0xb) = *(undefined1 *)(param_1 + 0xe);
    *(undefined1 *)(puVar3 + 6) = *(undefined1 *)(param_1 + 0x10);
    if (*(int *)(param_1 + 0x14) == 5000000) {
      uVar2 = 2;
    }
    else {
      uVar2 = 1;
    }
    *(undefined1 *)((int)puVar3 + 0xd) = uVar2;
    uVar2 = *(undefined1 *)(param_1 + 0x18);
    *(undefined4 *)(puVar3 + 8) = param_2;
    *(undefined1 *)(puVar3 + 7) = uVar2;
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar3;
    puVar3[1] = 1;
    FUN_ram_f0039750();
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0x4cd,0x515,0x518,0xbd0,0xbe8,0xc23,0xc61,0xcbf <<<
// ===== FUN_ram_f00248a4 @ 0xf00248a4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00248a4(int param_1)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  byte *pbVar4;
  undefined4 uVar5;
  int iVar6;
  int *piVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  uint uVar10;
  uint uVar11;
  uint auStack_24 [2];
  
  do {
    _DAT_ram_f006509c = _DAT_ram_f0065098;
    _DAT_ram_f0065098 = param_1;
    if (param_1 == 1) {
      *(undefined1 *)(_DAT_ram_f00650a0 + 0x9c) = 0;
      for (puVar9 = *(undefined4 **)(_DAT_ram_f00650a0 + 0x25c);
          puVar9 != (undefined4 *)(_DAT_ram_f00650a0 + 0x25c); puVar9 = (undefined4 *)*puVar9) {
        if (puVar9 == (undefined4 *)0x0) {
          (*_DAT_ram_00014800)(0,0,0xf0063138,0xbd0,_DAT_ram_00014800);
        }
        *(undefined1 *)((int)puVar9 + 0xd) = 0;
      }
      if (*(uint *)(_DAT_ram_f00650a0 + 0x264) == 0) {
        param_1 = 2;
      }
      else if (*(uint *)(_DAT_ram_f00650a0 + 0x264) < 2) {
        puVar8 = *(undefined4 **)(_DAT_ram_f00650a0 + 0x25c);
        if ((puVar8 == puVar9) || (puVar8 == (undefined4 *)0x0)) {
          (*_DAT_ram_00014800)(0,0,0xf0063138,0xbe8,_DAT_ram_00014800);
          puVar8 = (undefined4 *)0x0;
        }
        uVar11 = 0;
        uVar10 = 0;
        while( true ) {
          bVar2 = *(byte *)(puVar8 + 2);
          if (bVar2 <= uVar11) break;
          uVar10 = uVar10 * (14999 < uVar10) | (uint)(uVar10 < 15000) * 15000;
          iVar3 = *(int *)(*(int *)(&DAT_ram_020a0068 +
                                   (uint)*(byte *)((int)puVar8 + uVar11 + 9) * 4) + 0xc);
          if ((iVar3 == 2) || (iVar3 == 4)) {
            uVar10 = 0x4b000;
          }
          else if ((bVar2 == 1) && (*(int *)(*(int *)(_DAT_ram_f00650a0 + 0x54) + 8) == 7)) {
            *(byte *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)((int)puVar8 + uVar11 + 9) * 4) +
                     0x370) = bVar2;
            uVar10 = 0xdac;
            (*_DAT_ram_00014830)(_DAT_ram_00014830);
          }
          uVar11 = uVar11 + 1 & 0xff;
        }
        if (30000000 < *(uint *)(_DAT_ram_f00650a0 + 0x98)) {
          (*_DAT_ram_00014800)(0,0,0xf0063138,0xc23,_DAT_ram_00014800);
        }
        iVar3 = (*_DAT_ram_00014c14)(_DAT_ram_00014c14);
        iVar3 = uVar10 + iVar3;
        puVar8[4] = iVar3;
        puVar8[5] = iVar3 + *(int *)(_DAT_ram_f00650a0 + 0x98);
        FUN_ram_f0023a38(iVar3);
        *(undefined1 *)((int)puVar8 + 0xd) = 1;
        *(undefined1 *)(puVar8 + 3) = 0;
        *(undefined1 *)(_DAT_ram_f00650a0 + 0x9c) = 1;
        FUN_ram_f0023930(puVar8);
        FUN_ram_f00240f8(puVar8);
        uVar5 = 0;
LAB_ram_f0024cd8:
        FUN_ram_f0023c3c(puVar8,uVar5);
      }
    }
    else if (param_1 == 0) {
      iVar3 = FUN_ram_f002379c();
      if ((iVar3 != 0) && (*(int *)(_DAT_ram_f00650a0 + 0x70) == 0)) {
        *(int *)(_DAT_ram_f00650a0 + 0x54) = iVar3;
        iVar6 = *(int *)(iVar3 + 8);
        if ((iVar6 == 0) || (iVar6 == 7)) {
          iVar3 = *(int *)(iVar3 + 0x1c);
        }
        else {
          if (iVar6 != 2) {
            (*_DAT_ram_00014800)(0,0,0xf0063138,0xcbf,_DAT_ram_00014800);
          }
          iVar3 = *(int *)(iVar3 + 0x10);
        }
        *(int *)(_DAT_ram_f00650a0 + 0x98) = iVar3 * 1000 + 30000;
        param_1 = 1;
      }
    }
    else if (param_1 == 2) {
      if (*(int *)(_DAT_ram_f00650a0 + 0x54) == 0) {
        (*_DAT_ram_00014800)(0,0,0xf0063138,0xc61,_DAT_ram_00014800);
      }
      iVar6 = *(int *)(_DAT_ram_f00650a0 + 0x54);
      piVar7 = (int *)(iVar6 + 8);
      FUN_ram_f003e5f8();
      iVar3 = *piVar7;
      if ((iVar3 == 0) || (iVar3 == 7)) {
        if (2 < *(byte *)(iVar6 + 0xc)) {
          (*_DAT_ram_00014800)(0,0,0xf0063138,0x4cd,_DAT_ram_00014800);
        }
        DAT_ram_f0065004 =
             FUN_ram_f0023b48(*(undefined1 *)(iVar6 + 0xc),*(undefined4 *)(iVar6 + 0x14));
        FUN_ram_f004571c(*(undefined4 *)(iVar6 + 0x14),*(undefined1 *)(iVar6 + 0xe),
                         *(undefined4 *)(iVar6 + 0x10),0);
        FUN_ram_f00452b4(*(undefined4 *)(iVar6 + 0x14),*(undefined1 *)(iVar6 + 0xe),
                         *(undefined4 *)(iVar6 + 0x10),0);
        if ((*(char *)(iVar6 + 0xc) == '\x01') && (DAT_ram_f0065058 != '\0')) {
          auStack_24[0] = 0;
          (*_DAT_ram_00014a90)(auStack_24,_DAT_ram_00014a90);
          auStack_24[0] = auStack_24[0] & 0xffffffef;
          (*_DAT_ram_00014af0)(auStack_24[0],_DAT_ram_00014af0);
        }
        if (*piVar7 == 7) {
          FUN_ram_f003f6a4(iVar6,0,*(undefined4 *)(iVar6 + 0x1c));
        }
        else {
          FUN_ram_f00247b8(iVar6,*(undefined4 *)(iVar6 + 0x1c));
        }
        *(undefined1 *)(_DAT_ram_f00650a0 + 0x89) = *(undefined1 *)piVar7;
        *(undefined4 *)(_DAT_ram_f00650a0 + 0x94) = *(undefined4 *)(iVar6 + 0x1c);
        *(undefined1 *)(_DAT_ram_f00650a0 + 0x8a) = *(undefined1 *)(iVar6 + 0xe);
        *(undefined4 *)(_DAT_ram_f00650a0 + 0x90) = *(undefined4 *)(iVar6 + 0x14);
        *(undefined4 *)(_DAT_ram_f00650a0 + 0x8c) = *(undefined4 *)(iVar6 + 0x10);
      }
      else {
        if (iVar3 != 2) {
          (*_DAT_ram_00014800)(0,0,0xf0063138,0x515,_DAT_ram_00014800);
        }
        if (2 < *(byte *)(iVar6 + 0xc)) {
          (*_DAT_ram_00014800)(0,0,0xf0063138,0x518,_DAT_ram_00014800);
        }
        iVar3 = FUN_ram_f0025b48(0,0x14);
        if (iVar3 == 0) {
          uVar5 = 0x521;
          goto LAB_ram_f0024d6e;
        }
        DAT_ram_f0065004 = 0;
        *(undefined4 *)(iVar3 + 8) = 4;
        *(undefined1 *)(iVar3 + 0xc) = *(undefined1 *)(iVar6 + 0xc);
        *(undefined1 *)(iVar3 + 0xd) = *(undefined1 *)(iVar6 + 0xd);
        *(undefined4 *)(iVar3 + 0x10) = *(undefined4 *)(iVar6 + 0x10);
        FUN_ram_f0028344(0,iVar3,0);
        *(undefined1 *)(_DAT_ram_f00650a0 + 0x89) = *(undefined1 *)piVar7;
        *(undefined4 *)(_DAT_ram_f00650a0 + 0x94) = *(undefined4 *)(iVar6 + 0x10);
      }
      (*_DAT_ram_00014e1c)(0,_DAT_ram_00014e1c);
      (*_DAT_ram_00014e24)(0,_DAT_ram_00014e24);
      (*_DAT_ram_00014e20)(0,_DAT_ram_00014e20);
      (*_DAT_ram_00014e18)(0,_DAT_ram_00014e18);
      (*_DAT_ram_00014958)(0,_DAT_ram_00014958);
      FUN_ram_f003aba0(0);
      FUN_ram_f0020c1c(0);
    }
    else if (param_1 == 3) {
      puVar8 = *(undefined4 **)(_DAT_ram_f00650a0 + 0x25c);
      if (((undefined4 *)(_DAT_ram_f00650a0 + 0x25c) != puVar8) && (puVar8 != (undefined4 *)0x0)) {
        FUN_ram_f0024414();
        pbVar4 = (byte *)((int)puVar8 + 9);
        bVar2 = 0;
        do {
          if (*(byte *)(puVar8 + 2) <= bVar2) goto LAB_ram_f0024caa;
          bVar1 = *pbVar4;
          pbVar4 = pbVar4 + 1;
        } while ((*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)bVar1 * 4) + 0xc) != 2) &&
                (bVar2 = bVar2 + 1,
                *(int *)(*(int *)(&DAT_ram_020a0068 + (uint)bVar1 * 4) + 0xc) != 4));
        if (*(char *)(puVar8 + 3) == '\0') {
LAB_ram_f0024caa:
          *(undefined1 *)(puVar8 + 3) = 0;
          if (*(uint *)(_DAT_ram_f00650a0 + 0x264) < 2) {
            (*_DAT_ram_00014bcc)(0,_DAT_ram_00014bcc);
            _DAT_ram_60310000 = 0x100000;
          }
          param_1 = 0;
        }
        uVar5 = 1;
        goto LAB_ram_f0024cd8;
      }
      param_1 = 0;
    }
    else {
      uVar5 = 0xcf2;
LAB_ram_f0024d6e:
      (*_DAT_ram_00014800)(0,0,0xf0063138,uVar5,_DAT_ram_00014800);
    }
    if (param_1 == _DAT_ram_f0065098) {
      return;
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/cnm.c:0x564,0x573 <<<
// ===== FUN_ram_f0024fc0 @ 0xf0024fc0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0024fc0(int *param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  if (param_1 != (int *)0x0) {
    if (2 < *(byte *)(param_1 + 3)) {
      (*_DAT_ram_00014800)(0,0,0xf0063138,0x564,_DAT_ram_00014800);
    }
    piVar1 = *(int **)(_DAT_ram_f00650a0 + 0x4c);
    iVar3 = _DAT_ram_f00650a0 + 0x48;
    *(int **)(_DAT_ram_f00650a0 + 0x4c) = param_1;
    *param_1 = iVar3;
    param_1[1] = (int)piVar1;
    *piVar1 = (int)param_1;
    *(int *)(_DAT_ram_f00650a0 + 0x50) = *(int *)(_DAT_ram_f00650a0 + 0x50) + 1;
  }
  iVar3 = FUN_ram_f002379c();
  if ((iVar3 != 0) && (_DAT_ram_f0065098 == 0)) {
    if (*(int *)(_DAT_ram_f00650a0 + 0x70) == 0) {
      *(int *)(_DAT_ram_f00650a0 + 0x54) = iVar3;
      iVar2 = *(int *)(iVar3 + 8);
      if ((iVar2 == 0) || (iVar2 == 7)) {
        iVar3 = *(int *)(iVar3 + 0x1c);
      }
      else {
        if (iVar2 != 2) {
          (*_DAT_ram_00014800)(0,0,0xf0063138,0x573,_DAT_ram_00014800);
        }
        iVar3 = *(int *)(iVar3 + 0x10);
      }
      *(int *)(_DAT_ram_f00650a0 + 0x98) = iVar3 * 1000 + 30000;
      FUN_ram_f00248a4(1);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025090 @ 0xf0025090 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0025090(int param_1,char param_2)

{
  undefined4 uVar1;
  
  if (param_2 == '\0') {
    if (*(char *)(param_1 + 0x19) == *(char *)(_DAT_ram_f00650a0 + 0x88)) {
      FUN_ram_f00266d8(_DAT_ram_f00650a0 + 0x70);
      FUN_ram_f0024fc0(0);
      FUN_ram_f0020b28(0);
    }
  }
  else {
    FUN_ram_f0020b28(1);
    if (DAT_ram_f0064fd5 == '\0') {
      uVar1 = 0x834;
      if (DAT_ram_f0064fa9 == '\0') {
        uVar1 = 0x46;
      }
    }
    else {
      uVar1 = 0x834;
    }
    FUN_ram_f002672c(_DAT_ram_f00650a0 + 0x70,uVar1);
    *(undefined1 *)(_DAT_ram_f00650a0 + 0x88) = *(undefined1 *)(param_1 + 0x19);
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm.c:0x6eb,0x700,0x711,0x734,0x816,0x81f,0x855,0x856,0x87a,0x887,0x8f3,0x932 <<<
// ===== FUN_ram_f0025198 @ 0xf0025198 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0025198(int param_1,char param_2)

{
  byte bVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  byte *pbVar7;
  char *pcVar8;
  int *piVar9;
  int *unaff_s0;
  int iVar10;
  char cVar11;
  uint uVar12;
  int iVar13;
  int *piVar14;
  
  iVar3 = _DAT_ram_f00650a0;
  piVar14 = (int *)(_DAT_ram_f00650a0 + 0x25c);
  for (piVar9 = *(int **)(_DAT_ram_f00650a0 + 0x25c); piVar9 != piVar14; piVar9 = (int *)*piVar9) {
    if (piVar9 == (int *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf0063138,0x6eb,&DAT_ram_00014800,_DAT_ram_00014800);
    }
    pcVar8 = (char *)((int)piVar9 + 9);
    iVar10 = 0;
    while (unaff_s0 = piVar9, iVar10 < (int)(uint)*(byte *)(piVar9 + 2)) {
      cVar11 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      iVar10 = iVar10 + 1;
      if (*(char *)(param_1 + 0x19) == cVar11) {
        bVar2 = true;
        goto LAB_ram_f0025206;
      }
    }
  }
  bVar2 = false;
  piVar14 = piVar9;
LAB_ram_f0025206:
  if (param_2 == '\0') {
    if (bVar2) {
      if ((char)unaff_s0[2] == '\0') {
        (*_DAT_ram_00014800)(0,0,0xf0063138,0x855,_DAT_ram_00014800);
      }
      if (*(int **)(param_1 + 800) != unaff_s0) {
        (*_DAT_ram_00014800)(0,0,0xf0063138,0x856,_DAT_ram_00014800);
      }
      if ((*(char *)(param_1 + 0x19) == '\x01') && ((char)unaff_s0[0x13] != '\0')) {
        FUN_ram_f002c974(1,unaff_s0[0x14]);
        *(undefined1 *)(unaff_s0 + 0x13) = 0;
      }
      if ((*(char *)(param_1 + 0x19) == '\x02') && (*(char *)((int)unaff_s0 + 0x4d) != '\0')) {
        FUN_ram_f002c640(2,unaff_s0[0x17]);
        FUN_ram_f0023798(unaff_s0[0x18]);
        *(undefined1 *)((int)unaff_s0 + 0x4d) = 0;
      }
      FUN_ram_f00266d8(param_1 + 0x178);
      if (3 < *(byte *)(unaff_s0 + 2)) {
        (*_DAT_ram_00014800)(0,0,0xf0063138,0x87a,_DAT_ram_00014800);
      }
      bVar1 = *(byte *)(unaff_s0 + 2);
      pcVar8 = (char *)((int)unaff_s0 + 9);
      uVar12 = 0;
LAB_ram_f0025668:
      if (uVar12 < bVar1) {
        cVar11 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        if (*(char *)(param_1 + 0x19) != cVar11) goto LAB_ram_f0025666;
        while( true ) {
          if ((int)(bVar1 - 1) <= (int)uVar12) break;
          *(undefined1 *)((int)unaff_s0 + uVar12 + 9) = *(undefined1 *)((int)unaff_s0 + uVar12 + 10)
          ;
          uVar12 = uVar12 + 1 & 0xff;
        }
      }
      if (*(byte *)(unaff_s0 + 2) <= uVar12) {
        (*_DAT_ram_00014800)(0,0,0xf0063138,0x887,_DAT_ram_00014800);
      }
      cVar11 = (char)unaff_s0[2] + -1;
      *(char *)(unaff_s0 + 2) = cVar11;
      iVar3 = _DAT_ram_f00650a0;
      if (cVar11 == '\0') {
        piVar9 = (int *)unaff_s0[1];
        iVar10 = *unaff_s0;
        piVar14 = *(int **)(_DAT_ram_f00650a0 + 0x25c);
        *(int **)(iVar10 + 4) = piVar9;
        *piVar9 = iVar10;
        *unaff_s0 = 0;
        unaff_s0[1] = 0;
        *(int *)(_DAT_ram_f00650a0 + 0x264) = *(int *)(_DAT_ram_f00650a0 + 0x264) + -1;
        if (piVar14 == (int *)(iVar3 + 0x25c)) {
          piVar14 = (int *)0x0;
        }
        FUN_ram_f00266d8(unaff_s0 + 7);
        FUN_ram_f00266d8(unaff_s0 + 0xd);
        if (unaff_s0 == piVar14) {
          *(undefined1 *)(_DAT_ram_f00650a0 + 0x9d) = 0;
          iVar3 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
          if ((iVar3 == _DAT_ram_f00650a0 + 0x25c) || (iVar3 == 0)) {
            (*_DAT_ram_00014bcc)(0,_DAT_ram_00014bcc);
            _DAT_ram_60310000 = 0x100000;
          }
          else {
            FUN_ram_f0023a38(*(undefined4 *)(iVar3 + 0x14));
            if ((_DAT_ram_f0065098 < 2) && (FUN_ram_f0024414(), _DAT_ram_f0065098 == 0)) {
              FUN_ram_f0023c3c(iVar3,1);
            }
          }
          if (_DAT_ram_f0065098 == 3) {
            FUN_ram_f00248a4();
            goto LAB_ram_f00257fc;
          }
        }
        else if ((_DAT_ram_f0065098 == 0) && (*(int *)(_DAT_ram_f00650a0 + 0x264) == 1)) {
          (*_DAT_ram_00014bcc)(0,_DAT_ram_00014bcc);
          _DAT_ram_60310000 = 0x100000;
          FUN_ram_f0023c3c(piVar14,1);
        }
        if (_DAT_ram_f0065098 == 1) {
          if (*(char *)((int)unaff_s0 + 0xd) != '\0') {
            *(char *)(_DAT_ram_f00650a0 + 0x9c) = *(char *)(_DAT_ram_f00650a0 + 0x9c) + -1;
          }
          if (*(uint *)(_DAT_ram_f00650a0 + 0x264) <= (uint)*(byte *)(_DAT_ram_f00650a0 + 0x9c)) {
            if (*(int *)(_DAT_ram_f00650a0 + 0x54) == 0) {
              uVar5 = 3;
            }
            else {
              uVar5 = 2;
            }
            FUN_ram_f00248a4(uVar5,FUN_ram_f00248a4);
          }
        }
      }
LAB_ram_f00257fc:
      *(undefined4 *)(param_1 + 800) = 0;
      goto LAB_ram_f0025802;
    }
  }
  else {
    if (!bVar2) {
      for (unaff_s0 = *(int **)(iVar3 + 0x25c); unaff_s0 != piVar14; unaff_s0 = (int *)*unaff_s0) {
        if (unaff_s0 == (int *)0x0) {
          (*_DAT_ram_00014800)(0,0,0xf0063138,0x700,_DAT_ram_00014800);
        }
        if ((unaff_s0[0x1b] == *(int *)(param_1 + 0x2c8)) &&
           ((char)unaff_s0[0x1c] == *(char *)(param_1 + 0x2cc))) {
          if (2 < *(byte *)(unaff_s0 + 2)) {
            (*_DAT_ram_00014800)(0,0,0xf0063138,0x711,_DAT_ram_00014800);
          }
          *(undefined1 *)((int)unaff_s0 + *(byte *)(unaff_s0 + 2) + 9) =
               *(undefined1 *)(param_1 + 0x19);
          *(char *)(unaff_s0 + 2) = (char)unaff_s0[2] + '\x01';
          if ((_DAT_ram_f0065098 == 2) &&
             (((int *)(_DAT_ram_f00650a0 + 0x25c) == *(int **)(_DAT_ram_f00650a0 + 0x25c) ||
              (unaff_s0 != *(int **)(_DAT_ram_f00650a0 + 0x25c))))) {
            FUN_ram_f0024168(param_1,unaff_s0);
          }
          goto LAB_ram_f0025568;
        }
      }
      if (2 < *(uint *)(_DAT_ram_f00650a0 + 0x264)) {
        (*_DAT_ram_00014800)(0,0,0xf0063138,0x816,_DAT_ram_00014800);
      }
      unaff_s0 = (int *)(_DAT_ram_f00650a0 + 0xa0);
      if (((*(char *)(_DAT_ram_f00650a0 + 0xa8) != '\0') &&
          (unaff_s0 = (int *)(_DAT_ram_f00650a0 + 0x134),
          *(char *)(_DAT_ram_f00650a0 + 0x13c) != '\0')) &&
         (unaff_s0 = (int *)(_DAT_ram_f00650a0 + 0x1c8),
         *(char *)(_DAT_ram_f00650a0 + 0x1d0) != '\0')) {
        (*_DAT_ram_00014800)(0,0,0xf0063138,0x81f,_DAT_ram_00014800);
      }
      (*(code *)&SUB_ram_0006a0a0)(unaff_s0,0,0x94);
      FUN_ram_f00266a0(unaff_s0 + 7,&LAB_ram_f0023f58,unaff_s0);
      FUN_ram_f00266a0(unaff_s0 + 0xd,&LAB_ram_f0023908,unaff_s0);
      *(undefined1 *)((int)unaff_s0 + 9) = *(undefined1 *)(param_1 + 0x19);
      *(undefined1 *)(unaff_s0 + 2) = 1;
      unaff_s0[0x1b] = *(int *)(param_1 + 0x2c8);
      *(undefined1 *)(unaff_s0 + 0x1c) = *(undefined1 *)(param_1 + 0x2cc);
      iVar3 = FUN_ram_f00242a0(unaff_s0);
      unaff_s0[6] = iVar3;
      if (_DAT_ram_f0065098 == 2) {
        iVar3 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
        if ((iVar3 != _DAT_ram_f00650a0 + 0x25c) &&
           (iVar10 = *(int *)(_DAT_ram_f00650a0 + 0x260), iVar3 != 0)) {
          if (iVar10 == 0) {
            (*_DAT_ram_00014800)(0,0,0xf0063138,0x8f3,_DAT_ram_00014800);
          }
          unaff_s0[4] = *(int *)(iVar3 + 0x14) + -5000;
          unaff_s0[5] = *(int *)(iVar10 + 0x10) + 5000;
          FUN_ram_f0023a38();
          FUN_ram_f0023930(unaff_s0);
          if (*(int *)(param_1 + 0xc) == 2) {
            FUN_ram_f0024024(unaff_s0);
          }
          else if (*(int *)(param_1 + 0xc) == 4) {
            FUN_ram_f0024080(unaff_s0);
          }
          else {
            FUN_ram_f0023c3c(unaff_s0,0);
          }
        }
        iVar3 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
        puVar6 = (undefined4 *)(_DAT_ram_f00650a0 + 0x25c);
      }
      else {
        iVar3 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
        if ((iVar3 != _DAT_ram_f00650a0 + 0x25c) &&
           (iVar10 = *(int *)(_DAT_ram_f00650a0 + 0x260), iVar3 != 0)) {
          if (iVar10 == 0) {
            (*_DAT_ram_00014800)(0,0,0xf0063138,0x932,_DAT_ram_00014800);
          }
          iVar13 = param_1;
          if ((*(int *)(_DAT_ram_f00650a0 + 0x264) == 1) && (_DAT_ram_f0065098 == 0)) {
            pbVar7 = (byte *)(iVar3 + 9);
            uVar12 = 0;
            for (iVar4 = 0; iVar4 < (int)(uint)*(byte *)(iVar3 + 8); iVar4 = iVar4 + 1) {
              iVar13 = *(int *)(&DAT_ram_020a0068 + (uint)*pbVar7 * 4);
              uVar12 = uVar12 * (14999 < uVar12) | (uint)(uVar12 < 15000) * 15000;
              if ((*(int *)(iVar13 + 0xc) == 2) || (*(int *)(iVar13 + 0xc) == 4)) {
                uVar12 = 0x4b000;
              }
              pbVar7 = pbVar7 + 1;
            }
            iVar4 = (*_DAT_ram_00014c14)(_DAT_ram_00014c14);
            *(uint *)(iVar3 + 0x10) = iVar4 + uVar12;
            FUN_ram_f0023a38();
          }
          iVar4 = *(int *)(iVar10 + 0x10) + 5000;
          iVar10 = iVar4 + unaff_s0[6];
          unaff_s0[4] = iVar10;
          unaff_s0[5] = iVar4;
          *(int *)(iVar3 + 0x14) = iVar10 + 5000;
          *(undefined1 *)(unaff_s0 + 3) = 1;
          FUN_ram_f0024168(iVar13,unaff_s0);
          if (*(int *)(_DAT_ram_f00650a0 + 0x264) != 0) {
            FUN_ram_f0023930(iVar3);
            FUN_ram_f00240f8(iVar3);
          }
        }
        puVar6 = *(undefined4 **)(_DAT_ram_f00650a0 + 0x260);
        iVar3 = _DAT_ram_f00650a0 + 0x25c;
      }
      *(int **)(iVar3 + 4) = unaff_s0;
      *unaff_s0 = iVar3;
      unaff_s0[1] = (int)puVar6;
      *puVar6 = unaff_s0;
      *(int *)(_DAT_ram_f00650a0 + 0x264) = *(int *)(_DAT_ram_f00650a0 + 0x264) + 1;
LAB_ram_f0025568:
      *(int **)(param_1 + 800) = unaff_s0;
LAB_ram_f0025802:
      FUN_ram_f0023bdc(unaff_s0);
    }
    if ((unaff_s0 == (int *)0x0) || (3 < *(byte *)(unaff_s0 + 2))) {
      (*_DAT_ram_00014800)(0,0,0xf0063138,0x734,_DAT_ram_00014800);
    }
    piVar14 = *(int **)(_DAT_ram_f00650a0 + 0x25c);
    if (((((int *)(_DAT_ram_f00650a0 + 0x25c) != piVar14) && (piVar14 != (int *)0x0)) &&
        (unaff_s0 == piVar14)) && (_DAT_ram_f0065098 == 0)) {
      FUN_ram_f003af48(unaff_s0);
      FUN_ram_f0023880(unaff_s0);
    }
  }
  return;
LAB_ram_f0025666:
  uVar12 = uVar12 + 1 & 0xff;
  goto LAB_ram_f0025668;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025a70 @ 0xf0025a70 =====


/* WARNING: Removing unreachable block (ram,0xf0025aa0) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0025a70(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  
  _DAT_ram_f0065600 = 0;
  _DAT_ram_f00655f8 = &DAT_ram_f00655f8;
  _DAT_ram_f00655fc = (undefined4 *)&DAT_ram_f00655f8;
  iVar3 = 0x202318;
  puVar2 = (undefined4 *)&DAT_ram_f0065608;
  do {
    puVar2[2] = iVar3;
    puVar1 = _DAT_ram_f00655fc;
    iVar3 = iVar3 + 0x680;
    _DAT_ram_f00655fc = puVar2;
    *puVar2 = &DAT_ram_f00655f8;
    puVar2[1] = puVar1;
    *puVar1 = puVar2;
    puVar2 = puVar2 + 0x1c;
    _DAT_ram_f0065600 = _DAT_ram_f0065600 + 1;
  } while (iVar3 != 0x214798);
  _DAT_ram_f00669b8 = &DAT_ram_f00669b8;
  _DAT_ram_f00669bc = (undefined4 *)&DAT_ram_f00669b8;
  _DAT_ram_f00669c0 = 0;
  puVar2 = (undefined4 *)&DAT_ram_f00669c8;
  do {
    puVar2[2] = 0;
    puVar1 = _DAT_ram_f00669bc;
    _DAT_ram_f00669bc = puVar2;
    *puVar2 = &DAT_ram_f00669b8;
    puVar2[1] = puVar1;
    *puVar1 = puVar2;
    puVar2 = puVar2 + 0x1c;
    _DAT_ram_f00669c0 = _DAT_ram_f00669c0 + 1;
  } while (puVar2 != (undefined4 *)0xf00677c8);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025b14 @ 0xf0025b14 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * FUN_ram_f0025b14(void)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  
  piVar1 = _DAT_ram_f00655f8;
  if (_DAT_ram_f00655f8 == (int *)&DAT_ram_f00655f8) {
    piVar1 = (int *)0x0;
  }
  else {
    piVar2 = (int *)_DAT_ram_f00655f8[1];
    iVar3 = *_DAT_ram_f00655f8;
    *(int **)(iVar3 + 4) = piVar2;
    *piVar2 = iVar3;
    piVar1[1] = 0;
    *piVar1 = 0;
    _DAT_ram_f0065600 = _DAT_ram_f0065600 + -1;
  }
  return piVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025b3c @ 0xf0025b3c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0025b3c(void)

{
  return _DAT_ram_f00669c0;
}

// >>> MOD: wifi/mgmt/cnm_mem.c:0x407,0x408,0x411,0x41a <<<
// ===== FUN_ram_f0025b48 @ 0xf0025b48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f0025b48(int param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  uint uVar5;
  
  if (param_1 == 1) {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x407,_DAT_ram_00014800);
  }
  if (param_2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x408,_DAT_ram_00014800);
  }
  if (param_1 == 0) {
    uVar5 = param_2 + 0x1fU >> 5;
    if (0x20 < uVar5) {
      (*_DAT_ram_00014800)(0,0,0xf0063148,0x411,_DAT_ram_00014800);
    }
    piVar3 = (int *)&DAT_ram_f00655a8;
    iVar4 = 5;
  }
  else {
    uVar5 = param_2 + 0xffU >> 8;
    if (0x20 < uVar5) {
      (*_DAT_ram_00014800)(0,0,0xf0063148,0x41a,_DAT_ram_00014800);
    }
    piVar3 = (int *)&DAT_ram_f00655d0;
    iVar4 = 8;
  }
  if (uVar5 - 1 < 0x20) {
    uVar2 = 1 << (uVar5 - 1 & 0x1f);
    uVar2 = uVar2 - 1 | uVar2;
    for (uVar1 = 0; uVar1 <= 0x20 - uVar5; uVar1 = uVar1 + 1) {
      if ((uVar2 & piVar3[1]) == uVar2) {
        piVar3[1] = ~uVar2 & piVar3[1];
        *(char *)((int)piVar3 + uVar1 + 8) = (char)uVar5;
        return *piVar3 + (uVar1 << iVar4);
      }
      uVar2 = uVar2 * 2;
    }
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025c48 @ 0xf0025c48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * FUN_ram_f0025c48(undefined4 param_1)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  
  piVar3 = _DAT_ram_f00669b8;
  if (_DAT_ram_f00669b8 == (int *)&DAT_ram_f00669b8) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar1 = (int *)_DAT_ram_f00669b8[1];
    iVar2 = *_DAT_ram_f00669b8;
    *(int **)(iVar2 + 4) = piVar1;
    *piVar1 = iVar2;
    *piVar3 = 0;
    piVar3[1] = 0;
    _DAT_ram_f00669c0 = _DAT_ram_f00669c0 + -1;
    if (piVar3 != (int *)0x0) {
      iVar2 = FUN_ram_f0025b48(2,param_1);
      *(undefined1 *)(piVar3 + 3) = 3;
      *(undefined2 *)((int)piVar3 + 0x26) = 0;
      piVar3[2] = iVar2;
      piVar1 = _DAT_ram_f00669bc;
      if (iVar2 == 0) {
        _DAT_ram_f00669bc = piVar3;
        *piVar3 = (int)&DAT_ram_f00669b8;
        piVar3[1] = (int)piVar1;
        *piVar1 = (int)piVar3;
        _DAT_ram_f00669c0 = _DAT_ram_f00669c0 + 1;
        piVar3 = (int *)0x0;
      }
    }
  }
  return piVar3;
}

// >>> MOD: wifi/mgmt/cnm_mem.c:0x457,0x471 <<<
// ===== FUN_ram_f0025cac @ 0xf0025cac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0025cac(uint param_1)

{
  byte bVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined1 *puVar4;
  uint uVar5;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x457,_DAT_ram_00014800);
    return;
  }
  if ((param_1 < 0xf00651a8) || (0xf00655a7 < param_1)) {
    puVar4 = &DAT_ram_f00655d0;
    uVar5 = param_1 - _DAT_ram_f00655d0 >> 8;
    if (uVar5 < 0x20) goto LAB_ram_f0025d42;
    uVar3 = 0x469;
  }
  else {
    puVar4 = &DAT_ram_f00655a8;
    uVar5 = param_1 - _DAT_ram_f00655a8 >> 5;
    if (uVar5 < 0x20) goto LAB_ram_f0025d42;
    uVar3 = 0x463;
  }
  (*_DAT_ram_00014800)(0,0,0xf0063148,uVar3,_DAT_ram_00014800);
LAB_ram_f0025d42:
  if (puVar4[uVar5 + 8] == '\0') {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x471,_DAT_ram_00014800);
  }
  bVar1 = puVar4[uVar5 + 8];
  puVar4[uVar5 + 8] = 0;
  uVar2 = 1 << (bVar1 - 1 & 0x1f);
  *(uint *)(puVar4 + 4) = *(uint *)(puVar4 + 4) | (uVar2 - 1 | uVar2) << (uVar5 & 0x1f);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025d90 @ 0xf0025d90 =====


int FUN_ram_f0025d90(byte param_1)

{
  int iVar1;
  
  if (param_1 < 0x14) {
    iVar1 = (uint)param_1 * 0x4c0 + -0xff98838;
  }
  else {
    iVar1 = 0;
  }
  return iVar1;
}

// >>> MOD: wifi/mgmt/cnm_mem.c:0x57a <<<
// ===== FUN_ram_f0025ddc @ 0xf0025ddc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0025ddc(int param_1)

{
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x57a,_DAT_ram_00014800);
  }
  FUN_ram_f0040d3c(param_1);
  FUN_ram_f00415b0(param_1);
  FUN_ram_f00266d8(param_1 + 0x290);
  FUN_ram_f0029828(param_1);
  FUN_ram_f0035760(param_1);
  FUN_ram_f0039230(param_1);
  *(undefined1 *)(param_1 + 0x289) = 0;
  FUN_ram_f004b7e0(*(undefined1 *)(param_1 + 0x28a),0);
  FUN_ram_f002d6d8(param_1);
  FUN_ram_f003d488(param_1);
  FUN_ram_f0034724(param_1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025ef8 @ 0xf0025ef8 =====


void FUN_ram_f0025ef8(char param_1)

{
  char *pcVar1;
  char *pcVar2;
  
  pcVar1 = &DAT_ram_f00677d1;
  pcVar2 = &DAT_ram_f00677dc;
  do {
    if ((*pcVar1 != '\0') && (*pcVar2 == param_1)) {
      FUN_ram_f0025ddc(pcVar1 + -9);
      pcVar2[1] = '\0';
      *pcVar1 = '\0';
    }
    pcVar1 = pcVar1 + 0x4c0;
    pcVar2 = pcVar2 + 0x4c0;
  } while (pcVar1 != (char *)0xf006d6d1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025f4c @ 0xf0025f4c =====


void FUN_ram_f0025f4c(void)

{
  FUN_ram_f0025ef8();
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025f64 @ 0xf0025f64 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0025f64(void)

{
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f00655d0,0,0x28);
  _DAT_ram_f00655d0 = 0x214798;
  _DAT_ram_f00655d4 = 0xffffffff;
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f00655a8,0,0x28);
  _DAT_ram_f00655ac = 0xffffffff;
  _DAT_ram_f00655a8 = 0xf00651a8;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0025fc0 @ 0xf0025fc0 =====


void FUN_ram_f0025fc0(void)

{
  int iVar1;
  short sVar2;
  
  iVar1 = -0xff98838;
  sVar2 = 0;
  do {
    (*(code *)&SUB_ram_0006a0a0)(iVar1,0,0x4c0);
    *(char *)(iVar1 + 8) = (char)sVar2;
    FUN_ram_f00266a0(iVar1 + 0x58,&LAB_ram_f0030558,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x37c,&LAB_ram_f003f4d0,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x398,&LAB_ram_f003f384,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x3b0,&LAB_ram_f003f2b0,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x3dc,&LAB_ram_f003f648,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x3f4,&LAB_ram_f003f648,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x448,&LAB_ram_f003f9c8,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x480,&LAB_ram_f0040014,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x464,&LAB_ram_f003ff7c,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x42c,FUN_ram_f003fa48,iVar1);
    sVar2 = sVar2 + 1;
    FUN_ram_f00266a0(iVar1 + 0x410,FUN_ram_f003fddc,iVar1);
    iVar1 = iVar1 + 0x4c0;
  } while (sVar2 != 0x14);
  return;
}

// >>> MOD: wifi/mgmt/cnm_mem.c:0x5cc,0x5cd <<<
// ===== FUN_ram_f00260bc @ 0xf00260bc =====


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

// >>> MOD: wifi/mgmt/cnm_mem.c:0x66c <<<
// ===== FUN_ram_f0026470 @ 0xf0026470 =====


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

// >>> MOD: wifi/mgmt/cnm_mem.c:0x507 <<<
// ===== FUN_ram_f002653c @ 0xf002653c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f002653c(char param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  if (param_2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063148,0x507,_DAT_ram_00014800);
  }
  iVar2 = -0xff98838;
  while (((*(char *)(iVar2 + 9) == '\0' || (*(char *)(iVar2 + 0x14) != param_1)) ||
         (iVar1 = (*(code *)&SUB_ram_0006a074)(iVar2 + 10,param_2,6), iVar1 != 0))) {
    iVar2 = iVar2 + 0x4c0;
    if (iVar2 == -0xff92938) {
      return 0;
    }
  }
  return iVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00265a8 @ 0xf00265a8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00265a8(int param_1)

{
  if (_DAT_ram_f006d7d8 != 0) {
    (*_DAT_ram_00014840)(_DAT_ram_f006d7d8,_DAT_ram_f006d7d8,_DAT_ram_00014840);
    (*_DAT_ram_0001483c)(_DAT_ram_f006d7d8,param_1 * 1000 + 0x7fffU >> 0xf,_DAT_ram_0001483c);
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm_timer.c:0xe2 <<<
// ===== FUN_ram_f00265f0 @ 0xf00265f0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00265f0(undefined4 param_1)

{
  _DAT_ram_f006d7d4 = 0;
  _DAT_ram_f006d7cc = &DAT_ram_f006d7cc;
  _DAT_ram_f006d7d0 = &DAT_ram_f006d7cc;
  if (_DAT_ram_f006d7d8 == 0) {
    _DAT_ram_f006d7d8 = (*_DAT_ram_00014808)(param_1,_DAT_ram_00014808);
    if (_DAT_ram_f006d7d8 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf006315c,0xe2,_DAT_ram_00014800);
    }
  }
  else {
    (*_DAT_ram_00014840)(_DAT_ram_f006d7d8,_DAT_ram_f006d7d8,_DAT_ram_00014840);
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm_timer.c:0x111 <<<
// ===== FUN_ram_f0026644 @ 0xf0026644 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0026644(void)

{
  _DAT_ram_f006d7d4 = 0;
  _DAT_ram_f006d7cc = &DAT_ram_f006d7cc;
  _DAT_ram_f006d7d0 = &DAT_ram_f006d7cc;
  if (_DAT_ram_f006d7d8 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006315c,0x111,_DAT_ram_00014800);
  }
  (*_DAT_ram_00014840)(_DAT_ram_f006d7d8,_DAT_ram_00014840);
  (*_DAT_ram_00014820)(_DAT_ram_f006d7d8,_DAT_ram_00014820);
  _DAT_ram_f006d7d8 = 0;
  return;
}

// >>> MOD: wifi/mgmt/cnm_timer.c:0x129 <<<
// ===== FUN_ram_f00266a0 @ 0xf00266a0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00266a0(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  if (param_1 == (undefined4 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006315c,0x129,_DAT_ram_00014800);
  }
  param_1[4] = param_3;
  param_1[1] = 0;
  param_1[5] = param_2;
  *param_1 = 0;
  return;
}

// >>> MOD: wifi/mgmt/cnm_timer.c:0x159 <<<
// ===== FUN_ram_f00266d8 @ 0xf00266d8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00266d8(int *param_1)

{
  int iVar1;
  int *piVar2;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006315c,0x159,_DAT_ram_00014800);
  }
  iVar1 = *param_1;
  if (iVar1 != 0) {
    piVar2 = (int *)param_1[1];
    *(int **)(iVar1 + 4) = piVar2;
    *piVar2 = iVar1;
    param_1[1] = 0;
    *param_1 = 0;
    _DAT_ram_f006d7d4 = _DAT_ram_f006d7d4 + -1;
    if (_DAT_ram_f006d7cc == &DAT_ram_f006d7cc) {
      (*_DAT_ram_00014840)(_DAT_ram_f006d7d8,&DAT_ram_f006d7cc,_DAT_ram_00014840);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/cnm_timer.c:0x17c,0x184,0x197 <<<
// ===== FUN_ram_f002672c @ 0xf002672c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002672c(int *param_1,uint param_2)

{
  uint uVar1;
  short sVar2;
  int *piVar3;
  int iVar4;
  uint uVar5;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006315c,0x17c,_DAT_ram_00014800);
  }
  if (param_2 < 0xea61) {
    *(undefined2 *)(param_1 + 3) = 0;
  }
  else {
    if (0xea5f15a0 < param_2) {
      (*_DAT_ram_00014800)(0,0,0xf006315c,0x184,_DAT_ram_00014800);
    }
    uVar1 = param_2 / 60000;
    param_2 = param_2 + (uVar1 & 0xffff) * -60000;
    sVar2 = (short)uVar1;
    *(short *)(param_1 + 3) = sVar2;
    if (param_2 == 0) {
      *(short *)(param_1 + 3) = sVar2 + -1;
      param_2 = 60000;
    }
    else if (0xfffe < param_2) {
      (*_DAT_ram_00014800)(0,0,0xf006315c,0x197,_DAT_ram_00014800);
    }
  }
  uVar1 = (param_2 * 0x8000 + 999) / 1000;
  uVar5 = uVar1;
  if (uVar1 == 0) {
    uVar5 = 1;
  }
  iVar4 = (*_DAT_ram_00014830)(uVar1,_DAT_ram_00014830);
  iVar4 = iVar4 + uVar5;
  if ((_DAT_ram_f006d7cc == &DAT_ram_f006d7cc) || (iVar4 - _DAT_ram_f006d7dc < 0)) {
    _DAT_ram_f006d7dc = iVar4;
    FUN_ram_f00265a8(uVar5);
  }
  param_1[2] = iVar4;
  piVar3 = _DAT_ram_f006d7d0;
  if (*param_1 == 0) {
    _DAT_ram_f006d7d0 = param_1;
    *param_1 = (int)&DAT_ram_f006d7cc;
    param_1[1] = (int)piVar3;
    *piVar3 = (int)param_1;
    _DAT_ram_f006d7d4 = _DAT_ram_f006d7d4 + 1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002694c @ 0xf002694c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002694c(void)

{
  (*_DAT_ram_00014b38)(0,0,_DAT_ram_00014b38);
  (*_DAT_ram_00014984)(0xffffffff,0xffffffff,_DAT_ram_00014984);
  _DAT_ram_60310000 = 0xffffffff;
  _DAT_ram_60310004 = 0xffffffff;
  (*_DAT_ram_00014a14)(_DAT_ram_0209fed8,0,_DAT_ram_00014a14);
  (*_DAT_ram_00014a14)(_DAT_ram_0209fed8,1,_DAT_ram_00014a14);
  (*_DAT_ram_000149f4)(_DAT_ram_0209fed8,0,_DAT_ram_000149f4);
  (*_DAT_ram_000149f4)(_DAT_ram_0209fed8,1,_DAT_ram_000149f4);
  _DAT_ram_50000110 = 0x100;
  _DAT_ram_50000100 = 0xffffffff;
  (*_DAT_ram_00014860)(4,_DAT_ram_00014860);
  (*_DAT_ram_00014e98)(2,0,_DAT_ram_00014e98);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00269e4 @ 0xf00269e4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00269e4(undefined1 *param_1)

{
  int iVar1;
  
  iVar1 = (*_DAT_ram_00014e9c)(param_1,_DAT_ram_00014e9c);
  if (iVar1 == 0) {
    param_1[1] = 0;
    *param_1 = 0;
  }
  else {
    *param_1 = *(undefined1 *)(iVar1 + 0x14);
    param_1[1] = *(undefined1 *)(iVar1 + 0x15);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0026b00 @ 0xf0026b00 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0026b00(byte param_1,byte param_2)

{
  undefined4 local_18;
  undefined4 uStack_14;
  
  uStack_14 = 0;
  local_18 = 0;
  if (param_1 == 1) {
    (*(code *)&SUB_ram_0006a08c)((int)&local_18 + 2,0xf00630cc,5);
    _DAT_ram_5000012c = local_18;
    _DAT_ram_50000130 = uStack_14;
  }
  _DAT_ram_5000012c = (uint)param_2 + (uint)param_1 * 100 | _DAT_ram_5000012c & 0xffff0000;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00273ac @ 0xf00273ac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00273ac(void)

{
  (*_DAT_ram_00014a04)(0,_DAT_ram_00014a04);
  FUN_ram_f002694c();
  (*(code *)&SUB_ram_0006aa24)(2,0x6a,0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00273e4 @ 0xf00273e4 =====


void FUN_ram_f00273e4(char param_1,uint param_2,uint *param_3)

{
  char cVar1;
  uint uVar2;
  undefined2 *puVar3;
  ushort *puVar4;
  
  if (param_2 + 0x7ef77fff < 0xe) {
    param_2 = param_2 & 0xff;
    if (param_1 != '\0') {
      uVar2 = *param_3;
      cVar1 = FUN_ram_f00488dc(param_2);
      FUN_ram_f0048914(param_2,(int)(char)((char)uVar2 - cVar1));
      return;
    }
    if (param_3 == (uint *)0x0) {
      return;
    }
    uVar2 = FUN_ram_f00488dc(param_2);
  }
  else {
    if (param_2 == 0x81088010) {
      if (param_1 != '\0') {
        puVar3 = (undefined2 *)&DAT_ram_f006506e;
LAB_ram_f0027460:
        *puVar3 = (short)*param_3;
        FUN_ram_f004898c();
        return;
      }
      if (param_3 == (uint *)0x0) {
        return;
      }
      puVar4 = (ushort *)&DAT_ram_f006506e;
    }
    else {
      if (param_2 != 0x81088011) {
        param_2 = param_2 + 0x7ef77fe0;
        if (2 < param_2) {
          return;
        }
        if (param_1 != '\0') {
          *(short *)(&DAT_ram_f006fa34 + param_2 * 2) = (short)*param_3;
          return;
        }
        if (param_3 == (uint *)0x0) {
          return;
        }
        uVar2 = (uint)*(short *)(&DAT_ram_f006fa34 + param_2 * 2);
        goto LAB_ram_f00274ac;
      }
      if (param_1 != '\0') {
        puVar3 = (undefined2 *)&DAT_ram_f0065070;
        goto LAB_ram_f0027460;
      }
      if (param_3 == (uint *)0x0) {
        return;
      }
      puVar4 = (ushort *)&DAT_ram_f0065070;
    }
    uVar2 = (uint)*puVar4;
  }
LAB_ram_f00274ac:
  *param_3 = uVar2;
  return;
}

// >>> MOD: wifi/mgmt/hem.c:0x585 <<<
// ===== FUN_ram_f00278a8 @ 0xf00278a8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00278a8(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  int iVar5;
  undefined2 *puVar6;
  undefined4 uStack_2c;
  undefined2 local_28;
  undefined4 auStack_24 [2];
  
  do {
    _DAT_ram_f00650a8 = _DAT_ram_f00650a4;
    _DAT_ram_f00650a4 = param_1;
    if (param_1 == 4) {
      FUN_ram_f00481e8();
    }
    else if (param_1 == 5) {
      if (DAT_ram_f0065005 != '\0') {
        DAT_ram_f0065005 = '\0';
        iVar5 = FUN_ram_f0025c48(8);
        if ((iVar5 == 0) || (DAT_ram_f0064fb5 == -1)) {
          param_1 = 4;
        }
        else {
          puVar6 = *(undefined2 **)(iVar5 + 8);
          *(undefined1 *)(puVar6 + 2) = 0xfc;
          *(char *)((int)puVar6 + 5) = DAT_ram_f0064fb5;
          *puVar6 = 8;
          *(undefined2 **)(iVar5 + 0x28) = puVar6;
          *(undefined1 *)(iVar5 + 0x2f) = 0;
          puVar6[1] = 1;
          param_1 = 4;
          FUN_ram_f0039750();
          DAT_ram_f0064fb5 = -1;
        }
      }
    }
    else if (param_1 == 2) {
      _DAT_ram_f00650b0 = 0x6a0;
      _DAT_ram_f00650b4 = 0x6d8;
      _DAT_ram_f00650b8 = _DAT_ram_0209722c;
      _DAT_ram_f00650bc = _DAT_ram_02097228;
      FUN_ram_f0026b00(1,0);
      uVar2 = (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_020a0064,0,0x2160);
      puVar3 = (undefined4 *)(*_DAT_ram_00014e9c)(uVar2,_DAT_ram_00014e9c);
      *puVar3 = _DAT_ram_60205c60;
      (*_DAT_ram_00014814)(2,0xf0063238,&DAT_ram_60205c60,_DAT_ram_60205c60,_DAT_ram_00014814);
      uVar2 = (*_DAT_ram_00014814)
                        (2,0xf0063238,&DAT_ram_60205c50,_DAT_ram_60205c50,_DAT_ram_00014814);
      (*_DAT_ram_00014c2c)(uVar2,_DAT_ram_00014c2c);
      uVar2 = (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
      (*_DAT_ram_00014c38)(uVar2,_DAT_ram_00014c38);
      FUN_ram_f004afa4(_DAT_ram_0209fef0);
      *_DAT_ram_0209fee4 = 0x51;
      *_DAT_ram_0209fee8 = 0xc;
      *_DAT_ram_0209feec = 0x10;
      (*_DAT_ram_00014d70)
                (_DAT_ram_020a0074,_DAT_ram_0209fee8,_DAT_ram_0209feec,0x209f000,_DAT_ram_00014d70);
      uVar1 = _DAT_ram_600000bc;
      uVar2 = _DAT_ram_600000b8;
      uVar4 = (*_DAT_ram_00014a24)
                        (_DAT_ram_0209fed8,_DAT_ram_0209fedc,0x1c,_DAT_ram_0209fedc + 0x2a0,4,
                         _DAT_ram_0209fee0,0x10,2,_DAT_ram_0209fef0,_DAT_ram_020a0078,
                         _DAT_ram_0209fee4,0x6ab14,0x6aadc,0x6aaa8);
      _DAT_ram_600000bc = uVar1;
      _DAT_ram_600000b8 = uVar2;
      (*_DAT_ram_00014e48)(uVar4,_DAT_ram_00014e48);
      _DAT_ram_60130050 = 0x3000e7;
      _DAT_ram_60130054 = 0x28004c;
      _DAT_ram_60130060 = 0x10;
      _DAT_ram_60130000 = _DAT_ram_60130000 | 0x10800000;
      _DAT_ram_6013005c = _DAT_ram_6013005c | 0x80000000;
      FUN_ram_f00284a0();
      FUN_ram_f00430c8();
      uStack_2c = 0;
      local_28 = 0;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20a7) = 1;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20a9) = 1;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20a8) = 1;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20aa) = 0;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20ab) = 1;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20ac) = 0;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20ad) = 0;
      *(undefined4 *)(_DAT_ram_020a0064 + 0x20b4) = 1;
      *(undefined4 *)(_DAT_ram_020a0064 + 0x20b8) = 0;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x215c) = 0;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20ae) = 0xff;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20af) = 0;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20b0) = 0;
      FUN_ram_f0026b00(2,0);
      FUN_ram_f0025f64();
      FUN_ram_f0025a70();
      FUN_ram_f00265f0(2);
      FUN_ram_f0025fc0();
      FUN_ram_f0028478();
      _DAT_ram_f00650ac = (byte *)FUN_ram_f0025b48(2,0x40);
      if (_DAT_ram_f00650ac == (byte *)0x0) {
        (*_DAT_ram_00014800)(0,0,0xf00631dc,0x585,_DAT_ram_00014800);
      }
      DAT_ram_f0065007 = 0;
      if (_DAT_ram_f00650ac != (byte *)0x0) {
        FUN_ram_f0042eb8();
        if (((*_DAT_ram_f00650ac & 1) == 0) &&
           (iVar5 = (*(code *)&SUB_ram_0006a074)(_DAT_ram_f00650ac,&uStack_2c,6), iVar5 != 0)) {
          DAT_ram_f0065007 = 1;
        }
        else {
          auStack_24[0] = (*_DAT_ram_00014830)(_DAT_ram_00014830);
          *_DAT_ram_f00650ac = 0;
          _DAT_ram_f00650ac[1] = 8;
          _DAT_ram_f00650ac[2] = 0x22;
          (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f00650ac + 3,auStack_24);
        }
      }
      *(undefined4 *)(_DAT_ram_020a0064 + 0x20a0) = 2;
      *(undefined1 *)(_DAT_ram_020a0064 + 0x20a4) = 1;
      FUN_ram_f002365c();
      FUN_ram_f002444c();
      FUN_ram_f0021538(0);
      FUN_ram_f0026b00(3,0);
      FUN_ram_f003b4a0();
      DAT_ram_f0065006 = _DAT_ram_f006514c != 0x6625;
      FUN_ram_f0026b00(4,0);
      FUN_ram_f003c618();
      FUN_ram_f003e540();
      FUN_ram_f0026b00(5,0);
      FUN_ram_f0029580();
      FUN_ram_f0033e2c();
      FUN_ram_f0038f3c();
      FUN_ram_f004b1d0();
      FUN_ram_f004b3d4(DAT_ram_f0065005);
      FUN_ram_f004b1d4();
      FUN_ram_f004b520(DAT_ram_f0065005);
      FUN_ram_f004a784();
      FUN_ram_f0026b00(6,0);
      FUN_ram_f0042b1c();
      FUN_ram_f002a3a4();
      FUN_ram_f0031cdc();
      FUN_ram_f002fe3c(_DAT_ram_020a0064 + 0xf68);
      FUN_ram_f002fe3c(_DAT_ram_020a0064 + 0x12f8);
      FUN_ram_f002fe3c(_DAT_ram_020a0064 + 0x1aa8);
      FUN_ram_f003e608();
      FUN_ram_f00420e8();
      (*_DAT_ram_00014964)(0,_DAT_ram_00014964);
      (*_DAT_ram_00014b38)(1,1,_DAT_ram_00014b38);
      FUN_ram_f0026b00(7,0);
      (*_DAT_ram_00014a04)(1,_DAT_ram_00014a04);
      (*_DAT_ram_00014988)(0x27f,0,_DAT_ram_00014988);
      (*_DAT_ram_00014988)(0x80,0,_DAT_ram_00014988);
      FUN_ram_f004be1c(1,1);
      FUN_ram_f002856c();
      if (_DAT_ram_f00650ac != (byte *)0x0) {
        FUN_ram_f0025cac();
        _DAT_ram_f00650ac = (byte *)0x0;
      }
      FUN_ram_f0026b00(100,0);
      _DAT_ram_60000058 = 0x3001ff;
      _DAT_ram_603000a8 = _DAT_ram_603000a8 & 0xffff | 0xc50000;
      _DAT_ram_603000ac = 0xd100c8;
      _DAT_ram_603000b8 = 0x19b10078;
      _DAT_ram_603000f8 = _DAT_ram_603000f8 & 0xff | 0xc8c9ca00;
      DAT_ram_f0064fd8 = 0;
      FUN_ram_f00266a0(0xf00701b0,&LAB_ram_f0028508,0);
      _DAT_ram_f00650c0 = 0xe78;
      param_1 = 5;
    }
  } while (param_1 != _DAT_ram_f00650a4);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028000 @ 0xf0028000 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028000(void)

{
  DAT_ram_f0065005 = 0;
  _DAT_ram_f00650a8 = 0;
  _DAT_ram_f00650a4 = 0;
  FUN_ram_f00278a8(2);
  (*(code *)&SUB_ram_0006a2a0)();
  return;
}

// >>> MOD: wifi/mgmt/hem_cmd.c:0x2d9 <<<
// ===== FUN_ram_f0028190 @ 0xf0028190 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028190(int param_1)

{
  char cVar1;
  char *pcVar2;
  ushort uVar3;
  int iVar4;
  
  iVar4 = *(int *)(param_1 + 8);
  if (*(byte *)(iVar4 + 0x33) >> 6 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf0063240,0x2d9,_DAT_ram_00014800);
  }
  cVar1 = *(char *)(iVar4 + 0x34);
  uVar3 = 0;
  pcVar2 = &DAT_ram_f0063254;
  while (((*pcVar2 != cVar1 || (*(code **)(pcVar2 + 4) == (code *)0x0)) ||
         ((_DAT_ram_f00650a4 != 5 &&
          ((_DAT_ram_f00650a4 != 4 ||
           ((((((cVar1 != '\x01' && (cVar1 != -0x3e)) && (cVar1 != -0x3b)) &&
              ((cVar1 != '\x05' && (cVar1 != '(')))) &&
             ((cVar1 != ')' && ((cVar1 != '*' && (cVar1 != '1')))))) &&
            ((cVar1 != '<' && ((cVar1 != '=' && (cVar1 != '6'))))))))))))) {
    uVar3 = uVar3 + 1;
    pcVar2 = pcVar2 + 8;
    if (uVar3 == 0x42) {
LAB_ram_f002823c:
      (*(code *)&SUB_ram_0006a0d8)(param_1);
      return;
    }
  }
  iVar4 = (**(code **)(pcVar2 + 4))(param_1);
  if (iVar4 != 1) {
    (*(code *)&SUB_ram_0006a0d8)(param_1);
  }
  if (uVar3 < 0x42) {
    return;
  }
  goto LAB_ram_f002823c;
}

// >>> MOD: wifi/mgmt/hem_mbox.c:0x1a2 <<<
// ===== FUN_ram_f0028250 @ 0xf0028250 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028250(void)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  int iVar6;
  int *piVar7;
  
  piVar4 = (int *)&DAT_ram_f00648b4;
  uVar5 = 1;
  piVar7 = piVar4;
  do {
    if (*piVar7 != uVar5 - 1) {
      piVar2 = (int *)(&DAT_ram_f00648b4 + uVar5 * 8);
      for (uVar1 = uVar5; uVar1 < 9; uVar1 = uVar1 + 1) {
        iVar6 = *piVar2;
        piVar2 = piVar2 + 2;
        if (iVar6 == uVar5 - 1) {
          iVar3 = *(int *)(&DAT_ram_f00648b4 + uVar1 * 8);
          iVar6 = *(int *)(&DAT_ram_f00648b8 + uVar1 * 8);
          *(int *)(&DAT_ram_f00648b4 + uVar1 * 8) = *piVar7;
          *(int *)(&DAT_ram_f00648b8 + uVar1 * 8) = piVar7[1];
          *piVar7 = iVar3;
          piVar7[1] = iVar6;
          goto LAB_ram_f00282a8;
        }
      }
      (*_DAT_ram_00014800)(0,0,0xf0063464,400,_DAT_ram_00014800);
    }
LAB_ram_f00282a8:
    uVar5 = uVar5 + 1;
    piVar7 = piVar7 + 2;
    if (uVar5 == 10) {
      iVar6 = 0;
      while( true ) {
        if (*piVar4 != iVar6) {
          (*_DAT_ram_00014800)(0,0,0xf0063464,0x1a2,_DAT_ram_00014800);
        }
        if (*piVar4 != iVar6) break;
        iVar6 = iVar6 + 1;
        piVar4 = piVar4 + 2;
        if (iVar6 == 9) {
          return;
        }
      }
      do {
                    /* WARNING: Do nothing block with infinite loop */
      } while( true );
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/hem_mbox.c:0x1b7 <<<
// ===== FUN_ram_f0028304 @ 0xf0028304 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028304(int param_1)

{
  int iVar1;
  
  if (param_1 != 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063464,0x1b7,_DAT_ram_00014800);
  }
  param_1 = param_1 * 0xc;
  iVar1 = param_1 + -0xff92938;
  *(undefined4 *)(param_1 + -0xff92930) = 0;
  *(int *)iVar1 = iVar1;
  *(int *)(param_1 + -0xff92934) = iVar1;
  return;
}

// >>> MOD: wifi/mgmt/hem_mbox.c:0x1cf,0x1d0,0x1de <<<
// ===== FUN_ram_f0028344 @ 0xf0028344 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028344(int param_1,int *param_2,int param_3)

{
  undefined4 *puVar1;
  
  if (param_1 != 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063464,0x1cf,_DAT_ram_00014800);
  }
  if (param_2 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063464,0x1d0,_DAT_ram_00014800);
  }
  if (param_3 == 0) {
    param_1 = param_1 * 0xc;
    puVar1 = *(undefined4 **)(param_1 + -0xff92934);
    *(int **)(param_1 + -0xff92934) = param_2;
    *param_2 = param_1 + -0xff92938;
    param_2[1] = (int)puVar1;
    *puVar1 = param_2;
    *(int *)(param_1 + -0xff92930) = *(int *)(param_1 + -0xff92930) + 1;
  }
  else if (param_3 == 1) {
    (**(code **)(&DAT_ram_f00648b8 + param_2[2] * 8))
              (param_2,*(code **)(&DAT_ram_f00648b8 + param_2[2] * 8));
  }
  else {
    (*_DAT_ram_00014800)(0,0,0xf0063464,0x1de,_DAT_ram_00014800);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028478 @ 0xf0028478 =====


void FUN_ram_f0028478(void)

{
  FUN_ram_f0028250();
  FUN_ram_f0028304(0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00284a0 @ 0xf00284a0 =====


void FUN_ram_f00284a0(void)

{
  DAT_ram_f0065008 = 0;
  DAT_ram_f0065009 = 0;
  (*(code *)&SUB_ram_0006a19c)();
  DAT_ram_f006500a = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002856c @ 0xf002856c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002856c(void)

{
  FUN_ram_f00266a0(0xf006d6d4,&LAB_ram_f002849c,0);
  _DAT_ram_f00650d0 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00285a8 @ 0xf00285a8 =====


void FUN_ram_f00285a8(int param_1)

{
  ushort uVar1;
  undefined1 uVar2;
  bool bVar3;
  ushort uVar4;
  int iVar5;
  
  uVar1 = **(ushort **)(param_1 + 0x14);
  uVar4 = uVar1 & 0xfc;
  if (uVar4 == 0x40) {
LAB_ram_f00285d8:
    uVar2 = 5;
  }
  else {
    if (uVar4 < 0x41) {
      if (((uVar1 & 0xfc) == 0) || (uVar4 == 0x20)) goto LAB_ram_f00285d8;
LAB_ram_f00285ea:
      if (*(char *)(param_1 + 0x10) == '\x01') {
        bVar3 = (bool)(((byte)(*(ushort **)(param_1 + 0x14))[2] ^ 1) & 1);
      }
      else {
        bVar3 = *(int *)(param_1 + 0x20) != 0;
      }
      if (bVar3 == false) {
        iVar5 = *(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0xc);
        if (((iVar5 != 1) && (iVar5 == 2)) &&
           (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0xbb) !=
            '\0')) {
          uVar2 = 8;
          goto LAB_ram_f0028632;
        }
      }
    }
    else if (uVar4 == 0x80) {
      if (*(char *)(param_1 + 0xd) != '\x02') {
        uVar2 = 7;
        goto LAB_ram_f0028632;
      }
    }
    else {
      if (uVar4 == 0xb0) goto LAB_ram_f00285d8;
      if (uVar4 != 0x50) goto LAB_ram_f00285ea;
    }
    uVar2 = 4;
  }
LAB_ram_f0028632:
  *(undefined1 *)(param_1 + 0x40) = uVar2;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028638 @ 0xf0028638 =====


void FUN_ram_f0028638(int param_1)

{
  *(undefined1 *)(param_1 + 0x40) = *(undefined1 *)(*(byte *)(param_1 + 0xf) + 0xf00648fc);
  return;
}

// >>> MOD: wifi/mgmt/mqm.c:0x4c4 <<<
// ===== FUN_ram_f0028650 @ 0xf0028650 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028650(int param_1)

{
  bool bVar1;
  int iVar2;
  
  if (*(char *)(param_1 + 0x10) == '\x01') {
    bVar1 = (bool)(*(byte *)(*(int *)(param_1 + 0x14) + 4) & 1);
  }
  else {
    bVar1 = *(int *)(param_1 + 0x20) == 0;
  }
  if (bVar1 == false) {
    if (*(char *)(param_1 + 0x10) == '\x01') {
      bVar1 = (**(byte **)(param_1 + 0x14) & 0xfc) == 0x88;
    }
    else {
      bVar1 = false;
      if (*(int *)(param_1 + 0x20) != 0) {
        bVar1 = *(char *)(*(int *)(param_1 + 0x20) + 0x2a8) != '\0';
      }
    }
    if (bVar1) {
      if (7 < *(byte *)(param_1 + 0xf)) {
        (*_DAT_ram_00014800)(0,0,0xf00634e8,0x4c4,_DAT_ram_00014800);
      }
      *(undefined1 *)(param_1 + 0x40) = *(undefined1 *)(*(byte *)(param_1 + 0xf) + 0xf00648fc);
      if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0x35c) !=
          '\x01') {
        return;
      }
      iVar2 = 1;
      goto LAB_ram_f002870e;
    }
  }
  else {
    iVar2 = *(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0xc);
    if (iVar2 != 2) {
      if (iVar2 != 4) {
        iVar2 = 4;
      }
      goto LAB_ram_f002870e;
    }
    if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0xbb) != '\0')
    {
      iVar2 = 8;
      goto LAB_ram_f002870e;
    }
  }
  iVar2 = 4;
LAB_ram_f002870e:
  *(char *)(param_1 + 0x40) = (char)iVar2;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028718 @ 0xf0028718 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028718(int param_1)

{
  undefined1 local_a [2];
  
  (*_DAT_ram_000149b4)(local_a,_DAT_ram_000149b4);
  *(undefined1 *)(param_1 + 4) = local_a[0];
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028740 @ 0xf0028740 =====


bool FUN_ram_f0028740(int param_1)

{
  bool bVar1;
  
  bVar1 = false;
  if (param_1 != 0) {
    bVar1 = *(char *)(param_1 + 9) != '\0';
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028750 @ 0xf0028750 =====


byte FUN_ram_f0028750(int param_1)

{
  byte bVar1;
  int iVar2;
  
  iVar2 = FUN_ram_f0028740();
  bVar1 = 0;
  if (iVar2 != 0) {
    bVar1 = *(byte *)(param_1 + 0x17) >> 4 & 1;
  }
  return bVar1;
}

// >>> MOD: wifi/mgmt/mqm.c:0x512,0x530 <<<
// ===== FUN_ram_f0028770 @ 0xf0028770 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028770(int param_1)

{
  ushort uVar1;
  uint uVar2;
  int iVar3;
  ushort *puVar4;
  uint uVar5;
  int iVar6;
  
  uVar5 = (uint)*(byte *)(param_1 + 0x40);
  if (uVar5 < 9) {
    uVar2 = 1 << (uVar5 & 0x1f);
    if ((uVar2 & 0x1b0) == 0) {
      if ((uVar2 & 0x40) == 0) {
        if ((uVar2 & 0xf) == 0) goto LAB_ram_f0028848;
        iVar6 = *(int *)(param_1 + 0x20);
        iVar3 = FUN_ram_f0028750(iVar6);
        if (iVar3 != 0) goto LAB_ram_f002882a;
        puVar4 = (ushort *)(_DAT_ram_f0064f34 + uVar5 * 2 + 4);
      }
      else {
        iVar6 = *(int *)(*(int *)(param_1 + 0x20) + (*(byte *)(param_1 + 0xf) + 0x54) * 4 + 4);
        if ((iVar6 == 0) || (*(char *)(iVar6 + 1) != '\x02')) {
          (*_DAT_ram_00014800)(0,0,0xf00634e8,0x512,_DAT_ram_00014800);
        }
        puVar4 = (ushort *)(*(int *)(param_1 + 0x20) + (*(byte *)(param_1 + 0xf) + 0xdc) * 2);
      }
    }
    else {
      iVar6 = *(int *)(param_1 + 0x20);
      puVar4 = (ushort *)(_DAT_ram_f0064f34 + 0xc);
      if (((((*(uint *)(iVar6 + 0x10) & 0x80) != 0) && (uVar5 == 4)) &&
          ((**(ushort **)(param_1 + 0x14) & 0xfc) == 0x88)) &&
         (iVar3 = FUN_ram_f0028750(iVar6), iVar3 != 0)) {
LAB_ram_f002882a:
        puVar4 = (ushort *)((*(byte *)(param_1 + 0xf) + 0xdc) * 2 + iVar6);
      }
    }
    if (puVar4 != (ushort *)0x0) {
      uVar1 = *puVar4;
      *puVar4 = uVar1 & 0xfff;
      *(undefined1 *)(param_1 + 0x54) = 1;
      *(ushort *)(param_1 + 0x56) = uVar1 & 0xfff;
      *puVar4 = *puVar4 + 1;
    }
  }
  else {
LAB_ram_f0028848:
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0x530,_DAT_ram_00014800);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00288e8 @ 0xf00288e8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00288e8(void)

{
  _DAT_ram_60130008 = 0x2f002f;
  _DAT_ram_60130054 = 0x4c008c;
  _DAT_ram_60130064 = 0x90a09ff;
  if ((_DAT_ram_60120020 & 0xf00) >> 8 < 3) {
    _DAT_ram_60120020 = _DAT_ram_60120020 & 0xfffff0ff | 0x300;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002893c @ 0xf002893c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002893c(void)

{
  _DAT_ram_60130054 = 0x28004c;
  _DAT_ram_60130064 = 0x90a0968;
  if ((_DAT_ram_60120020 & 0xf00) >> 8 < 3) {
    _DAT_ram_60120020 = _DAT_ram_60120020 & 0xfffff0ff | 0x300;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028984 @ 0xf0028984 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028984(int param_1)

{
  char cVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  char *pcVar5;
  undefined2 *puVar6;
  char cVar7;
  int iVar8;
  uint uVar9;
  undefined2 local_48 [22];
  
  iVar8 = 0;
  puVar6 = local_48;
  uVar9 = 0xfffffffe;
  do {
    iVar3 = param_1 + iVar8 * 10;
    local_48[iVar8 * 5 + 3] = *(undefined2 *)(iVar3 + 0x2a6);
    local_48[iVar8 * 5] = *(undefined2 *)(iVar3 + 0x2a0);
    local_48[iVar8 * 5 + 1] = *(undefined2 *)(iVar3 + 0x2a2);
    local_48[iVar8 * 5 + 2] = *(undefined2 *)(iVar3 + 0x2a4);
    *(undefined1 *)(local_48 + iVar8 * 5 + 4) = 0;
    if (((uVar9 < 2) && (*_DAT_ram_f0065118 != '\0')) &&
       ((*(uint *)(_DAT_ram_f0065118 + 0x28) & 1) != 0)) {
      local_48[iVar8 * 5 + 2] = 1000;
      local_48[iVar8 * 5 + 3] = 1;
      local_48[iVar8 * 5] = 0;
    }
    FUN_ram_f004bed0(*(undefined1 *)(iVar8 + -0xff9b6fc),puVar6);
    uVar9 = uVar9 + 1;
    iVar8 = iVar8 + 1;
    puVar6 = puVar6 + 5;
  } while (uVar9 != 2);
  cVar7 = '\x03';
  iVar3 = 3;
  iVar8 = 2;
  do {
    if (*(char *)(param_1 + iVar3 * 10 + 0x2a9) != '\0') {
      cVar2 = '\0';
      pcVar5 = (char *)(param_1 + iVar8 * 10 + 0x2a9);
      do {
        iVar4 = (int)(char)((cVar7 + -1) - cVar2);
        if (iVar4 < 0) goto LAB_ram_f0028a58;
        cVar1 = *pcVar5;
        pcVar5 = pcVar5 + -10;
        cVar2 = cVar2 + '\x01';
      } while (cVar1 != '\0');
      FUN_ram_f004bed0(*(undefined1 *)(iVar3 + -0xff9b6fc),local_48 + iVar4 * 5);
    }
LAB_ram_f0028a58:
    cVar7 = cVar7 + -1;
    iVar8 = iVar8 + -1;
    iVar3 = iVar3 + -1;
    if (iVar8 == 0) {
      return;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028ad0 @ 0xf0028ad0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028ad0(void)

{
  undefined2 *puVar1;
  undefined2 *puVar2;
  undefined2 local_1c;
  undefined2 local_1a;
  undefined2 local_18;
  undefined2 local_16;
  undefined1 local_14;
  
  puVar2 = (undefined2 *)&DAT_ram_f00634fa;
  do {
    local_18 = *puVar2;
    puVar1 = puVar2 + -1;
    local_16 = puVar2[1];
    local_1a = puVar2[2];
    local_1c = puVar2[3];
    local_14 = *(undefined1 *)(puVar2 + 4);
    puVar2 = puVar2 + 6;
    FUN_ram_f004bed0(*(undefined1 *)puVar1,&local_1c);
  } while (puVar2 != (undefined2 *)0xf0063566);
  *(undefined1 *)(_DAT_ram_f0064f34 + 0x1a9) = 0;
  *(undefined1 *)(_DAT_ram_f0064f34 + 0x1b3) = 0;
  *(undefined1 *)(_DAT_ram_f0064f34 + 0x1bd) = 0;
  *(undefined1 *)(_DAT_ram_f0064f34 + 0x1c7) = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028b44 @ 0xf0028b44 =====


void FUN_ram_f0028b44(undefined4 param_1)

{
  (*(code *)&SUB_ram_0006a0d8)(param_1,&SUB_ram_0006a0d8);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028b5c @ 0xf0028b5c =====


void FUN_ram_f0028b5c(char param_1,int param_2,short param_3,undefined2 param_4)

{
  int iVar1;
  ushort uVar2;
  int iVar3;
  undefined2 *puVar4;
  
  iVar1 = FUN_ram_f0028750(param_2);
  if ((iVar1 != 0) && (iVar1 = FUN_ram_f0025c48(0x1e), iVar1 != 0)) {
    if (param_1 == '\0') {
      uVar2 = 0;
    }
    else {
      uVar2 = 0x800;
    }
    puVar4 = *(undefined2 **)(iVar1 + 8);
    iVar3 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_2 + 0x14) * 4);
    *puVar4 = 0xd0;
    puVar4[1] = 0;
    *(undefined1 *)(puVar4 + 0xc) = 3;
    *(undefined1 *)((int)puVar4 + 0x19) = 2;
    puVar4[0xd] = uVar2 | param_3 << 0xc;
    puVar4[0xe] = param_4;
    (*(code *)&SUB_ram_0006a08c)(puVar4 + 2,param_2 + 10,6);
    (*(code *)&SUB_ram_0006a08c)(puVar4 + 5,iVar3 + 0x51,6);
    (*(code *)&SUB_ram_0006a08c)(puVar4 + 8,iVar3 + 0x4b,6);
    FUN_ram_f0034540(iVar1,param_2,*(undefined1 *)(param_2 + 0x14),puVar4,0x18,puVar4 + 0xc,6,
                     &LAB_ram_f00290c8,0,0,0,0,0);
    *(char *)(iVar1 + 0xf) = (char)param_3;
    *(bool *)(iVar1 + 0x24) = param_1 != '\0';
    FUN_ram_f0037b58(iVar1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028c5c @ 0xf0028c5c =====


void FUN_ram_f0028c5c(undefined1 param_1,undefined1 param_2)

{
  int iVar1;
  undefined2 *puVar2;
  
  iVar1 = FUN_ram_f0025c48(0xc);
  if (iVar1 != 0) {
    puVar2 = *(undefined2 **)(iVar1 + 8);
    *puVar2 = 0xc;
    *(undefined1 *)(puVar2 + 2) = 0x12;
    *(undefined1 *)(puVar2 + 4) = param_1;
    *(undefined1 *)((int)puVar2 + 9) = param_2;
    *(undefined1 *)((int)puVar2 + 5) = 0;
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar2;
    puVar2[1] = 1;
    FUN_ram_f0039750();
  }
  return;
}

// >>> MOD: wifi/mgmt/mqm.c:0x8ea <<<
// ===== FUN_ram_f0028ca8 @ 0xf0028ca8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028ca8(int param_1,int param_2,uint param_3)

{
  undefined1 uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined2 *puVar4;
  
  iVar2 = FUN_ram_f0028750();
  if (((iVar2 != 0) && (*(int *)((param_2 + 0x54) * 4 + param_1 + 4) != 0)) &&
     (puVar3 = (undefined4 *)FUN_ram_f0025c48(0x14), puVar3 != (undefined4 *)0x0)) {
    puVar4 = (undefined2 *)puVar3[2];
    *puVar4 = 0x84;
    puVar4[1] = 0;
    (*(code *)&SUB_ram_0006a08c)(puVar4 + 2,param_1 + 10,6);
    (*(code *)&SUB_ram_0006a08c)
              (puVar4 + 5,*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x51,6
              );
    puVar4[8] = (ushort)(param_2 << 0xc) | 4;
    *(char *)(puVar4 + 9) = (char)(param_3 << 4);
    *(char *)((int)puVar4 + 0x13) = (char)(param_3 >> 4);
    if (puVar3[2] == 0) {
      (*_DAT_ram_00014800)(0,0,0xf00634e8,0x8ea,_DAT_ram_00014800);
    }
    if (*(char *)(puVar3 + 3) != '\x03') {
      (*_DAT_ram_00014800)(0,0,0xf00634e8,0x8ea,_DAT_ram_00014800);
    }
    puVar3[8] = param_1;
    uVar1 = *(undefined1 *)(param_1 + 0x14);
    puVar3[6] = puVar4 + 8;
    *(undefined1 *)(puVar3 + 0x16) = 1;
    *(undefined1 *)((int)puVar3 + 0x41) = 1;
    *(undefined1 *)(puVar3 + 4) = 1;
    *(undefined1 *)(puVar3 + 0x15) = 1;
    *(undefined1 *)((int)puVar3 + 0xd) = uVar1;
    *(undefined1 *)((int)puVar3 + 0x11) = 0x10;
    *(undefined2 *)((int)puVar3 + 0x12) = 4;
    *(char *)((int)puVar3 + 0xf) = (char)param_2;
    *(undefined1 *)((int)puVar3 + 0x29) = 0;
    puVar3[5] = puVar4;
    puVar3[0xb] = 0;
    *(undefined1 *)(puVar3 + 0x12) = 0;
    *(undefined1 *)((int)puVar3 + 0x49) = 0;
    *(undefined1 *)((int)puVar3 + 0x4a) = 0;
    *puVar3 = 0;
    *(undefined1 *)(puVar3 + 10) = 0;
    FUN_ram_f0037888(puVar3);
  }
  return;
}

// >>> MOD: wifi/mgmt/mqm.c:0x6a2 <<<
// ===== FUN_ram_f0028ddc @ 0xf0028ddc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0028ddc(int param_1,uint param_2)

{
  bool bVar1;
  int iVar2;
  
  iVar2 = FUN_ram_f0025d90(*(undefined1 *)(param_1 + 2));
  if (iVar2 == 0) {
    return;
  }
  if ((uint)*(byte *)(param_1 + 1) == (param_2 & 0xff)) {
    return;
  }
  if (*(byte *)(param_1 + 1) == 0) {
    *(int *)(iVar2 + (*(byte *)(param_1 + 3) + 0x44) * 4 + 4) = param_1;
    *(char *)((int)_DAT_ram_f0064f34 + 0xf) = *(char *)((int)_DAT_ram_f0064f34 + 0xf) + '\x01';
    if (8 < *(byte *)((int)_DAT_ram_f0064f34 + 0xf)) {
      (*_DAT_ram_00014800)(0,0,0xf00634e8,0x6a2,_DAT_ram_00014800);
    }
    *(undefined1 *)(param_1 + 6) = 0;
    bVar1 = true;
  }
  else {
    bVar1 = false;
  }
  if (param_2 == 0) {
    *(undefined4 *)(iVar2 + (*(byte *)(param_1 + 3) + 0x44) * 4 + 4) = 0;
    *(char *)((int)_DAT_ram_f0064f34 + 0xf) = *(char *)((int)_DAT_ram_f0064f34 + 0xf) + -1;
    if ((*(char *)((int)_DAT_ram_f0064f34 + 0xf) == '\0') && ((*_DAT_ram_f0064f34 & 4) != 0)) {
      FUN_ram_f00266d8(_DAT_ram_f0064f34 + 0x62);
      *_DAT_ram_f0064f34 = *_DAT_ram_f0064f34 & 0xfffffffb;
    }
  }
  else {
    if ((param_2 == 2) && ((*_DAT_ram_f0064f34 & 4) == 0)) {
      FUN_ram_f00266a0(_DAT_ram_f0064f34 + 0x62,&LAB_ram_f0028f6c);
      FUN_ram_f00266d8(_DAT_ram_f0064f34 + 0x62);
    }
    if (!bVar1) goto LAB_ram_f0028ef4;
  }
  (*_DAT_ram_00014ab4)(0,*(undefined1 *)(param_1 + 3),iVar2 + 10,_DAT_ram_00014ab4);
LAB_ram_f0028ef4:
  *(char *)(param_1 + 1) = (char)param_2;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0028f00 @ 0xf0028f00 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0028f00(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar3 = 0;
  do {
    iVar4 = _DAT_ram_f0064f34 + (iVar3 + 0x26) * 8;
    iVar3 = iVar3 + 1;
    if (((*(char *)(iVar4 + 1) == '\x02') &&
        (iVar1 = FUN_ram_f0025d90(*(undefined1 *)(iVar4 + 2)), iVar1 != 0)) &&
       (iVar2 = FUN_ram_f0028750(), iVar2 != 0)) {
      FUN_ram_f0028ddc(iVar4,3);
      FUN_ram_f0028b5c(0,iVar1,*(undefined1 *)(iVar4 + 3),1);
    }
  } while (iVar3 != 8);
  return 0;
}

// >>> MOD: wifi/mgmt/mqm.c:0x75c <<<
// ===== FUN_ram_f00291ec @ 0xf00291ec =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f00291ec(undefined1 param_1,undefined1 param_2,int param_3)

{
  char cVar1;
  char *pcVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  
  if ((param_3 == 0) || (param_3 == 3)) {
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0x75c,_DAT_ram_00014800);
  }
  if (*(byte *)(_DAT_ram_f0064f34 + 0xf) < 8) {
    FUN_ram_f0025d90(param_1);
    uVar3 = 0;
    pcVar2 = (char *)(_DAT_ram_f0064f34 + 0x131);
    do {
      cVar1 = *pcVar2;
      pcVar2 = pcVar2 + 8;
      if (cVar1 == '\0') {
        iVar4 = (uVar3 + 0x26) * 8;
        *(undefined1 *)(_DAT_ram_f0064f34 + iVar4 + 2) = param_1;
        *(undefined1 *)(_DAT_ram_f0064f34 + iVar4 + 3) = param_2;
        iVar5 = ((uVar3 & 0xff) + 0x26) * 8;
        *(undefined2 *)(_DAT_ram_f0064f34 + iVar4 + 4) = 0xffff;
        FUN_ram_f0028ddc(_DAT_ram_f0064f34 + iVar5,param_3);
        return _DAT_ram_f0064f34 + iVar5;
      }
      uVar3 = uVar3 + 1;
    } while (uVar3 != 8);
  }
  return 0;
}

// >>> MOD: wifi/mgmt/mqm.c:0xa3f,0xaaa <<<
// ===== FUN_ram_f002929c @ 0xf002929c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002929c(int param_1)

{
  bool bVar1;
  undefined1 uVar2;
  bool bVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  undefined2 *puVar10;
  int iStack_48;
  undefined2 local_30;
  ushort local_2e;
  undefined2 local_2c;
  ushort local_28;
  undefined2 local_26;
  undefined2 local_24;
  
  iVar9 = *(int *)(param_1 + 0x20);
  iVar4 = FUN_ram_f0028750(iVar9);
  if (iVar4 == 0) goto LAB_ram_f002956a;
  if ((*(char *)(_DAT_ram_020a0064 + 0x20a7) == '\0') ||
     (*(char *)(_DAT_ram_020a0064 + 0x20a9) == '\0')) {
    bVar1 = false;
  }
  else {
    bVar1 = *(char *)(iVar9 + 0x28c) != '\0';
  }
  iVar4 = *(int *)(param_1 + 0x14);
  (*(code *)&SUB_ram_0006a08c)(&local_28,iVar4 + 0x1b,6);
  bVar3 = false;
  uVar7 = (local_28 & 0x3c) >> 2;
  if ((local_28 >> 1 & 1) != 1) {
    bVar1 = false;
  }
  uVar2 = *(undefined1 *)(iVar9 + 8);
  FUN_ram_f003f498(iVar9,3);
  if (bVar1) {
    iStack_48 = *(int *)((uVar7 + 0x44) * 4 + iVar9 + 4);
    if (iStack_48 == 0) {
      if ((*(byte *)(_DAT_ram_f0064f34 + 0xf) < 8) &&
         (iStack_48 = FUN_ram_f00291ec(uVar2,uVar7,1), iStack_48 != 0)) {
        bVar1 = true;
        bVar3 = true;
      }
      else {
        bVar3 = false;
        bVar1 = false;
      }
    }
    else {
      if (*(char *)(iStack_48 + 1) != '\x02') goto LAB_ram_f002956a;
      bVar1 = true;
    }
  }
  else {
    bVar3 = false;
    bVar1 = false;
    iStack_48 = 0;
  }
  iVar5 = FUN_ram_f0025c48(0x21);
  if (iVar5 == 0) {
    if (!bVar3) goto LAB_ram_f002956a;
    if (iStack_48 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf00634e8,0xa3f,_DAT_ram_00014800);
    }
  }
  else {
    local_30 = 0;
    puVar10 = *(undefined2 **)(iVar5 + 8);
    *puVar10 = 0xd0;
    *(undefined1 *)(puVar10 + 0xc) = 3;
    *(undefined1 *)((int)puVar10 + 0x19) = 1;
    puVar10[1] = 0;
    *(undefined1 *)(puVar10 + 0xd) = *(undefined1 *)(iVar4 + 0x1a);
    if (!bVar1) {
      local_30 = 0x25;
    }
    uVar8 = (uint)(local_28 >> 6);
    if (7 < uVar8 - 1) {
      uVar8 = 8;
    }
    uVar6 = FUN_ram_f0020b14();
    if (uVar8 < uVar6) {
      uVar6 = uVar8;
    }
    local_2e = (ushort)(uVar7 << 2) | 2 | (ushort)(uVar6 << 6);
    if (*(char *)(iVar9 + 0x361) == '\0') {
      DAT_ram_f006e9c5 = (undefined1)uVar6;
    }
    local_2c = local_26;
    (*(code *)&SUB_ram_0006a08c)((int)puVar10 + 0x1b,&local_30,6);
    (*(code *)&SUB_ram_0006a08c)(puVar10 + 2,iVar9 + 10,6);
    (*(code *)&SUB_ram_0006a08c)
              (puVar10 + 5,*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar9 + 0x14) * 4) + 0x51,6)
    ;
    (*(code *)&SUB_ram_0006a08c)(puVar10 + 8,iVar4 + 0x10,6);
    FUN_ram_f0034540(iVar5,iVar9,*(undefined1 *)(iVar9 + 0x14),puVar10,0x18,puVar10 + 0xc,9,
                     &LAB_ram_f0029140,0,0,0,0,0);
    *(char *)(iVar5 + 0xf) = (char)uVar7;
    FUN_ram_f0037b58(iVar5);
    if (!bVar3) goto LAB_ram_f002956a;
    if (!bVar1) {
      (*_DAT_ram_00014800)(0,0,0xf00634e8,0xaaa,_DAT_ram_00014800);
    }
    iVar9 = FUN_ram_f0025c48(0x10);
    if (iVar9 != 0) {
      puVar10 = *(undefined2 **)(iVar9 + 8);
      *(undefined1 *)(puVar10 + 4) = uVar2;
      *puVar10 = 0x10;
      *(undefined1 *)(puVar10 + 2) = 0x11;
      *(undefined1 *)((int)puVar10 + 5) = 0;
      puVar10[5] = local_2e;
      puVar10[7] = local_24;
      puVar10[6] = local_26;
      *(undefined1 *)((int)puVar10 + 9) = *(undefined1 *)(iVar4 + 0x1a);
      *(undefined1 *)(iVar9 + 0x2f) = 0;
      *(undefined2 **)(iVar9 + 0x28) = puVar10;
      puVar10[1] = 1;
      FUN_ram_f0039750();
      goto LAB_ram_f002956a;
    }
  }
  FUN_ram_f0028ddc(iStack_48,0);
LAB_ram_f002956a:
  (*(code *)&SUB_ram_0006a0d8)(param_1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0029580 @ 0xf0029580 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0029580(void)

{
  int iVar1;
  int iVar2;
  
  *_DAT_ram_f0064f34 = 0;
  *(undefined2 *)(_DAT_ram_f0064f34 + 1) = 0;
  *(undefined2 *)((int)_DAT_ram_f0064f34 + 6) = 0;
  *(undefined2 *)(_DAT_ram_f0064f34 + 2) = 0;
  *(undefined2 *)((int)_DAT_ram_f0064f34 + 10) = 0;
  *(undefined2 *)(_DAT_ram_f0064f34 + 3) = 0;
  *(undefined1 *)((int)_DAT_ram_f0064f34 + 0xe) = 0;
  iVar1 = 0;
  do {
    *(char *)(_DAT_ram_f0064f34 + iVar1 * 9 + 4) = (char)iVar1;
    iVar2 = iVar1 + 1;
    *(undefined1 *)((int)_DAT_ram_f0064f34 + iVar1 * 0x24 + 0x11) = 0;
    FUN_ram_f00266a0(_DAT_ram_f0064f34 + iVar1 * 9 + 6,&LAB_ram_f00299d8,
                     _DAT_ram_f0064f34 + iVar1 * 9 + 4);
    iVar1 = iVar2;
  } while (iVar2 != 8);
  iVar1 = 0;
  *(undefined1 *)((int)_DAT_ram_f0064f34 + 0xf) = 0;
  do {
    iVar2 = iVar1 + 0x26;
    *(char *)(_DAT_ram_f0064f34 + iVar2 * 2) = (char)iVar1;
    iVar1 = iVar1 + 1;
    *(undefined1 *)((int)_DAT_ram_f0064f34 + iVar2 * 8 + 1) = 0;
  } while (iVar1 != 8);
  FUN_ram_f0028ad0();
  return;
}

// >>> MOD: wifi/mgmt/mqm.c:0x5f7,0x60c <<<
// ===== FUN_ram_f0029634 @ 0xf0029634 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0029634(undefined1 *param_1,uint param_2)

{
  char cVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = FUN_ram_f0025d90(param_1[2]);
  if (iVar2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0x5f7,_DAT_ram_00014800);
  }
  iVar3 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar2 + 0x14) * 4);
  if ((uint)(byte)param_1[1] == (param_2 & 0xff)) {
    return;
  }
  if ((byte)param_1[1] == 0) {
    *(undefined1 **)(((byte)param_1[3] + 0x54) * 4 + iVar2 + 4) = param_1;
    *(char *)((int)_DAT_ram_f0064f34 + 0xe) = *(char *)((int)_DAT_ram_f0064f34 + 0xe) + '\x01';
    if (8 < *(byte *)((int)_DAT_ram_f0064f34 + 0xe)) {
      (*_DAT_ram_00014800)(0,0,0xf00634e8,0x60c,_DAT_ram_00014800);
    }
    param_1[0x22] = 0;
  }
  if (param_2 == 0) {
    *(undefined4 *)(iVar2 + ((byte)param_1[3] + 0x54) * 4 + 4) = 0;
    *(char *)((int)_DAT_ram_f0064f34 + 0xe) = *(char *)((int)_DAT_ram_f0064f34 + 0xe) + -1;
    (*_DAT_ram_0001490c)(*param_1,0,_DAT_ram_0001490c);
    if ((*(char *)((int)_DAT_ram_f0064f34 + 0xe) == '\0') && ((*_DAT_ram_f0064f34 & 2) != 0)) {
      FUN_ram_f00266d8(_DAT_ram_f0064f34 + 0x5c);
      *_DAT_ram_f0064f34 = *_DAT_ram_f0064f34 & 0xfffffffd;
    }
    if (*(char *)(iVar3 + 0x364) == '\0') goto LAB_ram_f00297a4;
    cVar1 = *(char *)(iVar3 + 0x364) + -1;
  }
  else {
    if (param_2 != 2) goto LAB_ram_f00297a4;
    (*_DAT_ram_0001490c)(*param_1,1,_DAT_ram_0001490c);
    if ((*_DAT_ram_f0064f34 & 2) == 0) {
      FUN_ram_f00266a0(_DAT_ram_f0064f34 + 0x5c,&LAB_ram_f00298cc);
      FUN_ram_f00266d8(_DAT_ram_f0064f34 + 0x5c);
    }
    cVar1 = *(char *)(iVar3 + 0x364) + '\x01';
  }
  *(char *)(iVar3 + 0x364) = cVar1;
  if ((_DAT_ram_f00650a0 + 0x25c != *(int *)(_DAT_ram_f00650a0 + 0x25c)) &&
     (*(int *)(_DAT_ram_f00650a0 + 0x25c) != 0)) {
    FUN_ram_f003b8dc();
  }
LAB_ram_f00297a4:
  param_1[1] = (char)param_2;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00297b0 @ 0xf00297b0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f00297b0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  if ((param_1 == 0) && (param_2 != 0)) {
    iVar3 = 0;
    do {
      iVar1 = _DAT_ram_f0064f34;
      iVar5 = iVar3 * 0x24;
      iVar4 = _DAT_ram_f0064f34 + iVar5;
      iVar3 = iVar3 + 1;
      if ((*(char *)(iVar4 + 0x11) == '\x02') &&
         (iVar2 = FUN_ram_f0025d90(*(undefined1 *)(iVar4 + 0x12)), iVar2 == param_2)) {
        FUN_ram_f0029634(iVar5 + 0x10 + iVar1,0);
        FUN_ram_f0028b5c(1,param_2,*(undefined1 *)(iVar4 + 0x13),0x27);
      }
    } while (iVar3 != 8);
  }
  return 0;
}

// >>> MOD: wifi/mgmt/mqm.c:0x108e <<<
// ===== FUN_ram_f0029828 @ 0xf0029828 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0029828(int param_1)

{
  int *piVar1;
  char cVar2;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0x108e,_DAT_ram_00014800);
  }
  if ((*(byte *)(param_1 + 0x17) & 0x10) != 0) {
    *(undefined2 *)(param_1 + 0x194) = 0;
    piVar1 = (int *)(param_1 + 0x114);
    cVar2 = '\0';
    do {
      if (*piVar1 != 0) {
        FUN_ram_f0028ddc(*piVar1,0);
      }
      if (piVar1[0x10] != 0) {
        FUN_ram_f0029634(piVar1[0x10],0);
      }
      cVar2 = cVar2 + '\x01';
      piVar1 = piVar1 + 1;
    } while (cVar2 != '\x10');
  }
  if ((*(char *)(param_1 + 0x2a8) != '\0') && (*(int *)(param_1 + 0x10) == 0x41)) {
    if ((*_DAT_ram_f0065118 == '\0') || ((*(uint *)(_DAT_ram_f0065118 + 0x28) & 1) == 0)) {
      FUN_ram_f0028ad0();
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/mqm.c:0xaf5 <<<
// ===== FUN_ram_f0029a40 @ 0xf0029a40 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0029a40(int param_1)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0xaf5,_DAT_ram_00014800);
  }
  iVar4 = *(int *)(param_1 + 0x20);
  iVar2 = FUN_ram_f0028750(iVar4);
  if (iVar2 != 0) {
    uVar1 = *(ushort *)(*(int *)(param_1 + 0x14) + 0x1a);
    uVar3 = (uint)(uVar1 >> 0xc);
    if ((uVar1 & 0x800) == 0) {
      iVar2 = *(int *)(iVar4 + (uVar3 + 0x54) * 4 + 4);
      if (iVar2 != 0) {
        if (*(char *)(iVar2 + 1) == '\x01') {
          FUN_ram_f00266d8(iVar2 + 8);
        }
        FUN_ram_f0029634(iVar2,0);
      }
    }
    else {
      iVar2 = *(int *)(iVar4 + (uVar3 + 0x44) * 4 + 4);
      if (iVar2 != 0) {
        FUN_ram_f0028c5c(*(undefined1 *)(iVar4 + 8));
        FUN_ram_f0028ddc(iVar2,0);
      }
    }
  }
  (*(code *)&SUB_ram_0006a0d8)(param_1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0029af0 @ 0xf0029af0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0029af0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  undefined1 *puVar5;
  uint uVar6;
  undefined1 local_38;
  undefined1 local_37;
  char local_36;
  undefined2 local_34;
  undefined2 local_32;
  uint local_30;
  uint local_2c;
  ushort local_28;
  ushort local_26;
  undefined2 local_24;
  
  iVar4 = *(int *)(param_1 + 0x20);
  iVar1 = FUN_ram_f0028750(iVar4);
  if (iVar1 == 0) goto LAB_ram_f0029cf2;
  iVar1 = *(int *)(param_1 + 0x14);
  (*(code *)&SUB_ram_0006a08c)(&local_28,iVar1 + 0x1b,6);
  puVar5 = *(undefined1 **)(iVar4 + (local_26 & 0x3c) + 0x154);
  uVar6 = (local_26 & 0x3c) >> 2;
  if (puVar5 == (undefined1 *)0x0) {
    if (local_28 != 0) goto LAB_ram_f0029cf2;
    uVar2 = 0x26;
  }
  else {
    if (puVar5[1] == '\x01') {
      if (puVar5[4] == *(char *)(iVar1 + 0x1a)) {
        uVar3 = (uint)local_28;
        if (uVar3 == 0) {
          FUN_ram_f003f498(iVar4,4,uVar6);
          local_38 = (undefined1)uVar6;
          local_36 = (char)(local_26 >> 6);
          local_37 = *(undefined1 *)(iVar4 + 0x28a);
          if (local_36 == '\0') {
            FUN_ram_f0029634(puVar5,0);
            *(undefined1 *)(iVar4 + 0x28b) = 0;
            FUN_ram_f00266d8(iVar4 + 0x290);
            FUN_ram_f002672c(iVar4 + 0x290,180000);
          }
          else {
            local_34 = local_24;
            local_32 = *(undefined2 *)(iVar4 + (uVar6 + 0xdc) * 2);
            local_30 = uVar3;
            local_2c = uVar3;
            (*_DAT_ram_00014910)(*puVar5,&local_38,_DAT_ram_00014910);
            if (*(char *)(iVar4 + 0x361) == '\0') {
              DAT_ram_f006e8f5 = local_36;
            }
            *(undefined2 *)(puVar5 + 0x20) = *(undefined2 *)(iVar4 + (uVar6 + 0xdc) * 2);
            FUN_ram_f0029634(puVar5,2);
            FUN_ram_f0028ca8(iVar4,uVar6,*(undefined2 *)(iVar4 + (uVar6 + 0xdc) * 2));
          }
        }
        else {
          FUN_ram_f003f498(iVar4,5,uVar6);
          FUN_ram_f0029634(puVar5,0);
          *(undefined1 *)(iVar4 + 0x28b) = 0;
          FUN_ram_f00266d8(iVar4 + 0x290);
          FUN_ram_f002672c(iVar4 + 0x290,180000);
          if (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar4 + 0x14) * 4) + 0x2d4) == 3
             ) {
            FUN_ram_f00288e8();
          }
        }
        FUN_ram_f00266d8(puVar5 + 8);
      }
      goto LAB_ram_f0029cf2;
    }
    if (local_28 == 0) {
      FUN_ram_f0028ca8(iVar4,uVar6,*(undefined2 *)(iVar4 + (uVar6 + 0xdc) * 2));
      goto LAB_ram_f0029cf2;
    }
    FUN_ram_f0029634(puVar5,0);
    uVar2 = 1;
  }
  FUN_ram_f0028b5c(1,iVar4,uVar6,uVar2);
LAB_ram_f0029cf2:
  (*(code *)&SUB_ram_0006a0d8)(param_1);
  return;
}

// >>> MOD: wifi/mgmt/mqm.c:0x54f <<<
// ===== FUN_ram_f0029d08 @ 0xf0029d08 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0029d08(int param_1)

{
  char cVar1;
  int iVar2;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0x54f,_DAT_ram_00014800);
  }
  if ((*(int *)(param_1 + 0x20) != 0) && (iVar2 = FUN_ram_f0028750(), iVar2 != 0)) {
    cVar1 = *(char *)(*(int *)(param_1 + 0x14) + 0x19);
    if (cVar1 == '\x01') {
      FUN_ram_f0029af0(param_1);
      return;
    }
    if (cVar1 == '\0') {
      FUN_ram_f002929c(param_1);
      return;
    }
    if (cVar1 == '\x02') {
      FUN_ram_f0029a40(param_1);
      return;
    }
  }
  (*(code *)&SUB_ram_0006a0d8)(param_1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0029d8c @ 0xf0029d8c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f0029d8c(undefined1 param_1,undefined1 param_2,undefined2 param_3,undefined4 param_4)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  char *pcVar4;
  int iVar5;
  
  if (*(byte *)(_DAT_ram_f0064f34 + 0xe) < 8) {
    FUN_ram_f0025d90(param_1);
    uVar2 = 0;
    pcVar4 = (char *)(_DAT_ram_f0064f34 + 0x11);
    do {
      cVar1 = *pcVar4;
      pcVar4 = pcVar4 + 0x24;
      if (cVar1 == '\0') {
        iVar3 = uVar2 * 0x24;
        *(undefined1 *)(_DAT_ram_f0064f34 + iVar3 + 0x12) = param_1;
        iVar5 = (uVar2 & 0xff) * 0x24 + 0x10;
        *(undefined1 *)(_DAT_ram_f0064f34 + iVar3 + 0x13) = param_2;
        *(undefined2 *)(_DAT_ram_f0064f34 + iVar3 + 0x30) = param_3;
        FUN_ram_f0029634(_DAT_ram_f0064f34 + iVar5,param_4);
        return _DAT_ram_f0064f34 + iVar5;
      }
      uVar2 = uVar2 + 1;
    } while (uVar2 != 8);
  }
  return 0;
}

// >>> MOD: wifi/mgmt/mqm.c:0x7d1 <<<
// ===== FUN_ram_f0029e24 @ 0xf0029e24 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0029e24(int param_1,byte param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined2 *puVar4;
  uint uVar5;
  ushort local_28 [6];
  
  uVar5 = (uint)param_2;
  if ((((*(char *)(_DAT_ram_020a0064 + 0x20a7) != '\0') &&
       (*(char *)(_DAT_ram_020a0064 + 0x20a8) != '\0')) && (iVar1 = FUN_ram_f0028750(), iVar1 != 0))
     && ((iVar1 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4),
         *(int *)(param_1 + (uVar5 + 0x54) * 4 + 4) == 0 && (*(byte *)(_DAT_ram_f0064f34 + 0xe) < 8)
         ))) {
    iVar2 = FUN_ram_f0029d8c(*(undefined1 *)(param_1 + 8),uVar5,
                             *(undefined2 *)(param_1 + (uVar5 + 0xdc) * 2),1);
    if (iVar2 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf00634e8,0x7d1,_DAT_ram_00014800);
    }
    FUN_ram_f0028718(iVar2);
    iVar3 = FUN_ram_f0025c48(0x21);
    if (iVar3 == 0) {
      FUN_ram_f0029634(iVar2,0);
    }
    else {
      puVar4 = *(undefined2 **)(iVar3 + 8);
      *puVar4 = 0xd0;
      puVar4[1] = 0;
      *(undefined1 *)((int)puVar4 + 0x19) = 0;
      *(undefined1 *)(puVar4 + 0xc) = 3;
      *(undefined1 *)(puVar4 + 0xd) = *(undefined1 *)(iVar2 + 4);
      local_28[0] = (ushort)param_2 << 2 | 0x1002;
      local_28[2] = *(short *)(param_1 + (uVar5 + 0xdc) * 2) << 4;
      local_28[1] = 0;
      (*(code *)&SUB_ram_0006a08c)((int)puVar4 + 0x1b,local_28,6);
      (*(code *)&SUB_ram_0006a08c)(puVar4 + 2,param_1 + 10,6);
      (*(code *)&SUB_ram_0006a08c)(puVar4 + 5,iVar1 + 0x51,6);
      (*(code *)&SUB_ram_0006a08c)(puVar4 + 8,iVar1 + 0x4b,6);
      FUN_ram_f0034540(iVar3,param_1,*(undefined1 *)(param_1 + 0x14),puVar4,0x18,puVar4 + 0xc,9,
                       &LAB_ram_f002873c,0,0,0,0,0);
      FUN_ram_f0037b58(iVar3);
      FUN_ram_f00266d8(iVar2 + 8);
      FUN_ram_f002672c(iVar2 + 8,5000);
      FUN_ram_f003f498(param_1,0,uVar5);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/mqm.c:0x594,0x595 <<<
// ===== FUN_ram_f0029fd8 @ 0xf0029fd8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0029fd8(int param_1)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0x594,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x14) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0x595,_DAT_ram_00014800);
  }
  uVar3 = *(uint *)(param_1 + 0x14);
  if (uVar3 != 0) {
    iVar4 = *(int *)(param_1 + 0x20);
    uVar1 = *(ushort *)(uVar3 + 0x10);
    iVar2 = FUN_ram_f0028750(iVar4);
    if (iVar2 != 0) {
      if (*(int *)(((uVar1 >> 0xc) + 0x44) * 4 + iVar4 + 4) != 0) {
        *(undefined1 *)(param_1 + 0x3c) = 1;
        *(undefined1 *)(param_1 + 0x10) = 1;
        *(undefined1 *)(param_1 + 0x3b) = 0;
        *(undefined1 *)(param_1 + 0x3d) = 0;
        if ((uVar3 & 3) == 0) {
          iVar2 = 0;
        }
        else {
          iVar2 = 2;
        }
        *(undefined2 *)(param_1 + 0x30) = *(undefined2 *)(uVar3 + 0x12);
        *(uint *)(param_1 + 0x28) = (uVar3 - 0xc) - iVar2;
        *(ushort *)(param_1 + 0x2c) =
             (short)iVar2 + *(short *)(param_1 + 0x12) + 0xc + (ushort)*(byte *)(param_1 + 0x11);
        *(undefined1 *)(param_1 + 0x42) = 0;
        *(undefined1 *)(param_1 + 0x2f) = 0;
        *(char *)(param_1 + 0x2e) = (char)iVar2;
        *(byte *)(param_1 + 0xf) = (byte)((ushort)*(undefined2 *)(uVar3 + 0x10) >> 0xc);
        FUN_ram_f00387e0(param_1);
        FUN_ram_f00387ac(param_1);
        FUN_ram_f00388e8(param_1);
        return;
      }
      FUN_ram_f0028b5c(0,iVar4,*(undefined1 *)(param_1 + 0xf),0x26);
    }
    (*(code *)&SUB_ram_0006a0d8)(param_1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002a0ec @ 0xf002a0ec =====


undefined4 FUN_ram_f002a0ec(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_ram_f003e5c8(0);
  if (iVar1 == 0) {
    uVar2 = 1;
  }
  else {
    uVar2 = 2;
  }
  return uVar2;
}

// >>> MOD: wifi/mgmt/mqm.c:0x3a8 <<<
// ===== FUN_ram_f002a114 @ 0xf002a114 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f002a114(int param_1)

{
  int iVar1;
  ushort uVar2;
  
  if (*(int *)(param_1 + 0x14) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00634e8,0x3a8,_DAT_ram_00014800);
  }
  if ((((*(int *)(param_1 + 0x20) == 0) || (*(char *)(*(int *)(param_1 + 0x20) + 0x38) == '\0')) ||
      (*(char *)(param_1 + 0x24) < '\0')) ||
     ((*(ushort **)(param_1 + 0x14) != (ushort *)0x0 &&
      ((**(ushort **)(param_1 + 0x14) & 0xfc) == 0x50)))) {
    iVar1 = 2;
  }
  else {
    iVar1 = 1;
  }
  if ((*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0x1c) == 1) &&
     (iVar1 == 2)) {
    if ((**(ushort **)(param_1 + 0x14) & 0xc) == 0) {
      uVar2 = **(ushort **)(param_1 + 0x14) & 0xfc;
      if ((uVar2 == 0x50) || (uVar2 == 0x80)) {
        iVar1 = 0;
      }
    }
    else {
      iVar1 = FUN_ram_f002a0ec(param_1);
    }
  }
  return iVar1;
}

// >>> MOD: wifi/mgmt/p2p_fsm.c:0x196,0x197 <<<
// ===== FUN_ram_f002a1ac @ 0xf002a1ac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002a1ac(int param_1,undefined1 *param_2)

{
  undefined4 uVar1;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063574,0x196,_DAT_ram_00014800);
  }
  if (param_2 == (undefined1 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063574,0x197,_DAT_ram_00014800);
  }
  if ((((*(byte *)(param_1 + 1) < 7) || (*(char *)(param_1 + 2) != 'P')) ||
      (*(char *)(param_1 + 3) != 'o')) || (*(char *)(param_1 + 4) != -0x66)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
    *param_2 = *(undefined1 *)(param_1 + 5);
  }
  return uVar1;
}

// >>> MOD: wifi/mgmt/p2p_fsm.c:0x1c9,0x1cb,0x1cc <<<
// ===== FUN_ram_f002a228 @ 0xf002a228 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002a228(char *param_1,ushort param_2,undefined4 *param_3,undefined2 *param_4)

{
  byte *pbVar1;
  bool bVar2;
  ushort *puVar3;
  undefined4 uVar4;
  char *pcVar5;
  uint uVar6;
  char *pcVar7;
  uint uVar8;
  uint uVar9;
  
  if ((param_2 != 0) && (param_1 == (char *)0x0)) {
    (*_DAT_ram_00014800)(0,0,0xf0063574,0x1c9,_DAT_ram_00014800);
  }
  if (param_3 == (undefined4 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063574,0x1cb,_DAT_ram_00014800);
  }
  if (param_4 == (undefined2 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063574,0x1cc,_DAT_ram_00014800);
  }
  uVar6 = 0;
  pcVar5 = (char *)(_DAT_ram_020a0064 + 0x167c);
  pcVar7 = (char *)0x0;
  bVar2 = false;
  for (uVar9 = 0; uVar9 < param_2; uVar9 = uVar9 + *pbVar1 + 2 & 0xffff) {
    if ((((*param_1 == -0x23) && (param_1[2] == 'P')) && (param_1[3] == 'o')) &&
       ((param_1[4] == -0x66 && (param_1[5] == '\t')))) {
      if (pcVar7 == (char *)0x0) {
        pcVar7 = param_1 + 6;
        if (4 < (byte)param_1[1]) {
          uVar6 = (byte)param_1[1] - 4 & 0xffff;
        }
      }
      else {
        if (!bVar2) {
          (*(code *)&SUB_ram_0006a08c)(pcVar5,pcVar7,uVar6);
          bVar2 = true;
          pcVar7 = pcVar5;
        }
        uVar8 = (byte)param_1[1] - 4 & 0xffff;
        if (0x300 < uVar6 + uVar8) {
          uVar8 = 0x300 - uVar6 & 0xffff;
        }
        if (uVar8 != 0) {
          (*(code *)&SUB_ram_0006a08c)(pcVar5 + uVar6,param_1 + 6,uVar8);
          uVar6 = uVar6 + uVar8 & 0xffff;
        }
      }
    }
    pbVar1 = (byte *)(param_1 + 1);
    param_1 = param_1 + *pbVar1 + 2;
  }
  if (pcVar7 == (char *)0x0) {
    *param_3 = 0;
    *param_4 = 0;
    uVar4 = 0xc0000001;
  }
  else {
    pcVar5 = pcVar7;
    for (uVar9 = 0; puVar3 = (ushort *)(pcVar5 + 1), uVar9 < uVar6;
        uVar9 = uVar9 + *puVar3 + 3 & 0xffff) {
      pcVar5 = pcVar5 + *puVar3 + 3;
    }
    *param_3 = pcVar7;
    uVar4 = 0;
    *param_4 = (short)uVar6;
  }
  return uVar4;
}

// >>> MOD: wifi/mgmt/p2p_fsm.c:0xe2 <<<
// ===== FUN_ram_f002a3a4 @ 0xf002a3a4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002a3a4(void)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 local_20;
  undefined2 local_1c;
  
  puVar2 = _DAT_ram_020a006c;
  iVar1 = _DAT_ram_020a0064;
  local_20 = 0;
  local_1c = 0;
  *(undefined1 *)((int)_DAT_ram_020a006c + 0x19) = 1;
  *puVar2 = 1;
  puVar2[1] = 1;
  puVar2[3] = 0;
  *(undefined1 *)(puVar2 + 6) = 0;
  *(undefined1 *)(puVar2 + 10) = 0;
  (*(code *)&SUB_ram_0006a08c)((int)puVar2 + 0x4b,&local_20,6);
  puVar2[0xd5] = puVar2 + 0xd4;
  puVar2[0xd4] = puVar2 + 0xd4;
  puVar2[0x18] = puVar2 + 0x17;
  puVar2[3] = 3;
  puVar2[0x17] = puVar2 + 0x17;
  puVar2[0x19] = 0;
  *(undefined1 *)((int)puVar2 + 0x7b) = 0;
  *(undefined1 *)((int)puVar2 + 0x349) = 0;
  puVar2[0xd6] = 0;
  *(undefined1 *)(iVar1 + 0x1676) = 0;
  *(undefined2 *)(iVar1 + 0x1672) = 0;
  *(undefined2 *)(iVar1 + 0x1670) = 0;
  *(undefined1 *)(iVar1 + 0x1677) = 0;
  *(undefined1 *)(iVar1 + 0x1674) = 0;
  *(undefined1 *)(iVar1 + 0x1675) = 0;
  iVar1 = _DAT_ram_f0064f94;
  if (_DAT_ram_f0064f94 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063574,0xe2,_DAT_ram_00014800);
  }
  (*(code *)&SUB_ram_0006a0a0)(iVar1,0,0x70);
  *(undefined4 *)(iVar1 + 8) = _DAT_ram_0209fef4;
  *(undefined1 *)(iVar1 + 0xc) = 4;
  puVar2[0x1f] = iVar1;
  return;
}

// >>> MOD: wifi/mgmt/privacy.c:0x191 <<<
// ===== FUN_ram_f002a480 @ 0xf002a480 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f002a480(void)

{
  int iVar1;
  
  iVar1 = _DAT_ram_020a0068;
  if (*(int *)(_DAT_ram_020a0068 + 0x14) != 1) {
    if (7 < *(uint *)(_DAT_ram_020a0068 + 0x10)) {
      (*_DAT_ram_00014800)(0,0,0xf0063590,0x191,_DAT_ram_00014800);
    }
    if (2 < *(uint *)(iVar1 + 0x10)) {
      return *(uint *)(iVar1 + 0x10) != 5;
    }
  }
  return false;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002a4cc @ 0xf002a4cc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f002a4cc(int param_1)

{
  uint uVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 0x20);
  if (iVar2 == 0) {
    if (*(char *)(param_1 + 0xd) == '\0') {
      return (bool)*(undefined1 *)(_DAT_ram_020a0064 + 0x12e3);
    }
    if (*(char *)(param_1 + 0xd) == '\x01') {
      return *(char *)(_DAT_ram_020a0064 + 0x197c) != '\0';
    }
  }
  else {
    uVar1 = (uint)*(byte *)(iVar2 + 0x14);
    if (*(char *)(param_1 + 0x28) == '\0') {
      if (*(char *)(iVar2 + 0x289) != '\0') {
        return true;
      }
      if ((uVar1 == 0) && (*(uint *)(*(int *)(&DAT_ram_020a0068 + uVar1 * 4) + 0x10) < 3)) {
        return *(char *)(_DAT_ram_020a0064 + 0x12e3) != '\0';
      }
    }
    else if (((uVar1 != 0) || (2 < *(uint *)(*(int *)(&DAT_ram_020a0068 + uVar1 * 4) + 0x10))) &&
            (*(char *)(iVar2 + 0x289) != '\0')) {
      return *(char *)(iVar2 + 0x211) == '\0';
    }
  }
  return false;
}

// >>> MOD: wifi/mgmt/privacy.c:0x504 <<<
// ===== FUN_ram_f002a554 @ 0xf002a554 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f002a554(int param_1)

{
  bool bVar1;
  int iVar2;
  
  iVar2 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (*(char *)(iVar2 + 0x35c) == '\0') {
    bVar1 = false;
    if (*(int *)(iVar2 + 0x14) != 1) {
      if (7 < *(uint *)(iVar2 + 0x10)) {
        (*_DAT_ram_00014800)(0,0,0xf0063590,0x504,_DAT_ram_00014800);
      }
      bVar1 = 2 < *(uint *)(iVar2 + 0x10);
    }
  }
  else {
    bVar1 = true;
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002a8e0 @ 0xf002a8e0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002a8e0(byte param_1,byte param_2)

{
  undefined4 uVar1;
  
  if (*(char *)(_DAT_ram_020a0064 + (param_2 + 0x417) * 8 + 4) == '\0') {
    uVar1 = 1;
  }
  else {
    *(undefined1 *)(_DAT_ram_020a0064 + (param_1 + 0x417) * 8 + 4) = 1;
    uVar1 = FUN_ram_f004b748();
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002abec @ 0xf002abec =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002abec(int param_1,uint param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined2 *puVar3;
  
  iVar2 = FUN_ram_f0025c48(0x14);
  if (iVar2 != 0) {
    puVar3 = *(undefined2 **)(iVar2 + 8);
    *(undefined1 *)(puVar3 + 2) = 7;
    *(undefined1 *)((int)puVar3 + 5) = 0;
    *(uint *)(puVar3 + 4) = param_2 & 0xff;
    uVar1 = _DAT_ram_f00635a8;
    if ((param_2 & 0xff) == 0) {
      (*(code *)&SUB_ram_0006a08c)(puVar3 + 6,param_1 + 10,6);
    }
    else {
      *(undefined4 *)(puVar3 + 6) = _DAT_ram_f00635a4;
      puVar3[8] = (short)uVar1;
    }
    *puVar3 = 0x14;
    *(undefined1 *)(iVar2 + 0x2f) = 0;
    *(undefined2 **)(iVar2 + 0x28) = puVar3;
    puVar3[1] = 1;
    FUN_ram_f0039750(iVar2);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002ac70 @ 0xf002ac70 =====


void FUN_ram_f002ac70(int param_1)

{
  if (*(short *)(param_1 + 0x50) != 0) {
    if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x18) != '\0')
    {
      if (*(char *)(param_1 + 0x2aa) != '\0') {
        if ((*(int *)(param_1 + 0x44) - *(int *)(param_1 + 0x4c) < 0) &&
           (*(char *)(param_1 + 0x3b) == '\0')) {
          if (*(char *)(param_1 + 0x3a) == '\x01') {
            *(undefined1 *)(param_1 + 0x3a) = 0;
          }
          *(undefined4 *)(param_1 + 0x4c) = *(undefined4 *)(param_1 + 0x44);
        }
        else if (((*(int *)(param_1 + 0x44) != 0) || (*(int *)(param_1 + 0x4c) != 0)) &&
                (*(char *)(param_1 + 0x3a) == '\0')) {
          *(undefined1 *)(param_1 + 0x3b) = 0;
          *(undefined1 *)(param_1 + 0x3a) = 1;
        }
      }
      if ((*(int *)(param_1 + 0x40) - *(int *)(param_1 + 0x48) < 0) &&
         (*(char *)(param_1 + 0x3c) == '\0')) {
        *(undefined1 *)(param_1 + 0x39) = 0;
      }
      else if (((*(int *)(param_1 + 0x40) != 0) || (*(int *)(param_1 + 0x48) != 0)) &&
              (*(char *)(param_1 + 0x39) == '\0')) {
        *(undefined1 *)(param_1 + 0x3c) = 0;
        *(undefined1 *)(param_1 + 0x39) = 1;
      }
    }
    *(undefined1 *)(param_1 + 0x51) = 0;
    *(undefined1 *)(param_1 + 0x50) = 0;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002ad0c @ 0xf002ad0c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002ad0c(void)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar2 = 0;
  uVar1 = 0;
  iVar3 = 0;
  do {
    iVar4 = *(int *)(&DAT_ram_020a0068 + iVar3 * 4);
    iVar2 = iVar2 + 1;
    iVar3 = iVar3 + 1;
    if ((*(char *)(iVar4 + 0x18) != '\0') && ((uVar1 == 0 || (uVar1 < *(ushort *)(iVar4 + 0x34a)))))
    {
      uVar1 = *(ushort *)(iVar4 + 0x34a);
    }
  } while (iVar2 != 3);
  (*_DAT_ram_00014c68)(uVar1,_DAT_ram_00014c68);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002ad54 @ 0xf002ad54 =====


byte FUN_ram_f002ad54(ushort param_1,byte param_2,byte param_3)

{
  byte bVar1;
  
  if ((((param_1 & 0xfc) == 200) || ((param_1 & 0xfc) == 0x88)) && ((param_1 & 0x1000) != 0)) {
    bVar1 = param_3 >> (*(byte *)(param_2 + 0xf0064920) & 0x1f) & 1;
  }
  else {
    bVar1 = 0;
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002ad8c @ 0xf002ad8c =====


byte FUN_ram_f002ad8c(byte param_1,byte param_2)

{
  return param_2 >> (*(byte *)(param_1 + 0xf0064920) & 0x1f) & 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002ada4 @ 0xf002ada4 =====


undefined1 FUN_ram_f002ada4(byte param_1)

{
  undefined1 uVar1;
  
  if (param_1 < 4) {
    uVar1 = *(undefined1 *)(param_1 + 0xf0063650);
  }
  else {
    uVar1 = 2;
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002adbc @ 0xf002adbc =====


ushort FUN_ram_f002adbc(int param_1)

{
  int iVar1;
  ushort *puVar2;
  ushort uVar3;
  
  iVar1 = *(int *)(param_1 + 0x20);
  if (iVar1 != 0) {
    puVar2 = *(ushort **)(param_1 + 0x14);
    if ((*(char *)(iVar1 + 0x15) == '\x02') && ((puVar2[2] & 1) == 0)) {
      if (*(char *)(iVar1 + 0x38) != '\0') {
        if (((((*(uint *)(iVar1 + 0x10) & 0x80) == 0) &&
             (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar1 + 0x14) * 4) + 0x9a) ==
              '\0')) || (*(char *)(iVar1 + 0x2aa) == '\0')) ||
           (((*puVar2 & 0xfc) != 200 && ((*puVar2 & 0xfc) != 0x88)))) {
          uVar3 = 0;
          *(undefined4 *)(iVar1 + 0x48) = *(undefined4 *)(param_1 + 0x30);
          *(undefined1 *)(iVar1 + 0x51) = 1;
        }
        else {
          uVar3 = puVar2[0xc] & 0x10;
          if ((puVar2[0xc] & 0x10) != 0) {
            uVar3 = 1;
            *(undefined4 *)(iVar1 + 0x4c) = *(undefined4 *)(param_1 + 0x30);
            *(undefined1 *)(iVar1 + 0x51) = 1;
          }
        }
        FUN_ram_f002ac70(iVar1);
        return uVar3;
      }
      *(undefined1 *)(iVar1 + 0x3a) = 0;
      *(undefined1 *)(iVar1 + 0x39) = 0;
      return 0;
    }
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002aef4 @ 0xf002aef4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f002aef4(void)

{
  bool bVar1;
  
  if (_DAT_ram_f0064bb0 == 0) {
    bVar1 = *(char *)(_DAT_ram_020a0064 + 0x1f74) == '\0';
  }
  else {
    bVar1 = false;
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002afbc @ 0xf002afbc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002afbc(void)

{
  bool bVar1;
  undefined4 uVar2;
  int iVar3;
  byte bVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int *piStack_24;
  
  if (*(char *)(_DAT_ram_020a0064 + 0x1f74) != '\x01') {
    piStack_24 = (int *)&DAT_ram_020a0068;
    bVar4 = 0;
    iVar7 = 0;
    bVar1 = false;
    do {
      iVar3 = *piStack_24;
      if (iVar7 != 2) {
        if (*(char *)(iVar3 + 0x18) == '\0') {
          *(undefined1 *)(iVar3 + 0x29b) = 0;
        }
        else if (((*(char *)(iVar3 + 0x29c) == '\0') || (*(int *)(iVar3 + 0xc) != 0)) ||
                (*(int *)(iVar3 + 0xa8) != 3)) goto LAB_ram_f002b082;
        iVar5 = iVar3 + 0x28b;
        for (uVar6 = 0; uVar6 < *(byte *)(iVar3 + 0x29b); uVar6 = uVar6 + 1) {
          if (3 < bVar4) {
            bVar1 = false;
            break;
          }
          (*_DAT_ram_00014b68)(bVar4,1,1,2,0,0,0xf,0x20,iVar5);
          iVar5 = iVar5 + 4;
          bVar1 = true;
          bVar4 = bVar4 + 1;
        }
      }
      iVar7 = iVar7 + 1;
      piStack_24 = piStack_24 + 1;
    } while (iVar7 != 3);
    if (bVar1) {
      uVar2 = 1;
    }
    else {
LAB_ram_f002b082:
      uVar2 = 0;
    }
    (*_DAT_ram_00014b6c)(uVar2,0,0,0,_DAT_ram_00014b6c);
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x202d <<<
// ===== FUN_ram_f002b140 @ 0xf002b140 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f002b140(int param_1)

{
  int iVar1;
  
  iVar1 = _DAT_ram_020a0064;
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x202d,_DAT_ram_00014800);
  }
  return *(char *)(iVar1 + 0x1ef4) == '\0' && *(char *)(iVar1 + 0x1edc) == '\0';
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2161 <<<
// ===== FUN_ram_f002b184 @ 0xf002b184 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002b184(int param_1)

{
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2161,_DAT_ram_00014800);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002b1b0 @ 0xf002b1b0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002b1b0(int param_1)

{
  ushort uVar1;
  uint uVar2;
  undefined4 uVar3;
  
  uVar1 = *(ushort *)(param_1 + 0x288);
  uVar2 = (*_DAT_ram_00014c00)(*(undefined1 *)(param_1 + 0x344),_DAT_ram_00014c00);
  if ((uVar1 == uVar2) || (uVar2 == 0)) {
    uVar3 = 0;
  }
  else {
    *(short *)(param_1 + 0x288) = (short)uVar2;
    uVar3 = 1;
  }
  return uVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002b1e4 @ 0xf002b1e4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f002b1e4(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar1 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  iVar2 = *(int *)(iVar1 + 0x20c);
  if (iVar2 == 0) {
    return false;
  }
  if (iVar2 == 1) {
    return true;
  }
  if (iVar2 == 2) {
    if (param_1 == 0) {
      if (*(int *)(iVar1 + 0xc) != 0) {
        return false;
      }
      return *(int *)(iVar1 + 0xac) == 3;
    }
    if (param_1 == 1) {
      if (*(int *)(iVar1 + 0xc) == 0) {
        return *(int *)(iVar1 + 0xac) == 3;
      }
      if (*(int *)(iVar1 + 0xc) != 2) {
        return false;
      }
      return *(char *)(_DAT_ram_020a0064 + 0x1f04) != '\0';
    }
    if (param_1 == 2) {
      return false;
    }
    uVar3 = 0x2955;
  }
  else {
    uVar3 = 0x2958;
  }
  (*_DAT_ram_00014800)(0,0,0xf00635bc,uVar3,_DAT_ram_00014800);
  return false;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002b290 @ 0xf002b290 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002b290(char param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  if (param_1 == '\0') {
    uVar1 = 0;
    uVar2 = 0;
    uVar3 = 0;
    uVar4 = 0;
  }
  else if (param_2 == 1) {
    uVar3 = 0x18;
    uVar4 = 0xe8;
    uVar2 = 0x960;
    uVar1 = 1;
  }
  else {
    uVar2 = 0x4b0;
    uVar3 = 8;
    uVar4 = 0xe8;
    uVar1 = 1;
  }
  (*_DAT_ram_00014e84)(uVar1,uVar2,uVar3,uVar4,_DAT_ram_00014e84);
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002b3d8 @ 0xf002b3d8 =====


void FUN_ram_f002b3d8(undefined1 param_1)

{
  int iVar1;
  undefined2 *puVar2;
  
  iVar1 = FUN_ram_f0025c48(0xc);
  if (iVar1 != 0) {
    puVar2 = *(undefined2 **)(iVar1 + 8);
    *(undefined1 *)(puVar2 + 2) = 0x2d;
    *(undefined1 *)((int)puVar2 + 5) = 0;
    *puVar2 = 0xc;
    *(undefined1 *)(puVar2 + 4) = param_1;
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar2;
    puVar2[1] = 1;
    FUN_ram_f0039750();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002b420 @ 0xf002b420 =====


void FUN_ram_f002b420(undefined1 param_1,undefined1 param_2)

{
  int iVar1;
  undefined2 *puVar2;
  
  iVar1 = FUN_ram_f0025c48(0xc);
  if (iVar1 != 0) {
    puVar2 = *(undefined2 **)(iVar1 + 8);
    *(undefined1 *)(puVar2 + 2) = 0x1b;
    *puVar2 = 0xc;
    *(undefined1 *)((int)puVar2 + 5) = 0;
    *(undefined1 *)((int)puVar2 + 9) = param_2;
    *(undefined1 *)(puVar2 + 4) = param_1;
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar2;
    puVar2[1] = 1;
    FUN_ram_f0039750();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002b494 @ 0xf002b494 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002b494(int param_1)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  byte *pbVar5;
  byte *pbVar6;
  uint uVar7;
  undefined4 local_28;
  undefined2 local_24;
  
  local_28 = _DAT_ram_f0063640;
  local_24 = (undefined2)_DAT_ram_f0063644;
  iVar4 = *(int *)(param_1 + 0x18);
  if ((((0x89 < *(ushort *)(param_1 + 0x12)) &&
       (iVar2 = (*(code *)&SUB_ram_0006a074)(iVar4,0xf0064950,6), iVar2 == 0)) &&
      (*(short *)(iVar4 + 6) == 8)) &&
     ((pbVar5 = (byte *)(iVar4 + 8), *pbVar5 >> 4 == 4 && (*(char *)(iVar4 + 0x11) == '\x11')))) {
    (*_DAT_ram_00014814)(2,0xf00635fc,_DAT_ram_00014814);
    uVar1 = *(ushort *)(param_1 + 0x12);
    iVar4 = (*pbVar5 & 0xf) * 4;
    uVar3 = iVar4 + 0x76;
    if (uVar3 <= uVar1) {
      pbVar5 = pbVar5 + iVar4 + 8;
      uVar7 = 0;
      while( true ) {
        iVar4 = (*(code *)&SUB_ram_0006a074)(pbVar5,&local_28,6);
        if (iVar4 == 0) {
          pbVar6 = pbVar5;
          while( true ) {
            pbVar6 = pbVar6 + 6;
            iVar4 = (*(code *)&SUB_ram_0006a074)(_DAT_ram_020a0068 + 0x51,pbVar6,6);
            if (iVar4 != 0) break;
            if (pbVar6 == pbVar5 + 0x60) {
              (*_DAT_ram_00014814)(2,0xf0063624,_DAT_ram_00014814);
              return 1;
            }
          }
        }
        uVar7 = uVar7 + 1;
        if (uVar1 - uVar3 < uVar7) break;
        pbVar5 = pbVar5 + 1;
      }
    }
  }
  return 0;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2bd7 <<<
// ===== FUN_ram_f002b584 @ 0xf002b584 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002b584(int param_1)

{
  ushort uVar1;
  short sVar2;
  bool bVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  
  iVar5 = _DAT_ram_020a0064;
  iVar8 = *(int *)(param_1 + 0x20);
  if (_DAT_ram_020a0064 == -0x1edc) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2bd7,_DAT_ram_00014800);
  }
  piVar4 = _DAT_ram_020a0068;
  if (((((*(char *)(iVar5 + 0x1f74) != '\0') && (iVar8 != 0)) && (*(char *)(param_1 + 0xd) == '\0'))
      && (*_DAT_ram_020a0068 == 0)) &&
     (((*(byte *)(iVar5 + 0x1f75) & 4) == 0 || (DAT_ram_f0064fbb == '\x01')))) {
    if (((*(byte *)(iVar5 + 0x1f75) & 1) == 0) || (iVar8 = FUN_ram_f002b494(param_1), iVar8 != 1)) {
      bVar3 = false;
    }
    else {
      FUN_ram_f002b3d8(0);
      bVar3 = true;
    }
    uVar1 = *(ushort *)(iVar5 + 0x1f76);
    uVar7 = uVar1 & 1;
    if (((uVar1 & 2) != 0) && (bVar3)) {
      uVar7 = 0;
    }
    if (((uVar1 & 0x20) != 0) &&
       ((sVar2 = *(short *)(*(int *)(param_1 + 0x18) + 6), sVar2 == -0x7178 || (sVar2 == -0x4b78))))
    {
      uVar7 = 0;
    }
    if ((uVar1 & 0x44) != 0) {
      iVar8 = *(int *)(param_1 + 0x18);
      iVar5 = (*(code *)&SUB_ram_0006a074)(iVar8,0xf0064928,0x10);
      if (iVar5 == 0) {
        uVar9 = uVar1 & 4;
        if ((uVar1 & 4) != 0) {
          return 0;
        }
        if ((char)piVar4[0xa7] != '\0') {
          iVar5 = (int)piVar4 + 0x28b;
          for (; uVar9 < *(byte *)((int)piVar4 + 0x29b); uVar9 = uVar9 + 1) {
            iVar6 = (*(code *)&SUB_ram_0006a074)(iVar8 + 0x20,iVar5,4);
            if (iVar6 == 0) {
              return 0;
            }
            iVar5 = iVar5 + 4;
          }
        }
      }
    }
    if (uVar7 == 1) {
      (*(code *)&SUB_ram_0006a0d8)(param_1);
      return 1;
    }
  }
  return 0;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x298c,0x298d,0x298e,0x29a8 <<<
// ===== FUN_ram_f002b6b8 @ 0xf002b6b8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002b6b8(ushort *param_1,int param_2,undefined4 param_3,undefined4 param_4,
                     undefined4 param_5)

{
  uint uVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  int iVar5;
  
  if (param_1 == (ushort *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x298c,_DAT_ram_00014800);
  }
  if (param_2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x298d,_DAT_ram_00014800);
  }
  if (2 < *(byte *)(param_2 + 0x14)) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x298e,_DAT_ram_00014800);
  }
  uVar1 = *(uint *)(param_2 + 0x10);
  if (*(char *)(param_2 + 0x2a8) == '\0') {
    uVar2 = 8;
  }
  else {
    uVar2 = 0x88;
  }
  iVar5 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_2 + 0x14) * 4);
  if ((uVar1 & 0x40) == 0) {
    if ((uVar1 & 0x20) == 0) {
      uVar3 = uVar2;
      if ((uVar1 & 0x100) == 0) {
        (*_DAT_ram_00014800)(0,0,0xf00635bc,0x29a8,_DAT_ram_00014800);
        return;
      }
    }
    else {
      uVar3 = uVar2 | 0x200;
    }
  }
  else {
    uVar3 = uVar2 | 0x100;
    if (*(char *)(param_2 + 0x37) != '\0') {
      uVar3 = uVar2 | 0x1100;
    }
  }
  *param_1 = uVar3;
  iVar4 = iVar5 + 0x51;
  (*(code *)&SUB_ram_0006a08c)(param_1 + 2,param_2 + 10,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 5,iVar4,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 8,param_3,6);
  param_1[0xb] = 0;
  if (*(char *)(param_2 + 0x2a8) == '\0') {
    (*(code *)&SUB_ram_0006a08c)(param_1 + 0xc,0xf0064938,0x10);
    (*(code *)&SUB_ram_0006a08c)(param_1 + 0x19,param_4,6);
    (*(code *)&SUB_ram_0006a08c)(param_1 + 0x1c,param_5,4);
    (*(code *)&SUB_ram_0006a08c)(param_1 + 0x14,iVar4,6);
    param_1 = param_1 + 0x17;
  }
  else {
    param_1[0xc] = 0;
    (*(code *)&SUB_ram_0006a08c)(param_1 + 0xd,0xf0064938,0x10);
    (*(code *)&SUB_ram_0006a08c)(param_1 + 0x1a,param_4,6);
    (*(code *)&SUB_ram_0006a08c)(param_1 + 0x1d,param_5,4);
    (*(code *)&SUB_ram_0006a08c)(param_1 + 0x15,iVar4,6);
    param_1 = param_1 + 0x18;
  }
  (*(code *)&SUB_ram_0006a08c)(param_1,iVar5 + 0x28b,4);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x240a,0x240b,0x2424 <<<
// ===== FUN_ram_f002b844 @ 0xf002b844 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002b844(undefined2 *param_1,int param_2,undefined1 param_3,undefined1 param_4)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  undefined *puVar4;
  int iVar5;
  short *in_stack_00000004;
  undefined2 local_24;
  undefined1 local_22;
  
  iVar2 = _DAT_ram_020a0064;
  local_22 = (undefined1)((uint)_DAT_ram_f0063648 >> 0x10);
  local_24 = (undefined2)_DAT_ram_f0063648;
  if (param_1 == (undefined2 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x240a,_DAT_ram_00014800);
  }
  if (param_2 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x240b,_DAT_ram_00014800);
  }
  uVar3 = *(uint *)(param_2 + 0x10);
  iVar5 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_2 + 0x14) * 4);
  if ((uVar3 & 0x40) == 0) {
    if ((uVar3 & 0x20) == 0) {
      if ((uVar3 & 0x100) == 0) {
        (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2424,_DAT_ram_00014800);
        return;
      }
      puVar4 = (undefined *)0xd0;
    }
    else {
      puVar4 = (undefined *)0x2d0;
    }
  }
  else if (*(char *)(param_2 + 0x37) == '\0') {
    puVar4 = &dimbr;
  }
  else {
    puVar4 = (undefined *)0x11d0;
  }
  *param_1 = puVar4;
  (*(code *)&SUB_ram_0006a08c)(param_1 + 2,param_2 + 10,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 5,iVar5 + 0x51,6);
  (*(code *)&SUB_ram_0006a08c)(param_1 + 8,iVar5 + 0x4b,6);
  *(undefined1 *)(param_1 + 0xc) = 0x7f;
  param_1[0xb] = 0;
  (*(code *)&SUB_ram_0006a08c)((int)param_1 + 0x19,&local_24,3);
  *(undefined1 *)(param_1 + 0xf) = param_3;
  *(undefined1 *)((int)param_1 + 0x1d) = 2;
  *(undefined1 *)(param_1 + 0xe) = 9;
  *in_stack_00000004 = 7;
  *(undefined1 *)((int)param_1 + 0x1f) = 0xdd;
  *(undefined1 *)(param_1 + 0x10) = 0xd;
  *(undefined1 *)(param_1 + 0x12) = 9;
  *(undefined1 *)((int)param_1 + 0x23) = local_22;
  *(undefined1 *)((int)param_1 + 0x21) = (undefined1)local_24;
  *(undefined1 *)(param_1 + 0x11) = local_24._1_1_;
  *in_stack_00000004 = *in_stack_00000004 + 6;
  *(undefined1 *)((int)param_1 + 0x25) = 0;
  *(undefined1 *)(param_1 + 0x14) = param_4;
  *(undefined1 *)(param_1 + 0x13) = 1;
  *(undefined1 *)((int)param_1 + 0x27) = 0;
  *in_stack_00000004 = *in_stack_00000004 + 4;
  *(undefined1 *)(param_1 + 0x15) = 2;
  *(undefined1 *)((int)param_1 + 0x29) = 0xc;
  *(undefined1 *)((int)param_1 + 0x2b) = 0;
  cVar1 = *(char *)(iVar2 + 0x1679);
  *(char *)(param_1 + 0x16) = cVar1;
  *(char *)(iVar2 + 0x1679) = cVar1 + '\x01';
  *(undefined1 *)((int)param_1 + 0x2d) = 0;
  *in_stack_00000004 = *in_stack_00000004 + 5;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002b9d4 @ 0xf002b9d4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002b9d4(ushort param_1,char *param_2)

{
  byte *pbVar1;
  byte bVar2;
  undefined4 uVar3;
  char *pcVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  
  if (param_1 < 0x259) {
    for (uVar8 = 0; uVar8 < param_1; uVar8 = uVar8 + *pbVar1 + 2 & 0xffff) {
      uVar7 = (uint)_DAT_ram_020a0094;
      pcVar4 = &DAT_ram_f0070238;
      for (uVar5 = 0; uVar5 < uVar7; uVar5 = uVar5 + *pbVar1 + 2 & 0xffff) {
        if (*param_2 == *pcVar4) {
          uVar6 = (uint)(byte)param_2[1];
          uVar9 = (uint)(byte)pcVar4[1];
          if (uVar6 == 0) {
            iVar10 = uVar9 + 2;
            (*(code *)&SUB_ram_0006a08c)(pcVar4,pcVar4 + iVar10,(uVar7 - uVar5) - iVar10);
            _DAT_ram_020a0094 = _DAT_ram_020a0094 - (short)iVar10;
            goto LAB_ram_f002ba6e;
          }
          if (uVar6 == uVar9) {
            (*(code *)&SUB_ram_0006a08c)(pcVar4,param_2,uVar6 + 2);
            goto LAB_ram_f002ba6e;
          }
          iVar10 = uVar9 + 2;
          (*(code *)&SUB_ram_0006a08c)(pcVar4,pcVar4 + iVar10,(uVar7 - uVar5) - iVar10);
          uVar5 = (uint)_DAT_ram_020a0094;
          _DAT_ram_020a0094 = (ushort)(uVar5 - iVar10);
          pcVar4 = &DAT_ram_f0070238 + (uVar5 - iVar10 & 0xffff);
          bVar2 = param_2[1];
          goto LAB_ram_f002baa0;
        }
        pbVar1 = (byte *)(pcVar4 + 1);
        pcVar4 = pcVar4 + *pbVar1 + 2;
      }
      bVar2 = param_2[1];
LAB_ram_f002baa0:
      (*(code *)&SUB_ram_0006a08c)(pcVar4,param_2,bVar2 + 2);
      _DAT_ram_020a0094 = _DAT_ram_020a0094 + 2 + (ushort)(byte)param_2[1];
LAB_ram_f002ba6e:
      pbVar1 = (byte *)(param_2 + 1);
      param_2 = param_2 + *pbVar1 + 2;
    }
    uVar3 = 1;
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002bb7c @ 0xf002bb7c =====


undefined4 FUN_ram_f002bb7c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar1 = FUN_ram_f0025c48(0x46);
  if (iVar1 == 0) {
    uVar2 = 0xc000009a;
  }
  else {
    FUN_ram_f002b6b8(*(int *)(iVar1 + 8) + 8,*(undefined4 *)(param_1 + 0x58),param_2,param_3,param_4
                    );
    iVar3 = 0x1a;
    if (*(char *)(*(int *)(param_1 + 0x58) + 0x2a8) == '\0') {
      iVar3 = 0x18;
    }
    FUN_ram_f0034540(iVar1,*(int *)(param_1 + 0x58),*(undefined1 *)(param_1 + 0x19),
                     *(int *)(iVar1 + 8) + 8,iVar3,*(int *)(iVar1 + 8) + iVar3 + 8,0x24,0,0,0,
                     *(undefined1 *)(param_1 + 0x349),7,0);
    FUN_ram_f0037b58(iVar1);
    uVar2 = 0;
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002bc24 @ 0xf002bc24 =====


undefined4 FUN_ram_f002bc24(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  undefined1 auStack_2c [8];
  undefined1 auStack_24 [8];
  
  iVar4 = *(int *)(param_1 + 0x18);
  iVar3 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(*(int *)(param_1 + 0x20) + 0x14) * 4);
  iVar1 = (*(code *)&SUB_ram_0006a074)(iVar4,0xf0064928,0x10);
  if ((iVar1 == 0) && (*(char *)(iVar3 + 0x29c) != '\0')) {
    iVar1 = iVar3 + 0x28b;
    for (uVar5 = 0; uVar5 < *(byte *)(iVar3 + 0x29b); uVar5 = uVar5 + 1) {
      iVar2 = (*(code *)&SUB_ram_0006a074)(iVar4 + 0x20,iVar1,4);
      if (iVar2 == 0) {
        (*(code *)&SUB_ram_0006a08c)(auStack_2c,iVar4 + 0x10,6);
        (*(code *)&SUB_ram_0006a08c)(auStack_24,*(int *)(param_1 + 0x18) + 0x16,4);
        FUN_ram_f002bb7c(iVar3,iVar4 + 0x10,auStack_2c,auStack_24);
        return 1;
      }
      iVar1 = iVar1 + 4;
    }
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002bcd4 @ 0xf002bcd4 =====


undefined4
FUN_ram_f002bcd4(int param_1,undefined1 param_2,undefined1 param_3,undefined4 param_4,
                undefined4 param_5,undefined4 param_6)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 local_22 [3];
  
  iVar1 = FUN_ram_f0025c48(0xa0);
  if (iVar1 == 0) {
    uVar2 = 0xc000009a;
  }
  else {
    FUN_ram_f002b844(*(int *)(iVar1 + 8) + 8,param_1,param_2,param_3,param_4,param_5,param_6,
                     local_22);
    FUN_ram_f0034540(iVar1,param_1,(uint)*(byte *)(param_1 + 0x14),*(int *)(iVar1 + 8) + 8,0x18,
                     *(int *)(iVar1 + 8) + 0x20,local_22[0],0,0,1,
                     *(undefined1 *)
                      (*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x349),7,
                     0);
    FUN_ram_f0037b58(iVar1);
    uVar2 = 0;
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002bd88 @ 0xf002bd88 =====


bool FUN_ram_f002bd88(void)

{
  int iVar1;
  
  iVar1 = FUN_ram_f0040e14();
  return iVar1 == 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002bda8 @ 0xf002bda8 =====


bool FUN_ram_f002bda8(void)

{
  int iVar1;
  
  iVar1 = FUN_ram_f0040ecc();
  return iVar1 == 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002bdc8 @ 0xf002bdc8 =====


void FUN_ram_f002bdc8(undefined1 param_1)

{
  FUN_ram_f003e614(param_1);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x21c3 <<<
// ===== FUN_ram_f002bde4 @ 0xf002bde4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002bde4(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  
  iVar2 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x21c3,_DAT_ram_00014800);
  }
  if (*(int *)(iVar2 + 0xc) == 2) {
    puVar1 = *(undefined4 **)(iVar2 + 0x5c);
    if (puVar1 != (undefined4 *)(iVar2 + 0x5c)) {
      for (; puVar1 != (undefined4 *)(iVar2 + 0x5c); puVar1 = (undefined4 *)*puVar1) {
        if (*(char *)(puVar1 + 0xe) != '\0') {
          *(undefined1 *)((int)puVar1 + 0x39) = 0;
          *(undefined1 *)((int)puVar1 + 0x3a) = 0;
        }
      }
    }
  }
  else {
    *(undefined1 *)(iVar2 + 0xb8) = 0;
    *(undefined1 *)(iVar2 + 0xb7) = 0;
    *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(iVar2 + 0x19) + 0x3d6) * 4 + 4) = 2;
    FUN_ram_f002bdc8(*(undefined1 *)(iVar2 + 0x19));
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002be74 @ 0xf002be74 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002be74(int param_1,char param_2,ushort param_3)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  
  iVar1 = _DAT_ram_020a0064;
  iVar3 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (param_2 == '\0') {
    uVar2 = (uint)*(ushort *)(iVar3 + 0x6a);
    if (uVar2 == 0) {
      uVar2 = 100;
    }
    uVar2 = param_3 / uVar2 + 10;
    if (0x3f < uVar2) {
      uVar2 = 0x3f;
    }
    *(char *)(iVar3 + 0x79) = (char)uVar2;
    (*_DAT_ram_00014c6c)
              (*(undefined1 *)(iVar3 + 0x27c),*(undefined1 *)(iVar3 + 0x27d),
               *(undefined1 *)(iVar3 + 0x27e),*(undefined2 *)(iVar3 + 0x280),
               *(undefined1 *)(iVar3 + 0x79),_DAT_ram_00014c6c);
  }
  else {
    *(undefined1 *)(_DAT_ram_020a0064 + (uint)*(byte *)(iVar3 + 0x19) + 0x1f6e) = 0;
    iVar1 = iVar1 + (uint)*(byte *)(iVar3 + 0x19);
    if ((*(char *)(iVar1 + 0x1f6e) == '\0') || (*(char *)(iVar1 + 0x1f71) == '\0')) {
      FUN_ram_f004a7ac(iVar3);
    }
  }
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002bf0c @ 0xf002bf0c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002bf0c(int *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  if (*param_1 != 0) {
    return;
  }
  iVar1 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  iVar2 = _DAT_ram_020a0064;
  if ((-0x8000 - param_1[0x71]) + iVar1 < 0) {
    return;
  }
  if (param_1[3] == 0) {
    iVar1 = param_1[0x16];
    if (iVar1 == 0) {
      uVar3 = 0x2826;
LAB_ram_f002bffa:
      (*_DAT_ram_00014800)(0,0,0xf00635bc,uVar3,_DAT_ram_00014800);
      goto LAB_ram_f002bffc;
    }
    *(undefined1 *)(_DAT_ram_020a0064 + (uint)*(byte *)((int)param_1 + 0x19) + 0x1f71) = 0;
    iVar2 = iVar2 + (uint)*(byte *)((int)param_1 + 0x19);
    if ((*(char *)(iVar2 + 0x1f6e) == '\0') || (*(char *)(iVar2 + 0x1f71) == '\0')) {
      FUN_ram_f004a7ac(param_1);
    }
    if (*(char *)(iVar1 + 0x25) != '\0') goto LAB_ram_f002bffc;
    iVar2 = FUN_ram_f00228c0(iVar1,0,&LAB_ram_f00234cc);
    if (iVar2 == 0) {
      *(undefined1 *)(iVar1 + 0x25) = 1;
      goto LAB_ram_f002bffc;
    }
  }
  else {
    if (param_1[3] != 1) {
      uVar3 = 0x2835;
      goto LAB_ram_f002bffa;
    }
    *(undefined1 *)(_DAT_ram_020a0064 + (uint)*(byte *)((int)param_1 + 0x19) + 0x1f71) = 0;
    iVar2 = iVar2 + (uint)*(byte *)((int)param_1 + 0x19);
    if ((*(char *)(iVar2 + 0x1f6e) == '\0') || (*(char *)(iVar2 + 0x1f71) == '\0')) {
      FUN_ram_f004a7ac(param_1);
    }
  }
  FUN_ram_f002b420(*(undefined1 *)((int)param_1 + 0x19),4);
LAB_ram_f002bffc:
  _DAT_ram_f0064f48 = _DAT_ram_f0064f48 + 1;
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x26ba <<<
// ===== FUN_ram_f002c094 @ 0xf002c094 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c094(int param_1,uint param_2)

{
  int iVar1;
  undefined1 uVar2;
  undefined2 uVar3;
  
  iVar1 = _DAT_ram_020a0064;
  if (4 < param_2) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x26ba,_DAT_ram_00014800);
    return;
  }
  if (param_2 == 2) {
    uVar2 = 0x1f;
LAB_ram_f002c13a:
    *(undefined1 *)(param_1 + 0x27d) = uVar2;
    *(undefined1 *)(param_1 + 0x27e) = 1;
    *(undefined2 *)(param_1 + 0x280) = *(undefined2 *)(param_1 + 0x6a);
  }
  else {
    if (param_2 < 3) {
      if (param_2 == 1) {
        *(undefined1 *)(param_1 + 0x79) = 0xf;
        *(undefined1 *)(param_1 + 0x27d) = 0xf;
        *(undefined1 *)(param_1 + 0x27c) = 1;
        *(undefined2 *)(param_1 + 0x280) = *(undefined2 *)(param_1 + 0x6a);
        *(undefined1 *)(param_1 + 0x27e) = 1;
        goto LAB_ram_f002c158;
      }
    }
    else {
      if (param_2 == 3) {
        uVar2 = 0x3f;
        goto LAB_ram_f002c13a;
      }
      if (param_2 == 4) {
        *(undefined1 *)(param_1 + 0x27c) = 0;
        *(undefined1 *)(param_1 + 0x27d) = 0;
        *(undefined1 *)(param_1 + 0x27e) = 1;
        *(undefined2 *)(param_1 + 0x280) = *(undefined2 *)(param_1 + 0x6a);
        goto LAB_ram_f002c158;
      }
    }
    if (*(char *)(_DAT_ram_020a0064 + 0x1f74) == '\x01') {
      uVar2 = 0x1e;
    }
    else {
      uVar2 = 5;
    }
    *(undefined1 *)(param_1 + 0x27d) = uVar2;
    if (*(char *)(iVar1 + 0x1f74) == '\x01') {
      uVar3 = *(undefined2 *)(param_1 + 0x6a);
    }
    else {
      uVar3 = 0xc;
    }
    *(undefined2 *)(param_1 + 0x280) = uVar3;
    *(undefined1 *)(param_1 + 0x27e) = 1;
    *(undefined1 *)(param_1 + 0x79) = 10;
  }
  *(undefined1 *)(param_1 + 0x27c) = 1;
LAB_ram_f002c158:
  iVar1 = FUN_ram_f0020000();
  if (iVar1 != 0) {
    *(undefined2 *)(param_1 + 0x280) = 0xc;
  }
  (*_DAT_ram_00014c6c)
            (*(undefined1 *)(param_1 + 0x27c),*(undefined1 *)(param_1 + 0x27d),
             *(undefined1 *)(param_1 + 0x27e),*(undefined2 *)(param_1 + 0x280),
             *(undefined1 *)(param_1 + 0x79),_DAT_ram_00014c6c);
  *(uint *)(param_1 + 0x284) = param_2;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002c188 @ 0xf002c188 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002c188(int param_1,char param_2)

{
  uint uVar1;
  undefined4 uVar2;
  
  if (param_2 == '\0') {
    if (*(byte *)(param_1 + 0x344) < 2) {
      uVar1 = *(uint *)((uint)*(byte *)(param_1 + 0x346) * 4 + -0xff92914);
      if (1 < uVar1) {
        *(uint *)((uint)*(byte *)(param_1 + 0x346) * 4 + -0xff92914) = uVar1 - 2;
        (*_DAT_ram_00014b98)(*(undefined1 *)(param_1 + 0x344),0,_DAT_ram_00014b98);
        (*_DAT_ram_00014bd0)(*(undefined1 *)(param_1 + 0x346),0,0,_DAT_ram_00014bd0);
        (*_DAT_ram_00014b7c)
                  ((uint)*(byte *)(param_1 + 0x346),
                   *(undefined1 *)((uint)*(byte *)(param_1 + 0x346) * 4 + -0xff92914),
                   _DAT_ram_00014b7c);
        (*_DAT_ram_00014bd0)(*(undefined1 *)(param_1 + 0x346),1,1,_DAT_ram_00014bd0);
        (*_DAT_ram_00014b98)(*(undefined1 *)(param_1 + 0x344),1,_DAT_ram_00014b98);
      }
    }
    uVar1 = *(uint *)(param_1 + 0x284);
    if (uVar1 != 2) {
      if (uVar1 < 3) {
        if (uVar1 == 0) {
LAB_ram_f002c302:
          FUN_ram_f00266d8(param_1 + 0x130);
          return 0;
        }
        if (uVar1 == 1) {
          FUN_ram_f002c094(param_1,0);
          goto LAB_ram_f002c302;
        }
      }
      else {
        if (uVar1 == 3) {
          uVar2 = 2;
          goto LAB_ram_f002c2dc;
        }
        if (uVar1 == 4) goto LAB_ram_f002c2d6;
      }
      goto LAB_ram_f002c2e6;
    }
LAB_ram_f002c2c6:
    uVar2 = 1;
LAB_ram_f002c2dc:
    FUN_ram_f002c094(param_1,uVar2);
  }
  else {
    if (*(byte *)(param_1 + 0x344) < 2) {
      uVar1 = *(uint *)((uint)*(byte *)(param_1 + 0x346) * 4 + -0xff92914);
      if (uVar1 < 7) {
        *(uint *)((uint)*(byte *)(param_1 + 0x346) * 4 + -0xff92914) = uVar1 + 2;
        (*_DAT_ram_00014b98)(*(undefined1 *)(param_1 + 0x344),0,_DAT_ram_00014b98);
        (*_DAT_ram_00014bd0)(*(undefined1 *)(param_1 + 0x346),0,0,_DAT_ram_00014bd0);
        (*_DAT_ram_00014b7c)
                  ((uint)*(byte *)(param_1 + 0x346),
                   *(undefined1 *)((uint)*(byte *)(param_1 + 0x346) * 4 + -0xff92914),
                   _DAT_ram_00014b7c);
        (*_DAT_ram_00014bd0)(*(undefined1 *)(param_1 + 0x346),1,1,_DAT_ram_00014bd0);
        (*_DAT_ram_00014b98)(*(undefined1 *)(param_1 + 0x344),1,_DAT_ram_00014b98);
      }
    }
    uVar1 = *(uint *)(param_1 + 0x284);
    if (uVar1 == 2) {
LAB_ram_f002c2d6:
      uVar2 = 3;
      goto LAB_ram_f002c2dc;
    }
    if (uVar1 < 3) {
      if (uVar1 == 0) goto LAB_ram_f002c2c6;
      if (uVar1 == 1) {
        FUN_ram_f002c094(param_1,2);
        uVar2 = 1;
        goto LAB_ram_f002c2e8;
      }
    }
    else {
      if (uVar1 == 3) {
        uVar2 = 4;
        goto LAB_ram_f002c2dc;
      }
      if (uVar1 == 4) {
        uVar2 = 0;
        goto LAB_ram_f002c2e8;
      }
    }
  }
LAB_ram_f002c2e6:
  uVar2 = 1;
LAB_ram_f002c2e8:
  FUN_ram_f002672c(param_1 + 0x130,360000);
  return uVar2;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2371 <<<
// ===== FUN_ram_f002c334 @ 0xf002c334 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c334(void)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  undefined2 *puVar6;
  undefined4 *puVar7;
  undefined2 *puVar8;
  
  iVar5 = _DAT_ram_020a006c;
  iVar2 = _DAT_ram_020a0064;
  puVar7 = (undefined4 *)(_DAT_ram_020a0064 + 0x1ef0);
  *(char *)(_DAT_ram_020a0064 + 0x1679) = *(char *)(_DAT_ram_020a0064 + 0x1679) + '\x01';
  iVar3 = FUN_ram_f0025c48(0x90);
  if (iVar3 != 0) {
    puVar8 = *(undefined2 **)(iVar3 + 8);
    puVar6 = puVar8 + 4;
    (*(code *)&SUB_ram_0006a0a0)(puVar6,0,0x400);
    *(undefined1 *)(puVar8 + 2) = 0x1c;
    *(undefined1 *)((int)puVar8 + 5) = 0;
    *puVar8 = 0x90;
    (*(code *)&SUB_ram_0006a0a0)(puVar6,0,0x88);
    *(undefined1 *)puVar6 = *(undefined1 *)(iVar5 + 0x19);
    *(undefined1 *)(puVar8 + 7) = *(undefined1 *)(iVar2 + 0x1679);
    if (*(char *)(iVar2 + 0x1f04) == '\0') {
      puVar8[6] = 0;
      *(undefined1 *)((int)puVar8 + 0xb) = 0;
    }
    else {
      *(undefined1 *)((int)puVar8 + 0xb) = 1;
      uVar4 = *(uint *)(iVar2 + 0x1f10) >> 10;
      puVar8[6] = (short)uVar4;
      if ((uVar4 & 0xffff) >> 7 != 0) {
        (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2371,_DAT_ram_00014800);
      }
    }
    *(undefined1 *)(puVar8 + 8) = *(undefined1 *)(iVar2 + 0x1edc);
    bVar1 = *(char *)(iVar2 + 0x1edc) != '\0';
    if (bVar1) {
      *(undefined1 *)((int)puVar8 + 0x11) = *(undefined1 *)(iVar2 + 0x1ee8);
      *(undefined4 *)(puVar8 + 10) = *puVar7;
      *(undefined4 *)(puVar8 + 0xc) = *(undefined4 *)(iVar2 + 0x1eec);
      *(undefined4 *)(puVar8 + 0xe) = *(undefined4 *)(iVar2 + 0x1ee0);
    }
    uVar4 = (uint)bVar1;
    iVar5 = uVar4 * 2 + 1;
    *(undefined1 *)(puVar6 + iVar5 * 4) = *(undefined1 *)(iVar2 + 0x1ef4);
    if (*(char *)(iVar2 + 0x1ef4) != '\0') {
      *(undefined1 *)((int)puVar6 + iVar5 * 8 + 1) = 1;
      uVar4 = uVar4 + 1;
      *(undefined4 *)(puVar6 + iVar5 * 4 + 2) = *(undefined4 *)(iVar2 + 0x1f00);
      *(undefined4 *)(puVar6 + uVar4 * 8) = 0;
      *(undefined4 *)((int)(puVar6 + uVar4 * 8) + 4) = *(undefined4 *)(iVar2 + 0x1ef8);
    }
    *(char *)((int)puVar8 + 0xf) = (char)uVar4;
    *(undefined1 *)(iVar3 + 0x2f) = 0;
    *(undefined2 **)(iVar3 + 0x28) = puVar8;
    puVar8[1] = 1;
    FUN_ram_f0039750(iVar3);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002c468 @ 0xf002c468 =====


void FUN_ram_f002c468(int param_1)

{
  *(undefined4 *)(param_1 + 0x33c) = 0;
  *(undefined1 *)(param_1 + 0x37) = 0;
  *(undefined1 *)(param_1 + 0x38) = 0;
  *(undefined1 *)(param_1 + 0x39) = 0;
  *(undefined1 *)(param_1 + 0x3a) = 0;
  (*(code *)&SUB_ram_0006a0a0)(param_1 + 0x40,0,0x10);
  *(undefined1 *)(param_1 + 0x51) = 0;
  *(undefined1 *)(param_1 + 0x50) = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002c4ac @ 0xf002c4ac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c4ac(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = _DAT_ram_020a0064;
  *(undefined4 *)(param_1 + 0x1c) = 0;
  *(undefined4 *)(param_1 + 0x20) = 0;
  *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(param_1 + 0x19) + 0x3d6) * 4 + 4) = 1;
  *(undefined4 *)(param_1 + 0xa8) = 0;
  *(undefined1 *)(param_1 + 0xb0) = 0;
  *(undefined1 *)(param_1 + 0xb1) = 0;
  *(undefined1 *)(param_1 + 0xb3) = 1;
  *(undefined1 *)(param_1 + 0xb4) = 0;
  *(undefined1 *)(param_1 + 0xb6) = 0;
  *(undefined1 *)(param_1 + 0xb7) = 0;
  *(undefined1 *)(param_1 + 0xb8) = 0;
  *(undefined1 *)(param_1 + 0x261) = 0;
  *(undefined1 *)(param_1 + 0xbd) = 0;
  (*(code *)&SUB_ram_0006a0a0)(param_1 + 0xc0,0,0x10);
  *(undefined1 *)(param_1 + 0x27d) = 5;
  *(undefined2 *)(param_1 + 0x280) = 0xc;
  *(undefined1 *)(param_1 + 0x1c0) = 0;
  *(undefined1 *)(param_1 + 0x1c1) = 0;
  *(undefined1 *)(param_1 + 0x1cc) = 0;
  *(undefined4 *)(param_1 + 0x1d0) = 0;
  *(undefined1 *)(param_1 + 0x260) = 0;
  *(undefined1 *)(param_1 + 0x27c) = 1;
  *(undefined1 *)(param_1 + 0x27e) = 1;
  *(undefined1 *)((uint)*(byte *)(param_1 + 0x19) + iVar1 + 0x1f6e) = 1;
  *(undefined1 *)(iVar1 + (uint)*(byte *)(param_1 + 0x19) + 0x1f71) = 1;
  *(undefined4 *)(param_1 + 0x284) = 0;
  FUN_ram_f00266d8(param_1 + 0x130);
  *(undefined1 *)(param_1 + 0x29d) = 0;
  (*(code *)&SUB_ram_0006a0a0)(param_1 + 500,0,0x18);
  uVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  *(undefined4 *)(param_1 + 0x200) = uVar2;
  *(undefined2 *)(param_1 + 0x34a) = 0;
  *(undefined1 *)(param_1 + 0xb5) = 0;
  _DAT_ram_f0064f4c = 0;
  if (*(char *)(param_1 + 0x19) == '\x02') {
    *(undefined4 *)(param_1 + 0x20c) = 0;
    *(undefined4 *)(param_1 + 0x210) = 0;
  }
  if (*(byte *)(param_1 + 0x346) < 2) {
    *(undefined4 *)((uint)*(byte *)(param_1 + 0x346) * 4 + -0xff92914) = 0;
    (*_DAT_ram_00014b7c)
              ((uint)*(byte *)(param_1 + 0x346),
               *(undefined1 *)((uint)*(byte *)(param_1 + 0x346) * 4 + -0xff92914),_DAT_ram_00014b7c)
    ;
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2651,0x2652 <<<
// ===== FUN_ram_f002c5cc @ 0xf002c5cc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c5cc(int param_1,uint param_2)

{
  if (param_1 != 2) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2651,_DAT_ram_00014800);
  }
  if (1 < param_2) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2652,_DAT_ram_00014800);
  }
  (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_020a0064 + 0x1f2c + param_2 * 0x14,0);
  FUN_ram_f0030b3c();
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x25fc <<<
// ===== FUN_ram_f002c640 @ 0xf002c640 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c640(int param_1,undefined4 param_2)

{
  if (*(char *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x19) != '\x02') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x25fc,_DAT_ram_00014800);
  }
  FUN_ram_f002c5cc(param_1,param_2);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2625,0x263b <<<
// ===== FUN_ram_f002c690 @ 0xf002c690 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f002c690(int param_1,int param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;
  
  iVar1 = _DAT_ram_020a0064;
  if (*(char *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x19) != '\x02') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2625,_DAT_ram_00014800);
  }
  if (*(char *)(iVar1 + 0x1f2c) == '\0') {
    *(undefined1 *)(iVar1 + 0x1f2c) = 1;
    *(int *)(iVar1 + 0x1f34) = param_3 + param_2;
    *(int *)(iVar1 + 0x1f30) = param_2;
    *(int *)(iVar1 + 0x1f38) = param_3;
    *(undefined4 *)(iVar1 + 0x1f3c) = param_4;
    *(undefined1 *)(iVar1 + 0x1f2d) = 0;
    *(undefined1 *)(iVar1 + 0x1f2e) = 0;
    iVar2 = 0;
  }
  else {
    iVar2 = -1;
  }
  if (*(char *)(iVar1 + 8000) == '\0') {
    *(undefined1 *)(iVar1 + 0x1f42) = 0;
    *(undefined1 *)(iVar1 + 8000) = 1;
    *(undefined4 *)(iVar1 + 0x1f50) = param_4;
    *(int *)(iVar1 + 0x1f48) = param_3 + param_2;
    *(int *)(iVar1 + 0x1f44) = param_2;
    *(int *)(iVar1 + 0x1f4c) = param_3;
    *(undefined1 *)(iVar1 + 0x1f41) = 0;
    iVar2 = 1;
  }
  FUN_ram_f0030b3c();
  if (iVar2 == -1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x263b,_DAT_ram_00014800);
  }
  return iVar2;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x25cf <<<
// ===== FUN_ram_f002c750 @ 0xf002c750 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c750(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  if (*(char *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x19) != '\x02') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x25cf,_DAT_ram_00014800);
  }
  FUN_ram_f002c690(param_1,param_2,param_3,param_4);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2184 <<<
// ===== FUN_ram_f002c7a8 @ 0xf002c7a8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002c7a8(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  
  iVar3 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2184,_DAT_ram_00014800);
  }
  if (*(int *)(iVar3 + 0xc) == 2) {
    iVar1 = FUN_ram_f00343a8(param_1);
    if (iVar1 != 0) {
      return 1;
    }
    puVar2 = *(undefined4 **)(iVar3 + 0x5c);
    if (puVar2 != (undefined4 *)(iVar3 + 0x5c)) {
      for (; puVar2 != (undefined4 *)(iVar3 + 0x5c); puVar2 = (undefined4 *)*puVar2) {
        if (*(char *)(puVar2 + 0xe) == '\0') {
          return 1;
        }
        if ((puVar2[0xe] & 0xffffff) >> 8 != 0) {
          return 1;
        }
      }
    }
  }
  return 0;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2056 <<<
// ===== FUN_ram_f002c824 @ 0xf002c824 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002c824(int param_1,int param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 uVar2;
  
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2056,_DAT_ram_00014800);
  }
  iVar1 = _DAT_ram_020a0064;
  if (*(char *)(_DAT_ram_020a0064 + 0x1f18) == '\0') {
    *(undefined1 *)(_DAT_ram_020a0064 + 0x1f18) = 1;
    *(undefined4 *)(iVar1 + 0x1f24) = param_4;
    *(int *)(iVar1 + 0x1f20) = param_3 + param_2;
    *(undefined1 *)(iVar1 + 0x1f19) = 0;
    *(undefined1 *)(iVar1 + 0x1f1a) = 0;
    *(int *)(iVar1 + 0x1f28) = param_3;
    *(int *)(iVar1 + 0x1f1c) = param_2;
    uVar2 = 0;
  }
  else {
    uVar2 = 0xffffffff;
  }
  FUN_ram_f0030c40();
  return uVar2;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1fe1,0x1fe2 <<<
// ===== FUN_ram_f002c88c @ 0xf002c88c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c88c(int param_1,uint param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = _DAT_ram_020a0064;
  if (*(char *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x19) != '\x01') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1fe1,_DAT_ram_00014800);
  }
  if (param_2 == 0xffffffff) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1fe2,_DAT_ram_00014800);
  }
  else {
    if ((param_2 & 0x100) == 0) {
      if (param_2 != 0) {
        return;
      }
      iVar1 = iVar1 + 0x1edc;
      uVar2 = 0x18;
    }
    else {
      if ((param_2 & 0xfffffeff) != 0) {
        return;
      }
      iVar1 = iVar1 + 0x1ef4;
      uVar2 = 0x10;
    }
    (*(code *)&SUB_ram_0006a0a0)(iVar1,0,uVar2);
    FUN_ram_f0030c40();
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x200e <<<
// ===== FUN_ram_f002c928 @ 0xf002c928 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c928(int param_1)

{
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x200e,_DAT_ram_00014800);
  }
  FUN_ram_f002c88c(param_1,0);
  FUN_ram_f002c88c(param_1,0x100);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1d36 <<<
// ===== FUN_ram_f002c974 @ 0xf002c974 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002c974(int param_1,undefined4 param_2)

{
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1d36,_DAT_ram_00014800);
  }
  if ((*(int *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0xc) == 2) && (param_1 == 1)) {
    FUN_ram_f002c88c(1,param_2);
    FUN_ram_f002c334();
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1f86,0x1fbf <<<
// ===== FUN_ram_f002c9d4 @ 0xf002c9d4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002c9d4(int param_1,int param_2,int param_3,int param_4,undefined4 param_5)

{
  int iVar1;
  char *pcVar2;
  undefined4 uVar3;
  int iVar4;
  int local_28 [3];
  
  iVar1 = _DAT_ram_020a0064;
  iVar4 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  pcVar2 = (char *)(_DAT_ram_020a0064 + 0x1edc);
  if (*(char *)(iVar4 + 0x19) != '\x01') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1f86,_DAT_ram_00014800);
  }
  (*_DAT_ram_00014bf0)(*(undefined1 *)(iVar4 + 0x346),local_28,_DAT_ram_00014bf0);
  if (param_4 == 0) {
    FUN_ram_f002c88c(param_1,0x100);
    if (*(char *)(iVar1 + 0x1ef4) == '\0') {
      *(int *)(iVar1 + 0x1ef8) = param_2;
      *(undefined1 *)(iVar1 + 0x1ef6) = 0;
      *(int *)(iVar1 + 0x1f00) = param_3;
      *(undefined1 *)(iVar1 + 0x1ef4) = 1;
      *(undefined1 *)(iVar1 + 0x1ef5) = 0;
      *(int *)(iVar1 + 0x1efc) = param_3 + param_2;
      if ((param_2 - local_28[0] < 0) && (local_28[0] - (param_3 + param_2) < 0)) {
        *(undefined1 *)(iVar1 + 0x1ef6) = 1;
      }
      uVar3 = 0x100;
      goto LAB_ram_f002cada;
    }
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1fbf,_DAT_ram_00014800);
  }
  else if (*pcVar2 == '\0') {
    *(int *)(iVar1 + 0x1ee0) = param_2;
    *(undefined4 *)(iVar1 + 0x1ee8) = param_5;
    *(int *)(iVar1 + 0x1eec) = param_4;
    *(int *)(iVar1 + 0x1ef0) = param_3;
    *pcVar2 = '\x01';
    *(undefined1 *)(iVar1 + 0x1edd) = 0;
    *(undefined1 *)(iVar1 + 0x1ede) = 0;
    *(int *)(iVar1 + 0x1ee4) = param_3 + param_2;
    if ((param_2 - local_28[0] < 0) && (local_28[0] - (param_3 + param_2) < 0)) {
      *(undefined1 *)(iVar1 + 0x1ede) = 1;
      uVar3 = 0;
    }
    else {
      uVar3 = 0;
    }
    goto LAB_ram_f002cada;
  }
  uVar3 = 0xffffffff;
LAB_ram_f002cada:
  FUN_ram_f0030c40();
  return uVar3;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2223,0x2226 <<<
// ===== FUN_ram_f002caf0 @ 0xf002caf0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002caf0(int param_1,byte *param_2,ushort param_3)

{
  uint uVar1;
  uint uVar2;
  bool bVar3;
  bool bVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  byte *pbStack_4c;
  int local_40;
  int local_38;
  uint local_34;
  int local_28 [3];
  
  iVar6 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2223,_DAT_ram_00014800);
  }
  uVar1 = param_3 / 0xd;
  if (uVar1 * 0xd != (uint)param_3) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2226,_DAT_ram_00014800);
  }
  FUN_ram_f002c928(param_1);
  local_40 = 0;
  local_38 = 0;
  local_34 = 0;
  bVar3 = false;
  pbStack_4c = param_2;
  do {
    if (uVar1 <= local_34) {
      FUN_ram_f0031220();
      return;
    }
    iVar7 = *(int *)(pbStack_4c + 1);
    uVar10 = *(uint *)(pbStack_4c + 5);
    iVar9 = *(int *)(pbStack_4c + 9);
    uVar8 = (uint)*pbStack_4c;
    (*_DAT_ram_00014bf0)(*(undefined1 *)(iVar6 + 0x346),local_28);
    iVar5 = iVar9;
    if (((uVar10 == 0) || (uVar8 == 0)) || (-1 < iVar9 - local_28[0])) {
LAB_ram_f002cc04:
      if (local_28[0] - iVar5 < 0) {
        if (uVar10 != 0) {
          if (((local_40 == 0) && (local_38 == 0)) ||
             ((-1 < local_28[0] - local_38 || (-1 < local_40 - local_28[0]))))
          goto LAB_ram_f002cc44;
          bVar3 = true;
LAB_ram_f002cc48:
          iVar5 = iVar5 - uVar10;
        }
      }
      else {
        bVar4 = true;
        if (-1 < (local_28[0] - iVar7) - iVar5) {
          bVar4 = bVar3;
        }
        bVar3 = bVar4;
        if (uVar10 != 0) {
LAB_ram_f002cc44:
          if (bVar3) goto LAB_ram_f002cc48;
        }
      }
      if ((local_28[0] - iVar5 < 0) || ((local_28[0] - iVar7) - iVar5 < 0)) {
        iVar9 = FUN_ram_f0023e50(param_1,iVar5,iVar7,uVar10,uVar8);
        if (iVar9 != 0) {
          FUN_ram_f002c9d4(param_1,iVar5,iVar7,uVar10,uVar8);
        }
      }
    }
    else {
      iVar5 = iVar9 + (((uVar10 + local_28[0] + -1) - iVar9) / uVar10) * uVar10;
      if ((uint)(iVar5 - local_28[0]) < 2000) {
        iVar5 = iVar5 + uVar10;
      }
      if (uVar8 == 0xff) {
LAB_ram_f002cbf6:
        local_40 = iVar5 - uVar10;
        local_38 = local_40 + iVar7;
        goto LAB_ram_f002cc04;
      }
      uVar2 = (uint)(iVar5 - iVar9) / uVar10;
      if (uVar8 <= uVar2) {
        uVar8 = uVar8 - uVar2 & 0xff;
        goto LAB_ram_f002cbf6;
      }
    }
    local_34 = local_34 + 1;
    pbStack_4c = pbStack_4c + 0xd;
  } while( true );
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1f4d,0x1f58 <<<
// ===== FUN_ram_f002ccb8 @ 0xf002ccb8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002ccb8(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = _DAT_ram_020a0064;
  iVar2 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (*(char *)(iVar2 + 0x19) != '\x01') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1f4d,_DAT_ram_00014800);
  }
  if ((*(char *)(iVar1 + 0x1f04) != '\0') && (*(char *)(iVar2 + 0x19) == '\x01')) {
    *(undefined1 *)(iVar1 + 0x1f04) = 0;
    if (_DAT_ram_f0064f38 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1f58,_DAT_ram_00014800);
    }
    if (_DAT_ram_f0064f38 != 0) {
      _DAT_ram_f0064f38 = _DAT_ram_f0064f38 + -1;
    }
    if (_DAT_ram_f0064f38 == 0) {
      (*_DAT_ram_00014bd4)(*(undefined1 *)(iVar2 + 0x346),0,_DAT_ram_00014bd4);
    }
    (*(code *)&SUB_ram_0006a0a0)(iVar1 + 0x1f04,0,0x14);
    FUN_ram_f0030c40();
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1e29 <<<
// ===== FUN_ram_f002cd78 @ 0xf002cd78 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002cd78(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (*(char *)(iVar1 + 0x19) != '\x01') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1e29,_DAT_ram_00014800);
  }
  if (*(int *)(iVar1 + 0xc) == 2) {
    FUN_ram_f002ccb8(param_1);
    FUN_ram_f002c334();
    *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(iVar1 + 0x19) + 0x3d6) * 4 + 4) = 1;
  }
  return 0;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x22ca <<<
// ===== FUN_ram_f002cdec @ 0xf002cdec =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002cdec(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x22ca,_DAT_ram_00014800);
  }
  FUN_ram_f002cd78(param_1);
  iVar1 = FUN_ram_f0023864(param_1,param_2,param_3,param_4);
  if (iVar1 != 0) {
    FUN_ram_f002c824(param_1,param_2,param_3,param_4);
  }
  return 1;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2559,0x2560 <<<
// ===== FUN_ram_f002ce58 @ 0xf002ce58 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002ce58(int param_1,int param_2,undefined1 param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uStack_2c;
  undefined4 uStack_28;
  undefined4 auStack_24 [2];
  
  auStack_24[0] = 0;
  uStack_28 = 0;
  uStack_2c = 0;
  if (*(char *)(param_2 + 4) != '\0') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2559,_DAT_ram_00014800);
  }
  uVar5 = *(ushort *)(param_2 + 1) - 2 & 0xffff;
  uVar1 = uVar5 / 0xd;
  if (uVar5 != uVar1 * 0xd) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2560,_DAT_ram_00014800);
  }
  uVar9 = 0;
  uVar8 = 0;
  uVar7 = 0;
  uVar6 = 0;
  uVar3 = 0;
  uVar10 = 0;
  iVar2 = param_2;
  for (uVar5 = 0; uVar5 < uVar1; uVar5 = uVar5 + 1) {
    if (*(char *)(iVar2 + 5) == '\x01') {
      iVar4 = param_2 + uVar5 * 0xd;
      uVar10 = *(undefined4 *)(iVar4 + 0xe);
      uVar3 = *(undefined4 *)(iVar4 + 6);
      uVar6 = *(undefined4 *)(iVar4 + 10);
    }
    else if (*(char *)(iVar2 + 5) == '\x02') {
      iVar4 = param_2 + uVar5 * 0xd;
      uVar7 = *(undefined4 *)(iVar4 + 0xe);
      uVar8 = *(undefined4 *)(iVar4 + 6);
      uVar9 = *(undefined4 *)(iVar4 + 10);
    }
    iVar2 = iVar2 + 0xd;
  }
  iVar2 = FUN_ram_f0023870(*(undefined1 *)(param_1 + 0xd),uVar10,uVar3,uVar6,uVar7,uVar8,uVar9,
                           auStack_24,&uStack_28,&uStack_2c);
  if (iVar2 == 0) {
    FUN_ram_f002bcd4(*(undefined4 *)(param_1 + 0x20),param_3,5,auStack_24[0],uStack_28,uStack_2c);
  }
  else {
    FUN_ram_f002bcd4(*(undefined4 *)(param_1 + 0x20),param_3,0,auStack_24[0],uStack_28,uStack_2c);
    FUN_ram_f002cdec(*(undefined1 *)(param_1 + 0xd),auStack_24[0],uStack_28,uStack_2c);
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1ce2,0x1ce5,0x1ce6 <<<
// ===== FUN_ram_f002cfa8 @ 0xf002cfa8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_ram_f002cfa8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                undefined4 param_5)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  
  iVar1 = _DAT_ram_020a0064;
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1ce2,_DAT_ram_00014800);
  }
  iVar4 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (iVar4 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1ce5,_DAT_ram_00014800);
  }
  if (*(int *)(iVar4 + 0xc) != 2) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1ce6,_DAT_ram_00014800);
  }
  puVar2 = *(undefined4 **)(iVar4 + 0x5c);
  if ((undefined4 *)(iVar4 + 0x5c) != puVar2) {
    for (; (undefined4 *)(iVar4 + 0x5c) != puVar2; puVar2 = (undefined4 *)*puVar2) {
      if (puVar2[4] != 0x22) goto LAB_ram_f002d086;
    }
  }
  if ((param_1 == 1) && (*(int *)(iVar4 + 0xc) == 2)) {
    if (*(char *)(iVar1 + 0x1f04) != '\0') {
      FUN_ram_f002ccb8(1);
    }
    uVar3 = FUN_ram_f002c9d4(1,param_2,param_3,param_4,param_5);
    FUN_ram_f002c334();
  }
  else {
LAB_ram_f002d086:
    uVar3 = 0xffffffff;
  }
  return uVar3;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1f0a <<<
// ===== FUN_ram_f002d178 @ 0xf002d178 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002d178(int param_1,int param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  int local_28;
  
  iVar1 = _DAT_ram_020a0064;
  iVar4 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (*(char *)(iVar4 + 0x19) != '\x01') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1f0a,_DAT_ram_00014800);
  }
  if (*(char *)(iVar1 + 0x1f04) != '\0') {
    FUN_ram_f002ccb8(param_1);
  }
  if (*(char *)(iVar4 + 0x19) != '\x01') {
    return;
  }
  (*_DAT_ram_00014bf0)(*(undefined1 *)(iVar4 + 0x346));
  *(undefined1 *)(iVar1 + 0x1f06) = 0;
  *(undefined1 *)(iVar1 + 0x1f05) = 0;
  if ((param_2 - local_28 < 0) && ((local_28 - param_2) - param_3 < 0)) {
    *(undefined1 *)(iVar1 + 0x1f06) = 1;
  }
  *(undefined4 *)(iVar1 + 0x1f14) = param_4;
  *(int *)(iVar1 + 0x1f10) = param_3;
  if (*(char *)(iVar4 + 0x344) == '\0') {
    uVar2 = 0x200000;
    uVar3 = 0;
  }
  else {
    if (*(char *)(iVar4 + 0x344) != '\x01') goto LAB_ram_f002d23c;
    uVar3 = 0x100;
    uVar2 = 0;
  }
  (*_DAT_ram_00014988)(uVar2,uVar3,_DAT_ram_00014988);
LAB_ram_f002d23c:
  _DAT_ram_f0064f38 = _DAT_ram_f0064f38 + 1;
  (*_DAT_ram_00014bd4)(*(undefined1 *)(iVar4 + 0x346),1,_DAT_ram_00014bd4);
  *(undefined1 *)(iVar1 + 0x1f04) = 1;
  *(int *)(iVar1 + 0x1f0c) = param_3 + param_2;
  *(int *)(iVar1 + 0x1f08) = param_2;
  FUN_ram_f0030c40();
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x229c <<<
// ===== FUN_ram_f002d278 @ 0xf002d278 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002d278(int param_1,int param_2,char param_3)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar3 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (param_3 == '\0') {
    param_2 = 0;
    iVar4 = 0;
  }
  else {
    if (param_2 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf00635bc,0x229c,_DAT_ram_00014800);
    }
    if (*(short *)(iVar3 + 0x6a) == 0) {
      return;
    }
    iVar4 = param_2 << 10;
  }
  iVar2 = (*_DAT_ram_00014c08)(*(undefined1 *)(iVar3 + 0x346),_DAT_ram_00014c08);
  uVar1 = *(ushort *)(iVar3 + 0x6a);
  iVar3 = FUN_ram_f0023860(param_1,param_2);
  if (iVar3 != 0) {
    if (param_3 == '\0') {
      FUN_ram_f002ccb8(param_1);
    }
    else {
      FUN_ram_f002d178(param_1,iVar2 * 0x400 + iVar4,param_2 << 10,(uint)uVar1 << 10);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002d318 @ 0xf002d318 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002d318(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = _DAT_ram_020a0064;
  if (((param_1 == 1) && (*(int *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0xc) == 0)) &&
     ((*(char *)(param_2 + 3) != *(char *)(_DAT_ram_020a0064 + 0x1679) ||
      (*(char *)(_DAT_ram_020a0064 + 0x1678) == '\0')))) {
    *(undefined1 *)(_DAT_ram_020a0064 + 0x1678) = 1;
    *(undefined1 *)(iVar1 + 0x1679) = *(undefined1 *)(param_2 + 3);
    FUN_ram_f002d278(1,*(byte *)(param_2 + 4) & 0x7f,*(byte *)(param_2 + 4) >> 7);
    FUN_ram_f002caf0(1,param_2 + 5,*(short *)(param_2 + 1) + -2);
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1df3,0x1df4,0x1df8 <<<
// ===== FUN_ram_f002d38c @ 0xf002d38c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002d38c(int param_1)

{
  uint uVar1;
  undefined4 extraout_a1;
  int iVar2;
  uint uVar3;
  uint local_20 [3];
  
  iVar2 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (*(char *)(iVar2 + 0x19) != '\x01') {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1df3,_DAT_ram_00014800);
  }
  if (*(int *)(iVar2 + 0xc) != 2) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1df4,_DAT_ram_00014800);
  }
  uVar3 = *(uint *)(_DAT_ram_020a0064 + 0x1980);
  if (uVar3 < 0x2800) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1df8,_DAT_ram_00014800);
  }
  if (*(int *)(iVar2 + 0xc) == 2) {
    (*_DAT_ram_00014bf0)(*(undefined1 *)(iVar2 + 0x346),local_20,_DAT_ram_00014bf0);
    uVar1 = (*_DAT_ram_00014c08)(*(undefined1 *)(iVar2 + 0x346),extraout_a1,_DAT_ram_00014c08);
    FUN_ram_f002d178(param_1,((uVar1 & 0xffff) << 10 | local_20[0] & 0xfc000000) +
                             (uint)*(ushort *)(iVar2 + 0x6a) * 0xc00,uVar3,
                     (uint)*(ushort *)(iVar2 + 0x6a) << 10);
    FUN_ram_f002c334();
  }
  return 0;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1e8b,0x1e8c <<<
// ===== FUN_ram_f002d478 @ 0xf002d478 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002d478(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  
  iVar1 = _DAT_ram_020a0064;
  iVar3 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1e8b,_DAT_ram_00014800);
  }
  if (*(int *)(iVar3 + 0xc) != 2) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1e8c,_DAT_ram_00014800);
  }
  if ((*(char *)(iVar1 + 0x1f04) == '\0') && (*(char *)(iVar1 + 0x1f18) == '\0')) {
    puVar2 = *(undefined4 **)(iVar3 + 0x5c);
    if (puVar2 != (undefined4 *)(iVar3 + 0x5c)) {
      for (; puVar2 != (undefined4 *)(iVar3 + 0x5c); puVar2 = (undefined4 *)*puVar2) {
        if (puVar2[4] != 0x22) {
          return 0;
        }
        if (*(char *)(puVar2 + 0xe) == '\0') {
          return 0;
        }
      }
    }
    iVar1 = FUN_ram_f002b140(param_1);
    if (iVar1 != 0) {
      FUN_ram_f002d38c(param_1);
      return 1;
    }
  }
  return 0;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1da6,0x1da9,0x1dc9,0x1dd1 <<<
// ===== FUN_ram_f002d594 @ 0xf002d594 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002d594(int param_1,char param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar1 = _DAT_ram_020a0064;
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1da6,_DAT_ram_00014800);
  }
  iVar3 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  if (*(int *)(iVar3 + 0xc) != 0) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1da9,_DAT_ram_00014800);
  }
  iVar2 = FUN_ram_f0030b00();
  if (iVar2 != 0) {
    if (param_2 == '\0') {
      iVar2 = FUN_ram_f0030b28();
      if ((((iVar2 == 0) && (iVar2 = FUN_ram_f0030b14(), iVar2 == 0)) &&
          (*(char *)(iVar1 + 0x1f18) == '\0')) && (*(char *)(iVar1 + 0x1f04) != '\0')) {
        *(undefined1 *)(iVar1 + 0x1f6c) = 1;
        if (_DAT_ram_f0064f38 == 0) {
          (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1dd1,_DAT_ram_00014800);
        }
        if (_DAT_ram_f0064f38 != 0) {
          _DAT_ram_f0064f38 = _DAT_ram_f0064f38 + -1;
        }
        if (_DAT_ram_f0064f38 == 0) {
          (*_DAT_ram_00014bd4)(*(undefined1 *)(iVar3 + 0x346),0,_DAT_ram_00014bd4);
        }
      }
      else {
        (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1dc9,_DAT_ram_00014800);
      }
    }
    else {
      *(undefined1 *)(iVar1 + 0x1f6c) = 0;
      _DAT_ram_f0064f38 = _DAT_ram_f0064f38 + 1;
      (*_DAT_ram_00014bd4)(*(undefined1 *)(iVar3 + 0x346),1,_DAT_ram_00014bd4);
      iVar3 = (*_DAT_ram_00014c08)(*(undefined1 *)(iVar3 + 0x346),_DAT_ram_00014c08);
      *(int *)(iVar1 + 0x1f0c) = iVar3 * 0x400 + *(int *)(iVar1 + 0x1f10);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x1c98 <<<
// ===== FUN_ram_f002d6d8 @ 0xf002d6d8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002d6d8(int param_1)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  
  iVar5 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
  *(undefined1 *)(param_1 + 0x38) = 0;
  *(undefined1 *)(param_1 + 0x37) = 0;
  iVar2 = _DAT_ram_020a0064;
  if (*(int *)(iVar5 + 0xc) == 2) {
    uVar1 = *(ushort *)(param_1 + 0x1a);
    if (uVar1 < 0x15) {
      iVar7 = _DAT_ram_020a0064 + (uint)(uVar1 >> 3);
      *(byte *)(iVar7 + 0x1f54) = ~(byte)(1 << (uVar1 & 7)) & *(byte *)(iVar7 + 0x1f54);
      uVar8 = *(uint *)(iVar2 + 0x1f64);
      uVar6 = *(uint *)(iVar2 + 0x1f68);
      uVar3 = (uint)*(ushort *)(param_1 + 0x1a);
      if (uVar8 == uVar6) {
        if (uVar8 == uVar3) {
          *(undefined4 *)(iVar2 + 0x1f64) = 0;
          *(undefined4 *)(iVar2 + 0x1f68) = 0;
        }
      }
      else if (uVar8 == uVar3) {
        do {
          uVar8 = uVar8 + 1;
          if (uVar6 < uVar8) goto LAB_ram_f002d7bc;
        } while ((*(byte *)(iVar2 + ((int)uVar8 >> 3) + 0x1f54) >> (uVar8 & 7) & 1) == 0);
        *(uint *)(iVar2 + 0x1f64) = uVar8;
      }
      else if (uVar6 == uVar3) {
        do {
          uVar6 = uVar6 - 1;
          if (uVar6 < uVar8) goto LAB_ram_f002d7bc;
        } while ((*(byte *)(iVar2 + (int)uVar6 / 8 + 0x1f54) >> ((int)uVar6 % 8 & 0x1fU) & 1) == 0);
        *(uint *)(iVar2 + 0x1f68) = uVar6;
      }
LAB_ram_f002d7bc:
      puVar4 = *(undefined4 **)(iVar5 + 0x5c);
      if (puVar4 != (undefined4 *)(iVar5 + 0x5c)) {
        for (; puVar4 != (undefined4 *)(iVar5 + 0x5c); puVar4 = (undefined4 *)*puVar4) {
          if (*(char *)(puVar4 + 0xe) != '\0') {
            *(undefined1 *)(iVar5 + 0xbb) = 1;
            return;
          }
        }
        *(undefined1 *)(iVar5 + 0xbb) = 0;
        FUN_ram_f0037050(*(undefined1 *)(iVar5 + 0x19));
      }
    }
    else {
      (*_DAT_ram_00014800)(0,0,0xf00635bc,0x1c98,_DAT_ram_00014800);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002d7fc @ 0xf002d7fc =====


bool FUN_ram_f002d7fc(void)

{
  int iVar1;
  
  iVar1 = FUN_ram_f003e5fc();
  return iVar1 == 0;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0xa30 <<<
// ===== FUN_ram_f002e1ec @ 0xf002e1ec =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002e1ec(int param_1)

{
  ushort uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  
  iVar2 = FUN_ram_f0025c48(0x18);
  if (iVar2 == 0) {
    uVar3 = 0xc000009a;
  }
  else {
    FUN_ram_f0020c1c(0);
    iVar4 = *(int *)(iVar2 + 8);
    if ((undefined2 *)(iVar4 + 8) == (undefined2 *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf00635bc,0xa30,_DAT_ram_00014800);
    }
    uVar1 = *(ushort *)(param_1 + 0x70);
    *(undefined2 *)(iVar4 + 8) = 0x10a4;
    *(ushort *)(iVar4 + 10) = uVar1 | 0xc000;
    (*(code *)&SUB_ram_0006a08c)(iVar4 + 0xc,param_1 + 0x4b,6);
    (*(code *)&SUB_ram_0006a08c)(iVar4 + 0x12,param_1 + 0x51,6);
    FUN_ram_f0034540(iVar2,*(undefined4 *)(param_1 + 0x58),*(undefined1 *)(param_1 + 0x19),
                     *(int *)(iVar2 + 8) + 8,0x10,*(int *)(iVar2 + 8) + 0x18,0,0,0,1,
                     *(undefined1 *)(param_1 + 0x349),7,0);
    FUN_ram_f0037b58(iVar2);
    *(undefined1 *)(param_1 + 0xba) = 1;
    uVar3 = 0;
  }
  return uVar3;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0xe93,0xe94 <<<
// ===== FUN_ram_f002e2d0 @ 0xf002e2d0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002e2d0(int param_1)

{
  ushort uVar1;
  bool bVar2;
  int iVar3;
  uint uVar4;
  undefined1 *puVar5;
  undefined4 *puVar6;
  uint uVar7;
  undefined1 uVar8;
  int *piVar9;
  int iVar10;
  int iVar11;
  ushort uVar12;
  int iVar13;
  undefined4 uVar14;
  undefined8 uVar15;
  
  iVar10 = *(int *)(param_1 + 0x20);
  if (iVar10 == 0) {
    return;
  }
  iVar11 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar10 + 0x14) * 4);
  if (0xf < *(byte *)(iVar10 + 0x53)) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0xe93,_DAT_ram_00014800);
  }
  if (0xf < *(byte *)(iVar10 + 0x52)) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0xe94,_DAT_ram_00014800);
  }
  if (*(char *)(iVar10 + 0x15) != '\x02') {
    return;
  }
  uVar1 = *(ushort *)(param_1 + 0x32);
  uVar12 = uVar1 >> 0xc & 1;
  iVar13 = *(int *)(param_1 + 8);
  if (*(byte *)(iVar10 + 0x38) == uVar12) {
    bVar2 = false;
  }
  else {
    *(char *)(iVar10 + 0x38) = (char)uVar12;
    if (*(char *)(iVar10 + 0x361) == '\0') {
      iVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
      if ((*(int *)(iVar10 + 0x368) != 0) &&
         (uVar7 = iVar3 - *(int *)(iVar10 + 0x368), _DAT_ram_f006eacc < uVar7)) {
        _DAT_ram_f006eacc = uVar7;
      }
      DAT_ram_f006ead0 = DAT_ram_f006ead0 + '\x01';
      if ((uVar1 >> 0xc & 1) != 0) {
        *(int *)(iVar10 + 0x368) = iVar3;
      }
    }
    bVar2 = *(char *)(iVar10 + 0x38) == '\x01';
    if (bVar2) {
      *(undefined1 *)(iVar10 + 0x3b) = 1;
      *(undefined1 *)(iVar10 + 0x3c) = 1;
    }
    puVar6 = *(undefined4 **)(iVar11 + 0x5c);
    if (puVar6 != (undefined4 *)(iVar11 + 0x5c)) {
      for (; puVar6 != (undefined4 *)(iVar11 + 0x5c); puVar6 = (undefined4 *)*puVar6) {
        if (*(char *)(puVar6 + 0xe) != '\0') {
          *(undefined1 *)(iVar11 + 0xbb) = 1;
          goto LAB_ram_f002e3ea;
        }
      }
      *(undefined1 *)(iVar11 + 0xbb) = 0;
      if (*(int *)(iVar11 + 0x1c) == 0) {
        FUN_ram_f0037050(*(undefined1 *)(iVar11 + 0x19));
      }
    }
LAB_ram_f002e3ea:
    if (*(int *)(iVar11 + 0x1c) == 0) {
      FUN_ram_f00377e0(iVar10,*(undefined1 *)(iVar10 + 0x38));
    }
  }
  iVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  if (*(char *)(iVar10 + 0x38) == '\0') {
    return;
  }
  if (*(int *)(iVar11 + 0x1c) != 0) {
    return;
  }
  uVar15 = FUN_ram_f002ad54(uVar1,*(undefined1 *)(param_1 + 0xf),*(undefined1 *)(iVar10 + 0x53));
  uVar7 = (uint)((ulonglong)uVar15 >> 0x20);
  if (((int)uVar15 == 0) || (bVar2)) {
    if ((uVar1 & 0xfc) != 0xa4) goto LAB_ram_f002e5bc;
    *(undefined4 *)(iVar10 + 0x40) = *(undefined4 *)(iVar13 + 0x24);
    *(undefined1 *)(iVar10 + 0x50) = 1;
    FUN_ram_f002ac70(iVar10);
    iVar13 = FUN_ram_f0036d28(iVar10,~*(byte *)(iVar10 + 0x52) & 0xf);
    if (iVar13 == 0) {
      if (*(char *)(iVar10 + 0x52) == '\0') {
        uVar7 = (uint)DAT_ram_f0064fba;
        DAT_ram_f0064fb8 = 1;
        if (uVar7 < 10) {
          piVar9 = (int *)&DAT_ram_f006d71c;
          uVar4 = 0;
          do {
            if (uVar7 <= uVar4) {
              DAT_ram_f0064fba = DAT_ram_f0064fba + 1;
              *(int *)(&DAT_ram_f006d71c + uVar7 * 4) = iVar10;
              break;
            }
            iVar13 = *piVar9;
            piVar9 = piVar9 + 1;
            uVar4 = uVar4 + 1 & 0xff;
          } while (iVar13 != iVar10);
        }
      }
      else {
        iVar13 = FUN_ram_f0036d28(iVar10);
        if (iVar13 == 0) {
          uVar7 = (uint)DAT_ram_f0064fba;
          DAT_ram_f0064fb8 = 1;
          if (uVar7 < 10) {
            piVar9 = (int *)&DAT_ram_f006d71c;
            uVar4 = 0;
            do {
              if (uVar7 <= uVar4) {
                DAT_ram_f0064fba = DAT_ram_f0064fba + 1;
                puVar5 = &DAT_ram_f006d71c;
                goto LAB_ram_f002e624;
              }
              iVar13 = *piVar9;
              piVar9 = piVar9 + 1;
              uVar4 = uVar4 + 1 & 0xff;
            } while (iVar13 != iVar10);
          }
        }
      }
      goto LAB_ram_f002e5bc;
    }
    puVar5 = &DAT_ram_f0064fb8;
  }
  else {
    if (((*(char *)(iVar11 + 0x9a) == '\0') || (*(char *)(iVar10 + 0x2aa) == '\0')) ||
       ((*(char *)(iVar10 + 0x3a) != '\0' &&
        (uVar4 = *(int *)(iVar10 + 0x44) * 0x8000 + 999999, uVar7 = uVar4 % 1000000,
        (int)((-0x3d8 - uVar4 / 1000000) + iVar3) < 0)))) goto LAB_ram_f002e5bc;
    uVar14 = *(undefined4 *)(iVar13 + 0x24);
    *(undefined1 *)(iVar10 + 0x50) = 1;
    *(undefined4 *)(iVar10 + 0x44) = uVar14;
    FUN_ram_f002ac70(iVar10,uVar7);
    if (*(byte *)(iVar10 + 0x54) < 4) {
      uVar8 = *(undefined1 *)(*(byte *)(iVar10 + 0x54) + 0xf0063650);
    }
    else {
      uVar8 = 2;
    }
    iVar13 = FUN_ram_f0036a34(iVar10,*(undefined1 *)(iVar10 + 0x52),uVar8);
    if (iVar13 == 0) {
      uVar7 = FUN_ram_f0025b3c();
      if (0xf < uVar7) {
        uVar7 = (uint)DAT_ram_f0064fb9;
        DAT_ram_f0064fb7 = 1;
        if (uVar7 < 10) {
          piVar9 = (int *)&DAT_ram_f006d6f4;
          uVar4 = 0;
          do {
            if (uVar7 <= uVar4) {
              DAT_ram_f0064fb9 = DAT_ram_f0064fb9 + 1;
              puVar5 = &DAT_ram_f006d6f4;
LAB_ram_f002e624:
              *(int *)(puVar5 + uVar7 * 4) = iVar10;
              break;
            }
            iVar13 = *piVar9;
            piVar9 = piVar9 + 1;
            uVar4 = uVar4 + 1 & 0xff;
          } while (iVar13 != iVar10);
        }
      }
      goto LAB_ram_f002e5bc;
    }
    puVar5 = &DAT_ram_f0064fb7;
  }
  *puVar5 = 0;
LAB_ram_f002e5bc:
  FUN_ram_f002ac70(iVar10);
  if (((*(char *)(iVar11 + 0x19) == '\x01') && (*(int *)(iVar11 + 0xc) == 2)) &&
     (iVar10 = FUN_ram_f0030b00(), iVar10 != 0)) {
    FUN_ram_f0031078();
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x960 <<<
// ===== FUN_ram_f002e64c @ 0xf002e64c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002e64c(int param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  code *pcVar3;
  int iVar4;
  
  iVar4 = *(int *)(param_1 + 0x58);
  *(int *)(param_1 + 0xa8) = param_2;
  if (iVar4 == 0) {
    FUN_ram_f0038d88(0xf006e0f0,0xf00635bc);
    _DAT_ram_f006e130 = 0x909;
    _DAT_ram_f006e134 = (*_DAT_ram_00014c14)(_DAT_ram_00014c14);
    (*(code *)&SUB_ram_0006a0b0)(0xf006e138,&DAT_ram_f0063bd8);
    FUN_ram_f0039800();
    (*(code *)&SUB_ram_0006a0a0)(0xf006e0f0,0,0x3c8);
    return;
  }
  if (param_2 == 1) {
    if (*(char *)(param_1 + 0xb3) != '\0') {
      (*_DAT_ram_00014800)(0,0,0xf00635bc,0x960,_DAT_ram_00014800);
    }
    if (*(char *)(param_1 + 0xb3) != '\0') {
      return;
    }
    *(undefined1 *)(iVar4 + 0x37) = 1;
    FUN_ram_f00347c4(1,0x7f,iVar4);
    if (*(char *)(param_1 + 0xb6) == '\0') {
      FUN_ram_f00228c0(iVar4,0,FUN_ram_f002fff8);
      *(undefined1 *)(param_1 + 0xb6) = 1;
    }
LAB_ram_f002e83c:
    FUN_ram_f00412b0(param_1,*(undefined4 *)(param_1 + 0xa8));
    *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(param_1 + 0x19) + 0x3d6) * 4 + 4) = 1;
  }
  else {
    if (param_2 == 0) {
      (*(code *)&SUB_ram_0006a0a0)(param_1 + 0xc0,0,0x10);
      *(undefined1 *)(param_1 + 0x1cc) = 0;
      *(undefined4 *)(param_1 + 0x1d0) = 0;
      *(undefined1 *)(param_1 + 0xb7) = 0;
      FUN_ram_f00303b4(param_1);
      FUN_ram_f00266d8(param_1 + 0xd0);
      FUN_ram_f00266d8(param_1 + 0x264);
      *(undefined1 *)(param_1 + 0x261) = 0;
      *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(param_1 + 0x19) + 0x3d6) * 4 + 4) = 1;
      if ((*(int *)(param_1 + 0x20c) == 2) && (*(int *)(param_1 + 0x118) == 0)) {
        *(undefined4 *)(param_1 + 0x1f8) = 0;
        *(undefined1 *)(param_1 + 0x214) = 0;
        *(undefined4 *)(param_1 + 0x1fc) = 0;
        uVar1 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
        *(undefined4 *)(param_1 + 0x200) = uVar1;
        FUN_ram_f002672c(param_1 + 0x118,*(undefined2 *)(param_1 + 0x1ea));
      }
      FUN_ram_f0020cb8(*(undefined1 *)(param_1 + 0x19));
      uVar1 = 0;
      uVar2 = 0;
      pcVar3 = _DAT_ram_00014b6c;
    }
    else {
      if (param_2 == 2) {
        *(undefined1 *)(iVar4 + 0x37) = 0;
        FUN_ram_f00347c4(1,0x7f,iVar4,0);
        FUN_ram_f00228c0(iVar4,0,FUN_ram_f002fff8);
        if ((*(uint *)(iVar4 + 0x10) & 0x80) != 0) {
          *(undefined1 *)(iVar4 + 0x378) = 0;
        }
        goto LAB_ram_f002e83c;
      }
      if (param_2 == 3) {
        *(undefined1 *)(param_1 + 0x1cc) = 0;
        *(undefined4 *)(param_1 + 0x1d0) = 0;
        *(undefined1 *)(param_1 + 0xbf) = 1;
        FUN_ram_f003041c();
        if ((*(int *)(param_1 + 0x20c) == 2) && (*(char *)(param_1 + 0xb0) == '\0')) {
          FUN_ram_f00266d8(param_1 + 0x118);
        }
        FUN_ram_f002672c(param_1 + 0x264,*(undefined1 *)(param_1 + 0x1dd));
        *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(param_1 + 0x19) + 0x3d6) * 4 + 4) = 1;
        *(undefined1 *)(param_1 + 0x261) = 1;
        if ((((*(int *)(param_1 + 0xa8) == 3) && (*(char *)(param_1 + 0xb7) == '\0')) &&
            (*(char *)(param_1 + 0xb8) == '\0')) &&
           ((iVar4 = FUN_ram_f002bda8(param_1), iVar4 == 0 && (*(char *)(param_1 + 0x261) == '\0')))
           ) {
          *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(param_1 + 0x19) + 0x3d6) * 4 + 4) = 2;
          FUN_ram_f002bdc8(*(undefined1 *)(param_1 + 0x19));
        }
        FUN_ram_f0020cf4(*(undefined1 *)(param_1 + 0x19));
        FUN_ram_f002afbc();
        return;
      }
      uVar1 = 0xf00635bc;
      uVar2 = 0xa13;
      pcVar3 = _DAT_ram_00014800;
    }
    (*pcVar3)(0,0,uVar1,uVar2,pcVar3);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002e94c @ 0xf002e94c =====


undefined4 FUN_ram_f002e94c(int param_1)

{
  undefined4 uVar1;
  
  if (*(char *)(param_1 + 0xb4) == '\0') {
    if (*(int *)(param_1 + 0xa8) == 0) {
      uVar1 = 0;
    }
    else {
      uVar1 = 0x103;
    }
  }
  else {
    *(undefined1 *)(param_1 + 0xb4) = 0;
    FUN_ram_f002e64c(param_1,2);
    uVar1 = 0x103;
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002ea68 @ 0xf002ea68 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined1 FUN_ram_f002ea68(int param_1)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  
  if ((((*(int *)(param_1 + 0x20) != 0) && ((**(ushort **)(param_1 + 0x14) & 0x400) == 0)) &&
      (uVar1 = **(ushort **)(param_1 + 0x14) & 0xfc, uVar1 != 0x48)) && (uVar1 != 200)) {
    iVar7 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(*(int *)(param_1 + 0x20) + 0x14) * 4);
    if ((*(int *)(iVar7 + 0x20c) == 2) &&
       (iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830), *(char *)(iVar7 + 0xb0) == '\0')) {
      if (*(int *)(iVar7 + 0xa8) != 3) {
        *(int *)(iVar7 + 0x1fc) = *(int *)(iVar7 + 0x1fc) + 1;
        return 0;
      }
      uVar5 = (uint)*(ushort *)(iVar7 + 0x1e8);
      uVar6 = (uVar5 / 100) * (uint)*(ushort *)(iVar7 + 0x1e0);
      if (uVar6 != 0) {
        uVar3 = iVar2 * 1000 + 0x7fffU >> 0xf;
        uVar4 = uVar3 / uVar5;
        uVar5 = uVar3 % uVar5;
        if (*(uint *)(iVar7 + 500) != uVar4) {
          *(uint *)(iVar7 + 500) = uVar4;
          *(undefined4 *)(iVar7 + 0x1fc) = 1;
          *(undefined4 *)(iVar7 + 0x1f8) = 0;
          return 0;
        }
        uVar4 = *(int *)(iVar7 + 0x1fc) + 1;
        *(uint *)(iVar7 + 0x1fc) = uVar4;
        if (uVar4 <= uVar6) {
          return 0;
        }
      }
      FUN_ram_f002e94c(iVar7,0,uVar5);
      *(undefined1 *)(iVar7 + 0x214) = 1;
      return 1;
    }
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002eb2c @ 0xf002eb2c =====


undefined4 FUN_ram_f002eb2c(int param_1)

{
  if (*(char *)(param_1 + 0xb4) == '\0') {
    if (*(char *)(param_1 + 0xb3) == '\0') {
      *(undefined1 *)(param_1 + 0xb4) = 1;
      FUN_ram_f002e64c(param_1,1);
    }
  }
  else if (*(int *)(param_1 + 0xa8) == 3) {
    return 0;
  }
  return 0x103;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002eb6c @ 0xf002eb6c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002eb6c(byte param_1,byte param_2)

{
  int *piVar1;
  uint uVar2;
  uint uVar3;
  undefined4 uVar4;
  
  uVar3 = (uint)param_1;
  piVar1 = *(int **)(&DAT_ram_020a0068 + uVar3 * 4);
  uVar2 = (uint)param_2;
  if (piVar1[0x83] == uVar2) {
    return;
  }
  piVar1[0x83] = uVar2;
  if (*piVar1 != 0) {
    return;
  }
  if (uVar2 == 1) {
    if (uVar3 == 0) {
      if (piVar1[3] != 0) {
        if (piVar1[3] != 2) {
          return;
        }
        uVar4 = 0x1530;
        goto LAB_ram_f002ec1a;
      }
    }
    else {
      if (uVar3 != 1) {
        return;
      }
      if (piVar1[3] != 0) {
        return;
      }
      if (piVar1[7] != 0) {
        return;
      }
    }
    FUN_ram_f002eb2c();
  }
  else {
    if (uVar2 != 0) {
      if (uVar2 != 2) {
        return;
      }
      if ((uVar3 != 0) && (uVar3 != 1)) {
        return;
      }
      if (piVar1[3] != 0) {
        return;
      }
      FUN_ram_f002672c(piVar1 + 0x46,*(undefined2 *)((int)piVar1 + 0x1ea));
      return;
    }
    if (uVar3 == 0) {
      if (piVar1[3] != 0) {
        if (piVar1[3] != 2) {
          return;
        }
        uVar4 = 0x156d;
LAB_ram_f002ec1a:
        (*_DAT_ram_00014800)(uVar3,uVar3,0xf00635bc,uVar4,_DAT_ram_00014800);
        return;
      }
    }
    else {
      if (uVar3 != 1) {
        return;
      }
      if (piVar1[3] != 0) {
        if (piVar1[3] != 2) {
          return;
        }
        FUN_ram_f002cd78(1);
        return;
      }
      if (piVar1[7] != 0) {
        return;
      }
    }
    FUN_ram_f002e94c();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002ecac @ 0xf002ecac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002ecac(int param_1)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  
  iVar6 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(*(int *)(param_1 + 0x20) + 0x14) * 4);
  if (*(int *)(iVar6 + 0x20c) == 2) {
    if (((((*(ushort *)(param_1 + 0x32) & 0x400) == 0) &&
         (uVar1 = *(ushort *)(param_1 + 0x32) & 0xfc, uVar1 != 0x48)) && (uVar1 != 200)) &&
       (((*(byte *)(*(int *)(param_1 + 0x14) + 4) & 1) == 0 && (*(char *)(iVar6 + 0xb0) == '\0'))))
    {
      iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
      uVar3 = (iVar2 * 1000 + 0x7fffU >> 0xf) / (uint)*(ushort *)(iVar6 + 0x1e8);
      uVar4 = 0;
      if (*(int *)(iVar6 + 0xa8) != 3) {
        *(uint *)(iVar6 + 500) = uVar3;
        *(int *)(iVar6 + 0x1f8) = *(int *)(iVar6 + 0x1f8) + 1;
        return 0;
      }
      if (*(uint *)(iVar6 + 500) == uVar3) {
        uVar3 = *(int *)(iVar6 + 0x1f8) + 1;
        *(uint *)(iVar6 + 0x1f8) = uVar3;
        if ((*(ushort *)(iVar6 + 0x1e8) / 100) * (uint)*(ushort *)(iVar6 + 0x1e4) < uVar3) {
          if ((*(char *)(iVar6 + 0x19) == '\0') && (*(char *)(param_1 + 0x3d) == '\x01')) {
            if ((*(ushort *)(param_1 + 0x38) & 0x60) == 0x20) {
              FUN_ram_f002eb6c(0,0);
              uVar4 = 1;
            }
            else {
              uVar4 = 1;
            }
          }
          else {
            FUN_ram_f002e94c(iVar6);
            *(undefined1 *)(iVar6 + 0x214) = 1;
            uVar4 = 1;
          }
        }
      }
      else {
        *(uint *)(iVar6 + 500) = uVar3;
        *(undefined4 *)(iVar6 + 0x1f8) = 1;
        *(undefined4 *)(iVar6 + 0x1fc) = 0;
      }
      uVar3 = (uint)*(byte *)(iVar6 + 0x72) * (uint)*(ushort *)(iVar6 + 0x6a) * 0x8000 + 0xa03e7;
      uVar5 = uVar3 % 1000;
      if ((int)((iVar2 - uVar3 / 1000) - *(int *)(iVar6 + 0x204)) < 0) {
        uVar3 = *(int *)(iVar6 + 0x208) + 1;
        *(uint *)(iVar6 + 0x208) = uVar3;
        if (3 < uVar3) {
          if (*(char *)(iVar6 + 0x19) == '\0') {
            uVar5 = (uint)*(byte *)(param_1 + 0x3d);
            uVar4 = 1;
            if (uVar5 == 1) {
              if ((*(ushort *)(param_1 + 0x38) & 0x60) == 0x20) {
                FUN_ram_f002eb6c(0,0);
                uVar4 = 1;
              }
              else {
                uVar4 = 1;
              }
              goto LAB_ram_f002ee34;
            }
          }
          FUN_ram_f002e94c(iVar6,uVar4,uVar5);
          *(undefined1 *)(iVar6 + 0x214) = 1;
          uVar4 = 1;
        }
      }
      else {
        *(undefined4 *)(iVar6 + 0x208) = 0;
      }
LAB_ram_f002ee34:
      *(int *)(iVar6 + 0x204) = iVar2;
      return uVar4;
    }
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002ee54 @ 0xf002ee54 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002ee54(int param_1,int param_2)

{
  ushort uVar1;
  char cVar2;
  int iVar3;
  byte bVar4;
  int *piVar5;
  int iVar6;
  uint uVar7;
  
  iVar6 = *(int *)(param_1 + 0x20);
  if (*(char *)(param_2 + 0xb0) != '\0') {
    return;
  }
  _DAT_ram_f0064f44 = _DAT_ram_f0064f44 + 1;
  if (*(int *)(param_2 + 0x1c) != 0) {
    return;
  }
  if ((*(int *)(param_2 + 0x20c) == 2) ||
     ((*(char *)(param_2 + 0x1ca) != '\0' && (*(char *)(param_2 + 0x1cb) == '\x02')))) {
    uVar7 = (uint)*(byte *)(param_2 + 0x72);
    uVar1 = *(ushort *)(param_2 + 0x6a);
    if (uVar7 == 1) {
      uVar7 = 3;
    }
    iVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    if ((int)((iVar3 - *(int *)(param_2 + 0x1d0)) -
             ((((uVar1 * uVar7 + 10) * 0x400 + 999) / 1000) * 0x8000 + 999) / 1000) < 0) {
      cVar2 = *(char *)(param_2 + 0x1cc) + '\x01';
    }
    else {
      cVar2 = '\0';
    }
    *(char *)(param_2 + 0x1cc) = cVar2;
    *(int *)(param_2 + 0x1d0) = iVar3;
    if (2 < *(byte *)(param_2 + 0x1cc)) {
      FUN_ram_f002e94c(param_2);
      return;
    }
  }
  if (*(char *)(param_2 + 0x9a) == '\0') {
    if (*(char *)(param_2 + 0xb7) == '\0') {
      FUN_ram_f002e1ec(param_2);
    }
  }
  else {
    if ((*(char *)(iVar6 + 0x2aa) == '\0') || (*(char *)(param_2 + 0x1c8) != '\x0f')) {
      if (*(char *)(param_2 + 0xb7) != '\0') goto LAB_ram_f002ef92;
      FUN_ram_f002e1ec(param_2);
      piVar5 = (int *)&DAT_ram_f0064f3c;
    }
    else {
      bVar4 = *(byte *)(param_2 + 0x1c9) & 8;
      if ((*(byte *)(param_2 + 0x1c9) & 8) == 0) {
        (*_DAT_ram_00014800)(bVar4,bVar4,0xf00635bc,0xae2,_DAT_ram_00014800);
      }
      if ((*(char *)(param_2 + 0x1cb) != '\0') || (*(char *)(param_2 + 0xb8) != '\0'))
      goto LAB_ram_f002ef92;
      FUN_ram_f0022800(iVar6,7,0,0,0);
      piVar5 = (int *)&DAT_ram_f0064f40;
    }
    *piVar5 = *piVar5 + 1;
  }
LAB_ram_f002ef92:
  FUN_ram_f002ecac(param_1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002efa8 @ 0xf002efa8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002efa8(int param_1)

{
  short sVar1;
  uint uVar2;
  int iVar3;
  ushort *puVar4;
  int *piVar5;
  int iVar6;
  char *local_20;
  ushort local_1a;
  
  iVar3 = _DAT_ram_020a0064;
  piVar5 = *(int **)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4);
  iVar6 = *(int *)(param_1 + 0x14);
  sVar1 = *(short *)(param_1 + 0x12);
  if ((piVar5[0x16] == *(int *)(param_1 + 0x20)) && (*(int *)(param_1 + 0x20) != 0)) {
    if ((piVar5[3] == 0) &&
       ((((piVar5[0x2a] == 3 || (piVar5[0x2a] == 1)) && (*(int *)(_DAT_ram_020a0064 + 0x380) == 3))
        && ((*(byte *)(*(int *)(param_1 + 8) + 0x10) & 0x20) != 0)))) {
      FUN_ram_f002ee54(param_1,piVar5);
    }
    iVar6 = FUN_ram_f002a228(iVar6 + 0x24,sVar1 + -0xc,&local_20,&local_1a);
    if (iVar6 == 0) {
      for (uVar2 = 0; uVar2 < local_1a; uVar2 = uVar2 + *puVar4 + 3) {
        puVar4 = (ushort *)(local_20 + 1);
        if (*local_20 == '\f') {
          if (*piVar5 == 0) {
            FUN_ram_f002d318(*(undefined1 *)((int)piVar5 + 0x19));
          }
          goto LAB_ram_f002f0a2;
        }
        local_20 = local_20 + *puVar4 + 3;
      }
    }
    if (((*(char *)(param_1 + 0xd) == '\x01') && (piVar5[0x16] == *(int *)(param_1 + 0x20))) &&
       (*(int *)(param_1 + 0x20) != 0)) {
      if (*(char *)(iVar3 + 0x1f04) != '\0') {
        FUN_ram_f002ccb8();
      }
      iVar3 = FUN_ram_f002b140(*(undefined1 *)(param_1 + 0xd));
      if (iVar3 == 0) {
        FUN_ram_f002c928(*(undefined1 *)(param_1 + 0xd));
      }
    }
LAB_ram_f002f0a2:
    if ((*(char *)((int)piVar5 + 0x28a) != '\0') && (iVar3 = FUN_ram_f002b1b0(piVar5), iVar3 == 0))
    {
      (*(code *)&SUB_ram_0006a0d8)(param_1);
      return 1;
    }
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002f2d8 @ 0xf002f2d8 =====


void FUN_ram_f002f2d8(undefined4 param_1)

{
  FUN_ram_f0020d4c();
  FUN_ram_f0037300(param_1);
  FUN_ram_f00237f8(param_1);
  FUN_ram_f0040f4c(param_1);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x2691 <<<
// ===== FUN_ram_f002f314 @ 0xf002f314 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002f314(int param_1)

{
  int iVar1;
  
  if (param_1 != 2) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x2691,_DAT_ram_00014800);
  }
  iVar1 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  *(undefined4 *)(iVar1 + 0x1c) = 0;
  *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(iVar1 + 0x19) + 0x3d6) * 4 + 4) = 1;
  FUN_ram_f002f2d8(param_1);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x20ea <<<
// ===== FUN_ram_f002f374 @ 0xf002f374 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002f374(int param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x20ea,_DAT_ram_00014800);
  }
  piVar2 = *(int **)(&DAT_ram_020a0068 + param_1 * 4);
  piVar2[7] = 0;
  iVar1 = _DAT_ram_020a0064;
  iVar3 = piVar2[0x16];
  if (piVar2[3] != 0) {
    if (piVar2[3] == 2) {
      *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)((int)piVar2 + 0x19) + 0x3d6) * 4 + 4) = 1;
    }
    goto LAB_ram_f002f480;
  }
  if (*piVar2 != 0) goto LAB_ram_f002f480;
  *(undefined1 *)(_DAT_ram_020a0064 + (uint)*(byte *)((int)piVar2 + 0x19) + 0x1f71) = 1;
  iVar1 = iVar1 + (uint)*(byte *)((int)piVar2 + 0x19);
  if ((*(char *)(iVar1 + 0x1f6e) != '\0') && (*(char *)(iVar1 + 0x1f71) != '\0')) {
    FUN_ram_f004aea8(piVar2);
  }
  if ((char)piVar2[0x2f] != '\0') {
    FUN_ram_f002eb2c(piVar2);
    *(undefined1 *)(piVar2 + 0x2f) = 0;
    return;
  }
  iVar1 = piVar2[0x83];
  if (iVar1 == 1) {
    FUN_ram_f002eb2c(piVar2);
LAB_ram_f002f42c:
    FUN_ram_f002672c(piVar2 + 0x46,*(undefined2 *)((int)piVar2 + 0x1ea));
  }
  else if (iVar1 == 0) {
    FUN_ram_f002e94c(piVar2);
  }
  else if (iVar1 == 2) goto LAB_ram_f002f42c;
  if (*(char *)((int)piVar2 + 0xbd) == '\x01') {
    *(undefined1 *)((int)piVar2 + 0xbd) = 0;
    FUN_ram_f00228c0(iVar3,0,&LAB_ram_f00233a0);
  }
LAB_ram_f002f480:
  FUN_ram_f002f2d8(param_1);
  *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)((int)piVar2 + 0x19) + 0x3d6) * 4 + 4) = 1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002f4ac @ 0xf002f4ac =====


void FUN_ram_f002f4ac(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  FUN_ram_f0020d88();
  FUN_ram_f0037844(param_1);
  FUN_ram_f00237f4(param_1);
  FUN_ram_f00266d8(iVar1 + 0x178);
  FUN_ram_f00410f4(param_1);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x266e <<<
// ===== FUN_ram_f002f504 @ 0xf002f504 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002f504(int param_1)

{
  int iVar1;
  
  if (param_1 != 2) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x266e,_DAT_ram_00014800);
  }
  iVar1 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  *(undefined4 *)(iVar1 + 0x1c) = 1;
  *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(iVar1 + 0x19) + 0x3d6) * 4 + 4) = 2;
  FUN_ram_f002bdc8(*(undefined1 *)(iVar1 + 0x19));
  FUN_ram_f002f4ac(param_1);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0x209e <<<
// ===== FUN_ram_f002f574 @ 0xf002f574 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002f574(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (param_1 != 1) {
    (*_DAT_ram_00014800)(0,0,0xf00635bc,0x209e,_DAT_ram_00014800);
  }
  iVar2 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
  *(undefined4 *)(iVar2 + 0x1c) = 1;
  iVar1 = _DAT_ram_020a0064;
  if (*(int *)(iVar2 + 0xc) == 0) {
    *(undefined1 *)(_DAT_ram_020a0064 + (uint)*(byte *)(iVar2 + 0x19) + 0x1f71) = 0;
    iVar1 = iVar1 + (uint)*(byte *)(iVar2 + 0x19);
    if ((*(char *)(iVar1 + 0x1f6e) == '\0') || (*(char *)(iVar1 + 0x1f71) == '\0')) {
      FUN_ram_f004a7ac(iVar2);
    }
    if ((((*(int *)(iVar2 + 0xa8) != 3) || (*(char *)(iVar2 + 0xb7) != '\0')) ||
        (*(char *)(iVar2 + 0xb8) != '\0')) ||
       ((iVar1 = FUN_ram_f002bda8(iVar2), iVar1 != 0 || (*(char *)(iVar2 + 0x261) != '\0'))))
    goto LAB_ram_f002f648;
    iVar1 = _DAT_ram_020a0064 + (*(byte *)(iVar2 + 0x19) + 0x3d6) * 4;
  }
  else {
    if (*(int *)(iVar2 + 0xc) != 2) goto LAB_ram_f002f648;
    iVar1 = _DAT_ram_020a0064 + (*(byte *)(iVar2 + 0x19) + 0x3d6) * 4;
  }
  *(undefined4 *)(iVar1 + 4) = 2;
  FUN_ram_f002bdc8(*(undefined1 *)(iVar2 + 0x19));
LAB_ram_f002f648:
  FUN_ram_f002f4ac(param_1);
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0xbe9 <<<
// ===== FUN_ram_f002f8dc @ 0xf002f8dc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002f8dc(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
  if (*(short *)(iVar2 + 0x1c0) != 0) {
    if (*(int *)(iVar2 + 0x58) != param_1) {
      (*_DAT_ram_00014800)(0,0,0xf00635bc,0xbe9,_DAT_ram_00014800);
    }
    if ((*(char *)(iVar2 + 0x18) != '\0') && (*(int *)(iVar2 + 0xa8) == 3)) {
      if (*(char *)(iVar2 + 0x9a) != '\0') {
        if ((*(int *)(iVar2 + 0xc4) - *(int *)(iVar2 + 0xcc) < 0) &&
           (*(char *)(iVar2 + 0xbe) == '\0')) {
          if (*(char *)(iVar2 + 0xb8) == '\x01') {
            *(undefined1 *)(iVar2 + 0xb8) = 0;
            FUN_ram_f00266d8(iVar2 + 0xe8);
          }
        }
        else if (((*(int *)(iVar2 + 0xc4) != 0) || (*(int *)(iVar2 + 0xcc) != 0)) &&
                (*(char *)(iVar2 + 0xb8) == '\0')) {
          *(undefined1 *)(iVar2 + 0xbe) = 0;
          *(undefined1 *)(iVar2 + 0xb8) = 1;
          FUN_ram_f002672c(iVar2 + 0xe8,1000);
        }
      }
      if ((*(int *)(iVar2 + 0xc0) - *(int *)(iVar2 + 200) < 0) && (*(char *)(iVar2 + 0xbf) == '\0'))
      {
        if (*(char *)(iVar2 + 0xb7) == '\x01') {
          *(undefined1 *)(iVar2 + 0xb7) = 0;
          FUN_ram_f00266d8(iVar2 + 0xd0);
        }
      }
      else if (((*(int *)(iVar2 + 0xc0) != 0) || (*(int *)(iVar2 + 200) != 0)) &&
              (*(char *)(iVar2 + 0xb7) == '\0')) {
        *(undefined1 *)(iVar2 + 0xbf) = 0;
        *(undefined1 *)(iVar2 + 0xb7) = 1;
        FUN_ram_f002672c(iVar2 + 0xd0,1000);
      }
      if (((*(int *)(iVar2 + 0xa8) == 3) && (*(char *)(iVar2 + 0xb7) == '\0')) &&
         ((*(char *)(iVar2 + 0xb8) == '\0' &&
          ((iVar1 = FUN_ram_f002bda8(iVar2), iVar1 == 0 && (*(char *)(iVar2 + 0x261) == '\0')))))) {
        *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(iVar2 + 0x19) + 0x3d6) * 4 + 4) = 2;
        FUN_ram_f002bdc8(*(undefined1 *)(iVar2 + 0x19));
        FUN_ram_f0020c1c(1);
        if (*(char *)(iVar2 + 0xb1) != '\0') {
          *(undefined4 *)(iVar2 + 0x1c) = 1;
          FUN_ram_f002f4ac(*(undefined1 *)(iVar2 + 0x19));
          *(undefined1 *)(iVar2 + 0xb1) = 0;
        }
      }
      else {
        FUN_ram_f0020c1c(0);
        *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(iVar2 + 0x19) + 0x3d6) * 4 + 4) = 1;
      }
    }
    *(undefined1 *)(iVar2 + 0x1c1) = 0;
    *(undefined1 *)(iVar2 + 0x1c0) = 0;
    return;
  }
  return;
}

// >>> MOD: wifi/mgmt/pwr_mgt_fsm.c:0xdd4 <<<
// ===== FUN_ram_f002fa8c @ 0xf002fa8c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002fa8c(int param_1)

{
  ushort uVar1;
  ushort uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  undefined4 auStack_24 [2];
  
  iVar7 = *(int *)(param_1 + 0x20);
  if (iVar7 == 0) {
    return;
  }
  piVar6 = *(int **)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar7 + 0x14) * 4);
  if (*piVar6 != 0) {
    return;
  }
  iVar5 = *(int *)(param_1 + 8);
  iVar8 = *(int *)(param_1 + 0x14);
  uVar1 = *(ushort *)(param_1 + 0x32);
  FUN_ram_f00412dc(piVar6,iVar7,param_1);
  if (*(int *)(param_1 + 0x20) != piVar6[0x16]) {
    return;
  }
  if (*(int *)(param_1 + 0x20) == 0) {
    return;
  }
  if ((*(byte *)(iVar8 + 4) & 1) == 0) {
    iVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    piVar6[0x71] = iVar3;
  }
  if (piVar6[0x2a] != 3) {
    if (piVar6[0x2a] != 0) {
      return;
    }
    if (((uVar1 & 0xfc) != 8) && ((uVar1 & 0xfc) != 0x88)) {
      return;
    }
    FUN_ram_f002ecac(param_1);
    return;
  }
  uVar2 = uVar1 & 0xfc;
  if (uVar2 == 0x80) {
    return;
  }
  if ((*(char *)((int)piVar6 + 0x9a) != '\0') && (*(char *)(iVar7 + 0x2aa) != '\0')) {
    if ((uVar2 == 200) || (uVar2 == 0x88)) {
      iVar3 = FUN_ram_f002ad8c(*(undefined1 *)(param_1 + 0xf),(char)piVar6[0x72]);
      if (iVar3 != 0) {
        if (*(char *)((int)piVar6 + 0x1c9) == '\0') {
          (*_DAT_ram_00014800)(0,0,0xf00635bc,0xdd4,_DAT_ram_00014800);
        }
        uVar2 = *(ushort *)(iVar8 + 0x18);
        if ((uVar2 & 0x10) != 0) {
          iVar5 = *(int *)(iVar5 + 0x24);
          *(undefined1 *)(piVar6 + 0x70) = 1;
          piVar6[0x33] = iVar5;
        }
        if (((*(char *)((int)piVar6 + 0xb1) == '\0') && (piVar6[7] != 1)) &&
           (((iVar5 = FUN_ram_f002ecac(param_1), iVar5 == 0 &&
             (((*(byte *)(iVar8 + 4) & 1) == 0 && ((uVar2 & 0x10) != 0)))) &&
            ((uVar1 & 0x2400) == 0x2000)))) {
          auStack_24[0] = _DAT_ram_f006364c;
          uVar4 = 3;
          do {
            if ((*(byte *)((int)piVar6 + 0x1c9) >> (uVar4 & 0x1f) & 1) != 0) {
              FUN_ram_f0022800(iVar7,*(undefined1 *)((int)auStack_24 + uVar4),0,0,0);
              piVar6 = (int *)&DAT_ram_f0064f40;
              goto LAB_ram_f002fc6e;
            }
            uVar4 = uVar4 - 1;
          } while (uVar4 != 0xffffffff);
        }
        goto LAB_ram_f002fc74;
      }
    }
    else if ((uVar1 & 0xc) == 0) {
      return;
    }
  }
  if (((*(byte *)(iVar8 + 4) & 1) == 0) &&
     (((((uVar1 & 0xc) == 0 || ((uVar1 & 0xc) == 8)) && ((uVar1 & 0x400) == 0)) &&
      ((*(char *)((int)piVar6 + 0xba) != '\0' || (*(char *)((int)piVar6 + 0xb7) != '\0')))))) {
    piVar6[0x32] = *(int *)(*(int *)(param_1 + 8) + 0x24);
    *(undefined1 *)(piVar6 + 0x70) = 1;
  }
  if ((((*(char *)((int)piVar6 + 0xb1) == '\0') && (piVar6[7] != 1)) &&
      ((iVar5 = FUN_ram_f002ecac(param_1), iVar5 == 0 && ((*(byte *)(iVar8 + 4) & 1) == 0)))) &&
     ((((uVar1 & 0xc) == 0 || ((uVar1 & 0xc) == 8)) && ((uVar1 & 0x2400) == 0x2000)))) {
    FUN_ram_f002e1ec(piVar6);
    piVar6 = (int *)&DAT_ram_f0064f3c;
LAB_ram_f002fc6e:
    *piVar6 = *piVar6 + 1;
  }
LAB_ram_f002fc74:
  FUN_ram_f002f8dc(iVar7);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002fcb8 @ 0xf002fcb8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002fcb8(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0x20) == 0) {
    return;
  }
  iVar1 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(*(int *)(param_1 + 0x20) + 0x14) * 4);
  if (*(char *)(iVar1 + 0x19) == '\0') {
    iVar1 = *(int *)(iVar1 + 0xc);
    if (iVar1 == 1) {
      return;
    }
    if (iVar1 == 0) {
LAB_ram_f002fd18:
      FUN_ram_f002fa8c();
      return;
    }
    if (iVar1 != 2) {
      uVar2 = 0xb6a;
LAB_ram_f002fd48:
      (*_DAT_ram_00014800)(0,0,0xf00635bc,uVar2,_DAT_ram_00014800);
      return;
    }
  }
  else {
    if (*(char *)(iVar1 + 0x19) != '\x01') {
      return;
    }
    iVar1 = *(int *)(iVar1 + 0xc);
    if (iVar1 == 1) {
      return;
    }
    if (iVar1 == 0) goto LAB_ram_f002fd18;
    if (iVar1 != 2) {
      if (iVar1 == 3) {
        return;
      }
      uVar2 = 0xb7b;
      goto LAB_ram_f002fd48;
    }
  }
  FUN_ram_f002e2d0();
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002fd54 @ 0xf002fd54 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f002fd54(int param_1)

{
  int iVar1;
  int *piVar2;
  
  piVar2 = *(int **)(&DAT_ram_020a0068 + param_1 * 4);
  *(undefined1 *)(piVar2 + 0x2c) = 1;
  if ((*piVar2 != 0) || (piVar2[3] != 0)) {
LAB_ram_f002fe02:
    piVar2[7] = 1;
    FUN_ram_f002f4ac(*(undefined1 *)((int)piVar2 + 0x19));
    return 1;
  }
  if (piVar2[7] == 1) {
    return 1;
  }
  FUN_ram_f00266d8(piVar2 + 0x58);
  (*_DAT_ram_00014cb0)(*(undefined1 *)((int)piVar2 + 0x346),1,_DAT_ram_00014cb0);
  if (*(char *)((int)piVar2 + 0xb3) == '\0') {
    piVar2[0x2b] = piVar2[0x2a];
    if (piVar2[0x2a] == 3) {
      if ((((*(char *)((int)piVar2 + 0xb7) == '\0') && ((char)piVar2[0x2e] == '\0')) &&
          (iVar1 = FUN_ram_f002bd88(piVar2), iVar1 == 0)) &&
         (*(char *)((int)piVar2 + 0x261) == '\0')) goto LAB_ram_f002fe02;
    }
    else {
      if (param_1 == 1) {
        FUN_ram_f002d594(1,1);
      }
      if (piVar2[7] != 0) {
        *(undefined1 *)((int)piVar2 + 0xb1) = 1;
        *(undefined1 *)(piVar2 + 0x2f) = 1;
        goto LAB_ram_f002fe18;
      }
      iVar1 = FUN_ram_f002eb2c(piVar2);
      if (iVar1 != 0x103) goto LAB_ram_f002fe02;
    }
    *(undefined1 *)((int)piVar2 + 0xb1) = 1;
  }
LAB_ram_f002fe18:
  FUN_ram_f002672c(piVar2 + 0x5e,(char)piVar2[0x7c]);
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002fe3c @ 0xf002fe3c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f002fe3c(int param_1)

{
  int iVar1;
  
  *(undefined1 *)(param_1 + 0x1d9) = 1;
  *(undefined2 *)(param_1 + 0x1e2) = 1;
  *(undefined2 *)(param_1 + 0x1e4) = 3;
  *(undefined2 *)(param_1 + 0x1e8) = 100;
  *(undefined2 *)(param_1 + 0x1ea) = 500;
  *(undefined1 *)(param_1 + 0x1db) = 0x1e;
  *(undefined1 *)(param_1 + 0x1dd) = 5;
  *(undefined2 *)(param_1 + 0x1de) = 10;
  *(undefined2 *)(param_1 + 0x1e6) = 5;
  *(undefined1 *)(param_1 + 0x1d8) = 0xf;
  *(undefined1 *)(param_1 + 0x1f0) = 7;
  *(undefined1 *)(param_1 + 0x1d7) = 0xf;
  *(undefined2 *)(param_1 + 0x1e0) = 0;
  iVar1 = _DAT_ram_020a0064;
  if (*(char *)(param_1 + 0x19) == '\x01') {
    *(undefined4 *)(_DAT_ram_020a0064 + 0x1980) = 0x7800;
    *(undefined1 *)(iVar1 + 0x1aa0) = 0;
    FUN_ram_f00266a0(iVar1 + 0x1a88,&LAB_ram_f002a21c,iVar1 + 0x1670);
  }
  *(undefined1 *)(param_1 + 0x1c8) = 0xf;
  *(undefined1 *)(param_1 + 0x1c9) = 0xf;
  *(undefined4 *)(param_1 + 0x20c) = 0;
  (*_DAT_ram_00014988)(0x10000,0,_DAT_ram_00014988);
  (*_DAT_ram_00014988)(0x20000,0,_DAT_ram_00014988);
  (*_DAT_ram_00014988)(0,0x40000,_DAT_ram_00014988);
  FUN_ram_f00266a0(param_1 + 0x218,&LAB_ram_f003147c,param_1);
  FUN_ram_f00266a0(param_1 + 0x230,&LAB_ram_f002e988,param_1);
  FUN_ram_f00266a0(param_1 + 0x248,&LAB_ram_f002e988,param_1);
  FUN_ram_f00266a0(param_1 + 0x264,&LAB_ram_f002f694,param_1);
  FUN_ram_f00266a0(param_1 + 0x118,&LAB_ram_f002f21c,param_1);
  FUN_ram_f00266a0(param_1 + 0xe8,&LAB_ram_f002f724,param_1);
  FUN_ram_f00266a0(param_1 + 0xd0,&LAB_ram_f002f800,param_1);
  FUN_ram_f00266a0(param_1 + 0x100,&LAB_ram_f002f0d4,param_1);
  FUN_ram_f00266a0(param_1 + 0x130,&LAB_ram_f002c314,param_1);
  FUN_ram_f00266a0(param_1 + 0x178,&LAB_ram_f002f65c,param_1);
  FUN_ram_f00266a0(param_1 + 0x148,&LAB_ram_f002d524,param_1);
  FUN_ram_f00266a0(param_1 + 0x160,&LAB_ram_f002c028,param_1);
  FUN_ram_f00266a0(param_1 + 0x1a8,&LAB_ram_f002b470,param_1);
  (*_DAT_ram_00014c88)(0,0,_DAT_ram_00014c88);
  (*_DAT_ram_00014c78)(0,0,_DAT_ram_00014c78);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f002fff8 @ 0xf002fff8 =====


undefined4 FUN_ram_f002fff8(int param_1)

{
  ushort uVar1;
  ushort uVar2;
  int iVar3;
  int *piVar4;
  
  iVar3 = *(int *)(param_1 + 0x20);
  if (iVar3 == 0) {
    return 0;
  }
  if (*(char *)(iVar3 + 9) == '\0') {
    return 0;
  }
  piVar4 = *(int **)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4);
  if (*piVar4 != 0) {
    return 0;
  }
  if (piVar4[3] == 0) {
    uVar1 = **(ushort **)(param_1 + 0x14);
    if ((*(uint *)(iVar3 + 0x10) & 0x80) != 0) {
      *(undefined1 *)(iVar3 + 0x378) = 0;
    }
    uVar2 = uVar1 & 0x1000;
    if ((char)piVar4[0x2d] == '\0') {
      if ((uVar1 & 0x1000) != 0) {
        return 0;
      }
      if (piVar4[0x2a] != 2) {
        return 0;
      }
    }
    else {
      if ((uVar1 & 0x1000) == 0) {
        return 0;
      }
      if (piVar4[0x2a] != 1) {
        return 0;
      }
      if ((*(uint *)(iVar3 + 0x10) & 0x80) == 0) {
        *(undefined1 *)((int)piVar4 + 0xb6) = 0;
      }
      if (*(char *)((int)piVar4 + 0xb6) != '\0') {
        return 0;
      }
      iVar3 = FUN_ram_f0040e78(piVar4);
      if (iVar3 != 1) {
        return 0;
      }
      uVar2 = 3;
    }
    FUN_ram_f002e64c(piVar4,uVar2);
    return 0;
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003008c @ 0xf003008c =====


void FUN_ram_f003008c(int param_1)

{
  ushort uVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  int iVar5;
  ushort *puVar6;
  
  iVar5 = *(int *)(param_1 + 0x20);
  if (iVar5 != 0) {
    piVar4 = *(int **)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar5 + 0x14) * 4);
    if ((*piVar4 == 0) && (iVar2 = FUN_ram_f00413b4(param_1,piVar4,iVar5), iVar2 != 0)) {
      if ((*(int *)(param_1 + 0x20) == piVar4[0x16]) && (*(int *)(param_1 + 0x20) != 0)) {
        FUN_ram_f002fff8(param_1,0);
        puVar6 = *(ushort **)(param_1 + 0x14);
        uVar1 = *puVar6;
        if ((uVar1 & 0xc) == 8) {
          FUN_ram_f00266d8(piVar4 + 0x8c);
          iVar2 = FUN_ram_f0020000();
          if (iVar2 == 0) {
            uVar3 = 30000;
          }
          else {
            uVar3 = 10000;
          }
          FUN_ram_f002672c(piVar4 + 0x8c,uVar3,FUN_ram_f002672c);
        }
        if (piVar4[0x2a] == 3) {
          if ((puVar6[2] & 1) == 0) {
            if (((*(char *)((int)piVar4 + 0x9a) == '\0') || (*(char *)(iVar5 + 0x2aa) == '\0')) ||
               (iVar2 = FUN_ram_f002ad54(uVar1,*(undefined1 *)(param_1 + 0xf),
                                         *(undefined1 *)((int)piVar4 + 0x1c9)), iVar2 == 0)) {
              if ((uVar1 & 0xfc) == 0xa4) {
                piVar4[0x30] = *(int *)(param_1 + 0x30);
                *(undefined1 *)((int)piVar4 + 0x1c1) = 1;
                *(undefined1 *)((int)piVar4 + 0xba) = 0;
              }
            }
            else {
              piVar4[0x31] = *(int *)(param_1 + 0x30);
              *(undefined1 *)((int)piVar4 + 0x1c1) = 1;
            }
            if (((uVar1 & 0xfc) == 8) || ((uVar1 & 0xfc) == 0x88)) {
              FUN_ram_f002ea68(param_1);
            }
            FUN_ram_f002f8dc(iVar5);
          }
        }
        else if (piVar4[0x2a] == 0) {
          if (((uVar1 & 0xfc) == 8) || ((uVar1 & 0xfc) == 0x88)) {
            FUN_ram_f002ea68(param_1);
          }
        }
      }
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00301dc @ 0xf00301dc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00301dc(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0x20) == 0) {
    return;
  }
  iVar1 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(*(int *)(param_1 + 0x20) + 0x14) * 4);
  if (*(char *)(iVar1 + 0x19) == '\0') {
    iVar1 = *(int *)(iVar1 + 0xc);
    if (iVar1 == 1) {
      return;
    }
    if (iVar1 == 0) {
LAB_ram_f003023c:
      FUN_ram_f003008c();
      return;
    }
    if (iVar1 != 2) {
      uVar2 = 0xbb1;
LAB_ram_f003026c:
      (*_DAT_ram_00014800)(0,0,0xf00635bc,uVar2,_DAT_ram_00014800);
      return;
    }
  }
  else {
    if (*(char *)(iVar1 + 0x19) != '\x01') {
      return;
    }
    iVar1 = *(int *)(iVar1 + 0xc);
    if (iVar1 == 1) {
      return;
    }
    if (iVar1 == 0) goto LAB_ram_f003023c;
    if (iVar1 != 2) {
      if (iVar1 == 3) {
        return;
      }
      uVar2 = 0xbc2;
      goto LAB_ram_f003026c;
    }
  }
  FUN_ram_f002adbc();
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030278 @ 0xf0030278 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0030278(int param_1,char param_2)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  piVar3 = *(int **)(&DAT_ram_020a0068 + param_1 * 4);
  if ((char)piVar3[0x2c] != '\0') {
    *(undefined1 *)((int)piVar3 + 0xb1) = 0;
    *(undefined1 *)(piVar3 + 0x2c) = 0;
    if (*(char *)((int)piVar3 + 0x19) == '\0') {
      if ((*piVar3 == 0) && (piVar3[3] == 0)) {
        if (*(char *)((int)piVar3 + 0xb3) != '\0') {
          return 1;
        }
        if ((param_2 == '\x01') && ((piVar3[0x83] == 0 || (piVar3[0x2b] == 0)))) {
          FUN_ram_f002e94c(piVar3);
        }
        FUN_ram_f002672c(piVar3 + 0x58,0x14);
      }
    }
    else {
      if (*(char *)((int)piVar3 + 0x19) != '\x01') {
        return 0;
      }
      iVar1 = FUN_ram_f0030f88();
      iVar2 = _DAT_ram_020a0064;
      if (iVar1 != 0) {
        *(undefined1 *)(_DAT_ram_020a0064 + (uint)*(byte *)((int)piVar3 + 0x19) + 0x1f6e) = 1;
        iVar2 = iVar2 + (uint)*(byte *)((int)piVar3 + 0x19);
        if (*(char *)(iVar2 + 0x1f6e) == '\0') {
          return 0;
        }
        if (*(char *)(iVar2 + 0x1f71) == '\0') {
          return 0;
        }
        FUN_ram_f004aea8(piVar3);
        return 0;
      }
      if ((*piVar3 == 0) && (piVar3[3] == 0)) {
        if (*(char *)((int)piVar3 + 0xb3) != '\0') {
          return 1;
        }
        if ((piVar3[0x83] == 0) || (piVar3[0x2b] == 0)) {
          FUN_ram_f002e94c(piVar3);
        }
        iVar2 = _DAT_ram_020a0064;
        *(undefined1 *)(_DAT_ram_020a0064 + (uint)*(byte *)((int)piVar3 + 0x19) + 0x1f6e) = 1;
        iVar2 = iVar2 + (uint)*(byte *)((int)piVar3 + 0x19);
        if ((*(char *)(iVar2 + 0x1f6e) != '\0') && (*(char *)(iVar2 + 0x1f71) != '\0')) {
          FUN_ram_f004aea8(piVar3);
        }
        (*_DAT_ram_00014cb0)
                  (*(undefined1 *)((int)piVar3 + 0x346),*(undefined1 *)((int)piVar3 + 0x347),
                   _DAT_ram_00014cb0);
      }
    }
    piVar3[7] = 0;
    FUN_ram_f002f2d8(*(undefined1 *)((int)piVar3 + 0x19));
  }
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00303b4 @ 0xf00303b4 =====


void FUN_ram_f00303b4(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (param_1 != 0) {
    *(undefined1 *)(param_1 + 0xb8) = 0;
    *(undefined1 *)(param_1 + 0x1c0) = 0;
    *(undefined1 *)(param_1 + 0x1c1) = 0;
    iVar2 = 0;
    FUN_ram_f00266d8(param_1 + 0xe8);
    do {
      iVar1 = FUN_ram_f0025d90(iVar2);
      iVar2 = iVar2 + 1;
      if ((iVar1 != 0) && (*(char *)(iVar1 + 0x14) == *(char *)(param_1 + 0x19))) {
        *(undefined1 *)(iVar1 + 0x70) = 0;
        *(undefined1 *)(iVar1 + 0x71) = 0;
        *(undefined1 *)(iVar1 + 0x55) = 0;
        FUN_ram_f00266d8(iVar1 + 0x58);
      }
    } while (iVar2 != 0x14);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003041c @ 0xf003041c =====


void FUN_ram_f003041c(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (param_1 != 0) {
    *(undefined1 *)(param_1 + 0xbe) = 1;
    *(undefined4 *)(param_1 + 0xc4) = 0;
    *(undefined4 *)(param_1 + 0xcc) = 0;
    iVar2 = 0;
    do {
      iVar1 = FUN_ram_f0025d90(iVar2);
      iVar2 = iVar2 + 1;
      if ((iVar1 != 0) && (*(char *)(iVar1 + 0x14) == *(char *)(param_1 + 0x19))) {
        *(undefined4 *)(iVar1 + 0x80) = 0;
        *(undefined1 *)(iVar1 + 0x56) = 1;
        *(undefined4 *)(iVar1 + 0x78) = 0;
      }
    } while (iVar2 != 0x14);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003046c @ 0xf003046c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003046c(int param_1)

{
  int iVar1;
  
  if ((((*(int *)(param_1 + 0xa8) == 3) && (*(char *)(param_1 + 0xb7) == '\0')) &&
      (*(char *)(param_1 + 0xb8) == '\0')) &&
     ((iVar1 = FUN_ram_f002bda8(), iVar1 == 0 && (*(char *)(param_1 + 0x261) == '\0')))) {
    *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(param_1 + 0x19) + 0x3d6) * 4 + 4) = 2;
    FUN_ram_f002bdc8(*(undefined1 *)(param_1 + 0x19));
    FUN_ram_f0020c1c(1);
    if (*(char *)(param_1 + 0xb1) != '\0') {
      *(undefined4 *)(param_1 + 0x1c) = 1;
      FUN_ram_f002f4ac(*(undefined1 *)(param_1 + 0x19));
      *(undefined1 *)(param_1 + 0xb1) = 0;
    }
  }
  else {
    FUN_ram_f0020c1c(0);
    *(undefined4 *)(_DAT_ram_020a0064 + (*(byte *)(param_1 + 0x19) + 0x3d6) * 4 + 4) = 1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030510 @ 0xf0030510 =====


undefined4 FUN_ram_f0030510(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((((param_1 == 0) || (iVar1 = *(int *)(param_1 + 0x20), iVar1 == 0)) ||
      (2 < *(byte *)(iVar1 + 0x14))) || (*(char *)(iVar1 + 0x15) != '\x02')) {
    uVar2 = 0xc0000001;
  }
  else {
    uVar2 = FUN_ram_f003046c(*(undefined4 *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar1 + 0x14) * 4))
    ;
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003065c @ 0xf003065c =====


void FUN_ram_f003065c(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
  if (*(short *)(param_1 + 0x70) == 0) {
    return;
  }
  *(undefined1 *)(param_1 + 0x71) = 0;
  *(undefined1 *)(param_1 + 0x70) = 0;
  if (*(char *)(iVar1 + 0x18) != '\0') {
    if (*(int *)(iVar1 + 0xa8) == 3) {
      if (*(char *)(param_1 + 0x2a8) == '\x01') {
        if ((*(int *)(param_1 + 0x78) - *(int *)(param_1 + 0x80) < 0) &&
           (*(char *)(param_1 + 0x56) == '\0')) {
          if (*(char *)(param_1 + 0x55) == '\x01') {
            *(undefined1 *)(param_1 + 0x55) = 0;
            FUN_ram_f00266d8(param_1 + 0x58);
          }
        }
        else if (((*(int *)(param_1 + 0x78) != 0) || (*(int *)(param_1 + 0x80) != 0)) &&
                (*(char *)(param_1 + 0x55) == '\0')) {
          *(undefined1 *)(param_1 + 0x56) = 0;
          *(undefined1 *)(param_1 + 0x55) = 1;
          FUN_ram_f002672c(param_1 + 0x58,1000);
        }
      }
      FUN_ram_f003046c(iVar1);
      return;
    }
    return;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030700 @ 0xf0030700 =====


void FUN_ram_f0030700(int param_1)

{
  int iVar1;
  undefined2 *puVar2;
  int iVar3;
  undefined4 uVar4;
  
  if ((((((param_1 != 0) && (puVar2 = *(undefined2 **)(param_1 + 0x14), puVar2 != (undefined2 *)0x0)
         ) && (iVar3 = *(int *)(param_1 + 0x20), iVar3 != 0)) &&
       ((*(byte *)(iVar3 + 0x14) < 3 && (*(char *)(iVar3 + 0x15) == '\x02')))) &&
      ((*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar3 + 0x14) * 4) + 0xa8) == 3 &&
       (((*(byte *)(puVar2 + 2) & 1) == 0 && (*(char *)(iVar3 + 0x2a8) != '\0')))))) &&
     ((*(char *)(iVar3 + 0x2aa) != '\0' &&
      (iVar1 = FUN_ram_f002ad54(*puVar2,*(undefined1 *)(param_1 + 0xf),*(undefined1 *)(iVar3 + 0x53)
                               ), iVar1 != 0)))) {
    uVar4 = *(undefined4 *)(param_1 + 0x30);
    *(undefined1 *)(iVar3 + 0x71) = 1;
    *(undefined4 *)(iVar3 + 0x78) = uVar4;
    FUN_ram_f003065c(iVar3);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030784 @ 0xf0030784 =====


undefined4 FUN_ram_f0030784(int param_1)

{
  ushort uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  
  if ((((param_1 == 0) || (iVar4 = *(int *)(param_1 + 0x20), iVar4 == 0)) ||
      (2 < *(byte *)(iVar4 + 0x14))) || (*(char *)(iVar4 + 0x15) != '\x02')) {
    uVar3 = 0xc0000001;
  }
  else {
    iVar5 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar4 + 0x14) * 4);
    if ((*(int *)(iVar5 + 0xa8) == 3) && (*(char *)(iVar4 + 0x2aa) != '\0')) {
      uVar1 = *(ushort *)(param_1 + 0x32) & 0xfc;
      if (((uVar1 == 200) || (uVar1 == 0x88)) &&
         (iVar2 = FUN_ram_f002ad8c(*(undefined1 *)(param_1 + 0xf),*(undefined1 *)(iVar4 + 0x52)),
         iVar2 != 0)) {
        if ((*(ushort *)(*(int *)(param_1 + 0x14) + 0x18) & 0x10) != 0) {
          *(undefined4 *)(iVar4 + 0x80) = *(undefined4 *)(*(int *)(param_1 + 8) + 0x24);
          *(undefined1 *)(iVar4 + 0x70) = 1;
        }
        if ((*(char *)(iVar5 + 0xb1) == '\0') && (*(int *)(iVar5 + 0x1c) != 1)) {
          FUN_ram_f002ecac(param_1);
        }
        FUN_ram_f003065c(iVar4);
      }
    }
    uVar3 = 0;
  }
  return uVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003083c @ 0xf003083c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003083c(int param_1)

{
  undefined1 uVar1;
  ushort uVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int *piVar8;
  int iVar9;
  
  if (((((param_1 == 0) || (*(int *)(param_1 + 0x14) == 0)) || (*(int *)(param_1 + 8) == 0)) ||
      ((iVar9 = *(int *)(param_1 + 0x20), iVar9 == 0 || (2 < *(byte *)(iVar9 + 0x14))))) ||
     (*(char *)(iVar9 + 0x15) != '\x02')) {
    return 0xc0000001;
  }
  uVar2 = *(ushort *)(param_1 + 0x32) >> 0xc & 1;
  iVar7 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar9 + 0x14) * 4);
  if (*(byte *)(iVar9 + 0x38) != uVar2) {
    *(char *)(iVar9 + 0x38) = (char)uVar2;
    if (uVar2 == 1) {
      *(undefined1 *)(iVar9 + 0x3b) = 1;
      *(undefined1 *)(iVar9 + 0x3c) = 1;
    }
    if (*(int *)(iVar7 + 0x1c) == 0) {
      FUN_ram_f00377e0(iVar9,*(undefined1 *)(iVar9 + 0x38));
    }
    if (uVar2 == 1) {
      return 0;
    }
  }
  if ((*(ushort *)(*(int *)(param_1 + 0x14) + 0x18) & 0x10) == 0) {
    iVar7 = *(int *)(param_1 + 8);
    if ((*(char *)(iVar9 + 0x38) != '\0') &&
       (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar9 + 0x14) * 4) + 0x1c) == 0)) {
      if (*(char *)(iVar9 + 0x2aa) == '\0') {
        FUN_ram_f002ac70(iVar9);
        return 0;
      }
      iVar3 = FUN_ram_f002ad54(*(undefined2 *)(param_1 + 0x32),*(undefined1 *)(param_1 + 0xf),
                               *(undefined1 *)(iVar9 + 0x53));
      if (iVar3 == 0) {
        FUN_ram_f002ac70(iVar9);
        return 0;
      }
      iVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
      if ((*(char *)(iVar9 + 0x3a) == '\0') ||
         (-1 < (int)((-0x3d8 - (*(int *)(iVar9 + 0x44) * 0x8000 + 999999U) / 1000000) + iVar3))) {
        *(undefined4 *)(iVar9 + 0x44) = *(undefined4 *)(iVar7 + 0x24);
        *(undefined1 *)(iVar9 + 0x50) = 1;
        FUN_ram_f002ac70(iVar9);
        uVar1 = *(undefined1 *)(iVar9 + 0x52);
        uVar4 = FUN_ram_f002ada4(*(undefined1 *)(iVar9 + 0x54));
        iVar7 = FUN_ram_f0036a34(iVar9,uVar1,uVar4);
        if (iVar7 == 0) {
          FUN_ram_f0025b3c();
          uVar5 = FUN_ram_f0025b3c();
          if (0xf < uVar5) {
            uVar5 = (uint)DAT_ram_f0064fb9;
            DAT_ram_f0064fb7 = 1;
            if (uVar5 < 10) {
              piVar8 = (int *)&DAT_ram_f006d6f4;
              uVar6 = 0;
              do {
                if (uVar5 <= uVar6) {
                  DAT_ram_f0064fb9 = DAT_ram_f0064fb9 + 1;
                  *(int *)(&DAT_ram_f006d6f4 + uVar5 * 4) = iVar9;
                  break;
                }
                iVar7 = *piVar8;
                piVar8 = piVar8 + 1;
                uVar6 = uVar6 + 1 & 0xff;
              } while (iVar7 != iVar9);
            }
          }
        }
        else {
          DAT_ram_f0064fb7 = 0;
        }
      }
    }
  }
  FUN_ram_f002ac70(iVar9);
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030a30 @ 0xf0030a30 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0030a30(int param_1)

{
  int iVar1;
  undefined1 auStack_10 [8];
  
  iVar1 = _DAT_ram_020a0064;
  (*_DAT_ram_00014bf0)
            (*(undefined1 *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x346),auStack_10,
             _DAT_ram_00014bf0);
  return *(char *)(iVar1 + 0x1ede) != '\0';
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030a6c @ 0xf0030a6c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0030a6c(int param_1)

{
  int iVar1;
  undefined1 auStack_10 [8];
  
  iVar1 = _DAT_ram_020a0064;
  (*_DAT_ram_00014bf0)
            (*(undefined1 *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x346),auStack_10,
             _DAT_ram_00014bf0);
  return *(char *)(iVar1 + 0x1ef6) != '\0';
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030aa8 @ 0xf0030aa8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0030aa8(int param_1)

{
  int iVar1;
  bool bVar2;
  undefined1 auStack_10 [8];
  
  iVar1 = _DAT_ram_020a0064;
  (*_DAT_ram_00014bf0)
            (*(undefined1 *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x346),auStack_10,
             _DAT_ram_00014bf0);
  if ((*(char *)(iVar1 + 0x1edd) != '\0') || (bVar2 = true, *(char *)(iVar1 + 0x1ede) != '\x01')) {
    if (*(char *)(iVar1 + 0x1ef5) == '\0') {
      bVar2 = *(char *)(iVar1 + 0x1ef6) == '\x01';
    }
    else {
      bVar2 = false;
    }
  }
  return bVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030b00 @ 0xf0030b00 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0030b00(void)

{
  return *(char *)(_DAT_ram_020a0064 + 0x1f04) != '\0';
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030b14 @ 0xf0030b14 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0030b14(void)

{
  return *(char *)(_DAT_ram_020a0064 + 0x1ef4) != '\0';
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030b28 @ 0xf0030b28 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0030b28(void)

{
  return *(char *)(_DAT_ram_020a0064 + 0x1edc) != '\0';
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030b3c @ 0xf0030b3c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0030b3c(void)

{
  int iVar1;
  uint uVar2;
  char *pcVar3;
  int iVar4;
  bool bVar5;
  int iVar6;
  int iVar7;
  undefined4 uVar8;
  int local_20 [2];
  
  iVar1 = _DAT_ram_020a0070;
  iVar6 = _DAT_ram_020a0064;
  if (*(char *)(_DAT_ram_020a0070 + 0x18) != '\0') {
    iVar7 = 0;
    (*_DAT_ram_00014bc8)(1,0,_DAT_ram_00014bc8);
    (*_DAT_ram_00014e00)(0,*(undefined1 *)(iVar1 + 0x346),0,0,0,0,0);
    (*_DAT_ram_00014bf0)(*(undefined1 *)(iVar1 + 0x346),local_20,_DAT_ram_00014bf0);
    uVar8 = 0;
    pcVar3 = (char *)(iVar6 + 0x1f2c);
    iVar6 = 0;
    uVar2 = 0xffffffff;
    bVar5 = false;
    do {
      if (*pcVar3 != '\0') {
        iVar4 = *(int *)(pcVar3 + 4);
        if ((local_20[0] - iVar4 < 0) && ((uint)(iVar4 - local_20[0]) < uVar2)) {
          uVar8 = 1;
          uVar2 = iVar4 - local_20[0];
          iVar7 = iVar4;
        }
        iVar4 = *(int *)(pcVar3 + 8);
        bVar5 = true;
        if ((local_20[0] - iVar4 < 0) && ((uint)(iVar4 - local_20[0]) < uVar2)) {
          uVar8 = 0;
          uVar2 = iVar4 - local_20[0];
          iVar7 = iVar4;
        }
      }
      iVar6 = iVar6 + 1;
      pcVar3 = pcVar3 + 0x14;
    } while (iVar6 != 2);
    if (bVar5) {
      if (uVar2 < 1000) {
        FUN_ram_f002672c(iVar1 + 0x218,1);
      }
      else {
        (*_DAT_ram_00014b70)(1,*(undefined1 *)(iVar1 + 0x346),iVar7,0,_DAT_ram_00014b70);
        (*_DAT_ram_00014bc8)(1,1,_DAT_ram_00014bc8);
        (*_DAT_ram_00014e00)(1,*(undefined1 *)(iVar1 + 0x346),iVar7,1,uVar8,0,0);
      }
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0030c40 @ 0xf0030c40 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0030c40(void)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  bool bVar6;
  int iVar7;
  int local_20 [2];
  
  iVar2 = _DAT_ram_020a006c;
  iVar7 = _DAT_ram_020a0064;
  if (*(char *)(_DAT_ram_020a006c + 0x18) != '\0') {
    (*_DAT_ram_00014bc8)(0,0,_DAT_ram_00014bc8);
    (*_DAT_ram_00014e00)(0,*(undefined1 *)(iVar2 + 0x346),0,0,0,0,0);
    (*_DAT_ram_00014bf0)(*(undefined1 *)(iVar2 + 0x346),local_20,_DAT_ram_00014bf0);
    if (*(char *)(iVar7 + 0x1edc) == '\0') {
      uVar5 = 0xffffffff;
      iVar3 = 0;
      bVar1 = false;
      bVar6 = false;
    }
    else {
      iVar3 = *(int *)(iVar7 + 0x1ee0);
      if ((local_20[0] - iVar3 < 0) && (uVar5 = iVar3 - local_20[0], uVar5 != 0xffffffff)) {
        if ((*(char *)(_DAT_ram_020a0064 + 0x1f06) == '\0') &&
           (*(char *)(_DAT_ram_020a0064 + 0x1f1a) == '\0')) {
          bVar6 = true;
        }
        else {
          bVar6 = false;
        }
      }
      else {
        bVar6 = false;
        uVar5 = 0xffffffff;
        iVar3 = 0;
      }
      iVar4 = *(int *)(iVar7 + 0x1ee4);
      if ((local_20[0] - iVar4 < 0) && ((uint)(iVar4 - local_20[0]) < uVar5)) {
        bVar6 = false;
        uVar5 = iVar4 - local_20[0];
        iVar3 = iVar4;
      }
      bVar1 = true;
    }
    if (*(char *)(iVar7 + 0x1ef4) != '\0') {
      iVar4 = *(int *)(iVar7 + 0x1ef8);
      if ((local_20[0] - iVar4 < 0) && ((uint)(iVar4 - local_20[0]) < uVar5)) {
        bVar6 = *(char *)(_DAT_ram_020a0064 + 0x1f1a) == '\0';
        iVar3 = iVar4;
        uVar5 = iVar4 - local_20[0];
      }
      iVar4 = *(int *)(iVar7 + 0x1efc);
      if ((local_20[0] - iVar4 < 0) && ((uint)(iVar4 - local_20[0]) < uVar5)) {
        bVar6 = false;
        uVar5 = iVar4 - local_20[0];
        iVar3 = iVar4;
      }
      bVar1 = true;
    }
    if (*(char *)(iVar7 + 0x1f04) != '\0') {
      iVar4 = *(int *)(iVar7 + 0x1f0c);
      if ((local_20[0] - iVar4 < 0) && ((uint)(iVar4 - local_20[0]) < uVar5)) {
        bVar6 = false;
        uVar5 = iVar4 - local_20[0];
        iVar3 = iVar4;
      }
      bVar1 = true;
    }
    if (*(char *)(iVar7 + 0x1f18) == '\0') {
      if (!bVar1) {
        return;
      }
    }
    else {
      iVar4 = *(int *)(iVar7 + 0x1f1c);
      if ((local_20[0] - iVar4 < 0) && ((uint)(iVar4 - local_20[0]) < uVar5)) {
        bVar6 = false;
        iVar3 = iVar4;
        uVar5 = iVar4 - local_20[0];
      }
      iVar7 = *(int *)(iVar7 + 0x1f20);
      if ((local_20[0] - iVar7 < 0) && ((uint)(iVar7 - local_20[0]) < uVar5)) {
        bVar6 = false;
        uVar5 = iVar7 - local_20[0];
        iVar3 = iVar7;
      }
    }
    if (uVar5 < 1000) {
      FUN_ram_f002672c(iVar2 + 0x218,1);
    }
    else {
      (*_DAT_ram_00014b70)(0,*(undefined1 *)(iVar2 + 0x346),iVar3,0,_DAT_ram_00014b70);
      (*_DAT_ram_00014bc8)(0,1,_DAT_ram_00014bc8);
      (*_DAT_ram_00014e00)(1,*(undefined1 *)(iVar2 + 0x346),iVar3,1,bVar6,0,0);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/pm_timer.c:0x593 <<<
// ===== FUN_ram_f0030e04 @ 0xf0030e04 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0030e04(void)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  
  iVar2 = _DAT_ram_020a0070;
  iVar1 = _DAT_ram_020a0064;
  uVar4 = *(uint *)(_DAT_ram_020a0070 + 0x1c);
  *(uint *)(_DAT_ram_020a0070 + 0x20) = uVar4;
  if ((*(char *)(iVar1 + 0x1f2c) == '\0') || (*(char *)(iVar1 + 0x1f2e) == '\0')) {
    uVar3 = 0;
    if (*(char *)(iVar1 + 8000) != '\0') {
      uVar3 = (uint)(*(char *)(iVar1 + 0x1f42) != '\0');
    }
  }
  else {
    uVar3 = 1;
  }
  if (*(char *)(iVar2 + 0xb0) != '\0') {
    if (*(int *)(iVar2 + 0x1c) == 1) {
      uVar3 = 1;
    }
  }
  *(uint *)(iVar2 + 0x1c) = uVar3;
  if (uVar3 != uVar4) {
    if (uVar3 == 1) {
      FUN_ram_f002f504(2);
    }
    else if (uVar3 == 0) {
      FUN_ram_f002f314(2);
    }
    else {
      (*_DAT_ram_00014800)(0,0,0xf006366c,0x593,_DAT_ram_00014800);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/pm_timer.c:0x5d4,0x5d5 <<<
// ===== FUN_ram_f0030e9c @ 0xf0030e9c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0030e9c(void)

{
  int *piVar1;
  int iVar2;
  undefined1 *puVar3;
  int *piVar4;
  int iVar5;
  int *piVar6;
  int local_28 [3];
  
  iVar2 = _DAT_ram_020a0064;
  if (*(char *)(_DAT_ram_020a0070 + 0x18) != '\0') {
    piVar6 = (int *)(_DAT_ram_020a0064 + 0x1f3c);
    puVar3 = (undefined1 *)(_DAT_ram_020a0064 + 0x1f2e);
    iVar5 = 0;
    (*_DAT_ram_00014bf0)(*(undefined1 *)(_DAT_ram_020a0070 + 0x346),local_28,_DAT_ram_00014bf0);
    piVar4 = (int *)(iVar2 + 0x1f34);
    do {
      puVar3[-1] = *puVar3;
      if (puVar3[-2] != '\0') {
        piVar1 = (int *)(puVar3 + 2);
        if (*piVar1 - local_28[0] < 0) {
          if (-1 < *piVar4 - local_28[0]) {
            *puVar3 = 1;
            goto LAB_ram_f0030f66;
          }
          iVar2 = *piVar6;
          if (iVar2 == 0) {
            puVar3[-2] = 0;
          }
          else {
            do {
              *piVar1 = *piVar1 + iVar2;
              *piVar4 = *piVar4 + iVar2;
            } while (-1 < (local_28[0] - *piVar1) + -1000);
            if (-1 < local_28[0] - *piVar1) {
              (*_DAT_ram_00014800)(0,0,0xf006366c,0x5d4,_DAT_ram_00014800);
            }
            if (-1 < local_28[0] - *piVar4) {
              (*_DAT_ram_00014800)(0,0,0xf006366c,0x5d5,_DAT_ram_00014800);
            }
          }
        }
        *puVar3 = 0;
      }
LAB_ram_f0030f66:
      iVar5 = iVar5 + 1;
      puVar3 = puVar3 + 0x14;
      piVar4 = piVar4 + 5;
      piVar6 = piVar6 + 5;
    } while (iVar5 != 2);
    FUN_ram_f0030e04();
  }
  return;
}

// >>> MOD: wifi/mgmt/pm_timer.c:0x548 <<<
// ===== FUN_ram_f0030f88 @ 0xf0030f88 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0030f88(void)

{
  int *piVar1;
  int iVar2;
  bool bVar3;
  
  iVar2 = _DAT_ram_020a0064;
  piVar1 = (int *)(_DAT_ram_020a006c + 0xc);
  *(undefined4 *)(_DAT_ram_020a006c + 0x20) = *(undefined4 *)(_DAT_ram_020a006c + 0x1c);
  if ((*piVar1 == 2) || (*piVar1 == 0)) {
    if (*(char *)(iVar2 + 0x1f04) == '\0') {
      if (*(char *)(_DAT_ram_020a0064 + 0x1f1a) == '\0') {
        iVar2 = FUN_ram_f0030a6c(1);
        if (iVar2 != 0) {
          return true;
        }
        if (*(char *)(_DAT_ram_020a0064 + 0x1f06) != '\0') {
          (*_DAT_ram_00014800)(0,0,0xf006366c,0x548,_DAT_ram_00014800);
          return false;
        }
        iVar2 = FUN_ram_f0030a30(1);
        return iVar2 != 0;
      }
    }
    else {
      if ((*(char *)(_DAT_ram_020a0064 + 0x1f1a) == '\0') &&
         ((iVar2 = FUN_ram_f0030a6c(1), iVar2 != 0 ||
          ((*(char *)(_DAT_ram_020a0064 + 0x1f06) == '\0' &&
           (iVar2 = FUN_ram_f0030a30(1), iVar2 != 0)))))) {
        bVar3 = true;
      }
      else {
        bVar3 = false;
      }
      iVar2 = FUN_ram_f0030a6c(1);
      if (iVar2 != 0) {
        return bVar3;
      }
      if (*(char *)(_DAT_ram_020a0064 + 0x1f06) != '\0') {
        return bVar3;
      }
      iVar2 = FUN_ram_f002c7a8(1);
      if (iVar2 == 0) {
        return bVar3;
      }
    }
  }
  return false;
}

// >>> MOD: wifi/mgmt/pm_timer.c:0x4bb,0x501 <<<
// ===== FUN_ram_f0031078 @ 0xf0031078 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0031078(void)

{
  int *piVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar3 = _DAT_ram_020a006c;
  iVar4 = _DAT_ram_020a0064;
  piVar1 = (int *)(_DAT_ram_020a006c + 0xc);
  *(undefined4 *)(_DAT_ram_020a006c + 0x20) = *(undefined4 *)(_DAT_ram_020a006c + 0x1c);
  if ((*piVar1 != 2) && (*piVar1 != 0)) {
    return;
  }
  if (*(char *)(iVar4 + 0x1f04) == '\0') {
    if (*(char *)(_DAT_ram_020a0064 + 0x1f1a) == '\0') {
      iVar4 = FUN_ram_f0030a6c(1);
      if (iVar4 == 0) {
        if (*(char *)(_DAT_ram_020a0064 + 0x1f06) != '\0') {
          (*_DAT_ram_00014800)(0,0,0xf006366c,0x4bb,_DAT_ram_00014800);
          bVar2 = false;
          goto LAB_ram_f0031164;
        }
        iVar4 = FUN_ram_f0030a30(1);
        if (iVar4 == 0) goto LAB_ram_f003115e;
      }
      bVar2 = true;
      goto LAB_ram_f0031164;
    }
  }
  else {
    if ((*(char *)(_DAT_ram_020a0064 + 0x1f1a) == '\0') &&
       ((iVar4 = FUN_ram_f0030a6c(1), iVar4 != 0 ||
        ((*(char *)(_DAT_ram_020a0064 + 0x1f06) == '\0' &&
         ((iVar4 = FUN_ram_f0030a30(1), iVar4 != 0 ||
          ((*(int *)(iVar3 + 0xc) != 0 && (*(int *)(iVar3 + 0xc) == 2)))))))))) {
      bVar2 = true;
    }
    else {
      bVar2 = false;
    }
    if ((((*(int *)(iVar3 + 0xc) != 2) || (iVar4 = FUN_ram_f0030a6c(1), iVar4 != 0)) ||
        (*(char *)(_DAT_ram_020a0064 + 0x1f06) != '\0')) ||
       (iVar4 = FUN_ram_f002c7a8(1), iVar4 == 0)) goto LAB_ram_f0031164;
  }
LAB_ram_f003115e:
  bVar2 = false;
LAB_ram_f0031164:
  if (((*(char *)(iVar3 + 0xb0) != '\0') && (*(int *)(iVar3 + 0x1c) == 1)) || (bVar2)) {
    iVar4 = FUN_ram_f0030aa8(1);
    if (iVar4 != 0) {
      FUN_ram_f002bde4(1);
    }
    iVar4 = 1;
  }
  else {
    if ((*(uint *)(_DAT_ram_020a0064 + 0x1f04) & 0xffff00) == 0x10000) {
      FUN_ram_f002b184(1);
    }
    iVar5 = FUN_ram_f0030aa8(1);
    iVar4 = 0;
    if (iVar5 != 0) {
      FUN_ram_f0033fc4();
    }
  }
  *(int *)(iVar3 + 0x1c) = iVar4;
  if (iVar4 != *(int *)(iVar3 + 0x20)) {
    if (iVar4 == 1) {
      FUN_ram_f002f574(1);
    }
    else if (iVar4 == 0) {
      FUN_ram_f002f374(1);
    }
    else {
      (*_DAT_ram_00014800)(0,0,0xf006366c,0x501,_DAT_ram_00014800);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/pm_timer.c:0x2d7,0x2d8,0x338,0x339,0x35d,0x35e <<<
// ===== FUN_ram_f0031220 @ 0xf0031220 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0031220(void)

{
  int iVar1;
  undefined1 uVar2;
  int iVar3;
  int iVar4;
  int local_18 [2];
  
  iVar4 = _DAT_ram_020a006c;
  iVar1 = _DAT_ram_020a0064;
  if (*(char *)(_DAT_ram_020a006c + 0x18) == '\0') {
    return;
  }
  (*_DAT_ram_00014bf0)(*(undefined1 *)(_DAT_ram_020a006c + 0x346),local_18,_DAT_ram_00014bf0);
  *(undefined1 *)(iVar1 + 0x1edd) = *(undefined1 *)(iVar1 + 0x1ede);
  if (*(char *)(iVar1 + 0x1edc) != '\0') {
    if (*(int *)(iVar1 + 0x1ee0) - local_18[0] < 0) {
      if (*(int *)(iVar1 + 0x1ee4) - local_18[0] < 0) {
        if (*(int *)(iVar1 + 0x1ee8) == 0) {
          *(undefined1 *)(iVar1 + 0x1edc) = 0;
        }
        else {
          do {
            iVar3 = *(int *)(iVar1 + 0x1ee8);
            if ((iVar3 != 0xff) && (iVar3 != 0)) {
              *(int *)(iVar1 + 0x1ee8) = iVar3 + -1;
            }
            iVar3 = *(int *)(iVar1 + 0x1ee0) + *(int *)(iVar1 + 0x1eec);
            *(int *)(iVar1 + 0x1ee0) = iVar3;
            *(int *)(iVar1 + 0x1ee4) = *(int *)(iVar1 + 0x1ee4) + *(int *)(iVar1 + 0x1eec);
            if ((local_18[0] + 1000) - iVar3 < 0) goto LAB_ram_f00312a0;
          } while (*(int *)(iVar1 + 0x1ee8) != 0);
          *(undefined1 *)(iVar1 + 0x1edc) = 0;
LAB_ram_f00312a0:
          if (*(char *)(iVar1 + 0x1edc) != '\0') {
            if (-1 < local_18[0] - *(int *)(iVar1 + 0x1ee0)) {
              (*_DAT_ram_00014800)(0,0,0xf006366c,0x2d7,_DAT_ram_00014800);
            }
            if (-1 < local_18[0] - *(int *)(iVar1 + 0x1ee4)) {
              (*_DAT_ram_00014800)(0,0,0xf006366c,0x2d8,_DAT_ram_00014800);
            }
          }
        }
        goto LAB_ram_f00312f6;
      }
      uVar2 = 1;
    }
    else {
LAB_ram_f00312f6:
      uVar2 = 0;
    }
    *(undefined1 *)(iVar1 + 0x1ede) = uVar2;
  }
  *(undefined1 *)(iVar1 + 0x1ef5) = *(undefined1 *)(iVar1 + 0x1ef6);
  if (*(char *)(iVar1 + 0x1ef4) != '\0') {
    if (*(int *)(iVar1 + 0x1ef8) - local_18[0] < 0) {
      if (*(int *)(iVar1 + 0x1efc) - local_18[0] < 0) {
        *(undefined1 *)(iVar1 + 0x1ef6) = 0;
        *(undefined1 *)(iVar1 + 0x1ef4) = 0;
        if (*(int *)(iVar4 + 0xc) == 2) {
          FUN_ram_f002c334();
        }
        goto LAB_ram_f0031346;
      }
      uVar2 = 1;
    }
    else {
      uVar2 = 0;
    }
    *(undefined1 *)(iVar1 + 0x1ef6) = uVar2;
  }
LAB_ram_f0031346:
  *(undefined1 *)(iVar1 + 0x1f05) = *(undefined1 *)(iVar1 + 0x1f06);
  if (*(char *)(iVar1 + 0x1f04) != '\0') {
    if (*(int *)(iVar1 + 0x1f08) - local_18[0] < 0) {
      if (*(int *)(iVar1 + 0x1f0c) - local_18[0] < 0) {
        do {
          iVar4 = *(int *)(iVar1 + 0x1f14) + *(int *)(iVar1 + 0x1f08);
          *(int *)(iVar1 + 0x1f08) = iVar4;
          *(int *)(iVar1 + 0x1f0c) = *(int *)(iVar1 + 0x1f0c) + *(int *)(iVar1 + 0x1f14);
          iVar4 = local_18[0] - iVar4;
        } while (-1 < iVar4 + 1000);
        if (-1 < iVar4) {
          (*_DAT_ram_00014800)(0,0,0xf006366c,0x338,_DAT_ram_00014800);
        }
        if (-1 < local_18[0] - *(int *)(iVar1 + 0x1f0c)) {
          (*_DAT_ram_00014800)(0,0,0xf006366c,0x339,_DAT_ram_00014800);
        }
        goto LAB_ram_f00313da;
      }
      uVar2 = 1;
    }
    else {
LAB_ram_f00313da:
      uVar2 = 0;
    }
    *(undefined1 *)(iVar1 + 0x1f06) = uVar2;
  }
  *(undefined1 *)(iVar1 + 0x1f19) = *(undefined1 *)(iVar1 + 0x1f1a);
  if (*(char *)(iVar1 + 0x1f18) == '\0') goto LAB_ram_f0031468;
  if (*(int *)(iVar1 + 0x1f1c) - local_18[0] < 0) {
    if (*(int *)(iVar1 + 0x1f20) - local_18[0] < 0) {
      iVar4 = *(int *)(iVar1 + 0x1f1c) + *(int *)(iVar1 + 0x1f24);
      *(int *)(iVar1 + 0x1f1c) = iVar4;
      *(int *)(iVar1 + 0x1f20) = *(int *)(iVar1 + 0x1f20) + *(int *)(iVar1 + 0x1f24);
      if (-1 < local_18[0] - iVar4) {
        (*_DAT_ram_00014800)(0,0,0xf006366c,0x35d,_DAT_ram_00014800);
      }
      if (-1 < local_18[0] - *(int *)(iVar1 + 0x1f20)) {
        (*_DAT_ram_00014800)(0,0,0xf006366c,0x35e,_DAT_ram_00014800);
      }
      goto LAB_ram_f0031462;
    }
    uVar2 = 1;
  }
  else {
LAB_ram_f0031462:
    uVar2 = 0;
  }
  *(undefined1 *)(iVar1 + 0x1f1a) = uVar2;
LAB_ram_f0031468:
  FUN_ram_f0031078();
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00314b4 @ 0xf00314b4 =====


void FUN_ram_f00314b4(void)

{
  FUN_ram_f0031220();
  FUN_ram_f0030c40();
  FUN_ram_f0030e9c();
  FUN_ram_f0030b3c();
  return;
}

// >>> MOD: wifi/mgmt/rate.c:0xfb,0xfc,0xfd <<<
// ===== FUN_ram_f00314ec @ 0xf00314ec =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00314ec(int param_1,int param_2,undefined2 *param_3,undefined2 *param_4,
                     undefined1 *param_5)

{
  byte bVar1;
  uint uVar2;
  undefined1 uVar3;
  undefined2 uVar4;
  uint uVar5;
  uint uVar6;
  undefined2 uVar7;
  uint uVar8;
  uint uVar9;
  
  if (param_3 == (undefined2 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063684,0xfb,param_5,_DAT_ram_00014800);
  }
  if (param_4 == (undefined2 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063684,0xfc);
  }
  if (param_5 == (undefined1 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063684,0xfd);
  }
  if (param_1 == 0) {
    uVar3 = 0;
    uVar5 = 0;
    uVar8 = 0;
  }
  else {
    uVar3 = 0;
    uVar5 = 0;
    uVar8 = 0;
    for (uVar9 = 0; uVar9 < *(byte *)(param_1 + 1); uVar9 = uVar9 + 1) {
      bVar1 = *(byte *)(param_1 + uVar9 + 2);
      uVar6 = 0;
      do {
        if (*(byte *)(uVar6 + 0xf0063698) == (bVar1 & 0x7f)) {
          uVar2 = 1 << (uVar6 & 0x1f) & 0xffff;
          uVar8 = uVar2 | uVar8;
          if ((char)bVar1 < '\0') {
            uVar5 = uVar2 | uVar5;
          }
          if (uVar6 != 0xf) goto LAB_ram_f00315be;
          break;
        }
        uVar6 = uVar6 + 1;
      } while (uVar6 != 0xf);
      if ((char)bVar1 < '\0') {
        uVar3 = 1;
      }
LAB_ram_f00315be:
    }
  }
  uVar4 = (undefined2)uVar5;
  uVar7 = (undefined2)uVar8;
  if (param_2 != 0) {
    uVar9 = 0;
    while( true ) {
      uVar4 = (undefined2)uVar5;
      uVar7 = (undefined2)uVar8;
      if (*(byte *)(param_2 + 1) <= uVar9) break;
      uVar6 = 0;
      bVar1 = *(byte *)(param_2 + uVar9 + 2);
      do {
        if (*(byte *)(uVar6 + 0xf0063698) == (bVar1 & 0x7f)) {
          uVar2 = 1 << (uVar6 & 0x1f) & 0xffff;
          uVar8 = uVar2 | uVar8;
          if ((char)bVar1 < '\0') {
            uVar5 = uVar2 | uVar5;
          }
          if (uVar6 != 0xf) goto LAB_ram_f0031620;
          break;
        }
        uVar6 = uVar6 + 1;
      } while (uVar6 != 0xf);
      if ((char)bVar1 < '\0') {
        uVar3 = 1;
      }
LAB_ram_f0031620:
      uVar9 = uVar9 + 1;
    }
  }
  *param_3 = uVar7;
  *param_4 = uVar4;
  *param_5 = uVar3;
  return;
}

// >>> MOD: wifi/mgmt/rate.c:0x15d,0x15e <<<
// ===== FUN_ram_f0031640 @ 0xf0031640 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0031640(ushort param_1,ushort param_2,int param_3,undefined1 *param_4)

{
  uint uVar1;
  uint uVar2;
  byte bVar3;
  byte *pbVar4;
  byte *pbVar5;
  int iVar6;
  uint uVar7;
  
  if (param_3 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063684,0x15d,_DAT_ram_00014800);
  }
  if (param_4 == (undefined1 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063684,0x15e,_DAT_ram_00014800);
  }
  uVar7 = 0;
  iVar6 = 0;
  do {
    uVar1 = uVar7 & 0x1f;
    pbVar4 = (byte *)(uVar7 + 0xf0063698);
    uVar2 = uVar7 & 0x1f;
    uVar7 = uVar7 + 1;
    pbVar5 = (byte *)(param_3 + iVar6);
    if ((param_1 >> uVar1 & 1) != 0) {
      bVar3 = *pbVar4;
      iVar6 = iVar6 + 1;
      *pbVar5 = bVar3;
      if ((param_2 >> uVar2 & 1) != 0) {
        *pbVar5 = bVar3 | 0x80;
      }
    }
  } while (uVar7 != 0xf);
  *param_4 = (char)iVar6;
  return;
}

// >>> MOD: wifi/mgmt/rate.c:0x18a <<<
// ===== FUN_ram_f00316c8 @ 0xf00316c8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_ram_f00316c8(ushort param_1,undefined1 *param_2)

{
  ushort uVar1;
  uint uVar2;
  
  if (param_2 == (undefined1 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063684,0x18a,_DAT_ram_00014800);
  }
  uVar2 = 0xd;
  do {
    uVar1 = param_1 >> (uVar2 & 0x1f);
    if ((uVar1 & 1) != 0) {
      *param_2 = (char)uVar2;
      return 1;
    }
    uVar2 = uVar2 - 1;
  } while (uVar2 != 0xffffffff);
  return uVar1 & 1;
}

// >>> MOD: wifi/mgmt/rate.c:0x1ab <<<
// ===== FUN_ram_f003170c @ 0xf003170c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_ram_f003170c(ushort param_1,undefined1 *param_2)

{
  ushort uVar1;
  uint uVar2;
  
  if (param_2 == (undefined1 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0063684,0x1ab,_DAT_ram_00014800);
  }
  uVar2 = 0;
  do {
    uVar1 = param_1 >> (uVar2 & 0x1f);
    if ((uVar1 & 1) != 0) {
      *param_2 = (char)uVar2;
      return 1;
    }
    uVar2 = uVar2 + 1;
  } while (uVar2 != 0xe);
  return uVar1 & 1;
}

// >>> MOD: wifi/mgmt/scan.c:0x26d,0x274 <<<
// ===== FUN_ram_f0031908 @ 0xf0031908 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0031908(int param_1,int param_2,int param_3,undefined2 param_4)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  undefined1 auStack_30 [15];
  byte local_21 [5];
  
  (*(code *)&SUB_ram_0006a0a0)(auStack_30,0,0xf);
  puVar2 = (undefined1 *)(*(int *)(param_1 + 0x18) + (uint)*(ushort *)(param_1 + 0x12));
  if (puVar2 == (undefined1 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf00636b8,0x26d,_DAT_ram_00014800);
  }
  *puVar2 = 0;
  puVar2[1] = (char)param_3;
  if (param_3 != 0) {
    if (param_2 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf00636b8,0x274,_DAT_ram_00014800);
    }
    (*(code *)&SUB_ram_0006a08c)(puVar2 + 2,param_2,param_3);
  }
  *(ushort *)(param_1 + 0x12) = *(short *)(param_1 + 0x12) + 2 + (ushort)(byte)puVar2[1];
  puVar2 = puVar2 + (byte)puVar2[1] + 2;
  if (DAT_ram_f0064fd5 == '\0') {
    uVar1 = 0xf;
  }
  else {
    uVar1 = 0;
  }
  FUN_ram_f0031640(param_4,uVar1,auStack_30,local_21,FUN_ram_f0031640);
  uVar4 = (uint)local_21[0];
  if (8 < uVar4) {
    local_21[0] = 8;
  }
  uVar3 = (uint)local_21[0];
  uVar4 = uVar4 - uVar3;
  uVar5 = uVar4 & 0xff;
  if (uVar3 != 0) {
    *puVar2 = 1;
    puVar2[1] = local_21[0];
    (*(code *)&SUB_ram_0006a08c)(puVar2 + 2,auStack_30,uVar3);
    *(ushort *)(param_1 + 0x12) = *(short *)(param_1 + 0x12) + 2 + (ushort)(byte)puVar2[1];
    puVar2 = puVar2 + (byte)puVar2[1] + 2;
  }
  if (uVar5 != 0) {
    *puVar2 = 0x32;
    puVar2[1] = (char)uVar4;
    (*(code *)&SUB_ram_0006a08c)(puVar2 + 2,auStack_30 + uVar3,uVar5);
    *(ushort *)(param_1 + 0x12) = *(short *)(param_1 + 0x12) + 2 + (ushort)(byte)puVar2[1];
  }
  return;
}

// >>> MOD: wifi/mgmt/scan.c:0x234,0x3a6,0x3aa <<<
// ===== FUN_ram_f0031a3c @ 0xf0031a3c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0031a3c(int param_1)

{
  byte bVar1;
  byte bVar2;
  undefined1 uVar3;
  uint uVar4;
  int iVar5;
  undefined4 uVar6;
  short sVar7;
  int iVar8;
  undefined2 uVar9;
  uint uStack_50;
  uint uStack_4c;
  undefined4 uStack_30;
  undefined2 local_2c;
  undefined4 local_28;
  undefined2 local_24;
  undefined1 local_21 [5];
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00636b8,0x3a6,_DAT_ram_00014800);
  }
  bVar1 = *(byte *)(param_1 + 0x94);
  bVar2 = *(byte *)(param_1 + 0x95);
  if (bVar1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00636b8,0x3aa,_DAT_ram_00014800);
  }
  sVar7 = _DAT_ram_f006d744 + *(short *)(param_1 + 0x2ce);
  uStack_50 = 0;
  uStack_4c = 0;
  while( true ) {
    if (bVar1 <= uStack_4c) {
      return 0;
    }
    uVar4 = FUN_ram_f0025b3c();
    if ((uVar4 < 8) || (iVar5 = FUN_ram_f0025c48(sVar7 + 0x55), iVar5 == 0)) break;
    iVar8 = *(int *)(iVar5 + 8);
    local_28 = _DAT_ram_f00636e4;
    local_24 = (undefined2)_DAT_ram_f00636e8;
    uStack_30 = _DAT_ram_f00636dc;
    local_2c = (undefined2)_DAT_ram_f00636e0;
    if ((undefined1 *)(iVar8 + 8) == (undefined1 *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf00636b8,0x234,_DAT_ram_00014800);
    }
    *(undefined1 *)(iVar8 + 8) = 0x40;
    *(undefined1 *)(iVar8 + 9) = 0;
    (*(code *)&SUB_ram_0006a08c)(iVar8 + 0xc,&local_28,6);
    (*(code *)&SUB_ram_0006a08c)(iVar8 + 0x12,param_1 + 0x8e,6);
    (*(code *)&SUB_ram_0006a08c)(iVar8 + 0x18,&uStack_30,6);
    *(undefined2 *)(iVar8 + 0x1e) = 0;
    uVar9 = _DAT_ram_f006349c;
    if ((_DAT_ram_f006513c == 0x24ba58) && (uVar9 = _DAT_ram_f0063498, *(int *)(param_1 + 4) != 1))
    {
      uVar9 = *(undefined2 *)(*(int *)(_DAT_ram_020a0064 + 0x209c) * 4 + -0xff9cb70);
    }
    FUN_ram_f003170c(uVar9,local_21);
    uVar3 = *(undefined1 *)(param_1 + 4);
    iVar8 = *(int *)(iVar5 + 8);
    uVar6 = (*_DAT_ram_00014ea8)(0,local_21[0]);
    FUN_ram_f0034540(iVar5,0,uVar3,iVar8 + 8,0x18,iVar8 + 0x20,0,0,0,1,uVar6,0,0);
    if ((uStack_4c < bVar2) && (uStack_50 < 4)) {
      FUN_ram_f0031908(iVar5,param_1 + 0xe + uStack_50 * 0x20,
                       *(undefined1 *)(param_1 + uStack_50 + 10),uVar9);
      if ((uStack_50 + 1) * ((uint)bVar2 / (uint)*(byte *)(param_1 + 9)) <= uStack_4c + 1) {
        uStack_50 = uStack_50 + 1 & 0xff;
      }
    }
    else {
      local_28 = CONCAT31(local_28._1_3_,DAT_ram_f0063bd8);
      FUN_ram_f0031908(iVar5,&local_28,0,uVar9);
    }
    if (_DAT_ram_f006d748 != (code *)0x0) {
      (*_DAT_ram_f006d748)(iVar5);
    }
    (*(code *)&SUB_ram_0006a08c)
              (*(int *)(iVar5 + 0x18) + (uint)*(ushort *)(iVar5 + 0x12),param_1 + 0x2d0,
               *(undefined2 *)(param_1 + 0x2ce));
    *(short *)(iVar5 + 0x12) = *(short *)(param_1 + 0x2ce) + *(short *)(iVar5 + 0x12);
    uStack_4c = uStack_4c + 1;
    FUN_ram_f0037b58(iVar5);
  }
  return 0xc000009a;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031cdc @ 0xf0031cdc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0031cdc(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = _DAT_ram_020a0064;
  *(undefined4 *)(_DAT_ram_020a0064 + 0x380) = 3;
  *(undefined4 *)(iVar1 + 0x378) = 0;
  *(undefined4 *)(iVar1 + 0x37c) = 0;
  *(undefined1 *)(iVar1 + 0xe71) = 0;
  FUN_ram_f00266a0(iVar1 + 0xe40,&LAB_ram_f0032b80,0);
  FUN_ram_f00266a0(iVar1 + 0xe74,&LAB_ram_f0033508,0);
  *(undefined1 *)(iVar1 + 0x964) = 0x30;
  *(undefined1 *)(iVar1 + 0xe19) = 0xc;
  *(undefined4 *)(iVar1 + 0xe14) = 0x78;
  *(undefined1 *)(iVar1 + 0x41b) = 0xc;
  *(undefined1 *)(iVar1 + 0x41c) = 0x30;
  *(undefined1 *)(iVar1 + 0x963) = 0xc;
  *(undefined1 *)(iVar1 + 0x651) = 0;
  *(undefined1 *)(iVar1 + 0xb99) = 0;
  uVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  *(undefined4 *)(iVar1 + 0xe38) = uVar2;
  *(undefined1 *)(iVar1 + 0xe3c) = 0;
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006d9a8,0,0x600);
  *(undefined4 *)(iVar1 + 0xe94) = 0;
  *(int *)(iVar1 + 0xe90) = iVar1 + 0xe8c;
  *(int *)(iVar1 + 0xe8c) = iVar1 + 0xe8c;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031d90 @ 0xf0031d90 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short FUN_ram_f0031d90(void)

{
  ushort uVar1;
  int iVar2;
  
  iVar2 = _DAT_ram_020a0064;
  (*_DAT_ram_00014998)(&DAT_ram_f006e4c0,_DAT_ram_00014998);
  uVar1 = *(ushort *)(iVar2 + 0xe60);
  if ((_DAT_ram_f006e4cc & 0xffff) < (uint)uVar1) {
    uVar1 = ~uVar1;
  }
  else {
    uVar1 = -uVar1;
  }
  return (short)_DAT_ram_f006e4cc + uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031dd4 @ 0xf0031dd4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0031dd4(int param_1,byte param_2)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = _DAT_ram_020a0064;
  if ((param_1 != 0x24ba58) || (param_2 < 0xc)) {
    uVar2 = FUN_ram_f0031d90();
    if (*(char *)(iVar1 + 0xe64) == '\x01') {
      if (uVar2 <= *(ushort *)(iVar1 + 0xe62)) {
        return;
      }
      *(byte *)(iVar1 + 0xe6c) = param_2;
    }
    else {
      *(byte *)(iVar1 + 0xe6c) = param_2;
      *(undefined1 *)(iVar1 + 0xe64) = 1;
    }
    *(short *)(iVar1 + 0xe62) = (short)uVar2;
    *(int *)(iVar1 + 0xe68) = param_1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031e28 @ 0xf0031e28 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f0031e28(int param_1)

{
  int iVar1;
  
  if (param_1 == 2) {
    iVar1 = _DAT_ram_020a0064 + 0x8cc;
  }
  else {
    iVar1 = _DAT_ram_020a0064 + 900;
  }
  return iVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031e40 @ 0xf0031e40 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * FUN_ram_f0031e40(void)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  
  iVar1 = _DAT_ram_020a0064;
  piVar2 = *(int **)(_DAT_ram_020a0064 + 0xe8c);
  if ((int *)(_DAT_ram_020a0064 + 0xe8c) == piVar2) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar3 = (int *)piVar2[1];
    iVar4 = *piVar2;
    *(int **)(iVar4 + 4) = piVar3;
    *piVar3 = iVar4;
    piVar2[1] = 0;
    *piVar2 = 0;
    *(int *)(iVar1 + 0xe94) = *(int *)(iVar1 + 0xe94) + -1;
  }
  return piVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031e70 @ 0xf0031e70 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

char * FUN_ram_f0031e70(undefined4 param_1,byte param_2)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  int iVar4;
  char *pcVar5;
  char *pcVar6;
  char *pcVar7;
  uint uVar8;
  int iVar9;
  
  iVar1 = _DAT_ram_020a0064;
  uVar8 = 0;
  pcVar5 = &DAT_ram_f006d9a8 + (uint)param_2 * 0x300;
  for (pcVar7 = pcVar5; (uVar8 < *(byte *)(iVar1 + 0xe19) && (*pcVar7 != '\0'));
      pcVar7 = pcVar7 + 0x40) {
    iVar4 = *(int *)(iVar1 + 0xe14);
    iVar9 = *(int *)(pcVar7 + 4);
    iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    if ((iVar9 + iVar4 * 0x18000) - iVar2 < 0) break;
    uVar8 = uVar8 + 1 & 0xff;
  }
  if (*(byte *)(iVar1 + 0xe19) <= uVar8) {
    pcVar3 = pcVar5 + (uint)*(byte *)(iVar1 + 0xe19) * 0x40;
    for (pcVar6 = pcVar5; pcVar7 = pcVar5, pcVar6 != pcVar3; pcVar6 = pcVar6 + 0x40) {
      if (*(int *)(pcVar6 + 8) < *(int *)(pcVar5 + 8)) {
        pcVar5 = pcVar6;
      }
    }
  }
  return pcVar7;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031f08 @ 0xf0031f08 =====


void FUN_ram_f0031f08(void)

{
  DAT_ram_f006d9a8 = 0;
  DAT_ram_f006dca8 = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031f1c @ 0xf0031f1c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0031f1c(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int *piVar4;
  uint uVar5;
  
  iVar1 = _DAT_ram_020a0064;
  iVar2 = FUN_ram_f003ab1c(*(undefined1 *)(param_1 + 0x49));
  uVar3 = FUN_ram_f003aaf8(*(undefined1 *)(param_1 + 0x49));
  piVar4 = (int *)(iVar1 + 0x970);
  uVar5 = 0;
  while( true ) {
    if (*(byte *)(iVar1 + 0x96c) <= uVar5) {
      return 0;
    }
    if ((*piVar4 == iVar2) && (*(byte *)(piVar4 + 1) == uVar3)) break;
    uVar5 = uVar5 + 1;
    piVar4 = piVar4 + 2;
  }
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031f74 @ 0xf0031f74 =====


bool FUN_ram_f0031f74(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  
  iVar2 = (*(code *)&SUB_ram_0006a074)(param_1 + 0x38,param_2 + 0x38,6);
  if ((iVar2 == 0) &&
     (iVar2 = (*(code *)&SUB_ram_0006a074)
                        (param_1 + 0x18,param_2 + 0x18,*(undefined1 *)(param_1 + 0x3e)), iVar2 == 0)
     ) {
    bVar1 = *(short *)(param_1 + 0x3e) == *(short *)(param_2 + 0x3e);
  }
  else {
    bVar1 = false;
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0031fc0 @ 0xf0031fc0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

char * FUN_ram_f0031fc0(undefined4 param_1,byte param_2)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  byte bVar4;
  
  iVar1 = _DAT_ram_020a0064;
  pcVar3 = &DAT_ram_f006d9a8 + (uint)param_2 * 0x300;
  bVar4 = 0;
  while( true ) {
    if (*(byte *)(iVar1 + 0xe19) <= bVar4) {
      return (char *)0x0;
    }
    if ((*pcVar3 == '\x01') && (iVar2 = FUN_ram_f0031f74(param_1,pcVar3), iVar2 == 1)) break;
    bVar4 = bVar4 + 1;
    pcVar3 = pcVar3 + 0x40;
  }
  return pcVar3;
}

// >>> MOD: wifi/mgmt/scan_fsm.c:0xc47,0xc94 <<<
// ===== FUN_ram_f0032024 @ 0xf0032024 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0032024(int param_1)

{
  byte *pbVar1;
  short sVar2;
  byte bVar3;
  undefined2 uVar4;
  int iVar5;
  undefined1 uVar6;
  ushort uVar7;
  int iVar8;
  undefined4 *puVar9;
  undefined4 uVar10;
  char cVar11;
  char *pcVar12;
  uint uVar13;
  undefined4 local_68;
  undefined4 local_64;
  int local_60;
  undefined4 uStack_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 uStack_50;
  undefined4 uStack_4c;
  undefined4 local_48;
  undefined4 uStack_44;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  undefined4 local_38;
  undefined4 uStack_34;
  undefined4 uStack_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined2 local_24;
  undefined2 uStack_22;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00636ec,0xc47,_DAT_ram_00014800);
  }
  iVar5 = _DAT_ram_020a0064;
  if ((((*(char *)(_DAT_ram_020a0064 + 0xe3c) != '\0') && (*(int *)(_DAT_ram_020a0064 + 0x380) == 2)
       ) && ((uVar7 = **(ushort **)(param_1 + 0x14) & 0xfc, uVar7 == 0x50 || (uVar7 == 0x80)))) &&
     (iVar8 = FUN_ram_f0031f1c(param_1), iVar8 != 0)) {
    (*(code *)&SUB_ram_0006a0a0)(&local_68,0,0x40);
    iVar8 = *(int *)(param_1 + 0x14);
    uStack_30 = *(undefined4 *)(iVar8 + 0x10);
    pcVar12 = (char *)(iVar8 + 0x24);
    uVar4 = (undefined2)*(undefined4 *)(iVar8 + 0x14);
    _local_24 = CONCAT22(uStack_22,uVar4);
    sVar2 = *(short *)(param_1 + 0x12);
    bVar3 = *(byte *)(param_1 + 0x11);
    local_2c = (uint)local_2c._3_1_ << 0x18;
    local_2c = CONCAT22(local_2c._2_2_,uVar4);
    local_28 = uStack_30;
    for (uVar13 = 0; uVar13 < (ushort)(sVar2 + -0x24 + (ushort)bVar3);
        uVar13 = uVar13 + *pbVar1 + 2 & 0xffff) {
      if (*pcVar12 == '\0') {
        (*(code *)&SUB_ram_0006a08c)(&uStack_50,pcVar12 + 2,pcVar12[1]);
      }
      pbVar1 = (byte *)(pcVar12 + 1);
      pcVar12 = pcVar12 + *pbVar1 + 2;
    }
    if (local_2c._2_1_ != '\0') {
      if (*(byte *)(param_1 + 0x48) < 0xdd) {
        local_60 = ((int)(uint)*(byte *)(param_1 + 0x48) >> 1) + -0x6e;
      }
      else {
        local_60 = 0;
      }
      uVar6 = FUN_ram_f003aaf8(*(undefined1 *)(param_1 + 0x49));
      local_2c = CONCAT13(uVar6,(undefined3)local_2c);
      local_64 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
      local_54 = 0;
      local_58 = 0;
      cVar11 = *(char *)(iVar5 + 0xe18) - *(char *)(iVar5 + 0xe1c);
      puVar9 = (undefined4 *)FUN_ram_f0031fc0(&local_68,cVar11);
      if ((puVar9 == (undefined4 *)0x0) &&
         (puVar9 = (undefined4 *)FUN_ram_f0031e70(&local_68,cVar11), puVar9 == (undefined4 *)0x0)) {
        (*_DAT_ram_00014800)(0,0,0xf00636ec,0xc94,_DAT_ram_00014800);
      }
      *puVar9 = local_68;
      puVar9[1] = local_64;
      puVar9[2] = local_60;
      puVar9[3] = uStack_5c;
      puVar9[4] = local_58;
      puVar9[5] = local_54;
      puVar9[6] = uStack_50;
      puVar9[7] = uStack_4c;
      puVar9[8] = local_48;
      puVar9[9] = uStack_44;
      puVar9[10] = uStack_40;
      puVar9[0xb] = uStack_3c;
      puVar9[0xc] = local_38;
      puVar9[0xd] = uStack_34;
      puVar9[0xe] = uStack_30;
      puVar9[0xf] = local_2c;
      *(undefined1 *)puVar9 = 1;
      uVar10 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
      *(undefined4 *)(iVar5 + 0xe38) = uVar10;
      return 1;
    }
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00321f0 @ 0xf00321f0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f00321f0(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  iVar1 = _DAT_ram_020a0064;
  puVar3 = *(undefined4 **)(_DAT_ram_020a0064 + 0xe8c);
  do {
    puVar2 = puVar3;
    if (puVar2 == (undefined4 *)(_DAT_ram_020a0064 + 0xe8c)) {
      return 0;
    }
    puVar3 = (undefined4 *)*puVar2;
  } while (puVar2[2] != param_1);
  puVar4 = (undefined4 *)puVar2[1];
  puVar3[1] = puVar4;
  *puVar4 = puVar3;
  puVar2[1] = 0;
  *puVar2 = 0;
  *(int *)(iVar1 + 0xe94) = *(int *)(iVar1 + 0xe94) + -1;
  FUN_ram_f0025cac();
  return 1;
}

// >>> MOD: wifi/mgmt/scan_fsm.c:0x844 <<<
// ===== FUN_ram_f0032244 @ 0xf0032244 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0032244(int param_1,byte param_2,char *param_3)

{
  char cVar1;
  char cVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  undefined4 *puVar8;
  char *pcVar9;
  uint uVar10;
  undefined4 local_194;
  char local_190 [367];
  byte local_21 [5];
  
  uVar10 = *(uint *)(param_1 + 0x9c);
  uVar4 = (uint)param_2;
  if (uVar10 < 3) {
    if (uVar10 == 0) {
      *(undefined4 *)(param_1 + 0x9c) = 1;
      FUN_ram_f0032244(param_1,0,0);
      *(undefined4 *)(param_1 + 0x9c) = 2;
      FUN_ram_f0032244(param_1,0,0);
      *(undefined4 *)(param_1 + 0x9c) = 0;
      return;
    }
    puVar8 = &local_194;
    uVar3 = 0x24ba58;
    if (uVar10 != 1) {
      uVar3 = 5000000;
    }
    FUN_ram_f003b600(uVar3,0x2e,local_21,puVar8);
    uVar10 = (uint)local_21[0];
    iVar6 = 0x2e - (uint)*(byte *)(param_1 + 0xa0);
    uVar4 = 0;
    while( true ) {
      if (uVar4 == (uVar10 * ((int)uVar10 < iVar6) | iVar6 * (uint)(iVar6 <= (int)uVar10))) break;
      *(undefined4 *)(param_1 + (uVar4 + *(byte *)(param_1 + 0xa0) + 0x14) * 8 + 4) = *puVar8;
      *(undefined1 *)(param_1 + (uVar4 + *(byte *)(param_1 + 0xa0) + 0x14) * 8 + 8) =
           *(undefined1 *)(puVar8 + 1);
      puVar8 = puVar8 + 2;
      *(undefined4 *)(param_1 + (uVar4 + *(byte *)(param_1 + 0xa0) + 0x84) * 4 + 4) = 0;
      uVar4 = uVar4 + 1;
    }
    cVar2 = (char)uVar4 + *(byte *)(param_1 + 0xa0);
  }
  else {
    if (uVar10 != 3) {
      if (uVar10 == 4) {
        iVar6 = 0x2e - (uint)*(byte *)(param_1 + 0xa0);
        uVar10 = uVar4 * ((int)uVar4 < iVar6) | iVar6 * (uint)(iVar6 <= (int)uVar4);
        for (uVar4 = 0; uVar4 < uVar10; uVar4 = uVar4 + 1) {
          if (*param_3 == '\x01') {
            *(undefined4 *)(param_1 + (uVar4 + *(byte *)(param_1 + 0xa0) + 0x14) * 8 + 4) = 0x24ba58
            ;
          }
          else if (*param_3 == '\x02') {
            *(undefined4 *)(param_1 + (uVar4 + *(byte *)(param_1 + 0xa0) + 0x14) * 8 + 4) = 5000000;
          }
          else {
            *(undefined4 *)(param_1 + (uVar4 + *(byte *)(param_1 + 0xa0) + 0x14) * 8 + 4) = 0;
          }
          *(char *)(param_1 + (uVar4 + *(byte *)(param_1 + 0xa0) + 0x14) * 8 + 8) = param_3[1];
          param_3 = param_3 + 2;
          *(undefined4 *)(param_1 + (uVar4 + *(byte *)(param_1 + 0xa0) + 0x84) * 4 + 4) = 0;
        }
        *(char *)(param_1 + 0xa0) = (char)uVar10 + *(char *)(param_1 + 0xa0);
        return;
      }
      (*_DAT_ram_00014800)(0,0,0xf00636ec,0x844,_DAT_ram_00014800);
      return;
    }
    FUN_ram_f003b600(0x24ba58,0x2e,local_21,&local_194);
    uVar4 = 0;
    iVar6 = 0x2e - (uint)*(byte *)(param_1 + 0xa0);
    cVar2 = '\x01';
    iVar5 = 0;
    do {
      uVar10 = 0;
      pcVar9 = local_190;
      do {
        uVar7 = uVar10;
        if ((uint)local_21[0] <= (uVar7 & 0xff)) goto LAB_ram_f00323b2;
        cVar1 = *pcVar9;
        pcVar9 = pcVar9 + 8;
        uVar10 = uVar7 + 1;
      } while (cVar1 != cVar2);
      *(undefined4 *)(param_1 + (iVar5 + (uint)*(byte *)(param_1 + 0xa0) + 0x14) * 8 + 4) =
           (&local_194)[uVar7 * 2];
      *(char *)(param_1 + (iVar5 + (uint)*(byte *)(param_1 + 0xa0) + 0x14) * 8 + 8) =
           local_190[uVar7 * 8];
      uVar4 = uVar4 + 1 & 0xff;
      *(undefined4 *)(param_1 + (iVar5 + (uint)*(byte *)(param_1 + 0xa0) + 0x84) * 4 + 4) = 0;
LAB_ram_f00323b2:
      if (uVar4 == (iVar6 * (uint)(iVar6 < 3) | (uint)(2 < iVar6) * 3)) break;
      cVar2 = cVar2 + '\x05';
      iVar5 = iVar5 + 1;
    } while (iVar5 != 3);
    cVar2 = (char)uVar4 + *(char *)(param_1 + 0xa0);
  }
  *(char *)(param_1 + 0xa0) = cVar2;
  return;
}

// >>> MOD: wifi/mgmt/scan_fsm.c:0x3ef,0x40a <<<
// ===== FUN_ram_f0032490 @ 0xf0032490 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0032490(uint param_1)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int *piVar5;
  char cVar6;
  undefined1 uVar7;
  int iVar8;
  int iVar9;
  undefined4 extraout_a1;
  undefined4 extraout_a1_00;
  int iVar10;
  undefined2 *puVar11;
  int iVar12;
  uint uVar13;
  undefined4 *puVar14;
  uint uVar15;
  undefined8 uVar16;
  uint auStack_24 [2];
  
  iVar2 = _DAT_ram_020a0064;
  iVar3 = _DAT_ram_020a0064 + 0x8cc;
  iVar8 = _DAT_ram_020a0064 + 0xe74;
LAB_ram_f00324b4:
  do {
    while( true ) {
      uVar15 = _DAT_ram_0209f800;
      iVar12 = (_DAT_ram_0209f800 * 0x34 + 1) * 8;
      uVar13 = (uint)*(byte *)(iVar12 + 0x209f809);
      uVar4 = (*_DAT_ram_00014830)();
      iVar10 = uVar13 + uVar15 * 0x1a;
      iVar9 = (iVar10 + 1) * 0x10;
      *(undefined4 *)(iVar9 + 0x209f804) = uVar4;
      iVar10 = iVar10 * 0x10;
      *(uint *)(iVar9 + 0x209f808) = param_1;
      *(undefined4 *)(iVar10 + 0x209f81c) = *(undefined4 *)(iVar2 + 0x380);
      *(undefined1 *)(iVar10 + 0x209f820) = 0;
      *(char *)(iVar12 + 0x209f809) = (char)((uVar13 + 1) % 0x19);
      piVar5 = (int *)FUN_ram_f0031e28(*(undefined4 *)(iVar2 + 0x380));
      *(uint *)(iVar2 + 0x378) = param_1;
      if (param_1 == 5) {
        FUN_ram_f002672c(iVar2 + 0xe40,
                         (uint)*(byte *)((int)piVar5 + 0x97) - (uint)*(byte *)((int)piVar5 + 0x96));
        piVar5[0x14e] = (uint)*(byte *)((int)piVar5 + 0x97) - (uint)*(byte *)((int)piVar5 + 0x96);
        FUN_ram_f003718c(3,0x20);
        FUN_ram_f0031a3c(piVar5);
        return;
      }
      if (param_1 < 6) break;
      if (param_1 == 0xb) {
        *(undefined4 *)(iVar2 + 0x380) = 2;
        if (*(char *)(iVar2 + 0xe1c) == '\0') {
          *(undefined1 *)(iVar2 + 0xe1c) = *(undefined1 *)(iVar2 + 0xe18);
        }
        (*_DAT_ram_00014814)(2,0xf0063730,*(undefined1 *)(iVar2 + 0xe1c),_DAT_ram_00014814);
        FUN_ram_f00266d8(iVar8);
        *(undefined1 *)(iVar2 + 0x96d) = 0;
LAB_ram_f0032b1c:
        param_1 = 2;
      }
      else if (param_1 < 0xc) {
        if (param_1 == 6) {
          if (*(int *)(iVar2 + 900) == 0) {
            iVar3 = piVar5[0x14c];
LAB_ram_f00329c2:
            piVar5[0x14e] = iVar3;
          }
          else {
            if (*(char *)(iVar2 + 0xe70) == '\x01') {
              if ((char)piVar5[0x150] == '\x01') {
                iVar3 = 100;
              }
              else {
                iVar3 = (uint)*(byte *)(piVar5 + 0x26) + (uint)*(byte *)((int)piVar5 + 0x97);
              }
              goto LAB_ram_f00329c2;
            }
            if ((char)piVar5[0x150] == '\x01') {
              iVar3 = FUN_ram_f0031d90();
              iVar8 = (*_DAT_ram_00014e3c)(0,_DAT_ram_00014e3c);
              if ((uint)(iVar8 * iVar3) < (uint)*(byte *)((int)piVar5 + 0x97) * 0x2ee) {
                if ((uint)(iVar8 * iVar3) < (uint)*(byte *)((int)piVar5 + 0x97) * 500) {
                  iVar3 = 0x30;
                }
                else {
                  iVar3 = 0x1c;
                }
              }
              else {
                iVar3 = 8;
              }
              goto LAB_ram_f00329c2;
            }
            piVar5[0x14e] = piVar5[0x14d];
          }
          iVar3 = piVar5[0x14e];
LAB_ram_f0032a72:
          FUN_ram_f002672c(iVar2 + 0xe40,iVar3);
          return;
        }
        if (param_1 != 7) {
          return;
        }
        iVar9 = FUN_ram_f0025c48(0xc);
        if (iVar9 == 0) {
          iVar3 = 0;
          goto LAB_ram_f0032a72;
        }
        puVar11 = *(undefined2 **)(iVar9 + 8);
        *(undefined1 *)(puVar11 + 2) = 0x15;
        *puVar11 = 0xc;
        *(undefined1 *)((int)puVar11 + 5) = 0;
        *(char *)(puVar11 + 4) = (char)piVar5[0xb3];
        *(undefined1 *)((int)puVar11 + 9) = *(undefined1 *)(iVar2 + 0xe64);
        if (*(char *)(iVar2 + 0xe64) == '\x01') {
          uVar7 = 1;
          if (*(int *)(iVar2 + 0xe68) != 0x24ba58) {
            if (*(int *)(iVar2 + 0xe68) == 5000000) {
              uVar7 = 2;
            }
            else {
              uVar7 = 0;
            }
          }
          *(undefined1 *)(puVar11 + 5) = uVar7;
          *(undefined1 *)((int)puVar11 + 0xb) = *(undefined1 *)(iVar2 + 0xe6c);
        }
        *(undefined1 *)(iVar9 + 0x2f) = 0;
        *(undefined2 **)(iVar9 + 0x28) = puVar11;
        puVar11[1] = 1;
        uVar4 = FUN_ram_f0039750(iVar9);
        *(undefined4 *)(iVar2 + 0x380) = 3;
        param_1 = 0;
        uVar4 = (*_DAT_ram_00014830)(uVar4,_DAT_ram_00014830);
        *(undefined4 *)(iVar2 + 0x37c) = uVar4;
      }
      else if (param_1 == 0xc) {
        (*_DAT_ram_00014814)(2,0xf006374c,_DAT_ram_00014814);
        if (*(char *)(iVar2 + 0xe1c) != '\0') {
          *(char *)(iVar2 + 0xe1c) = *(char *)(iVar2 + 0xe1c) + -1;
        }
        if (*(char *)(iVar2 + 0xe1c) == '\0') {
          FUN_ram_f002672c(iVar8,*(int *)(iVar2 + 0xe14) * 1000);
          *(undefined4 *)(iVar2 + 0x380) = 3;
          param_1 = 0;
        }
        else {
          param_1 = 0xb;
        }
      }
      else {
        if (param_1 != 0xd) {
          return;
        }
        *(undefined4 *)(iVar2 + 0x380) = 3;
        if (*(char *)(iVar2 + 0xe3c) == '\0') {
          FUN_ram_f0031f08(iVar3);
          (*_DAT_ram_00014814)(2,0xf0063704,_DAT_ram_00014814);
          param_1 = 0;
        }
        else {
          *(undefined1 *)(iVar2 + 0xe1c) = 0;
          param_1 = 0;
          FUN_ram_f002672c(iVar8,*(int *)(iVar2 + 0xe14) * 1000);
          (*_DAT_ram_00014814)(2,0xf0063714,_DAT_ram_00014814);
        }
      }
    }
    if (param_1 != 2) {
      if (2 < param_1) {
        if (param_1 != 3) {
          if (param_1 != 4) {
            return;
          }
          FUN_ram_f002672c(iVar2 + 0xe40,*(undefined1 *)((int)piVar5 + 0x96));
          piVar5[0x14e] = (uint)*(byte *)((int)piVar5 + 0x96);
          return;
        }
        uVar15 = (uint)*(byte *)((int)piVar5 + 0xa1);
        if (*(byte *)(piVar5 + 0x28) <= uVar15) {
          iVar9 = *(int *)(iVar2 + 0x380);
          goto joined_r0xf00327a0;
        }
        uVar16 = (*_DAT_ram_00014a90)(auStack_24,_DAT_ram_00014a90);
        uVar4 = (undefined4)((ulonglong)uVar16 >> 0x20);
        if ((int)uVar16 == 0) {
          *(uint *)(iVar2 + 0xe58) = auStack_24[0];
          if (((piVar5[1] == 1) && (*piVar5 == 0)) || (*(short *)((int)piVar5 + 0x9a) != 0)) {
            auStack_24[0] = auStack_24[0] & 0xfffff7ef;
          }
          else {
            auStack_24[0] = auStack_24[0] & 0xfffff7ff;
          }
          (*_DAT_ram_00014af0)(auStack_24[0],_DAT_ram_00014af0);
          uVar4 = extraout_a1;
        }
        cVar6 = (char)piVar5[(uVar15 + 0x14) * 2 + 2];
        iVar9 = piVar5[(uVar15 + 0x14) * 2 + 1];
        if (((DAT_ram_f0065039 != cVar6) || (_DAT_ram_f006513c != iVar9)) ||
           (_DAT_ram_f0065144 != piVar5[uVar15 + 0x85])) {
          uVar16 = FUN_ram_f004571c(iVar9,cVar6,0);
          (*_DAT_ram_00014ab8)(uVar16,_DAT_ram_00014ab8);
          uVar4 = extraout_a1_00;
        }
        (*_DAT_ram_00014998)(&DAT_ram_f006e4c0,uVar4,_DAT_ram_00014998);
        *(undefined4 *)(iVar2 + 0xe5c) = _DAT_ram_f006e4e0;
        *(undefined2 *)(iVar2 + 0xe60) = _DAT_ram_f006e4cc;
        if (*piVar5 == 1) {
          iVar10 = FUN_ram_f003c308(iVar9,cVar6,0);
          if ((iVar10 == 1) && (iVar9 = FUN_ram_f003c334(iVar9,cVar6,0), iVar9 == 0)) {
            *(undefined1 *)(iVar2 + 0xe70) = 0;
            if (*(char *)((int)piVar5 + 0x96) == '\0') {
              param_1 = 5;
            }
            else {
              param_1 = 4;
            }
            goto LAB_ram_f00324b4;
          }
          uVar7 = 1;
        }
        else {
          uVar7 = 0;
        }
        *(undefined1 *)(iVar2 + 0xe70) = uVar7;
        param_1 = 6;
        goto LAB_ram_f00324b4;
      }
      if (param_1 != 0) {
        if (param_1 != 1) {
          return;
        }
        *(undefined4 *)(iVar2 + 0x380) = 0;
        goto LAB_ram_f0032b1c;
      }
      iVar9 = FUN_ram_f0031e40();
      if (iVar9 == 0) {
        return;
      }
      if (*(int *)(iVar9 + 8) == 0) {
        uVar15 = (_DAT_ram_0209f800 + 1) % 3;
        param_1 = 1;
        puVar14 = (undefined4 *)(uVar15 * 0x1a0 + 0x209f804);
        _DAT_ram_0209f800 = uVar15;
        (*(code *)&SUB_ram_0006a0a0)(puVar14,0,0x1a0);
        uVar4 = (*_DAT_ram_00014830)();
        *puVar14 = uVar4;
LAB_ram_f003265a:
        bVar1 = true;
        iVar10 = uVar15 * 0x34 + 1;
        puVar14[1] = *(undefined4 *)(iVar9 + 8);
        *(int *)(iVar10 * 8 + 0x209f804) = piVar5[0x27];
        *(char *)(iVar10 * 8 + 0x209f808) = (char)piVar5[1];
      }
      else {
        if (*(int *)(iVar9 + 8) == 2) {
          uVar15 = (_DAT_ram_0209f800 + 1) % 3;
          param_1 = 0xb;
          puVar14 = (undefined4 *)(uVar15 * 0x1a0 + 0x209f804);
          _DAT_ram_0209f800 = uVar15;
          (*(code *)&SUB_ram_0006a0a0)(puVar14,0,0x1a0);
          uVar4 = (*_DAT_ram_00014830)();
          *puVar14 = uVar4;
          goto LAB_ram_f003265a;
        }
        (*_DAT_ram_00014800)(0,0,0xf00636ec,0x3ef,_DAT_ram_00014800);
        bVar1 = false;
      }
      FUN_ram_f0025cac(iVar9);
      if (!bVar1) {
        return;
      }
      goto LAB_ram_f00324b4;
    }
    if (*(byte *)((int)piVar5 + 0xa1) < *(byte *)(piVar5 + 0x28)) {
      iVar3 = FUN_ram_f0025b48(0,0x14);
      if (iVar3 == 0) {
        (*_DAT_ram_00014800)(0,0,0xf00636ec,0x40a,_DAT_ram_00014800);
        return;
      }
      *(undefined4 *)(iVar3 + 8) = 2;
      *(char *)(iVar3 + 0xc) = (char)piVar5[1];
      cVar6 = *(char *)((int)piVar5 + 0x2cd) + '\x01';
      *(char *)((int)piVar5 + 0x2cd) = cVar6;
      *(char *)(iVar3 + 0xd) = cVar6;
      if (((*(byte *)(_DAT_ram_f0065118 + 0x10c) & 1) == 0) &&
         ((*(byte *)(_DAT_ram_f0065118 + 0x10c) >> 1 & 1) == 0)) {
        uVar15 = (uint)*(byte *)(piVar5 + 0x28) - (uint)*(byte *)((int)piVar5 + 0xa1);
        if ((int)uVar15 < 6) {
          uVar15 = uVar15 & 0xff;
        }
        else {
          uVar15 = 5;
        }
      }
      else {
        uVar15 = 1;
      }
      if (*piVar5 == 0) {
        uVar13 = (uint)*(ushort *)((int)piVar5 + 0x9a);
        piVar5[0x14c] = uVar13;
      }
      else {
        if ((char)piVar5[0x150] == '\x01') {
          iVar8 = 100;
        }
        else {
          iVar8 = (uint)*(byte *)(piVar5 + 0x26) + (uint)*(byte *)((int)piVar5 + 0x97);
        }
        piVar5[0x14c] = iVar8;
        uVar13 = (uint)*(byte *)(piVar5 + 0x26);
      }
      piVar5[0x14d] = uVar13;
      iVar8 = uVar15 * (piVar5[0x14c] + 5);
      piVar5[0x14a] = iVar8;
      *(int *)(iVar3 + 0x10) = iVar8;
      *(undefined1 *)(iVar2 + 0xe71) = 1;
      FUN_ram_f0028344(0,iVar3,0);
      return;
    }
    iVar9 = *(int *)(iVar2 + 0x380);
joined_r0xf00327a0:
    if (iVar9 == 2) {
      param_1 = 0xc;
    }
    else {
      param_1 = 7;
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/scan_fsm.c:0x671,0x679 <<<
// ===== FUN_ram_f0032e68 @ 0xf0032e68 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0032e68(char param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  
  iVar1 = _DAT_ram_020a0064;
  iVar2 = FUN_ram_f0031e28(*(undefined4 *)(_DAT_ram_020a0064 + 0x380));
  iVar8 = _DAT_ram_0209f800;
  iVar5 = *(int *)(iVar1 + 0x378);
  iVar6 = (_DAT_ram_0209f800 * 0x34 + 1) * 8;
  uVar7 = (uint)*(byte *)(iVar6 + 0x209f809);
  uVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  iVar8 = uVar7 + iVar8 * 0x1a;
  iVar4 = (iVar8 + 1) * 0x10;
  *(undefined4 *)(iVar4 + 0x209f804) = uVar3;
  iVar8 = iVar8 * 0x10;
  *(int *)(iVar4 + 0x209f808) = iVar5;
  *(undefined4 *)(iVar8 + 0x209f81c) = *(undefined4 *)(iVar1 + 0x380);
  *(undefined1 *)(iVar8 + 0x209f820) = 1;
  *(char *)(iVar6 + 0x209f809) = (char)((uVar7 + 1) % 0x19);
  uVar7 = *(uint *)(iVar1 + 0x378);
  if (uVar7 < 7) {
    if (uVar7 == 0) goto LAB_ram_f0032f6e;
    if (uVar7 != 2) {
      (*_DAT_ram_00014af0)(*(undefined4 *)(iVar1 + 0xe58),_DAT_ram_00014af0);
    }
    FUN_ram_f00266d8(iVar1 + 0xe40);
LAB_ram_f0032f32:
    if (*(int *)(iVar1 + 0x380) != 2) {
      *(undefined4 *)(iVar1 + 0x380) = 3;
      iVar5 = 0;
      goto LAB_ram_f0032f6e;
    }
  }
  else {
    if (uVar7 == 7) goto LAB_ram_f0032f32;
    if (1 < uVar7 - 0xb) {
      (*_DAT_ram_00014800)(0,0,0xf00636ec,0x671,_DAT_ram_00014800);
      goto LAB_ram_f0032f6e;
    }
  }
  FUN_ram_f00266d8(iVar1 + 0xe74);
  iVar5 = 0xd;
LAB_ram_f0032f6e:
  if ((param_1 == '\0') && (*(char *)(iVar1 + 0xe71) == '\x01')) {
    iVar8 = FUN_ram_f0025b48(0,0x10);
    if (iVar8 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf00636ec,0x679,_DAT_ram_00014800);
      return;
    }
    *(undefined4 *)(iVar8 + 8) = 3;
    *(undefined1 *)(iVar8 + 0xc) = *(undefined1 *)(iVar2 + 4);
    *(undefined1 *)(iVar8 + 0xd) = *(undefined1 *)(iVar2 + 0x2cd);
    *(undefined1 *)(iVar1 + 0xe71) = 0;
    (*_DAT_ram_00014814)(2,0xf006375c,_DAT_ram_00014814);
    FUN_ram_f0028344(0,iVar8,0);
  }
  if (*(int *)(iVar1 + 0x378) != iVar5) {
    FUN_ram_f0032490(iVar5);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0032ffc @ 0xf0032ffc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0032ffc(int param_1)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  undefined4 uVar4;
  
  iVar1 = _DAT_ram_020a0064;
  piVar2 = (int *)FUN_ram_f0025b48(0,0xc);
  if (piVar2 == (int *)0x0) {
    uVar4 = 0;
  }
  else {
    piVar2[2] = param_1;
    piVar3 = *(int **)(iVar1 + 0xe90);
    *(int **)(iVar1 + 0xe90) = piVar2;
    *piVar2 = iVar1 + 0xe8c;
    piVar2[1] = (int)piVar3;
    *piVar3 = (int)piVar2;
    *(int *)(iVar1 + 0xe94) = *(int *)(iVar1 + 0xe94) + 1;
    if ((*(int *)(iVar1 + 0x380) == 2) && (piVar2[2] == 0)) {
      (*_DAT_ram_00014814)(2,0xf006377c,_DAT_ram_00014814);
      FUN_ram_f0032ffc(2);
      FUN_ram_f0032e68(0);
    }
    uVar4 = 1;
  }
  return uVar4;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003359c @ 0xf003359c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_ram_f003359c(undefined1 param_1,byte param_2,byte param_3,short param_4,ushort param_5,
                byte param_6,undefined1 param_7,undefined4 param_8,undefined1 param_9,byte param_10,
                uint *param_11,undefined1 param_12,short param_13,undefined4 param_14)

{
  char cVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  char cVar7;
  undefined1 uVar8;
  undefined1 *puVar9;
  int iVar10;
  undefined4 *puVar11;
  
  iVar10 = _DAT_ram_020a0064;
  uVar3 = (uint)param_2;
  if (DAT_ram_f0064fc1 == '\x01') {
    return 0;
  }
  if (DAT_ram_f0064fc4 == '\x01') {
    return 0;
  }
  *(uint *)(_DAT_ram_020a0064 + 900) = (uint)param_3;
  *(uint *)(iVar10 + 0x388) = uVar3;
  if (uVar3 == 1) {
    (*(code *)&SUB_ram_0006a08c)(iVar10 + 0x412,_DAT_ram_020a006c + 0x51,6);
  }
  else {
    if (uVar3 == 2) {
      iVar4 = _DAT_ram_020a0070 + 0x51;
    }
    else {
      iVar4 = _DAT_ram_020a0064 + 0x2088;
    }
    (*(code *)&SUB_ram_0006a08c)(iVar10 + 0x412,iVar4,6,2,&SUB_ram_0006a08c);
  }
  if (*(int *)(iVar10 + 900) == 1) {
    *(undefined1 *)(iVar10 + 0x38c) = param_9;
    puVar9 = (undefined1 *)(iVar10 + 0x38e);
    iVar4 = iVar10 + 0x392;
    *(byte *)(iVar10 + 0x38d) = param_10;
    uVar3 = 0;
    while (uVar3 < param_10) {
      uVar3 = uVar3 + 1;
      *puVar9 = (char)*param_11;
      uVar5 = *param_11;
      if (uVar5 != 0) {
        uVar6 = 0x20;
        if (uVar5 < 0x21) {
          uVar6 = uVar5;
        }
        (*(code *)&SUB_ram_0006a08c)(iVar4,param_11 + 1,uVar6);
      }
      param_11 = param_11 + 9;
      puVar9 = puVar9 + 1;
      iVar4 = iVar4 + 0x20;
    }
    if (param_5 == 0) {
      *(undefined1 *)(iVar10 + 0x8c4) = 1;
      *(undefined1 *)(iVar10 + 0x41b) = 0xc;
      cVar7 = '0';
LAB_ram_f003370c:
      *(char *)(iVar10 + 0x41c) = cVar7;
    }
    else {
      *(undefined1 *)(iVar10 + 0x8c4) = 0;
      cVar7 = (char)param_5;
      if (param_5 < 0xd) {
        *(undefined1 *)(iVar10 + 0x41c) = 0;
        *(char *)(iVar10 + 0x41b) = cVar7;
      }
      else {
        if (param_4 == 0) {
          cVar7 = cVar7 + -0xc;
          *(undefined1 *)(iVar10 + 0x41b) = 0xc;
          goto LAB_ram_f003370c;
        }
        *(char *)(iVar10 + 0x41c) = cVar7 - (char)param_4;
        *(char *)(iVar10 + 0x41b) = (char)param_4;
      }
    }
LAB_ram_f0033710:
    *(ushort *)(iVar10 + 0x41e) = param_5;
  }
  else {
    *(undefined1 *)(iVar10 + 0x8c4) = 0;
    *(undefined1 *)(iVar10 + 0x38d) = 0;
    if (param_5 != 0) goto LAB_ram_f0033710;
    *(undefined2 *)(iVar10 + 0x41e) = 100;
  }
  *(undefined1 *)(iVar10 + 0x41a) = param_12;
  *(undefined1 *)(iVar10 + 0x424) = 0;
  *(uint *)(iVar10 + 0x420) = (uint)param_6;
  FUN_ram_f0032244(iVar10 + 900,param_7,param_8);
  *(short *)(iVar10 + 0x652) = param_13;
  if (param_13 != 0) {
    (*(code *)&SUB_ram_0006a08c)(iVar10 + 0x654,param_14,param_13);
  }
  *(undefined1 *)(iVar10 + 0x650) = param_1;
  *(undefined1 *)(iVar10 + 0x425) = 0;
  if (*(int *)(iVar10 + 900) != 1) {
    *(undefined1 *)(iVar10 + 0x419) = 0;
    *(undefined1 *)(iVar10 + 0x418) = 0;
    goto LAB_ram_f00337ec;
  }
  if ((*(int *)(iVar10 + 0x388) == 1) && (*(int *)(iVar10 + 0x420) == 4)) {
    cVar7 = *(char *)(iVar10 + 0x38d);
    uVar8 = 1;
    if (cVar7 == '\0') goto LAB_ram_f00337da;
LAB_ram_f00337bc:
    *(char *)(iVar10 + 0x418) = cVar7;
  }
  else {
    cVar7 = *(char *)(iVar10 + 0x38d);
    if (cVar7 != '\0') {
      if ((*(byte *)(iVar10 + 0x38c) & 2) == 0) {
        cVar1 = cVar7 + '\x01';
      }
      else {
        if (*(int *)(iVar10 + 0x420) == 3) {
          cVar7 = cVar7 * '\x02';
          goto LAB_ram_f00337bc;
        }
        cVar1 = cVar7 * -2;
        cVar7 = cVar7 * '\x02';
      }
      *(char *)(iVar10 + 0x418) = cVar1;
      *(char *)(iVar10 + 0x419) = cVar7;
      goto LAB_ram_f00337ec;
    }
    uVar8 = 2;
    cVar7 = '\0';
LAB_ram_f00337da:
    *(undefined1 *)(iVar10 + 0x418) = uVar8;
  }
  *(char *)(iVar10 + 0x419) = cVar7;
LAB_ram_f00337ec:
  *(undefined4 *)(iVar10 + 0x8c8) = 0;
  *(undefined1 *)(iVar10 + 0xe64) = 0;
  if (*(int *)(iVar10 + 0x378) == 0) {
    uVar3 = (_DAT_ram_0209f800 + 1) % 3;
    puVar11 = (undefined4 *)(uVar3 * 0x1a0 + 0x209f804);
    _DAT_ram_0209f800 = uVar3;
    (*(code *)&SUB_ram_0006a0a0)(puVar11,0,0x1a0);
    uVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    *puVar11 = uVar2;
    iVar10 = uVar3 * 0x34 + 1;
    *(uint *)(uVar3 * 0x1a0 + 0x209f808) = (uint)param_3;
    *(uint *)(iVar10 * 8 + 0x209f804) = (uint)param_6;
    *(byte *)(iVar10 * 8 + 0x209f808) = param_2;
    FUN_ram_f0032490(1);
  }
  else {
    FUN_ram_f0032ffc();
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00339c0 @ 0xf00339c0 =====


void FUN_ram_f00339c0(uint *param_1,uint *param_2)

{
  uint uVar1;
  
  uVar1 = (*param_1 >> 0xf | *param_1 << 0x11) ^ *param_2;
  *param_2 = uVar1;
  uVar1 = uVar1 + *param_1;
  *param_1 = uVar1;
  uVar1 = ((uVar1 & 0xff00ff) << 8 | (uVar1 & 0xff00ff00) >> 8) ^ *param_2;
  *param_2 = uVar1;
  uVar1 = uVar1 + *param_1;
  *param_1 = uVar1;
  uVar1 = (uVar1 >> 0x1d | uVar1 * 8) ^ *param_2;
  *param_2 = uVar1;
  uVar1 = uVar1 + *param_1;
  *param_1 = uVar1;
  uVar1 = (uVar1 >> 2 | uVar1 * 0x40000000) ^ *param_2;
  *param_2 = uVar1;
  *param_1 = *param_1 + uVar1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0033a2c @ 0xf0033a2c =====


void FUN_ram_f0033a2c(byte *param_1,byte *param_2,uint param_3,byte *param_4,byte *param_5,
                     byte param_6,undefined1 *param_7)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  byte *pbVar4;
  uint local_34 [4];
  uint local_24 [2];
  
  puVar3 = local_34;
  local_34[1] = (uint)param_5[4] | (uint)param_4[1] << 0x18 | (uint)*param_4 << 0x10 |
                (uint)param_5[5] << 8;
  local_34[0] = (uint)param_5[1] << 8 | (uint)param_5[2] << 0x10 | (uint)*param_5 |
                (uint)param_5[3] << 0x18;
  puVar2 = local_34 + 3;
  local_34[2] = (uint)param_4[3] << 8 | (uint)param_4[4] << 0x10 | (uint)param_4[2] |
                (uint)param_4[5] << 0x18;
  local_24[0] = (uint)param_1[1] << 8 | (uint)param_1[2] << 0x10 | (uint)*param_1 |
                (uint)param_1[3] << 0x18;
  local_34[3] = (uint)param_1[5] << 8 | (uint)param_1[6] << 0x10 | (uint)param_1[4] |
                (uint)param_1[7] << 0x18;
  do {
    uVar1 = *puVar3;
    puVar3 = puVar3 + 1;
    local_24[0] = local_24[0] ^ uVar1;
    FUN_ram_f00339c0(local_24,puVar2);
  } while (puVar3 != puVar2);
  local_24[0] = local_24[0] ^ param_6;
  FUN_ram_f00339c0(local_24,puVar2);
  pbVar4 = param_2;
  for (uVar1 = 0; uVar1 < param_3 >> 2; uVar1 = uVar1 + 1) {
    local_24[0] = local_24[0] ^
                  ((uint)pbVar4[1] << 8 | (uint)pbVar4[2] << 0x10 | (uint)*pbVar4 |
                  (uint)pbVar4[3] << 0x18);
    FUN_ram_f00339c0(local_24,puVar2);
    pbVar4 = pbVar4 + 4;
  }
  uVar1 = param_3 & 3;
  if (uVar1 == 2) {
    uVar1 = (uint)param_2[param_3 - 1] << 8;
    local_34[0] = param_2[param_3 - 2] | 0x5a0000;
  }
  else {
    if (uVar1 != 3) {
      if (uVar1 == 1) {
        local_34[0] = param_2[param_3 - 1] | 0x5a00;
      }
      else {
        local_34[0] = 0x5a;
      }
      goto LAB_ram_f0033caa;
    }
    local_34[0] = param_2[param_3 - 3] | 0x5a000000 | (uint)param_2[param_3 - 2] << 8;
    uVar1 = (uint)param_2[param_3 - 1] << 0x10;
  }
  local_34[0] = local_34[0] | uVar1;
LAB_ram_f0033caa:
  local_24[0] = local_24[0] ^ local_34[0];
  local_34[1] = 0;
  FUN_ram_f00339c0(local_24,local_34 + 3);
  FUN_ram_f00339c0(local_24,local_34 + 3);
  param_7[1] = (char)(local_24[0] >> 8);
  param_7[2] = (char)(local_24[0] >> 0x10);
  param_7[3] = (char)(local_24[0] >> 0x18);
  *param_7 = (char)local_24[0];
  param_7[7] = (char)(local_34[3] >> 0x18);
  param_7[5] = (char)(local_34[3] >> 8);
  param_7[6] = (char)(local_34[3] >> 0x10);
  param_7[4] = (char)local_34[3];
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0033d24 @ 0xf0033d24 =====


undefined4 FUN_ram_f0033d24(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  char *pcVar2;
  ushort *puVar3;
  ushort *puVar4;
  byte bVar5;
  ushort uVar6;
  int iVar7;
  int iVar8;
  char local_20;
  char local_1f;
  char local_1e;
  char local_1d;
  char local_1c;
  char local_1b;
  char local_1a;
  char local_19;
  
  puVar3 = *(ushort **)(param_1 + 0x14);
  iVar8 = *(int *)(param_1 + 0x18);
  uVar6 = *(ushort *)(param_1 + 0x12);
  if ((*puVar3 & 0x300) == 0) {
    puVar4 = puVar3 + 5;
  }
  else {
    if ((*puVar3 & 0x300) != 0x200) {
      return 1;
    }
    puVar4 = puVar3 + 8;
  }
  if ((*(ushort *)(param_1 + 0x32) & 0xfc) == 0x88) {
    bVar5 = (byte)puVar3[0xc] & 0xf;
  }
  else {
    bVar5 = 0;
  }
  iVar7 = uVar6 - 8;
  FUN_ram_f0033a2c(param_2,iVar8,iVar7,puVar4,puVar3 + 2,bVar5,&local_20);
  pcVar2 = (char *)(iVar8 + iVar7);
  if (((((*pcVar2 == local_20) && (pcVar2[1] == local_1f)) && (pcVar2[2] == local_1e)) &&
      ((pcVar2[3] == local_1d && (pcVar2[4] == local_1c)))) &&
     ((pcVar2[5] == local_1b && ((pcVar2[6] == local_1a && (pcVar2[7] == local_19)))))) {
    uVar6 = (ushort)iVar7;
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  *(ushort *)(param_1 + 0x12) = uVar6;
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0033de8 @ 0xf0033de8 =====


void FUN_ram_f0033de8(undefined4 param_1,undefined4 param_2,undefined1 param_3,undefined2 param_4,
                     undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  undefined1 auStack_20 [12];
  
  FUN_ram_f0033a2c(param_7,param_5,param_4,param_2,param_1,param_3,auStack_20);
  (*(code *)&SUB_ram_0006a08c)(param_6,auStack_20,8);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0033e2c @ 0xf0033e2c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0033e2c(void)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined1 local_14;
  undefined1 local_13;
  undefined1 local_12;
  undefined1 local_11;
  
  *_DAT_ram_f0064f50 = 0;
  uVar1 = 0;
  *(undefined2 *)(_DAT_ram_f0064f50 + 1) = 0;
  _DAT_ram_f0064f50[2] = 0;
  *(undefined2 *)(_DAT_ram_f0064f50 + 0x46) = 0;
  do {
    _DAT_ram_f0064f50[2] = _DAT_ram_f0064f50[2] & ~(3 << (uVar1 & 0x1f));
    uVar1 = uVar1 + 2;
  } while (uVar1 != 0x12);
  _DAT_ram_f0064f50[2] = _DAT_ram_f0064f50[2] & 0xffff3fff;
  _DAT_ram_f0064f50[2] = _DAT_ram_f0064f50[2] | 0x8000;
  _DAT_ram_f0064f50[6] = _DAT_ram_f0064f50 + 6;
  _DAT_ram_f0064f50[7] = _DAT_ram_f0064f50 + 6;
  _DAT_ram_f0064f50[8] = 0;
  _DAT_ram_f0064f50[3] = _DAT_ram_f0064f50 + 3;
  _DAT_ram_f0064f50[4] = _DAT_ram_f0064f50 + 3;
  _DAT_ram_f0064f50[5] = 0;
  _DAT_ram_f0064f50[9] = _DAT_ram_f0064f50 + 9;
  _DAT_ram_f0064f50[10] = _DAT_ram_f0064f50 + 9;
  _DAT_ram_f0064f50[0xb] = 0;
  _DAT_ram_f0064f50[0xc] = _DAT_ram_f0064f50 + 0xc;
  _DAT_ram_f0064f50[0xd] = _DAT_ram_f0064f50 + 0xc;
  _DAT_ram_f0064f50[0xe] = 0;
  _DAT_ram_f0064f50[0xf] = _DAT_ram_f0064f50 + 0xf;
  _DAT_ram_f0064f50[0x10] = _DAT_ram_f0064f50 + 0xf;
  _DAT_ram_f0064f50[0x11] = 0;
  *(undefined1 *)(_DAT_ram_f0064f50 + 0x12) = 2;
  *(undefined1 *)((int)_DAT_ram_f0064f50 + 0x49) = 0;
  (*_DAT_ram_00014908)(0,_DAT_ram_00014908);
  (*_DAT_ram_00014900)(4,_DAT_ram_00014900);
  local_11 = 0x10;
  local_14 = 0x10;
  local_13 = 0x10;
  local_12 = 0x10;
  (*_DAT_ram_000148fc)(&local_14,_DAT_ram_000148fc);
  (*_DAT_ram_00014d84)(1,_DAT_ram_00014d84);
  iVar2 = 0;
  do {
    iVar3 = iVar2 + 1;
    _DAT_ram_f0064f50[iVar2 + 0x47] = 0;
    iVar2 = iVar3;
  } while (iVar3 != 9);
  *(undefined1 *)(_DAT_ram_f0064f50 + 0x50) = 10;
  *(undefined1 *)((int)_DAT_ram_f0064f50 + 0x142) = 1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0033f8c @ 0xf0033f8c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0033f8c(int param_1)

{
  if (*(char *)(_DAT_ram_f0064f50 + 0x142) != '\0') {
    *(undefined4 *)(param_1 + 0x358) = 1;
    *(undefined4 *)(param_1 + 0x35c) = 0;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0033fa8 @ 0xf0033fa8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0033fa8(int param_1)

{
  if (*(char *)(_DAT_ram_f0064f50 + 0x142) != '\0') {
    *(undefined4 *)(param_1 + 0x368) = 1;
    *(undefined4 *)(param_1 + 0x36c) = 0;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0033fc4 @ 0xf0033fc4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0033fc4(void)

{
  uint uVar1;
  
  if (*(short *)(_DAT_ram_f0064f50 + 4) != 0) {
    uVar1 = 0;
    do {
      if (((*(ushort *)(_DAT_ram_f0064f50 + 4) >> (uVar1 & 0x1f) & 1) != 0) &&
         ((*(uint *)(_DAT_ram_f0064f50 + 8) >> (uVar1 * 2 & 0x1f) & 3) == 0)) {
        (*_DAT_ram_00014b34)(uVar1,_DAT_ram_020a0078,_DAT_ram_00014b34);
        *(undefined1 *)(_DAT_ram_f0064f50 + 0x141) = 1;
        *(ushort *)(_DAT_ram_f0064f50 + 4) =
             ~(ushort)(1 << (uVar1 & 0x1f)) & *(ushort *)(_DAT_ram_f0064f50 + 4);
      }
      uVar1 = uVar1 + 1;
    } while (uVar1 != 9);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034034 @ 0xf0034034 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034034(undefined1 param_1)

{
  *(undefined1 *)(_DAT_ram_f0064f50 + 0x48) = param_1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034044 @ 0xf0034044 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034044(undefined1 param_1)

{
  *(undefined1 *)(_DAT_ram_f0064f50 + 0x49) = param_1;
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x13ff,0x1400,0x1413 <<<
// ===== FUN_ram_f0034054 @ 0xf0034054 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034054(uint param_1,uint param_2,int *param_3)

{
  int iVar1;
  uint uVar2;
  
  if (param_3 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x13ff,_DAT_ram_00014800);
  }
  if (*param_3 != 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1400,_DAT_ram_00014800);
  }
  iVar1 = (*_DAT_ram_00014b18)(param_2 & 0xff,1,20000,param_3,_DAT_ram_020a0078,_DAT_ram_00014b18);
  if (iVar1 != 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1413,_DAT_ram_00014800);
  }
  _DAT_ram_f0064fa2 = (undefined2)param_1;
  if (param_1 < 0xb) {
    uVar2 = 1 << (param_1 & 0x1f);
    if ((uVar2 & 0x4eb) == 0) {
      if ((uVar2 & 0x100) == 0) {
        if ((uVar2 & 4) != 0) {
          *(uint *)(_DAT_ram_f0064f50 + 8) =
               *(uint *)(_DAT_ram_f0064f50 + 8) & ~(3 << (param_2 * 2 & 0x1f));
          *(uint *)(_DAT_ram_f0064f50 + 8) =
               *(uint *)(_DAT_ram_f0064f50 + 8) | 2 << (param_2 * 2 & 0x1f);
        }
        goto LAB_ram_f00341b0;
      }
    }
    else {
      if (param_2 == 7) {
        *(uint *)(_DAT_ram_f0064f50 + 8) = *(uint *)(_DAT_ram_f0064f50 + 8) & 0xffff3fff;
        uVar2 = *(uint *)(_DAT_ram_f0064f50 + 8) | 0x8000;
LAB_ram_f003414e:
        *(uint *)(_DAT_ram_f0064f50 + 8) = uVar2;
        return;
      }
      if (param_2 == 8) {
        (*_DAT_ram_00014b3c)(8,0,_DAT_ram_020a0078,_DAT_ram_00014b3c);
        uVar2 = *(uint *)(_DAT_ram_f0064f50 + 8) & 0xfffcffff;
        goto LAB_ram_f003414e;
      }
      (*_DAT_ram_00014b3c)(param_2 & 0xff,0,_DAT_ram_020a0078,_DAT_ram_00014b3c);
    }
    uVar2 = *(uint *)(_DAT_ram_f0064f50 + 8) & ~(3 << (param_2 * 2 & 0x1f));
  }
  else {
LAB_ram_f00341b0:
    *(uint *)(_DAT_ram_f0064f50 + 8) =
         *(uint *)(_DAT_ram_f0064f50 + 8) & ~(3 << (param_2 * 2 & 0x1f));
    uVar2 = *(uint *)(_DAT_ram_f0064f50 + 8) | 2 << (param_2 * 2 & 0x1f);
  }
  *(uint *)(_DAT_ram_f0064f50 + 8) = uVar2;
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x1728 <<<
// ===== FUN_ram_f00341e8 @ 0xf00341e8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00341e8(int param_1)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  bool bVar4;
  
  cVar1 = *(char *)(param_1 + 0x10);
  if (cVar1 == '\x01') {
    bVar4 = (bool)((*(byte *)(*(int *)(param_1 + 0x14) + 4) ^ 1) & 1);
  }
  else {
    bVar4 = *(int *)(param_1 + 0x20) != 0;
  }
  if (bVar4 == false) {
    if (cVar1 == '\x01') {
      bVar4 = (**(byte **)(param_1 + 0x14) & 0xc) == 8;
    }
    else {
      bVar4 = *(char *)(param_1 + 0x5b) == '\0';
    }
    if (!bVar4) {
LAB_ram_f00342a2:
      uVar2 = 0x7f;
      goto LAB_ram_f00342a6;
    }
    if (*(char *)(param_1 + 0xd) != '\x01') {
      uVar2 = 0;
      goto LAB_ram_f00342a6;
    }
    iVar3 = *(int *)(param_1 + 0x20);
    if ((iVar3 == 0) || (*(int *)(iVar3 + 0x10) != 0x42)) {
      uVar2 = *(undefined1 *)(_DAT_ram_020a0064 + 0x197d);
      goto LAB_ram_f00342a6;
    }
  }
  else {
    if (cVar1 == '\x01') {
      bVar4 = (**(byte **)(param_1 + 0x14) & 0xc) == 8;
    }
    else {
      bVar4 = *(char *)(param_1 + 0x5b) == '\0';
    }
    if (bVar4) {
      if (*(int *)(param_1 + 0x20) == 0) {
        (*_DAT_ram_00014800)(0,0,0xf006380c,0x1728,_DAT_ram_00014800);
      }
      iVar3 = *(int *)(param_1 + 0x20);
    }
    else {
      iVar3 = *(int *)(param_1 + 0x20);
      if (iVar3 == 0) goto LAB_ram_f00342a2;
    }
  }
  uVar2 = *(undefined1 *)(iVar3 + 0x28a);
LAB_ram_f00342a6:
  *(undefined1 *)(param_1 + 0x5c) = uVar2;
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x191d <<<
// ===== FUN_ram_f00342c4 @ 0xf00342c4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00342c4(int param_1)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = (uint)*(byte *)(param_1 + 0x40);
  if (8 < uVar2) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x191d,_DAT_ram_00014800);
  }
  iVar1 = _DAT_ram_f0064f50 + (uVar2 + 0x46) * 4;
  *(int *)(iVar1 + 4) = *(int *)(iVar1 + 4) + 1;
  iVar1 = *(int *)(param_1 + 0x20);
  if (((iVar1 != 0) && (*(char *)(iVar1 + 0x15) == '\x02')) && (uVar2 < 5)) {
    *(char *)(iVar1 + uVar2 + 0x2be) = *(char *)(iVar1 + uVar2 + 0x2be) + '\x01';
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003432c @ 0xf003432c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003432c(int param_1)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  
  uVar2 = (uint)*(byte *)(param_1 + 0x40);
  iVar3 = _DAT_ram_f0064f50 + (uVar2 + 0x46) * 4;
  iVar4 = *(int *)(iVar3 + 4);
  if (iVar4 != 0) {
    *(int *)(iVar3 + 4) = iVar4 + -1;
  }
  iVar3 = *(int *)(param_1 + 0x20);
  if (((iVar3 != 0) && (*(char *)(iVar3 + 0x15) == '\x02')) && (uVar2 < 5)) {
    cVar1 = *(char *)(iVar3 + uVar2 + 0x2be);
    if (cVar1 != '\0') {
      *(char *)(iVar3 + uVar2 + 0x2be) = cVar1 + -1;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003436c @ 0xf003436c =====


int FUN_ram_f003436c(int param_1,uint param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int *piVar5;
  
  uVar4 = 3;
  piVar5 = (int *)(param_1 + 200);
  iVar3 = 0;
  do {
    iVar2 = uVar4 * 0xc;
    uVar1 = uVar4 & 0x1f;
    uVar4 = uVar4 - 1;
    if (((param_2 >> uVar1 & 1) != 0) && (*piVar5 != param_1 + iVar2 + 0xa4)) {
      iVar3 = iVar3 + piVar5[2];
    }
    piVar5 = piVar5 + -3;
  } while (uVar4 != 0xffffffff);
  return iVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00343a8 @ 0xf00343a8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f00343a8(void)

{
  return *(int *)(_DAT_ram_f0064f50 + 0xc) != _DAT_ram_f0064f50 + 0xc;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00343c0 @ 0xf00343c0 =====


void FUN_ram_f00343c0(int param_1,char param_2)

{
  char cVar1;
  ushort uVar2;
  int iVar3;
  
  cVar1 = *(char *)(param_1 + 0x10);
  if (cVar1 == '\0') {
    iVar3 = 0;
  }
  else {
    iVar3 = *(int *)(param_1 + 0x14);
  }
  if (param_2 == '\0') {
    if (cVar1 == '\0') {
      *(undefined1 *)(param_1 + 0x5e) = 0;
      return;
    }
    uVar2 = *(ushort *)(iVar3 + 0x18) & 0xffef;
  }
  else {
    if (cVar1 == '\0') {
      *(undefined1 *)(param_1 + 0x5e) = 1;
      return;
    }
    uVar2 = *(ushort *)(iVar3 + 0x18) | 0x10;
  }
  *(ushort *)(iVar3 + 0x18) = uVar2;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00343fc @ 0xf00343fc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

byte FUN_ram_f00343fc(undefined4 param_1,byte param_2)

{
  undefined4 local_4;
  
  local_4 = _DAT_ram_f00638a0;
  return *(byte *)((int)&local_4 + (param_2 & 3)) | param_2 & 0xfc;
}

// >>> MOD: wifi/mgmt/txm.c:0x2040 <<<
// ===== FUN_ram_f0034424 @ 0xf0034424 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0034424(int param_1,undefined1 param_2,byte param_3)

{
  uint uVar1;
  uint uVar2;
  int *piVar3;
  char *pcVar4;
  char cVar5;
  int iVar6;
  
  if (*(char *)(param_1 + 9) == '\0') {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x2040,_DAT_ram_00014800);
  }
  uVar1 = FUN_ram_f00343fc(0,param_2);
  piVar3 = (int *)(param_1 + 0xd4);
  pcVar4 = (char *)(param_1 + 0x2c2);
  uVar2 = 4;
  do {
    cVar5 = '\0';
    if ((param_3 & 1) != 0) {
      cVar5 = *pcVar4;
    }
    if (uVar2 == 4) {
      iVar6 = *piVar3;
joined_r0xf0034496:
      if (iVar6 != param_1 + uVar2 * 0xc + 0xa4) {
        return 1;
      }
    }
    else if ((uVar1 >> (uVar2 & 0x1f) & 1) != 0) {
      iVar6 = *piVar3;
      goto joined_r0xf0034496;
    }
    if (cVar5 != '\0') {
      return 1;
    }
    uVar2 = uVar2 - 1;
    piVar3 = piVar3 + -3;
    pcVar4 = pcVar4 + -1;
    if (uVar2 == 0xffffffff) {
      return 0;
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/txm.c:0x1bb7,0x1bbb <<<
// ===== FUN_ram_f00344b4 @ 0xf00344b4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00344b4(int param_1,int param_2)

{
  undefined1 uVar1;
  ushort uVar2;
  int iVar3;
  ushort *puVar4;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1bb7,_DAT_ram_00014800);
  }
  puVar4 = (ushort *)0x0;
  if ((*(char *)(param_2 + 0x10) != '\0') &&
     (puVar4 = *(ushort **)(param_2 + 0x14), puVar4 == (ushort *)0x0)) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1bbb,_DAT_ram_00014800);
  }
  iVar3 = FUN_ram_f0034424(param_1,0xf,0);
  if ((iVar3 == 0) && (*(char *)(param_2 + 0x38) != '\x03')) {
    uVar1 = 0;
    if (*(char *)(param_2 + 0x10) == '\0') {
LAB_ram_f003452e:
      *(undefined1 *)(param_2 + 0x5d) = uVar1;
      return;
    }
    uVar2 = *puVar4 & 0xdfff;
  }
  else {
    if (*(char *)(param_2 + 0x10) == '\0') {
      uVar1 = 1;
      goto LAB_ram_f003452e;
    }
    uVar2 = *puVar4 | 0x2000;
  }
  *puVar4 = uVar2;
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x2163,0x2166 <<<
// ===== FUN_ram_f0034540 @ 0xf0034540 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034540(undefined4 *param_1,undefined4 param_2,undefined1 param_3,undefined4 param_4,
                     undefined1 param_5,undefined4 param_6,undefined2 param_7,undefined4 param_8,
                     undefined1 param_9,undefined1 param_10,undefined1 param_11,undefined1 param_12,
                     undefined4 param_13)

{
  if (param_1[2] == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x2163);
  }
  if ((1 < (byte)(*(char *)(param_1 + 3) - 3U)) && (*(char *)(param_1 + 3) != '\x01')) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x2166);
  }
  *(undefined1 *)(param_1 + 0x16) = 1;
  param_1[8] = param_2;
  *(undefined1 *)((int)param_1 + 0xd) = param_3;
  param_1[5] = param_4;
  *(undefined1 *)((int)param_1 + 0x11) = param_5;
  param_1[6] = param_6;
  *(undefined2 *)((int)param_1 + 0x12) = param_7;
  *(undefined1 *)((int)param_1 + 0x41) = param_9;
  param_1[0xb] = param_8;
  *(undefined1 *)(param_1 + 0x12) = param_10;
  *(undefined1 *)((int)param_1 + 0x49) = param_11;
  *(undefined1 *)((int)param_1 + 0x4a) = param_12;
  *(undefined1 *)(param_1 + 0x13) = 0;
  *param_1 = param_13;
  *(undefined1 *)((int)param_1 + 0xf) = 0;
  *(undefined1 *)(param_1 + 4) = 1;
  *(undefined1 *)(param_1 + 10) = 0;
  *(undefined1 *)((int)param_1 + 0x29) = 0;
  *(undefined1 *)(param_1 + 0x15) = 0;
  *(undefined1 *)((int)param_1 + 0x4b) = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034610 @ 0xf0034610 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034610(char param_1,char param_2)

{
  if (param_1 == '\0') {
    if (param_2 == '\0') {
      *(undefined1 *)(_DAT_ram_020a0064 + 0x12f5) = 0;
    }
    else if (param_2 == '\x01') {
      *(undefined1 *)(_DAT_ram_020a0064 + 0x1aa1) = 0;
    }
  }
  else if (param_2 == '\0') {
    *(undefined1 *)(_DAT_ram_020a0064 + 0x12f5) = 1;
  }
  else if (param_2 == '\x01') {
    *(undefined1 *)(_DAT_ram_020a0064 + 0x1aa1) = 1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034648 @ 0xf0034648 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034648(undefined2 param_1)

{
  *(undefined2 *)(_DAT_ram_f0064f50 + 0x118) = param_1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034658 @ 0xf0034658 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0034658(void)

{
  bool bVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  
  iVar5 = 0;
  do {
    iVar2 = FUN_ram_f0025d90(iVar5);
    if (*(char *)(iVar2 + 9) != '\0') {
      piVar3 = (int *)(iVar2 + 0xa4);
      piVar4 = piVar3;
      do {
        if ((int *)*piVar4 != piVar3) goto LAB_ram_f0034718;
        piVar3 = piVar3 + 3;
        piVar4 = piVar4 + 3;
      } while (piVar3 != (int *)(iVar2 + 0xe0));
    }
    iVar5 = iVar5 + 1;
  } while (iVar5 != 0x14);
  if (((((*(int *)(_DAT_ram_f0064f50 + 0xc) == _DAT_ram_f0064f50 + 0xc) &&
        (*(int *)(_DAT_ram_f0064f50 + 0x18) == _DAT_ram_f0064f50 + 0x18)) &&
       (*(int *)(_DAT_ram_f0064f50 + 0x24) == _DAT_ram_f0064f50 + 0x24)) &&
      ((*(int *)(_DAT_ram_f0064f50 + 0x30) == _DAT_ram_f0064f50 + 0x30 &&
       ((*(char *)(_DAT_ram_020a0068 + 0x18) == '\0' ||
        (*(int *)(_DAT_ram_020a0068 + 0x350) == _DAT_ram_020a0068 + 0x350)))))) &&
     ((*(char *)(_DAT_ram_020a006c + 0x18) == '\0' ||
      (*(int *)(_DAT_ram_020a006c + 0x350) == _DAT_ram_020a006c + 0x350)))) {
    if (*(char *)(_DAT_ram_020a0070 + 0x18) == '\0') {
      bVar1 = true;
    }
    else {
      bVar1 = *(int *)(_DAT_ram_020a0070 + 0x350) == _DAT_ram_020a0070 + 0x350;
    }
  }
  else {
LAB_ram_f0034718:
    bVar1 = false;
  }
  return bVar1;
}

// >>> MOD: wifi/mgmt/txm.c:0x207b <<<
// ===== FUN_ram_f0034724 @ 0xf0034724 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034724(int param_1)

{
  undefined1 uVar1;
  undefined1 auStack_40 [12];
  undefined1 auStack_34 [9];
  undefined1 local_2b;
  undefined4 local_28;
  undefined2 local_24;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x207b,_DAT_ram_00014800);
  }
  uVar1 = *(undefined1 *)(param_1 + 0x28a);
  (*(code *)&SUB_ram_0006a0a0)(auStack_34,0,10);
  local_28 = 0;
  local_24 = 0;
  (*(code *)&SUB_ram_0006a0a0)(auStack_40,0,10);
  (*_DAT_ram_00014e54)(uVar1,auStack_34,&local_28,auStack_40,_DAT_ram_00014e54);
  local_2b = *(char *)(param_1 + 0x37) != '\0';
  (*_DAT_ram_00014e4c)(uVar1,auStack_34,&local_28,auStack_40,_DAT_ram_00014e4c);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00347c4 @ 0xf00347c4 =====


void FUN_ram_f00347c4(int param_1,int param_2,undefined4 param_3)

{
  if ((param_2 != 0) && (param_1 == 1)) {
    FUN_ram_f0034724(param_3);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00347e8 @ 0xf00347e8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00347e8(int param_1)

{
  undefined2 uVar1;
  undefined1 uVar2;
  undefined1 auStack_40 [12];
  undefined1 auStack_34 [6];
  undefined1 local_2e;
  undefined1 local_2b;
  undefined4 local_28;
  ushort local_24;
  
  uVar2 = *(undefined1 *)(param_1 + 0x28a);
  (*(code *)&SUB_ram_0006a0a0)(auStack_34,0,10);
  local_28 = 0;
  local_24 = 0;
  (*(code *)&SUB_ram_0006a0a0)(auStack_40,0,10);
  (*_DAT_ram_00014e54)(uVar2,auStack_34,&local_28,auStack_40,_DAT_ram_00014e54);
  *(undefined1 *)(param_1 + 699) = *(undefined1 *)(param_1 + 0x35);
  uVar1 = *(undefined2 *)(_DAT_ram_f0064f50 + 0x118);
  *(undefined2 *)(param_1 + 0x2cc) = 0;
  *(undefined2 *)(param_1 + 700) = uVar1;
  *(undefined4 *)(param_1 + 0x2c4) = 0;
  *(undefined4 *)(param_1 + 0x2c8) = 0;
  *(undefined2 *)(param_1 + 0x2ce) = 0;
  local_2e = (*(byte *)(param_1 + 0x17) & 0x10) != 0;
  if ((bool)local_2e) {
    local_24 = CONCAT11(*(byte *)(param_1 + 0x2a),(char)((*(byte *)(param_1 + 0x2a) & 0x1c) >> 2)) &
               0x3ff;
  }
  local_2b = 0;
  (*_DAT_ram_00014e4c)(uVar2,auStack_34,&local_28,auStack_40,_DAT_ram_00014e4c);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00348a0 @ 0xf00348a0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00348a0(int param_1)

{
  uint uVar1;
  bool bVar2;
  uint uVar3;
  uint uVar4;
  uint *puVar5;
  
  if (*(char *)(_DAT_ram_f0064f50 + 0x142) != '\0') {
    if (*(char *)(param_1 + 0x10) == '\x01') {
      bVar2 = (bool)((*(byte *)(*(int *)(param_1 + 0x14) + 4) ^ 1) & 1);
    }
    else {
      bVar2 = *(int *)(param_1 + 0x20) != 0;
    }
    if (bVar2 == false) {
      puVar5 = (uint *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0x368);
    }
    else {
      puVar5 = (uint *)(*(int *)(param_1 + 0x20) + 0x358);
    }
    (*(code *)&SUB_ram_0006a08c)(*(undefined4 *)(param_1 + 8),puVar5,6);
    uVar1 = *puVar5;
    uVar3 = puVar5[1];
    uVar4 = uVar1 + 1;
    *puVar5 = uVar4;
    puVar5[1] = uVar3 + (uVar4 == 0);
    if (*(char *)(param_1 + 0x43) != '\x01') {
      uVar1 = uVar1 + 0x11;
      *puVar5 = uVar1;
      puVar5[1] = uVar3 + (uVar1 < 0x11);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034a68 @ 0xf0034a68 =====


void FUN_ram_f0034a68(int param_1,short param_2)

{
  if ((param_1 != 0) && (*(char *)(param_1 + 9) != '\0')) {
    if (param_2 != 0) {
      FUN_ram_f00297b0(0,param_1);
    }
    *(short *)(param_1 + 700) = param_2;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034a98 @ 0xf0034a98 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0034a98(int param_1)

{
  char cVar1;
  int iVar2;
  
  if (*(char *)(param_1 + 0xd) == '\0') {
    cVar1 = *(char *)(_DAT_ram_020a0064 + 0x12f5);
  }
  else {
    if (*(char *)(param_1 + 0xd) != '\x01') {
      return false;
    }
    cVar1 = *(char *)(_DAT_ram_020a0064 + 0x1aa1);
  }
  if (cVar1 == '\0') {
    return false;
  }
  iVar2 = FUN_ram_f002a4cc();
  return iVar2 != 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034ad8 @ 0xf0034ad8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034ad8(int param_1)

{
  char cVar1;
  int iVar2;
  bool bVar3;
  byte bVar4;
  char cVar5;
  ushort uVar6;
  ushort uVar7;
  
  cVar1 = *(char *)(param_1 + 0x10);
  *(undefined1 *)(param_1 + 0x44) = 0;
  *(undefined2 *)(param_1 + 0x46) = 0xffff;
  *(undefined1 *)(param_1 + 0x43) = 1;
  iVar2 = *(int *)(param_1 + 0x20);
  if (cVar1 == '\x01') {
    bVar3 = (bool)((*(byte *)(*(int *)(param_1 + 0x14) + 4) ^ 1) & 1);
  }
  else {
    bVar3 = iVar2 != 0;
  }
  if (bVar3 != false) {
    if (cVar1 == '\x01') {
      bVar4 = **(byte **)(param_1 + 0x14) & 0xc ^ 8;
    }
    else {
      bVar4 = *(byte *)(param_1 + 0x5b);
    }
    if (bVar4 == 0) {
      if (iVar2 == 0) {
        uVar6 = *(ushort *)(_DAT_ram_f0064f50 + 0x118);
      }
      else {
        uVar6 = *(ushort *)(iVar2 + 700);
      }
      if (uVar6 != 0) {
        if (uVar6 < 0x100) {
          uVar6 = 0x100;
        }
        if (cVar1 == '\x01') {
          bVar3 = (bool)((*(byte *)(*(int *)(param_1 + 0x14) + 4) ^ 1) & 1);
        }
        else {
          bVar3 = iVar2 != 0;
        }
        if (bVar3 != false) {
          if (cVar1 == '\x01') {
            bVar3 = (**(byte **)(param_1 + 0x14) & 0xc) == 8;
          }
          else {
            bVar3 = *(char *)(param_1 + 0x5b) == '\0';
          }
          if (bVar3) {
            uVar7 = *(ushort *)(param_1 + 0x12);
            iVar2 = FUN_ram_f0034a98(param_1);
            if ((iVar2 != 0) && ((*(byte *)(param_1 + 0x24) & 0x10) == 0)) {
              uVar7 = uVar7 + 8;
            }
            cVar1 = '\x01';
            do {
              cVar5 = cVar1 + '\x01';
              if (uVar7 <= uVar6) break;
              uVar7 = uVar7 - uVar6;
              cVar1 = cVar5;
            } while (cVar5 != '\x10');
            *(ushort *)(param_1 + 0x46) = uVar6;
            *(char *)(param_1 + 0x43) = cVar1;
          }
        }
      }
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x21c7 <<<
// ===== FUN_ram_f0034bd8 @ 0xf0034bd8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0034bd8(int param_1)

{
  ushort uVar1;
  char cVar2;
  bool bVar3;
  int iVar4;
  ushort *puVar5;
  ushort *puVar6;
  byte bVar7;
  ushort *puVar8;
  
  bVar7 = *(byte *)(param_1 + 0x24);
  if (((bVar7 & 0x10) != 0) || (cVar2 = *(char *)(param_1 + 0x10), cVar2 == '\0')) {
    return 0;
  }
  if (cVar2 == '\x01') {
    bVar3 = (bool)((*(byte *)(*(int *)(param_1 + 0x14) + 4) ^ 1) & 1);
  }
  else {
    bVar3 = (bVar7 & 0x10) < *(uint *)(param_1 + 0x20);
  }
  if (bVar3 == false) {
    return 0;
  }
  if (cVar2 == '\x01') {
    bVar3 = (**(byte **)(param_1 + 0x14) & 0xc) == 8;
  }
  else {
    bVar3 = *(char *)(param_1 + 0x5b) == '\0';
  }
  if (!bVar3) {
    return 0;
  }
  if ((cVar2 == '\x01') && ((**(byte **)(param_1 + 0x14) & 0xc) == 4)) {
    return 0;
  }
  if (*(byte *)(param_1 + 0x43) < 2) {
    return 0;
  }
  if ((*(char *)(param_1 + 0xd) == '\0') && (*(char *)(_DAT_ram_020a0064 + 0x12f5) != '\0')) {
LAB_ram_f0034c78:
    iVar4 = _DAT_ram_020a0064 + 0x12e4;
  }
  else {
    iVar4 = *(int *)(param_1 + 0x20);
    if (iVar4 == 0) {
      return 0;
    }
    if (*(char *)(iVar4 + 0x14) != '\x01') {
      return 0;
    }
    if (*(char *)(_DAT_ram_020a0064 + 0x1aa1) == '\0') {
      return 0;
    }
    if (*(char *)(param_1 + 0xd) == '\0') goto LAB_ram_f0034c78;
    iVar4 = iVar4 + 0x345;
  }
  if (iVar4 == 0) {
    *(byte *)(param_1 + 0x24) = bVar7 | 0x10;
    return 0;
  }
  puVar8 = *(ushort **)(param_1 + 0x14);
  uVar1 = *puVar8;
  if ((uVar1 & 0x300) == 0x100) {
    puVar5 = puVar8 + 8;
  }
  else {
    if ((uVar1 & 0x300) == 0x200) {
      puVar5 = puVar8 + 2;
      puVar6 = puVar8 + 8;
      goto LAB_ram_f0034cee;
    }
    if ((uVar1 & 0x300) != 0) {
      if ((uVar1 & 0x100) != 0) {
        (*_DAT_ram_00014800)(0,0,0xf006380c,0x21c7,_DAT_ram_00014800);
      }
      *(byte *)(param_1 + 0x24) = *(byte *)(param_1 + 0x24) | 0x10;
      return 1;
    }
    puVar5 = puVar8 + 2;
  }
  puVar6 = puVar8 + 5;
LAB_ram_f0034cee:
  if ((uVar1 & 0xfc) == 0x88) {
    bVar7 = (byte)puVar8[0xc] & 0xf;
  }
  else {
    bVar7 = 0;
  }
  FUN_ram_f0033de8(puVar5,puVar6,bVar7,(uint)*(ushort *)(param_1 + 0x12),*(int *)(param_1 + 0x18),
                   *(int *)(param_1 + 0x18) + (uint)*(ushort *)(param_1 + 0x12),iVar4);
  *(short *)(param_1 + 0x12) = *(short *)(param_1 + 0x12) + 8;
  *(byte *)(param_1 + 0x24) = *(byte *)(param_1 + 0x24) | 0x10;
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034d38 @ 0xf0034d38 =====


undefined4
FUN_ram_f0034d38(undefined1 param_1,undefined1 param_2,undefined1 param_3,undefined1 param_4)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined2 *puVar4;
  
  uVar1 = FUN_ram_f0025b3c();
  if ((uVar1 < 0x10) || (iVar2 = FUN_ram_f0025c48(0xc), iVar2 == 0)) {
    uVar3 = 0xc000009a;
  }
  else {
    puVar4 = *(undefined2 **)(iVar2 + 8);
    *puVar4 = 0xc;
    *(undefined1 *)(puVar4 + 2) = 0x1e;
    *(undefined1 *)(puVar4 + 4) = param_1;
    *(undefined1 *)((int)puVar4 + 9) = param_2;
    *(undefined1 *)(puVar4 + 5) = param_3;
    *(undefined1 *)((int)puVar4 + 0xb) = param_4;
    *(undefined1 *)((int)puVar4 + 5) = 0;
    *(undefined1 *)(iVar2 + 0x2f) = 0;
    *(undefined2 **)(iVar2 + 0x28) = puVar4;
    puVar4[1] = 1;
    FUN_ram_f0039750();
    uVar3 = 0;
  }
  return uVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034dac @ 0xf0034dac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0034dac(uint param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = 0;
  do {
    iVar1 = FUN_ram_f0025d90(iVar3);
    iVar3 = iVar3 + 1;
    if ((((*(char *)(iVar1 + 9) != '\0') && (*(char *)(iVar1 + 0x2ba) != '\0')) &&
        (*(byte *)(iVar1 + 0x14) == param_1)) &&
       ((*(char *)(iVar1 + 0x15) == '\x02' &&
        (iVar2 = FUN_ram_f0034d38(*(undefined1 *)(iVar1 + 8),1,
                                  *(char *)(_DAT_ram_f0064f50 + 0x140) - *(char *)(iVar1 + 0x2b8),
                                  *(undefined1 *)(iVar1 + 0x2b9)), iVar2 == 0)))) {
      *(undefined1 *)(iVar1 + 0x2b9) = 0;
      *(undefined1 *)(iVar1 + 0x2ba) = 0;
    }
  } while (iVar3 != 0x14);
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x1f42 <<<
// ===== FUN_ram_f0034e20 @ 0xf0034e20 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0034e20(int param_1)

{
  byte bVar1;
  int iVar2;
  
  if (*(char *)(param_1 + 0x2b8) < '\0') {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1f42,_DAT_ram_00014800);
  }
  if (*(char *)(param_1 + 0x38) != '\0') {
    bVar1 = *(byte *)(param_1 + 0x2b8);
    if ((bVar1 < *(byte *)(_DAT_ram_f0064f50 + 0x140)) && (bVar1 < 2)) {
      iVar2 = FUN_ram_f0034d38(*(undefined1 *)(param_1 + 8),1,
                               *(byte *)(_DAT_ram_f0064f50 + 0x140) - bVar1,
                               *(undefined1 *)(param_1 + 0x2b9));
      if (iVar2 == 0) {
        *(undefined1 *)(param_1 + 0x2b9) = 0;
      }
      *(bool *)(param_1 + 0x2ba) = iVar2 != 0;
    }
  }
  return 0;
}

// >>> MOD: wifi/mgmt/txm.c:0x1ec9 <<<
// ===== FUN_ram_f0034e90 @ 0xf0034e90 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0034e90(undefined1 param_1,char param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  undefined2 *puVar5;
  
  uVar1 = FUN_ram_f0025b3c();
  if ((uVar1 < 5) || (iVar2 = FUN_ram_f0025c48(0xc), iVar2 == 0)) {
    uVar4 = 0xc000009a;
  }
  else {
    puVar5 = *(undefined2 **)(iVar2 + 8);
    *puVar5 = 0xc;
    *(undefined1 *)(puVar5 + 2) = 0x1a;
    *(undefined1 *)(puVar5 + 4) = param_1;
    *(undefined1 *)((int)puVar5 + 5) = 0;
    *(char *)((int)puVar5 + 9) = param_2;
    iVar3 = FUN_ram_f0025d90(param_1);
    if (iVar3 == 0) {
      uVar4 = 0xc0010015;
    }
    else {
      *(undefined1 *)((int)puVar5 + 0xb) = 0;
      *(undefined1 *)(puVar5 + 5) = 0;
      if (*(char *)(_DAT_ram_f0064f50 + 0x140) == '\0') {
        (*_DAT_ram_00014800)(0,0,0xf006380c,0x1ec9,_DAT_ram_00014800);
      }
      if (param_2 != '\0') {
        if (*(byte *)(iVar3 + 0x2b8) < *(byte *)(_DAT_ram_f0064f50 + 0x140)) {
          *(byte *)((int)puVar5 + 0xb) =
               *(byte *)(_DAT_ram_f0064f50 + 0x140) - *(byte *)(iVar3 + 0x2b8);
        }
      }
      *(undefined1 *)(iVar2 + 0x2f) = 0;
      *(undefined2 **)(iVar2 + 0x28) = puVar5;
      puVar5[1] = 1;
      FUN_ram_f0039750(iVar2);
      uVar4 = 0;
    }
  }
  return uVar4;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0034f64 @ 0xf0034f64 =====


undefined4 FUN_ram_f0034f64(undefined1 param_1,char param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 *puVar3;
  
  iVar1 = FUN_ram_f0025c48(0xc);
  if (iVar1 == 0) {
    uVar2 = 0xc000009a;
  }
  else {
    puVar3 = *(undefined2 **)(iVar1 + 8);
    *puVar3 = 0xc;
    *(undefined1 *)(puVar3 + 2) = 0x19;
    *(undefined1 *)((int)puVar3 + 5) = 0;
    *(undefined1 *)(puVar3 + 4) = param_1;
    *(char *)((int)puVar3 + 9) = param_2;
    if (param_2 == '\0') {
      *(undefined1 *)(puVar3 + 5) = 0;
    }
    else {
      *(undefined1 *)(puVar3 + 5) = 2;
    }
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar3;
    puVar3[1] = 1;
    FUN_ram_f0039750();
    uVar2 = 0;
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00351f4 @ 0xf00351f4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00351f4(int param_1,ushort *param_2)

{
  bool bVar1;
  char cVar2;
  short sVar3;
  ushort uVar4;
  uint uVar5;
  int iVar6;
  ushort *puVar7;
  undefined4 uVar8;
  ushort *puVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  
  iVar10 = *(int *)(param_1 + 8);
  if ((*(char *)(param_1 + 0x10) != '\0') || (*(char *)(_DAT_ram_020a0064 + 0x20ad) != '\0')) {
    *(int *)(param_1 + 0x14) = iVar10 + 0x40;
    uVar5 = (byte)param_2[2] & 0x3f;
    *(char *)(param_1 + 0x11) = (char)uVar5;
    *(uint *)(param_1 + 0x18) = iVar10 + 0x40 + uVar5;
    *(ushort *)(param_1 + 0x12) = ((*param_2 & 0xfff) - 0x10) - (short)uVar5;
    return;
  }
  *(int *)(param_1 + 0x14) = iVar10 + 8;
  if ((*(int *)(param_1 + 0x20) == 0) || (*(char *)(*(int *)(param_1 + 0x20) + 0x2a8) == '\0')) {
    cVar2 = '\x18';
  }
  else {
    cVar2 = '\x1a';
  }
  *(char *)(param_1 + 0x11) = cVar2;
  bVar1 = (*(byte *)((int)param_2 + 5) & 0x30) == 0x20;
  if (bVar1) {
    *(char *)(param_1 + 0x11) = cVar2 + '\x06';
  }
  iVar11 = iVar10 + 0x40;
  puVar9 = *(ushort **)(param_1 + 0x14);
  if ((ushort)(*(ushort *)(iVar10 + 0x4c) << 8 | *(ushort *)(iVar10 + 0x4c) >> 8) < 0x5dd) {
    *(int *)(param_1 + 0x18) = *(int *)(param_1 + 8) + 0x4e;
    iVar12 = 0;
    sVar3 = (*param_2 & 0xfff) - 0x1e;
  }
  else {
    iVar12 = *(int *)(param_1 + 8) + 0x46;
    *(int *)(param_1 + 0x18) = iVar12;
    sVar3 = (*param_2 & 0xfff) - 0x16;
  }
  *(short *)(param_1 + 0x12) = sVar3;
  if ((*(int *)(param_1 + 0x20) == 0) || (*(char *)(*(int *)(param_1 + 0x20) + 0x2a8) == '\0')) {
    uVar4 = 8;
  }
  else {
    uVar4 = 0x88;
  }
  puVar9[1] = 0;
  *puVar9 = uVar4;
  if ((*(int *)(param_1 + 0x20) != 0) && (*(char *)(*(int *)(param_1 + 0x20) + 0x37) != '\0')) {
    *puVar9 = uVar4 | 0x1000;
  }
  iVar6 = FUN_ram_f002a4cc(param_1);
  if (iVar6 != 0) {
    *puVar9 = *puVar9 | 0x4000;
  }
  iVar6 = *(int *)(param_1 + 0x20);
  if (iVar6 == 0) {
    if (*(char *)(param_1 + 0xd) == '\0') {
LAB_ram_f0035406:
      (*(code *)&SUB_ram_0006a08c)(puVar9 + 2,iVar11,6);
      (*(code *)&SUB_ram_0006a08c)(puVar9 + 5,iVar10 + 0x46,6);
      puVar7 = puVar9 + 8;
      iVar6 = _DAT_ram_020a0068 + 0x4b;
    }
    else {
      if (*(char *)(param_1 + 0xd) != '\x01') {
        uVar8 = 0xcb2;
        goto LAB_ram_f0035486;
      }
LAB_ram_f0035438:
      *puVar9 = *puVar9 | 0x200;
      (*(code *)&SUB_ram_0006a08c)(puVar9 + 2,iVar11,6);
      (*(code *)&SUB_ram_0006a08c)(puVar9 + 5,_DAT_ram_020a006c + 0x4b,6);
      puVar7 = puVar9 + 8;
      iVar6 = iVar10 + 0x46;
    }
  }
  else {
    uVar5 = *(uint *)(iVar6 + 0x10);
    if (uVar5 != 0x24) {
      if (uVar5 < 0x25) {
        if (uVar5 == 0x11) goto LAB_ram_f0035406;
        if ((0x10 < uVar5) && (uVar5 - 0x21 < 2)) goto LAB_ram_f0035438;
      }
      else {
        if (uVar5 == 0x44) goto LAB_ram_f0035382;
        if (uVar5 < 0x45) {
          if (uVar5 - 0x41 < 2) {
            *puVar9 = *puVar9 | 0x100;
            (*(code *)&SUB_ram_0006a08c)
                      (puVar9 + 2,
                       *(int *)(&DAT_ram_020a0068 +
                               (uint)*(byte *)(*(int *)(param_1 + 0x20) + 0x14) * 4) + 0x4b,6);
            (*(code *)&SUB_ram_0006a08c)(puVar9 + 5,iVar10 + 0x46,6);
            puVar7 = puVar9 + 8;
            iVar6 = iVar11;
            goto LAB_ram_f0035432;
          }
        }
        else if (uVar5 == 0x81) {
          FUN_ram_f003ef48(iVar6,puVar9,iVar11);
          goto LAB_ram_f0035488;
        }
      }
      (*_DAT_ram_00014814)(2,0xf006381c,uVar5,_DAT_ram_00014814);
      uVar8 = 0xc97;
LAB_ram_f0035486:
      (*_DAT_ram_00014800)(0,0,0xf006380c,uVar8,_DAT_ram_00014800);
      goto LAB_ram_f0035488;
    }
LAB_ram_f0035382:
    *puVar9 = *puVar9 | 0x300;
    iVar6 = iVar10 + 0x46;
    (*(code *)&SUB_ram_0006a08c)(puVar9 + 2,iVar11,6);
    (*(code *)&SUB_ram_0006a08c)(puVar9 + 5,iVar6,6);
    (*(code *)&SUB_ram_0006a08c)(puVar9 + 8,iVar11,6);
    puVar7 = puVar9 + 0xc;
  }
LAB_ram_f0035432:
  (*(code *)&SUB_ram_0006a08c)(puVar7,iVar6,6);
LAB_ram_f0035488:
  if ((*(int *)(param_1 + 0x20) != 0) && (*(char *)(*(int *)(param_1 + 0x20) + 0x2a8) != '\0')) {
    if (bVar1) {
      puVar9[0xf] = 0;
      puVar9[0xf] = *(byte *)(param_1 + 0xf) & 0xf;
    }
    else {
      puVar9[0xc] = (ushort)bVar1;
      puVar9[0xc] = *(byte *)(param_1 + 0xf) & 0xf;
    }
  }
  if (iVar12 != 0) {
    uVar4 = *(ushort *)(iVar10 + 0x4c) << 8 | *(ushort *)(iVar10 + 0x4c) >> 8;
    if ((uVar4 == 0x80f3) || (uVar4 == 0x8137)) {
      uVar8 = 0xf0064948;
    }
    else {
      uVar8 = 0xf0064950;
    }
    (*(code *)&SUB_ram_0006a08c)(iVar12,uVar8,6);
  }
  *(undefined1 *)(param_1 + 0x10) = 1;
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x3f3 <<<
// ===== FUN_ram_f00355c4 @ 0xf00355c4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00355c4(int param_1,uint param_2)

{
  int iVar1;
  code *pcVar2;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x3f3,_DAT_ram_00014800);
  }
  FUN_ram_f003f3d0(param_1,param_2 & 0xff);
  pcVar2 = *(code **)(param_1 + 0x2c);
  if (pcVar2 != (code *)0x0) {
    *(undefined4 *)(param_1 + 0x2c) = 0;
    iVar1 = (*pcVar2)(param_1,param_2);
    if (iVar1 == 1) {
      return;
    }
  }
  (*(code *)&SUB_ram_0006a0d8)(param_1);
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x20d2,0x20de <<<
// ===== FUN_ram_f0035620 @ 0xf0035620 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0035620(int param_1,uint param_2)

{
  bool bVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  int *piVar9;
  byte bVar10;
  int iVar11;
  int *piVar12;
  
  if (*(char *)(param_1 + 0x38) == '\0') {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x20d2,_DAT_ram_00014800);
  }
  iVar11 = 0;
  piVar12 = (int *)(param_1 + 0xac);
  bVar10 = 0;
  do {
    piVar8 = (int *)piVar12[-2];
    piVar7 = (int *)(param_1 + iVar11 * 0xc + 0xa4);
    if (piVar7 != piVar8) {
      piVar9 = (int *)*piVar8;
      while( true ) {
        if (piVar8[8] == 0) {
          (*_DAT_ram_00014800)(0,0,0xf006380c,0x20de,_DAT_ram_00014800);
        }
        uVar2 = piVar8[7];
        uVar3 = ~uVar2;
        if (uVar2 <= param_2) {
          uVar3 = -uVar2;
        }
        if (param_2 + uVar3 < 0x50000) break;
        piVar6 = (int *)piVar8[1];
        iVar4 = *piVar8;
        *(int **)(iVar4 + 4) = piVar6;
        *piVar6 = iVar4;
        *piVar8 = 0;
        piVar8[1] = 0;
        bVar10 = bVar10 + 1;
        *piVar12 = *piVar12 + -1;
        FUN_ram_f00355c4(piVar8,3);
        iVar4 = _DAT_ram_f0064f50;
        piVar8 = (int *)(_DAT_ram_f0064f50 + 0xc4);
        iVar5 = *(int *)(_DAT_ram_f0064f50 + 0xc0) + 1;
        *(int *)(_DAT_ram_f0064f50 + 0xc0) = iVar5;
        *(uint *)(iVar4 + 0xc4) = *piVar8 + (uint)(iVar5 == 0);
        if ((2 < bVar10) ||
           (bVar1 = piVar7 == piVar9, piVar8 = piVar9, piVar9 = (int *)*piVar9, bVar1)) break;
      }
    }
    iVar11 = iVar11 + 1;
    piVar12 = piVar12 + 3;
    if (iVar11 == 5) {
      return;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003570c @ 0xf003570c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003570c(uint param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  
  uVar1 = (*_DAT_ram_00014830)(param_1,_DAT_ram_00014830);
  iVar3 = 0;
  do {
    iVar2 = FUN_ram_f0025d90(iVar3);
    iVar3 = iVar3 + 1;
    if (((*(char *)(iVar2 + 9) != '\0') && (*(char *)(iVar2 + 0x38) != '\0')) &&
       (*(byte *)(iVar2 + 0x14) == param_1)) {
      FUN_ram_f0035620(iVar2,uVar1);
    }
  } while (iVar3 != 0x14);
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x188b <<<
// ===== FUN_ram_f0035760 @ 0xf0035760 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0035760(int param_1)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  undefined4 *puVar6;
  int *piVar7;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x188b,_DAT_ram_00014800);
  }
  iVar4 = 0;
  puVar6 = (undefined4 *)(param_1 + 0xa4);
  piVar5 = (int *)(param_1 + 0xac);
  do {
    piVar7 = (int *)(param_1 + iVar4 * 0xc + 0xa4);
    while (piVar1 = (int *)*puVar6, piVar1 != piVar7) {
      iVar3 = *piVar1;
      piVar2 = (int *)piVar1[1];
      *(int **)(iVar3 + 4) = piVar2;
      *piVar2 = iVar3;
      *piVar1 = 0;
      piVar1[1] = 0;
      *piVar5 = *piVar5 + -1;
      *piVar1 = 0;
      FUN_ram_f00355c4(piVar1,0x41);
    }
    iVar4 = iVar4 + 1;
    puVar6[1] = piVar7;
    *puVar6 = piVar7;
    *piVar5 = 0;
    piVar5 = piVar5 + 3;
    puVar6 = puVar6 + 3;
  } while (iVar4 != 5);
  *(undefined1 *)(param_1 + 0x2c3) = 0;
  *(undefined1 *)(param_1 + 0x2b8) = 0;
  *(undefined1 *)(param_1 + 0x2ba) = 0;
  *(undefined1 *)(param_1 + 0x2be) = 0;
  *(undefined1 *)(param_1 + 0x2bf) = 0;
  *(undefined1 *)(param_1 + 0x2c0) = 0;
  *(undefined1 *)(param_1 + 0x2c1) = 0;
  *(undefined1 *)(param_1 + 0x2c2) = 0;
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x1114,0x111d,0x1147 <<<
// ===== FUN_ram_f0035804 @ 0xf0035804 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0035804(uint *param_1)

{
  bool bVar1;
  uint *puVar2;
  uint *puVar3;
  byte bVar4;
  uint *puVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint *puStack_44;
  uint uStack_40;
  uint uStack_3c;
  int iStack_38;
  uint uStack_28;
  undefined2 local_24;
  undefined2 local_22 [3];
  
  DAT_ram_f0064fbd = DAT_ram_f0064fbd + '\x01';
  if (param_1 == (uint *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1114,_DAT_ram_00014800);
  }
  uStack_3c = 0;
  puStack_44 = param_1;
  do {
    uVar11 = puStack_44[0xb];
    puVar5 = (uint *)(puStack_44[1] & 0xfffffffc);
    if (uVar11 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf006380c,0x111d,_DAT_ram_00014800);
    }
    _DAT_ram_f0064f64 = _DAT_ram_f0064f64 + 1;
    _DAT_ram_f0064f5c = puStack_44;
    iVar6 = *(int *)(uVar11 + 0x20);
    if (iVar6 == 0) {
      iStack_38 = 0;
    }
    else {
      iStack_38 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar6 + 0x14) * 4);
    }
    bVar4 = *(char *)(uVar11 + 0x44) + 1;
    _DAT_ram_f0064f60 = puVar5;
    *(byte *)(uVar11 + 0x44) = bVar4;
    if (*(byte *)(uVar11 + 0x43) < bVar4) {
      (*_DAT_ram_00014800)(0,0,0xf006380c,0x1147,_DAT_ram_00014800);
    }
    *(undefined1 *)(uVar11 + 0x29) = 0;
    *(undefined4 *)(uVar11 + 0x30) = 0;
    *(undefined4 *)(uVar11 + 0x34) = 0;
    *(uint *)(uVar11 + 0x30) = puStack_44[7];
    *(undefined4 *)(uVar11 + 0x34) = 0;
    if (*(char *)(uVar11 + 0x10) == '\x01') {
      bVar1 = (**(byte **)(uVar11 + 0x14) & 0xc) == 8;
    }
    else {
      bVar1 = *(char *)(uVar11 + 0x5b) == '\0';
    }
    if (bVar1) {
      uStack_3c = 1;
    }
    if ((*puStack_44 & 1) == 0) {
      if ((puStack_44[6] & 0x1c) == 0) {
        if (iVar6 != 0) {
          *(char *)(iVar6 + 699) =
               (char)((int)((uint)*(byte *)((int)puStack_44 + 0x19) +
                           (uint)*(byte *)(iVar6 + 699) * 3) >> 2);
        }
        puVar3 = _DAT_ram_f0064f50;
        uVar9 = _DAT_ram_f0064f50[0x14];
        puVar2 = _DAT_ram_f0064f50 + 0x15;
        _DAT_ram_f0064f50[0x14] = uVar9 + 1;
        puVar3[0x15] = *puVar2 + (uint)(uVar9 + 1 == 0);
        puVar2 = _DAT_ram_f0064f50;
        if ((iVar6 != 0) && ((uStack_3c & *(char *)(iVar6 + 0x14) == '\0') != 0)) {
          uVar9 = _DAT_ram_f0064f50[0x3a];
          puVar3 = _DAT_ram_f0064f50 + 0x3b;
          _DAT_ram_f0064f50[0x3a] = uVar9 + 1;
          puVar2[0x3b] = *puVar3 + (uint)(uVar9 + 1 == 0);
        }
        puVar2 = _DAT_ram_f0064f50;
        uVar9 = ((ushort)puStack_44[9] & 0x1f80) >> 7;
        if (1 < uVar9) {
          if (uVar9 == 2) {
            uVar10 = _DAT_ram_f0064f50[0x22];
            puVar3 = _DAT_ram_f0064f50 + 0x23;
            _DAT_ram_f0064f50[0x22] = uVar10 + 1;
            puVar2[0x23] = *puVar3 + (uint)(uVar10 + 1 == 0);
          }
          else {
            uVar10 = _DAT_ram_f0064f50[0x24];
            puVar3 = _DAT_ram_f0064f50 + 0x25;
            _DAT_ram_f0064f50[0x24] = uVar10 + 1;
            puVar2[0x25] = *puVar3 + (uint)(uVar10 + 1 == 0);
          }
        }
        if (*(char *)(uVar11 + 0x10) == '\x01') {
          if (((**(byte **)(uVar11 + 0x14) & 0xfc) == 0x88) ||
             ((**(byte **)(uVar11 + 0x14) & 0xc) == 8)) {
LAB_ram_f0035bbe:
            FUN_ram_f0041768(iVar6,uVar11,uVar9,*(undefined2 *)((int)puStack_44 + 0xe),
                             (ushort)puStack_44[9] >> 0xd,*(undefined2 *)((int)puStack_44 + 0x26));
          }
        }
        else if (((*(int *)(uVar11 + 0x20) != 0) &&
                 (*(char *)(*(int *)(uVar11 + 0x20) + 0x2a8) != '\0')) ||
                (*(char *)(uVar11 + 0x5b) == '\0')) goto LAB_ram_f0035bbe;
        FUN_ram_f004052c(iVar6,uVar11,1,(ushort)puStack_44[9] >> 0xd,uVar9,
                         *(undefined2 *)((int)puStack_44 + 0x26));
        puVar3 = _DAT_ram_f0064f50;
        uStack_40 = 0;
        uVar9 = _DAT_ram_f0064f50[0x1a];
        puVar2 = _DAT_ram_f0064f50 + 0x1b;
        _DAT_ram_f0064f50[0x1a] = uVar9 + 1;
        puVar3[0x1b] = *puVar2 + (uint)(uVar9 + 1 == 0);
      }
      else {
        *(undefined1 *)(uVar11 + 0x29) = 1;
        uVar9 = puStack_44[6];
        if ((uVar9 & 0x10) == 0) {
          if ((uVar9 & 4) == 0) {
            uStack_40 = uStack_3c;
            if ((uVar9 & 8) != 0) {
              uStack_40 = 5;
            }
            if ((uVar9 & 8) == 0) {
              uStack_40 = 0xc0000001;
            }
          }
          else {
            uStack_40 = 4;
          }
        }
        else {
          uStack_40 = 6;
        }
        if (((iVar6 != 0) && (*(char *)(iVar6 + 0x361) == '\0')) &&
           (_DAT_ram_f006e854 = _DAT_ram_f006e854 + 1, uStack_40 < 7)) {
          iVar7 = (uStack_40 + 3) * 4;
          *(int *)(&DAT_ram_f006e848 + iVar7) = *(int *)(&DAT_ram_f006e848 + iVar7) + 1;
        }
        puVar3 = _DAT_ram_f0064f50;
        uVar9 = _DAT_ram_f0064f50[0x26];
        puVar2 = _DAT_ram_f0064f50 + 0x27;
        _DAT_ram_f0064f50[0x26] = uVar9 + 1;
        puVar3[0x27] = *puVar2 + (uint)(uVar9 + 1 == 0);
        puVar2 = _DAT_ram_f0064f50;
        if (((iVar6 != 0) && (*(char *)(iVar6 + 0x14) == '\0')) && (uStack_3c != 0)) {
          uVar9 = _DAT_ram_f0064f50[0x3c];
          puVar3 = _DAT_ram_f0064f50 + 0x3d;
          _DAT_ram_f0064f50[0x3c] = uVar9 + 1;
          puVar2[0x3d] = *puVar3 + (uint)(uVar9 + 1 == 0);
        }
        puVar2 = _DAT_ram_f0064f50;
        uVar9 = puStack_44[6];
        if ((uVar9 & 4) == 0) {
          if ((uVar9 & 8) != 0) {
            uVar9 = _DAT_ram_f0064f50[0x2e];
            uVar10 = _DAT_ram_f0064f50[0x2f];
            goto LAB_ram_f0035a70;
          }
          if ((uVar9 & 0x10) != 0) {
            uVar9 = _DAT_ram_f0064f50[0x28];
            puVar3 = _DAT_ram_f0064f50 + 0x29;
            _DAT_ram_f0064f50[0x28] = uVar9 + 1;
            puVar2[0x29] = *puVar3 + (uint)(uVar9 + 1 == 0);
          }
        }
        else {
          uVar9 = _DAT_ram_f0064f50[0x2c];
          puVar3 = _DAT_ram_f0064f50 + 0x2d;
          _DAT_ram_f0064f50[0x2c] = uVar9 + 1;
          puVar2[0x2d] = *puVar3 + (uint)(uVar9 + 1 == 0);
          uVar9 = _DAT_ram_f0064f50[0x2e];
          uVar10 = _DAT_ram_f0064f50[0x2f];
LAB_ram_f0035a70:
          puVar2 = _DAT_ram_f0064f50;
          _DAT_ram_f0064f50[0x2e] = uVar9 + 1;
          puVar2[0x2f] = uVar10 + (uVar9 + 1 == 0);
        }
        puVar3 = _DAT_ram_f0064f50;
        uVar9 = _DAT_ram_f0064f50[0x1e];
        puVar2 = _DAT_ram_f0064f50 + 0x1f;
        _DAT_ram_f0064f50[0x1e] = uVar9 + 1;
        puVar3[0x1f] = *puVar2 + (uint)(uVar9 + 1 == 0);
      }
    }
    else {
      *(undefined1 *)(uVar11 + 0x29) = 1;
      if ((iVar6 == 0) || (*(char *)(iVar6 + 0x361) != '\0')) {
        uStack_40 = 0x40;
      }
      else {
        _DAT_ram_f006e854 = _DAT_ram_f006e854 + 1;
        _DAT_ram_f006e85c = _DAT_ram_f006e85c + 1;
        uStack_40 = 0x40;
      }
    }
    if ((**(byte **)(uVar11 + 0x14) & 0xfc) == 0xb0) {
      if (_DAT_ram_f0064f8c < 100) {
        iVar7 = _DAT_ram_f0064f8c * 5;
        iVar8 = _DAT_ram_f0064f8c * 10;
        *(undefined1 *)(iVar8 + -0xff91358) = 1;
        *(undefined1 *)(iVar8 + -0xff91357) = 3;
        uStack_28 = uStack_40;
        local_22[0] = 0;
        local_24 = 1;
        (*(code *)&SUB_ram_0006a08c)((iVar7 + 1) * 2 + -0xff91358,&local_24,2);
        (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91354,&uStack_28);
        (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91350,local_22,2);
        _DAT_ram_f0064f8c = _DAT_ram_f0064f8c + 1;
        if (_DAT_ram_f0064f8c < 100) goto LAB_ram_f0035cd6;
      }
      _DAT_ram_f0064f8c = 0;
    }
LAB_ram_f0035cd6:
    if ((**(byte **)(uVar11 + 0x14) & 0xfc) == 0x40) {
      if (_DAT_ram_f0064f8c < 100) {
        iVar7 = _DAT_ram_f0064f8c * 5;
        iVar8 = _DAT_ram_f0064f8c * 10;
        *(undefined1 *)(iVar8 + -0xff91358) = 1;
        *(undefined1 *)(iVar8 + -0xff91357) = 3;
        local_22[0] = 2;
        uStack_28 = uStack_40;
        local_24 = 0;
        (*(code *)&SUB_ram_0006a08c)((iVar7 + 1) * 2 + -0xff91358,local_22,2);
        (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91354,&uStack_28);
        (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91350,&local_24,2);
        _DAT_ram_f0064f8c = _DAT_ram_f0064f8c + 1;
        if (_DAT_ram_f0064f8c < 100) goto LAB_ram_f0035d8a;
      }
      _DAT_ram_f0064f8c = 0;
    }
LAB_ram_f0035d8a:
    if (uStack_40 != 0) {
      FUN_ram_f004052c(iVar6,uVar11,2,0,uStack_40,0);
    }
    puVar3 = _DAT_ram_f0064f50;
    uVar9 = _DAT_ram_f0064f50[0x36];
    puVar2 = _DAT_ram_f0064f50 + 0x37;
    _DAT_ram_f0064f50[0x36] = uVar9 + 1;
    puVar3[0x37] = *puVar2 + (uint)(uVar9 + 1 == 0);
    puVar3 = _DAT_ram_f0064f50;
    uVar9 = ((ushort)puStack_44[9] & 0x1f80) >> 7;
    uVar10 = _DAT_ram_f0064f50[0x38];
    puVar2 = _DAT_ram_f0064f50 + 0x39;
    _DAT_ram_f0064f50[0x38] = uVar10 + uVar9;
    puVar3[0x39] = *puVar2 + (uint)(uVar10 + uVar9 < uVar9);
    puVar2 = _DAT_ram_f0064f50;
    if ((uStack_3c & *(char *)(iVar6 + 0x14) == '\0') != 0) {
      uVar10 = _DAT_ram_f0064f50[0x3e];
      puVar3 = _DAT_ram_f0064f50 + 0x3f;
      _DAT_ram_f0064f50[0x3e] = uVar10 + uVar9;
      puVar2[0x3f] = *puVar3 + (uint)(uVar10 + uVar9 < uVar9);
    }
    if (uStack_40 == 0) {
      FUN_ram_f003c9f4(uVar11,puStack_44);
    }
    FUN_ram_f003d4a4(uVar11,puStack_44,uStack_40);
    if (((((*(char *)((int)puStack_44 + 0x1b) < '\0') && (*(char *)(iVar6 + 9) != '\0')) &&
         (*(char *)(iVar6 + 0x28b) != '\0')) &&
        ((*(char *)(iVar6 + 0x15) == '\x02' && (*(short *)(iVar6 + 700) == 0)))) &&
       ((*(char *)(uVar11 + 0x54) != '\0' &&
        ((0xf < *(ushort *)(uVar11 + 0x56) && (*(int *)(iStack_38 + 0x1c) == 0)))))) {
      iVar7 = *(int *)(iStack_38 + 0xc);
      if (iVar7 == 0) {
        if (*(int *)(iStack_38 + 0xa8) == 0) goto LAB_ram_f0035eb6;
      }
      else if (iVar7 == 2) {
        if (*(char *)(iVar6 + 0x38) == '\0') {
LAB_ram_f0035eb6:
          FUN_ram_f0029e24(iVar6,*(undefined1 *)(uVar11 + 0xf));
        }
      }
      else if ((iVar7 == 4) || (iVar7 == 1)) goto LAB_ram_f0035eb6;
    }
    if (*(char *)(uVar11 + 0x44) == *(char *)(uVar11 + 0x43)) {
      if ((uStack_40 == 0) ||
         (((*(uint *)(iVar6 + 0x10) & 0x80) != 0 && (DAT_ram_f0064fbe == '\x01')))) {
        FUN_ram_f00301dc(uVar11);
      }
      if (((*(char *)(iVar6 + 9) != '\0') && (*(char *)(iVar6 + 0x38) != '\0')) &&
         (*(code **)(uVar11 + 0x3c) != (code *)0x0)) {
        (**(code **)(uVar11 + 0x3c))(uVar11,uStack_40);
      }
      if (*(uint *)(iVar6 + 0x2c4) < 0x20000) {
        *(uint *)(iVar6 + 0x2c4) = *(uint *)(iVar6 + 0x2c4) + 1;
        if (uStack_40 == 0) {
          *(uint *)(iVar6 + 0x2c8) =
               *(int *)(iVar6 + 0x2c8) + (uint)*(ushort *)((int)puStack_44 + 0x26);
        }
        else if (uStack_40 == 6) {
          *(short *)(iVar6 + 0x2ce) = *(short *)(iVar6 + 0x2ce) + 1;
        }
        else {
          *(short *)(iVar6 + 0x2cc) = *(short *)(iVar6 + 0x2cc) + 1;
        }
      }
      FUN_ram_f00355c4(uVar11,uStack_40);
      FUN_ram_f003432c(uVar11);
    }
    (*_DAT_ram_00014b30)(puStack_44,_DAT_ram_020a0078);
    puStack_44 = puVar5;
    if (puVar5 == (uint *)0x0) {
      *_DAT_ram_f0064f50 = *_DAT_ram_f0064f50 & 0xfffffffe;
      DAT_ram_f0064fbd = DAT_ram_f0064fbd + -1;
      return;
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/txm.c:0xeca,0xeda,0xedc,0xee4,0x107c <<<
// ===== FUN_ram_f0035fc4 @ 0xf0035fc4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0035fc4(uint param_1)

{
  short sVar1;
  undefined1 uVar2;
  undefined4 uVar3;
  uint uVar4;
  uint uVar5;
  uint extraout_a1;
  byte bVar6;
  bool bVar7;
  byte bVar8;
  ushort uVar9;
  int iVar10;
  byte bVar11;
  ushort uVar12;
  byte unaff_s2;
  uint *puVar13;
  ushort *puVar14;
  uint local_30;
  uint uStack_2c;
  uint uStack_28;
  
  if (8 < *(byte *)(param_1 + 0x40)) {
    uVar3 = FUN_ram_f0038d88(0xf006e0f0,0xf006380c);
    _DAT_ram_f006e130 = 0xe1e;
    _DAT_ram_f006e134 = (*_DAT_ram_00014c14)(uVar3,_DAT_ram_00014c14);
    (*(code *)&SUB_ram_0006a0b0)(0xf006e138,0xf0063834);
    FUN_ram_f0039800();
    (*(code *)&SUB_ram_0006a0a0)(0xf006e0f0,0,0x3c8);
  }
  if (((*(uint *)(_DAT_ram_f0064f50 + 8) >> ((uint)*(byte *)(param_1 + 0x40) * 2 & 0x1f) & 3) == 2)
     && (1 < (*(byte *)(param_1 + 0x40) - 7 & 0xff))) {
    uVar3 = 0x3d;
  }
  else {
    bVar6 = *(byte *)(param_1 + 0x43);
    bVar8 = *(byte *)(param_1 + 0x44);
    uVar4 = (*_DAT_ram_00014b24)(_DAT_ram_020a0078,_DAT_ram_00014b24);
    if ((uint)bVar6 - (uint)bVar8 <= uVar4) {
      if (1 < *(byte *)(param_1 + 0x43)) {
        uVar4 = FUN_ram_f0034bd8(param_1);
      }
      if ((*(byte *)(param_1 + 0x4b) & 0x10) == 0) {
        uVar5 = (*_DAT_ram_00014830)(uVar4,_DAT_ram_00014830);
        uVar4 = *(uint *)(param_1 + 0x1c);
        if (uVar5 < uVar4) {
          uVar4 = ~uVar4;
        }
        else {
          uVar4 = -uVar4;
        }
        uVar4 = ((uVar5 + uVar4) * 1000 + 0x7fff >> 0xf) * 1000 + 0x1ff >> 0xb;
        if (399 < uVar4) {
          uVar3 = 0x3f;
          goto LAB_ram_f0036106;
        }
        uVar12 = (short)&bpam0 - (short)uVar4;
      }
      else {
        uVar12 = (ushort)*(byte *)(param_1 + 0x4c);
      }
      if ((*(char *)(param_1 + 0x54) == '\0') &&
         (FUN_ram_f0028770(param_1), (**(ushort **)(param_1 + 0x14) & 0x4000) != 0)) {
        FUN_ram_f00348a0(param_1);
      }
      uStack_28 = (uint)*(ushort *)(param_1 + 0x46);
      uVar9 = *(ushort *)(param_1 + 0x12);
      if (*(char *)(param_1 + 0x43) != '\x01') {
        uVar9 = uVar9 - *(ushort *)(param_1 + 0x46) * (ushort)*(byte *)(param_1 + 0x44);
      }
      local_30 = (uint)uVar9;
      puVar14 = (ushort *)0x0;
      puVar13 = (uint *)0x0;
      uVar4 = (uint)*(byte *)(param_1 + 0x44);
      uStack_2c = (uint)*(byte *)(param_1 + 0x44);
      do {
        uVar5 = (uint)*(byte *)(param_1 + 0x40);
        if (*(byte *)(param_1 + 0x43) <= uStack_2c) {
          if (uVar5 == 7) {
            if ((*(uint *)(_DAT_ram_f0064f50 + 8) >> 0xe & 3) != 2) {
              (*_DAT_ram_00014800)(0,0,0xf006380c,0x107c,_DAT_ram_00014800);
            }
            (*_DAT_ram_00014b3c)(7,puVar13,_DAT_ram_020a0078,_DAT_ram_00014b3c);
            *(uint *)(_DAT_ram_f0064f50 + 8) = *(uint *)(_DAT_ram_f0064f50 + 8) & 0xffff3fff;
          }
          else if (uVar5 != 8) {
            *(ushort *)(_DAT_ram_f0064f50 + 4) =
                 (ushort)(1 << (uVar5 & 0x1f)) | *(ushort *)(_DAT_ram_f0064f50 + 4);
          }
          FUN_ram_f00342c4(param_1);
          return;
        }
        if (uVar5 == 7) {
          if ((*(uint *)(_DAT_ram_f0064f50 + 8) >> 0xe & 3) != 2) {
            (*_DAT_ram_00014800)(0,0,0xf006380c,0xeca,_DAT_ram_00014800);
            uVar4 = extraout_a1;
          }
          puVar13 = (uint *)(*_DAT_ram_00014b20)(_DAT_ram_020a0078,uVar4,_DAT_ram_00014b20);
LAB_ram_f0036230:
          if (puVar13 == (uint *)0x0) {
LAB_ram_f0036234:
            (*_DAT_ram_00014800)(0,0,0xf006380c,0xee4,_DAT_ram_00014800);
            puVar13 = (uint *)0x0;
          }
        }
        else {
          if (uVar5 != 8) {
            puVar13 = (uint *)(*_DAT_ram_00014b28)
                                        (uVar5,_DAT_ram_020a0078,uVar5,
                                         (uint)*(byte *)(param_1 + 0x43),_DAT_ram_00014b28);
            goto LAB_ram_f0036230;
          }
          if ((*(ushort *)(_DAT_ram_f0064f50 + 10) & 3) != 0) {
            (*_DAT_ram_00014800)(0,0,0xf006380c,0xeda,_DAT_ram_00014800);
          }
          puVar13 = (uint *)(*_DAT_ram_00014b28)(8,_DAT_ram_020a0078,_DAT_ram_00014b28);
          if (puVar13 == (uint *)0x0) {
            (*_DAT_ram_00014800)(0,0,0xf006380c,0xedc,_DAT_ram_00014800);
            goto LAB_ram_f0036234;
          }
        }
        puVar13[0xb] = param_1;
        if (*(char *)(param_1 + 0x10) == '\0') {
          *puVar13 = *puVar13 & 3;
          bVar6 = (byte)puVar13[3] & 0x20;
          *puVar13 = *(uint *)(param_1 + 0x14) & 0xfffffffc | *puVar13;
          *(byte *)(puVar13 + 3) = bVar6;
          if (*(char *)(param_1 + 0x5e) != '\0') {
            *(byte *)(puVar13 + 3) = bVar6 | 1;
          }
          if (*(char *)(param_1 + 0x5d) != '\0') {
            *(byte *)(puVar13 + 3) = (byte)puVar13[3] | 2;
          }
          bVar6 = (byte)puVar13[3] | 0x40;
        }
        else {
          *puVar13 = *puVar13 & 3;
          bVar6 = (byte)puVar13[3] & 0x20;
          uVar4 = *(uint *)(param_1 + 0x14);
          *(byte *)(puVar13 + 3) = bVar6;
          *puVar13 = uVar4 & 0xfffffffc | *puVar13;
          bVar6 = bVar6 | *(byte *)(param_1 + 0x11) >> 1 & 0x1f;
        }
        *(byte *)(puVar13 + 3) = bVar6;
        if (uStack_2c == *(byte *)(param_1 + 0x43) - 1) {
          uStack_28 = local_30;
        }
        else {
          *(ushort *)((int)puVar13 + 0x16) = *(ushort *)((int)puVar13 + 0x16) | 0x2000;
        }
        uVar9 = *(ushort *)((int)puVar13 + 0xe) & 0xf000 | (ushort)uStack_28 & 0xfff;
        puVar13[2] = *(int *)(param_1 + 0x18) + (*(ushort *)(param_1 + 0x12) - local_30);
        *(ushort *)((int)puVar13 + 0xe) = uVar9;
        if (*(char *)(param_1 + 0x42) != '\0') {
          *(ushort *)((int)puVar13 + 0xe) = uVar9 | 0x4000;
        }
        *(ushort *)(puVar13 + 4) = uVar12 & 0x7ff | (ushort)puVar13[4];
        bVar6 = FUN_ram_f00341e8(param_1);
        *(byte *)((int)puVar13 + 0xd) = bVar6 & 0x7f;
        if (*(char *)(param_1 + 0x10) != '\0') {
          puVar14 = *(ushort **)(param_1 + 0x14);
          if ((*puVar14 & 0xfc) == 0xa4) {
            *(ushort *)((int)puVar13 + 0xe) = *(ushort *)((int)puVar13 + 0xe) | 0x1000;
          }
          else if ((*puVar14 & 0xfc) == 0xc4) {
            *(ushort *)((int)puVar13 + 0xe) = *(ushort *)((int)puVar13 + 0xe) | 0x1000;
            *(undefined1 *)(param_1 + 0x41) = 1;
          }
        }
        if (*(char *)(param_1 + 0x10) == '\x01') {
          bVar7 = (bool)((*(byte *)(*(int *)(param_1 + 0x14) + 4) ^ 1) & 1);
        }
        else {
          bVar7 = *(int *)(param_1 + 0x20) != 0;
        }
        if (bVar7 == false) {
          *(ushort *)(puVar13 + 4) = (ushort)puVar13[4] | 0x800;
        }
        else {
          if (*(int *)(param_1 + 0x20) != 0) {
            if (*(char *)(*(int *)(param_1 + 0x20) + 0x94) != '\0') {
              *(byte *)(puVar13 + 3) = (byte)puVar13[3] | 0x20;
            }
            iVar10 = *(int *)(param_1 + 0x20);
            if (((iVar10 != 0) && (*(char *)(iVar10 + 0x361) == '\0')) &&
               (DAT_ram_f006e845 = *(undefined1 *)(iVar10 + 0x94),
               *(char *)(*(int *)(param_1 + 0x20) + 0x94) == '\x01')) {
              DAT_ram_f006e846 = 1;
            }
            uVar9 = *(ushort *)((int)puVar13 + 0x16) & 0xe0ff;
            *(ushort *)((int)puVar13 + 0x16) = uVar9;
            *(ushort *)((int)puVar13 + 0x16) =
                 uVar9 | (*(byte *)(*(int *)(param_1 + 0x20) + 0x344) & 0x1f) << 8;
            if (*(char *)(param_1 + 0xd) == '\x02') {
              if (*(char *)(param_1 + 0x10) == '\x01') {
                if (((**(byte **)(param_1 + 0x14) & 0xc) == 8) ||
                   ((**(byte **)(param_1 + 0x14) & 0xfc) == 0x88)) {
LAB_ram_f0036468:
                  if ((*_DAT_ram_020a0070 == 0) && (*(char *)(param_1 + 0x28) == '\0')) {
                    *(byte *)(puVar13 + 3) = (byte)puVar13[3] | 0x20;
                  }
                }
              }
              else if ((*(char *)(param_1 + 0x5b) == '\0') ||
                      ((*(int *)(param_1 + 0x20) != 0 &&
                       (*(char *)(*(int *)(param_1 + 0x20) + 0x2a8) != '\0'))))
              goto LAB_ram_f0036468;
            }
            if (((puVar14 != (ushort *)0x0) && (*(char *)(param_1 + 0x10) != '\0')) &&
               (iVar10 = FUN_ram_f003e618(param_1,puVar14), iVar10 != 0)) {
              if (*(char *)(*(int *)(param_1 + 0x20) + 0x37) == '\0') {
                uVar9 = *puVar14 & 0xefff;
              }
              else {
                uVar9 = *puVar14 | 0x1000;
              }
              *puVar14 = uVar9;
            }
            if (((*(char *)(*(int *)(param_1 + 0x20) + 0x15) != '\x02') &&
                (*(byte *)((int)puVar13 + 0xd) = *(byte *)((int)puVar13 + 0xd) | 0x80,
                puVar14 != (ushort *)0x0)) && (*(char *)(param_1 + 0x10) != '\0')) {
              *puVar14 = *puVar14 & 0xefff;
            }
          }
          if (*(char *)(param_1 + 0x41) != '\0') {
            *(ushort *)((int)puVar13 + 0xe) = *(ushort *)((int)puVar13 + 0xe) | 0x8000;
          }
          if (((*(int *)(param_1 + 0x20) != 0) &&
              (*(char *)(*(int *)(param_1 + 0x20) + 0x361) == '\0')) &&
             (*(char *)(param_1 + 0x41) == '\x01')) {
            DAT_ram_f006e848 = DAT_ram_f006e848 | 5;
          }
        }
        if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0x345) ==
            '\0') {
          uVar9 = *(ushort *)((int)puVar13 + 0x16) & 0xfffc;
        }
        else {
          uVar9 = *(ushort *)((int)puVar13 + 0x16) & 0xfffc | 1;
        }
        *(ushort *)((int)puVar13 + 0x16) = uVar9;
        if (*(char *)(param_1 + 0x48) != '\0') {
          bVar6 = *(byte *)((int)puVar13 + 0x13);
          *(byte *)((int)puVar13 + 0x13) = bVar6 | 0x80;
          bVar11 = bVar6 & 0xf0 | 0x80;
          uVar2 = *(undefined1 *)(param_1 + 0x49);
          *(byte *)((int)puVar13 + 0x13) = bVar11;
          *(undefined1 *)((int)puVar13 + 0x12) = uVar2;
          bVar8 = *(byte *)(param_1 + 0x4a) & 0xf;
          *(byte *)((int)puVar13 + 0x13) = bVar11 | bVar8;
          bVar8 = bVar6 & 0xc0 | 0x80 | bVar8;
          if ((*(byte *)(param_1 + 0x4b) & 1) == 0) {
            *(byte *)((int)puVar13 + 0x13) = bVar8;
            bVar6 = (*(byte *)(_DAT_ram_f0064f50 + 0x48) & 3) << 4;
          }
          else {
            *(byte *)((int)puVar13 + 0x13) = bVar8;
            if ((*(byte *)(param_1 + 0x4b) & 2) != 0) {
              unaff_s2 = 0;
            }
            bVar6 = unaff_s2;
            if ((*(byte *)(param_1 + 0x4b) & 2) == 0) {
              unaff_s2 = 0x20;
              bVar6 = unaff_s2;
            }
          }
          *(byte *)((int)puVar13 + 0x13) = bVar6 | bVar8;
          if ((*(byte *)(param_1 + 0x4b) & 4) == 0) {
            bVar6 = *(byte *)((int)puVar13 + 0x13);
            if (*(char *)(_DAT_ram_f0064f50 + 0x49) != '\0') goto LAB_ram_f003660a;
            bVar6 = bVar6 & 0xbf;
          }
          else {
            bVar6 = *(byte *)((int)puVar13 + 0x13);
            if ((*(byte *)(param_1 + 0x4b) & 8) == 0) {
              bVar6 = bVar6 & 0xbf;
            }
            else {
LAB_ram_f003660a:
              bVar6 = bVar6 | 0x40;
            }
          }
          *(byte *)((int)puVar13 + 0x13) = bVar6;
        }
        if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0x346) ==
            '\0') {
          uVar9 = (ushort)puVar13[4] & 0x7fff;
        }
        else {
          uVar9 = (ushort)puVar13[4] | 0x8000;
        }
        *(ushort *)(puVar13 + 4) = uVar9;
        if (*(char *)(param_1 + 0x10) == '\x01') {
          bVar7 = (**(byte **)(param_1 + 0x14) & 0xfc) == 0x88;
        }
        else {
          bVar7 = false;
          if (*(int *)(param_1 + 0x20) != 0) {
            bVar7 = *(char *)(*(int *)(param_1 + 0x20) + 0x2a8) != '\0';
          }
        }
        if (bVar7) {
          uVar9 = (ushort)puVar13[4] & 0x8fff;
          *(ushort *)(puVar13 + 4) = uVar9;
          *(ushort *)(puVar13 + 4) = uVar9 | (*(byte *)(param_1 + 0xf) & 7) << 0xc;
        }
        if (*(char *)(param_1 + 0x10) == '\0') {
          if ((*(int *)(param_1 + 0x20) == 0) ||
             (*(char *)(*(int *)(param_1 + 0x20) + 0x2a8) == '\0')) {
            uVar9 = *(ushort *)((int)puVar13 + 0x16) & 0xbf03 | 8;
          }
          else {
            uVar9 = *(ushort *)((int)puVar13 + 0x16) & 0xbf03 | 0x88;
          }
          *(ushort *)((int)puVar13 + 0x16) = uVar9;
          iVar10 = FUN_ram_f002a4cc(param_1);
          if (iVar10 == 0) {
            uVar9 = *(ushort *)((int)puVar13 + 0x16) & 0xbfff;
          }
          else {
            uVar9 = *(ushort *)((int)puVar13 + 0x16) | 0x4000;
          }
        }
        else {
          uVar9 = *(ushort *)((int)puVar13 + 0x16) & 0x3f03;
          *(ushort *)((int)puVar13 + 0x16) = uVar9;
          uVar9 = uVar9 | **(ushort **)(param_1 + 0x14) & 0xc0fc;
        }
        *(ushort *)((int)puVar13 + 0x16) = uVar9;
        *(ushort *)(puVar13 + 5) = (ushort)puVar13[5] & 0xf;
        sVar1 = *(short *)(param_1 + 0x56);
        *(undefined1 *)((int)puVar13 + 0x1b) = 0x80;
        *(ushort *)(puVar13 + 5) = (ushort)uStack_2c & 0xf | sVar1 << 4;
        FUN_ram_f0020c1c(0);
        uVar3 = FUN_ram_f00415d0();
        *(undefined4 *)(param_1 + 100) = uVar3;
        local_30 = local_30 - uStack_28 & 0xffff;
        *puVar13 = *puVar13 | 1;
        uVar4 = uStack_2c;
        uStack_2c = uStack_2c + 1 & 0xff;
      } while( true );
    }
    uVar3 = 0x3e;
  }
LAB_ram_f0036106:
  FUN_ram_f00355c4(param_1,uVar3);
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x1fb5 <<<
// ===== FUN_ram_f0036814 @ 0xf0036814 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0036814(int *param_1)

{
  int iVar1;
  int *piVar2;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1fb5,_DAT_ram_00014800);
  }
  do {
    piVar2 = (int *)*param_1;
    iVar1 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    param_1[7] = iVar1;
    iVar1 = FUN_ram_f00415d0();
    param_1[0x18] = iVar1;
    FUN_ram_f0035fc4(param_1);
    FUN_ram_f0033fc4();
    param_1 = piVar2;
  } while (piVar2 != (int *)0x0);
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x1c94,0x1c9a,0x1cd2,0x1d0f <<<
// ===== FUN_ram_f0036a34 @ 0xf0036a34 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0036a34(int param_1,uint param_2,uint param_3)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined1 *puVar7;
  int iVar8;
  int *piVar9;
  int *piVar10;
  uint uVar11;
  int *piVar12;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1c94,_DAT_ram_00014800);
  }
  if (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x1c) != 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1c9a,_DAT_ram_00014800);
  }
  if ((param_2 != 0) && (param_3 != 0)) {
    uVar1 = FUN_ram_f00343fc(0,param_2 & 0xff);
    puVar5 = (undefined4 *)(param_1 + 0xd4);
    piVar9 = (int *)(param_1 + 0xdc);
    *(undefined4 *)(param_1 + 0x2ac) = 0;
    *(uint *)(param_1 + 0x2b0) = uVar1;
    *(undefined1 *)(param_1 + 0x2ab) = 0;
    uVar11 = 4;
    iVar6 = 0;
    do {
      iVar2 = param_1 + uVar11 * 0xc;
      piVar12 = (int *)(iVar2 + 0xa4);
      if (uVar11 == 4) {
        while (piVar10 = (int *)*puVar5, piVar10 != piVar12) {
          iVar8 = *piVar10;
          piVar3 = (int *)piVar10[1];
          *(int **)(iVar8 + 4) = piVar3;
          *piVar3 = iVar8;
          *piVar10 = 0;
          piVar10[1] = 0;
          *piVar9 = *piVar9 + -1;
          if (piVar10 == (int *)0x0) break;
          iVar2 = (*_DAT_ram_00014830)(iVar2,_DAT_ram_00014830);
          piVar10[7] = iVar2;
          FUN_ram_f00344b4(param_1,piVar10);
          FUN_ram_f0035fc4(piVar10);
          iVar2 = FUN_ram_f0033fc4();
        }
      }
      else if (((uVar1 >> (uVar11 & 0x1f) & 1) != 0) && ((int *)*puVar5 != piVar12)) {
        *(uint *)(param_1 + 0x2ac) = *(uint *)(param_1 + 0x2ac) | 1 << (uVar11 & 0x1f);
        iVar6 = iVar6 + 1;
      }
      uVar11 = uVar11 - 1;
      piVar9 = piVar9 + -3;
      puVar5 = puVar5 + -3;
    } while (uVar11 != 0xffffffff);
    if (iVar6 != 0) {
      if (*(int *)(param_1 + 0x2ac) == 0) {
        (*_DAT_ram_00014800)(0,0,0xf006380c,0x1cd2,_DAT_ram_00014800);
      }
      *(char *)(param_1 + 0x2ab) = (char)param_3;
      uVar1 = 3;
      do {
        if ((*(uint *)(param_1 + 0x2ac) >> (uVar1 & 0x1f) & 1) != 0) {
          iVar2 = param_1 + uVar1 * 0xc;
          piVar9 = (int *)(iVar2 + 0xa4);
          if (iVar6 != 1) {
            iVar6 = uVar1 * 0xc + param_1;
            piVar12 = *(int **)(iVar6 + 0xa4);
            if (piVar12 != piVar9) {
              piVar9 = (int *)piVar12[1];
              iVar2 = *piVar12;
              *(int **)(iVar2 + 4) = piVar9;
              *piVar9 = iVar2;
              piVar12[1] = 0;
              *piVar12 = 0;
              *(int *)(iVar6 + 0xac) = *(int *)(iVar6 + 0xac) + -1;
              if (piVar12 != (int *)0x0) goto LAB_ram_f0036c98;
            }
            (*_DAT_ram_00014800)(0,0,0xf006380c,0x1d0f,_DAT_ram_00014800);
            piVar12 = (int *)0x0;
LAB_ram_f0036c98:
            FUN_ram_f00344b4(param_1,piVar12);
            if (param_3 < 2) {
              FUN_ram_f00343c0(piVar12,1);
              puVar7 = (undefined1 *)0x0;
            }
            else {
              FUN_ram_f00343c0(piVar12,0);
              puVar7 = &LAB_ram_f0036880;
            }
            piVar12[0xf] = (int)puVar7;
            if (*(char *)(param_1 + 0x2ab) != '\0') {
              *(char *)(param_1 + 0x2ab) = *(char *)(param_1 + 0x2ab) + -1;
            }
            *(uint *)(param_1 + 0x2ac) = *(uint *)(param_1 + 0x2ac) & ~(1 << (uVar1 & 0x1f));
            iVar6 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
            piVar12[7] = iVar6;
            FUN_ram_f0035fc4(piVar12);
            FUN_ram_f0033fc4();
            return 1;
          }
          uVar1 = 0;
          goto LAB_ram_f0036c40;
        }
        uVar1 = uVar1 - 1;
      } while (uVar1 != 0xffffffff);
    }
  }
  return 0;
LAB_ram_f0036c40:
  if ((param_3 <= uVar1) || (piVar12 = *(int **)(iVar2 + 0xa4), piVar12 == piVar9)) {
LAB_ram_f0036c48:
    *(undefined4 *)(param_1 + 0x2ac) = 0;
    *(undefined1 *)(param_1 + 0x2ab) = 0;
    return 1;
  }
  piVar10 = (int *)piVar12[1];
  iVar6 = *piVar12;
  *(int **)(iVar6 + 4) = piVar10;
  *piVar10 = iVar6;
  *piVar12 = 0;
  piVar12[1] = 0;
  *(int *)(iVar2 + 0xac) = *(int *)(iVar2 + 0xac) + -1;
  if (piVar12 == (int *)0x0) goto LAB_ram_f0036c48;
  FUN_ram_f00344b4(param_1,piVar12);
  uVar4 = 0;
  if ((param_3 - 1 == uVar1) || (*(int **)(iVar2 + 0xa4) == piVar9)) {
    uVar4 = 1;
  }
  uVar4 = FUN_ram_f00343c0(piVar12,uVar4);
  uVar1 = uVar1 + 1;
  iVar6 = (*_DAT_ram_00014830)(uVar4,_DAT_ram_00014830);
  piVar12[7] = iVar6;
  FUN_ram_f0035fc4(piVar12);
  FUN_ram_f0033fc4();
  goto LAB_ram_f0036c40;
}

// >>> MOD: wifi/mgmt/txm.c:0x1bfa,0x1bfc,0x1c05,0x1c41 <<<
// ===== FUN_ram_f0036d28 @ 0xf0036d28 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0036d28(int param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  char *pcVar6;
  uint uVar7;
  int iVar8;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1bfa,_DAT_ram_00014800);
  }
  if (*(char *)(param_1 + 9) == '\0') {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x1bfc,_DAT_ram_00014800);
  }
  if (param_2 != 0) {
    if (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x1c) != 0) {
      (*_DAT_ram_00014800)(0,0,0xf006380c,0x1c05,_DAT_ram_00014800);
    }
    pcVar6 = (char *)(param_1 + 0x2be);
    iVar3 = 0;
    do {
      iVar3 = iVar3 + 1;
      if (*pcVar6 != '\0') {
        *(undefined1 *)(param_1 + 0x2c3) = 1;
        return 1;
      }
      pcVar6 = pcVar6 + 1;
    } while (iVar3 != 5);
    uVar1 = FUN_ram_f00343fc(0,param_2 & 0xff);
    if (*(int *)(param_1 + 0x2b4) == 0) {
      piVar4 = (int *)(param_1 + 0xa4);
      uVar2 = 0;
      do {
        iVar3 = param_1 + uVar2 * 0xc + 0xa4;
        if ((uVar2 == 4) && (*(int *)(param_1 + 0xd4) != iVar3)) {
          uVar7 = *(uint *)(param_1 + 0x2b4) | 0x10;
LAB_ram_f0036e14:
          *(uint *)(param_1 + 0x2b4) = uVar7;
        }
        else if (((uVar1 >> (uVar2 & 0x1f) & 1) != 0) && (*piVar4 != iVar3)) {
          uVar7 = *(uint *)(param_1 + 0x2b4) | 1 << (uVar2 & 0x1f);
          goto LAB_ram_f0036e14;
        }
        uVar2 = uVar2 + 1;
        piVar4 = piVar4 + 3;
      } while (uVar2 != 5);
    }
    *(undefined1 *)(param_1 + 0x2c3) = 0;
    uVar1 = 4;
    do {
      if ((*(uint *)(param_1 + 0x2b4) >> (uVar1 & 0x1f) & 1) != 0) {
        iVar3 = param_1 + uVar1 * 0xc;
        piVar4 = *(int **)(iVar3 + 0xa4);
        if (piVar4 != (int *)(iVar3 + 0xa4)) {
          piVar5 = (int *)piVar4[1];
          iVar8 = *piVar4;
          *(int **)(iVar8 + 4) = piVar5;
          *piVar5 = iVar8;
          piVar4[1] = 0;
          *piVar4 = 0;
          *(int *)(iVar3 + 0xac) = *(int *)(iVar3 + 0xac) + -1;
          if (piVar4 != (int *)0x0) goto LAB_ram_f0036e80;
        }
        (*_DAT_ram_00014800)(0,0,0xf006380c,0x1c41,_DAT_ram_00014800);
        piVar4 = (int *)0x0;
LAB_ram_f0036e80:
        *(uint *)(param_1 + 0x2b4) = *(uint *)(param_1 + 0x2b4) & ~(1 << (uVar1 & 0x1f));
        FUN_ram_f00344b4(param_1,piVar4);
        iVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
        piVar4[7] = iVar3;
        piVar4[0xf] = (int)&LAB_ram_f00351a8;
        FUN_ram_f0035fc4(piVar4);
        FUN_ram_f0033fc4();
        return 1;
      }
      uVar1 = uVar1 - 1;
    } while (uVar1 != 0xffffffff);
  }
  return 0;
}

// >>> MOD: wifi/mgmt/txm.c:0x19cf <<<
// ===== FUN_ram_f0036ee8 @ 0xf0036ee8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0036ee8(int *param_1)

{
  byte bVar1;
  char cVar2;
  bool bVar3;
  ushort uVar4;
  uint uVar5;
  int *piVar6;
  int iVar7;
  ushort *puVar8;
  int *piVar9;
  int iVar10;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x19cf,_DAT_ram_00014800);
  }
  bVar3 = false;
  while( true ) {
    while( true ) {
      piVar6 = (int *)*param_1;
      if ((piVar6 == param_1) || (piVar6 == (int *)0x0)) goto LAB_ram_f003703a;
      iVar10 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)((int)piVar6 + 0xd) * 4);
      if (*(char *)(iVar10 + 0x18) != '\0') break;
      piVar9 = (int *)piVar6[1];
      iVar10 = *piVar6;
      *(int **)(iVar10 + 4) = piVar9;
      *piVar9 = iVar10;
      piVar6[1] = 0;
      *piVar6 = 0;
      param_1[2] = param_1[2] + -1;
      FUN_ram_f00355c4(piVar6,0x44);
    }
    if ((*_DAT_ram_f0064f50 & 1) != 0) goto LAB_ram_f003703a;
    bVar1 = *(byte *)(piVar6 + 0x11);
    iVar7 = *(byte *)((int)piVar6 + 0x43) + 6;
    uVar5 = (*_DAT_ram_00014b24)
                      (_DAT_ram_020a0078,iVar7,(uint)*(byte *)((int)piVar6 + 0x43),_DAT_ram_00014b24
                      );
    if (uVar5 < iVar7 - (uint)bVar1) break;
    piVar6 = (int *)*param_1;
    if (piVar6 == param_1) goto LAB_ram_f003703a;
    piVar9 = (int *)piVar6[1];
    iVar7 = *piVar6;
    *(int **)(iVar7 + 4) = piVar9;
    *piVar9 = iVar7;
    piVar6[1] = 0;
    *piVar6 = 0;
    param_1[2] = param_1[2] + -1;
    if (piVar6 == (int *)0x0) goto LAB_ram_f003703a;
    if ((char)piVar6[0x10] == '\b') {
      if (*(char *)(iVar10 + 0xbb) == '\0') {
        *(undefined1 *)(piVar6 + 0x10) = 4;
      }
      else {
        cVar2 = (char)piVar6[4];
        if (cVar2 == '\0') {
          puVar8 = (ushort *)0x0;
        }
        else {
          puVar8 = (ushort *)piVar6[5];
        }
        if ((int *)*param_1 == param_1) {
          if (cVar2 == '\0') {
            *(undefined1 *)((int)piVar6 + 0x5d) = 0;
          }
          else {
            uVar4 = *puVar8 & 0xdfff;
LAB_ram_f0036ff4:
            *puVar8 = uVar4;
          }
        }
        else {
          if (cVar2 != '\0') {
            uVar4 = *puVar8 | 0x2000;
            goto LAB_ram_f0036ff4;
          }
          *(undefined1 *)((int)piVar6 + 0x5d) = 1;
        }
      }
    }
    iVar10 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    piVar6[7] = iVar10;
    iVar10 = FUN_ram_f00415d0();
    piVar6[0x18] = iVar10;
    FUN_ram_f0035fc4(piVar6);
    if (1 < (byte)((char)piVar6[0x10] - 7U)) {
      bVar3 = true;
    }
  }
  *_DAT_ram_f0064f50 = *_DAT_ram_f0064f50 | 1;
LAB_ram_f003703a:
  if (bVar3) {
    FUN_ram_f0033fc4();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0037050 @ 0xf0037050 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0037050(void)

{
  undefined4 uStack_c;
  
  FUN_ram_f0036ee8(_DAT_ram_f0064f50 + 0xc);
  if (*(int *)(_DAT_ram_f0064f50 + 0x13c) != 0) {
    uStack_c = 0;
    FUN_ram_f0034054(8,8,&uStack_c);
    (*_DAT_ram_00014b3c)(8,uStack_c,_DAT_ram_020a0078,_DAT_ram_00014b3c);
    *(uint *)(_DAT_ram_f0064f50 + 8) = *(uint *)(_DAT_ram_f0064f50 + 8) & 0xfffcffff;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00370b8 @ 0xf00370b8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00370b8(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  
  if (*(int *)(_DAT_ram_f0064f50 + 0x24) != _DAT_ram_f0064f50 + 0x24) {
    FUN_ram_f0036ee8();
  }
  iVar3 = 0;
  do {
    iVar1 = FUN_ram_f0025d90(iVar3);
    if (((*(char *)(iVar1 + 9) != '\0') &&
        (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar1 + 0x14) * 4) + 0x1c) != 1)) &&
       (*(char *)(iVar1 + 0x38) == '\0')) {
      piVar5 = (int *)(iVar1 + 0xd4);
      iVar4 = 4;
      do {
        iVar2 = iVar1 + iVar4 * 0xc + 0xa4;
        iVar4 = iVar4 + -1;
        if (*piVar5 != iVar2) {
          FUN_ram_f0036ee8(iVar2);
        }
        piVar5 = piVar5 + -3;
      } while (iVar4 != -1);
    }
    iVar3 = iVar3 + 1;
  } while (iVar3 != 0x14);
  piVar5 = (int *)&DAT_ram_020a0068;
  do {
    iVar3 = *piVar5;
    piVar5 = piVar5 + 1;
    if (((*(char *)(iVar3 + 0x18) != '\0') && (*(int *)(iVar3 + 0x1c) == 0)) &&
       (*(int *)(iVar3 + 0x350) != iVar3 + 0x350)) {
      FUN_ram_f0036ee8(iVar3 + 0x350);
    }
  } while (piVar5 != (int *)&DAT_ram_020a0074);
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x149c <<<
// ===== FUN_ram_f003718c @ 0xf003718c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003718c(uint param_1,uint param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int local_24 [2];
  
  if (param_2 != 0) {
    uVar2 = 0;
    local_24[0] = 0;
    if ((param_1 < 0xb) && ((1 << (param_1 & 0x1f) & 0x49dU) != 0)) {
      uVar3 = 0;
      do {
        uVar1 = uVar2 * 2;
        if ((param_2 >> (uVar2 & 0x1f) & 1) != 0) {
          if ((*(uint *)(_DAT_ram_f0064f50 + 8) >> (uVar1 & 0x1f) & 3) == 0) {
            *(uint *)(_DAT_ram_f0064f50 + 8) =
                 ~(3 << (uVar1 & 0x1f)) & *(uint *)(_DAT_ram_f0064f50 + 8);
            uVar3 = uVar3 | 1 << (uVar2 & 0x1f);
            *(uint *)(_DAT_ram_f0064f50 + 8) =
                 *(uint *)(_DAT_ram_f0064f50 + 8) | 2 << (uVar1 & 0x1f);
          }
        }
        uVar2 = uVar2 + 1;
      } while (uVar2 != 9);
      uVar2 = 0;
      (*_DAT_ram_00014b1c)(uVar3 & 0xffff,1,20000,_DAT_ram_020a0078,_DAT_ram_00014b1c);
      do {
        if ((uVar3 >> (uVar2 & 0x1f) & 1) != 0) {
          local_24[0] = 0;
          FUN_ram_f0034054(param_1,uVar2,local_24);
          if (local_24[0] != 0) {
            _DAT_ram_f0064fa4 = 0;
            FUN_ram_f0035804(local_24[0]);
            FUN_ram_f00370b8();
          }
        }
        uVar2 = uVar2 + 1;
      } while (uVar2 != 9);
    }
    else {
      (*_DAT_ram_00014800)(0,0,0xf006380c,0x149c,_DAT_ram_00014800);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00372b4 @ 0xf00372b4 =====


void FUN_ram_f00372b4(int param_1,char param_2)

{
  int iVar1;
  int iVar2;
  
  if ((*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x1c) == 0) ||
     (param_2 == '\x01')) {
    iVar2 = 4;
    do {
      iVar1 = iVar2 * 0xc;
      iVar2 = iVar2 + -1;
      FUN_ram_f0036ee8(param_1 + iVar1 + 0xa4);
    } while (iVar2 != -1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0037300 @ 0xf0037300 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0037300(uint param_1)

{
  FUN_ram_f00370b8();
  if (*(char *)(*(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0xbb) == '\0') {
    FUN_ram_f0036ee8(_DAT_ram_f0064f50 + 0xc);
  }
  FUN_ram_f0034f64(param_1 & 0xff,0);
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003734c @ 0xf003734c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003734c(uint param_1)

{
  bool bVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  int *piVar7;
  int local_34 [4];
  int aiStack_24 [2];
  
  if (*(char *)(_DAT_ram_f0064f50 + 0x141) != '\0') {
    FUN_ram_f003718c(7,0x1ff);
  }
  FUN_ram_f0034f64(param_1 & 0xff,0);
  piVar6 = local_34;
  local_34[3] = _DAT_ram_f0064f50 + 0x30;
  local_34[0] = *(int *)(&DAT_ram_020a0068 + param_1 * 4) + 0x350;
  local_34[1] = _DAT_ram_f0064f50 + 0xc;
  local_34[2] = _DAT_ram_f0064f50 + 0x24;
  do {
    piVar5 = (int *)*piVar6;
    if ((piVar5 != (int *)0x0) && (piVar2 = (int *)*piVar5, piVar2 != piVar5)) {
      piVar7 = (int *)*piVar2;
      do {
        if (*(byte *)((int)piVar2 + 0xd) == param_1) {
          piVar3 = (int *)piVar2[1];
          iVar4 = *piVar2;
          *(int **)(iVar4 + 4) = piVar3;
          *piVar3 = iVar4;
          *piVar2 = 0;
          piVar2[1] = 0;
          piVar5[2] = piVar5[2] + -1;
          FUN_ram_f00355c4(piVar2,0x43);
        }
        bVar1 = piVar7 != piVar5;
        piVar2 = piVar7;
        piVar7 = (int *)*piVar7;
      } while (bVar1);
    }
    piVar6 = piVar6 + 1;
  } while (aiStack_24 != piVar6);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003740c @ 0xf003740c =====


void FUN_ram_f003740c(undefined4 param_1,char param_2)

{
  if (param_2 == '\0') {
    FUN_ram_f003734c();
  }
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0xd37 <<<
// ===== FUN_ram_f0037428 @ 0xf0037428 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0037428(int *param_1,char param_2)

{
  uint uVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  
  uVar1 = (uint)*(byte *)(param_1 + 0x10);
  if (uVar1 == 5) {
    piVar4 = (int *)(_DAT_ram_f0064f50 + 0x24);
  }
  else if (uVar1 < 6) {
    iVar3 = param_1[8];
    if (uVar1 < 4) {
      piVar4 = (int *)(iVar3 + uVar1 * 0xc + 0xa4);
    }
    else if (iVar3 == 0) {
      piVar4 = (int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x350
                      );
    }
    else {
      piVar4 = (int *)(iVar3 + 0xd4);
    }
  }
  else if (uVar1 == 7) {
    piVar4 = (int *)(_DAT_ram_f0064f50 + 0x18);
  }
  else if (uVar1 < 7) {
    piVar4 = (int *)(_DAT_ram_f0064f50 + 0x30);
  }
  else if (uVar1 == 8) {
    piVar4 = (int *)(_DAT_ram_f0064f50 + 0xc);
  }
  else {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0xd37,_DAT_ram_00014800);
    piVar4 = (int *)0x0;
  }
  iVar3 = param_1[8];
  if ((iVar3 != 0) && ((*(uint *)(iVar3 + 0x10) & 0x80) != 0)) {
    (*_DAT_ram_00014814)
              (2,0xf0063864,*(undefined1 *)(iVar3 + 10),*(undefined1 *)(iVar3 + 0xb),
               *(undefined1 *)(iVar3 + 0xc),*(undefined1 *)(iVar3 + 0xd),
               *(undefined1 *)(iVar3 + 0xe),*(undefined1 *)(iVar3 + 0xf),(char)param_1[0x10],
               *(undefined2 *)((int)param_1 + 0x26),piVar4[2]);
  }
  iVar3 = FUN_ram_f003f094(param_1);
  if (iVar3 == -0x3fffffff) {
    FUN_ram_f00355c4(param_1,0xc001000f);
  }
  else {
    if (param_2 == '\0') {
      piVar2 = (int *)piVar4[1];
      piVar4[1] = (int)param_1;
      *param_1 = (int)piVar4;
      param_1[1] = (int)piVar2;
      *piVar2 = (int)param_1;
    }
    else {
      iVar3 = *piVar4;
      *(int **)(iVar3 + 4) = param_1;
      *param_1 = iVar3;
      param_1[1] = (int)piVar4;
      *piVar4 = (int)param_1;
    }
    piVar4[2] = piVar4[2] + 1;
  }
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0x155a <<<
// ===== FUN_ram_f003757c @ 0xf003757c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003757c(int param_1,uint param_2,uint param_3)

{
  uint *puVar1;
  uint *puVar2;
  uint *puVar3;
  uint *puVar4;
  uint *puVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint *puVar9;
  uint *puVar10;
  uint *puVar11;
  uint *puVar12;
  uint uStack_48;
  uint *puStack_38;
  uint *local_34;
  uint *puStack_30;
  uint *local_24 [2];
  
  if (param_2 != 0) {
    local_24[0] = (uint *)0x0;
    uVar8 = 0;
    uVar7 = 0;
    do {
      uVar6 = uVar8 * 2;
      if ((param_2 >> (uVar8 & 0x1f) & 1) != 0) {
        if ((_DAT_ram_f0064f50[2] >> (uVar6 & 0x1f) & 3) != 2) {
          _DAT_ram_f0064f50[2] = ~(3 << (uVar6 & 0x1f)) & _DAT_ram_f0064f50[2];
          uVar7 = uVar7 | 1 << (uVar8 & 0x1f);
          _DAT_ram_f0064f50[2] = _DAT_ram_f0064f50[2] | 2 << (uVar6 & 0x1f);
        }
      }
      uVar8 = uVar8 + 1;
    } while (uVar8 != 9);
    uStack_48 = 0;
    (*_DAT_ram_00014b1c)(uVar7 & 0xffff,1,20000,_DAT_ram_020a0078,_DAT_ram_00014b1c);
    do {
      if ((uVar7 >> (uStack_48 & 0x1f) & 1) != 0) {
        local_24[0] = (uint *)0x0;
        FUN_ram_f0034054(param_1,uStack_48,local_24);
        if (local_24[0] != (uint *)0x0) {
          local_34 = (uint *)0x0;
          puStack_38 = (uint *)0x0;
          puStack_30 = (uint *)0x0;
          puVar5 = local_24[0];
          puVar9 = _DAT_ram_f006389c;
          puVar3 = _DAT_ram_f006389c;
          puVar12 = _DAT_ram_f006389c;
          do {
            uVar8 = puVar5[1];
            puVar4 = (uint *)puVar5[0xb];
            if ((*puVar5 & 1) == 0) {
              puVar4 = puVar5;
              if (puVar9 != (uint *)0x0) {
                puVar9[1] = (uint)puVar5 & 0xfffffffc | puVar9[1] & 3;
                puVar5 = local_34;
              }
            }
            else {
              uVar6 = (*(ushort *)((int)puVar5 + 0x16) ^ 0x2000) >> 0xd & 1;
              puVar10 = puVar3;
              puVar11 = puVar12;
              puVar1 = puStack_38;
              puVar2 = puStack_30;
              if (param_1 == 6) {
                if (uVar6 != 0) {
                  if (puVar4[8] == param_3) {
LAB_ram_f00376ac:
                    puVar10 = puVar4;
                    puVar1 = puVar4;
                    if (puVar3 != (uint *)0x0) {
                      *puVar4 = (uint)puVar3;
                      puVar1 = puStack_38;
                    }
                  }
                  else {
LAB_ram_f00376b6:
                    puVar11 = puVar4;
                    puVar2 = puVar4;
                    if (puVar12 != (uint *)0x0) {
                      *puVar12 = (uint)puVar4;
                      puVar2 = puStack_30;
                    }
                  }
                  goto LAB_ram_f00376e0;
                }
              }
              else {
                if (param_1 != 8) {
                  if (param_1 == 5) {
                    if (uVar6 != 0) {
                      if ((param_3 >> (*(byte *)((int)puVar4 + 0xd) & 0x1f) & 1) == 0)
                      goto LAB_ram_f00376b6;
                      goto LAB_ram_f00376ac;
                    }
                    goto LAB_ram_f00376f0;
                  }
                  (*_DAT_ram_00014800)(0,0,0xf006380c,0x155a,_DAT_ram_00014800);
                }
                if (uVar6 != 0) {
LAB_ram_f00376e0:
                  puStack_30 = puVar2;
                  puStack_38 = puVar1;
                  FUN_ram_f003432c(puVar4);
                  puVar3 = puVar10;
                  puVar12 = puVar11;
                }
              }
LAB_ram_f00376f0:
              (*_DAT_ram_00014b30)(puVar5,_DAT_ram_020a0078);
              *_DAT_ram_f0064f50 = *_DAT_ram_f0064f50 & 0xfffffffe;
              puVar4 = puVar9;
              puVar5 = local_34;
            }
            local_34 = puVar5;
            puVar5 = (uint *)(uVar8 & 0xfffffffc);
            puVar9 = puVar4;
          } while ((uint *)(uVar8 & 0xfffffffc) != (uint *)0x0);
          if (puStack_38 != (uint *)0x0) {
            *puStack_38 = 0;
            do {
              puVar5 = (uint *)*puVar3;
              FUN_ram_f0037428(puVar3,1);
              puVar3 = puVar5;
            } while (puVar5 != (uint *)0x0);
          }
          if (puVar4 != (uint *)0x0) {
            puVar4[1] = puVar4[1] & 3;
            _DAT_ram_f0064fa4 = 1;
            FUN_ram_f0035804(local_34);
          }
          if (puVar12 != (uint *)0x0) {
            *puVar12 = 0;
            FUN_ram_f0036814(puStack_30);
          }
          if (puVar4 != (uint *)0x0) {
            FUN_ram_f00370b8();
          }
        }
      }
      uStack_48 = uStack_48 + 1;
    } while (uStack_48 != 9);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00377e0 @ 0xf00377e0 =====


undefined4 FUN_ram_f00377e0(int param_1,char param_2)

{
  if (param_2 == '\0') {
    FUN_ram_f00372b4(param_1,0);
    FUN_ram_f0034e90(*(undefined1 *)(param_1 + 8),0);
    *(undefined4 *)(param_1 + 0x2b4) = 0;
    *(undefined1 *)(param_1 + 0x2ab) = 0;
  }
  else {
    FUN_ram_f003757c(6,0x7f,param_1);
    FUN_ram_f0034e90(*(undefined1 *)(param_1 + 8),1);
    *(undefined1 *)(param_1 + 0x2b9) = 0;
    *(undefined1 *)(param_1 + 0x2ab) = 0;
    *(undefined4 *)(param_1 + 0x2b4) = 0;
  }
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0037844 @ 0xf0037844 =====


undefined4 FUN_ram_f0037844(uint param_1)

{
  FUN_ram_f0034f64(param_1 & 0xff);
  FUN_ram_f003757c(5,0x17f,1 << (param_1 & 0x1f));
  FUN_ram_f003718c(2,0x80);
  return 1;
}

// >>> MOD: wifi/mgmt/txm.c:0xb1d <<<
// ===== FUN_ram_f0037888 @ 0xf0037888 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0037888(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x20) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0xb1d,_DAT_ram_00014800);
  }
  if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0x18) == '\0') {
    FUN_ram_f00355c4(param_1,0x3c);
  }
  else {
    *(undefined1 *)(param_1 + 0x42) = 0;
    uVar1 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    *(undefined4 *)(param_1 + 0x1c) = uVar1;
    if (*(char *)(param_1 + 0x48) == '\0') {
      *(undefined1 *)(param_1 + 0x48) = 1;
      *(undefined1 *)(param_1 + 0x4a) = 7;
      *(undefined1 *)(param_1 + 0x49) =
           *(undefined1 *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0x349)
      ;
    }
    FUN_ram_f0028638(param_1);
    FUN_ram_f0034ad8(param_1);
    FUN_ram_f0035fc4(param_1);
    FUN_ram_f0033fc4();
  }
  return;
}

// >>> MOD: wifi/mgmt/txm.c:0xa6d,0xa7c,0xaaf,0xab0,0xab5,0xac6 <<<
// ===== FUN_ram_f0037940 @ 0xf0037940 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0037940(int *param_1)

{
  undefined1 uVar1;
  byte bVar2;
  bool bVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  int *piVar7;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0xa6d,_DAT_ram_00014800);
  }
  do {
    piVar7 = (int *)*param_1;
    if ((param_1[5] == 0) || (*(char *)((int)param_1 + 0x11) == '\0')) {
      (*_DAT_ram_00014800)(0,0,0xf006380c,0xa7c,_DAT_ram_00014800);
    }
    iVar4 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    param_1[7] = iVar4;
    iVar4 = FUN_ram_f00415d0();
    param_1[0x18] = iVar4;
    if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x18) ==
        '\0') {
      uVar6 = 0x3a;
LAB_ram_f00379dc:
      FUN_ram_f00355c4(param_1,uVar6);
    }
    else {
      iVar4 = param_1[8];
      if (iVar4 == 0) {
        if ((char)param_1[0x12] == '\0') {
          *(undefined1 *)(param_1 + 0x12) = 1;
          uVar1 = *(undefined1 *)
                   (*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x349);
          *(undefined1 *)((int)param_1 + 0x4a) = 0;
          *(undefined1 *)((int)param_1 + 0x49) = uVar1;
        }
      }
      else if ((*(char *)(iVar4 + 9) == '\0') || (*(char *)(iVar4 + 0x15) != '\x02')) {
        uVar6 = 0x3b;
        goto LAB_ram_f00379dc;
      }
      if ((char)param_1[0x16] == '\0') {
        FUN_ram_f0028650(param_1);
      }
      else {
        FUN_ram_f00285a8(param_1);
      }
      if ((char)param_1[0x10] == '\b') {
        if (param_1[8] != 0) {
          (*_DAT_ram_00014800)(0,0,0xf006380c,0xaaf,_DAT_ram_00014800);
        }
        if ((char)param_1[4] == '\x01') {
          bVar3 = (bool)((*(byte *)(param_1[5] + 4) ^ 1) & 1);
        }
        else {
          bVar3 = param_1[8] != 0;
        }
        if (bVar3 != false) {
          (*_DAT_ram_00014800)(0,0,0xf006380c,0xab0,_DAT_ram_00014800);
        }
        if (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0xc) !=
            2) {
          (*_DAT_ram_00014800)(0,0,0xf006380c,0xab5,_DAT_ram_00014800);
        }
      }
      else {
        if ((char)param_1[0x10] == '\a') {
          (*_DAT_ram_00014800)(0,0,0xf006380c,0xac6,_DAT_ram_00014800);
        }
        if ((((param_1[8] == 0) || (*(char *)(param_1[8] + 0x38) == '\0')) ||
            ((char)param_1[9] < '\0')) && ((*_DAT_ram_f0064f50 & 1) == 0)) {
          bVar2 = *(byte *)((int)param_1 + 0x43);
          uVar5 = (*_DAT_ram_00014b24)(_DAT_ram_020a0078,_DAT_ram_00014b24);
          if (uVar5 < bVar2 + 6) {
            FUN_ram_f0037428(param_1,0);
            *_DAT_ram_f0064f50 = *_DAT_ram_f0064f50 | 1;
          }
          else {
            FUN_ram_f0035fc4(param_1);
            FUN_ram_f0033fc4();
          }
          goto LAB_ram_f0037b44;
        }
      }
      FUN_ram_f0037428(param_1,0);
    }
LAB_ram_f0037b44:
    param_1 = piVar7;
    if (piVar7 == (int *)0x0) {
      return;
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/txm.c:0x969,0x97f,0x9e5,0xa44 <<<
// ===== FUN_ram_f0037b58 @ 0xf0037b58 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0037b58(int *param_1)

{
  byte bVar1;
  char cVar2;
  uint *puVar3;
  uint *puVar4;
  int *piVar5;
  int *piVar6;
  int iVar7;
  undefined1 uVar8;
  undefined4 uVar9;
  int iVar10;
  uint uVar11;
  undefined4 local_28;
  ushort local_24;
  ushort local_22 [3];
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x969,_DAT_ram_00014800);
  }
  do {
    piVar5 = (int *)*param_1;
    if (((((char)param_1[10] != '\0') || ((char)param_1[4] == '\0')) || (param_1[6] == 0)) ||
       ((param_1[5] == 0 || (piVar6 = piVar5, *(char *)((int)param_1 + 0x11) == '\0')))) {
      piVar6 = (int *)(*_DAT_ram_00014800)(0,0,0xf006380c,0x97f,_DAT_ram_00014800);
    }
    if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x18) ==
        '\0') {
      uVar9 = 0x37;
LAB_ram_f0037be4:
      FUN_ram_f00355c4(param_1,uVar9);
    }
    else {
      if ((param_1[8] != 0) && (*(char *)(param_1[8] + 9) == '\0')) {
        uVar9 = 0x38;
        goto LAB_ram_f0037be4;
      }
      iVar7 = (*_DAT_ram_00014830)(piVar6,_DAT_ram_00014830);
      param_1[7] = iVar7;
      if ((char)param_1[0x12] == '\0') {
        *(undefined1 *)(param_1 + 0x12) = 1;
        if ((*(ushort *)param_1[5] & 0xfc) == 0x50) {
          uVar8 = 2;
        }
        else {
          uVar8 = 7;
        }
        *(undefined1 *)((int)param_1 + 0x4a) = uVar8;
        if (_DAT_ram_f006513c == 0x24ba58) {
          if (param_1[8] == 0) {
            uVar8 = *(undefined1 *)
                     (*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)((int)param_1 + 0xd) * 4) + 0x349)
            ;
          }
          else {
            uVar8 = *(undefined1 *)(param_1[8] + 0x24);
          }
        }
        else {
          uVar8 = 0x6b;
        }
        *(undefined1 *)((int)param_1 + 0x49) = uVar8;
      }
      FUN_ram_f00285a8(param_1);
      if ((*(ushort *)param_1[5] & 0xfc) == 0xb0) {
        if (_DAT_ram_f0064f8c < 100) {
          iVar10 = _DAT_ram_f0064f8c * 5;
          iVar7 = _DAT_ram_f0064f8c * 10;
          local_24 = 1;
          local_28 = 0;
          bVar1 = *(byte *)(param_1 + 0x10);
          *(undefined1 *)(iVar7 + -0xff91358) = 1;
          *(undefined1 *)(iVar7 + -0xff91357) = 1;
          local_22[0] = (ushort)bVar1;
          (*(code *)&SUB_ram_0006a08c)((iVar10 + 1) * 2 + -0xff91358,&local_24,2);
          (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91354,&local_28);
          (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91350,local_22,2);
          _DAT_ram_f0064f8c = _DAT_ram_f0064f8c + 1;
          if (99 < _DAT_ram_f0064f8c) {
            _DAT_ram_f0064f8c = 0;
          }
        }
        else {
          _DAT_ram_f0064f8c = 0;
        }
      }
      if ((*(ushort *)param_1[5] & 0xfc) == 0x40) {
        if (_DAT_ram_f0064f8c < 100) {
          iVar7 = _DAT_ram_f0064f8c * 10;
          iVar10 = _DAT_ram_f0064f8c * 5;
          local_22[0] = 2;
          local_28 = 0;
          bVar1 = *(byte *)(param_1 + 0x10);
          *(undefined1 *)(iVar7 + -0xff91358) = 1;
          *(undefined1 *)(iVar7 + -0xff91357) = 1;
          local_24 = (ushort)bVar1;
          (*(code *)&SUB_ram_0006a08c)((iVar10 + 1) * 2 + -0xff91358,local_22,2);
          (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91354,&local_28);
          (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91350,&local_24,2);
          _DAT_ram_f0064f8c = _DAT_ram_f0064f8c + 1;
          if (99 < _DAT_ram_f0064f8c) {
            _DAT_ram_f0064f8c = 0;
          }
        }
        else {
          _DAT_ram_f0064f8c = 0;
        }
      }
      cVar2 = (char)param_1[0x10];
      if (((1 < (byte)(cVar2 - 4U)) && (cVar2 != '\a')) && (cVar2 != '\b')) {
        (*_DAT_ram_00014800)(0,0,0xf006380c,0x9e5,_DAT_ram_00014800);
      }
      FUN_ram_f0034ad8(param_1);
      uVar11 = (uint)*(byte *)(param_1 + 0x10);
      if (uVar11 == 7) {
        FUN_ram_f0035fc4(param_1);
        goto LAB_ram_f0037f02;
      }
      if (uVar11 < 8) {
        if (1 < uVar11 - 4) goto LAB_ram_f0037ee8;
        iVar7 = FUN_ram_f002a114(param_1);
        if (iVar7 == 1) {
LAB_ram_f0037eca:
          FUN_ram_f0037428(param_1,0);
        }
        else if (iVar7 == 0) {
          FUN_ram_f00355c4(param_1,0x39);
          puVar4 = _DAT_ram_f0064f50;
          uVar11 = _DAT_ram_f0064f50[0x34];
          puVar3 = _DAT_ram_f0064f50 + 0x35;
          _DAT_ram_f0064f50[0x34] = uVar11 + 1;
          puVar4[0x35] = *puVar3 + (uint)(uVar11 + 1 == 0);
        }
        else {
          if ((*_DAT_ram_f0064f50 & 1) != 0) goto LAB_ram_f0037eca;
          bVar1 = *(byte *)((int)param_1 + 0x43);
          uVar11 = (*_DAT_ram_00014b24)(_DAT_ram_020a0078,_DAT_ram_00014b24);
          if (uVar11 < bVar1) {
            FUN_ram_f0037428(param_1,0);
            *_DAT_ram_f0064f50 = *_DAT_ram_f0064f50 | 1;
          }
          else {
            FUN_ram_f0035fc4(param_1);
            FUN_ram_f0033fc4();
          }
        }
      }
      else {
        if (uVar11 == 8) goto LAB_ram_f0037eca;
LAB_ram_f0037ee8:
        (*_DAT_ram_00014800)(0,0,0xf006380c,0xa44,_DAT_ram_00014800);
      }
    }
LAB_ram_f0037f02:
    param_1 = piVar5;
    if (piVar5 == (int *)0x0) {
      return;
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/txm.c:0x697,0x83f,0x8a3,0x8a4,0x8b4,0x91e <<<
// ===== FUN_ram_f0037f14 @ 0xf0037f14 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0037f14(int *param_1)

{
  undefined2 uVar1;
  byte bVar2;
  byte bVar3;
  char cVar4;
  undefined1 uVar5;
  undefined1 uVar6;
  uint *puVar7;
  uint *puVar8;
  undefined1 uVar9;
  uint uVar10;
  int iVar11;
  int iVar12;
  int *piVar13;
  undefined4 *puVar14;
  undefined4 uVar15;
  bool bVar16;
  short sVar17;
  int *piVar18;
  undefined1 *puVar19;
  int iVar20;
  uint uVar21;
  int iVar22;
  ushort *puVar23;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006380c,0x697,_DAT_ram_00014800);
  }
  uVar5 = 0;
  uVar9 = 0;
  do {
    piVar13 = (int *)*param_1;
    iVar22 = param_1[2];
    puVar23 = (ushort *)(iVar22 + 0x30);
    bVar2 = *(byte *)(iVar22 + 0x35);
    bVar3 = *(byte *)(iVar22 + 0x33);
    *(undefined2 *)(param_1 + 0x1a) = *(undefined2 *)(iVar22 + 0x3e);
    uVar21 = (bVar2 & 0x30) >> 4;
    bVar3 = bVar3 >> 6;
    iVar20 = *(int *)(&DAT_ram_020a0068 + uVar21 * 4);
    *(undefined2 *)((int)param_1 + 0x26) = 0;
    if (bVar3 == 2) {
      bVar2 = *(byte *)(iVar22 + 0x33);
      *(undefined1 *)((int)param_1 + 0x2f) = 0;
      *(char *)((int)param_1 + 0xe) = (char)((bVar2 & 0x3c) >> 2);
      *param_1 = 0;
      param_1[10] = (int)puVar23;
      *(undefined2 *)(iVar22 + 0x32) = 2;
      *puVar23 = *puVar23 & 0xfff;
      iVar20 = _DAT_ram_0209fce8 + (uint)*(byte *)((int)param_1 + 0x2f) * 0xc;
      puVar14 = *(undefined4 **)(iVar20 + 4);
      *(int **)(iVar20 + 4) = param_1;
      *param_1 = iVar20;
      param_1[1] = (int)puVar14;
      *puVar14 = param_1;
      piVar18 = (int *)(_DAT_ram_0209fce8 + (uint)*(byte *)((int)param_1 + 0x2f) * 0xc + 8);
      *piVar18 = *piVar18 + 1;
      FUN_ram_f00387ac(param_1);
      (*(code *)&SUB_ram_0006a854)(*(undefined1 *)((int)param_1 + 0x2f));
    }
    else {
      *(byte *)((int)param_1 + 0x5b) = bVar3;
      *(bool *)(param_1 + 0x16) = bVar3 == 3;
      bVar2 = *(byte *)(iVar22 + 0x3d);
      uVar10 = bVar2 & 1;
      *(char *)((int)param_1 + 0x59) = (char)uVar10;
      *(undefined1 *)((int)param_1 + 0x5a) = *(undefined1 *)(iVar22 + 0x3c);
      *(byte *)((int)param_1 + 0x42) = *(byte *)(iVar22 + 0x3d) >> 1 & 1;
      *(byte *)(param_1 + 0x12) = *(byte *)(iVar22 + 0x3d) >> 2 & 1;
      if ((bVar2 & 1) == 0) {
        param_1[0xb] = uVar10;
      }
      else {
        param_1[0xb] = (int)&LAB_ram_f0034fcc;
      }
      param_1[8] = 0;
      uVar6 = (undefined1)((*(byte *)(iVar22 + 0x33) & 0x3c) >> 2);
      *(undefined1 *)((int)param_1 + 0xe) = uVar6;
      puVar8 = _DAT_ram_f0064f50;
      if (_DAT_ram_f00650a4 == 4) {
        uVar15 = 0x2a;
      }
      else {
        if ((char)param_1[0x16] != '\0') {
          puVar19 = &LAB_ram_f0034fcc;
          if (*(char *)((int)param_1 + 0x59) == '\0') {
            puVar19 = (undefined1 *)0x0;
          }
          iVar20 = FUN_ram_f0025d90(*(undefined1 *)(iVar22 + 0x3a));
          uVar10 = *(byte *)(iVar22 + 0x34) & 0x3f;
          FUN_ram_f0034540(param_1,iVar20,uVar21,param_1[2] + 0x40,uVar10,param_1[2] + 0x40 + uVar10
                           ,((*puVar23 & 0xfff) - 0x10) - uVar10 & 0xffff,puVar19,0,0,0,0,0);
          if ((((char)param_1[3] == '\0') &&
              ((*_DAT_ram_00014800)
                         (_DAT_ram_f006389c,_DAT_ram_f006389c,0xf006380c,0x733,_DAT_ram_00014800),
              iVar20 != 0)) && (*(char *)(iVar20 + 9) != '\0')) {
            *(char *)(iVar20 + 0x2b8) = *(char *)(iVar20 + 0x2b8) + '\x01';
          }
          FUN_ram_f0037b58(param_1);
          return;
        }
        if (*(char *)(iVar20 + 0x18) == '\0') {
          uVar15 = 0x2b;
        }
        else {
          uVar10 = *(uint *)(iVar20 + 0xc);
          if (uVar10 < 3) {
            cVar4 = *(char *)(iVar22 + 0x3a);
            if (uVar10 == 0) {
              iVar11 = FUN_ram_f0025d90();
              puVar8 = _DAT_ram_f0064f50;
              if ((iVar11 == 0) || ((*(uint *)(iVar11 + 0x10) & 0x80) == 0)) {
                iVar11 = *(int *)(iVar20 + 0x58);
                if ((iVar11 != 0) && (*(char *)(iVar11 + 9) != '\0')) {
                  if (*(char *)(iVar11 + 0x361) == '\0') {
                    _DAT_ram_f006e84c = _DAT_ram_f006e84c + 1;
                  }
                  if (((*(char *)(iVar11 + 0x14) != '\0') ||
                      ((*(byte *)(iVar22 + 0x35) & 0x40) != 0)) ||
                     ((*(uint *)(_DAT_ram_020a0068 + 0x10) < 3 ||
                      ((*(uint *)(_DAT_ram_020a0068 + 0x10) == 5 ||
                       (*(char *)(iVar11 + 0x289) != '\0')))))) goto LAB_ram_f00383a6;
                }
                uVar21 = _DAT_ram_f0064f50[0x26];
                puVar7 = _DAT_ram_f0064f50 + 0x27;
                _DAT_ram_f0064f50[0x26] = uVar21 + 1;
                puVar8[0x27] = *puVar7 + (uint)(uVar21 + 1 == 0);
                uVar15 = 0x30;
              }
              else {
                if (*(char *)(iVar11 + 9) != '\0') {
                  if (*(char *)(iVar11 + 0x361) == '\0') {
                    _DAT_ram_f006e84c = _DAT_ram_f006e84c + 1;
                  }
LAB_ram_f00383a6:
                  if (param_1[2] == 0) {
                    (*_DAT_ram_00014800)(0,0,0xf006380c,0x83f,_DAT_ram_00014800);
                  }
                  *(char *)((int)param_1 + 0xd) = (char)uVar21;
                  param_1[8] = iVar11;
                  param_1[5] = 0;
                  *(undefined1 *)((int)param_1 + 0x11) = 0;
                  param_1[6] = 0;
                  *(undefined2 *)((int)param_1 + 0x12) = 0;
                  *(char *)((int)param_1 + 0xf) = (char)((*puVar23 & 0x7000) >> 0xc);
                  *(byte *)(param_1 + 4) = *(byte *)(iVar22 + 0x35) >> 7;
                  iVar12 = (*_DAT_ram_00014830)();
                  param_1[7] = iVar12;
                  *(undefined1 *)((int)param_1 + 0xe) = uVar6;
                  bVar2 = *(byte *)(iVar22 + 0x35);
                  *(undefined1 *)((int)param_1 + 0x29) = 0;
                  *(byte *)(param_1 + 10) = bVar2 >> 6 & 1;
                  param_1[0xb] = 0;
                  *(byte *)(param_1 + 0xe) = *(byte *)(iVar22 + 0x3b) & 3;
                  bVar2 = *(byte *)(iVar22 + 0x3b);
                  *(undefined1 *)((int)param_1 + 0x42) = 0;
                  *(char *)((int)param_1 + 0x39) = (char)((bVar2 & 0x1c) >> 2);
                  *(undefined1 *)(param_1 + 0x12) = uVar9;
                  *(undefined1 *)((int)param_1 + 0x4a) = 0;
                  *(undefined1 *)((int)param_1 + 0x49) = uVar5;
                  *(undefined2 *)(param_1 + 0x14) = *(undefined2 *)(iVar22 + 0x36);
                  uVar1 = *(undefined2 *)(iVar22 + 0x38);
                  *(undefined1 *)((int)param_1 + 0x41) = 0;
                  *(undefined2 *)((int)param_1 + 0x52) = uVar1;
                  *(undefined1 *)(param_1 + 0x15) = 0;
                  *(undefined1 *)(param_1 + 0x16) = 0;
                  iVar12 = FUN_ram_f00415d0();
                  param_1[0x18] = iVar12;
                  if ((*(byte *)(iVar22 + 0x3d) & 8) != 0) {
                    param_1[0xb] = (int)&LAB_ram_f00350bc;
                  }
                  if ((char)param_1[10] != '\0') {
                    if (iVar11 == 0) {
                      uVar15 = 0x34;
                      goto LAB_ram_f0038130;
                    }
                    if ((char)param_1[4] == '\0') {
                      param_1[6] = param_1[2] + 0x4e;
                      sVar17 = (*puVar23 & 0xfff) - 0x1e;
                    }
                    else {
                      param_1[6] = param_1[2] + (*(byte *)(iVar22 + 0x34) & 0x3f) + 0x40;
                      sVar17 = ((*puVar23 & 0xfff) - 0x10) - (*(byte *)(iVar22 + 0x34) & 0x3f);
                    }
                    *(short *)((int)param_1 + 0x12) = sVar17;
                  }
                  FUN_ram_f00351f4(param_1,puVar23);
                  if (iVar11 != 0) {
                    FUN_ram_f004052c(param_1[8],param_1,0,0,*(undefined1 *)(iVar11 + 0x2b8),0);
                  }
                  FUN_ram_f0028650(param_1);
                  FUN_ram_f0034ad8(param_1);
                  cVar4 = (char)param_1[0x10];
                  if (cVar4 == '\b') {
                    if ((iVar11 != 0) || (*(int *)(iVar20 + 0xc) != 2)) {
                      (*_DAT_ram_00014800)(0,0,0xf006380c,0x8a3,_DAT_ram_00014800);
                    }
                    if ((char)param_1[4] == '\x01') {
                      bVar16 = (bool)((*(byte *)(param_1[5] + 4) ^ 1) & 1);
                    }
                    else {
                      bVar16 = param_1[8] != 0;
                    }
                    if (bVar16 != false) {
                      (*_DAT_ram_00014800)(0,0,0xf006380c,0x8a4,_DAT_ram_00014800);
                    }
                  }
                  else {
                    if ((cVar4 == '\x05') || (cVar4 == '\a')) {
                      (*_DAT_ram_00014800)(0,0,0xf006380c,0x8b4,_DAT_ram_00014800);
                    }
                    if ((iVar11 != 0) && (*(char *)(iVar11 + 9) != '\0')) {
                      if ((char)param_1[3] == '\0') {
                        *(char *)(iVar11 + 0x2b8) = *(char *)(iVar11 + 0x2b8) + '\x01';
                      }
                      if (*(char *)(iVar11 + 0x38) != '\0') {
                        if ((byte)_DAT_ram_f0064f50[0x50] + 2 < (uint)*(byte *)(iVar11 + 0x2b8)) {
                          FUN_ram_f00355c4(param_1,0x36);
                          puVar7 = _DAT_ram_f0064f50;
                          uVar21 = _DAT_ram_f0064f50[0x1e];
                          puVar8 = _DAT_ram_f0064f50 + 0x1f;
                          _DAT_ram_f0064f50[0x1e] = uVar21 + 1;
                          puVar7[0x1f] = *puVar8 + (uint)(uVar21 + 1 == 0);
                          goto LAB_ram_f00386de;
                        }
                        goto LAB_ram_f00386bc;
                      }
                    }
                    iVar22 = 1;
                    if (((*(int *)(iVar20 + 0x1c) != 1) ||
                        (iVar22 = FUN_ram_f003e5c8(param_1,iVar20,iVar11), iVar22 != 0)) &&
                       ((*_DAT_ram_f0064f50 & 1) == 0)) {
                      bVar2 = *(byte *)((int)param_1 + 0x43);
                      uVar21 = (*_DAT_ram_00014b24)(_DAT_ram_020a0078,iVar22,_DAT_ram_00014b24);
                      if (uVar21 < bVar2 + 6) {
                        FUN_ram_f0037428(param_1,0);
                        *_DAT_ram_f0064f50 = *_DAT_ram_f0064f50 | 1;
                        goto LAB_ram_f00386de;
                      }
                      if (iVar11 != 0) {
                        if (4 < *(byte *)(param_1 + 0x10)) {
                          (*_DAT_ram_00014800)(0,0,0xf006380c,0x91e,_DAT_ram_00014800);
                        }
                        if (*(int *)(iVar11 + (uint)*(byte *)(param_1 + 0x10) * 0xc + 0xa4) !=
                            iVar11 + 0xa4 + (uint)*(byte *)(param_1 + 0x10) * 0xc)
                        goto LAB_ram_f00386bc;
                      }
                      FUN_ram_f0035fc4(param_1);
                      FUN_ram_f0033fc4();
                      goto LAB_ram_f00386de;
                    }
                  }
LAB_ram_f00386bc:
                  FUN_ram_f0037428(param_1,0);
                  goto LAB_ram_f00386de;
                }
                uVar21 = _DAT_ram_f0064f50[0x26];
                puVar7 = _DAT_ram_f0064f50 + 0x27;
                _DAT_ram_f0064f50[0x26] = uVar21 + 1;
                puVar8[0x27] = *puVar7 + (uint)(uVar21 + 1 == 0);
                uVar15 = 0x2f;
              }
            }
            else if (cVar4 == -2) {
              uVar21 = _DAT_ram_f0064f50[0x26];
              puVar7 = _DAT_ram_f0064f50 + 0x27;
              _DAT_ram_f0064f50[0x26] = uVar21 + 1;
              puVar8[0x27] = *puVar7 + (uint)(uVar21 + 1 == 0);
              uVar15 = 0x2c;
            }
            else {
              if (cVar4 == -1) {
LAB_ram_f003839a:
                uVar5 = *(undefined1 *)(iVar20 + 0x349);
                uVar9 = 1;
                iVar11 = 0;
                goto LAB_ram_f00383a6;
              }
              iVar11 = FUN_ram_f0025d90(cVar4);
              puVar8 = _DAT_ram_f0064f50;
              if ((iVar11 == 0) || (*(char *)(iVar11 + 9) == '\0')) {
                uVar21 = _DAT_ram_f0064f50[0x26];
                puVar7 = _DAT_ram_f0064f50 + 0x27;
                _DAT_ram_f0064f50[0x26] = uVar21 + 1;
                puVar8[0x27] = *puVar7 + (uint)(uVar21 + 1 == 0);
                uVar15 = 0x2e;
              }
              else {
                if (*(char *)(iVar11 + 0x361) == '\0') {
                  _DAT_ram_f006e84c = _DAT_ram_f006e84c + 1;
                }
                if ((((*(char *)(iVar11 + 0x14) != '\x01') ||
                     ((*(byte *)(iVar22 + 0x35) & 0x40) != 0)) ||
                    (iVar12 = FUN_ram_f002a554(1), puVar8 = _DAT_ram_f0064f50, iVar12 == 0)) ||
                   (*(char *)(iVar11 + 0x289) != '\0')) goto LAB_ram_f00383a6;
                uVar21 = _DAT_ram_f0064f50[0x26];
                puVar7 = _DAT_ram_f0064f50 + 0x27;
                _DAT_ram_f0064f50[0x26] = uVar21 + 1;
                puVar8[0x27] = *puVar7 + (uint)(uVar21 + 1 == 0);
                uVar15 = 0x2d;
              }
            }
          }
          else if (uVar10 == 3) {
            uVar15 = 0x32;
          }
          else if (uVar10 == 4) {
            iVar11 = FUN_ram_f0025d90(*(undefined1 *)(iVar22 + 0x3a));
            if (iVar11 == 0) goto LAB_ram_f003839a;
            if (((*(char *)(iVar11 + 9) == '\0') || (*(char *)(iVar11 + 0x14) != '\x02')) ||
               (((*(byte *)(iVar22 + 0x35) & 0x40) != 0 ||
                ((iVar12 = FUN_ram_f002a554(), iVar12 == 0 || (*(char *)(iVar11 + 0x289) != '\0'))))
               )) goto LAB_ram_f00383a6;
            uVar15 = 0x31;
          }
          else {
            (*_DAT_ram_00014800)
                      (_DAT_ram_f006389c,_DAT_ram_f006389c,0xf006380c,0x7e8,_DAT_ram_00014800);
            uVar15 = 0x33;
          }
        }
      }
LAB_ram_f0038130:
      FUN_ram_f00355c4(param_1,uVar15);
    }
LAB_ram_f00386de:
    param_1 = piVar13;
    if (piVar13 == (int *)0x0) {
      return;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00386fc @ 0xf00386fc =====


void FUN_ram_f00386fc(int param_1,int param_2)

{
  ushort uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  
  uVar3 = *(uint *)(param_1 + 0x40);
  uVar2 = (uVar3 & 0x7ffffff) >> 0x14;
  if ((uVar3 & 0x7ffff) >> 0x11 < 2) {
    iVar4 = -0xff9c558;
  }
  else {
    if ((uVar3 & 0x18000000) == 0x8000000) {
      if ((int)uVar3 < 0) {
        iVar4 = -0xff9c528;
      }
      else {
        iVar4 = -0xff9c538;
      }
      uVar1 = *(ushort *)(iVar4 + uVar2 * 2);
      goto LAB_ram_f003875c;
    }
    if ((int)uVar3 < 0) {
      iVar4 = -0xff9c540;
    }
    else {
      iVar4 = -0xff9c548;
    }
  }
  uVar1 = (ushort)*(byte *)(iVar4 + uVar2);
LAB_ram_f003875c:
  *(ushort *)(param_2 + 0x2d4) = uVar1;
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0xb69 <<<
// ===== FUN_ram_f0038764 @ 0xf0038764 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0038764(int param_1)

{
  if (*(int *)(param_1 + 0x28) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xb69,_DAT_ram_00014800);
  }
  *(undefined1 *)(param_1 + 0x3f) = *(undefined1 *)(_DAT_ram_0209fce8 + 0xe0);
  *(undefined1 *)(param_1 + 0x40) = *(undefined1 *)(_DAT_ram_0209fce8 + 0xe1);
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0xb81 <<<
// ===== FUN_ram_f00387ac @ 0xf00387ac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00387ac(int param_1)

{
  if (*(int *)(param_1 + 0x28) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xb81,_DAT_ram_00014800);
  }
  *(undefined1 *)(param_1 + 0x40) = 0;
  *(undefined1 *)(param_1 + 0x3f) = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00387e0 @ 0xf00387e0 =====


void FUN_ram_f00387e0(int param_1)

{
  bool bVar1;
  char cVar2;
  undefined2 *puVar3;
  byte bVar4;
  undefined1 uVar5;
  undefined2 uVar6;
  ushort uVar7;
  byte bVar8;
  ushort uVar9;
  
  puVar3 = *(undefined2 **)(param_1 + 0x28);
  uVar6 = 0;
  *puVar3 = *(undefined2 *)(param_1 + 0x2c);
  puVar3[1] = 0;
  if (*(char *)(param_1 + 0x42) != '\0') {
    uVar6 = 3;
  }
  puVar3[1] = uVar6;
  *(undefined1 *)(puVar3 + 2) = 0;
  bVar4 = *(byte *)(param_1 + 0x2e) & 3;
  *(byte *)(puVar3 + 2) = bVar4;
  bVar8 = 0;
  if (*(char *)(param_1 + 0x10) != '\0') {
    bVar8 = *(char *)(param_1 + 0x11) << 2;
  }
  *(undefined1 *)((int)puVar3 + 5) = 0;
  *(byte *)(puVar3 + 2) = bVar8 | bVar4;
  bVar1 = *(char *)(param_1 + 0x10) != '\0';
  *(bool *)((int)puVar3 + 5) = bVar1;
  if (((*(int *)(param_1 + 0x20) != 0) && (*(char *)(param_1 + 0x3d) != '\0')) &&
     (*(int *)(*(int *)(param_1 + 0x20) + (*(byte *)(param_1 + 0xf) + 0x44) * 4 + 4) != 0)) {
    *(byte *)((int)puVar3 + 5) = bVar1 | 2;
  }
  if (*(char *)(param_1 + 0x3b) == '\0') {
    bVar8 = 0;
  }
  else {
    bVar8 = 4;
  }
  bVar8 = bVar8 | *(byte *)((int)puVar3 + 5);
  *(byte *)((int)puVar3 + 5) = bVar8;
  if ((*(char *)(param_1 + 0x43) != '\0') || (bVar4 = 0, *(char *)(param_1 + 0x44) != '\0')) {
    bVar4 = 8;
  }
  bVar4 = bVar4 | bVar8;
  *(byte *)((int)puVar3 + 5) = bVar4;
  cVar2 = *(char *)(param_1 + 0xd);
  puVar3[3] = 0;
  *(byte *)((int)puVar3 + 5) = bVar4 | cVar2 << 4;
  uVar9 = *(ushort *)(param_1 + 0x30) >> 4;
  puVar3[3] = uVar9;
  uVar7 = 0;
  if (*(short *)(param_1 + 0x3c) != 0) {
    uVar7 = (*(byte *)(param_1 + 0xf) & 7) << 0xc;
  }
  uVar7 = uVar7 | uVar9;
  puVar3[3] = uVar7;
  if (*(char *)(param_1 + 0x3c) == '\0') {
    uVar9 = 0;
  }
  else {
    uVar9 = 0x8000;
  }
  puVar3[3] = uVar9 | uVar7;
  if (*(int *)(param_1 + 0x20) == 0) {
    uVar5 = 0xfe;
  }
  else {
    uVar5 = *(undefined1 *)(*(int *)(param_1 + 0x20) + 8);
  }
  *(undefined1 *)(puVar3 + 4) = uVar5;
  *(undefined1 *)((int)puVar3 + 9) = *(undefined1 *)(param_1 + 0x48);
  *(undefined1 *)(puVar3 + 5) = *(undefined1 *)(param_1 + 0x49);
  *(undefined1 *)((int)puVar3 + 0xb) = 0;
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0xc06,0xc07 <<<
// ===== FUN_ram_f00388e8 @ 0xf00388e8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00388e8(int *param_1)

{
  int iVar1;
  int *piVar2;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xc06,_DAT_ram_00014800);
  }
  if (param_1[10] == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xc07,_DAT_ram_00014800);
  }
  iVar1 = _DAT_ram_0209fce8 + (uint)*(byte *)((int)param_1 + 0x2f) * 0xc;
  piVar2 = *(int **)(iVar1 + 4);
  *(int **)(iVar1 + 4) = param_1;
  *param_1 = iVar1;
  param_1[1] = (int)piVar2;
  *piVar2 = (int)param_1;
  piVar2 = (int *)(_DAT_ram_0209fce8 + (uint)*(byte *)((int)param_1 + 0x2f) * 0xc + 8);
  *piVar2 = *piVar2 + 1;
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0xe95 <<<
// ===== FUN_ram_f0038980 @ 0xf0038980 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0038980(undefined1 param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = (*_DAT_ram_00014da4)(param_1,param_2,_DAT_ram_00014da4);
  if (iVar1 != 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xe95,_DAT_ram_00014800);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0038a10 @ 0xf0038a10 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0038a10(void)

{
  bool bVar1;
  
  if (*(int *)(_DAT_ram_0209fce8 + 8) == 0) {
    bVar1 = *(int *)(_DAT_ram_0209fce8 + 0x14) == 0;
  }
  else {
    bVar1 = false;
  }
  return bVar1;
}

// >>> MOD: wifi/mgmt/rxm.c:0xfe7 <<<
// ===== FUN_ram_f0038ad0 @ 0xf0038ad0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0038ad0(uint param_1,ushort param_2)

{
  int iVar1;
  ushort uVar2;
  ushort uVar3;
  
  uVar2 = param_2 >> 2 & 1;
  uVar3 = param_2 >> 1 & 1;
  *(byte *)(_DAT_ram_0209fce8 + 0xe0) = (byte)uVar2 | (byte)uVar3;
  param_2 = param_2 & 1;
  *(char *)(_DAT_ram_0209fce8 + 0xe1) = (char)param_2;
  iVar1 = (*_DAT_ram_000149c8)((param_1 & 7) != 0,param_2,param_2,uVar2,uVar3,_DAT_ram_000149c8);
  if (iVar1 != 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xfe7,_DAT_ram_00014800);
  }
  return iVar1 == 0;
}

// >>> MOD: wifi/mgmt/rxm.c:0x10cb,0x10cc,0x10cd <<<
// ===== FUN_ram_f0038b38 @ 0xf0038b38 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0038b38(int param_1)

{
  ushort uVar1;
  int iVar2;
  ushort *puVar3;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x10cb,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x14) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x10cc,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x18) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x10cd,_DAT_ram_00014800);
  }
  puVar3 = *(ushort **)(param_1 + 0x14);
  uVar1 = *puVar3;
  if (((uint)puVar3 & 3) == 0) {
    iVar2 = 0;
  }
  else {
    iVar2 = 2;
  }
  *(undefined1 *)(param_1 + 0x10) = 1;
  *(bool *)(param_1 + 0x3b) = (uVar1 & 0x300) == 0x300;
  *(undefined1 *)(param_1 + 0x3c) = 0;
  *(undefined1 *)(param_1 + 0x3d) = 0;
  uVar1 = puVar3[0xb];
  *(int *)(param_1 + 0x28) = (int)puVar3 + (-0xc - iVar2);
  *(ushort *)(param_1 + 0x2c) =
       (short)iVar2 + *(short *)(param_1 + 0x12) + 0xc + (ushort)*(byte *)(param_1 + 0x11);
  *(ushort *)(param_1 + 0x30) = uVar1;
  *(undefined1 *)(param_1 + 0x42) = 0;
  *(char *)(param_1 + 0x2e) = (char)iVar2;
  *(undefined1 *)(param_1 + 0x2f) = 0;
  FUN_ram_f00387e0(param_1);
  *(ushort *)(*(int *)(param_1 + 0x28) + 2) = *(ushort *)(*(int *)(param_1 + 0x28) + 2) | 4;
  FUN_ram_f00387ac(param_1);
  FUN_ram_f00388e8(param_1);
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0x1114,0x1115,0x1116,0x1117 <<<
// ===== FUN_ram_f0038c28 @ 0xf0038c28 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0038c28(int param_1)

{
  ushort uVar1;
  int iVar2;
  ushort *puVar3;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x1114,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x14) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x1115,_DAT_ram_00014800);
  }
  if (*(char *)(param_1 + 0x11) == '\0') {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x1116,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x18) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x1117,_DAT_ram_00014800);
  }
  puVar3 = *(ushort **)(param_1 + 0x14);
  uVar1 = *puVar3;
  *(undefined1 *)(param_1 + 0x10) = 1;
  *(undefined1 *)(param_1 + 0x3c) = 0;
  *(undefined1 *)(param_1 + 0x3d) = 0;
  *(bool *)(param_1 + 0x3b) = (uVar1 & 0x300) == 0x300;
  if (((uint)puVar3 & 3) == 0) {
    iVar2 = 0;
  }
  else {
    iVar2 = 2;
  }
  *(ushort *)(param_1 + 0x30) = puVar3[0xb];
  *(int *)(param_1 + 0x28) = (int)puVar3 + (-0xc - iVar2);
  *(ushort *)(param_1 + 0x2c) =
       (short)iVar2 + *(short *)(param_1 + 0x12) + 0xc + (ushort)*(byte *)(param_1 + 0x11);
  *(undefined1 *)(param_1 + 0x2f) = 0;
  *(undefined1 *)(param_1 + 0x42) = 1;
  *(char *)(param_1 + 0x2e) = (char)iVar2;
  FUN_ram_f00387e0(param_1);
  FUN_ram_f00387ac(param_1);
  FUN_ram_f00388e8(param_1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0038d30 @ 0xf0038d30 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0038d30(uint param_1)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  uVar2 = 1 << (param_1 & 0x1f);
  iVar3 = 0;
  do {
    iVar4 = iVar3 + 0x1d;
    iVar5 = _DAT_ram_0209fce8 + iVar4 * 8;
    iVar3 = iVar3 + 1;
    if ((*(char *)(_DAT_ram_0209fce8 + iVar4 * 8) != '\0') &&
       (bVar1 = *(byte *)(iVar5 + 1), (uVar2 & bVar1) != 0)) {
      *(byte *)(iVar5 + 1) = ~(byte)uVar2 & bVar1;
      if (*(char *)(_DAT_ram_0209fce8 + iVar4 * 8 + 1) == '\0') {
        *(undefined1 *)(_DAT_ram_0209fce8 + iVar4 * 8) = 0;
      }
    }
  } while (iVar3 != 0x20);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0038d88 @ 0xf0038d88 =====


void FUN_ram_f0038d88(int param_1,int param_2)

{
  char cVar1;
  int iVar2;
  
  iVar2 = 0;
  do {
    cVar1 = *(char *)(param_2 + iVar2);
    *(char *)(param_1 + iVar2) = cVar1;
    iVar2 = iVar2 + 1;
  } while (cVar1 != '\0');
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0038d98 @ 0xf0038d98 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f0038d98(uint param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar3 = 0;
  iVar4 = 0;
  do {
    iVar2 = iVar4 + 0x1d;
    iVar1 = _DAT_ram_0209fce8 + iVar2 * 8;
    iVar4 = iVar4 + 1;
    if ((*(char *)(_DAT_ram_0209fce8 + iVar2 * 8) != '\0') &&
       ((1 << (param_1 & 0x1f) & (uint)*(byte *)(iVar1 + 1)) != 0)) {
      (*(code *)&SUB_ram_0006a08c)(param_2 + 8 + iVar3 * 6,iVar1 + 2,6);
      iVar3 = iVar3 + 1;
    }
  } while (iVar4 != 0x20);
  return iVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0038dfc @ 0xf0038dfc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0038dfc(undefined1 param_1)

{
  int iVar1;
  
  FUN_ram_f0038d30(param_1);
  iVar1 = 0;
  FUN_ram_f004be5c();
  do {
    if (*(char *)(_DAT_ram_0209fce8 + (iVar1 + 0x1d) * 8) != '\0') {
      (*_DAT_ram_00014ad8)(iVar1,_DAT_ram_0209fce8 + (iVar1 + 0x1d) * 8 + 2,_DAT_ram_00014ad8);
    }
    iVar1 = iVar1 + 1;
  } while (iVar1 != 0x20);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0038e4c @ 0xf0038e4c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f0038e4c(byte param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  
  uVar3 = (uint)param_1;
  iVar1 = 0;
  while( true ) {
    iVar2 = iVar1 + 0x1d;
    iVar4 = iVar2 * 8;
    iVar5 = _DAT_ram_0209fce8 + iVar4;
    if (*(char *)(_DAT_ram_0209fce8 + iVar2 * 8) == '\0') {
      (*(code *)&SUB_ram_0006a08c)(_DAT_ram_0209fce8 + iVar4 + 2,param_2,6);
      *(byte *)(_DAT_ram_0209fce8 + iVar4 + 1) =
           (byte)(1 << (uVar3 & 0x1f)) | *(byte *)(_DAT_ram_0209fce8 + iVar4 + 1);
      *(undefined1 *)(_DAT_ram_0209fce8 + iVar2 * 8) = 1;
      goto LAB_ram_f0038eee;
    }
    iVar2 = (*(code *)&SUB_ram_0006a074)(param_2,iVar5 + 2,6);
    if (iVar2 == 0) break;
    iVar1 = iVar1 + 1;
    if (iVar1 == 0x20) {
LAB_ram_f0038eee:
      return iVar1 != 0x20;
    }
  }
  if ((*(byte *)(iVar5 + 1) >> (uVar3 & 0x1f) & 1) == 0) {
    *(byte *)(iVar5 + 1) = *(byte *)(iVar5 + 1) | (byte)(1 << (uVar3 & 0x1f));
  }
  goto LAB_ram_f0038eee;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0038f00 @ 0xf0038f00 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0038f00(void)

{
  *(undefined4 *)(_DAT_ram_0209fce8 + 0xd8) = 0;
  *(undefined4 *)(_DAT_ram_0209fce8 + 0xdc) = 0;
  (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_0209fce8 + 0xe8,0,0x100);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0038f3c @ 0xf0038f3c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0038f3c(void)

{
  undefined4 uVar1;
  
  *(undefined1 *)(_DAT_ram_0209fce8 + 0x18) = 0x10;
  *(undefined1 *)(_DAT_ram_0209fce8 + 0x19) = 2;
  *(int *)_DAT_ram_0209fce8 = _DAT_ram_0209fce8;
  *(int *)(_DAT_ram_0209fce8 + 4) = _DAT_ram_0209fce8;
  *(undefined4 *)(_DAT_ram_0209fce8 + 8) = 0;
  *(int *)(_DAT_ram_0209fce8 + 0xc) = _DAT_ram_0209fce8 + 0xc;
  *(int *)(_DAT_ram_0209fce8 + 0x10) = _DAT_ram_0209fce8 + 0xc;
  *(undefined4 *)(_DAT_ram_0209fce8 + 0x14) = 0;
  *(undefined4 *)(_DAT_ram_0209fce8 + 0x1c) = 0;
  *(int *)(_DAT_ram_0209fce8 + 0x20) = _DAT_ram_0209fce8 + 0x20;
  *(int *)(_DAT_ram_0209fce8 + 0x24) = _DAT_ram_0209fce8 + 0x20;
  *(undefined4 *)(_DAT_ram_0209fce8 + 0x28) = 0;
  FUN_ram_f004be5c();
  (*_DAT_ram_00014ab8)(_DAT_ram_00014ab8);
  uVar1 = 0xf1b;
  if ((DAT_ram_f0064fd4 ^ 1) == 0) {
    uVar1 = 0xf0b;
  }
  (*_DAT_ram_00014af0)(uVar1,0xf0b,DAT_ram_f0064fd4 ^ 1,_DAT_ram_00014af0);
  (*_DAT_ram_00014a4c)(1,_DAT_ram_00014a4c);
  (*_DAT_ram_00014a50)(1,_DAT_ram_00014a50);
  (*_DAT_ram_00014ab0)(1,_DAT_ram_00014ab0);
  (*_DAT_ram_00014ad4)(0x186,_DAT_ram_00014ad4);
  FUN_ram_f0038f00();
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0039018 @ 0xf0039018 =====


int FUN_ram_f0039018(undefined4 param_1,int param_2)

{
  int iVar1;
  int iVar2;
  undefined8 uVar3;
  
  iVar2 = 0;
  do {
    iVar1 = FUN_ram_f0025d90(iVar2,param_2);
    param_2 = iVar1 + 10;
    iVar2 = iVar2 + 1;
    if (*(char *)(iVar1 + 9) != '\0') {
      uVar3 = (*(code *)&SUB_ram_0006a074)(param_1,param_2,6);
      param_2 = (int)((ulonglong)uVar3 >> 0x20);
      if ((int)uVar3 == 0) {
        return iVar1;
      }
    }
  } while (iVar2 != 0x14);
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0039060 @ 0xf0039060 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0039060(uint param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  
  uVar1 = (*_DAT_ram_00014830)(param_1,_DAT_ram_00014830);
  iVar5 = 0;
  do {
    iVar2 = FUN_ram_f0025d90(iVar5);
    if (((*(char *)(iVar2 + 9) != '\0') && (*(char *)(iVar2 + 0x15) == '\x02')) &&
       (*(byte *)(iVar2 + 0x14) == param_1)) {
      uVar4 = *(uint *)(iVar2 + 0x2d0);
      uVar3 = ~uVar4;
      if (uVar4 <= uVar1) {
        uVar3 = -uVar4;
      }
      if (0x1f8000 < uVar1 + uVar3) {
        FUN_ram_f00228c0(iVar2,0,&LAB_ram_f0039968);
        *(uint *)(iVar2 + 0x2d0) = uVar1;
        return;
      }
    }
    iVar5 = iVar5 + 1;
    if (iVar5 == 0x14) {
      return;
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/rxm.c:0x1156 <<<
// ===== FUN_ram_f00390e4 @ 0xf00390e4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00390e4(int param_1)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  short sVar4;
  ushort *puVar5;
  int iVar6;
  int iVar7;
  char *local_20;
  ushort local_1a [3];
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x1156,_DAT_ram_00014800);
  }
  iVar2 = _DAT_ram_020a0064;
  iVar6 = *(int *)(param_1 + 0x14);
  iVar7 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4);
  if ((((*(char *)(iVar6 + 0x19) == 'P') && (*(char *)(iVar6 + 0x1a) == 'o')) &&
      (*(char *)(iVar6 + 0x1b) == -0x66)) && (*(char *)(iVar6 + 0x1c) == '\t')) {
    cVar1 = *(char *)(iVar6 + 0x1d);
    sVar4 = *(short *)(param_1 + 0x12) + -7;
    if (cVar1 == '\x01') {
      iVar2 = FUN_ram_f002a228(iVar6 + 0x1f,sVar4,&local_20,local_1a);
      if (iVar2 == 0) {
        for (uVar3 = 0; uVar3 < local_1a[0]; uVar3 = uVar3 + *puVar5 + 3) {
          puVar5 = (ushort *)(local_20 + 1);
          if (*local_20 == '\f') {
            FUN_ram_f002ce58(param_1,local_20,*(undefined1 *)(iVar6 + 0x1e));
            return;
          }
          local_20 = local_20 + *puVar5 + 3;
        }
      }
    }
    else if (cVar1 == '\0') {
      iVar2 = FUN_ram_f002a228(iVar6 + 0x1f,sVar4,&local_20,local_1a);
      if (iVar2 == 0) {
        for (uVar3 = 0; uVar3 < local_1a[0]; uVar3 = uVar3 + *puVar5 + 3) {
          puVar5 = (ushort *)(local_20 + 1);
          if (*local_20 == '\f') {
            FUN_ram_f002d318(*(undefined1 *)(iVar7 + 0x19));
            return;
          }
          local_20 = local_20 + *puVar5 + 3;
        }
      }
    }
    else if (((cVar1 == '\x03') && (*(int *)(_DAT_ram_020a0064 + 0x1304) == 0)) &&
            (*(int *)(_DAT_ram_020a0064 + 0x12f8) == 0)) {
      *(undefined1 *)(_DAT_ram_020a0064 + 0x1aa0) = 1;
      FUN_ram_f002672c(iVar2 + 0x1a88,0x67);
      FUN_ram_f0023d9c(1,0x19000);
    }
  }
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0xea9 <<<
// ===== FUN_ram_f0039230 @ 0xf0039230 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0039230(int param_1)

{
  int iVar1;
  undefined1 *puVar2;
  undefined2 *puVar3;
  int *piVar4;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xea9,_DAT_ram_00014800);
  }
  iVar1 = 0;
  puVar3 = (undefined2 *)(param_1 + 0xe0);
  puVar2 = (undefined1 *)(param_1 + 0x102);
  do {
    iVar1 = iVar1 + 1;
    *puVar3 = 0xffff;
    puVar3 = puVar3 + 1;
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  } while (iVar1 != 0x11);
  iVar1 = 0;
  piVar4 = (int *)(param_1 + 0x1e0);
  do {
    iVar1 = iVar1 + 1;
    if (*piVar4 != 0) {
      (*(code *)&SUB_ram_0006a0d8)(*piVar4);
      *piVar4 = 0;
    }
    piVar4 = piVar4 + 4;
  } while (iVar1 != 3);
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0x9c6,0x9c7,0x9c8,0x9c9 <<<
// ===== FUN_ram_f003929c @ 0xf003929c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003929c(int param_1)

{
  uint uVar1;
  int iVar2;
  code *pcVar3;
  undefined4 uVar4;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x9c6,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x14) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x9c7,_DAT_ram_00014800);
  }
  if (*(char *)(param_1 + 0x11) == '\0') {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x9c8,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x18) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x9c9,_DAT_ram_00014800);
  }
  uVar1 = (uint)(**(byte **)(param_1 + 0x14) >> 4);
  if ((((*_DAT_ram_020a0068 == 0) && (*(char *)(_DAT_ram_020a0064 + 0x1f74) == '\x01')) &&
      (*(int *)(uVar1 * 4 + -0xff9c704) == 0)) && ((uVar1 != 10 && (uVar1 != 0xc)))) {
    (*(code *)&SUB_ram_0006a0d8)(param_1);
    return;
  }
  pcVar3 = *(code **)(uVar1 * 4 + -0xff9c704);
  if (pcVar3 == (code *)0x0) {
    if (*(char *)(param_1 + 0x11) != '\0') goto LAB_ram_f00393aa;
    uVar4 = 0x9fc;
  }
  else {
    iVar2 = (*pcVar3)(param_1);
    if (iVar2 == 1) {
      return;
    }
    if (*(char *)(param_1 + 0x11) != '\0') goto LAB_ram_f00393aa;
    uVar4 = 0x9f1;
  }
  (*_DAT_ram_00014800)(0,0,0xf00638a4,uVar4,_DAT_ram_00014800);
LAB_ram_f00393aa:
  FUN_ram_f0038c28(param_1);
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0x41c,0x479 <<<
// ===== FUN_ram_f00393bc @ 0xf00393bc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f00393bc(int param_1,undefined4 param_2,ushort param_3)

{
  ushort uVar1;
  int iVar2;
  ushort *puVar3;
  int iVar4;
  int iVar5;
  short *psVar6;
  ushort uVar7;
  ushort uVar8;
  int *piVar9;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0x41c,_DAT_ram_00014800);
  }
  uVar1 = *(ushort *)(param_1 + 0x30);
  uVar7 = param_3 & 0x400;
  uVar8 = uVar1 & 0xf;
  if ((param_3 & 0x400) == 0) {
    if ((uVar1 & 0xf) == 0) {
      return param_1;
    }
    uVar8 = 1;
  }
  else if ((uVar1 & 0xf) == 0) {
    uVar7 = 1;
  }
  else {
    uVar8 = 0;
    uVar7 = 0;
  }
  iVar4 = 0;
  iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  do {
    piVar9 = (int *)((iVar4 + 0x1e) * 0x10 + *(int *)(param_1 + 0x20));
    iVar4 = iVar4 + 1;
    if ((*piVar9 != 0) && (-1 < iVar2 - piVar9[1])) {
      (*(code *)&SUB_ram_0006a0d8)(*piVar9);
      *piVar9 = 0;
    }
  } while (iVar4 != 3);
  iVar4 = *(int *)(param_1 + 0x20);
  iVar2 = 0;
  piVar9 = (int *)(iVar4 + 0x1e0);
  puVar3 = (ushort *)(iVar4 + 0x1d8);
  do {
    iVar5 = iVar2 * 0x10;
    iVar2 = iVar2 + 1;
    psVar6 = (short *)(iVar4 + iVar5 + 0x1d8);
    iVar5 = *piVar9;
    if (uVar7 == 0) {
      if (iVar5 != 0) {
        if (*(char *)(param_1 + 0x3d) == '\0') {
          if ((*(ushort *)(iVar5 + 0x32) & 0xfc) != 0x88) goto LAB_ram_f0039496;
        }
        else if ((*(ushort *)(iVar5 + 0x32) & 0xfc) == 0x88) {
LAB_ram_f0039496:
          if (*puVar3 == uVar1) {
LAB_ram_f00394a6:
            if (psVar6 == (short *)0x0) {
              (*_DAT_ram_00014800)(0,0,0xf00638a4,0x479,_DAT_ram_00014800);
            }
            if (uVar7 != 0) {
              iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
              *(int *)(psVar6 + 6) = iVar2 + 0x218f6;
              *(int *)(psVar6 + 4) = param_1;
              iVar2 = *(int *)(param_1 + 0x18);
              uVar7 = *(ushort *)(param_1 + 0x12);
              *psVar6 = uVar1 + 1;
              *(uint *)(psVar6 + 2) = iVar2 + (uint)uVar7;
              return 0;
            }
            *(short *)(*(int *)(psVar6 + 4) + 0x12) =
                 *(short *)(param_1 + 0x12) + *(short *)(*(int *)(psVar6 + 4) + 0x12);
            if (0x800 < *(ushort *)(*(int *)(psVar6 + 4) + 0x12)) {
              (*(code *)&SUB_ram_0006a0d8)();
              psVar6[4] = 0;
              psVar6[5] = 0;
              (*(code *)&SUB_ram_0006a0d8)(param_1);
              return 0;
            }
            (*(code *)&SUB_ram_0006a08c)
                      (*(undefined4 *)(psVar6 + 2),*(undefined4 *)(param_1 + 0x18),
                       *(undefined2 *)(param_1 + 0x12));
            (*(code *)&SUB_ram_0006a0d8)(param_1);
            if (uVar8 == 0) {
              uVar1 = *(ushort *)(param_1 + 0x12);
              *psVar6 = *psVar6 + 1;
              *(uint *)(psVar6 + 2) = *(int *)(psVar6 + 2) + (uint)uVar1;
              return 0;
            }
            iVar2 = *(int *)(psVar6 + 4);
            psVar6[4] = 0;
            psVar6[5] = 0;
            return iVar2;
          }
        }
      }
    }
    else if (iVar5 == 0) goto LAB_ram_f00394a6;
    piVar9 = piVar9 + 4;
    puVar3 = puVar3 + 8;
    if (iVar2 == 3) {
      (*(code *)&SUB_ram_0006a0d8)(param_1);
      return 0;
    }
  } while( true );
}

// >>> MOD: wifi/mgmt/rxm.c:0xc6e,0xc6f <<<
// ===== FUN_ram_f0039640 @ 0xf0039640 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_ram_f0039640(int param_1)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  short sVar5;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xc6e,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x14) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xc6f,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x20) == 0) {
    return 0;
  }
  if (*(char *)(param_1 + 0x47) == '\0') {
    sVar5 = *(short *)(*(int *)(param_1 + 0x14) + 0x16);
  }
  else {
    sVar5 = 0;
  }
  *(short *)(param_1 + 0x30) = sVar5;
  if ((*(ushort *)(param_1 + 0x32) & 0xfc) == 0x88) {
    if (*(int *)(*(int *)(param_1 + 0x20) + (*(byte *)(param_1 + 0xf) + 0x44) * 4 + 4) != 0) {
      *(short *)(*(int *)(param_1 + 0x20) + (*(byte *)(param_1 + 0xf) + 0x70) * 2) = sVar5;
      return 0;
    }
    iVar2 = FUN_ram_f0028750();
    uVar3 = (uint)*(byte *)(param_1 + 0xf);
    if (iVar2 == 0) {
      if (7 < uVar3) {
        return 1;
      }
      uVar3 = (uint)*(byte *)(uVar3 + 0xf00648fc);
    }
  }
  else {
    uVar3 = 0x10;
  }
  uVar1 = *(ushort *)(param_1 + 0x32);
  iVar2 = *(int *)(param_1 + 0x20);
  iVar4 = uVar3 + 0x70;
  if ((uVar1 & 0x800) != 0) {
    if (*(short *)(iVar2 + iVar4 * 2) == sVar5) {
      if (*(char *)(iVar2 + uVar3 + 0x102) == '\0') {
        return 1;
      }
      if (*(char *)(param_1 + 0x3e) == '\x01') {
        *(undefined1 *)(iVar2 + uVar3 + 0x102) = 0;
      }
    }
    else {
      *(short *)(iVar2 + iVar4 * 2) = sVar5;
      if (*(char *)(param_1 + 0x3e) == '\x03') {
        *(undefined1 *)(*(int *)(param_1 + 0x20) + uVar3 + 0x102) = 1;
      }
    }
    return 0;
  }
  *(short *)(iVar2 + iVar4 * 2) = sVar5;
  *(undefined1 *)(*(int *)(param_1 + 0x20) + uVar3 + 0x102) = 0;
  return uVar1 & 0x800;
}

// >>> MOD: wifi/mgmt/rxm.c:0xdf4,0xdf5,0xdf7 <<<
// ===== FUN_ram_f0039750 @ 0xf0039750 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0039750(int *param_1)

{
  int iVar1;
  int *piVar2;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xdf4,_DAT_ram_00014800);
  }
  if (param_1[10] == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xdf5,_DAT_ram_00014800);
  }
  if (1 < *(byte *)((int)param_1 + 0x2f)) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xdf7,_DAT_ram_00014800);
  }
  iVar1 = _DAT_ram_0209fce8 + (uint)*(byte *)((int)param_1 + 0x2f) * 0xc;
  piVar2 = *(int **)(iVar1 + 4);
  *(int **)(iVar1 + 4) = param_1;
  param_1[1] = (int)piVar2;
  *param_1 = iVar1;
  *piVar2 = (int)param_1;
  piVar2 = (int *)(_DAT_ram_0209fce8 + (uint)*(byte *)((int)param_1 + 0x2f) * 0xc + 8);
  *piVar2 = *piVar2 + 1;
  FUN_ram_f00387ac(param_1);
  (*(code *)&SUB_ram_0006a854)(*(undefined1 *)((int)param_1 + 0x2f));
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0039800 @ 0xf0039800 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0039800(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 *puVar3;
  
  iVar1 = FUN_ram_f0025c48(0x3d0);
  if (iVar1 == 0) {
    (*_DAT_ram_00014814)(2,0xf00638b4,_DAT_ram_00014814);
    uVar2 = 0xc000009a;
  }
  else {
    puVar3 = *(undefined2 **)(iVar1 + 8);
    (*(code *)&SUB_ram_0006a08c)(puVar3 + 4,0xf006e0f0,0x3c8);
    *(undefined1 *)(puVar3 + 2) = 0xfe;
    *(undefined1 *)((int)puVar3 + 5) = 0;
    *puVar3 = 0x3d0;
    *(undefined1 *)(iVar1 + 0x2f) = 1;
    *(undefined2 **)(iVar1 + 0x28) = puVar3;
    puVar3[1] = 1;
    FUN_ram_f0039750(iVar1);
    uVar2 = 0;
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0039884 @ 0xf0039884 =====


undefined4
FUN_ram_f0039884(undefined2 param_1,undefined4 param_2,undefined4 param_3,undefined2 param_4)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 *puVar3;
  
  iVar1 = FUN_ram_f0025c48(0x20);
  if (iVar1 == 0) {
    uVar2 = 0xc000009a;
  }
  else {
    puVar3 = *(undefined2 **)(iVar1 + 8);
    *(undefined1 *)(puVar3 + 2) = 0x23;
    *(undefined1 *)((int)puVar3 + 5) = 0;
    *puVar3 = 0x20;
    puVar3[4] = param_1;
    (*(code *)&SUB_ram_0006a08c)(puVar3 + 6,param_2,6);
    (*(code *)&SUB_ram_0006a08c)(puVar3 + 9,param_3,6);
    puVar3[0xf] = param_4;
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar3;
    puVar3[1] = 1;
    FUN_ram_f0039750(iVar1);
    uVar2 = 0;
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0039914 @ 0xf0039914 =====


undefined4 FUN_ram_f0039914(undefined1 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 *puVar3;
  
  iVar1 = FUN_ram_f0025c48(0xc);
  if (iVar1 == 0) {
    uVar2 = 0xc000009a;
  }
  else {
    puVar3 = *(undefined2 **)(iVar1 + 8);
    *(undefined1 *)(puVar3 + 2) = 0x21;
    *(undefined1 *)((int)puVar3 + 5) = 0;
    *puVar3 = 0xc;
    *(undefined1 *)(puVar3 + 4) = param_1;
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar3;
    puVar3[1] = 1;
    FUN_ram_f0039750();
    uVar2 = 0;
  }
  return uVar2;
}

// >>> MOD: wifi/mgmt/rxm.c:0xa4a,0xa4b,0xa4c,0xa4d,0xa7a <<<
// ===== FUN_ram_f0039b78 @ 0xf0039b78 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0039b78(int param_1)

{
  ushort uVar1;
  int *piVar2;
  bool bVar3;
  int iVar4;
  undefined1 uVar5;
  int iVar6;
  uint uVar7;
  short sVar8;
  uint uVar9;
  undefined1 auStack_30 [8];
  undefined1 auStack_28 [12];
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xa4a,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x14) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xa4b,_DAT_ram_00014800);
  }
  if (*(char *)(param_1 + 0x11) == '\0') {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xa4c,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x18) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xa4d,_DAT_ram_00014800);
  }
  uVar1 = *(ushort *)(param_1 + 0x32);
  uVar9 = *(uint *)(param_1 + 0x14);
  if (*(char *)(param_1 + 0x47) != '\0') {
    if ((uVar9 & 3) == 0) {
      iVar6 = 0;
    }
    else {
      iVar6 = 2;
    }
    *(uint *)(param_1 + 0x28) = (uVar9 - 0xc) - iVar6;
    *(ushort *)(param_1 + 0x2c) =
         (short)iVar6 + *(short *)(param_1 + 0x12) + 0xc + (ushort)*(byte *)(param_1 + 0x11);
    *(undefined1 *)(param_1 + 0x42) = 0;
    *(char *)(param_1 + 0x41) = (char)iVar6 + '\x18';
    *(undefined1 *)(param_1 + 0x10) = 0;
    *(undefined1 *)(param_1 + 0x3c) = 0;
    *(undefined1 *)(param_1 + 0x3b) = 0;
    *(char *)(param_1 + 0x2e) = (char)iVar6;
    *(undefined1 *)(param_1 + 0x2f) = 0;
    goto LAB_ram_f0039f42;
  }
  iVar6 = *(int *)(param_1 + 0x18);
  if (*(int *)(param_1 + 0x20) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xa7a,_DAT_ram_00014800);
  }
  uVar7 = *(uint *)(*(int *)(param_1 + 0x20) + 0x10);
  if (uVar7 != 0x24) {
    if (uVar7 < 0x25) {
      if (uVar7 != 0x11) {
        if (uVar7 < 0x11) goto LAB_ram_f0039eb2;
        uVar7 = uVar7 - 0x21;
LAB_ram_f0039c9e:
        if (1 < uVar7) goto LAB_ram_f0039eb2;
      }
    }
    else if (uVar7 != 0x44) {
      if (uVar7 < 0x45) {
        uVar7 = uVar7 - 0x41;
        goto LAB_ram_f0039c9e;
      }
      if (uVar7 != 0x81) goto LAB_ram_f0039eb2;
    }
  }
  if (*(char *)(param_1 + 0x3e) != '\0') {
    iVar6 = *(int *)(param_1 + 0x18) + 2;
    *(int *)(param_1 + 0x18) = iVar6;
    (*(code *)&SUB_ram_0006a08c)(auStack_28,iVar6,6);
    (*(code *)&SUB_ram_0006a08c)(auStack_30,*(int *)(param_1 + 0x18) + 6,6);
    iVar6 = *(int *)(param_1 + 0x18) + 0xe;
    *(short *)(param_1 + 0x12) = *(short *)(param_1 + 0x12) + -0xe;
    *(int *)(param_1 + 0x18) = iVar6;
  }
  sVar8 = *(short *)(iVar6 + 6);
  bVar3 = sVar8 == -0x7178 || sVar8 == -0x4b78;
  iVar4 = (*(code *)&SUB_ram_0006a074)(iVar6,0xf0064948,6);
  if ((iVar4 == 0) ||
     (((iVar4 = (*(code *)&SUB_ram_0006a074)(iVar6,0xf0064950,6,&SUB_ram_0006a074), iVar4 == 0 &&
       (sVar8 != 0x3781)) && (sVar8 != -0xc80)))) {
    iVar6 = FUN_ram_f0040780(*(undefined4 *)(param_1 + 0x14),*(undefined4 *)(param_1 + 0x18),
                             *(undefined2 *)(param_1 + 0x12),*(undefined4 *)(param_1 + 0x20));
    if (iVar6 == 0) {
      (*(code *)&SUB_ram_0006a0d8)(param_1);
      return;
    }
    sVar8 = *(short *)(param_1 + 0x12) + 6;
    uVar7 = *(int *)(param_1 + 0x18) - 6;
    uVar5 = 0;
  }
  else {
    iVar6 = (*(code *)&SUB_ram_0006a074)(iVar6,0xf0064958,6);
    if (iVar6 == 0) {
      if ((sVar8 != 0x100) && (sVar8 != 0x200)) {
        if (sVar8 == 0x300) {
          bVar3 = true;
          uVar5 = 1;
          goto LAB_ram_f0039db4;
        }
        if ((sVar8 != 0x400) && (sVar8 != 0x500)) {
LAB_ram_f0039eb2:
          (*(code *)&SUB_ram_0006a0d8)(param_1);
          iVar6 = _DAT_ram_0209fce8;
          piVar2 = (int *)(_DAT_ram_0209fce8 + 0x6c);
          iVar4 = *(int *)(_DAT_ram_0209fce8 + 0x68) + 1;
          *(int *)(_DAT_ram_0209fce8 + 0x68) = iVar4;
          *(uint *)(iVar6 + 0x6c) = *piVar2 + (uint)(iVar4 == 0);
          iVar6 = _DAT_ram_0209fce8;
          piVar2 = (int *)(_DAT_ram_0209fce8 + 0xbc);
          iVar4 = *(int *)(_DAT_ram_0209fce8 + 0xb8) + 1;
          *(int *)(_DAT_ram_0209fce8 + 0xb8) = iVar4;
          *(uint *)(iVar6 + 0xbc) = *piVar2 + (uint)(iVar4 == 0);
          return;
        }
      }
      uVar5 = 1;
    }
    else {
      uVar5 = 0;
    }
LAB_ram_f0039db4:
    uVar7 = *(int *)(param_1 + 0x18) - 0xe;
    *(ushort *)(*(int *)(param_1 + 0x18) + -2) =
         *(ushort *)(param_1 + 0x12) << 8 | *(ushort *)(param_1 + 0x12) >> 8;
    sVar8 = *(short *)(param_1 + 0x12) + 0xe;
  }
  *(undefined1 *)(param_1 + 0x3b) = uVar5;
  *(undefined1 *)(param_1 + 0x10) = 0;
  *(undefined1 *)(param_1 + 0x3c) = 0;
  if ((uVar7 & 3) == 0) {
    iVar6 = 0;
  }
  else {
    iVar6 = 2;
  }
  *(undefined2 *)(param_1 + 0x30) = *(undefined2 *)(uVar9 + 0x16);
  *(uint *)(param_1 + 0x28) = (uVar7 - 0xc) - iVar6;
  *(short *)(param_1 + 0x2c) = (short)iVar6 + sVar8 + 0xc;
  *(undefined1 *)(param_1 + 0x42) = 0;
  *(char *)(param_1 + 0x2e) = (char)iVar6;
  *(undefined1 *)(param_1 + 0x2f) = 0;
  if (*(char *)(param_1 + 0x3e) == '\0') {
    if ((uVar1 & 0x200) == 0) {
      iVar6 = uVar9 + 10;
      if ((uVar1 & 0x100) == 0) {
        iVar4 = uVar9 + 4;
      }
      else {
        iVar4 = uVar9 + 0x10;
      }
      (*(code *)&SUB_ram_0006a08c)(auStack_28,iVar4,6);
    }
    else if ((uVar1 & 0x100) == 0) {
      (*(code *)&SUB_ram_0006a08c)(auStack_28,uVar9 + 4,6);
      iVar6 = uVar9 + 0x10;
    }
    else {
      (*(code *)&SUB_ram_0006a08c)(auStack_28,uVar9 + 4,6);
      iVar6 = uVar9 + 10;
    }
    (*(code *)&SUB_ram_0006a08c)(auStack_30,iVar6,6);
  }
  (*(code *)&SUB_ram_0006a08c)(uVar7,auStack_28,6);
  (*(code *)&SUB_ram_0006a08c)(uVar7 + 6,auStack_30,6);
  *(char *)(param_1 + 0x41) = *(char *)(param_1 + 0x2e) + '\x18';
  if (((!bVar3) && (iVar6 = FUN_ram_f002a554(*(undefined1 *)(param_1 + 0xd)), iVar6 != 0)) &&
     ((*(byte *)(*(int *)(param_1 + 8) + 0x11) & 0xf) == 0)) {
    (*(code *)&SUB_ram_0006a0d8)(param_1);
    iVar6 = _DAT_ram_0209fce8;
    piVar2 = (int *)(_DAT_ram_0209fce8 + 0x6c);
    iVar4 = *(int *)(_DAT_ram_0209fce8 + 0x68) + 1;
    *(int *)(_DAT_ram_0209fce8 + 0x68) = iVar4;
    *(uint *)(iVar6 + 0x6c) = *piVar2 + (uint)(iVar4 == 0);
    return;
  }
LAB_ram_f0039f42:
  FUN_ram_f00387e0(param_1);
  FUN_ram_f0038764(param_1);
  FUN_ram_f00388e8(param_1);
  return;
}

// >>> MOD: wifi/mgmt/rxm.c:0xa13,0xa14 <<<
// ===== FUN_ram_f0039f70 @ 0xf0039f70 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0039f70(int param_1)

{
  ushort uVar1;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xa13,_DAT_ram_00014800);
  }
  if (*(int *)(param_1 + 0x14) == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00638a4,0xa14,_DAT_ram_00014800);
  }
  uVar1 = **(ushort **)(param_1 + 0x14) & 0xfc;
  if (uVar1 == 0x84) {
    FUN_ram_f0029fd8(param_1);
  }
  else {
    if (uVar1 == 0xa4) {
      FUN_ram_f002fcb8(param_1);
    }
    (*(code *)&SUB_ram_0006a0d8)(param_1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003aadc @ 0xf003aadc =====


byte FUN_ram_f003aadc(int param_1,byte param_2)

{
  if ((param_1 != 0x24ba58) && (param_2 < 0xf)) {
    param_2 = param_2 - 0xf;
  }
  return param_2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003aaf8 @ 0xf003aaf8 =====


byte FUN_ram_f003aaf8(byte param_1)

{
  if (param_1 < 0xf2) {
    if (param_1 < 0xe4) {
      if (0xd5 < param_1) {
        param_1 = param_1 + 0x2b;
      }
    }
    else {
      param_1 = param_1 + 0x1d;
    }
  }
  else {
    param_1 = param_1 + 0xf;
  }
  return param_1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ab1c @ 0xf003ab1c =====


undefined4 FUN_ram_f003ab1c(byte param_1)

{
  undefined4 uVar1;
  
  if ((param_1 < 0xf) || ((byte)(param_1 + 0x2a) < 0x1c)) {
    uVar1 = 0x24ba58;
  }
  else {
    uVar1 = 5000000;
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ab40 @ 0xf003ab40 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f003ab40(void)

{
  return _DAT_ram_f006514c == 0x6625;
}

// >>> MOD: wifi/mgmt/rlm.c:0x68d <<<
// ===== FUN_ram_f003ab54 @ 0xf003ab54 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f003ab54(int *param_1)

{
  bool bVar1;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,0x68d,_DAT_ram_00014800);
  }
  bVar1 = false;
  if ((char)param_1[6] != '\0') {
    if (((*param_1 == 0) || (param_1[3] == 2)) || (param_1[3] == 1)) {
      bVar1 = true;
    }
    else {
      bVar1 = *(char *)((int)param_1 + 0x19) == '\x02';
    }
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003aba0 @ 0xf003aba0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003aba0(int param_1)

{
  byte bVar1;
  uint uVar2;
  byte bVar3;
  short sVar4;
  byte *pbVar5;
  
  uVar2 = _DAT_ram_60120080;
  if (param_1 != 0) {
    pbVar5 = (byte *)(param_1 + 9);
    bVar3 = 0;
    do {
      if (*(byte *)(param_1 + 8) <= bVar3) {
        sVar4 = 0x14;
        goto LAB_ram_f003abda;
      }
      bVar1 = *pbVar5;
      pbVar5 = pbVar5 + 1;
      bVar3 = bVar3 + 1;
    } while (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)bVar1 * 4) + 0x83) == '\0');
  }
  sVar4 = 9;
LAB_ram_f003abda:
  if (_DAT_ram_f006506c != sVar4) {
    _DAT_ram_60120080 = _DAT_ram_60120080 | 0x100;
    _DAT_ram_f006506c = sVar4;
    (*_DAT_ram_00014dd0)(0,sVar4,_DAT_ram_00014dd0);
    _DAT_ram_60120080 = uVar2 & 0x300 | _DAT_ram_60120080 & 0xfffffcff;
  }
  return;
}

// >>> MOD: wifi/mgmt/rlm.c:0x47b <<<
// ===== FUN_ram_f003ac24 @ 0xf003ac24 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003ac24(int param_1,int param_2)

{
  bool bVar1;
  byte bVar2;
  byte bVar3;
  bool bVar4;
  int iVar5;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,0x47b,_DAT_ram_00014800);
  }
  iVar5 = FUN_ram_f003ab54(param_2);
  if (iVar5 != 0) {
    iVar5 = 5000000;
    if (*(char *)(param_1 + 1) != '\x02') {
      iVar5 = 0x24ba58;
    }
    if (*(int *)(param_2 + 0x2c8) == iVar5) {
      bVar1 = *(char *)(param_1 + 2) != *(char *)(param_2 + 0x2cc);
    }
    else {
      bVar1 = true;
    }
    bVar2 = *(byte *)(param_1 + 0xc);
    *(int *)(param_2 + 0x2c8) = iVar5;
    bVar3 = *(byte *)(param_2 + 0x2cd);
    *(undefined1 *)(param_2 + 0x2cc) = *(undefined1 *)(param_1 + 2);
    *(uint *)(param_2 + 0x324) = (uint)*(byte *)(param_1 + 3);
    *(undefined1 *)(param_2 + 0x2d2) = *(undefined1 *)(param_1 + 4);
    *(uint *)(param_2 + 0x2d4) = (uint)*(byte *)(param_1 + 5);
    *(uint *)(param_2 + 0x2d8) = (uint)*(byte *)(param_1 + 6);
    *(uint *)(param_2 + 0x2dc) = (uint)*(byte *)(param_1 + 7);
    *(undefined1 *)(param_2 + 0x2cd) = *(undefined1 *)(param_1 + 0xc);
    if (((*(char *)(param_1 + 0xf) == 'r') && (*(byte *)(param_1 + 0xd) < 2)) &&
       (*(byte *)(param_1 + 0xe) < 2)) {
      *(byte *)(param_2 + 0x83) = *(byte *)(param_1 + 0xe);
      bVar4 = true;
    }
    else {
      *(undefined1 *)(param_2 + 0x83) = 1;
      *(undefined1 *)(param_2 + 0x82) = 1;
      bVar4 = false;
    }
    if (bVar1) {
      FUN_ram_f0025198(param_2,0);
    }
    FUN_ram_f0025198(param_2,1);
    if (((bVar2 ^ bVar3) & 4) != 0) {
      FUN_ram_f003dab8(*(undefined1 *)(param_2 + 0x19));
    }
    if ((bVar4) && (*(char *)(param_2 + 0x82) != *(char *)(param_1 + 0xd))) {
      *(char *)(param_2 + 0x82) = *(char *)(param_1 + 0xd);
      FUN_ram_f003e03c();
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ad98 @ 0xf003ad98 =====


void FUN_ram_f003ad98(int param_1)

{
  *(undefined1 *)(param_1 + 0x2d2) = 0;
  *(undefined2 *)(param_1 + 0x2ce) = 0;
  *(undefined2 *)(param_1 + 0x2d0) = 0;
  *(undefined1 *)(param_1 + 0x2cd) = 0;
  *(undefined4 *)(param_1 + 0x324) = 0;
  *(undefined4 *)(param_1 + 0x2d4) = 0;
  *(undefined4 *)(param_1 + 0x2d8) = 0;
  *(undefined4 *)(param_1 + 0x2dc) = 0;
  FUN_ram_f00266d8(param_1 + 0x328);
  *(undefined1 *)(param_1 + 0x340) = 0;
  *(undefined1 *)(param_1 + 0x2e0) = 0;
  *(undefined4 *)(param_1 + 0x2e4) = 0;
  *(undefined4 *)(param_1 + 0x2e8) = 0;
  *(undefined1 *)(param_1 + 0x2ec) = 0;
  FUN_ram_f00266d8(param_1 + 0x2f0);
  FUN_ram_f00266d8(param_1 + 0x308);
  FUN_ram_f0025198(param_1,0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ae04 @ 0xf003ae04 =====


void FUN_ram_f003ae04(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x7c);
  if ((iVar1 != 0) && (*(short *)(iVar1 + 0x12) != 0)) {
    FUN_ram_f0022bd0(iVar1,param_1,0);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ae2c @ 0xf003ae2c =====


void FUN_ram_f003ae2c(int param_1)

{
  int iVar1;
  undefined2 *puVar2;
  
  iVar1 = FUN_ram_f0025c48(0x10);
  if (iVar1 != 0) {
    puVar2 = *(undefined2 **)(iVar1 + 8);
    *(undefined1 *)(puVar2 + 4) = *(undefined1 *)(param_1 + 0x19);
    *(undefined1 *)((int)puVar2 + 9) = *(undefined1 *)(param_1 + 0x2e0);
    *(undefined1 *)(puVar2 + 5) = *(undefined1 *)(param_1 + 0x2e4);
    *(undefined1 *)((int)puVar2 + 0xb) = *(undefined1 *)(param_1 + 0x2e8);
    *(undefined1 *)(puVar2 + 6) = *(undefined1 *)(param_1 + 0x2ec);
    *(undefined1 *)((int)puVar2 + 0xd) = *(undefined1 *)(param_1 + 0x340);
    *(undefined1 *)(puVar2 + 2) = 0x1d;
    *puVar2 = 0x10;
    *(undefined1 *)((int)puVar2 + 5) = 0;
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar2;
    puVar2[1] = 1;
    FUN_ram_f0039750();
  }
  return;
}

// >>> MOD: wifi/mgmt/rlm.c:0x2d2,0x2d3 <<<
// ===== FUN_ram_f003ae98 @ 0xf003ae98 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003ae98(int param_1)

{
  ushort uVar1;
  uint uVar2;
  undefined1 auStack_30 [12];
  undefined1 auStack_24 [12];
  undefined4 local_18;
  undefined2 local_14;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,0x2d2,_DAT_ram_00014800);
  }
  if (0x13 < *(byte *)(param_1 + 0x28a)) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,0x2d3,_DAT_ram_00014800);
  }
  (*_DAT_ram_00014e54)
            (*(undefined1 *)(param_1 + 0x28a),auStack_24,&local_18,auStack_30,_DAT_ram_00014e54);
  uVar2 = *(byte *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x76) & 0x10
  ;
  if ((*(byte *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x76) & 0x10)
      == 0) {
    local_14 = (undefined2)uVar2;
    local_18 = uVar2;
  }
  else {
    uVar1 = *(ushort *)(param_1 + 0x28);
    local_18 = CONCAT31(CONCAT21(local_18._2_2_,(char)(uVar1 >> 6)),(char)(uVar1 >> 1)) & 0xffff0101
    ;
    local_18 = CONCAT13(local_18._3_1_,CONCAT12((char)(uVar1 >> 5),(undefined2)local_18)) &
               0xff01ffff;
  }
  (*_DAT_ram_00014e4c)
            (*(undefined1 *)(param_1 + 0x28a),auStack_24,&local_18,auStack_30,_DAT_ram_00014e4c);
  return;
}

// >>> MOD: wifi/mgmt/rlm.c:0x2a2,0x2a7 <<<
// ===== FUN_ram_f003af48 @ 0xf003af48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003af48(int param_1)

{
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,0x2a2,_DAT_ram_00014800);
  }
  if (3 < *(byte *)(param_1 + 8)) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,0x2a7,_DAT_ram_00014800);
  }
  DAT_ram_f0065004 = *(undefined1 *)(param_1 + 0x90);
  FUN_ram_f003b6c8(param_1);
  FUN_ram_f003b84c(param_1);
  FUN_ram_f003b8dc(param_1);
  FUN_ram_f003b804(param_1);
  FUN_ram_f003aba0(param_1);
  FUN_ram_f004571c(*(undefined4 *)(param_1 + 0x6c),*(undefined1 *)(param_1 + 0x70),
                   *(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x7c));
  return;
}

// >>> MOD: wifi/mgmt/rlm.c:0x4fa <<<
// ===== FUN_ram_f003aff4 @ 0xf003aff4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003aff4(int param_1)

{
  char cVar1;
  int iVar2;
  ushort uVar3;
  int iVar4;
  int iVar5;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,0x4fa,_DAT_ram_00014800);
  }
  if (2 < *(byte *)(param_1 + 0xd)) {
    return 0xc0000001;
  }
  iVar5 = *(int *)(param_1 + 0x14);
  iVar2 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4);
  iVar4 = *(int *)(param_1 + 0x20);
  if (*(char *)(iVar5 + 0x19) != '\x03') {
    return 0xc0000001;
  }
  if (iVar4 == 0) {
    return 0xc0000001;
  }
  if (*(int *)(iVar2 + 0x58) != iVar4) {
    return 0xc0000001;
  }
  if (*(char *)(iVar4 + 0x15) != '\x02') {
    return 0xc0000001;
  }
  if (*(short *)(param_1 + 0x12) == 0) {
    return 0xc0000001;
  }
  uVar3 = *(ushort *)(iVar2 + 0x2d0);
  *(ushort *)(iVar2 + 0x2d0) = uVar3 | 0x400;
  cVar1 = *(char *)(iVar5 + 0x1a);
  if (cVar1 == '\0') {
    uVar3 = uVar3 & 0xf7ff | 0x400;
  }
  else {
    if (cVar1 != '\x01') goto LAB_ram_f003b072;
    uVar3 = uVar3 | 0xc00;
  }
  *(ushort *)(iVar2 + 0x2d0) = uVar3;
LAB_ram_f003b072:
  if (((_DAT_ram_f00650a0 + 0x25c != *(int *)(_DAT_ram_f00650a0 + 0x25c)) &&
      (*(int *)(_DAT_ram_f00650a0 + 0x25c) != 0)) && (_DAT_ram_f0065098 == 0)) {
    FUN_ram_f003af48();
  }
  (*(code *)&SUB_ram_0006a0d8)(param_1);
  return 1;
}

// >>> MOD: wifi/mgmt/rlm.c:0x1fb,0x256,0x348,0x349,0x3c4,0x3c5 <<<
// ===== FUN_ram_f003b0b8 @ 0xf003b0b8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003b0b8(int param_1,byte *param_2,ushort param_3)

{
  undefined1 uVar1;
  byte bVar2;
  bool bVar3;
  bool bVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  byte *pbVar8;
  uint uVar9;
  byte *pbVar10;
  uint uVar11;
  uint uVar12;
  byte *pbVar13;
  int iVar14;
  byte *pbVar15;
  byte *pbVar16;
  uint uVar17;
  uint uStack_38;
  int *piStack_34;
  undefined4 uStack_30;
  undefined1 auStack_26 [2];
  ushort local_24;
  undefined1 auStack_21 [5];
  
  uVar9 = (uint)param_3;
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,0x256,_DAT_ram_00014800);
  }
  if (param_2 == (byte *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006393c,599,_DAT_ram_00014800);
  }
  uStack_30 = 0;
  piStack_34 = (int *)&DAT_ram_020a0068;
  uStack_38 = 0;
  do {
    iVar14 = *piStack_34;
    iVar5 = FUN_ram_f003ab54(iVar14);
    if (iVar5 != 0) {
      if (*(int *)(iVar14 + 0xc) == 0) {
        iVar5 = (*(code *)&SUB_ram_0006a074)(iVar14 + 0x4b,*(int *)(param_1 + 0x14) + 0x10,6);
        if (iVar5 == 0) {
          if (param_2 == (byte *)0x0) {
            (*_DAT_ram_00014800)(0,0,0xf006393c,0x1fb,_DAT_ram_00014800);
          }
          if (2 < *(uint *)(_DAT_ram_020a0064 + 0x380)) {
            pbVar10 = param_2;
            for (uVar11 = 0; uVar11 < uVar9; uVar11 = uVar11 + *pbVar13 + 2 & 0xffff) {
              if (*pbVar10 == 0x3d) {
                if (((*(byte *)(iVar14 + 0x76) & 0x10) != 0) &&
                   (*(short *)(iVar14 + 0x2d0) != *(short *)(pbVar10 + 6))) {
                  *(short *)(iVar14 + 0x2d0) = *(short *)(pbVar10 + 6);
                  uVar11 = 1;
                  goto LAB_ram_f003b1b4;
                }
                break;
              }
              pbVar13 = pbVar10 + 1;
              pbVar10 = pbVar10 + *pbVar13 + 2;
            }
          }
          uVar11 = 0;
LAB_ram_f003b1b4:
          uStack_38 = uVar11 | uStack_38;
          uStack_30 = 1;
        }
      }
      else if (*(int *)(iVar14 + 0xc) == 2) {
        if (param_1 == 0) {
          (*_DAT_ram_00014800)(0,0,0xf006393c,0x3c4,_DAT_ram_00014800);
        }
        if (param_2 == (byte *)0x0) {
          (*_DAT_ram_00014800)(0,0,0xf006393c,0x3c5,_DAT_ram_00014800);
        }
        if (*(char *)(iVar14 + 0x19) == DAT_ram_f0064fd2) {
          if (param_1 == 0) {
            (*_DAT_ram_00014800)(0,0,0xf006393c,0x348,_DAT_ram_00014800);
          }
          if (param_2 == (byte *)0x0) {
            (*_DAT_ram_00014800)(0,0,0xf006393c,0x349,_DAT_ram_00014800);
          }
          uVar11 = *(uint *)(iVar14 + 0x324);
          if (uVar11 == 0) {
LAB_ram_f003b326:
            uVar11 = 0;
          }
          else {
            bVar3 = false;
            uVar1 = *(undefined1 *)(param_1 + 0x49);
            uVar17 = 0;
            uVar6 = FUN_ram_f003aaf8(uVar1);
            pbVar10 = param_2;
            for (uVar12 = 0; uVar12 < uVar9; uVar12 = uVar12 + *pbVar13 + 2 & 0xffff) {
              bVar2 = *pbVar10;
              if (bVar2 == 0x2d) {
                uVar7 = (pbVar10[3] & 0x40) << 8;
LAB_ram_f003b2b8:
                if (uVar7 != 0) {
                  bVar3 = true;
                }
              }
              else if (bVar2 < 0x2e) {
                if (bVar2 == 3) {
                  uVar6 = (uint)pbVar10[2];
                }
              }
              else if (bVar2 == 0x3d) {
                uVar6 = (uint)pbVar10[2];
                uVar17 = pbVar10[3] & 3;
              }
              else if (bVar2 == 0x48) {
                uVar7 = pbVar10[2] & 2;
                goto LAB_ram_f003b2b8;
              }
              pbVar13 = pbVar10 + 1;
              pbVar10 = pbVar10 + *pbVar13 + 2;
            }
            iVar5 = FUN_ram_f003ab1c(uVar1);
            if (((iVar5 != 0x24ba58) || (*(byte *)(iVar14 + 0x2cc) != uVar6)) ||
               ((!bVar3 && ((uVar17 == 0 || (uVar17 == uVar11)))))) goto LAB_ram_f003b326;
            bVar3 = *(char *)(iVar14 + 0x340) == '\0';
            if (bVar3) {
              *(undefined1 *)(iVar14 + 0x340) = 1;
            }
            uVar11 = (uint)bVar3;
            FUN_ram_f002672c(iVar14 + 0x328,7000);
          }
          uVar6 = 0;
          uVar12 = FUN_ram_f003aaf8(*(undefined1 *)(param_1 + 0x49));
          bVar3 = false;
          uVar17 = 0;
          pbVar10 = (byte *)0x0;
          pbVar13 = param_2;
          pbVar16 = (byte *)0x0;
          while (uVar17 < uVar9) {
            bVar2 = *pbVar13;
            pbVar8 = pbVar10;
            pbVar15 = pbVar16;
            if (bVar2 == 0x2a) {
              if ((pbVar13[2] & 1) != 0) {
                uVar6 = 1;
              }
            }
            else if (bVar2 < 0x2b) {
              pbVar8 = pbVar13;
              if ((bVar2 != 1) && (pbVar8 = pbVar10, bVar2 == 3)) {
                uVar12 = (uint)pbVar13[2];
              }
            }
            else {
              pbVar15 = pbVar13;
              if ((bVar2 != 0x32) && (pbVar15 = pbVar16, bVar2 == 0x3d)) {
                uVar12 = (uint)pbVar13[2];
                bVar3 = true;
              }
            }
            pbVar10 = pbVar13 + 1;
            pbVar13 = pbVar13 + *pbVar10 + 2;
            uVar17 = uVar17 + *pbVar10 + 2 & 0xffff;
            pbVar10 = pbVar8;
            pbVar16 = pbVar15;
          }
          if (*(byte *)(iVar14 + 0x2cc) != uVar12) goto LAB_ram_f003b44a;
          if (bVar3) {
            if (uVar6 != 0) goto LAB_ram_f003b3da;
          }
          else {
            if ((uVar6 != 0) ||
               (FUN_ram_f00314ec(pbVar10,pbVar16,&local_24,auStack_26,auStack_21),
               (local_24 & 0x3fc0) == 0)) {
LAB_ram_f003b3da:
              bVar4 = *(char *)(iVar14 + 0x2e0) == '\0';
              if (bVar4) {
                uVar11 = 1;
                *(undefined1 *)(iVar14 + 0x2e0) = 1;
              }
              uVar6 = (uint)bVar4;
              FUN_ram_f002672c(iVar14 + 0x2f0,6000);
              if (bVar3) goto LAB_ram_f003b42c;
            }
            uVar12 = _DAT_ram_f006394c;
            if (*(int *)(iVar14 + 0x2e4) != 1) {
              *(undefined4 *)(iVar14 + 0x2e4) = 1;
              uVar6 = uVar12;
              uVar11 = uVar12;
            }
            FUN_ram_f002672c(iVar14 + 0x308,6000);
          }
LAB_ram_f003b42c:
          if (uVar11 != 0) {
            FUN_ram_f003ae04(iVar14);
            FUN_ram_f003ae2c(iVar14);
          }
        }
        else {
LAB_ram_f003b44a:
          uVar6 = 0;
        }
        uStack_38 = uVar6 & 0xff | uStack_38;
      }
    }
    piStack_34 = piStack_34 + 1;
    if (piStack_34 == (int *)&DAT_ram_020a0070) {
      if ((((uStack_38 != 0) && (_DAT_ram_f00650a0 + 0x25c != *(int *)(_DAT_ram_f00650a0 + 0x25c)))
          && (*(int *)(_DAT_ram_f00650a0 + 0x25c) != 0)) && (_DAT_ram_f0065098 == 0)) {
        FUN_ram_f003af48();
      }
      return uStack_30;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003b4a0 @ 0xf003b4a0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003b4a0(void)

{
  FUN_ram_f003c280();
  FUN_ram_f0045d70();
  _DAT_ram_f006506c = 0;
  FUN_ram_f003b76c();
  FUN_ram_f003bbc8();
  (*_DAT_ram_00014de4)(0,_DAT_ram_00014de4);
  (*_DAT_ram_00014d68)(1,_DAT_ram_00014d68);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003b600 @ 0xf003b600 =====


void FUN_ram_f003b600(int param_1,byte param_2,undefined1 *param_3,int param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int *piVar5;
  uint uVar6;
  int *piVar7;
  byte *pbVar8;
  uint uVar9;
  
  piVar7 = (int *)&DAT_ram_f0064968;
  pbVar8 = &DAT_ram_f0064971;
  uVar9 = 0;
  do {
    iVar1 = *piVar7;
    if (((iVar1 != 0) && (iVar1 != 4000000)) && ((param_1 == 0 || (iVar1 == param_1)))) {
      uVar6 = 0;
      while ((piVar5 = (int *)(param_4 + uVar9 * 8), uVar6 < *pbVar8 && (uVar9 < param_2))) {
        iVar4 = *piVar7;
        *piVar5 = iVar4;
        uVar2 = (uint)pbVar8[-1] + uVar6 * *(int *)(pbVar8 + -5);
        uVar3 = uVar2 & 0xff;
        iVar1 = iVar4 + uVar3 * 5000;
        *(char *)(piVar5 + 1) = (char)uVar2;
        if ((iVar4 == 0x24ba58) && (uVar3 == 0xe)) {
          iVar1 = 0x25e720;
        }
        iVar1 = FUN_ram_f0043270(iVar1,0);
        if (iVar1 != 0) {
          uVar9 = uVar9 + 1 & 0xff;
        }
        uVar6 = uVar6 + 1 & 0xff;
      }
    }
    pbVar8 = pbVar8 + 0x10;
    piVar7 = piVar7 + 4;
  } while (pbVar8 != (byte *)0xf00649d1);
  *param_3 = (char)uVar9;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003b6c8 @ 0xf003b6c8 =====


void FUN_ram_f003b6c8(int param_1)

{
  ushort uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  
  *(undefined4 *)(param_1 + 0x8c) = 0;
  iVar4 = 0;
  for (uVar5 = 0; uVar5 < *(byte *)(param_1 + 8); uVar5 = uVar5 + 1 & 0xff) {
    uVar2 = (uint)*(byte *)(param_1 + uVar5 + 9);
    if (uVar2 != 2) {
      iVar6 = *(int *)(&DAT_ram_020a0068 + uVar2 * 4);
      iVar3 = FUN_ram_f003ab54(iVar6);
      if ((iVar3 != 0) && ((*(byte *)(iVar6 + 0x76) & 0x10) != 0)) {
        if ((*(int *)(iVar6 + 0x324) == 0) || (iVar3 = *(int *)(iVar6 + 0x324), iVar4 != 0)) {
          iVar3 = iVar4;
        }
        uVar1 = *(ushort *)(iVar6 + 0x2d0);
        uVar2 = (uint)uVar1;
        iVar4 = iVar3;
        if ((uVar1 & 0x400) != 0) {
          if ((uVar1 & 0x800) != 0) {
            uVar2 = 2;
          }
          if ((uVar1 & 0x800) == 0) {
            uVar2 = 1;
          }
          if (*(uint *)(param_1 + 0x8c) == 0) {
            *(uint *)(param_1 + 0x8c) = uVar2;
          }
          else if (uVar2 != *(uint *)(param_1 + 0x8c)) {
            *(undefined4 *)(param_1 + 0x8c) = 1;
          }
        }
      }
    }
  }
  *(int *)(param_1 + 0x74) = iVar4;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003b76c @ 0xf003b76c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003b76c(void)

{
  FUN_ram_f00266a0(_DAT_ram_020a0068 + 0x328,&LAB_ram_f003b7c8,_DAT_ram_020a0068);
  FUN_ram_f00266a0(_DAT_ram_020a006c + 0x328,&LAB_ram_f003b7c8,_DAT_ram_020a006c);
  FUN_ram_f00266a0(_DAT_ram_020a0070 + 0x328,&LAB_ram_f003b7c8,_DAT_ram_020a0070);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003b804 @ 0xf003b804 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003b804(int param_1)

{
  *(undefined4 *)(param_1 + 0x88) = 1;
  (*_DAT_ram_00014970)(1,0,_DAT_ram_00014970);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003b828 @ 0xf003b828 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f003b828(void)

{
  bool bVar1;
  int iVar2;
  
  iVar2 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
  if ((_DAT_ram_f00650a0 + 0x25c == iVar2) || (iVar2 == 0)) {
    bVar1 = false;
  }
  else {
    bVar1 = *(int *)(iVar2 + 0x8c) != 0;
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003b84c @ 0xf003b84c =====


void FUN_ram_f003b84c(int param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  
  uVar4 = 0;
  *(undefined4 *)(param_1 + 0x84) = 0;
  do {
    if (*(byte *)(param_1 + 8) <= uVar4) {
      return;
    }
    uVar1 = (uint)*(byte *)(param_1 + uVar4 + 9);
    if (uVar1 != 2) {
      iVar3 = *(int *)(&DAT_ram_020a0068 + uVar1 * 4);
      iVar2 = FUN_ram_f003ab54(iVar3);
      if ((iVar2 != 0) && ((*(byte *)(iVar3 + 0x76) & 0x10) != 0)) {
        if ((*(int *)(iVar3 + 0x2d8) == 2) || (*(int *)(iVar3 + 0x2e8) == 2)) {
          *(undefined4 *)(param_1 + 0x84) = 2;
          return;
        }
        if ((((*(int *)(iVar3 + 0xc) == 1) || (*(int *)(iVar3 + 0x2d8) == 1)) ||
            (*(int *)(iVar3 + 0x2e8) == 1)) && (*(int *)(param_1 + 0x84) == 0)) {
          *(undefined4 *)(param_1 + 0x84) = 1;
        }
      }
    }
    uVar4 = uVar4 + 1 & 0xff;
  } while( true );
}

// >>> MOD: wifi/mgmt/rlm_protection.c:0x236 <<<
// ===== FUN_ram_f003b8dc @ 0xf003b8dc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003b8dc(int param_1)

{
  byte bVar1;
  undefined1 uVar2;
  bool bVar3;
  char cVar4;
  int iVar5;
  byte *pbVar6;
  int *piVar7;
  char cVar8;
  undefined1 uVar9;
  uint uVar10;
  int iVar11;
  uint uVar12;
  uint uVar13;
  undefined1 uVar14;
  
  piVar7 = (int *)(param_1 + 0x74);
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0063968,0x236,_DAT_ram_00014800);
  }
  uVar10 = 0;
  uVar13 = *(uint *)(param_1 + 0x80);
  *(undefined4 *)(param_1 + 0x80) = 0;
  if (*(int *)(param_1 + 0x8c) != 2) {
    for (; uVar10 < *(byte *)(param_1 + 8); uVar10 = uVar10 + 1 & 0xff) {
      uVar12 = (uint)*(byte *)((int)piVar7 + (uVar10 - 0x6b));
      if (uVar12 != 2) {
        iVar11 = *(int *)(&DAT_ram_020a0068 + uVar12 * 4);
        iVar5 = FUN_ram_f003ab54(iVar11);
        if (iVar5 != 0) {
          if ((*(char *)(iVar11 + 0x2d2) != '\0') || (*(char *)(iVar11 + 0x2e0) != '\0')) {
            *(undefined4 *)(param_1 + 0x80) = 1;
            break;
          }
          if ((*(byte *)(iVar11 + 0x76) & 0x10) != 0) {
            if (((*(int *)(iVar11 + 0xc) == 1) || (*(int *)(iVar11 + 0x2d4) == 1)) ||
               (*(int *)(iVar11 + 0x2e4) == 1)) {
              *(undefined4 *)(param_1 + 0x80) = 2;
            }
            else if ((*(int *)(iVar11 + 0x2d4) == 2) && (*(int *)(param_1 + 0x80) == 0)) {
              *(undefined4 *)(param_1 + 0x80) = 3;
            }
          }
        }
      }
    }
    uVar13 = *(uint *)(param_1 + 0x80) ^ uVar13;
  }
  FUN_ram_f003ce08(param_1);
  bVar3 = false;
  pbVar6 = (byte *)(param_1 + 9);
  cVar4 = '\0';
  while (cVar8 = cVar4 + '\x01', *(char *)(param_1 + 8) != cVar4) {
    bVar1 = *pbVar6;
    pbVar6 = pbVar6 + 1;
    cVar4 = cVar8;
    if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)bVar1 * 4) + 0x364) != '\0') {
      bVar3 = true;
    }
  }
  if (bVar3) {
    *(undefined4 *)(param_1 + 0x80) = 2;
  }
  uVar10 = _DAT_ram_60120080;
  if ((DAT_ram_f006500b == '\0') && (iVar5 = *(int *)(param_1 + 0x80), iVar5 - 1U < 3)) {
    uVar9 = *(undefined1 *)(iVar5 + -0xff9c671);
    uVar2 = *(undefined1 *)(iVar5 + -0xff9c67d);
    uVar14 = *(undefined1 *)(iVar5 + -0xff9c679);
    cVar4 = *(char *)(iVar5 + -0xff9c675);
  }
  else {
    uVar9 = 0;
    uVar2 = 0;
    cVar4 = '\0';
    uVar14 = 0;
  }
  _DAT_ram_60120080 = _DAT_ram_60120080 | 0x100;
  if (*piVar7 != 0) {
    if (*(int *)(param_1 + 0x8c) == 2) {
      iVar11 = 1;
      *(undefined4 *)(param_1 + 0x80) = 0;
      iVar5 = iVar11;
      goto LAB_ram_f003ba80;
    }
    if ((*(int *)(param_1 + 0x8c) == 1) || (*(int *)(param_1 + 0x80) == 1)) {
      iVar11 = 0;
      iVar5 = 1;
      goto LAB_ram_f003ba80;
    }
  }
  iVar11 = 0;
  iVar5 = iVar11;
LAB_ram_f003ba80:
  (*_DAT_ram_00014958)(iVar5,_DAT_ram_00014958);
  if (iVar5 != 0) {
    (*_DAT_ram_00014db0)(iVar11,_DAT_ram_00014db0);
  }
  if ((*piVar7 == 0) || (*(int *)(param_1 + 0x80) == 1)) {
    bVar3 = false;
  }
  else {
    bVar3 = *(int *)(param_1 + 0x8c) != 1;
  }
  if ((bool)*(char *)(param_1 + 0x78) != bVar3) {
    *(bool *)(param_1 + 0x78) = bVar3;
    for (uVar12 = 0; uVar12 < *(byte *)(param_1 + 8); uVar12 = uVar12 + 1 & 0xff) {
      FUN_ram_f003dab8(*(undefined1 *)(param_1 + uVar12 + 9));
    }
    FUN_ram_f003ce08(param_1);
  }
  if ((uVar13 != 0) || (_DAT_ram_f00650d4 != *(int *)(param_1 + 0x80))) {
    _DAT_ram_f00650d4 = *(int *)(param_1 + 0x80);
  }
  (*_DAT_ram_00014e28)(0,_DAT_ram_00014e28);
  (*_DAT_ram_00014e1c)(uVar2,_DAT_ram_00014e1c);
  (*_DAT_ram_00014e24)(uVar14,_DAT_ram_00014e24);
  if (cVar4 == '\0') {
    bVar3 = *(int *)(param_1 + 0x84) == 1;
  }
  else {
    bVar3 = true;
  }
  (*_DAT_ram_00014e20)(bVar3,_DAT_ram_00014e20);
  (*_DAT_ram_00014e18)(uVar9,_DAT_ram_00014e18);
  _DAT_ram_60120080 = uVar10 & 0x300 | _DAT_ram_60120080 & 0xfffffcff;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003bb88 @ 0xf003bb88 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003bb88(undefined1 param_1)

{
  DAT_ram_f006500b = param_1;
  if (((_DAT_ram_f00650a0 + 0x25c != *(int *)(_DAT_ram_f00650a0 + 0x25c)) &&
      (*(int *)(_DAT_ram_f00650a0 + 0x25c) != 0)) && (_DAT_ram_f0065098 == 0)) {
    FUN_ram_f003b8dc();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003bbc8 @ 0xf003bbc8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003bbc8(void)

{
  int iVar1;
  int *piVar2;
  
  piVar2 = (int *)&DAT_ram_020a0068;
  do {
    iVar1 = *piVar2;
    piVar2 = piVar2 + 1;
    FUN_ram_f00266a0(iVar1 + 0x2f0,&LAB_ram_f003bc58,iVar1);
    FUN_ram_f00266a0(iVar1 + 0x308,&LAB_ram_f003bc1c,iVar1);
  } while (piVar2 != (int *)&DAT_ram_020a0074);
  _DAT_ram_f00650d4 = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003bc90 @ 0xf003bc90 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003bc90(void)

{
  (*_DAT_ram_00014998)(&DAT_ram_f006e4c0,_DAT_ram_00014998);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003bcb0 @ 0xf003bcb0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003bcb0(void)

{
  int iVar1;
  
  iVar1 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  if (-1 < (-0x10000 - _DAT_ram_f006e4bc) + iVar1) {
    FUN_ram_f003bc90();
    _DAT_ram_f006e4bc = iVar1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003bcf0 @ 0xf003bcf0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003bcf0(int *param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  (*_DAT_ram_00014998)(&DAT_ram_f006e4c0,_DAT_ram_00014998);
  uVar3 = _DAT_ram_6020060c;
  uVar2 = _DAT_ram_60200608;
  uVar1 = _DAT_ram_60200604;
  _DAT_ram_60200600 = _DAT_ram_60200600 & 0xffdfffff;
  _DAT_ram_6020080c = _DAT_ram_6020080c | 0x300000;
  *param_1 = _DAT_ram_f006e4e0 - _DAT_ram_f00650e0;
  _DAT_ram_f00650e0 = _DAT_ram_f006e4e0;
  param_1[1] = _DAT_ram_f006e4c0 - _DAT_ram_f00650e4;
  _DAT_ram_f00650e4 = _DAT_ram_f006e4c0;
  param_1[7] = uVar3 >> 0x10;
  param_1[2] = uVar1 & 0xffff;
  param_1[3] = uVar1 >> 0x10;
  param_1[4] = uVar2 & 0xffff;
  param_1[5] = uVar2 >> 0x10;
  param_1[6] = uVar3 & 0xffff;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003bdb4 @ 0xf003bdb4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003bdb4(void)

{
  undefined1 auStack_10 [4];
  undefined1 auStack_c [8];
  
  (*_DAT_ram_00014a8c)(&DAT_ram_f00650d8,auStack_c,&DAT_ram_f00650dc,auStack_10,_DAT_ram_00014a8c);
  DAT_ram_f006500d = 0;
  DAT_ram_f006500c = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c280 @ 0xf003c280 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003c280(void)

{
  if (DAT_ram_f006e4b8 == '\0') {
    (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_f0064f50 + 0x50,0,0x98);
    *(undefined4 *)(_DAT_ram_0209fce8 + 0x30) = 0;
    (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e4c0,0,0x50);
    DAT_ram_f006e4b8 = '\x01';
    _DAT_ram_f00650e0 = 0;
    _DAT_ram_f00650e4 = 0;
  }
  (*_DAT_ram_00014960)(0x7f07ff,1,_DAT_ram_00014960);
  _DAT_ram_f006e4bc = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c308 @ 0xf003c308 =====


bool FUN_ram_f003c308(int param_1,char param_2)

{
  bool bVar1;
  
  if ((param_1 == 5000000) || (param_1 == 0x4c5504)) {
    bVar1 = 99 < (byte)(param_2 - 0x31U);
  }
  else {
    bVar1 = true;
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c334 @ 0xf003c334 =====


undefined4 FUN_ram_f003c334(int param_1,byte param_2)

{
  int iVar1;
  byte *pbVar2;
  uint uVar3;
  
  pbVar2 = &DAT_ram_f00649d5;
  do {
    if (param_1 == *(int *)(pbVar2 + -9)) {
      uVar3 = 0;
      while (uVar3 < *pbVar2) {
        iVar1 = uVar3 * *(int *)(pbVar2 + -5);
        uVar3 = uVar3 + 1;
        if ((uint)param_2 == ((uint)pbVar2[-1] + iVar1 & 0xff)) {
          return 1;
        }
      }
    }
    pbVar2 = pbVar2 + 0x10;
  } while (pbVar2 != (byte *)0xf0064a35);
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c3e8 @ 0xf003c3e8 =====


void FUN_ram_f003c3e8(undefined1 param_1,ushort param_2)

{
  code *pcVar1;
  
  if ((param_2 >> 8 < 4) &&
     (pcVar1 = *(code **)((uint)(param_2 >> 8) * 4 + -0xff9c650), pcVar1 != (code *)0x0)) {
    (*pcVar1)(param_1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c4f8 @ 0xf003c4f8 =====


uint FUN_ram_f003c4f8(byte param_1,byte param_2)

{
  uint uVar1;
  
  if ((char)param_1 < '\0') {
    uVar1 = (uint)*(byte *)((param_1 & 0x7f) + 0xf0064a28) << (param_2 & 0x1f) & 0xff;
  }
  else {
    uVar1 = (uint)*(byte *)(param_1 + 0xf0064a34);
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c528 @ 0xf003c528 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003c528(undefined1 param_1)

{
  (*_DAT_ram_00014e70)(param_1,1,0,0,0,_DAT_ram_00014e70);
  (*_DAT_ram_00014e70)(param_1,2,0,0,0,_DAT_ram_00014e70);
  (*_DAT_ram_00014e70)(param_1,3,0,0,0,_DAT_ram_00014e70);
  (*_DAT_ram_00014e70)(param_1,4,0,0,0,_DAT_ram_00014e70);
  (*_DAT_ram_00014e70)(param_1,5,0,0,0,_DAT_ram_00014e70);
  (*_DAT_ram_00014e70)(param_1,6,0,0,0,_DAT_ram_00014e70);
  (*_DAT_ram_00014e74)(param_1,0,0,_DAT_ram_00014e74);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c5a4 @ 0xf003c5a4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003c5a4(int param_1,byte param_2)

{
  undefined1 uVar1;
  
  if ((char)param_2 < '\0') {
    param_2 = param_2 & 0x7f;
    if ((*(char *)(_DAT_ram_020a0064 + 0x20af) == '\x01') ||
       (((*(ushort *)(param_1 + 0x28) & 0x10) != 0 && (DAT_ram_f006501b == '\0')))) {
      uVar1 = 3;
    }
    else {
      uVar1 = 2;
    }
  }
  else {
    uVar1 = *(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x82) !=
            '\0';
  }
  (*_DAT_ram_00014ea8)(uVar1,param_2,_DAT_ram_00014ea8);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c618 @ 0xf003c618 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003c618(void)

{
  undefined4 extraout_a1;
  undefined4 extraout_a1_00;
  undefined4 extraout_a1_01;
  undefined4 extraout_a1_02;
  undefined4 extraout_a1_03;
  undefined4 local_28;
  undefined4 uStack_24;
  
  DAT_ram_f0065030 = 1;
  DAT_ram_f006502a = 1;
  DAT_ram_f006502b = 0;
  DAT_ram_f0065018 = 0;
  DAT_ram_f0065019 = 0;
  DAT_ram_f006501a = 0;
  local_28 = _DAT_ram_f0063ae8;
  uStack_24 = _DAT_ram_f0063aec;
  _DAT_ram_f00650f0 = 1;
  _DAT_ram_f00650e8 = (*_DAT_ram_00014830)(_DAT_ram_f0063ae8,_DAT_ram_f0063aec,_DAT_ram_00014830);
  DAT_ram_f0065010 = 0x24;
  DAT_ram_f0065011 = 0x23;
  DAT_ram_f0065012 = 0x62;
  DAT_ram_f0065013 = 0x58;
  DAT_ram_f0065014 = 5;
  DAT_ram_f0065017 = 8;
  _DAT_ram_f00650ec = 0x20c4;
  _DAT_ram_f0065100 = 0x40000;
  DAT_ram_f006501e = 0x80;
  DAT_ram_f006501f = 7;
  DAT_ram_f0065020 = 0x10;
  DAT_ram_f0065021 = 0x20;
  DAT_ram_f0065025 = 3;
  DAT_ram_f0065027 = 2;
  DAT_ram_f0065028 = 6;
  DAT_ram_f0065029 = 4;
  DAT_ram_f006501b = 1;
  _DAT_ram_f0065110 = 0;
  _DAT_ram_f0065114 = 0;
  DAT_ram_f006500e = 0x24;
  DAT_ram_f006500f = 0x23;
  DAT_ram_f0065015 = 0x80;
  DAT_ram_f0065016 = 0x80;
  _DAT_ram_f00650fc = 0x40000;
  DAT_ram_f0065031 = 1;
  DAT_ram_f0065022 = 3;
  DAT_ram_f0065023 = 3;
  DAT_ram_f0065024 = 3;
  DAT_ram_f0065026 = 4;
  DAT_ram_f006502e = 1;
  _DAT_ram_f0065108 = 0x230000;
  DAT_ram_f006503b = 0;
  DAT_ram_f006502f = 1;
  DAT_ram_f006502d = 0;
  DAT_ram_f006501c = 0;
  DAT_ram_f006501d = 0;
  DAT_ram_f006502c = 0;
  _DAT_ram_f00650f4 = _DAT_ram_f00650e8;
  _DAT_ram_f00650f8 = _DAT_ram_f00650e8;
  _DAT_ram_f0065104 = _DAT_ram_f00650e8;
  (*_DAT_ram_00014dc0)(&local_28,3,_DAT_ram_00014dc0);
  (*_DAT_ram_00014dd4)(0x1f,extraout_a1,_DAT_ram_00014dd4);
  (*_DAT_ram_00014930)(_DAT_ram_f00650f0,extraout_a1_00,_DAT_ram_00014930);
  (*_DAT_ram_00014dbc)(0,extraout_a1_01,_DAT_ram_00014dbc);
  (*_DAT_ram_00014904)(1,extraout_a1_02,_DAT_ram_00014904);
  (*_DAT_ram_00014978)(0x4000,0x4000,_DAT_ram_00014978);
  (*_DAT_ram_00014dfc)(0x2000,extraout_a1_03,_DAT_ram_00014dfc);
  (*_DAT_ram_00014de8)(6,0,_DAT_ram_00014de8);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c868 @ 0xf003c868 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003c868(int param_1,ushort *param_2,int param_3,int param_4)

{
  ushort uVar1;
  byte bVar2;
  ushort uVar3;
  char cVar4;
  uint uVar5;
  uint uVar6;
  byte *pbVar7;
  uint uVar8;
  byte *pbVar9;
  int local_48;
  uint uStack_44;
  ushort local_24;
  ushort local_22 [3];
  
  pbVar7 = (byte *)(param_1 + 0x317);
  local_48 = 0;
  uStack_44 = 0;
  uVar3 = 0;
  pbVar9 = (byte *)(param_1 + 0x30e);
  do {
    uVar5 = uStack_44 + 1;
    (*_DAT_ram_00014e70)
              (*(undefined1 *)(param_1 + 0x28a),uVar5,local_22,&local_24,0,_DAT_ram_00014e70);
    if (param_3 != 0) {
      *(ushort *)(param_3 + local_48) = local_22[0];
    }
    if (param_4 != 0) {
      *(ushort *)(param_4 + local_48) = local_24;
    }
    if ((uint)local_22[0] < (uint)DAT_ram_f0065014) {
      if (*pbVar7 < 4) {
        bVar2 = *pbVar7 + 1;
        goto LAB_ram_f003c95a;
      }
      *pbVar9 = 0xff;
    }
    else {
      uVar6 = ((uint)local_24 << 7) / (uint)local_22[0];
      bVar2 = (byte)uVar6;
      if ((uint)DAT_ram_f0065012 < (uVar6 & 0xff)) {
        bVar2 = DAT_ram_f0065012;
      }
      uVar8 = (uint)*pbVar9;
      uVar6 = (uint)bVar2;
      if (uVar8 == 0xff) {
        *pbVar9 = bVar2;
        uVar1 = (ushort)(1 << (uStack_44 & 0x1f));
      }
      else {
        if (uVar8 < uVar6) {
          bVar2 = (byte)((int)(uVar8 * 3 + 2 + uVar6) >> 2);
        }
        else {
          bVar2 = (byte)((int)(uVar8 + 1 + uVar6) >> 1);
        }
        *pbVar9 = bVar2;
        uVar1 = (ushort)(1 << (uStack_44 & 0x1f));
      }
      uVar3 = uVar3 | uVar1;
      bVar2 = 0;
LAB_ram_f003c95a:
      *pbVar7 = bVar2;
    }
    local_48 = local_48 + 2;
    pbVar7 = pbVar7 + 1;
    pbVar9 = pbVar9 + 1;
    uStack_44 = uVar5;
    if (uVar5 == 6) {
      *param_2 = uVar3;
      if (*(char *)(param_1 + 0x31e) == '\0') {
        *(char *)(param_1 + 0x314) = (char)((int)((uint)*(byte *)(param_1 + 0x314) * 7 + 8) >> 3);
        cVar4 = DAT_ram_f0065017;
      }
      else {
        cVar4 = *(char *)(param_1 + 0x31e) + -1;
      }
      *(char *)(param_1 + 0x31e) = cVar4;
      return;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c9b4 @ 0xf003c9b4 =====


int FUN_ram_f003c9b4(byte param_1,char param_2)

{
  char cVar1;
  
  if ((char)param_1 < '\0') {
    if (param_2 == '\x01') {
      cVar1 = '\x06';
    }
    else {
      cVar1 = '\0';
    }
    cVar1 = cVar1 + *(char *)((param_1 & 0x7f) + 0xf0064a54);
  }
  else {
    cVar1 = *(char *)(param_1 + 0xf0064a44);
  }
  return (int)cVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c9f4 @ 0xf003c9f4 =====


void FUN_ram_f003c9f4(void)

{
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003c9f8 @ 0xf003c9f8 =====


void FUN_ram_f003c9f8(void)

{
  return;
}

// >>> MOD: wifi/mgmt/rlm_ar.c:0x18d1 <<<
// ===== FUN_ram_f003c9fc @ 0xf003c9fc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003c9fc(int param_1,uint param_2,uint param_3)

{
  char cVar1;
  byte bVar2;
  undefined1 uVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  undefined1 *puVar10;
  byte *pbVar11;
  uint uVar12;
  undefined1 *puVar13;
  
  uVar12 = 0;
  puVar13 = (undefined1 *)(param_1 + 0x2de);
  puVar10 = (undefined1 *)(param_1 + 0x2e3);
  do {
    cVar1 = puVar13[0x26];
    if (((uVar12 & 0xff) == 0) && (cVar1 == '\0')) {
      (*_DAT_ram_00014800)(0,0,0xf00639c8,0x18d1,_DAT_ram_00014800);
    }
    bVar2 = cVar1 - 1;
    uVar4 = (uint)bVar2;
    *puVar10 = *(undefined1 *)(param_1 + uVar12 * 0xe + uVar4 + 0x2e8);
    if (uVar4 == 0) {
      uVar5 = 0;
    }
    else {
      uVar5 = uVar4 - 1 & 0xff;
    }
    puVar10[-1] = *(undefined1 *)(param_1 + uVar12 * 0xe + uVar5 + 0x2e8);
    if ((param_2 & 1) != 0) {
      *puVar10 = 0;
    }
    iVar8 = (int)(char)bVar2;
    puVar13[-2] = 0xff;
    pbVar11 = (byte *)(param_1 + iVar8 + uVar12 * 0xe + 0x2e8);
    iVar7 = 0;
    iVar6 = iVar8;
    do {
      if (iVar6 < 0) goto LAB_ram_f003cacc;
      bVar2 = *pbVar11;
      pbVar11 = pbVar11 + -1;
      iVar9 = iVar8 - iVar7;
      iVar6 = (int)(char)((char)iVar6 + -1);
      iVar7 = iVar7 + 1;
    } while (bVar2 < 6);
    puVar13[-2] = bVar2;
    if ((*(char *)(param_1 + uVar12 * 0xe + iVar9 + 0x2e8) == '\x06') && ((param_3 & 1) != 0)) {
      puVar13[-2] = 0x80;
    }
LAB_ram_f003cacc:
    *puVar13 = 0xff;
    if ((param_2 & 1) == 0) {
      uVar3 = *(undefined1 *)(param_1 + uVar12 * 0xe + uVar4 + 0x2e8);
    }
    else {
      uVar3 = 0;
    }
    uVar12 = uVar12 + 1;
    *puVar13 = uVar3;
    puVar10 = puVar10 + 4;
    puVar13 = puVar13 + 1;
    if (uVar12 == 2) {
      return 0;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003cb04 @ 0xf003cb04 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_ram_f003cb04(undefined1 param_1,undefined1 param_2,undefined1 param_3,undefined1 param_4)

{
  undefined1 auStack_40 [12];
  undefined1 auStack_34 [12];
  undefined1 local_28;
  undefined1 local_27;
  undefined1 local_26;
  
  (*_DAT_ram_00014e54)(param_1,auStack_34,&local_28,auStack_40,param_2,_DAT_ram_00014e54);
  local_28 = param_2;
  local_27 = param_4;
  local_26 = param_3;
  (*_DAT_ram_00014e4c)(param_1,auStack_34,&local_28,auStack_40,_DAT_ram_00014e4c);
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003cb60 @ 0xf003cb60 =====


void FUN_ram_f003cb60(int param_1)

{
  if (*(char *)(param_1 + 0x361) == '\0') {
    if (*(char *)(param_1 + 0x2da) == '\x01') {
      DAT_ram_f006e848 = DAT_ram_f006e848 | 1;
    }
    if (*(char *)(param_1 + 0x2db) == '\x01') {
      DAT_ram_f006e848 = DAT_ram_f006e848 | 2;
    }
  }
  FUN_ram_f003cb04(*(undefined1 *)(param_1 + 0x28a),*(undefined1 *)(param_1 + 0x30c),
                   *(undefined1 *)(param_1 + 0x2da),*(undefined1 *)(param_1 + 0x2db));
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003cbc8 @ 0xf003cbc8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003cbc8(int param_1,int param_2)

{
  ushort uVar1;
  
  if (((((param_1 != 0) && (*(char *)(_DAT_ram_020a0064 + 0x20b0) == '\0')) &&
       (DAT_ram_f006502e != '\0')) &&
      (((*(byte *)(_DAT_ram_020a0064 + 0x209a) & 0x10) != 0 && (*(char *)(param_1 + 9) != '\0'))))
     && ((*(char *)(param_1 + 0x15) == '\x02' &&
         (((*(byte *)(param_1 + 0x17) & 0x10) != 0 &&
          ((*(byte *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x76) &
           0x10) != 0)))))) {
    *(undefined1 *)(param_1 + 0x330) = 0;
    if (param_2 == 1) {
      if (*(char *)(param_1 + 0x30c) == '\0') {
        uVar1 = *(ushort *)(param_1 + 0x28) & 0x20;
      }
      else {
        uVar1 = *(ushort *)(param_1 + 0x28) & 0x40;
      }
      if (uVar1 == 0) {
        return;
      }
    }
    *(int *)(param_1 + 0x32c) = param_2;
    if (param_2 == 0) {
      *(undefined1 *)(param_1 + 0x2db) = 0;
      *(undefined1 *)(param_1 + 0x2da) = 0;
    }
    else if (param_2 == 1) {
      if (*(char *)(param_1 + 0x30c) == '\0') {
        *(undefined1 *)(param_1 + 0x2da) = 1;
        *(undefined1 *)(param_1 + 0x2db) = 0;
      }
      else {
        *(undefined1 *)(param_1 + 0x2da) = 0;
        *(undefined1 *)(param_1 + 0x2db) = 1;
      }
    }
    FUN_ram_f003cb60();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003cc84 @ 0xf003cc84 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_ram_f003cc84(undefined1 param_1,undefined1 *param_2,undefined1 *param_3,undefined1 *param_4)

{
  undefined1 auStack_38 [12];
  undefined1 auStack_2c [12];
  undefined1 local_20;
  undefined1 local_1f;
  undefined1 local_1e;
  
  (*_DAT_ram_00014e54)(param_1,auStack_2c,&local_20,auStack_38,_DAT_ram_00014e54);
  *param_2 = local_20;
  *param_3 = local_1e;
  *param_4 = local_1f;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ccc4 @ 0xf003ccc4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_ram_f003ccc4(int param_1)

{
  undefined1 uVar1;
  int iVar2;
  uint uVar3;
  byte local_18 [5];
  char local_13;
  char local_12;
  char local_11 [5];
  
  if ((*(char *)(param_1 + 9) != '\0') && (*(char *)(param_1 + 0x15) == '\x02')) {
    uVar1 = *(undefined1 *)(param_1 + 0x28a);
    (*_DAT_ram_00014e68)(uVar1,local_18,_DAT_ram_00014e68);
    uVar3 = local_18[0] & 0x3f;
    if (-1 < (char)local_18[0]) {
      return (ushort)*(byte *)((local_18[0] & 0xf) + 0xf0063aa8);
    }
    FUN_ram_f003cc84(uVar1,local_11,&local_12,&local_13);
    if (local_11[0] != '\0') {
      local_12 = local_13;
    }
    if (uVar3 < 8) {
      if (local_11[0] == '\0') {
        if (local_12 == '\0') {
          iVar2 = -0xff9c548;
        }
        else {
          iVar2 = -0xff9c540;
        }
        return (ushort)*(byte *)(iVar2 + uVar3);
      }
      if (local_12 == '\0') {
        iVar2 = -0xff9c538;
      }
      else {
        iVar2 = -0xff9c528;
      }
      return *(ushort *)(iVar2 + uVar3 * 2);
    }
  }
  return 0xc;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003cd78 @ 0xf003cd78 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003cd78(char param_1)

{
  if (param_1 == '\0') {
    (*_DAT_ram_00014dfc)(0x2000,_DAT_ram_00014dfc);
    DAT_ram_f0065017 = 8;
  }
  else {
    (*_DAT_ram_00014dfc)(0x1000,_DAT_ram_00014dfc);
    DAT_ram_f0065017 = 0x10;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003cdb0 @ 0xf003cdb0 =====


uint FUN_ram_f003cdb0(char param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  
  iVar3 = 0;
  uVar4 = 0;
  do {
    iVar1 = FUN_ram_f0025d90(iVar3);
    iVar3 = iVar3 + 1;
    if (((*(char *)(iVar1 + 9) != '\0') && (*(char *)(iVar1 + 0x15) == '\x02')) &&
       (*(char *)(iVar1 + 0x14) == param_1)) {
      uVar2 = FUN_ram_f003ccc4();
      if (uVar4 < uVar2) {
        uVar4 = uVar2;
      }
    }
  } while (iVar3 != 0x14);
  return uVar4;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ce08 @ 0xf003ce08 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003ce08(int param_1)

{
  ushort uVar1;
  int iVar2;
  char cVar3;
  ushort uVar4;
  uint uVar5;
  char cVar6;
  undefined1 uVar7;
  byte *pbVar8;
  sbyte local_21 [5];
  
  pbVar8 = (byte *)(param_1 + 9);
  uVar5 = 0x3fcf;
  uVar7 = 1;
  cVar6 = '\0';
  uVar1 = 0xffff;
LAB_ram_f003ce64:
  uVar4 = uVar1;
  cVar3 = cVar6 + '\x01';
  if (cVar6 != *(char *)(param_1 + 8)) {
    iVar2 = *(int *)(&DAT_ram_020a0068 + (uint)*pbVar8 * 4);
    pbVar8 = pbVar8 + 1;
    cVar6 = cVar3;
    if (*(char *)(iVar2 + 0x18) != '\0') goto code_r0xf003ce46;
    goto LAB_ram_f003ce60;
  }
  local_21[0] = 6;
  if (uVar4 == 0xffff) goto LAB_ram_f003cebe;
  if ((*(int *)(param_1 + 0x6c) == 5000000) ||
     ((*(char *)(param_1 + 0x78) != '\0' && (*(int *)(param_1 + 0x8c) == 2)))) {
LAB_ram_f003cea2:
    if (uVar4 < 0x2f) {
      local_21[0] = 6;
    }
    else {
      local_21[0] = 10;
    }
  }
  else {
    if (*(int *)(param_1 + 0x80) == 1) {
      if (0x10 < uVar4) {
        local_21[0] = 3;
        goto LAB_ram_f003cebe;
      }
    }
    else if (0x10 < uVar4) goto LAB_ram_f003cea2;
    local_21[0] = 0;
  }
LAB_ram_f003cebe:
  if ((uVar5 >> local_21[0] & 1) == 0) {
    if ((*(int *)(param_1 + 0x6c) == 5000000) ||
       ((*(char *)(param_1 + 0x78) != '\0' && (*(int *)(param_1 + 0x8c) == 2)))) {
      iVar2 = FUN_ram_f003170c(uVar5 & 0x3fc0,local_21);
      if (iVar2 == 0) {
        local_21[0] = 6;
      }
    }
    else if (*(int *)(param_1 + 0x80) == 1) {
      iVar2 = FUN_ram_f003170c(uVar5 & 0xf,local_21);
      if (iVar2 == 0) {
        local_21[0] = 1;
      }
    }
    else {
      iVar2 = FUN_ram_f003170c(uVar5,local_21);
      if (iVar2 == 0) {
        local_21[0] = 2;
      }
    }
  }
  FUN_ram_f0020480(local_21[0],uVar7);
  (*_DAT_ram_00014de8)(local_21[0],uVar7,_DAT_ram_00014de8);
  (*_DAT_ram_00014dc8)(local_21[0],uVar7,_DAT_ram_00014dc8);
  (*_DAT_ram_00014dc4)(6,uVar7,_DAT_ram_00014dc4);
  return;
code_r0xf003ce46:
  uVar1 = *(ushort *)(iVar2 + 0x342);
  if (*(char *)(iVar2 + 0x82) == '\0') {
    uVar7 = 0;
  }
  uVar5 = uVar5 & *(ushort *)(iVar2 + 0x84);
  if ((uVar1 == 0) || (uVar4 <= uVar1)) {
LAB_ram_f003ce60:
    uVar1 = uVar4;
  }
  goto LAB_ram_f003ce64;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003cf78 @ 0xf003cf78 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003cf78(int param_1,uint param_2,char param_3)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  
  uVar3 = 0;
  if (param_1 != 0) {
    iVar2 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
    *(undefined1 *)(param_1 + 0x338) = 0;
    if (param_3 == '\x01') {
      *(undefined1 *)(param_1 + 0x344) = 0;
    }
    if (param_2 < 3) {
      cVar1 = *(char *)(iVar2 + param_2 + 0x360);
      if (((DAT_ram_f006502d == '\0') && (((int)cVar1 + 0x13U & 0xff) < 0x1d)) &&
         (_DAT_ram_f0065098 == 0)) {
        iVar4 = *(int *)(_DAT_ram_f00650a0 + 0x25c);
        if (iVar4 == _DAT_ram_f00650a0 + 0x25c) {
          iVar4 = 0;
        }
        if (((*(int *)(iVar2 + 800) == iVar4) &&
            (iVar2 = FUN_ram_f00484bc((int)cVar1,*(undefined1 *)(param_1 + 0x30c)), iVar2 != 0)) &&
           ((*(char *)(param_1 + 0x14) != '\x01' || (*(char *)(_DAT_ram_020a0064 + 0x1a84) != '\0'))
           )) {
          *(char *)(param_1 + 0x344) = cVar1;
          *(uint *)(param_1 + 0x334) = param_2;
          return 0;
        }
      }
    }
    uVar3 = 0xc0000001;
  }
  return uVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003d034 @ 0xf003d034 =====


void FUN_ram_f003d034(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (param_1 == 0) {
    iVar2 = 0;
    do {
      iVar1 = FUN_ram_f0025d90(iVar2);
      iVar2 = iVar2 + 1;
      if ((*(char *)(iVar1 + 9) != '\0') && (*(char *)(iVar1 + 0x15) == '\x02')) {
        FUN_ram_f003cf78(iVar1,0,1);
      }
    } while (iVar2 != 0x14);
  }
  else {
    FUN_ram_f003cf78(param_1,0,1);
  }
  return;
}

// >>> MOD: wifi/mgmt/rlm_ar.c:0xb6a <<<
// ===== FUN_ram_f003d110 @ 0xf003d110 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003d110(int param_1,byte param_2)

{
  undefined1 uVar1;
  undefined4 *puVar2;
  undefined1 *puVar3;
  undefined4 local_1c;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00639c8,0xb6a,_DAT_ram_00014800);
  }
  local_1c = 0;
  if (((param_1 != 0) && (*(char *)(param_1 + 0x361) == '\0')) && (param_2 == 1)) {
    DAT_ram_f006e848 = DAT_ram_f006e848 | 3;
  }
  puVar3 = (undefined1 *)((param_2 + 0xb8) * 4 + param_1);
  puVar2 = &local_1c;
  do {
    uVar1 = *puVar3;
    puVar3 = puVar3 + 1;
    uVar1 = FUN_ram_f003c5a4(param_1,uVar1);
    *(undefined1 *)puVar2 = uVar1;
    puVar2 = (undefined4 *)((int)puVar2 + 1);
  } while (puVar2 != (undefined4 *)&stack0xffffffe8);
  if ((param_1 != 0) && (*(char *)(param_1 + 0x361) == '\0')) {
    _DAT_ram_f006ec8c = local_1c;
  }
  (*_DAT_ram_00014e7c)(*(undefined1 *)(param_1 + 0x28a),&local_1c,_DAT_ram_00014e7c);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003d1b4 @ 0xf003d1b4 =====


void FUN_ram_f003d1b4(int param_1)

{
  (*(code *)&SUB_ram_0006a0a0)(param_1 + 0x30e,0xff,6);
  (*(code *)&SUB_ram_0006a0a0)(param_1 + 0x317,0,6);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003d1e8 @ 0xf003d1e8 =====


void FUN_ram_f003d1e8(int param_1)

{
  FUN_ram_f003d1b4();
  DAT_ram_f006501a = 0;
  *(undefined1 *)(param_1 + 0x330) = 0;
  *(undefined1 *)(param_1 + 800) = 0;
  *(undefined1 *)(param_1 + 799) = 0;
  *(undefined1 *)(param_1 + 0x321) = 0;
  *(undefined1 *)(param_1 + 0x315) = 0;
  *(undefined1 *)(param_1 + 0x316) = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003d21c @ 0xf003d21c =====


void FUN_ram_f003d21c(int param_1,char param_2)

{
  if (*(char *)(param_1 + 0x30c) != param_2) {
    *(char *)(param_1 + 0x30c) = param_2;
    FUN_ram_f003d1e8();
    *(undefined1 *)(param_1 + 0x31d) = 0xff;
    *(undefined1 *)(param_1 + 0x314) = 0;
  }
  return;
}

// >>> MOD: wifi/mgmt/rlm_ar.c:0x1956 <<<
// ===== FUN_ram_f003d24c @ 0xf003d24c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003d24c(int param_1,ushort param_2,byte param_3,char param_4)

{
  char cVar1;
  ushort uVar2;
  ushort *puVar3;
  undefined1 *puVar4;
  uint uVar5;
  ushort uVar6;
  byte bVar7;
  uint uVar8;
  int iVar9;
  int iStack_40;
  undefined1 local_30 [20];
  
  iStack_40 = param_1 + 0x2e8;
  cVar1 = '\0';
  puVar4 = (undefined1 *)(param_1 + 0x304);
  do {
    *puVar4 = 0;
    if (cVar1 == '\0') {
      iVar9 = -0xff9b5a0;
LAB_ram_f003d27e:
      uVar8 = 0;
      uVar5 = 0;
      uVar6 = param_2;
      bVar7 = param_3;
      while( true ) {
        puVar3 = (ushort *)(iVar9 + uVar8 * 8);
        if (*puVar3 == 0xff) break;
        uVar2 = uVar6 & *puVar3;
        if ((uVar2 & 8) == 0) {
          if (((bVar7 & (byte)puVar3[1]) != 0) || (uVar2 != 0)) {
            local_30[uVar5] = *(undefined1 *)((int)puVar3 + 7);
            uVar6 = uVar6 & ~puVar3[2];
            bVar7 = bVar7 & ~(byte)puVar3[3];
            uVar5 = uVar5 + 1 & 0xff;
          }
        }
        else {
          uVar2 = uVar6 & 0xff7f;
          local_30[uVar5] = 3;
          uVar5 = uVar5 + 1 & 0xff;
          if ((uVar6 & 4) != 0) {
            uVar2 = uVar6 & 0xff3f;
          }
          uVar6 = uVar2;
          if (cVar1 == '\0') {
            bVar7 = bVar7 & 0xfe;
          }
        }
        if (0xe < uVar5) {
          (*_DAT_ram_00014800)(0,0,0xf00639c8,0x1956,_DAT_ram_00014800);
          uVar5 = 0xe;
          break;
        }
        if (uVar5 == 0xe) break;
        uVar8 = uVar8 + 1 & 0xff;
      }
      if (cVar1 == '\0') {
        if (uVar5 == 0) {
          local_30[0] = 3;
LAB_ram_f003d350:
          uVar5 = 1;
        }
      }
      else if (uVar5 == 0) {
        local_30[0] = 8;
        goto LAB_ram_f003d350;
      }
      (*(code *)&SUB_ram_0006a08c)(iStack_40,local_30,uVar5);
      *puVar4 = (char)uVar5;
    }
    else if (param_4 != '\0') {
      iVar9 = -0xff9b4f8;
      goto LAB_ram_f003d27e;
    }
    cVar1 = cVar1 + '\x01';
    iStack_40 = iStack_40 + 0xe;
    puVar4 = puVar4 + 1;
    if (cVar1 == '\x02') {
      return 0;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003d388 @ 0xf003d388 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003d388(int param_1,byte param_2)

{
  short sVar1;
  byte bVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  undefined8 uVar11;
  
  bVar2 = *(byte *)(param_1 + 0x14);
  uVar7 = 0xffff;
  iVar9 = *(int *)(&DAT_ram_020a0068 + (uint)bVar2 * 4);
  iVar10 = 0;
  uVar8 = 0xffff;
  uVar5 = (uint)param_2;
  do {
    uVar11 = FUN_ram_f0025d90(iVar10,uVar5);
    uVar5 = (uint)((ulonglong)uVar11 >> 0x20);
    iVar3 = (int)uVar11;
    iVar10 = iVar10 + 1;
    if (((*(char *)(iVar3 + 9) != '\0') && (*(char *)(iVar3 + 0x15) == '\x02')) &&
       ((uVar5 = 0, param_1 != iVar3 || (param_2 == 0)))) {
      uVar4 = FUN_ram_f003c4f8(*(undefined1 *)(iVar3 + (*(byte *)(iVar3 + 0x30c) + 0xb8) * 4));
      uVar6 = uVar4;
      if (uVar8 < (uVar4 & 0xffff)) {
        uVar6 = uVar8;
      }
      uVar5 = (uint)*(byte *)(iVar3 + 0x14);
      uVar8 = uVar6 & 0xffff;
      if ((uVar5 == bVar2) && (uVar5 = (uint)(uVar4 < uVar7), uVar5 != 0)) {
        uVar7 = uVar4;
      }
    }
  } while (iVar10 != 0x14);
  sVar1 = *(short *)(iVar9 + 0x342);
  if (uVar7 == 0xffff) {
    *(undefined2 *)(iVar9 + 0x342) = 0;
  }
  else {
    *(short *)(iVar9 + 0x342) = (short)uVar7;
  }
  if (((*(short *)(iVar9 + 0x342) != sVar1) &&
      (_DAT_ram_f00650a0 + 0x25c != *(int *)(_DAT_ram_f00650a0 + 0x25c))) &&
     (*(int *)(_DAT_ram_f00650a0 + 0x25c) != 0)) {
    FUN_ram_f003ce08();
  }
  if (uVar8 < 3) {
    FUN_ram_f0020dc8(1);
  }
  else {
    FUN_ram_f0020dc8(0);
    if (0x23 < uVar8) {
      DAT_ram_f006501e = 0x80;
      return;
    }
  }
  DAT_ram_f006501e = 0x40;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003d488 @ 0xf003d488 =====


void FUN_ram_f003d488(undefined4 param_1)

{
  FUN_ram_f003d388(param_1,1);
  return;
}

// >>> MOD: wifi/mgmt/rlm_ar.c:0x1470 <<<
// ===== FUN_ram_f003d4a4 @ 0xf003d4a4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003d4a4(int param_1,int param_2,int param_3)

{
  byte bVar1;
  char cVar2;
  short sVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  
  iVar6 = *(int *)(param_1 + 0x20);
  if (DAT_ram_f006502a == '\0') {
    return;
  }
  if (*(char *)(param_1 + 0x48) != '\0') {
    return;
  }
  if (param_3 == 2) {
    return;
  }
  if (param_3 - 0x28U < 0x1e) {
    return;
  }
  if (*(short *)(param_2 + 0xe) < 0) {
    return;
  }
  if (iVar6 == 0) {
    return;
  }
  if (*(char *)(iVar6 + 9) == '\0') {
    return;
  }
  if (*(char *)(iVar6 + 0x15) != '\x02') {
    return;
  }
  if (param_3 == 0) {
    if (*(ushort *)(param_2 + 0x24) >> 0xd == 4) {
      sVar3 = *(short *)(param_2 + 0x26) + *(short *)(iVar6 + 0x324);
    }
    else {
      sVar3 = 0;
    }
    *(short *)(iVar6 + 0x324) = sVar3;
    *(undefined1 *)(iVar6 + 0x322) = 0;
  }
  else {
    *(char *)(iVar6 + 0x322) = *(char *)(iVar6 + 0x322) + '\x01';
    bVar1 = 0x80;
    if (DAT_ram_f0064fd5 == '\0') {
      bVar1 = 0x40;
    }
    if ((bVar1 <= *(byte *)(iVar6 + 0x322)) && (*(char *)(iVar6 + 0x14) == '\0')) {
      *(undefined2 *)(iVar6 + 0x324) = 0;
      *(undefined1 *)(iVar6 + 0x322) = 0;
      FUN_ram_f002b420(0,5);
    }
  }
  if ((DAT_ram_f0064fd5 == '\0') &&
     ((0xf < *(byte *)(iVar6 + 0x322) || (19999 < *(ushort *)(iVar6 + 0x324))))) {
    FUN_ram_f0042a24(iVar6);
  }
  if (2 < DAT_ram_f0065018) {
    (*_DAT_ram_00014800)(0,0,0xf00639c8,0x1470,_DAT_ram_00014800);
  }
  if (1 < DAT_ram_f0065018) {
    return;
  }
  if (*(char *)(iVar6 + 800) != '\0') {
    return;
  }
  *(char *)(iVar6 + 799) = *(char *)(iVar6 + 799) + '\x01';
  if ((param_3 == 0) || ((*(byte *)(param_2 + 0x18) & 4) != 0)) {
    cVar2 = *(char *)(iVar6 + 0x321);
    bVar1 = (byte)((ushort)*(undefined2 *)(param_2 + 0x24) >> 0xd);
  }
  else {
    if ((*(byte *)(param_2 + 0x18) & 0x10) == 0) {
      cVar2 = *(char *)(iVar6 + 0x321) + '\b';
    }
    else {
      cVar2 = *(char *)(iVar6 + 0x321) + '\f';
    }
    *(char *)(iVar6 + 0x321) = cVar2;
    if ((DAT_ram_f006501d != '\x01') || (*(short *)(iVar6 + 700) != 0)) goto LAB_ram_f003d614;
    cVar2 = *(char *)(iVar6 + 0x321);
    bVar1 = DAT_ram_f006501e >> 1;
  }
  *(byte *)(iVar6 + 0x321) = bVar1 + cVar2;
LAB_ram_f003d614:
  if (DAT_ram_f006501e <= *(byte *)(iVar6 + 0x321)) {
    iVar4 = FUN_ram_f0025b48(0,0x10);
    uVar5 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    *(undefined4 *)(iVar6 + 0x328) = uVar5;
    if (iVar4 != 0) {
      *(undefined4 *)(iVar4 + 8) = 6;
      *(int *)(iVar4 + 0xc) = iVar6;
      *(undefined1 *)(iVar6 + 800) = 1;
      FUN_ram_f0028344(0,iVar4,0);
      DAT_ram_f0065018 = DAT_ram_f0065018 + 1;
    }
    *(undefined1 *)(iVar6 + 0x321) = 0;
    *(undefined1 *)(iVar6 + 799) = 0;
  }
  return;
}

// >>> MOD: wifi/mgmt/rlm_ar.c:0xd01,0xd0b <<<
// ===== FUN_ram_f003d680 @ 0xf003d680 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003d680(int param_1,byte param_2,byte param_3,char param_4)

{
  undefined1 uVar1;
  byte bVar2;
  byte bVar3;
  bool bVar4;
  byte bVar5;
  short sVar6;
  short sVar7;
  uint uVar8;
  byte *pbVar9;
  byte *pbVar10;
  undefined4 uVar11;
  ushort uVar12;
  undefined4 uVar13;
  uint uVar14;
  int iVar15;
  ushort uVar16;
  int iVar17;
  int iVar18;
  undefined2 uVar19;
  uint uVar20;
  uint uVar21;
  ushort local_26;
  ushort local_24;
  undefined1 auStack_21 [5];
  
  uVar14 = (uint)param_2;
  uVar21 = (uint)param_3;
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00639c8,0xd01,_DAT_ram_00014800);
  }
  bVar3 = *(byte *)(param_1 + uVar21 + 0x304);
  pbVar10 = (byte *)(param_1 + 0x2e0 + uVar21 * 4);
  bVar5 = *(byte *)(param_1 + uVar21 + 0x306);
  uVar20 = (uint)bVar5;
  uVar1 = *(undefined1 *)(param_1 + 0x28a);
  if (bVar3 <= uVar14) {
    (*_DAT_ram_00014800)(0,0,0xf00639c8,0xd0b,_DAT_ram_00014800);
  }
  if ((uVar21 == 0) || (*pbVar10 < 6)) {
    bVar2 = *(byte *)(param_1 + 0x30e);
  }
  else {
    bVar2 = *(byte *)(param_1 + 0x30f);
  }
  uVar16 = (ushort)bVar2;
  if ((param_4 == '\0') && (uVar14 == uVar20)) {
    uVar21 = (uint)*(byte *)(param_1 + 699);
    if (uVar14 != 0) {
      return 0;
    }
    if (DAT_ram_f006502e != '\0') {
      if (uVar21 < 0x4d) {
        *(byte *)(param_1 + 0x330) = param_2;
      }
      else {
        bVar5 = *(char *)(param_1 + 0x330) + 1;
        *(byte *)(param_1 + 0x330) = bVar5;
        if ((0x1f < bVar5) && (*(int *)(param_1 + 0x32c) == 0)) {
          (*_DAT_ram_00014e70)(uVar1,7,&local_24,&local_26,auStack_21,_DAT_ram_00014e70);
          (*_DAT_ram_00014e74)(uVar1,0,0,_DAT_ram_00014e74);
          if ((uint)local_24 <= local_26 + 2) {
            FUN_ram_f003cbc8(param_1,1,0);
          }
          *(undefined1 *)(param_1 + 0x330) = 0;
        }
      }
    }
    if (DAT_ram_f006502f == '\0') {
      return 0;
    }
    iVar18 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
    if (*(byte *)(iVar18 + 0x363) < uVar16) {
      *(undefined1 *)(param_1 + 0x338) = 0;
      uVar11 = 0;
      uVar13 = 1;
    }
    else {
      iVar15 = *(int *)(param_1 + 0x334);
      if (iVar15 == 0) {
        if (*(byte *)(iVar18 + 0x35e) == 0) {
          return 0;
        }
        if (uVar21 < *(byte *)(iVar18 + 0x35e)) {
          *(undefined1 *)(param_1 + 0x338) = 0;
        }
        else {
          bVar5 = *(char *)(param_1 + 0x338) + 1;
          *(byte *)(param_1 + 0x338) = bVar5;
          if (bVar5 < 8) {
            return 0;
          }
          FUN_ram_f003cf78(param_1,1,0);
        }
        return 0;
      }
      if (iVar15 == 1) {
        if (*(byte *)(iVar18 + 0x35f) != 0) {
          if (uVar21 < *(byte *)(iVar18 + 0x35f)) {
            *(undefined1 *)(param_1 + 0x338) = 0;
          }
          else {
            bVar5 = *(char *)(param_1 + 0x338) + 1;
            *(byte *)(param_1 + 0x338) = bVar5;
            if (0xf < bVar5) {
              FUN_ram_f003cf78(param_1,2,0);
            }
          }
        }
        if ((int)(*(byte *)(iVar18 + 0x35e) - 10) <= (int)uVar21) {
          return 0;
        }
        uVar11 = 0;
        uVar13 = 0;
      }
      else {
        if (iVar15 != 2) {
          return 0;
        }
        if (*(byte *)(iVar18 + 0x35f) == 0) {
          return 0;
        }
        if (*(byte *)(iVar18 + 0x35f) <= uVar21) {
          return 0;
        }
        *(undefined1 *)(param_1 + 0x338) = 0;
        uVar11 = 1;
        uVar13 = 0;
      }
    }
    FUN_ram_f003cf78(param_1,uVar11,uVar13);
    return 0;
  }
  *(undefined1 *)(param_1 + 0x338) = 0;
  if ((uVar20 < uVar14) && (*(int *)(param_1 + 0x334) != 0)) {
    FUN_ram_f003cf78(param_1,0,1);
  }
  *(undefined1 *)(param_1 + 0x330) = 0;
  if (((uVar20 < uVar14) && (*(int *)(param_1 + 0x32c) == 1)) &&
     (FUN_ram_f003cbc8(param_1,0,0), 0x54 < *(byte *)(param_1 + 699))) {
    return 0;
  }
  iVar18 = param_1 + 0x2e8 + uVar21 * 0xe;
  if (param_4 != '\0') {
    *(undefined1 *)(param_1 + 0x31d) = 0xff;
    *(undefined1 *)(param_1 + 0x314) = 0;
    goto LAB_ram_f003d818;
  }
  if (uVar14 == uVar20) goto LAB_ram_f003d818;
  if (uVar16 == 0xff) {
    uVar16 = (ushort)DAT_ram_f0065013;
  }
  if (uVar16 < DAT_ram_f0065013) {
    if (uVar14 < uVar20) {
      if (((uint)*(byte *)(param_1 + 0x30d) < (uint)*(byte *)(param_1 + 699)) &&
         (7 < (int)((uint)*(byte *)(param_1 + 699) - (uint)*(byte *)(param_1 + 0x30d))))
      goto LAB_ram_f003d802;
    }
    uVar8 = (uint)*(byte *)(param_1 + 0x31d);
    if (uVar8 == 0xff) {
LAB_ram_f003d7be:
      uVar12 = 5;
      uVar8 = uVar14;
    }
    else {
      if (uVar14 < uVar20) {
        if ((uVar8 < uVar14) || (uVar20 <= uVar8)) goto LAB_ram_f003d7be;
      }
      else if ((uVar14 < uVar8) || (uVar8 <= uVar20)) goto LAB_ram_f003d7be;
      uVar12 = (ushort)*(byte *)(param_1 + 0x314);
    }
    sVar6 = FUN_ram_f003c4f8(*(undefined1 *)(iVar18 + uVar8),uVar21);
    sVar7 = FUN_ram_f003c4f8(*(undefined1 *)(iVar18 + uVar20),uVar21);
    if ((ushort)(sVar6 * (0x80 - uVar12)) <= (ushort)(sVar7 * (0x80 - uVar16))) {
      return 0;
    }
  }
LAB_ram_f003d802:
  *(byte *)(param_1 + 0x31d) = bVar5;
  *(char *)(param_1 + 0x314) = (char)uVar16;
LAB_ram_f003d818:
  *(byte *)(param_1 + uVar21 + 0x306) = param_2;
  *pbVar10 = *(byte *)(iVar18 + uVar14);
  *(byte *)(param_1 + 0x308) = param_2;
  *(undefined1 *)(param_1 + 0x30d) = *(undefined1 *)(param_1 + 699);
  uVar19 = *(undefined2 *)(_DAT_ram_f0064f50 + 0x118);
  if ((DAT_ram_f006501d == '\0') ||
     (uVar20 = FUN_ram_f003c4f8(*(byte *)(iVar18 + uVar14),0), 0x2d < uVar20)) {
    bVar4 = false;
  }
  else {
    bVar4 = true;
    uVar19 = 0x100;
  }
  FUN_ram_f0034a68(param_1,uVar19);
  if (uVar14 + 1 < (uint)bVar3) {
    uVar14 = uVar14 + 1 & 0xff;
  }
  pbVar9 = pbVar10 + 1;
  *pbVar9 = *(byte *)(iVar18 + uVar14);
  *(char *)(param_1 + 0x309) = (char)uVar14;
  if ((DAT_ram_f006501d == '\0') || (bVar4)) {
    bVar5 = *(byte *)(param_1 + uVar21 + 0x2dc);
    if ((bVar5 == 0xff) || (*pbVar9 < 6)) {
      pbVar10[2] = *pbVar9;
    }
    else {
      pbVar10[2] = bVar5;
    }
    pbVar10[3] = *(byte *)(param_1 + uVar21 + 0x2de);
  }
  else {
    iVar15 = (int)(char)(bVar3 - 1);
    pbVar10[2] = *pbVar9;
    pbVar10[3] = *pbVar9;
    pbVar9 = (byte *)(iVar18 + iVar15);
    iVar17 = 0;
    for (iVar18 = iVar15; -1 < iVar18; iVar18 = (int)(char)((char)iVar18 + -1)) {
      bVar5 = *pbVar9;
      uVar14 = FUN_ram_f003c4f8(bVar5,0);
      pbVar9 = pbVar9 + -1;
      if ((0x2d < uVar14) && ((int)(uint)*(byte *)(param_1 + 0x309) < iVar17 + iVar15)) {
        pbVar10[3] = bVar5;
        break;
      }
      iVar17 = iVar17 + -1;
    }
  }
  FUN_ram_f003d1e8(param_1);
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003dab8 @ 0xf003dab8 =====


void FUN_ram_f003dab8(byte param_1)

{
  byte bVar1;
  ushort uVar2;
  bool bVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  
  iVar7 = *(int *)(&DAT_ram_020a0068 + (uint)param_1 * 4);
  if ((*(byte *)(iVar7 + 0x76) & 0x10) != 0) {
    iVar6 = 0;
    bVar3 = false;
    do {
      iVar4 = FUN_ram_f0025d90(iVar6);
      if ((((*(char *)(iVar4 + 9) != '\0') && (*(char *)(iVar4 + 0x15) == '\x02')) &&
          ((uint)*(byte *)(iVar4 + 0x14) == (uint)param_1)) &&
         ((*(byte *)(iVar4 + 0x17) & 0x10) != 0)) {
        bVar1 = *(byte *)(iVar7 + 0x2cd);
        uVar5 = bVar1 & 4;
        if (((bVar1 & 4) != 0) &&
           (uVar2 = *(ushort *)(iVar4 + 0x28) >> 1, uVar5 = uVar2 & 1, (uVar2 & 1) != 0)) {
          bVar3 = true;
        }
        if (*(byte *)(iVar4 + 0x30c) != uVar5) {
          FUN_ram_f003cbc8(iVar4,0,0);
          FUN_ram_f003d034(iVar4);
          FUN_ram_f003d21c(iVar4,uVar5);
          if (*(char *)(iVar4 + uVar5 + 0x304) == '\0') {
            *(undefined1 *)(iVar4 + uVar5 + 0x304) = 1;
          }
          FUN_ram_f003d680(iVar4,*(undefined1 *)(iVar4 + uVar5 + 0x306),uVar5,1);
          FUN_ram_f003cb60(iVar4);
          FUN_ram_f003d110(iVar4,*(undefined1 *)(iVar4 + 0x30c));
        }
      }
      iVar6 = iVar6 + 1;
    } while (iVar6 != 0x14);
    if ((bVar3) && (iVar7 = FUN_ram_f003b828(), iVar7 != 0)) {
      DAT_ram_f006502c = 1;
    }
    else {
      DAT_ram_f006502c = 0;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003dbc8 @ 0xf003dbc8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003dbc8(int param_1,byte param_2)

{
  byte bVar1;
  undefined1 uVar2;
  byte bVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  byte bVar7;
  byte *pbVar8;
  undefined1 *puVar9;
  
  pbVar8 = (byte *)(param_1 + 0x304);
  iVar6 = 0;
  do {
    bVar1 = *pbVar8;
    if (bVar1 != 0) {
      if (1 < bVar1) {
        pbVar8[-0x2c] = 1;
      }
      puVar9 = (undefined1 *)(param_1 + iVar6 * 0xe + 0x2e8);
      bVar3 = 0;
      do {
        bVar7 = bVar3;
        if (bVar1 <= bVar7) {
          bVar7 = bVar1 - 1;
          break;
        }
        uVar2 = *puVar9;
        puVar9 = puVar9 + 1;
        iVar4 = FUN_ram_f003c9b4(uVar2,iVar6);
        bVar3 = bVar7 + 1;
      } while ((int)(uint)param_2 <= iVar4 + 0xb);
      FUN_ram_f003d680(param_1,bVar7,iVar6,1);
    }
    iVar6 = iVar6 + 1;
    pbVar8 = pbVar8 + 1;
    if (iVar6 == 2) {
      uVar5 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
      *(undefined4 *)(param_1 + 0x328) = uVar5;
      return 0;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003dc5c @ 0xf003dc5c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003dc5c(int param_1,undefined1 param_2)

{
  undefined1 uVar1;
  undefined1 uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  ushort uVar10;
  int iVar11;
  
  if (param_1 == 0) {
    return;
  }
  if (DAT_ram_f0065030 == '\0') {
    return;
  }
  if (DAT_ram_f006502a == '\0') {
    return;
  }
  if (*(char *)(param_1 + 9) == '\0') {
    return;
  }
  if (*(char *)(param_1 + 0x15) != '\x02') {
    return;
  }
  iVar11 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
  uVar1 = *(undefined1 *)(param_1 + 0x28a);
  uVar5 = (uint)*(ushort *)(param_1 + 0x1e);
  uVar4 = (uint)*(byte *)(param_1 + 0x26);
  if (((*(uint *)(param_1 + 0x10) & 0x80) == 0) && ((*(byte *)(iVar11 + 0x76) & 0x10) == 0)) {
    uVar4 = *(byte *)(iVar11 + 0x76) & 0x10;
  }
  if ((*(byte *)(param_1 + 0x17) & 0x10) == 0) {
    uVar4 = *(byte *)(param_1 + 0x17) & 0x10;
  }
  if ((uVar5 == 0) && (uVar4 == 0)) {
    uVar5 = 8;
  }
  uVar9 = uVar5 & ~_DAT_ram_f0065114;
  uVar7 = uVar4 & ~_DAT_ram_f0065110;
  if ((uVar9 != 0) || (uVar7 != 0)) goto LAB_ram_f003dd8e;
  iVar3 = FUN_ram_f00316c8(uVar5);
  if (iVar3 == 0) {
    uVar5 = 0;
  }
  else {
    uVar5 = FUN_ram_f003c4f8(0,0);
  }
  uVar8 = 7;
  do {
    uVar6 = uVar8 - 1;
    if ((uVar4 >> (uVar8 & 0x1f) & 1) != 0) {
      uVar8 = uVar8 & 0xff;
      uVar4 = FUN_ram_f003c4f8(uVar8 | 0x80,0);
      goto LAB_ram_f003dd62;
    }
    uVar8 = uVar6;
  } while (uVar6 != 0xffffffff);
  uVar4 = 0;
  uVar8 = 0;
LAB_ram_f003dd62:
  if ((uVar5 != 0) || (uVar4 != 0)) {
    if (uVar4 <= uVar5) {
      uVar9 = 1;
      goto LAB_ram_f003dd8e;
    }
    uVar7 = 1 << (uVar8 & 0x1f) & 0xff;
    if (uVar7 != 0) goto LAB_ram_f003dd8e;
  }
  uVar9 = 8;
LAB_ram_f003dd8e:
  (*(code *)&SUB_ram_0006a0a0)(param_1 + 0x2d8,0,100);
  *(undefined1 *)(param_1 + 0x31d) = 0xff;
  *(undefined1 *)(param_1 + 0x314) = 0;
  (*(code *)&SUB_ram_0006a0a0)(param_1 + 0x2e0,0,4);
  (*(code *)&SUB_ram_0006a0a0)(param_1 + 0x2e4,6,4);
  FUN_ram_f003d21c(param_1,0);
  if (*(char *)(_DAT_ram_020a0064 + 0x20b0) == '\x01') {
    *(byte *)(param_1 + 0x2da) = (byte)(*(ushort *)(param_1 + 0x28) >> 5) & 1;
    *(byte *)(param_1 + 0x2db) = (byte)(*(ushort *)(param_1 + 0x28) >> 6) & 1;
  }
  else {
    *(undefined1 *)(param_1 + 0x2db) = 0;
    *(undefined1 *)(param_1 + 0x2da) = 0;
  }
  if ((((*(byte *)(iVar11 + 0x76) & 0x10) != 0) &&
      ((*(byte *)(_DAT_ram_020a0064 + 0x209a) & 0x10) != 0)) &&
     ((*(byte *)(param_1 + 0x17) & 0x10) != 0)) {
    uVar10 = *(byte *)(iVar11 + 0x2cd) & 4;
    if ((*(byte *)(iVar11 + 0x2cd) & 4) != 0) {
      uVar10 = *(ushort *)(param_1 + 0x28) >> 1 & 1;
    }
    if (*(char *)(_DAT_ram_020a0064 + 0x20b0) == '\0') {
      FUN_ram_f003cbc8(param_1,0,0);
      (*_DAT_ram_00014e74)(uVar1,0,0,_DAT_ram_00014e74);
    }
    FUN_ram_f003d21c(param_1,uVar10);
  }
  FUN_ram_f003cb60(param_1);
  uVar10 = *(ushort *)(param_1 + 0x28) & 2;
  if ((*(ushort *)(param_1 + 0x28) & 2) != 0) {
    uVar10 = *(byte *)(param_1 + 0x17) >> 4 & 1;
  }
  FUN_ram_f003d24c(param_1,uVar9,uVar7,uVar10);
  FUN_ram_f003c9fc(param_1,uVar9,uVar7);
  *(undefined1 *)(param_1 + 0x2d9) = 0;
  *(undefined1 *)(param_1 + 0x2d8) = 0;
  if (*(char *)(_DAT_ram_020a0064 + 0x20ae) != -1) {
    *(undefined1 *)(param_1 + 0x304) = 1;
    uVar2 = *(undefined1 *)(_DAT_ram_020a0064 + 0x20ae);
    *(undefined1 *)(param_1 + 0x305) = 1;
    *(undefined1 *)(param_1 + 0x2e8) = uVar2;
    *(undefined1 *)(param_1 + 0x2f6) = *(undefined1 *)(_DAT_ram_020a0064 + 0x20ae);
  }
  FUN_ram_f003dbc8(param_1,param_2);
  FUN_ram_f003d110(param_1,*(undefined1 *)(param_1 + 0x30c));
  FUN_ram_f003d388(param_1,0);
  FUN_ram_f003c528(uVar1);
  FUN_ram_f003cf78(param_1,0,1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003dff8 @ 0xf003dff8 =====


void FUN_ram_f003dff8(void)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  do {
    iVar1 = FUN_ram_f0025d90(iVar2);
    iVar2 = iVar2 + 1;
    if ((*(char *)(iVar1 + 9) != '\0') && (*(char *)(iVar1 + 0x15) == '\x02')) {
      FUN_ram_f003dc5c(iVar1,*(undefined1 *)(iVar1 + 699),0);
    }
  } while (iVar2 != 0x14);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e03c @ 0xf003e03c =====


void FUN_ram_f003e03c(void)

{
  FUN_ram_f003dff8();
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e054 @ 0xf003e054 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003e054(undefined4 param_1,undefined4 param_2,char param_3)

{
  _DAT_ram_f0065110 = param_1;
  _DAT_ram_f0065114 = param_2;
  if (param_3 != '\0') {
    FUN_ram_f003dff8();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e080 @ 0xf003e080 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003e080(char param_1)

{
  undefined4 uVar1;
  
  if (param_1 == '\0') {
    uVar1 = 0x2000;
  }
  else {
    FUN_ram_f003e054(0,1,0);
    uVar1 = 0xffff;
  }
  (*_DAT_ram_00014dfc)(uVar1,_DAT_ram_00014dfc);
  DAT_ram_f006501d = param_1;
  FUN_ram_f003dff8();
  FUN_ram_f003bb88(param_1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e0d8 @ 0xf003e0d8 =====


void FUN_ram_f003e0d8(int param_1)

{
  FUN_ram_f003dc5c(param_1,*(undefined1 *)(param_1 + 0x35),0);
  return;
}

// >>> MOD: wifi/mgmt/rlm_ar.c:0xed0,0xedf <<<
// ===== FUN_ram_f003e0f8 @ 0xf003e0f8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003e0f8(int param_1,int param_2,byte param_3,byte param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar4 = (uint)param_3;
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf00639c8,0xed0,_DAT_ram_00014800);
  }
  iVar1 = param_1 + (uint)param_4;
  uVar2 = (uint)*(byte *)(iVar1 + 0x306);
  uVar3 = (uint)*(byte *)(iVar1 + 0x304);
  if (uVar3 <= uVar2) {
    (*_DAT_ram_00014800)(0,0,0xf00639c8,0xedf,_DAT_ram_00014800);
    uVar2 = uVar3 - 1 & 0xff;
  }
  if (param_2 == 0) {
    if (uVar2 < uVar4) {
      uVar2 = 0;
      goto LAB_ram_f003e16e;
    }
    uVar2 = uVar2 - uVar4;
  }
  else {
    uVar2 = uVar2 + uVar4;
    if (uVar3 <= uVar2) {
      uVar2 = uVar3 - 1 & 0xff;
      goto LAB_ram_f003e16e;
    }
  }
  uVar2 = uVar2 & 0xff;
LAB_ram_f003e16e:
  FUN_ram_f003d680(param_1,uVar2,(uint)param_4,0);
  return;
}

// >>> MOD: wifi/mgmt/rlm_ar.c:0x1004 <<<
// ===== FUN_ram_f003e184 @ 0xf003e184 =====


/* WARNING: Removing unreachable block (ram,0xf003e29a) */
/* WARNING: Removing unreachable block (ram,0xf003e284) */
/* WARNING: Removing unreachable block (ram,0xf003e248) */
/* WARNING: Removing unreachable block (ram,0xf003e254) */
/* WARNING: Removing unreachable block (ram,0xf003e252) */
/* WARNING: Removing unreachable block (ram,0xf003e26e) */
/* WARNING: Removing unreachable block (ram,0xf003e290) */
/* WARNING: Removing unreachable block (ram,0xf003e2a6) */
/* WARNING: Removing unreachable block (ram,0xf003e2aa) */
/* WARNING: Removing unreachable block (ram,0xf003e272) */
/* WARNING: Removing unreachable block (ram,0xf003e216) */
/* WARNING: Removing unreachable block (ram,0xf003e228) */
/* WARNING: Removing unreachable block (ram,0xf003e226) */
/* WARNING: Removing unreachable block (ram,0xf003e27a) */
/* WARNING: Removing unreachable block (ram,0xf003e2fc) */
/* WARNING: Removing unreachable block (ram,0xf003e300) */
/* WARNING: Removing unreachable block (ram,0xf003e30a) */
/* WARNING: Removing unreachable block (ram,0xf003e30e) */
/* WARNING: Removing unreachable block (ram,0xf003e2d4) */
/* WARNING: Removing unreachable block (ram,0xf003e2d8) */
/* WARNING: Removing unreachable block (ram,0xf003e2dc) */
/* WARNING: Removing unreachable block (ram,0xf003e2e6) */
/* WARNING: Removing unreachable block (ram,0xf003e2ea) */
/* WARNING: Removing unreachable block (ram,0xf003e312) */
/* WARNING: Removing unreachable block (ram,0xf003e338) */
/* WARNING: Removing unreachable block (ram,0xf003e2ec) */
/* WARNING: Removing unreachable block (ram,0xf003e2f6) */
/* WARNING: Removing unreachable block (ram,0xf003e2fa) */
/* WARNING: Removing unreachable block (ram,0xf003e342) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003e184(int param_1)

{
  int iVar1;
  byte bVar2;
  bool bVar3;
  uint uVar4;
  
  if ((*(char *)(param_1 + 9) != '\0') && (*(char *)(param_1 + 0x15) == '\x02')) {
    uVar4 = (uint)*(byte *)(param_1 + 0x30c);
    if (*(char *)(param_1 + uVar4 + 0x2d8) != '\0') {
      FUN_ram_f003c868();
      if ((uVar4 != 0) && (0xd < *(byte *)(param_1 + uVar4 + 0x306))) {
        (*_DAT_ram_00014800)(0,0,0xf00639c8,0x1004,_DAT_ram_00014800);
      }
      bVar2 = *(char *)(param_1 + 0x316) + 1;
      *(byte *)(param_1 + 0x316) = bVar2;
      bVar3 = 2 < bVar2;
      if (bVar3) {
        *(undefined1 *)(param_1 + 0x316) = 0;
      }
      if (*(byte *)(param_1 + 0x315) < 0xb) {
        if (!bVar3) {
          return;
        }
      }
      else {
        *(undefined1 *)(param_1 + 0x315) = 0;
        bVar3 = false;
      }
      iVar1 = FUN_ram_f003e0f8(param_1,bVar3,1,uVar4);
      if (iVar1 != 0) {
        FUN_ram_f003d110(param_1,uVar4);
        FUN_ram_f003d388(param_1,0);
      }
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e540 @ 0xf003e540 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003e540(void)

{
  _DAT_ram_f0065118 = _DAT_ram_020a0064 + 0x1f78;
  (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_f0065118,0,0x110);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e5c8 @ 0xf003e5c8 =====


bool FUN_ram_f003e5c8(int param_1,int param_2,int param_3)

{
  bool bVar1;
  
  if ((((param_3 == 0) || ((*(uint *)(param_3 + 0x10) & 0x80) == 0)) ||
      (*(char *)(param_3 + 0x4a1) == '\0')) ||
     (*(char *)(param_2 + 0x2cc) == *(char *)(param_3 + 0x4a1))) {
    bVar1 = false;
  }
  else if (param_1 == 0) {
    bVar1 = true;
  }
  else {
    bVar1 = *(char *)(param_3 + 0x499) == '\x01';
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e5f8 @ 0xf003e5f8 =====


void FUN_ram_f003e5f8(void)

{
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e5fc @ 0xf003e5fc =====


undefined1 FUN_ram_f003e5fc(void)

{
  return DAT_ram_f0065032;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e608 @ 0xf003e608 =====


void FUN_ram_f003e608(void)

{
  DAT_ram_f0065032 = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e614 @ 0xf003e614 =====


void FUN_ram_f003e614(void)

{
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e618 @ 0xf003e618 =====


undefined4 FUN_ram_f003e618(int param_1,ushort *param_2)

{
  undefined4 uVar1;
  ushort uVar2;
  
  if (DAT_ram_f0065032 == '\0') {
    uVar1 = 0;
  }
  else if ((((param_1 == 0) || (param_2 == (ushort *)0x0)) || (*(int *)(param_1 + 0x20) == 0)) ||
          (((*(uint *)(*(int *)(param_1 + 0x20) + 0x10) & 0x80) == 0 ||
           (2 < *(byte *)(param_1 + 0xd))))) {
    uVar1 = 0xc0000001;
  }
  else {
    if ((*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0xa8) == 1) ||
       (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0xd) * 4) + 0xa8) == 3)) {
      uVar2 = *param_2 | 0x1000;
    }
    else {
      uVar2 = *param_2 & 0xefff;
    }
    uVar1 = 0;
    *param_2 = uVar2;
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003e6f8 @ 0xf003e6f8 =====


void FUN_ram_f003e6f8(int param_1,undefined1 *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  *param_2 = 0x65;
  param_2[1] = 0x12;
  iVar3 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
  (*(code *)&SUB_ram_0006a08c)(param_2 + 2,iVar3 + 0x4b,6);
  iVar2 = param_1 + 10;
  iVar3 = iVar3 + 0x51;
  iVar1 = iVar3;
  if (*(char *)(param_1 + 0x371) == '\x01') {
    iVar1 = iVar2;
    iVar2 = iVar3;
  }
  (*(code *)&SUB_ram_0006a08c)(param_2 + 8,iVar1,6);
  (*(code *)&SUB_ram_0006a08c)(param_2 + 0xe,iVar2,6);
  return;
}

// >>> MOD: wifi/mgmt/tdls.c:0x3ac <<<
// ===== FUN_ram_f003e76c @ 0xf003e76c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003e76c(int param_1,int param_2,uint param_3,byte *param_4)

{
  char cVar1;
  undefined1 uVar2;
  byte bVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  undefined4 uVar8;
  int *piVar9;
  char *pcVar10;
  undefined1 *puVar11;
  byte *pbVar12;
  undefined1 *puStack_30;
  short sStack_2c;
  
  uVar4 = FUN_ram_f0025b3c();
  if (uVar4 < 5) {
LAB_ram_f003ee94:
    uVar8 = 0xc000009a;
  }
  else {
    FUN_ram_f0025b3c();
    if (param_3 == 5) {
      pcVar10 = (char *)(param_2 + 0x4af);
      piVar9 = (int *)(param_2 + 0x4b4);
    }
    else if (param_3 < 6) {
      if (param_3 == 3) {
        piVar9 = (int *)0xf006511c;
        pcVar10 = (char *)0xf0065033;
      }
      else {
        if (param_3 != 4) {
          return 0xc0000001;
        }
        pcVar10 = (char *)(param_2 + 0x3d2);
        piVar9 = (int *)(param_2 + 0x3d4);
      }
    }
    else if (param_3 == 6) {
      pcVar10 = (char *)(param_2 + 0x4b0);
      piVar9 = (int *)(param_2 + 0x4b8);
    }
    else {
      if (param_3 != 9) {
        return 0xc0000001;
      }
      pcVar10 = (char *)(param_2 + 0x3d3);
      piVar9 = (int *)(param_2 + 0x3d8);
    }
    iVar5 = (*_DAT_ram_00014830)();
    if (*pcVar10 == '\x01') {
      if (-1 < (int)((iVar5 + (uint)DAT_ram_f0064fc5 * -0x8000) - *piVar9)) {
        *pcVar10 = '\0';
        goto LAB_ram_f003e81c;
      }
    }
    else {
LAB_ram_f003e81c:
      *piVar9 = iVar5;
      if (param_3 == 5) {
        sStack_2c = 0x44;
        if (param_4[0xf] == 0) {
          sStack_2c = 0x41;
        }
        puStack_30 = &LAB_ram_f003e574;
        uVar8 = 0x4c;
      }
      else if (param_3 < 6) {
        if (param_3 == 3) {
          puStack_30 = (undefined1 *)0x0;
          sStack_2c = 0x3b;
          uVar8 = 0x43;
        }
        else {
          if (param_3 != 4) {
            return 0xc0000001;
          }
          puStack_30 = &LAB_ram_f003f1bc;
          sStack_2c = 0x3d;
          uVar8 = 0x45;
        }
      }
      else if (param_3 == 6) {
        puStack_30 = &LAB_ram_f003fd18;
        if (*(short *)(param_4 + 2) != 0) {
          puStack_30 = (undefined1 *)0x0;
        }
        sStack_2c = 0x41;
        uVar8 = 0x49;
      }
      else {
        if (param_3 != 9) {
          return 0xc0000001;
        }
        puStack_30 = &LAB_ram_f003e678;
        sStack_2c = 0x3a;
        uVar8 = 0x42;
      }
      puVar6 = (undefined4 *)FUN_ram_f0025c48(uVar8);
      if (puVar6 == (undefined4 *)0x0) goto LAB_ram_f003ee94;
      iVar5 = puVar6[2];
      puVar11 = (undefined1 *)(iVar5 + 0x28);
      if (param_3 == 5) {
        *(undefined1 *)(iVar5 + 8) = 0x88;
        *(undefined1 *)(iVar5 + 9) = 0;
        *(undefined1 *)(iVar5 + 10) = 0;
        *(undefined1 *)(iVar5 + 0xb) = 0;
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0xc,param_2 + 10,6);
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x12,param_1 + 0x51,6);
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x18,param_1 + 0x4b,6);
        *(undefined1 *)(iVar5 + 0x21) = 0;
        *(undefined1 *)(iVar5 + 0x1e) = 0;
        *(undefined1 *)(iVar5 + 0x1f) = 0;
        *(undefined1 *)(iVar5 + 0x20) = 5;
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x22,0xf0064950,6);
        *puVar11 = 0x89;
        *(undefined1 *)(iVar5 + 0x29) = 0xd;
        *(undefined1 *)(iVar5 + 0x2a) = 2;
        *(undefined1 *)(iVar5 + 0x2b) = 0xc;
        *(undefined1 *)(iVar5 + 0x2c) = 5;
        *(byte *)(iVar5 + 0x2d) = *param_4;
        if (*param_4 < 0xf) {
          *(byte *)(iVar5 + 0x2e) = param_4[1];
        }
        else {
          *(undefined1 *)(iVar5 + 0x2e) = 0x16;
        }
        if (param_4[0xf] != 0) {
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x49,param_4 + 0xd,3);
        }
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x2f,param_4 + 0x10,0x14);
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x43,param_4 + 0x24,6);
        *(undefined1 *)(param_2 + 0x37) = 0;
        FUN_ram_f0034724(param_2);
      }
      else if (param_3 < 6) {
        if (param_3 == 3) {
          *(undefined1 *)(iVar5 + 10) = 0;
          *(undefined1 *)(iVar5 + 0xb) = 0;
          if (param_2 == 0) {
            *(undefined1 *)(iVar5 + 8) = 0x88;
            *(undefined1 *)(iVar5 + 9) = 1;
            (*(code *)&SUB_ram_0006a08c)(iVar5 + 0xc,param_1 + 0x4b,6);
            pbVar12 = param_4 + 5;
            (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x12,param_1 + 0x51,6);
            (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x18,pbVar12,6);
            param_2 = *(int *)(param_1 + 0x58);
          }
          else {
            pbVar12 = (byte *)(param_2 + 10);
            if (*(char *)(param_2 + 0x38) == '\x01') {
              *(undefined1 *)(iVar5 + 8) = 0x88;
              *(undefined1 *)(iVar5 + 9) = 1;
              (*(code *)&SUB_ram_0006a08c)(iVar5 + 0xc,param_1 + 0x4b,6);
              (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x12,param_1 + 0x51,6);
              (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x18,pbVar12,6);
              param_2 = *(int *)(param_1 + 0x58);
            }
            else {
              *(undefined1 *)(iVar5 + 8) = 0x88;
              *(undefined1 *)(iVar5 + 9) = 0;
              (*(code *)&SUB_ram_0006a08c)(iVar5 + 0xc,pbVar12,6);
              (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x12,param_1 + 0x51,6);
              (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x18,param_1 + 0x4b,6);
            }
          }
          *(undefined1 *)(iVar5 + 0x21) = 0;
          *(undefined1 *)(iVar5 + 0x1e) = 0;
          *(undefined1 *)(iVar5 + 0x1f) = 0;
          *(undefined1 *)(iVar5 + 0x20) = 5;
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x22,0xf0064950,6);
          *puVar11 = 0x89;
          *(undefined1 *)(iVar5 + 0x29) = 0xd;
          *(undefined1 *)(iVar5 + 0x2a) = 2;
          *(undefined1 *)(iVar5 + 0x2b) = 0xc;
          *(undefined1 *)(iVar5 + 0x2c) = 3;
          *(byte *)(iVar5 + 0x2d) = param_4[2];
          bVar3 = param_4[3];
          *(undefined1 *)(iVar5 + 0x2f) = 0x65;
          *(byte *)(iVar5 + 0x2e) = bVar3;
          *(undefined1 *)(iVar5 + 0x30) = 0x12;
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x31,param_1 + 0x4b,6);
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x37,param_1 + 0x51,6);
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x3d,pbVar12,6);
          if (param_2 == 0) {
            (*(code *)&SUB_ram_0006a0d8)(puVar6);
            return 0xc0000001;
          }
        }
        else if (param_3 == 4) {
          *(undefined1 *)(iVar5 + 9) = 1;
          *(undefined1 *)(iVar5 + 8) = 0x88;
          *(undefined1 *)(iVar5 + 10) = 0;
          *(undefined1 *)(iVar5 + 0xb) = 0;
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0xc,param_1 + 0x4b,6);
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x12,param_1 + 0x51,6);
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x18,param_2 + 10,6);
          *(undefined1 *)(iVar5 + 0x20) = 5;
          *(undefined1 *)(iVar5 + 0x21) = 0;
          *(undefined1 *)(iVar5 + 0x1e) = 0;
          *(undefined1 *)(iVar5 + 0x1f) = 0;
          (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x22,0xf0064950,6);
          *puVar11 = 0x89;
          *(undefined1 *)(iVar5 + 0x29) = 0xd;
          *(undefined1 *)(iVar5 + 0x2a) = 2;
          *(undefined1 *)(iVar5 + 0x2b) = 0xc;
          *(undefined1 *)(iVar5 + 0x2c) = 4;
          cVar1 = *(char *)(param_2 + 0x370);
          *(char *)(iVar5 + 0x2d) = cVar1;
          *(char *)(param_2 + 0x370) = cVar1 + '\x01';
          FUN_ram_f003e6f8(param_2,iVar5 + 0x2e);
          *(undefined1 *)(iVar5 + 0x42) = 0x6a;
          *(undefined1 *)(iVar5 + 0x44) = 0xf;
          *(undefined1 *)(iVar5 + 0x43) = 1;
          param_2 = *(int *)(param_1 + 0x58);
        }
      }
      else if (param_3 == 6) {
        FUN_ram_f0025b3c();
        *(undefined1 *)(iVar5 + 8) = 0x88;
        *(undefined1 *)(iVar5 + 9) = 0;
        *(undefined1 *)(iVar5 + 10) = 0;
        *(undefined1 *)(iVar5 + 0xb) = 0;
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0xc,param_2 + 10,6);
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x12,param_1 + 0x51,6);
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x18,param_1 + 0x4b,6);
        *(undefined1 *)(iVar5 + 0x20) = 5;
        *(undefined1 *)(iVar5 + 0x21) = 0;
        *(undefined1 *)(iVar5 + 0x1e) = 0;
        *(undefined1 *)(iVar5 + 0x1f) = 0;
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x22,0xf0064950,6);
        *puVar11 = 0x89;
        *(undefined1 *)(iVar5 + 0x29) = 0xd;
        *(undefined1 *)(iVar5 + 0x2a) = 2;
        *(undefined1 *)(iVar5 + 0x2b) = 0xc;
        *(undefined1 *)(iVar5 + 0x2c) = 6;
        *(byte *)(iVar5 + 0x2d) = param_4[2];
        *(byte *)(iVar5 + 0x2e) = param_4[3];
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x2f,param_4 + 0x10,0x14);
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x43,param_4 + 0x24,6);
      }
      else if (param_3 == 9) {
        *(undefined1 *)(iVar5 + 8) = 0x88;
        *(undefined1 *)(iVar5 + 9) = 0;
        *(undefined1 *)(iVar5 + 10) = 0;
        *(undefined1 *)(iVar5 + 0xb) = 0;
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0xc,param_2 + 10,6);
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x12,param_1 + 0x51,6);
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x18,param_1 + 0x4b,6);
        *(undefined1 *)(iVar5 + 0x20) = 5;
        *(undefined1 *)(iVar5 + 0x21) = 0;
        *(undefined1 *)(iVar5 + 0x1e) = 0;
        *(undefined1 *)(iVar5 + 0x1f) = 0;
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x22,0xf0064950,6);
        *puVar11 = 0x89;
        *(undefined1 *)(iVar5 + 0x29) = 0xd;
        *(undefined1 *)(iVar5 + 0x2a) = 2;
        *(undefined1 *)(iVar5 + 0x2b) = 0xc;
        *(undefined1 *)(iVar5 + 0x2c) = 9;
        *(byte *)(iVar5 + 0x2d) = param_4[4];
        (*(code *)&SUB_ram_0006a08c)(iVar5 + 0x2e,param_4 + 0x10,0x14);
      }
      if (puVar6[2] == 0) {
        (*_DAT_ram_00014800)(0,0,0xf0063af0,0x3ac);
      }
      puVar6[8] = param_2;
      uVar2 = *(undefined1 *)(param_2 + 0x14);
      *(undefined1 *)((int)puVar6 + 0x11) = 0x1a;
      puVar6[6] = puVar6[2] + 0x22;
      *(short *)((int)puVar6 + 0x12) = sStack_2c + -0x1a;
      *(undefined1 *)(puVar6 + 4) = 1;
      *(undefined1 *)((int)puVar6 + 0xd) = uVar2;
      puVar6[5] = puVar6[2] + 8;
      *(undefined1 *)((int)puVar6 + 0x49) = 0x6c;
      *(undefined1 *)((int)puVar6 + 0x4a) = 0xf;
      puVar6[0xb] = puStack_30;
      *(undefined1 *)((int)puVar6 + 0xf) = 5;
      *(undefined1 *)(puVar6 + 0x16) = 0;
      puVar6[7] = 0;
      *(undefined1 *)((int)puVar6 + 0xe) = 0;
      *(undefined1 *)(puVar6 + 10) = 0;
      *(undefined1 *)((int)puVar6 + 0x29) = 0;
      *(undefined1 *)(puVar6 + 0xe) = 0;
      *(undefined1 *)((int)puVar6 + 0x39) = 0;
      *(undefined1 *)((int)puVar6 + 0x42) = 0;
      *(undefined1 *)(puVar6 + 0x12) = 0;
      *(undefined2 *)(puVar6 + 0x14) = 0;
      *(undefined2 *)((int)puVar6 + 0x52) = 0;
      *(undefined1 *)((int)puVar6 + 0x41) = 0;
      *(undefined1 *)(puVar6 + 0x15) = 0;
      puVar6[0x18] = 0;
      FUN_ram_f0028650(puVar6);
      FUN_ram_f0034ad8(puVar6);
      *puVar6 = 0;
      iVar7 = FUN_ram_f002a4cc(puVar6);
      if (iVar7 != 0) {
        *(byte *)(iVar5 + 9) = (byte)((ushort)*(undefined2 *)(iVar5 + 8) >> 8) | 0x40;
        *(char *)(iVar5 + 8) = (char)*(undefined2 *)(iVar5 + 8);
      }
      if (param_3 == 9) {
        *(byte *)(puVar6 + 9) = *(byte *)(puVar6 + 9) | 0x80;
      }
      FUN_ram_f0037940(puVar6);
    }
    uVar8 = 0;
  }
  return uVar8;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003eeb0 @ 0xf003eeb0 =====


undefined4 FUN_ram_f003eeb0(undefined4 param_1,int param_2,char param_3,char param_4)

{
  undefined4 uVar1;
  
  if (((param_3 == '\0') && (*(char *)(param_2 + 0x38) == '\x01')) &&
     (*(char *)(param_2 + 0x3cc) == '\x01')) {
    return 0;
  }
  uVar1 = FUN_ram_f003e76c(param_1,param_2,4,0,0);
  if (param_4 == '\0') {
    *(undefined1 *)(param_2 + 0x394) = 0;
  }
  *(undefined1 *)(param_2 + 0x3cc) = 1;
  *(undefined1 *)(param_2 + 0x3ce) = 0;
  *(undefined1 *)(param_2 + 0x3d0) = 0;
  FUN_ram_f002672c(param_2 + 0x37c,1000);
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ef14 @ 0xf003ef14 =====


void FUN_ram_f003ef14(undefined4 param_1,int param_2,int param_3)

{
  undefined2 uVar1;
  
  uVar1 = *(undefined2 *)(param_3 + 2);
  if (*(char *)(param_2 + 0x49a) == '\x01') {
    uVar1 = 0x25;
  }
  *(undefined2 *)(param_3 + 2) = uVar1;
  FUN_ram_f003e76c(param_1,param_2,6,param_3,0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ef48 @ 0xf003ef48 =====


void FUN_ram_f003ef48(int param_1,int param_2,int param_3)

{
  int iVar1;
  
  if ((((DAT_ram_f0065032 != '\0') && (param_2 != 0)) && (param_3 != 0)) &&
     ((param_1 != 0 && (*(byte *)(param_1 + 0x14) < 3)))) {
    iVar1 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
    (*(code *)&SUB_ram_0006a08c)(param_2 + 4,param_3,6);
    (*(code *)&SUB_ram_0006a08c)(param_2 + 10,param_3 + 6,6);
    (*(code *)&SUB_ram_0006a08c)(param_2 + 0x10,iVar1 + 0x4b,6);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003efac @ 0xf003efac =====


undefined4
FUN_ram_f003efac(byte param_1,undefined4 param_2,int param_3,int param_4,int param_5,int param_6)

{
  int iVar1;
  undefined4 uVar2;
  short *psVar3;
  
  iVar1 = FUN_ram_f0025c48(param_4 + 0xc + param_6);
  if (iVar1 == 0) {
    uVar2 = 0xc0000001;
  }
  else {
    psVar3 = *(short **)(iVar1 + 8);
    *(undefined1 *)(psVar3 + 2) = 0x80;
    *(undefined1 *)((int)psVar3 + 5) = 0;
    *psVar3 = (short)param_4 + 0x10 + (short)param_6;
    *(uint *)(psVar3 + 4) = (uint)param_1;
    *(undefined4 *)(psVar3 + 6) = param_2;
    if (param_3 != 0) {
      (*(code *)&SUB_ram_0006a08c)(psVar3 + 8,param_3,param_4);
    }
    if (param_5 != 0) {
      (*(code *)&SUB_ram_0006a08c)((int)(psVar3 + 4) + param_4 + 8,param_5,param_6);
    }
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(short **)(iVar1 + 0x28) = psVar3;
    psVar3[1] = 1;
    FUN_ram_f0039750(iVar1);
    uVar2 = 0;
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003f058 @ 0xf003f058 =====


void FUN_ram_f003f058(int param_1,undefined1 param_2)

{
  FUN_ram_f0035760();
  *(undefined1 *)(param_1 + 0x2ba) = 1;
  FUN_ram_f003efac(0,param_2,param_1 + 8,1,0,0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003f094 @ 0xf003f094 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003f094(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  
  if ((((DAT_ram_f0065032 == '\0') || (param_1 == 0)) ||
      (iVar3 = *(int *)(param_1 + 0x20), iVar3 == 0)) || ((*(uint *)(iVar3 + 0x10) & 0x80) == 0)) {
LAB_ram_f003f134:
    uVar2 = 0;
  }
  else {
    if (*(byte *)(iVar3 + 0x14) < 3) {
      if (*(char *)(iVar3 + 0x15) == '\x02') {
        if ((*(char *)(iVar3 + 0x38) != '\0') &&
           (iVar4 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar3 + 0x14) * 4),
           *(int *)(iVar4 + 0x1c) != 1)) {
          iVar1 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
          if ((*(int *)(iVar3 + 0x3c8) == 0) || (-1 < (-0x8000 - *(int *)(iVar3 + 0x3c8)) + iVar1))
          {
            *(int *)(iVar3 + 0x3c8) = iVar1;
            iVar4 = FUN_ram_f003eeb0(iVar4,iVar3,0,0);
            if (iVar4 != 0) {
              uVar2 = 2;
              goto LAB_ram_f003f120;
            }
          }
        }
        goto LAB_ram_f003f134;
      }
      uVar2 = 5;
    }
    else {
      uVar2 = 4;
    }
LAB_ram_f003f120:
    FUN_ram_f003f058(iVar3,uVar2);
    uVar2 = 0xc0000001;
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003f140 @ 0xf003f140 =====


void FUN_ram_f003f140(int param_1)

{
  int iVar1;
  
  if ((*(char *)(param_1 + 0x38) != '\0') &&
     (*(int *)(param_1 + 0xb8) + *(int *)(param_1 + 0xac) + *(int *)(param_1 + 0xc4) +
      *(int *)(param_1 + 0xd0) + *(int *)(param_1 + 0xdc) != 0)) {
    if (*(char *)(param_1 + 0x3cc) == '\x01') {
      FUN_ram_f003eeb0(*(undefined4 *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4),
                       param_1,0,0);
    }
    else {
      iVar1 = FUN_ram_f003eeb0(*(undefined4 *)
                                (&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4),param_1,1,
                               0);
      if (iVar1 != 0) {
        FUN_ram_f003f058(param_1,2);
      }
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003f3d0 @ 0xf003f3d0 =====


void FUN_ram_f003f3d0(int param_1,undefined1 param_2)

{
  int iVar1;
  
  if (((((param_1 != 0) && (*(ushort *)(param_1 + 0x12) < 0x1f5)) && (*(int *)(param_1 + 0x20) != 0)
       ) && ((*(int *)(param_1 + 0x14) != 0 && (iVar1 = *(int *)(param_1 + 0x18), iVar1 != 0)))) &&
     ((*(short *)(iVar1 + 6) == 0xd89 &&
      ((*(byte *)(iVar1 + 10) < 3 || (*(byte *)(iVar1 + 10) == 10)))))) {
    FUN_ram_f003efac(1,param_2,*(int *)(param_1 + 0x14),0x1a);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003f498 @ 0xf003f498 =====


void FUN_ram_f003f498(int param_1,undefined1 param_2,undefined1 param_3)

{
  undefined1 local_c [8];
  
  if ((param_1 != 0) && ((*(uint *)(param_1 + 0x10) & 0x80) != 0)) {
    local_c[0] = param_2;
    FUN_ram_f003efac(2,param_3,param_1 + 8,1,local_c,1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003f6a4 @ 0xf003f6a4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003f6a4(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint local_18 [2];
  
  if (DAT_ram_f0065032 == '\0') {
    return;
  }
  if (param_1 == 0) {
    return;
  }
  (*_DAT_ram_00014830)(_DAT_ram_00014830);
  _DAT_ram_f0065124 = _DAT_ram_f0065124 + 1;
  iVar1 = FUN_ram_f002653c(0,param_1 + 0x20);
  if (iVar1 == 0) {
    return;
  }
  if (2 < *(byte *)(iVar1 + 0x14)) {
    return;
  }
  iVar2 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar1 + 0x14) * 4);
  *(undefined1 *)(iVar2 + 0x1dd) = 5;
  *(undefined1 *)(iVar2 + 0x1f0) = 7;
  *(undefined1 *)(iVar2 + 0x370) = 0;
  *(undefined1 *)(iVar1 + 0x49c) = 0;
  *(undefined1 *)(iVar1 + 0x4a1) = *(undefined1 *)(iVar1 + 0x4a4);
  *(undefined1 *)(iVar1 + 0x4a2) = *(undefined1 *)(iVar1 + 0x4a5);
  *(undefined1 *)(iVar1 + 0x4a3) = *(undefined1 *)(iVar1 + 0x4a6);
  if (*(char *)(iVar1 + 0x4ae) == '\0') {
    *(undefined1 *)(iVar1 + 0x4a0) = 1;
  }
  *(undefined1 *)(iVar1 + 0x3cd) = 1;
  if (*(char *)(iVar1 + 0x4ae) == '\0') {
    *(undefined1 *)(iVar1 + 0x37) = 0;
    *(undefined1 *)(iVar1 + 0x38) = 0;
    FUN_ram_f0034724(iVar1);
  }
  FUN_ram_f00266d8(iVar1 + 0x3dc);
  FUN_ram_f00266d8(iVar1 + 0x3f4);
  if (*(char *)(iVar1 + 0x4ae) != '\0') goto LAB_ram_f003f81c;
  FUN_ram_f0022800(iVar1,5,0,1,&LAB_ram_f0041498);
  (*_DAT_ram_00014bf0)(0,local_18,_DAT_ram_00014bf0);
  uVar4 = (local_18[0] & 0x3fffc00) >> 10;
  uVar3 = (*_DAT_ram_00014c08)(0,_DAT_ram_00014c08);
  if (uVar3 < uVar4) {
    uVar3 = 10000;
LAB_ram_f003f7ca:
    if (*(char *)(iVar1 + 0x49f) == '\x01') {
      uVar3 = uVar3 >> 1;
      if (uVar3 < 10000) goto LAB_ram_f003f7da;
      if (90000 < uVar3) {
        uVar3 = 90000;
      }
    }
    else {
      uVar3 = uVar3 - 3000;
    }
  }
  else {
    uVar3 = (uVar3 - uVar4) * 0x400;
    if (9999 < uVar3) goto LAB_ram_f003f7ca;
LAB_ram_f003f7da:
    uVar3 = 10000;
  }
  FUN_ram_f00266d8(iVar1 + 0x480);
  *(undefined1 *)(iVar1 + 0x498) = 0;
  FUN_ram_f002672c(iVar1 + 0x480,uVar3 / 1000);
LAB_ram_f003f81c:
  if (*(char *)(iVar1 + 0x4ae) == '\x01') {
    *(undefined1 *)(iVar1 + 0x4ae) = 0;
    FUN_ram_f002672c(iVar1 + 0x448);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003f8b0 @ 0xf003f8b0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003f8b0(int param_1)

{
  undefined1 uVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar2 = FUN_ram_f0025b48(0,0x10);
  if (iVar2 == 0) {
    if (DAT_ram_f0065035 < 0xb) {
      DAT_ram_f0065035 = DAT_ram_f0065035 + 1;
      *(undefined1 *)(param_1 + 0x4ae) = 0;
      uVar3 = 10;
    }
    else {
      DAT_ram_f0065035 = DAT_ram_f0065035 + 1;
      *(undefined1 *)(param_1 + 0x4ae) = 0;
      uVar3 = 1000;
    }
    FUN_ram_f002672c(param_1 + 0x448,uVar3,FUN_ram_f002672c);
    uVar3 = 0xc0000001;
  }
  else {
    DAT_ram_f0065035 = '\0';
    *(undefined4 *)(iVar2 + 8) = 8;
    uVar1 = *(undefined1 *)(param_1 + 0x14);
    *(undefined1 *)(iVar2 + 0xd) = 0;
    *(undefined1 *)(iVar2 + 0xc) = uVar1;
    (*_DAT_ram_00014830)(_DAT_ram_00014830);
    FUN_ram_f00266d8(param_1 + 0x448);
    FUN_ram_f00266d8(param_1 + 0x480);
    *(undefined1 *)(param_1 + 0x498) = 0;
    FUN_ram_f003718c(10,4);
    uVar3 = 5000000;
    if (*(byte *)(param_1 + 0x4a4) < 0xf) {
      uVar3 = 0x24ba58;
    }
    FUN_ram_f004571c(uVar3,*(undefined1 *)
                            (*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) +
                            0x2cc),
                     *(undefined4 *)
                      (*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x324),0)
    ;
    FUN_ram_f0030278(*(undefined1 *)(param_1 + 0x14),0);
    FUN_ram_f002be74(*(undefined1 *)(param_1 + 0x14),0,0);
    FUN_ram_f0025cac(iVar2);
    uVar3 = 0;
  }
  return uVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003fa48 @ 0xf003fa48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003fa48(int param_1)

{
  int iVar1;
  undefined1 auStack_48 [8];
  undefined4 uStack_40;
  byte local_3c;
  undefined1 local_3b;
  byte local_3a;
  uint uStack_38;
  undefined4 uStack_34;
  undefined4 local_30;
  uint uStack_2c;
  undefined4 uStack_28;
  undefined4 local_20 [3];
  
  (*_DAT_ram_00014830)(param_1,_DAT_ram_00014830);
  if (((DAT_ram_f0065032 != '\0') && (param_1 != 0)) &&
     (local_3c = *(byte *)(param_1 + 0x14), local_3c < 3)) {
    uStack_40 = 7;
    local_3b = 0;
    local_3a = *(byte *)(param_1 + 0x4a4);
    uStack_34 = 5000000;
    uStack_38 = (uint)*(byte *)(param_1 + 0x4a6);
    if (local_3a < 0xf) {
      uStack_34 = 0x24ba58;
    }
    local_30 = 0;
    (*(code *)&SUB_ram_0006a08c)(local_20,param_1 + 10,6);
    uStack_28 = local_20[0];
    uStack_2c = (*_DAT_ram_00014c08)(0,_DAT_ram_00014c08);
    if (uStack_2c < *(ushort *)(param_1 + 0x4ac)) {
      uStack_2c = (uint)*(ushort *)(param_1 + 0x4ac);
    }
    uStack_2c = uStack_2c >> 10;
    iVar1 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
    *(undefined1 *)(iVar1 + 0x1f0) = 1;
    *(undefined1 *)(iVar1 + 0x1dd) = 1;
    *(undefined1 *)(param_1 + 0x49e) = 1;
    DAT_ram_f0065004 = FUN_ram_f0023b48(*(undefined1 *)(param_1 + 0x14),uStack_34);
    FUN_ram_f004571c(uStack_34,local_3a,uStack_38,0);
    FUN_ram_f003f6a4(auStack_48,0,uStack_2c);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003fb50 @ 0xf003fb50 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f003fb50(int param_1)

{
  undefined1 uVar1;
  byte bVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  
  if (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x2cc) ==
      *(char *)(param_1 + 0x4a4)) {
    FUN_ram_f00266d8(param_1 + 0x480);
    *(undefined1 *)(param_1 + 0x498) = 0;
    FUN_ram_f002672c(param_1 + 0x480,1);
    *(undefined1 *)(param_1 + 0x498) = 1;
  }
  else if (*(char *)(param_1 + 0x49e) != '\x01') {
    if (*(char *)(param_1 + 0x4a1) == *(char *)(param_1 + 0x4a4)) {
      FUN_ram_f00266d8(param_1 + 0x448);
    }
    else {
      *(undefined1 *)(param_1 + 0x4a0) = 0;
      FUN_ram_f00266d8(param_1 + 0x480);
      *(undefined1 *)(param_1 + 0x498) = 0;
      *(undefined1 *)(param_1 + 0x499) = 0;
      *(undefined1 *)(param_1 + 0x4ae) = 0;
      FUN_ram_f002672c(param_1 + 0x448,(*(ushort *)(param_1 + 0x4ac) >> 10) + 5);
      iVar3 = FUN_ram_f0025b48(0,0x28);
      if (iVar3 == 0) {
        *(undefined1 *)(param_1 + 0x499) = 1;
      }
      else {
        *(undefined4 *)(iVar3 + 8) = 7;
        uVar1 = *(undefined1 *)(param_1 + 0x14);
        *(undefined1 *)(iVar3 + 0xd) = 0;
        *(undefined1 *)(iVar3 + 0xc) = uVar1;
        uVar5 = 5000000;
        *(undefined1 *)(iVar3 + 0xe) = *(undefined1 *)(param_1 + 0x4a4);
        *(uint *)(iVar3 + 0x10) = (uint)*(byte *)(param_1 + 0x4a6);
        bVar2 = *(byte *)(param_1 + 0x4a4);
        *(undefined4 *)(iVar3 + 0x18) = 0;
        if (bVar2 < 0xf) {
          uVar5 = 0x24ba58;
        }
        *(undefined4 *)(iVar3 + 0x14) = uVar5;
        (*(code *)&SUB_ram_0006a08c)(iVar3 + 0x20,param_1 + 10,6);
        uVar4 = (*_DAT_ram_00014c08)(0,_DAT_ram_00014c08);
        *(uint *)(iVar3 + 0x1c) = uVar4;
        if (uVar4 < *(ushort *)(param_1 + 0x4ac)) {
          *(uint *)(iVar3 + 0x1c) = (uint)*(ushort *)(param_1 + 0x4ac);
        }
        *(uint *)(iVar3 + 0x1c) = *(uint *)(iVar3 + 0x1c) >> 10;
        iVar6 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
        *(undefined1 *)(iVar6 + 0x1dd) = 1;
        *(undefined1 *)(iVar6 + 0x1f0) = 1;
        *(undefined1 *)(param_1 + 0x49e) = 1;
        (*_DAT_ram_00014830)(_DAT_ram_00014830);
        DAT_ram_f0065034 = DAT_ram_f0065034 + '\x01';
        *(undefined1 *)(param_1 + 0x3a) = 0;
        *(undefined1 *)(param_1 + 0x55) = 0;
        FUN_ram_f002fd54(*(undefined1 *)(iVar6 + 0x19));
        FUN_ram_f002be74(*(undefined1 *)(iVar6 + 0x19),1,0);
        FUN_ram_f003fa48(param_1);
        DAT_ram_f0064fc4 = 1;
        FUN_ram_f0025cac(iVar3);
        *(undefined1 *)(param_1 + 0x47c) = 1;
        FUN_ram_f00266d8(param_1 + 0x464);
      }
    }
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003fddc @ 0xf003fddc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f003fddc(int param_1)

{
  char cVar1;
  byte bVar2;
  undefined4 uVar3;
  int iVar4;
  undefined1 local_48;
  undefined1 local_47;
  undefined1 local_3b;
  char local_3a;
  undefined1 local_39;
  undefined1 auStack_38 [20];
  undefined1 local_24;
  undefined1 local_23;
  undefined2 local_22;
  undefined2 local_20;
  
  if (((DAT_ram_f0065032 != '\0') && (param_1 != 0)) && (*(byte *)(param_1 + 0x14) < 3)) {
    iVar4 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
    if (*(int *)(iVar4 + 0xa8) == 3) {
      cVar1 = *(char *)(param_1 + 0x429);
      if (cVar1 == '\x01') {
        (*(code *)&SUB_ram_0006a0a0)(&local_48,0,0x2e);
        local_48 = *(undefined1 *)(param_1 + 0x4a4);
        local_47 = *(undefined1 *)(param_1 + 0x4a5);
        FUN_ram_f003e6f8(param_1,auStack_38);
        local_3b = 0x3e;
        local_39 = *(undefined1 *)(param_1 + 0x4a6);
        local_24 = 0x68;
        local_23 = 4;
        local_22 = 15000;
        local_20 = 21000;
        local_3a = cVar1;
        FUN_ram_f003e76c(iVar4,param_1,5,&local_48,0);
        if (*(char *)(param_1 + 0x460) == '\x01') {
          FUN_ram_f002672c(param_1 + 0x464,2000);
        }
      }
      else if (cVar1 == '\x02') {
        iVar4 = FUN_ram_f003ef14(iVar4,param_1,&DAT_ram_f006d74c);
        if (iVar4 == 0) {
          uVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
          *(undefined4 *)(param_1 + 0x444) = uVar3;
        }
        *(bool *)(param_1 + 0x49c) = iVar4 == 0;
      }
      *(undefined1 *)(param_1 + 0x429) = 0;
    }
    else {
      bVar2 = *(char *)(param_1 + 0x428) + 1;
      *(byte *)(param_1 + 0x428) = bVar2;
      if (10 < bVar2) {
        FUN_ram_f002eb2c(iVar4);
      }
      FUN_ram_f002672c(param_1 + 0x410,2);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f003ff24 @ 0xf003ff24 =====


void FUN_ram_f003ff24(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4);
  *(undefined1 *)(param_1 + 0x428) = 0;
  *(undefined1 *)(param_1 + 0x429) = 1;
  if (*(int *)(iVar1 + 0xa8) == 3) {
    FUN_ram_f003fddc();
  }
  else {
    FUN_ram_f002eb2c(iVar1);
    FUN_ram_f002672c(param_1 + 0x410,2);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0040174 @ 0xf0040174 =====


void FUN_ram_f0040174(int param_1,int param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 auStack_40 [2];
  undefined2 local_3e;
  undefined1 auStack_3b [43];
  
  if (param_1 == 0) {
    if (param_2 == 0) {
      return;
    }
    uVar1 = 0;
  }
  else {
    uVar1 = (uint)*(byte *)(param_1 + 0x14);
    if (2 < uVar1) {
      return;
    }
  }
  uVar2 = *(undefined4 *)(&DAT_ram_020a0068 + uVar1 * 4);
  (*(code *)&SUB_ram_0006a0a0)(auStack_40,0,0x2e);
  if (param_2 != 0) {
    (*(code *)&SUB_ram_0006a08c)(auStack_3b,param_2,6);
  }
  local_3e = 0x1a;
  FUN_ram_f003e76c(uVar2,param_1,3,auStack_40,0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00401e4 @ 0xf00401e4 =====


undefined4 FUN_ram_f00401e4(int param_1,short param_2,undefined1 *param_3)

{
  byte bVar1;
  short sVar2;
  byte bVar3;
  undefined4 uVar4;
  undefined1 *puVar5;
  byte *pbVar6;
  byte *pbVar7;
  
  if (param_2 < 0xd) {
LAB_ram_f004033e:
    uVar4 = 0xc0000001;
  }
  else {
    (*(code *)&SUB_ram_0006a0a0)(param_3,0,0x2e);
    bVar1 = *(byte *)(param_1 + 10);
    param_2 = param_2 + -0xb;
    puVar5 = (undefined1 *)(param_1 + 0xb);
    if (bVar1 == 5) {
      if (param_2 < 2) goto LAB_ram_f004033e;
      *param_3 = *puVar5;
      param_3[1] = *(undefined1 *)(param_1 + 0xc);
LAB_ram_f0040246:
      sVar2 = -2;
      pbVar7 = (byte *)(param_1 + 0xd);
    }
    else {
      if (bVar1 < 6) {
        bVar3 = 4;
      }
      else {
        if (bVar1 == 6) {
          if (2 < param_2) {
            (*(code *)&SUB_ram_0006a08c)(param_3 + 2,puVar5,2);
            goto LAB_ram_f0040246;
          }
          goto LAB_ram_f004033e;
        }
        bVar3 = 9;
      }
      if ((bVar1 != bVar3) || (param_2 < 1)) goto LAB_ram_f004033e;
      pbVar7 = (byte *)(param_1 + 0xc);
      sVar2 = -1;
      param_3[4] = *puVar5;
    }
    for (param_2 = param_2 + sVar2; 0 < param_2; param_2 = (param_2 + -2) - (ushort)bVar3) {
      bVar1 = *pbVar7;
      bVar3 = pbVar7[1];
      pbVar6 = pbVar7 + 2;
      if (bVar1 == 0x68) {
        param_3[0x24] = 0x68;
        param_3[0x25] = bVar3;
        (*(code *)&SUB_ram_0006a08c)(param_3 + 0x26,pbVar6,2);
        pbVar7 = pbVar7 + 4;
        uVar4 = 2;
        puVar5 = param_3 + 0x28;
LAB_ram_f0040318:
        (*(code *)&SUB_ram_0006a08c)(puVar5,pbVar7,uVar4);
      }
      else if (bVar1 < 0x69) {
        if (bVar1 == 0x3e) {
          param_3[0xd] = 0x3e;
          param_3[0xe] = bVar3;
          param_3[0xf] = *pbVar6;
        }
        else if (bVar1 == 0x65) {
          param_3[0x10] = 0x65;
          param_3[0x11] = bVar3;
          (*(code *)&SUB_ram_0006a08c)(param_3 + 0x12,pbVar6,6);
          (*(code *)&SUB_ram_0006a08c)(param_3 + 0x18,pbVar7 + 8,6);
          pbVar7 = pbVar7 + 0xe;
          uVar4 = 6;
          puVar5 = param_3 + 0x1e;
          goto LAB_ram_f0040318;
        }
      }
      else if (bVar1 == 0x6a) {
        param_3[0x2a] = 0x6a;
        param_3[0x2b] = bVar3;
        param_3[0x2c] = *pbVar6;
      }
      pbVar7 = pbVar6 + bVar3;
    }
    uVar4 = 0;
  }
  return uVar4;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004052c @ 0xf004052c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004052c(int param_1,int param_2,byte param_3,uint param_4,int param_5,uint param_6)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  if ((param_1 != 0) && ((*(uint *)(param_1 + 0x10) & 0x80) != 0)) {
    if (param_3 == 2) {
      if (**(char **)(param_2 + 0x14) == -0x78) {
        if (*(short *)(*(int *)(param_2 + 0x18) + 6) == 0xd89) {
          if (*(char *)(*(int *)(param_2 + 0x18) + 10) == '\t') {
            _DAT_ram_f006e540 = _DAT_ram_f006e540 + 1;
          }
        }
        else {
          _DAT_ram_f006e544 = _DAT_ram_f006e544 + 1;
          if ((param_5 == 2) || (param_5 - 0x28U < 0x1e)) {
            _DAT_ram_f006e550 = _DAT_ram_f006e550 | 1;
          }
          else if (param_5 == 3) {
            _DAT_ram_f006e550 = _DAT_ram_f006e550 | 2;
          }
          else if (param_5 == 4) {
            _DAT_ram_f006e550 = _DAT_ram_f006e550 | 4;
          }
          else if (param_5 == 5) {
            _DAT_ram_f006e550 = _DAT_ram_f006e550 | 0x10;
          }
          else if (param_5 == 6) {
            _DAT_ram_f006e550 = _DAT_ram_f006e550 | 0x20;
          }
          else {
            _DAT_ram_f006e550 = _DAT_ram_f006e550 | 0x80000000;
          }
        }
      }
    }
    else if (param_3 < 3) {
      if (param_3 == 0) {
        if (*(char *)(param_2 + 0x10) == '\x01') {
          bVar1 = **(byte **)(param_2 + 0x14) & 0xc ^ 8;
        }
        else {
          bVar1 = *(byte *)(param_2 + 0x5b);
        }
        if (bVar1 == 0) {
          _DAT_ram_f006e52c = _DAT_ram_f006e52c + 1;
          iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
          if (-1 < (-0x50000 - *(int *)(param_1 + 0x36c)) + iVar2) {
            if (_DAT_ram_f006e568 == 0) {
              iVar4 = 9;
            }
            else {
              iVar4 = _DAT_ram_f006e568 - 1;
            }
            iVar3 = FUN_ram_f003ccc4(param_1);
            *(char *)(iVar4 * 2 + -0xff91aab) = (char)(iVar3 >> 1);
            FUN_ram_f003efac(3,*(undefined1 *)(param_1 + 8),&DAT_ram_f006e52c,0x270,0,0);
            *(int *)(param_1 + 0x36c) = iVar2;
            _DAT_ram_f006e538 = 0;
          }
        }
      }
      else if ((param_3 == 1) && (**(char **)(param_2 + 0x14) == -0x78)) {
        if (*(short *)(*(int *)(param_2 + 0x18) + 6) == 0xd89) {
          if (*(char *)(*(int *)(param_2 + 0x18) + 10) == '\t') {
            _DAT_ram_f006e53c = _DAT_ram_f006e53c + 1;
          }
        }
        else {
          _DAT_ram_f006e534 = param_5 + _DAT_ram_f006e534;
          _DAT_ram_f006e530 = _DAT_ram_f006e530 + 1;
          if (_DAT_ram_f006e538 < param_6) {
            _DAT_ram_f006e538 = param_6;
          }
          if (param_4 < 0x20) {
            *(int *)(&DAT_ram_f006e52c + (param_4 + 0x10) * 4) =
                 *(int *)(&DAT_ram_f006e52c + (param_4 + 0x10) * 4) + 1;
          }
          if (9 < _DAT_ram_f006e568) {
            _DAT_ram_f006e568 = 0;
          }
          (&DAT_ram_f006e52c)[(_DAT_ram_f006e568 + 0x14) * 2] = (char)param_4;
          *(undefined1 *)(_DAT_ram_f006e568 * 2 + -0xff91aab) = 0;
          _DAT_ram_f006e568 = _DAT_ram_f006e568 + 1;
        }
      }
    }
    else if (param_3 == 3) {
      _DAT_ram_f006e548 = _DAT_ram_f006e548 + 1;
    }
    else if (param_3 == 0x11) {
      _DAT_ram_f006e54c = _DAT_ram_f006e54c + 1;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0040780 @ 0xf0040780 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0040780(ushort *param_1,int param_2,undefined2 param_3,int param_4)

{
  byte bVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  undefined4 uVar8;
  undefined1 uVar9;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  undefined4 local_38;
  undefined4 uStack_34;
  undefined4 uStack_30;
  undefined4 uStack_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined2 uStack_1c;
  
  if ((((param_1 != (ushort *)0x0) && (param_2 != 0)) && (param_4 != 0)) &&
     (uVar3 = (uint)*(byte *)(param_4 + 0x14), uVar3 < 3)) {
    if (*(short *)(param_2 + 6) == 0xd89) {
      if ((*(char *)(param_2 + 8) == '\x02') && (*(char *)(param_2 + 9) == '\f')) {
        iVar4 = *(int *)(&DAT_ram_020a0068 + uVar3 * 4);
        if ((*(char *)(param_4 + 0x4a1) == '\0') ||
           ((*(char *)(iVar4 + 0x2cc) == *(char *)(param_4 + 0x4a1) ||
            ((byte)(*(char *)(param_2 + 10) - 5U) < 2)))) {
          bVar1 = *(byte *)(param_2 + 10);
          if (bVar1 == 4) {
            if (*(char *)(param_4 + 0x15) == '\x02') {
              iVar4 = FUN_ram_f002653c(*(undefined1 *)(iVar4 + 0x19),param_1 + 8);
              if ((((iVar4 != 0) && (*(byte *)(iVar4 + 0x14) < 3)) &&
                  ((*(uint *)(iVar4 + 0x10) & 0x80) != 0)) &&
                 (((iVar5 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar4 + 0x14) * 4),
                   *(char *)(iVar5 + 0xb0) == '\0' && (*(int *)(iVar5 + 0x1c) != 1)) &&
                  (iVar6 = FUN_ram_f00401e4(param_2,param_3,&local_48), iVar6 == 0)))) {
                iVar5 = FUN_ram_f003e76c(iVar5,iVar4,9,&local_48,0);
                if (iVar5 != 0) {
                  FUN_ram_f002672c(iVar4 + 0x3b0,100);
                }
                *(undefined1 *)(iVar4 + 0x3ce) = 1;
                *(undefined1 *)(iVar4 + 0x3d1) = (undefined1)local_44;
              }
            }
          }
          else if (bVar1 < 5) {
            if (bVar1 == 1) {
              iVar4 = FUN_ram_f002653c(*(undefined1 *)(iVar4 + 0x19),param_1 + 8);
              if (iVar4 == 0) {
                return 0xc0000001;
              }
              if ((*(uint *)(iVar4 + 0x10) & 0x80) == 0) {
                return 0xc0000001;
              }
              uVar9 = 0;
            }
            else {
              if (bVar1 != 2) {
                return 0xc0000001;
              }
              iVar4 = FUN_ram_f002653c(*(undefined1 *)(iVar4 + 0x19),param_1 + 8);
              if (iVar4 == 0) {
                return 0xc0000001;
              }
              if ((*(uint *)(iVar4 + 0x10) & 0x80) == 0) {
                return 0xc0000001;
              }
              uVar9 = 1;
            }
            *(undefined1 *)(iVar4 + 0x371) = uVar9;
          }
          else if (bVar1 == 6) {
            if (*(char *)(param_4 + 0x15) == '\x02') {
              if ((*param_1 & 0x200) == 0) {
                param_1 = param_1 + 5;
              }
              else {
                param_1 = param_1 + 8;
              }
              iVar4 = FUN_ram_f002653c(*(undefined1 *)(iVar4 + 0x19),param_1,2,FUN_ram_f002653c);
              if ((iVar4 != 0) && (*(byte *)(iVar4 + 0x14) < 3)) {
                if ((*(uint *)(iVar4 + 0x10) & 0x80) != 0) {
                  cVar2 = *(char *)(iVar4 + 0x4a1);
                  iVar5 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar4 + 0x14) * 4);
                  if (((cVar2 == '\0') || (*(char *)(iVar5 + 0x2cc) == cVar2)) &&
                     (*(char *)(iVar4 + 0x4a4) == *(char *)(iVar5 + 0x2cc))) {
                    return 0xc0000001;
                  }
                  if (*(char *)(iVar4 + 0x429) != '\0') {
                    if (*(char *)(iVar4 + 0x429) != '\x02') {
                      return 0xc0000001;
                    }
                    FUN_ram_f003f8b0(iVar4);
                    return 0xc0000001;
                  }
                  if (*(char *)(iVar4 + 0x4a4) == *(char *)(iVar5 + 0x2cc)) {
                    return 0xc0000001;
                  }
                  if ((((cVar2 != '\0') && (cVar2 != *(char *)(iVar5 + 0x2cc))) ||
                      ((*(char *)(iVar5 + 0xb0) == '\0' && (*(int *)(iVar5 + 0x1c) != 1)))) &&
                     (FUN_ram_f00401e4(param_2,param_3,&local_48), local_48._2_2_ == 0)) {
                    if ((*(char *)(iVar4 + 0x4a1) != '\0') &&
                       (*(char *)(iVar4 + 0x4a1) != *(char *)(iVar5 + 0x2cc))) {
                      *(char *)(iVar4 + 0x4a4) = *(char *)(iVar5 + 0x2cc);
                    }
                    (*_DAT_ram_00014830)(_DAT_ram_00014830);
                    *(ushort *)(iVar4 + 0x4aa) = local_24._2_2_;
                    *(undefined2 *)(iVar4 + 0x4ac) = (undefined2)local_20;
                    if ((*(char *)(iVar4 + 0x4a1) != '\0') &&
                       (*(char *)(iVar5 + 0x2cc) != *(char *)(iVar4 + 0x4a1))) {
                      uVar8 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
                      *(undefined4 *)(iVar4 + 0x444) = uVar8;
                      *(undefined1 *)(iVar4 + 0x49c) = 1;
                    }
                    *(undefined1 *)(iVar4 + 0x49f) = 1;
                    FUN_ram_f003fb50(iVar4);
                    return 0xc0000001;
                  }
                }
                *(undefined1 *)(iVar4 + 0x49c) = 0;
              }
            }
          }
          else if (bVar1 < 6) {
            if (*(char *)(param_4 + 0x15) == '\x02') {
              if ((*param_1 & 0x200) == 0) {
                param_1 = param_1 + 5;
              }
              else {
                param_1 = param_1 + 8;
              }
              iVar4 = FUN_ram_f002653c(*(undefined1 *)(iVar4 + 0x19),param_1,2,FUN_ram_f002653c);
              if (((iVar4 != 0) && (*(byte *)(iVar4 + 0x14) < 3)) &&
                 (((*(uint *)(iVar4 + 0x10) & 0x80) != 0 && (*(char *)(iVar4 + 0x429) == '\0')))) {
                iVar5 = *(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(iVar4 + 0x14) * 4);
                FUN_ram_f00401e4(param_2,param_3,&local_48);
                if (*(char *)(iVar4 + 0x49a) == '\x01') {
                  local_48 = CONCAT22(0x25,(ushort)local_48);
                  FUN_ram_f003ef14(iVar5,iVar4,&local_48);
                }
                else {
                  uVar3 = local_48 & 0xff;
                  if ((*(char *)(iVar4 + 0x49c) == '\x01') &&
                     (iVar6 = (*_DAT_ram_00014830)(_DAT_ram_00014830),
                     -1 < (-0x50000 - *(int *)(iVar4 + 0x444)) + iVar6)) {
                    *(undefined1 *)(iVar4 + 0x49e) = 0;
                    *(undefined1 *)(iVar4 + 0x49c) = 0;
                    *(undefined1 *)(iVar4 + 0x4a0) = 0;
                  }
                  if ((*(char *)(iVar4 + 0x49c) == '\0') && (*(char *)(iVar4 + 0x429) == '\0')) {
                    uVar7 = (uint)*(byte *)(iVar4 + 0x4a1);
                    if ((uVar7 == 0) || (uVar7 == *(byte *)(iVar5 + 0x2cc))) {
                      if (uVar3 == uVar7) {
                        return 0xc0000001;
                      }
                    }
                    else if (uVar3 != *(byte *)(iVar5 + 0x2cc)) {
                      return 0xc0000001;
                    }
                    (*_DAT_ram_00014830)(_DAT_ram_00014830);
                    DAT_ram_f0065036 = DAT_ram_f0065036 + '\x01';
                    *(undefined1 *)(iVar4 + 0x4a4) = (undefined1)local_48;
                    *(undefined1 *)(iVar4 + 0x4a5) = local_48._1_1_;
                    *(undefined1 *)(iVar4 + 0x4a6) = uStack_3c._3_1_;
                    *(ushort *)(iVar4 + 0x4aa) = local_24._2_2_;
                    *(undefined2 *)(iVar4 + 0x4ac) = (undefined2)local_20;
                    if (local_24._2_2_ < 15000) {
                      *(undefined2 *)(iVar4 + 0x4aa) = 15000;
                    }
                    if (*(ushort *)(iVar4 + 0x4ac) < 21000) {
                      *(undefined2 *)(iVar4 + 0x4ac) = 21000;
                    }
                    local_48 = (uint)(ushort)local_48;
                    local_24 = CONCAT22(*(undefined2 *)(iVar4 + 0x4aa),0x468);
                    local_20 = CONCAT22(local_20._2_2_,*(undefined2 *)(iVar4 + 0x4ac));
                    if (*(int *)(iVar5 + 0xa8) == 3) {
                      FUN_ram_f003e6f8(iVar4,&local_38);
                      iVar5 = FUN_ram_f003ef14(iVar5,iVar4,&local_48);
                      if (iVar5 == 0) {
                        uVar8 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
                        *(undefined1 *)(iVar4 + 0x49c) = 1;
                        *(undefined4 *)(iVar4 + 0x444) = uVar8;
                      }
                    }
                    else {
                      uVar8 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
                      *(undefined4 *)(iVar4 + 0x444) = uVar8;
                      *(undefined1 *)(iVar4 + 0x49c) = 1;
                      FUN_ram_f002eb2c(iVar5);
                      _DAT_ram_f006d74c = local_48;
                      _DAT_ram_f006d750 = local_44;
                      _DAT_ram_f006d754 = uStack_40;
                      _DAT_ram_f006d758 = uStack_3c;
                      _DAT_ram_f006d75c = local_38;
                      _DAT_ram_f006d760 = uStack_34;
                      _DAT_ram_f006d764 = uStack_30;
                      _DAT_ram_f006d768 = uStack_2c;
                      _DAT_ram_f006d76c = local_28;
                      _DAT_ram_f006d770 = local_24;
                      _DAT_ram_f006d774 = local_20;
                      _DAT_ram_f006d778 = uStack_1c;
                      *(undefined1 *)(iVar4 + 0x428) = 0;
                      *(undefined1 *)(iVar4 + 0x429) = 2;
                      FUN_ram_f002672c(iVar4 + 0x410,2);
                    }
                  }
                }
              }
            }
          }
          else if ((bVar1 == 9) &&
                  (FUN_ram_f004052c(param_4,0,0x11,0,0,0), *(char *)(param_4 + 0x15) == '\x02')) {
            *(undefined1 *)(param_4 + 0x3cd) = 0;
            *(undefined1 *)(param_4 + 0x3cc) = 0;
            FUN_ram_f00266d8(param_4 + 0x37c);
            FUN_ram_f00266d8(param_4 + 0x398);
            FUN_ram_f00266d8(param_4 + 0x3b0);
            FUN_ram_f003f140(param_4);
            *(undefined4 *)(param_4 + 0x3c8) = 0;
          }
        }
      }
    }
    else if ((*(uint *)(param_4 + 0x10) & 0x80) == 0) {
      if (((DAT_ram_f0065032 != '\0') && (*(short *)(param_2 + 6) != 0x608)) &&
         (((param_1[2] & 1) == 0 &&
          ((iVar4 = FUN_ram_f002653c(uVar3,param_1 + 8), iVar4 != 0 &&
           ((*(uint *)(iVar4 + 0x10) & 0x80) != 0)))))) {
        iVar5 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
        if (*(int *)(iVar4 + 0x374) != 0) {
          if ((int)((iVar5 - *(int *)(iVar4 + 0x374)) + (uint)DAT_ram_f0064fc5 * -0x8000) < 0) {
            return 0xc0000001;
          }
          FUN_ram_f003f058(iVar4,6);
          iVar5 = 0;
        }
        *(int *)(iVar4 + 0x374) = iVar5;
      }
    }
    else {
      *(undefined4 *)(param_4 + 0x374) = 0;
      FUN_ram_f004052c(param_4,0,3,0,0,0);
    }
  }
  return 0xc0000001;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0040d3c @ 0xf0040d3c =====


void FUN_ram_f0040d3c(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (param_1 != 0) {
    FUN_ram_f00266d8(param_1 + 0x37c);
    FUN_ram_f00266d8(param_1 + 0x398);
    FUN_ram_f00266d8(param_1 + 0x3b0);
    FUN_ram_f00266d8(param_1 + 0x3dc);
    FUN_ram_f00266d8(param_1 + 0x3f4);
    FUN_ram_f00266d8(param_1 + 0x410);
    FUN_ram_f00266d8(param_1 + 0x448);
    FUN_ram_f00266d8(param_1 + 0x480);
    FUN_ram_f00266d8(param_1 + 0x464);
    FUN_ram_f00266d8(param_1 + 0x42c);
    DAT_ram_f0064fc4 = 0;
    if ((*(uint *)(param_1 + 0x10) & 0x80) != 0) {
      if (*(byte *)(param_1 + 0x14) < 3) {
        if ((*(char *)(param_1 + 0x4a1) != '\0') &&
           (*(char *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0x2cc) !=
            *(char *)(param_1 + 0x4a1))) {
          FUN_ram_f003f8b0(param_1);
        }
      }
      *(undefined1 *)(param_1 + 0x15) = 0;
      *(undefined4 *)(param_1 + 0x10) = 0;
      DAT_ram_f0065032 = '\0';
      iVar1 = 0;
      do {
        iVar2 = iVar1 + 1;
        iVar1 = FUN_ram_f0025d90(iVar1);
        if (((iVar1 != 0) && ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) &&
           (*(char *)(iVar1 + 0x15) == '\x02')) {
          DAT_ram_f0065032 = DAT_ram_f0065032 + '\x01';
        }
        iVar1 = iVar2;
      } while (iVar2 != 0x14);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0040e14 @ 0xf0040e14 =====


undefined4 FUN_ram_f0040e14(int param_1)

{
  int iVar1;
  int iVar2;
  
  if ((DAT_ram_f0065032 != '\0') && (param_1 != 0)) {
    iVar1 = 0;
    do {
      iVar2 = iVar1 + 1;
      iVar1 = FUN_ram_f0025d90(iVar1);
      if ((((iVar1 != 0) && ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) &&
          (*(char *)(iVar1 + 0x15) == '\x02')) &&
         (((*(char *)(iVar1 + 0x55) == '\x01' || (*(char *)(iVar1 + 0x3a) == '\x01')) ||
          (*(char *)(iVar1 + 0x3cc) == '\x01')))) {
        return 1;
      }
      iVar1 = iVar2;
    } while (iVar2 != 0x14);
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0040e78 @ 0xf0040e78 =====


undefined4 FUN_ram_f0040e78(int param_1)

{
  int iVar1;
  int iVar2;
  
  if ((DAT_ram_f0065032 != '\0') && (param_1 != 0)) {
    iVar1 = 0;
    do {
      iVar2 = iVar1 + 1;
      iVar1 = FUN_ram_f0025d90(iVar1);
      if ((((iVar1 != 0) && ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) &&
          (*(char *)(iVar1 + 0x15) == '\x02')) && (*(char *)(iVar1 + 0x378) == '\x01')) {
        return 0;
      }
      iVar1 = iVar2;
    } while (iVar2 != 0x14);
  }
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0040ecc @ 0xf0040ecc =====


undefined4 FUN_ram_f0040ecc(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  
  if ((DAT_ram_f0065032 == '\0') || (param_1 == 0)) {
    uVar2 = 0;
  }
  else {
    uVar2 = 0;
    iVar3 = 0;
    do {
      iVar1 = FUN_ram_f0025d90(iVar3);
      iVar3 = iVar3 + 1;
      if (((iVar1 != 0) && ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) &&
         (*(char *)(iVar1 + 0x15) == '\x02')) {
        if ((*(char *)(iVar1 + 0x55) == '\x01') || (*(char *)(iVar1 + 0x3a) == '\x01')) {
          return 1;
        }
        if ((*(char *)(iVar1 + 0x4a1) != '\0') &&
           (*(char *)(param_1 + 0x2cc) != *(char *)(iVar1 + 0x4a1))) {
          uVar2 = 1;
        }
        if (*(char *)(iVar1 + 0x3cc) == '\x01') {
          return 1;
        }
        if (*(char *)(iVar1 + 0x47c) == '\x01') {
          return 1;
        }
      }
    } while (iVar3 != 0x14);
  }
  return uVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0040f4c @ 0xf0040f4c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0040f4c(uint param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint local_28 [3];
  
  if ((DAT_ram_f0065032 != '\0') && (param_1 < 3)) {
    iVar6 = *(int *)(&DAT_ram_020a0068 + param_1 * 4);
    *(undefined1 *)(iVar6 + 0x1dd) = 5;
    iVar5 = 0;
    *(undefined1 *)(iVar6 + 0x1f0) = 7;
    do {
      iVar1 = FUN_ram_f0025d90(iVar5);
      if ((((iVar1 != 0) && (*(byte *)(iVar1 + 0x14) == param_1)) &&
          ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) && (*(char *)(iVar1 + 0x15) == '\x02')) {
        if (((*(char *)(iVar1 + 0x4a1) == '\0') ||
            (*(char *)(iVar1 + 0x4a1) == *(char *)(iVar6 + 0x2cc))) ||
           (*(char *)(iVar1 + 0x4a4) != *(char *)(iVar6 + 0x2cc))) {
          *(undefined1 *)(iVar1 + 0x3cc) = 0;
          *(undefined1 *)(iVar1 + 0x3cd) = 0;
          FUN_ram_f00266d8(iVar1 + 0x37c);
          FUN_ram_f00266d8(iVar1 + 0x398);
          FUN_ram_f00266d8(iVar1 + 0x3b0);
          FUN_ram_f003f140(iVar1);
          FUN_ram_f003eeb0(iVar6,iVar1,1,0);
        }
        else {
          (*_DAT_ram_00014830)(iVar1,_DAT_ram_00014830);
          *(undefined1 *)(iVar1 + 0x4a1) = *(undefined1 *)(iVar1 + 0x4a4);
          *(undefined1 *)(iVar1 + 0x4a2) = *(undefined1 *)(iVar1 + 0x4a5);
          *(undefined1 *)(iVar1 + 0x4a3) = *(undefined1 *)(iVar1 + 0x4a6);
          *(undefined1 *)(iVar1 + 0x38) = 0;
          *(undefined1 *)(iVar1 + 0x49c) = 0;
          FUN_ram_f00266d8(iVar1 + 0x480);
          *(undefined1 *)(iVar1 + 0x498) = 0;
          *(undefined1 *)(iVar1 + 0x49e) = 0;
          FUN_ram_f0022800(iVar1,5,0,1,0);
          if (*(char *)(iVar1 + 0x460) == '\x01') {
            (*_DAT_ram_00014bf0)(0,local_28,_DAT_ram_00014bf0);
            uVar4 = (local_28[0] & 0x3fffc00) >> 10;
            uVar2 = (*_DAT_ram_00014c08)(0,_DAT_ram_00014c08);
            if (uVar4 < uVar2) {
              iVar3 = uVar2 - uVar4;
            }
            else {
              iVar3 = 0;
            }
            uVar2 = (uint)*(ushort *)(iVar6 + 0x6a) * 0x2800 + 5000 + iVar3 * 0x400;
            FUN_ram_f002672c(iVar1 + 0x464,uVar2 / 1000,uVar2 % 1000);
          }
          *(undefined1 *)(iVar1 + 0x3cd) = 0;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 != 0x14);
    (*_DAT_ram_00014830)(_DAT_ram_00014830);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00410f4 @ 0xf00410f4 =====


void FUN_ram_f00410f4(void)

{
  int iVar1;
  int iVar2;
  
  if (DAT_ram_f0065032 != '\0') {
    iVar1 = 0;
    do {
      iVar2 = iVar1 + 1;
      iVar1 = FUN_ram_f0025d90(iVar1);
      if (((iVar1 != 0) && ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) &&
         (*(char *)(iVar1 + 0x15) == '\x02')) {
        *(undefined1 *)(iVar1 + 0x3cd) = 1;
      }
      iVar1 = iVar2;
    } while (iVar2 != 0x14);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004113c @ 0xf004113c =====


void FUN_ram_f004113c(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  do {
    iVar1 = FUN_ram_f0025d90(iVar2);
    if ((((iVar1 != 0) && ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) &&
        (*(char *)(iVar1 + 0x15) == '\x02')) &&
       ((*(char *)(param_1 + 0x370) != '\x01' && (*(char *)(iVar1 + 0x49c) == '\0')))) {
      if ((*(char *)(iVar1 + 0x4a1) != '\0') &&
         (*(char *)(param_1 + 0x2cc) != *(char *)(iVar1 + 0x4a1))) {
        return;
      }
      if (param_2 == 1) {
        *(undefined1 *)(iVar1 + 0x37) = 1;
      }
      else {
        *(undefined1 *)(iVar1 + 0x37) = 0;
      }
      FUN_ram_f00347c4(1,0x7f,iVar1,0);
      if ((*(int *)(param_1 + 0x1c) != 1) && (*(char *)(iVar1 + 0x378) != '\x01')) {
        if (*(char *)(iVar1 + 0x38) == '\0') {
          FUN_ram_f0022800(iVar1,5,0,0,FUN_ram_f002fff8);
        }
        else {
          FUN_ram_f003eeb0(param_1,iVar1,0,0);
        }
        if ((param_2 == 1) && ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) {
          *(undefined1 *)(iVar1 + 0x378) = 1;
        }
      }
    }
    iVar2 = iVar2 + 1;
    if (iVar2 == 0x14) {
      return;
    }
  } while( true );
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0041210 @ 0xf0041210 =====


void FUN_ram_f0041210(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (((param_1 != 0) && (*(byte *)(param_1 + 0x14) < 3)) &&
     ((*(uint *)(param_1 + 0x10) & 0x80) != 0)) {
    FUN_ram_f002672c(param_1 + 0x3dc,30000);
    iVar1 = *(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4) + 0xa8);
    if ((iVar1 != 0) && (iVar1 != 2)) {
      FUN_ram_f004113c(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 0x14) * 4),1);
    }
    *(undefined1 *)(param_1 + 0x49a) = 1;
    *(undefined1 *)(param_1 + 0x49e) = 0;
    *(undefined1 *)(param_1 + 0x498) = 0;
    iVar1 = 0;
    do {
      iVar2 = iVar1 + 1;
      iVar1 = FUN_ram_f0025d90(iVar1);
      if (((iVar1 != 0) && ((*(uint *)(iVar1 + 0x10) & 0x80) != 0)) &&
         (*(char *)(iVar1 + 0x15) == '\x02')) {
        DAT_ram_f0065032 = DAT_ram_f0065032 + '\x01';
      }
      iVar1 = iVar2;
    } while (iVar2 != 0x14);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00412b0 @ 0xf00412b0 =====


void FUN_ram_f00412b0(int param_1,int param_2)

{
  if (((DAT_ram_f0065032 != '\0') && (param_1 != 0)) && (param_2 - 1U < 2)) {
    FUN_ram_f004113c();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00412dc @ 0xf00412dc =====


void FUN_ram_f00412dc(int param_1,int param_2,int param_3)

{
  if (((((DAT_ram_f0065032 != '\0') && (param_1 != 0)) && (param_3 != 0)) &&
      ((*(int *)(param_3 + 0x14) != 0 && (param_2 != 0)))) &&
     ((*(byte *)(param_2 + 0x14) < 3 &&
      ((*(char *)(param_2 + 0x15) == '\x02' && ((*(uint *)(param_2 + 0x10) & 0x80) != 0)))))) {
    if ((*(char *)(param_2 + 0x4a1) == '\0') ||
       (*(char *)(param_1 + 0x2cc) == *(char *)(param_2 + 0x4a1))) {
      if (*(int *)(param_1 + 0x1c) == 0) {
        if (*(int *)(param_3 + 8) != 0) {
          FUN_ram_f003083c(param_3);
        }
        if ((*(int *)(param_3 + 8) != 0) && (*(int *)(param_3 + 0x14) != 0)) {
          FUN_ram_f0030784(param_3);
        }
        *(undefined1 *)(param_2 + 0x3ce) = 1;
        FUN_ram_f003f140(param_2);
      }
    }
    else {
      *(undefined1 *)(param_2 + 0x3cc) = 0;
      *(undefined1 *)(param_2 + 0x3cd) = 0;
      FUN_ram_f00266d8(param_2 + 0x37c);
      FUN_ram_f00266d8(param_2 + 0x398);
      FUN_ram_f00266d8(param_2 + 0x3b0);
      FUN_ram_f003f140(param_2);
      *(undefined1 *)(param_2 + 0x394) = 0;
      if (*(char *)(param_2 + 0x4a0) == '\x01') {
        FUN_ram_f00372b4(param_2);
        *(undefined1 *)(param_2 + 0x4a0) = 0;
      }
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00413b4 @ 0xf00413b4 =====


undefined4 FUN_ram_f00413b4(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  
  if (((((DAT_ram_f0065032 != '\0') && (param_1 != 0)) && (param_2 != 0)) &&
      ((param_3 != 0 && (*(char *)(param_3 + 0x15) == '\x02')))) &&
     ((uVar1 = **(ushort **)(param_1 + 0x14), (*(uint *)(param_3 + 0x10) & 0x80) != 0 &&
      ((*(char *)(param_3 + 0x4a1) == '\0' ||
       (*(char *)(param_2 + 0x2cc) == *(char *)(param_3 + 0x4a1))))))) {
    iVar3 = *(int *)(param_2 + 0xa8);
    FUN_ram_f002fff8(param_1,0);
    uVar1 = uVar1 & 0xfc;
    if ((uVar1 == 8) || (uVar1 == 0x88)) {
      FUN_ram_f002ea68(param_1);
    }
    iVar2 = FUN_ram_f002adbc(param_1);
    if (iVar2 == 0) {
      if (iVar3 == 3) {
        FUN_ram_f0030700(param_1,*(int *)(param_2 + 0xa8) == 3);
      }
    }
    else {
      FUN_ram_f0030510(param_1);
    }
    if ((*(char *)(param_3 + 9) != '\0') && (*(char *)(param_3 + 0x15) == '\x02')) {
      FUN_ram_f00266d8(param_3 + 0x3dc);
      FUN_ram_f002672c(param_3 + 0x3dc,30000);
      return 0;
    }
  }
  return 0xc0000001;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00415b0 @ 0xf00415b0 =====


void FUN_ram_f00415b0(int param_1)

{
  (&DAT_ram_f006e844)[(uint)*(byte *)(param_1 + 0x361) * 0x44c] = 0;
  *(undefined1 *)(param_1 + 0x361) = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00415d0 @ 0xf00415d0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f00415d0(void)

{
  undefined4 local_10 [2];
  
  (*_DAT_ram_00014bf0)(0,local_10,_DAT_ram_00014bf0);
  return local_10[0];
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00416b0 @ 0xf00416b0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00416b0(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  
  if ((param_1 != 0) && (*(char *)(param_1 + 0x361) == '\0')) {
    uVar1 = *(uint *)(param_2 + 0x40);
    _DAT_ram_f006e8fc = _DAT_ram_f006e8fc + 1;
    DAT_ram_f006e8f8 = *(undefined1 *)(param_3 + 0x48);
    uVar5 = (uVar1 & 0x7ffff) >> 0x11;
    DAT_ram_f006e8f9 = *(undefined1 *)(param_3 + 0x49);
    if ((uVar5 != 3) && (uVar3 = (uVar1 & 0x7ffffff) >> 0x14, uVar3 < 0x10)) {
      DAT_ram_f006e8fa = (byte)(uVar1 >> 0x1f);
      iVar2 = uVar5 * 0x10 + uVar3;
      iVar4 = iVar2 + 0x30;
      *(int *)(&DAT_ram_f006e844 + iVar4 * 4) = *(int *)(&DAT_ram_f006e844 + iVar4 * 4) + 1;
      if ((*(ushort *)(param_3 + 0x32) & 0x800) != 0) {
        iVar2 = iVar2 + 0xa6;
        *(int *)(&DAT_ram_f006e844 + iVar2 * 4) = *(int *)(&DAT_ram_f006e844 + iVar2 * 4) + 1;
      }
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0041724 @ 0xf0041724 =====


void FUN_ram_f0041724(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  
  if ((param_1 != 0) && (*(char *)(param_1 + 0x361) == '\0')) {
    uVar3 = (*(uint *)(param_2 + 0x40) & 0x7ffff) >> 0x11;
    if ((uVar3 != 3) &&
       ((uVar1 = (*(uint *)(param_2 + 0x40) & 0x7ffffff) >> 0x14, uVar1 < 0x10 &&
        ((*(ushort *)(param_3 + 0x32) & 0x800) != 0)))) {
      iVar2 = uVar3 * 0x10 + uVar1 + 0xa6;
      *(int *)(&DAT_ram_f006e844 + iVar2 * 4) = *(int *)(&DAT_ram_f006e844 + iVar2 * 4) + 1;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0041768 @ 0xf0041768 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0041768(int param_1,int param_2,int param_3,uint param_4,int param_5,uint param_6)

{
  byte bVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  
  if ((((param_1 != 0) && (param_2 != 0)) && (*(byte **)(param_2 + 0x14) != (byte *)0x0)) &&
     ((*(char *)(param_1 + 0x361) == '\0' && (*(char *)(param_1 + 0x360) != '\0')))) {
    bVar1 = **(byte **)(param_2 + 0x14) & 0xfc;
    if (bVar1 == 0x48) {
      _DAT_ram_f006ec60 = _DAT_ram_f006ec60 + 1;
    }
    else if (bVar1 == 200) {
      _DAT_ram_f006ec64 = _DAT_ram_f006ec64 + 1;
    }
    else {
      _DAT_ram_f006e850 = _DAT_ram_f006e850 + 1;
      bVar1 = DAT_ram_f006ec8c & 0x3f;
      if (bVar1 < 0x10) {
        if ((DAT_ram_f006ec8c & 0x80) == 0) {
          if (DAT_ram_f006e870 < bVar1) {
            DAT_ram_f006e870 = bVar1;
          }
        }
        else if (DAT_ram_f006e871 < bVar1) {
          DAT_ram_f006e871 = bVar1;
        }
      }
      if (param_5 == 0) {
        _DAT_ram_f006ec58 = _DAT_ram_f006ec58 + 1;
      }
      else {
        uVar2 = *(byte *)(param_5 + -0xff91375) & 0xf;
        if ((*(byte *)(param_5 + -0xff91375) & 0x80) == 0) {
          iVar3 = uVar2 + 0xc;
        }
        else {
          iVar3 = uVar2 + 0x1c;
        }
        *(int *)(&DAT_ram_f006e844 + iVar3 * 4) = *(int *)(&DAT_ram_f006e844 + iVar3 * 4) + 1;
      }
      if (param_4 < 500) {
        iVar3 = 0;
      }
      else if (param_4 - 500 < 500) {
        iVar3 = 1;
      }
      else {
        iVar3 = 2;
      }
      if (*(uint *)(param_2 + 0x60) < *(uint *)(param_2 + 100)) {
        iVar5 = (iVar3 + 0x60) * 4;
        uVar2 = *(uint *)(param_2 + 100) - *(uint *)(param_2 + 0x60);
        if (*(uint *)(&DAT_ram_f006e848 + iVar5) < uVar2) {
          *(uint *)(&DAT_ram_f006e848 + iVar5) = uVar2;
        }
        else if (uVar2 < *(uint *)(&DAT_ram_f006e844 + (iVar3 + 100) * 4)) {
          *(uint *)(&DAT_ram_f006e844 + (iVar3 + 100) * 4) = uVar2;
        }
        iVar5 = (iVar3 + 0x66) * 4;
        uVar6 = uVar2 + *(int *)(&DAT_ram_f006e848 + iVar5);
        *(uint *)(&DAT_ram_f006e848 + iVar5) = uVar6;
        if (uVar6 != uVar2) {
          *(uint *)(&DAT_ram_f006e848 + iVar5) = uVar6 >> 1;
        }
      }
      if (*(uint *)(&DAT_ram_f006e844 + (iVar3 + 0x6a) * 4) < param_6) {
        *(uint *)(&DAT_ram_f006e844 + (iVar3 + 0x6a) * 4) = param_6;
      }
      else {
        iVar5 = (iVar3 + 0x6c) * 4;
        if (param_6 < *(uint *)(&DAT_ram_f006e848 + iVar5)) {
          *(uint *)(&DAT_ram_f006e848 + iVar5) = param_6;
        }
      }
      iVar5 = iVar3 + 0x70;
      uVar2 = param_6 + *(int *)(&DAT_ram_f006e844 + iVar5 * 4);
      *(uint *)(&DAT_ram_f006e844 + iVar5 * 4) = uVar2;
      if (param_6 != uVar2) {
        *(uint *)(&DAT_ram_f006e844 + iVar5 * 4) = uVar2 >> 1;
      }
      uVar4 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
      uVar2 = FUN_ram_f00415d0();
      uVar6 = *(uint *)(param_2 + 0x60);
      if ((uVar6 < uVar2) && (uVar6 != 0)) {
        iVar5 = (iVar3 + 0x72) * 4;
        uVar2 = uVar2 - uVar6;
        uVar6 = uVar2 + *(ushort *)(param_2 + 0x68);
        if (*(uint *)(&DAT_ram_f006e848 + iVar5) < uVar2) {
          *(uint *)(&DAT_ram_f006e848 + iVar5) = uVar2;
          *(undefined4 *)(&DAT_ram_f006e844 + (iVar3 + 0x7c) * 4) = uVar4;
        }
        else if (uVar2 < *(uint *)(&DAT_ram_f006e844 + (iVar3 + 0x76) * 4)) {
          *(uint *)(&DAT_ram_f006e844 + (iVar3 + 0x76) * 4) = uVar2;
        }
        iVar5 = (iVar3 + 0x7e) * 4;
        if (*(uint *)(&DAT_ram_f006e848 + iVar5) < uVar6) {
          *(uint *)(&DAT_ram_f006e848 + iVar5) = uVar6;
        }
        else if (uVar6 < *(uint *)(&DAT_ram_f006e844 + (iVar3 + 0x82) * 4)) {
          *(uint *)(&DAT_ram_f006e844 + (iVar3 + 0x82) * 4) = uVar6;
        }
        iVar5 = (iVar3 + 0x78) * 4;
        uVar7 = uVar2 + *(int *)(&DAT_ram_f006e848 + iVar5);
        *(uint *)(&DAT_ram_f006e848 + iVar5) = uVar7;
        if (uVar7 != uVar2) {
          *(uint *)(&DAT_ram_f006e848 + iVar5) = uVar7 >> 1;
        }
        iVar3 = (iVar3 + 0x84) * 4;
        uVar7 = uVar6 + *(int *)(&DAT_ram_f006e848 + iVar3);
        *(uint *)(&DAT_ram_f006e848 + iVar3) = uVar7;
        if (uVar7 != uVar6) {
          *(uint *)(&DAT_ram_f006e848 + iVar3) = uVar7 >> 1;
        }
        if (_DAT_ram_f006ea74 != 0) {
          if (uVar2 < _DAT_ram_f006ea74) {
            uVar6 = uVar2 / *(uint *)(param_1 + 0x364);
            if (uVar6 < 4) {
              iVar3 = (uVar6 + 0x8c) * 4;
              *(int *)(&DAT_ram_f006e848 + iVar3) = *(int *)(&DAT_ram_f006e848 + iVar3) + 1;
            }
          }
          else {
            _DAT_ram_f006ea88 = _DAT_ram_f006ea88 + 1;
          }
        }
        uVar6 = (uint)*(ushort *)(param_2 + 0x68);
        if (uVar6 < _DAT_ram_f0064f7c) {
          _DAT_ram_f006ea8c = _DAT_ram_f006ea8c + 1;
        }
        else if (uVar6 < _DAT_ram_f0064f80) {
          _DAT_ram_f006ea90 = _DAT_ram_f006ea90 + 1;
        }
        else if (uVar6 < _DAT_ram_f0064f84) {
          _DAT_ram_f006ea94 = _DAT_ram_f006ea94 + 1;
        }
        else {
          _DAT_ram_f006ea98 = _DAT_ram_f006ea98 + 1;
        }
        if (uVar2 < _DAT_ram_f0064f70) {
          _DAT_ram_f006ea64 = _DAT_ram_f006ea64 + 1;
        }
        else if (uVar2 < _DAT_ram_f0064f74) {
          _DAT_ram_f006ea68 = _DAT_ram_f006ea68 + 1;
        }
        else if (uVar2 < _DAT_ram_f0064f78) {
          _DAT_ram_f006ea6c = _DAT_ram_f006ea6c + 1;
        }
        else {
          _DAT_ram_f006ea70 = _DAT_ram_f006ea70 + 1;
        }
      }
      _DAT_ram_f006eac4 = _DAT_ram_f006eac4 + param_3;
      if (DAT_ram_f0064fc7 == '\0') {
        DAT_ram_f0064fc7 = '\x01';
        FUN_ram_f002672c(0xf006e7c4,1000);
        _DAT_ram_60340020 = 0;
      }
      _DAT_ram_f006ebc8 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0041c18 @ 0xf0041c18 =====


void FUN_ram_f0041c18(int param_1)

{
  undefined1 uVar1;
  undefined4 *puVar2;
  
  if (param_1 != 0) {
    if (DAT_ram_f006e849 == *(char *)(param_1 + 8)) {
      (&DAT_ram_f006e844)[(uint)*(byte *)(param_1 + 0x361) * 0x44c] = 0;
      *(undefined1 *)(param_1 + 0x361) = 0;
    }
    if (DAT_ram_f006e844 == '\0') {
      *(undefined1 *)(param_1 + 0x360) = 0;
      *(undefined4 *)(param_1 + 0x364) = 0;
      *(undefined4 *)(param_1 + 0x368) = 0;
      (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e844,0,0x44c);
      puVar2 = (undefined4 *)&DAT_ram_f006ea1c;
      do {
        *puVar2 = 0xffffffff;
        puVar2[-9] = 0xffffffff;
        puVar2[-0x12] = 0xffffffff;
        puVar2[0xc] = 0xffffffff;
        puVar2[0x23] = 0xffffffff;
        puVar2[0x96] = 0xffffffff;
        puVar2 = puVar2 + 1;
      } while (puVar2 != (undefined4 *)0xf006ea28);
      DAT_ram_f006e844 = '\x01';
      DAT_ram_f006e849 = *(char *)(param_1 + 8);
      uVar1 = 0;
    }
    else {
      uVar1 = 0xff;
    }
    *(undefined1 *)(param_1 + 0x361) = uVar1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0041cbc @ 0xf0041cbc =====


void FUN_ram_f0041cbc(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,
                     undefined4 param_5)

{
  int iVar1;
  undefined2 *puVar2;
  
  iVar1 = FUN_ram_f0025c48(param_2 + 0x14);
  if (iVar1 != 0) {
    puVar2 = *(undefined2 **)(iVar1 + 8);
    *(undefined1 *)(puVar2 + 2) = 0x81;
    *(undefined1 *)((int)puVar2 + 5) = 0;
    *puVar2 = (short)(param_2 + 0x14);
    *(undefined4 *)(puVar2 + 4) = param_3;
    *(undefined4 *)(puVar2 + 8) = param_5;
    *(undefined4 *)(puVar2 + 6) = param_4;
    (*(code *)&SUB_ram_0006a08c)(puVar2 + 10,param_1,param_2);
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar2;
    puVar2[1] = 1;
    FUN_ram_f0039750(iVar1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0041d34 @ 0xf0041d34 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0041d34(int param_1)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  uint uVar5;
  char *pcVar6;
  int iVar7;
  int *piVar8;
  int *piVar9;
  uint uVar10;
  int *piVar11;
  int *piVar12;
  int *piVar13;
  uint *puVar14;
  
  cVar1 = *(char *)(param_1 + 4);
  iVar2 = FUN_ram_f0025d90(cVar1);
  if ((iVar2 != 0) && (*(char *)(iVar2 + 0x361) == '\0')) {
    DAT_ram_f006e9c7 = 0xb;
    _DAT_ram_f006eac0 = _DAT_ram_f0064f6c;
    pcVar6 = (char *)(_DAT_ram_f0064f34 + 0x11);
    iVar7 = 0;
    do {
      iVar7 = iVar7 + 1;
      if ((*pcVar6 == '\x02') && (pcVar6[1] == cVar1)) {
        DAT_ram_f006e8f4 = (byte)(1 << ((byte)pcVar6[2] & 0x1f)) | DAT_ram_f006e8f4;
      }
      pcVar6 = pcVar6 + 0x24;
    } while (iVar7 != 8);
    pcVar6 = (char *)(_DAT_ram_f0064f34 + 0x131);
    iVar7 = 0;
    do {
      iVar7 = iVar7 + 1;
      if ((*pcVar6 == '\x02') && (pcVar6[1] == cVar1)) {
        DAT_ram_f006e9c4 = (byte)(1 << ((byte)pcVar6[2] & 0x1f)) | DAT_ram_f006e9c4;
      }
      pcVar6 = pcVar6 + 8;
    } while (iVar7 != 8);
    piVar11 = (int *)&DAT_ram_f006ea1c;
    piVar3 = (int *)&DAT_ram_f006e9f8;
    puVar14 = (uint *)&DAT_ram_f006ea34;
    piVar13 = (int *)&DAT_ram_f006ec74;
    piVar12 = (int *)&DAT_ram_f006eaa8;
    piVar9 = (int *)&DAT_ram_f006ea4c;
    piVar4 = (int *)&DAT_ram_f006e9d4;
    piVar8 = piVar11;
    do {
      if (*piVar8 == -1) {
        *piVar8 = 0;
      }
      if (*piVar3 == -1) {
        *piVar3 = 0;
      }
      if (*piVar4 == -1) {
        *piVar4 = 0;
      }
      if (*piVar9 == -1) {
        *piVar9 = 0;
      }
      if (*piVar12 == -1) {
        *piVar12 = 0;
      }
      if (*piVar13 == -1) {
        *piVar13 = 0;
      }
      if (_DAT_ram_f006ea74 < (uint)piVar8[-3]) {
        _DAT_ram_f006ea74 = piVar8[-3];
      }
      if (*puVar14 != 0) {
        *puVar14 = *puVar14 * 1000 + 0x7fff >> 0xf;
      }
      piVar8 = piVar8 + 1;
      puVar14 = puVar14 + 1;
      piVar13 = piVar13 + 1;
      piVar12 = piVar12 + 1;
      piVar9 = piVar9 + 1;
      piVar4 = piVar4 + 1;
      piVar3 = piVar3 + 1;
    } while (piVar8 != (int *)0xf006ea28);
    _DAT_ram_f006eacc = _DAT_ram_f006eacc * 1000 + 0x7fff >> 0xf;
    iVar7 = (*_DAT_ram_00014830)(piVar3,_DAT_ram_00014830);
    _DAT_ram_f006ead4 = iVar7 * 1000 + 0x7fffU >> 0xf;
    _DAT_ram_f006eb9c = _DAT_ram_f006e79c;
    _DAT_ram_f006eba0 = _DAT_ram_f006e7a0;
    _DAT_ram_f006eba4 = _DAT_ram_f006e7a4;
    _DAT_ram_f006eba8 = _DAT_ram_f006e7a8;
    _DAT_ram_f006ebc8 = _DAT_ram_f006ebc8 * 1000 + 0x7fff >> 0xf;
    _DAT_ram_f006ebac = _DAT_ram_f006e7ac;
    _DAT_ram_f006ebb0 = _DAT_ram_f006e7b0;
    _DAT_ram_f006ebb4 = _DAT_ram_f006e7b4;
    _DAT_ram_f006ebb8 = _DAT_ram_f006e7b8;
    _DAT_ram_f006ebbc = _DAT_ram_f006e7bc;
    _DAT_ram_f006ebc0 = _DAT_ram_f006e7c0;
    _DAT_ram_f006ebd4 = _DAT_ram_f006e7dc;
    _DAT_ram_f006ebd8 = _DAT_ram_f006e7e0;
    _DAT_ram_f006ebdc = _DAT_ram_f006e7e4;
    _DAT_ram_f006ebe0 = _DAT_ram_f006e7e8;
    _DAT_ram_f006ebe4 = _DAT_ram_f006e7ec;
    _DAT_ram_f006ebe8 = _DAT_ram_f006e7f0;
    _DAT_ram_f006ebec = _DAT_ram_f006ec90;
    _DAT_ram_f006ebf0 = _DAT_ram_f006ec94;
    _DAT_ram_f006ebf4 = _DAT_ram_f006ec98;
    _DAT_ram_f006ebf8 = _DAT_ram_f006ec9c;
    _DAT_ram_f006ebfc = _DAT_ram_f006eca0;
    _DAT_ram_f006ec00 = _DAT_ram_f006eca4;
    _DAT_ram_f006ebc4 = FUN_ram_f0020704();
    _DAT_ram_f006ec54 = _DAT_ram_f0064f64;
    _DAT_ram_f006eac8 = _DAT_ram_60340018 & 0xffff;
    FUN_ram_f0041cbc(&DAT_ram_f006e844,0x44c,0,1,_DAT_ram_f0064f68);
    uVar5 = _DAT_ram_f006ea14;
    if (_DAT_ram_f006ea14 < _DAT_ram_f006ea10) {
      uVar5 = _DAT_ram_f006ea10;
    }
    uVar10 = _DAT_ram_f006ea18;
    if (_DAT_ram_f006ea18 < uVar5) {
      uVar10 = uVar5;
    }
    (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e844,0,0x448);
    do {
      *piVar11 = -1;
      piVar11[-9] = -1;
      piVar11[-0x12] = -1;
      piVar11[0xc] = -1;
      piVar11[0x23] = -1;
      piVar11[0x96] = -1;
      piVar11 = piVar11 + 1;
    } while (piVar11 != (int *)0xf006ea28);
    _DAT_ram_f006ea74 = uVar10;
    *(uint *)(iVar2 + 0x364) = uVar10 >> 2;
    (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e7dc,0,0x18);
    (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006ec90,0,0x18);
    *(undefined1 *)(iVar2 + 0x360) = 1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004201c @ 0xf004201c =====


void FUN_ram_f004201c(undefined1 param_1)

{
  undefined4 local_54;
  undefined1 local_50;
  
  local_54 = 0;
  local_50 = param_1;
  FUN_ram_f0041d34(&local_54);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00420e8 @ 0xf00420e8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00420e8(void)

{
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e844,0,0x44c);
  _DAT_ram_60340000 = _DAT_ram_60340000 | 8;
  _DAT_ram_f0064f74 = 2000;
  _DAT_ram_f0064f78 = 3000;
  _DAT_ram_f0064f7c = 10000;
  _DAT_ram_f0064f80 = 20000;
  _DAT_ram_f0064f84 = 30000;
  _DAT_ram_f0064f70 = 1000;
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e79c,0,0x28);
  FUN_ram_f00266a0(0xf006e7c4,&LAB_ram_f004156c,0);
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e7f4,0,0x50);
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e7dc,0,0x18);
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006ec90,0,0x18);
  (*(code *)&SUB_ram_0006a0a0)(0xf006eca8,0,1000);
  _DAT_ram_f0064f8c = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00421d0 @ 0xf00421d0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00421d0(undefined4 param_1,byte param_2)

{
  if (((((param_2 & 0x7f) < 3) && ((char)param_2 < '\0')) && ((param_2 & 0x7f) == 2)) &&
     ((_DAT_ram_f006e514 == 1 || (_DAT_ram_f006e514 == 0)))) {
    *(char *)(_DAT_ram_020a0064 + 0xf44) = (char)_DAT_ram_f006e514;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042254 @ 0xf0042254 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042254(undefined1 param_1)

{
  *(undefined1 *)(_DAT_ram_020a0064 + 0xf44) = param_1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042264 @ 0xf0042264 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042264(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = _DAT_ram_020a0064;
  _DAT_ram_60310024 = 0xfc000;
  if (*(char *)(_DAT_ram_020a0068 + 0x344) == '\0') {
    uVar2 = 0x100;
    _DAT_ram_60310000 = 0x100;
  }
  else {
    uVar2 = 0x1000;
    _DAT_ram_60310000 = 0x1000;
  }
  (*_DAT_ram_00014984)(uVar2,0,_DAT_ram_00014984);
  *(undefined1 *)(iVar1 + 0xf46) = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00422bc @ 0xf00422bc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00422bc(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = _DAT_ram_020a0064;
  _DAT_ram_60310024 = 0xfc000;
  if (*(char *)(_DAT_ram_020a0068 + 0x344) == '\0') {
    (*_DAT_ram_00014a54)(0,0,_DAT_ram_00014a54);
    (*_DAT_ram_00014a54)(0,1,_DAT_ram_00014a54);
    uVar2 = 0x100;
    _DAT_ram_60310000 = 0x100;
  }
  else {
    (*_DAT_ram_00014a54)(1,0,_DAT_ram_00014a54);
    (*_DAT_ram_00014a54)(1,1,_DAT_ram_00014a54);
    uVar2 = 0x1000;
    _DAT_ram_60310000 = 0x1000;
  }
  (*_DAT_ram_00014988)(uVar2,0,_DAT_ram_00014988);
  *(undefined1 *)(iVar1 + 0xf46) = 1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042344 @ 0xf0042344 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0042344(void)

{
  char cVar1;
  int iVar2;
  ushort uVar3;
  short sVar4;
  uint uVar5;
  ushort local_14;
  ushort local_12;
  
  iVar2 = _DAT_ram_020a0064;
  cVar1 = *(char *)(_DAT_ram_020a0064 + 0xf45);
  (*_DAT_ram_00014a70)
            (*(undefined1 *)(_DAT_ram_020a0064 + 0xf4c),&local_14,&local_12,_DAT_ram_00014a70);
  uVar3 = local_14;
  if (cVar1 == '\0') {
    *(undefined2 *)(iVar2 + 0xf52) = 0xff;
    *(undefined2 *)(iVar2 + 0xf56) = 0;
    uVar5 = (uint)local_12;
    if (_DAT_ram_f0064bd0 < uVar5) {
      uVar5 = uVar5 + _DAT_ram_f0064bcc * -2 & 0xffff;
      if (_DAT_ram_f0064bd0 < uVar5) {
        uVar5 = _DAT_ram_f0064bd0;
      }
      local_12 = (ushort)uVar5;
      *(undefined2 *)(iVar2 + 0xf50) = 0x1ff;
      local_14 = local_12;
    }
    else {
      if (_DAT_ram_f0064bd4 < uVar5) {
        uVar5 = uVar5 + _DAT_ram_f0064bcc * 2;
        local_14 = (ushort)uVar5;
        if (0x1ff < (uVar5 & 0xffff)) {
          local_14 = 0x1ff;
        }
        *(ushort *)(iVar2 + 0xf50) = local_14;
        sVar4 = (short)_DAT_ram_f0064bcc;
      }
      else {
        *(ushort *)(iVar2 + 0xf50) = local_12 + (short)_DAT_ram_f0064bcc;
        sVar4 = (short)(_DAT_ram_f0064bcc >> 1);
      }
      local_12 = local_12 - sVar4;
    }
    *(ushort *)(iVar2 + 0xf54) = local_12;
  }
  else {
    *(undefined2 *)(iVar2 + 0xf50) = 0x1ff;
    *(undefined2 *)(iVar2 + 0xf54) = 0;
    uVar5 = (uint)(byte)local_14;
    if (_DAT_ram_f0064bc0 < uVar5) {
      uVar5 = uVar5 + _DAT_ram_f0064bbc * -2 & 0xffff;
      if (_DAT_ram_f0064bc0 < uVar5) {
        uVar5 = _DAT_ram_f0064bc0;
      }
      local_14 = (ushort)uVar5;
      *(undefined2 *)(iVar2 + 0xf52) = 0xff;
      *(ushort *)(iVar2 + 0xf56) = local_14;
    }
    else {
      if (_DAT_ram_f0064bc4 < uVar5) {
        uVar5 = uVar5 + _DAT_ram_f0064bbc * 2;
        local_14 = (ushort)uVar5;
        if (0xff < (uVar5 & 0xffff)) {
          local_14 = 0xff;
        }
        *(ushort *)(iVar2 + 0xf52) = local_14;
        sVar4 = _DAT_ram_f0064bbc;
      }
      else {
        *(ushort *)(iVar2 + 0xf52) = (local_14 & 0xff) + (short)_DAT_ram_f0064bbc;
        sVar4 = (short)(_DAT_ram_f0064bbc >> 1);
      }
      *(ushort *)(iVar2 + 0xf56) = (uVar3 & 0xff) - sVar4;
    }
  }
  (*_DAT_ram_00014ad0)
            (*(undefined1 *)(iVar2 + 0xf4c),*(undefined1 *)(iVar2 + 0xf4d),
             *(undefined1 *)(iVar2 + 0xf4e),*(undefined2 *)(iVar2 + 0xf54),
             *(undefined2 *)(iVar2 + 0xf56),*(undefined1 *)(iVar2 + 0xf58),
             *(undefined1 *)(iVar2 + 0xf59),*(undefined1 *)(iVar2 + 0xf5a),
             *(undefined1 *)(iVar2 + 0xf5b));
  (*_DAT_ram_00014acc)
            (*(undefined1 *)(iVar2 + 0xf4c),*(undefined2 *)(iVar2 + 0xf50),
             *(undefined2 *)(iVar2 + 0xf52),*(undefined1 *)(iVar2 + 0xf58),_DAT_ram_00014acc);
  (*_DAT_ram_00014a54)(0,0,_DAT_ram_00014a54);
  (*_DAT_ram_00014a54)(0,1,_DAT_ram_00014a54);
  if (*(char *)(iVar2 + 0xf46) == '\0') {
    FUN_ram_f00422bc();
  }
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00424d0 @ 0xf00424d0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00424d0(void)

{
  int iVar1;
  undefined1 uVar2;
  
  iVar1 = _DAT_ram_020a0064;
  *(undefined1 *)(_DAT_ram_020a0064 + 0xf45) = 0;
  *(undefined4 *)(iVar1 + 0xf3c) = 0;
  uVar2 = DAT_ram_f0064bb4;
  *(undefined1 *)(iVar1 + 0xf48) = 0;
  *(undefined1 *)(iVar1 + 0xf4c) = 0;
  *(undefined1 *)(iVar1 + 0xf49) = 0;
  *(undefined1 *)(iVar1 + 0xf46) = 0;
  *(undefined1 *)(iVar1 + 0xf4a) = 0;
  *(undefined1 *)(iVar1 + 0xf47) = uVar2;
  if (_DAT_ram_f006513c == 5000000) {
    uVar2 = 2;
  }
  else {
    uVar2 = 3;
  }
  *(undefined1 *)(iVar1 + 0xf4d) = uVar2;
  *(undefined1 *)(iVar1 + 0xf4e) = 0;
  *(short *)(iVar1 + 0xf50) = (short)_DAT_ram_f0064bcc * 2 + (short)_DAT_ram_f0064bd0;
  *(undefined2 *)(iVar1 + 0xf54) = _DAT_ram_f0064bd0;
  *(short *)(iVar1 + 0xf52) = (short)_DAT_ram_f0064bbc * 2 + (short)_DAT_ram_f0064bc0;
  *(undefined2 *)(iVar1 + 0xf56) = _DAT_ram_f0064bc0;
  *(undefined1 *)(iVar1 + 0xf58) = DAT_ram_f0064bdc;
  *(undefined1 *)(iVar1 + 0xf5b) = 2;
  *(undefined1 *)(iVar1 + 0xf59) = 5;
  *(undefined1 *)(iVar1 + 0xf5a) = 1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042688 @ 0xf0042688 =====


undefined4 FUN_ram_f0042688(undefined2 param_1,undefined2 param_2,undefined2 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 *puVar3;
  
  iVar1 = FUN_ram_f0025c48(0xe);
  if (iVar1 == 0) {
    uVar2 = 0xc000009a;
  }
  else {
    puVar3 = *(undefined2 **)(iVar1 + 8);
    *(undefined1 *)(puVar3 + 2) = 0x20;
    *puVar3 = 0xe;
    *(undefined1 *)((int)puVar3 + 5) = 0;
    puVar3[6] = param_3;
    puVar3[4] = param_1;
    puVar3[5] = param_2;
    *(undefined1 *)(iVar1 + 0x2f) = 0;
    *(undefined2 **)(iVar1 + 0x28) = puVar3;
    puVar3[1] = 1;
    FUN_ram_f0039750();
    uVar2 = 0;
  }
  return uVar2;
}

// >>> MOD: wifi/mgmt/mt6582/roaming_fsm.c:0x4d2 <<<
// ===== FUN_ram_f00426f0 @ 0xf00426f0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00426f0(uint param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined1 local_9;
  
  iVar1 = _DAT_ram_020a0064;
  *(uint *)(_DAT_ram_020a0064 + 0xf1c) = param_1;
  if (param_1 == 1) {
    iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    if (iVar2 - *(int *)(iVar1 + 0xf38) < 0) {
      uVar3 = _DAT_ram_f0064bb8;
      if ((*(char *)(iVar1 + 0xf49) == '\0') &&
         (uVar3 = _DAT_ram_f0064bd8, *(char *)(iVar1 + 0xf48) != '\0')) {
        uVar3 = _DAT_ram_f0064bd8 >> 1;
      }
      if (uVar3 == 0) {
        uVar3 = 1;
      }
      FUN_ram_f002672c(iVar1 + 0xf20,uVar3 * 1000);
    }
    else if (*(char *)(iVar1 + 0xf49) == '\0') {
      if (*(char *)(iVar1 + 0xf4a) != '\0') {
        FUN_ram_f0042344();
      }
    }
    else {
      *(undefined1 *)(iVar1 + 0xf49) = 0;
      (*_DAT_ram_00014a68)(0,*(undefined1 *)(iVar1 + 0xf45),&local_9,_DAT_ram_00014a68);
      FUN_ram_f0042998(local_9,0);
    }
  }
  else if ((param_1 != 0) && (3 < param_1)) {
    (*_DAT_ram_00014800)(0,0,0xf0063b38,0x4d2,_DAT_ram_00014800);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00427c0 @ 0xf00427c0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00427c0(void)

{
  int iVar1;
  
  iVar1 = _DAT_ram_020a0064;
  if (*(int *)(_DAT_ram_020a0064 + 0xf1c) != 0) {
    FUN_ram_f00424d0();
    if (*(int *)(iVar1 + 0xf1c) == 1) {
      FUN_ram_f00266d8(iVar1 + 0xf20);
      FUN_ram_f0042264();
    }
    FUN_ram_f00426f0(0);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042810 @ 0xf0042810 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042810(void)

{
  int iVar1;
  undefined1 uVar2;
  int iVar3;
  
  iVar1 = _DAT_ram_020a0064;
  if (*(int *)(_DAT_ram_020a0064 + 0xf1c) != 1) {
    if (*(char *)(_DAT_ram_020a0064 + 0xf47) == '\0') {
      *(undefined4 *)(_DAT_ram_020a0064 + 0xf3c) = 0;
      uVar2 = DAT_ram_f0064bb4;
      *(undefined1 *)(iVar1 + 0xf4a) = 1;
      *(undefined1 *)(iVar1 + 0xf47) = uVar2;
    }
    else {
      *(char *)(_DAT_ram_020a0064 + 0xf47) = *(char *)(_DAT_ram_020a0064 + 0xf47) + -1;
      *(undefined1 *)(iVar1 + 0xf49) = 1;
    }
    *(undefined1 *)(iVar1 + 0xf48) = 1;
    iVar3 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    *(int *)(iVar1 + 0xf38) = iVar3;
    iVar3 = iVar3 + _DAT_ram_f0064bd8 * 0x8000;
    *(int *)(iVar1 + 0xf40) = iVar3;
    *(int *)(iVar1 + 0xf38) = iVar3;
    FUN_ram_f00426f0(1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00428ac @ 0xf00428ac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00428ac(void)

{
  int iVar1;
  int iVar2;
  
  iVar1 = _DAT_ram_020a0064;
  if (*(int *)(_DAT_ram_020a0064 + 0xf1c) != 1) {
    FUN_ram_f00424d0();
    if ((*(ushort *)(_DAT_ram_020a0068 + 0x86) & 0x3f) != 0) {
      *(undefined1 *)(iVar1 + 0xf45) = 1;
    }
    *(undefined1 *)(iVar1 + 0xf48) = 0;
    iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
    *(int *)(iVar1 + 0xf38) = iVar2;
    iVar2 = iVar2 + _DAT_ram_f0064bd8 * 0x8000;
    *(int *)(iVar1 + 0xf40) = iVar2;
    *(int *)(iVar1 + 0xf38) = iVar2;
    *(undefined1 *)(iVar1 + 0xf4a) = 1;
    FUN_ram_f00426f0(1);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042998 @ 0xf0042998 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042998(undefined1 param_1,undefined2 param_2)

{
  bool bVar1;
  int iVar2;
  
  iVar2 = _DAT_ram_020a0064;
  if (*(int *)(_DAT_ram_020a0064 + 0xf1c) == 2) {
    return;
  }
  if (*(int *)(_DAT_ram_020a0064 + 0xf1c) == 1) {
    FUN_ram_f00266d8(_DAT_ram_020a0064 + 0xf20);
    FUN_ram_f0042264();
    if (*(char *)(iVar2 + 0xf44) != '\0') {
      iVar2 = FUN_ram_f0042688(1,param_1,param_2);
      bVar1 = true;
      goto LAB_ram_f00429f2;
    }
  }
  bVar1 = false;
  iVar2 = 0;
LAB_ram_f00429f2:
  FUN_ram_f00426f0(2);
  if (iVar2 != 0) {
    FUN_ram_f0042810(2);
  }
  if (!bVar1) {
    FUN_ram_f00428ac(0);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042a24 @ 0xf0042a24 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042a24(int param_1)

{
  int iVar1;
  int iVar2;
  undefined1 local_11;
  
  iVar1 = _DAT_ram_020a0064;
  if (*(char *)(param_1 + 0x14) == '\0') {
    if ((*(int *)(_DAT_ram_020a0064 + 0xf1c) == 1) &&
       (iVar2 = (*_DAT_ram_00014830)(_DAT_ram_00014830), -1 < iVar2 - *(int *)(iVar1 + 0xf40))) {
      (*_DAT_ram_00014a68)(0,*(undefined1 *)(iVar1 + 0xf45),&local_11,_DAT_ram_00014a68);
      FUN_ram_f0042998(local_11,1);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042b1c @ 0xf0042b1c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042b1c(void)

{
  int iVar1;
  
  iVar1 = _DAT_ram_020a0064;
  *(undefined1 *)(_DAT_ram_020a0064 + 0xf44) = 1;
  *(undefined4 *)(iVar1 + 0xf1c) = 0;
  FUN_ram_f00424d0();
  FUN_ram_f00266a0(iVar1 + 0xf20,&LAB_ram_f0042880,0);
  (*_DAT_ram_00014ad0)
            (*(undefined1 *)(iVar1 + 0xf4c),*(undefined1 *)(iVar1 + 0xf4d),
             *(undefined1 *)(iVar1 + 0xf4e),*(undefined2 *)(iVar1 + 0xf54),
             *(undefined2 *)(iVar1 + 0xf56),*(undefined1 *)(iVar1 + 0xf58),
             *(undefined1 *)(iVar1 + 0xf59),*(undefined1 *)(iVar1 + 0xf5a),
             *(undefined1 *)(iVar1 + 0xf5b));
  (*_DAT_ram_00014acc)
            (*(undefined1 *)(iVar1 + 0xf4c),*(undefined2 *)(iVar1 + 0xf50),
             *(undefined2 *)(iVar1 + 0xf52),*(undefined1 *)(iVar1 + 0xf58),_DAT_ram_00014acc);
  (*_DAT_ram_00014a58)(*(undefined1 *)(iVar1 + 0xf4c),1,_DAT_ram_00014a58);
  (*_DAT_ram_00014a54)(*(undefined1 *)(iVar1 + 0xf4c),0,_DAT_ram_00014a54);
  (*_DAT_ram_00014a54)(*(undefined1 *)(iVar1 + 0xf4c),1,_DAT_ram_00014a54);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042bcc @ 0xf0042bcc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0042bcc(undefined2 param_1)

{
  undefined4 auStack_c [2];
  
  (*_DAT_ram_000148d8)(param_1,auStack_c,_DAT_ram_000148d8);
  return auStack_c[0];
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042c48 @ 0xf0042c48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042c48(int param_1,int param_2)

{
  uint *puVar1;
  char cVar2;
  
  (*(code *)&SUB_ram_0006a0a0)(param_1,0,0x40);
  *(undefined1 *)(param_1 + 0x16) = 0;
  cVar2 = '\0';
  puVar1 = (uint *)(param_2 + 0x1c);
  do {
    cVar2 = cVar2 + '\x01';
    if ((*puVar1 >> 0x18 & 1) != 0) {
      *(undefined1 *)(param_1 + 0x16) = 1;
      *(char *)(param_1 + 0x12) = (char)*puVar1;
      *(undefined1 *)(param_1 + 0x13) = *(undefined1 *)((int)puVar1 + 1);
      *(undefined1 *)(param_1 + 0x14) = *(undefined1 *)((int)puVar1 + 2);
      *(byte *)(param_1 + 0x15) = ((byte)(*puVar1 >> 0x18) & 0xf) >> 1;
    }
    puVar1 = puVar1 + 1;
  } while (cVar2 != '\x02');
  *(undefined1 *)(param_1 + 0x1b) = 0;
  if ((*(uint *)(param_2 + 0x20) & 0x80) != 0) {
    *(undefined1 *)(param_1 + 0x1b) = 1;
    *(byte *)(param_1 + 0x1a) = (byte)*(uint *)(param_2 + 0x20) & 0xf;
  }
  _DAT_ram_f0065074 = *(undefined2 *)(param_2 + 8);
  *(undefined1 *)(param_1 + 0x37) = *(undefined1 *)(param_2 + 0x2c);
  *(undefined1 *)(param_1 + 0x38) = *(undefined1 *)(param_2 + 0x2d);
  *(undefined1 *)(param_1 + 0x39) = *(undefined1 *)(param_2 + 0x2e);
  *(undefined1 *)(param_1 + 0x3a) = *(undefined1 *)(param_2 + 0x2f);
  *(undefined1 *)(param_1 + 0x3b) = *(undefined1 *)(param_2 + 0x30);
  *(undefined1 *)(param_1 + 0x3c) = *(undefined1 *)(param_2 + 0x31);
  *(undefined1 *)(param_1 + 0x3d) = *(undefined1 *)(param_2 + 0x32);
  *(undefined1 *)(param_1 + 0x3e) = *(undefined1 *)(param_2 + 0x33);
  *(byte *)(param_1 + 0x36) = (byte)*(undefined4 *)(param_2 + 0x34) & 1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042d0c @ 0xf0042d0c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042d0c(int param_1)

{
  byte bVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  
  (*(code *)&SUB_ram_0006a0a0)(param_1,0,0x40);
  if (_DAT_ram_f006514c == 0x6625) {
    uVar2 = FUN_ram_f0042bcc(0);
    iVar4 = 1;
    *(byte *)(param_1 + 0x28) = (byte)(uVar2 >> 0x16) & 1;
    do {
      uVar2 = FUN_ram_f0042bcc(iVar4 * 2 & 0xffff);
      if ((uVar2 >> 0x1a & 1) != 0) {
        if (iVar4 == 1) break;
        if (iVar4 == 0) {
          *(ushort *)(param_1 + 8) = (ushort)(uVar2 >> 8) & 0x7f;
          *(short *)(param_1 + 10) = (short)((uVar2 & 0x3ffffff) >> 0xf);
          *(undefined1 *)(param_1 + 6) = 1;
        }
      }
      iVar4 = (int)(short)((short)iVar4 + -1);
    } while (iVar4 != -1);
    uVar2 = FUN_ram_f0042bcc(3);
    if ((uVar2 & 0x1800) == 0x800) {
      *(undefined1 *)(param_1 + 0x2a) = 1;
      *(ushort *)(param_1 + 0x32) = (ushort)uVar2 & 0x7ff;
      *(ushort *)(param_1 + 0x34) = (ushort)(uVar2 >> 0x10) & 0x7ff;
      uVar2 = FUN_ram_f0042bcc(2);
      *(ushort *)(param_1 + 0x2c) = (ushort)uVar2 & 0x7f;
      *(ushort *)(param_1 + 0x2e) = (ushort)(uVar2 >> 8) & 0x7ff;
      *(ushort *)(param_1 + 0x30) =
           ((ushort)(uVar2 >> 0x10) & 0x7fff) >> 0xb | (ushort)((uVar2 & 0x3f80000) >> 0xf);
    }
    goto LAB_ram_f0042e44;
  }
  bVar1 = FUN_ram_f0042bcc(2);
  if ((bVar1 & 0x80) == 0) {
    bVar1 = FUN_ram_f0042bcc(0);
    if ((bVar1 & 0x80) != 0) goto LAB_ram_f0042d50;
  }
  else {
LAB_ram_f0042d50:
    *(byte *)(param_1 + 0xc) = bVar1 & 0x7f;
  }
  uVar2 = FUN_ram_f0042bcc(2);
  if ((uVar2 & 0x4000000) == 0) {
    uVar2 = FUN_ram_f0042bcc(0);
    if ((uVar2 >> 0x1a & 1) != 0) goto LAB_ram_f0042d7c;
  }
  else {
LAB_ram_f0042d7c:
    *(undefined1 *)(param_1 + 6) = 1;
    *(ushort *)(param_1 + 8) = (ushort)(uVar2 >> 8) & 0x7f;
    *(short *)(param_1 + 10) = (short)((uVar2 & 0x3ffffff) >> 0xf);
  }
  uVar2 = FUN_ram_f0042bcc(0);
  *(byte *)(param_1 + 0x28) = (byte)(uVar2 >> 0x16) & 1;
  uVar2 = FUN_ram_f0042bcc(1);
  *(char *)(param_1 + 0x29) = (char)((uVar2 & 0x3fffff) >> 0x14);
LAB_ram_f0042e44:
  uVar2 = FUN_ram_f0042bcc(5);
  if ((uVar2 & 0x3000000) == 0x1000000) {
    *(undefined1 *)(param_1 + 0x26) = 1;
    bVar1 = (byte)(uVar2 >> 0x18) & 0x3c;
    if ((uVar2 >> 0x1d & 1) == 0) {
      bVar1 = bVar1 >> 2;
    }
    else {
      bVar1 = bVar1 >> 2 | 0xf0;
    }
    *(byte *)(param_1 + 0x27) = bVar1;
  }
  if ((uVar2 & 0x30000) == 0x10000) {
    *(undefined1 *)(param_1 + 0x1e) = 1;
    *(short *)(param_1 + 0x24) = (short)uVar2;
    uVar3 = FUN_ram_f0042bcc(4);
    *(short *)(param_1 + 0x22) = (short)((uint)uVar3 >> 0x10);
    *(short *)(param_1 + 0x20) = (short)uVar3;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042eb8 @ 0xf0042eb8 =====


undefined4 FUN_ram_f0042eb8(void)

{
  DAT_ram_f0064fca = 0;
  DAT_ram_f0064fc9 = 0;
  FUN_ram_f0042d0c();
  return 0;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_phy.c:0x67b <<<
// ===== FUN_ram_f0042ee4 @ 0xf0042ee4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042ee4(undefined4 param_1,int param_2)

{
  int iStack_1c;
  
  iStack_1c = param_2;
  (*_DAT_ram_f00650b8)(0,param_1,param_2,_DAT_ram_f00650b8);
  (*_DAT_ram_f00650bc)(0,param_1,&iStack_1c,_DAT_ram_f00650bc);
  if (iStack_1c != param_2) {
    (*_DAT_ram_f00650b8)(0,param_1,param_2,_DAT_ram_f00650b8);
    (*_DAT_ram_f00650bc)(0,param_1,&iStack_1c,_DAT_ram_f00650bc);
    if (iStack_1c != param_2) {
      (*_DAT_ram_00014814)(2,0xf0063c3c,param_1,param_2,iStack_1c,_DAT_ram_00014814);
      (*_DAT_ram_00014800)(0,0,0xf0063c54,0x67b,_DAT_ram_00014800);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042f70 @ 0xf0042f70 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_ram_f0042f70(void)

{
  return _DAT_ram_f0092cc4 >> 0x11 & 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042f80 @ 0xf0042f80 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042f80(int param_1,int param_2)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  
  if (param_2 == 0x24ba58) {
    puVar2 = (undefined4 *)(&DAT_ram_f006f090 + (uint)(byte)((param_1 != 0) * '\f') * 4);
    puVar1 = (undefined4 *)&DAT_ram_f0063f0c;
    do {
      puVar3 = (undefined4 *)*puVar1;
      puVar1 = puVar1 + 1;
      uVar4 = *puVar2;
      puVar2 = puVar2 + 1;
      *puVar3 = uVar4;
    } while (puVar1 != (undefined4 *)0xf0063f3c);
    _DAT_ram_60204e2c = 0xe4;
    _DAT_ram_60204e04 = 0x10000;
    FUN_ram_f0042ee4(0x6c,0);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0042ff4 @ 0xf0042ff4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0042ff4(void)

{
  uint local_18;
  uint auStack_14 [2];
  
  (*_DAT_ram_f00650b8)(4,0xa04,0xffffffff,_DAT_ram_f00650b8);
  (*_DAT_ram_f00650b8)(4,0x80,0xffffffff,_DAT_ram_f00650b8);
  (*_DAT_ram_f00650bc)(4,0x24,auStack_14,_DAT_ram_f00650bc);
  (*_DAT_ram_f00650bc)(4,0x120,&local_18,_DAT_ram_f00650bc);
  _DAT_ram_f0065148 = auStack_14[0] & 0xffff;
  local_18 = (local_18 & 0x3fffff) >> 0x14;
  _DAT_ram_f006514c = auStack_14[0] >> 0x10;
  (*_DAT_ram_00014814)(2,0xf0063c70);
  if ((_DAT_ram_f006514c == 0x6625) && (local_18 == 1)) {
    DAT_ram_f006503c = 1;
    (*_DAT_ram_00014814)(2,0xf0063c8c,_DAT_ram_00014814);
    DAT_ram_f006503d = 1;
  }
  else {
    DAT_ram_f006503d = 0;
    DAT_ram_f006503c = 0;
  }
  _DAT_ram_f0065158 = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00430c8 @ 0xf00430c8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00430c8(void)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  FUN_ram_f0042ff4();
  _DAT_ram_f0065154 = _DAT_ram_00014e94;
  if (_DAT_ram_00014e94 == (code *)0x0) {
    _DAT_ram_f0065154 = FUN_ram_f0042f70;
  }
  iVar2 = (*_DAT_ram_f0065154)(_DAT_ram_f0065154);
  if (iVar2 != 0) {
    _DAT_ram_f0064c14 = 0x1003bdde;
    _DAT_ram_f0064c64 = 0x6662d6;
    _DAT_ram_f0064c1c = 0x2a1e;
    _DAT_ram_f0064c24 = 0xc2120000;
    _DAT_ram_f0064c5c = 0x625ace;
  }
  puVar3 = (undefined4 *)&DAT_ram_f0064be0;
  _DAT_ram_602048cc = _DAT_ram_602048cc | 0x10000000;
  do {
    puVar4 = (undefined4 *)*puVar3;
    puVar1 = puVar3 + 1;
    puVar3 = puVar3 + 2;
    *puVar4 = *puVar1;
  } while (puVar3 != (undefined4 *)&DAT_ram_f0064e18);
  _DAT_ram_60200a04 = _DAT_ram_60200a04 & 0x800000 | 0x20581200;
  _DAT_ram_60200600 = _DAT_ram_60200600 & 0xdfffffff;
  _DAT_ram_60205c70 = _DAT_ram_60205c70 & 0xffff00ff | 0x800;
  if (DAT_ram_f006503c != '\0') {
    (*_DAT_ram_00014eb4)(0x4c,0x42004200,_DAT_ram_00014eb4);
  }
  if (DAT_ram_f006503d == '\x01') {
    _DAT_ram_60205c60 = _DAT_ram_60205c60 & 0xffff00ff | 0x3100;
    _DAT_ram_60205c70 = _DAT_ram_60205c70 | 0x3f00000;
    _DAT_ram_60205c04 = 0x30002737;
    _DAT_ram_60204028 = 0x3801;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004322c @ 0xf004322c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004322c(void)

{
  uint uVar1;
  
  _DAT_ram_f0065174 = 0;
  uVar1 = (*_DAT_ram_0001482c)(_DAT_ram_0001482c);
  _DAT_ram_f0065174 = (uint)((uVar1 & 1) != 0);
  (*_DAT_ram_00014814)(2,0xf0063c9c,_DAT_ram_f0065174,_DAT_ram_00014814);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0043270 @ 0xf0043270 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint * FUN_ram_f0043270(uint param_1)

{
  uint *puVar1;
  ushort uVar2;
  ushort uVar3;
  
  _DAT_ram_f006515c = &DAT_ram_f0063db4;
  puVar1 = (uint *)&DAT_ram_f0063fbc;
  _DAT_ram_f0065128 = &DAT_ram_f0063fbc;
  if (param_1 < 0x25e721) {
    uVar3 = 0xe;
    puVar1 = (uint *)&DAT_ram_f0063db4;
  }
  else {
    if (param_1 < 4000000) {
      return (uint *)0x0;
    }
    if (_DAT_ram_f006514c != 0x6625) {
      return (uint *)0x0;
    }
    uVar3 = 0x54;
  }
  uVar2 = 0;
  for (; *puVar1 != param_1; puVar1 = puVar1 + 3) {
    if (uVar3 <= (ushort)(uVar2 + 1)) {
      return (uint *)0x0;
    }
    uVar2 = uVar2 + 1;
  }
  if (uVar3 <= uVar2) {
    return (uint *)0x0;
  }
  _DAT_ram_f0065128 = &DAT_ram_f0063fbc;
  _DAT_ram_f006515c = &DAT_ram_f0063db4;
  return puVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00432dc @ 0xf00432dc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00432dc(uint *param_1)

{
  ushort uVar1;
  undefined2 uVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  undefined4 *puVar6;
  undefined4 uVar7;
  uint uVar8;
  
  if (*param_1 < 0x25e721) {
    if (_DAT_ram_f006514c != 0x6625) goto LAB_ram_f0043e0a;
    _DAT_ram_60000058 = _DAT_ram_60000058 & 0xfefffeff;
    FUN_ram_f0042ee4(0x88,0x52325122);
    FUN_ram_f0042ee4(0x8c,0x1e1e191e);
    FUN_ram_f0042ee4(0x90,0x40d0a0d);
    FUN_ram_f0042ee4(0x94,0x18191419);
    FUN_ram_f0042ee4(0x98,0x30a070a);
    _DAT_ram_60200838 = _DAT_ram_60200838 & 0xfe01ffff | 0x3e0000;
    _DAT_ram_60205c68 = CONCAT22(_DAT_ram_f0065070,_DAT_ram_f006506e);
    if (DAT_ram_f0065040 == '\0') {
      _DAT_ram_60205c68 = _DAT_ram_60205c68 + 0xa0000;
    }
    else if (DAT_ram_f0065040 == '\x02') {
      _DAT_ram_60205c68 = _DAT_ram_60205c68 + -0xa0000;
    }
    _DAT_ram_60200834 = _DAT_ram_60200834 & 0xffffff00;
    if (DAT_ram_f006504c != '\0') {
      uVar2 = (*_DAT_ram_00014eb0)(0xe0,_DAT_ram_00014eb0);
      FUN_ram_f0042ee4(0xe0,CONCAT22(_DAT_ram_f0065072,uVar2));
    }
    if (-1 < _DAT_ram_601200fc) {
      if (DAT_ram_f006503c == '\0') {
        _DAT_ram_602048e0 = 0xeeeaded2;
        _DAT_ram_60204870 = _DAT_ram_60204870 & 0xfdffffff;
        _DAT_ram_60204824 = 0x80469652;
        _DAT_ram_60204864 = 0x81000202;
        puVar6 = (undefined4 *)&DAT_ram_60200a40;
        uVar7 = 0x3d6200;
      }
      else {
        _DAT_ram_60204864 = 0x81000101;
        _DAT_ram_602048b8 = 0x1003accd;
        _DAT_ram_602048e0 = 0xeeecdbd2;
        _DAT_ram_60204870 = _DAT_ram_60204870 & 0xfff | 0x200000;
        _DAT_ram_60204824 = 0x80469653;
        _DAT_ram_6020483c = 0x471a1466;
        puVar6 = (undefined4 *)&DAT_ram_6020482c;
        _DAT_ram_60200a40 = 0x3d6200;
        uVar7 = 0x7301abaa;
        _DAT_ram_60205210 = 0x3d1222;
      }
      _DAT_ram_60204844 = 0xa8adf0a;
      _DAT_ram_602048d0 = 0xbd120202;
      _DAT_ram_60204800 = 0x2c1e;
      _DAT_ram_60200a48 = 0x18a76085;
      _DAT_ram_60200a04 = _DAT_ram_60200a04 & 0x800000 | 0x20581200;
      _DAT_ram_60204c40 = 0xafc248;
      _DAT_ram_602048dc = 0x9f5ddc5;
      _DAT_ram_60204848 = 0xfafabd1;
      _DAT_ram_6020480c = 0x161f2b2b;
      *puVar6 = uVar7;
      _DAT_ram_6020486c = 0xf8f800;
    }
    _DAT_ram_602048d0 =
         (_DAT_ram_602048d0 & 0xff000000) + (uint)(DAT_ram_f006504d >> 1) * -0x1000000 |
         _DAT_ram_602048d0 & 0xffffff;
    puVar3 = (uint *)&DAT_ram_60204848;
    uVar4 = (_DAT_ram_60204848 & 0xff0) + (uint)(DAT_ram_f006504d >> 1) * -0x10 & 0xff0;
    uVar5 = _DAT_ram_60204848 & 0xfffff00f;
  }
  else {
    _DAT_ram_60000058 = _DAT_ram_60000058 & 0xfefffeff | 0x1000000;
    FUN_ram_f0042ee4(0x88,0x22212221);
    FUN_ram_f0042ee4(0x8c,0x13131319);
    FUN_ram_f0042ee4(0x90,0x808080b);
    FUN_ram_f0042ee4(0x94,0x13131319);
    FUN_ram_f0042ee4(0x98,0x808080b);
    uVar4 = *param_1;
    if (uVar4 - 0x4ebc40 < 0x2bf21) {
      uVar5 = (*_DAT_ram_00014eb0)(0x59c,_DAT_ram_00014eb0);
      FUN_ram_f0042ee4(0x59c,uVar5 & 0xfff8ffff | 0x50000);
LAB_ram_f004386c:
      if (uVar4 < 5400000) {
        if (uVar4 < 5200000) {
          if (uVar4 < 5000000) {
            uVar5 = (*_DAT_ram_00014eb0)(0x520,_DAT_ram_00014eb0);
            uVar5 = uVar5 & 0xc1ffffff | 0x30000000;
          }
          else {
            uVar5 = (*_DAT_ram_00014eb0)(0x520,_DAT_ram_00014eb0);
            uVar5 = uVar5 & 0xc1ffffff | 0x32000000;
          }
          FUN_ram_f0042ee4(0x520,uVar5);
          goto LAB_ram_f0043944;
        }
        uVar5 = (*_DAT_ram_00014eb0)(0x520,_DAT_ram_00014eb0);
        uVar5 = uVar5 & 0xc1ffffff | 0x34000000;
      }
      else {
        uVar5 = (*_DAT_ram_00014eb0)(0x520,_DAT_ram_00014eb0);
        uVar5 = uVar5 & 0xc1ffffff | 0x36000000;
      }
      FUN_ram_f0042ee4(0x520,uVar5);
LAB_ram_f0043944:
      if (5499999 < uVar4) {
        uVar7 = 0x44444444;
        goto LAB_ram_f0043958;
      }
      if (uVar4 < 0x517b60) {
        uVar7 = 0x66666666;
      }
      else {
        uVar7 = 0x55555555;
      }
      FUN_ram_f0042ee4(0x584,uVar7);
    }
    else {
      if (uVar4 - 5500000 < 0x67c29) {
        uVar5 = (*_DAT_ram_00014eb0)(0x59c,_DAT_ram_00014eb0);
        uVar5 = uVar5 & 0xfff8ffff | 0x60000;
      }
      else {
        uVar5 = (*_DAT_ram_00014eb0)(0x59c,_DAT_ram_00014eb0);
        uVar5 = uVar5 & 0xfff8ffff | 0x40000;
      }
      FUN_ram_f0042ee4(0x59c,uVar5);
      if (uVar4 < 5800000) {
        if (uVar4 < 5600000) goto LAB_ram_f004386c;
        uVar5 = (*_DAT_ram_00014eb0)(0x520,_DAT_ram_00014eb0);
        FUN_ram_f0042ee4(0x520,uVar5 & 0xc1ffffff | 0x38000000);
        if (0x565d5f < uVar4) goto LAB_ram_f004393a;
        goto LAB_ram_f0043944;
      }
      uVar5 = (*_DAT_ram_00014eb0)(0x520,_DAT_ram_00014eb0);
      FUN_ram_f0042ee4(0x520,uVar5 & 0xc1ffffff | 0x3a000000);
LAB_ram_f004393a:
      uVar7 = 0x33333333;
LAB_ram_f0043958:
      FUN_ram_f0042ee4(0x584,uVar7);
    }
    puVar3 = (uint *)&DAT_ram_f0063d90;
    uVar5 = 0;
    do {
      uVar8 = uVar5;
      uVar5 = uVar8 + 1 & 0xff;
      if (uVar4 < *puVar3) {
        uVar8 = 0;
        break;
      }
      puVar3 = puVar3 + 1;
    } while (uVar5 != 8);
    FUN_ram_f0042ee4(0x55c,*(undefined4 *)(uVar8 * 0xc + -0xff9c2cc));
    FUN_ram_f0042ee4(0x554,*(undefined4 *)(uVar8 * 0xc + -0xff9c2c8));
    if (DAT_ram_f006504c != '\0') {
      if (uVar4 < 0x57a968) {
        uVar4 = (*_DAT_ram_00014eb0)(0x55c,_DAT_ram_00014eb0);
        uVar1 = _DAT_ram_f006fda4;
      }
      else {
        uVar4 = (*_DAT_ram_00014eb0)(0x55c,_DAT_ram_00014eb0);
        uVar1 = _DAT_ram_f006fda6;
      }
      FUN_ram_f0042ee4(0x55c,uVar4 & 0xffff0000 | (uint)uVar1);
    }
    if (DAT_ram_f0065040 == '\x01') {
      _DAT_ram_60205c68 = *(int *)(&DAT_ram_f0070148 + uVar8 * 4);
    }
    else if (DAT_ram_f0065040 == '\0') {
      _DAT_ram_60205c68 = *(int *)(&DAT_ram_f0070148 + uVar8 * 4) + 0x100000;
    }
    else {
      _DAT_ram_60205c68 = *(int *)(&DAT_ram_f0070148 + uVar8 * 4) + -0x100000;
    }
    FUN_ram_f0042ee4(0x534,0x460047);
    FUN_ram_f0042ee4(0x538,0xc70044);
    FUN_ram_f0042ee4(0x53c,0xc400c6);
    FUN_ram_f0042ee4(0x540,0x1c601c7);
    FUN_ram_f0042ee4(0x544,0x3e701c4);
    FUN_ram_f0042ee4(0x548,0x3e403e6);
    FUN_ram_f0042ee4(0x54c,0x3ea03eb);
    FUN_ram_f0042ee4(0x550,0x3ea03ea);
    _DAT_ram_60200834 = _DAT_ram_60200834 & 0xffffff00 | 0xf8;
    if (-1 < _DAT_ram_601200fc) {
      if (DAT_ram_f006503c == '\0') {
        _DAT_ram_60204800 = 0x2c20;
        _DAT_ram_602048e0 = 0xe8e4d5c9;
        _DAT_ram_60204870 = _DAT_ram_60204870 | 0x2000000;
        _DAT_ram_60204864 = 0x81000404;
        uVar7 = 0x3d6000;
        puVar6 = (undefined4 *)&DAT_ram_60200a40;
      }
      else {
        _DAT_ram_60204800 = 0x2b20;
        _DAT_ram_60204864 = 0x81000101;
        _DAT_ram_602048b8 = 0x1002acce;
        _DAT_ram_602048e0 = 0xe8e5d6c9;
        _DAT_ram_60204870 = _DAT_ram_60204870 & 0xfff | 0x2200000;
        _DAT_ram_6020483c = 0x471a1476;
        puVar6 = (undefined4 *)&DAT_ram_6020482c;
        _DAT_ram_60200a40 = 0x3d6000;
        uVar7 = 0x7301aba9;
        _DAT_ram_60205210 = 0x3d2222;
      }
      _DAT_ram_60204844 = 0xa8a4f0a;
      _DAT_ram_60204824 = 0x80369652;
      _DAT_ram_602048d0 = 0xba140606;
      _DAT_ram_60200a48 = 0x18a76085;
      _DAT_ram_60200a04 = _DAT_ram_60200a04 & 0x800000 | 0x20781200;
      _DAT_ram_60204c40 = 0xaf8248;
      _DAT_ram_602048dc = 0xef4e0ca;
      _DAT_ram_60204848 = 0x606ba1;
      _DAT_ram_6020480c = 0x1a1f2b2b;
      *puVar6 = uVar7;
    }
    _DAT_ram_602048d0 =
         (_DAT_ram_602048d0 & 0xff000000) + (uint)(DAT_ram_f006504e >> 1) * -0x1000000 |
         _DAT_ram_602048d0 & 0xffffff;
    _DAT_ram_60204848 =
         (_DAT_ram_60204848 & 0xff0) + (uint)(DAT_ram_f006504e >> 1) * -0x10 & 0xff0 |
         _DAT_ram_60204848 & 0xfffff00f;
    puVar3 = (uint *)&DAT_ram_60200838;
    uVar5 = 0x3e0000;
    uVar4 = _DAT_ram_60200838 & 0xfe01ffff;
  }
  *puVar3 = uVar4 | uVar5;
LAB_ram_f0043e0a:
  _DAT_ram_60205cd0 = 7;
  _DAT_ram_60205cd4 = 6;
  _DAT_ram_60205cd8 = 4;
  _DAT_ram_60205cdc = 7;
  _DAT_ram_60205ce0 = 6;
  _DAT_ram_60205ce4 = 4;
  _DAT_ram_60205ce8 = 7;
  _DAT_ram_60205cec = 6;
  _DAT_ram_60205cf0 = 0x404;
  _DAT_ram_60205cf4 = 0x17;
  _DAT_ram_60205cf8 = 0x16;
  _DAT_ram_60205cfc = 0x14;
  _DAT_ram_60205d00 = 0x1b;
  _DAT_ram_60205d04 = 0x1a;
  _DAT_ram_60205c00 = _DAT_ram_60205c00 & 0xffffff00 | 0xc0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0043ed0 @ 0xf0043ed0 =====


char FUN_ram_f0043ed0(undefined4 param_1,char param_2,int param_3)

{
  char cVar1;
  
  if (param_3 == 1) {
    cVar1 = '\x02';
  }
  else {
    if (param_3 != 3) {
      return param_2;
    }
    cVar1 = -2;
  }
  return param_2 + cVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0043ee4 @ 0xf0043ee4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0043ee4(uint *param_1)

{
  uint uVar1;
  
  if (*param_1 < 0x25e721) {
    uVar1 = (*_DAT_ram_00014eb0)(0xb0,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0xb0,uVar1 & 0xfffc000f | param_1[1]);
    FUN_ram_f0042ee4(0x10c,param_1[2]);
    if (_DAT_ram_f006514c != 0x6625) {
      return;
    }
    uVar1 = (*_DAT_ram_00014eb0)(0x500,_DAT_ram_00014eb0);
    uVar1 = uVar1 & 0xfffffffe;
  }
  else {
    uVar1 = (*_DAT_ram_00014eb0)(0x520,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0x520,uVar1 & 0xffffc000 | param_1[1]);
    FUN_ram_f0042ee4(0x504,param_1[2]);
    uVar1 = (*_DAT_ram_00014eb0)(0x500,_DAT_ram_00014eb0);
    uVar1 = uVar1 | 3;
  }
  FUN_ram_f0042ee4(0x500,uVar1);
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_phy.c:0xe91 <<<
// ===== FUN_ram_f0043f84 @ 0xf0043f84 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0043f84(void)

{
  (*_DAT_ram_00014800)(0,0,0xf0063c54,0xe91,_DAT_ram_00014800);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0043fac @ 0xf0043fac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0043fac(int param_1,char param_2)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  int iVar4;
  
  iVar4 = 0;
  if (param_2 == '\0') {
    do {
      uVar2 = (*_DAT_ram_00014eb0)(*(undefined4 *)(iVar4 + -0xff9c080),_DAT_ram_00014eb0);
      *(undefined4 *)(param_1 + iVar4) = uVar2;
      iVar4 = iVar4 + 4;
    } while (iVar4 != 0x3c);
  }
  else {
    iVar4 = 0;
    do {
      puVar3 = (undefined4 *)(iVar4 + -0xff9c080);
      puVar1 = (undefined4 *)(param_1 + iVar4);
      iVar4 = iVar4 + 4;
      FUN_ram_f0042ee4(*puVar3,*puVar1);
    } while (iVar4 != 0x3c);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044008 @ 0xf0044008 =====


void FUN_ram_f0044008(int param_1,char param_2)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  
  iVar2 = 0;
  if (param_2 == '\0') {
    do {
      *(undefined4 *)(param_1 + iVar2) = **(undefined4 **)(iVar2 + -0xff9c0c4);
      iVar2 = iVar2 + 4;
    } while (iVar2 != 0x44);
  }
  else {
    iVar2 = 0;
    do {
      puVar3 = (undefined4 *)(iVar2 + -0xff9c0c4);
      puVar1 = (undefined4 *)(param_1 + iVar2);
      iVar2 = iVar2 + 4;
      *(undefined4 *)*puVar3 = *puVar1;
    } while (iVar2 != 0x44);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044048 @ 0xf0044048 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044048(uint param_1,int param_2,char param_3)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  uint uVar6;
  uint *puVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  undefined4 *puVar12;
  
  puVar7 = (uint *)&DAT_ram_f0063d90;
  uVar3 = 0;
  uVar6 = 0;
  do {
    uVar8 = uVar6 + 1 & 0xff;
    if (param_1 < *puVar7) break;
    puVar7 = puVar7 + 1;
    uVar3 = uVar6;
    uVar6 = uVar8;
  } while (uVar8 != 8);
  if ((DAT_ram_f0065038 != uVar3) || (param_3 != '\0')) {
    iVar9 = uVar3 * 0x124;
    iVar10 = iVar9 + -0xff90f10;
    DAT_ram_f0065038 = (byte)uVar3;
    if ((&DAT_ram_f006f211)[iVar9] != '\0') {
      iVar11 = 0;
      do {
        puVar1 = (undefined4 *)(iVar11 + -0xff9c154);
        puVar12 = (undefined4 *)(iVar10 + iVar11);
        iVar11 = iVar11 + 4;
        FUN_ram_f0042ee4(*puVar1,*puVar12);
      } while (iVar11 != 0x40);
      puVar1 = (undefined4 *)(((param_2 != 0) + 2) * 0x20 + iVar10);
      puVar12 = (undefined4 *)&DAT_ram_f0063eec;
      do {
        uVar2 = *puVar12;
        puVar12 = puVar12 + 1;
        uVar4 = *puVar1;
        puVar1 = puVar1 + 1;
        FUN_ram_f0042ee4(uVar2,uVar4);
      } while (puVar12 != (undefined4 *)&DAT_ram_f0063f0c);
      puVar12 = (undefined4 *)&DAT_ram_f0063e7c;
      puVar1 = (undefined4 *)(iVar9 + -0xff90e88);
      do {
        puVar5 = (undefined4 *)*puVar12;
        puVar12 = puVar12 + 1;
        uVar2 = *puVar1;
        puVar1 = puVar1 + 1;
        *puVar5 = uVar2;
        (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
      } while (puVar12 != (undefined4 *)0xf0063eac);
      _DAT_ram_60205c60 = *(undefined4 *)(iVar9 + -0xff90df8);
      uVar3 = (*_DAT_ram_00014eb0)(0xc0,_DAT_ram_00014eb0);
      FUN_ram_f0042ee4(0xc0,uVar3 | 0x300);
      FUN_ram_f0042ee4(0x114,*(uint *)(iVar9 + -0xff90e90) & 0xff | 0x84000000 |
                             (*(uint *)(iVar9 + -0xff90e90) & 0xff0000) >> 8);
      FUN_ram_f0042ee4(0x114,*(uint *)(iVar9 + -0xff90e8c) & 0xff |
                             (*(uint *)(iVar9 + -0xff90e8c) & 0xff0000) >> 8 | 0x88000000);
      uVar3 = (*_DAT_ram_00014eb0)(0xc0,_DAT_ram_00014eb0);
      FUN_ram_f0042ee4(0xc0,uVar3 & 0xfffffcff);
      puVar12 = (undefined4 *)&DAT_ram_f0063f0c;
      puVar1 = (undefined4 *)(iVar10 + ((uint)(param_2 != 0) * 0xc + 0x2e) * 4);
      do {
        puVar5 = (undefined4 *)*puVar12;
        puVar12 = puVar12 + 1;
        uVar2 = *puVar1;
        puVar1 = puVar1 + 1;
        *puVar5 = uVar2;
      } while (puVar12 != (undefined4 *)0xf0063f3c);
      _DAT_ram_60204e2c = 0xe4;
      _DAT_ram_60204e04 = 0x10000;
      FUN_ram_f0042ee4(0x6c,0);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044204 @ 0xf0044204 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044204(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  char cVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 local_2c [4];
  
  FUN_ram_f0042ee4(0x500,0x3fd61e1);
  FUN_ram_f0042ee4(0x44,0x880082d0);
  FUN_ram_f0042ee4(0x61c,4);
  FUN_ram_f0042ee4(200,0x820040c0);
  (*_DAT_ram_00014ea4)(0x41c,0xf0000000,1,_DAT_ram_00014ea4);
  FUN_ram_f0042ee4(200,0x20040c0);
  iVar2 = 0;
  do {
    puVar5 = (undefined4 *)(param_1 + iVar2 * 4);
    cVar3 = '\0';
    puVar4 = (undefined4 *)(iVar2 * 4 + -0xff9c154);
    do {
      uVar1 = *puVar4;
      puVar4 = puVar4 + 1;
      cVar3 = cVar3 + '\x01';
      uVar1 = (*_DAT_ram_00014eb0)(uVar1,_DAT_ram_00014eb0);
      *puVar5 = uVar1;
      puVar5 = puVar5 + 1;
    } while (cVar3 != '\b');
    iVar2 = iVar2 + 8;
  } while (iVar2 != 0x10);
  iVar2 = 0;
  local_2c[1] = 5;
  local_2c[0] = 0;
  FUN_ram_f0042ee4(0x610,0x44440b);
  do {
    FUN_ram_f0042ee4(0x61c,*(undefined4 *)((int)local_2c + iVar2));
    FUN_ram_f0042ee4(200,0x820040c0);
    (*_DAT_ram_00014ea4)(0x41c,0xf0000000,1,_DAT_ram_00014ea4);
    FUN_ram_f0042ee4(200,0x20040c0);
    puVar4 = (undefined4 *)((iVar2 + 8) * 8 + param_1);
    puVar5 = (undefined4 *)&DAT_ram_f0063eec;
    do {
      uVar1 = *puVar5;
      puVar5 = puVar5 + 1;
      uVar1 = (*_DAT_ram_00014eb0)(uVar1,puVar4,_DAT_ram_00014eb0);
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (puVar5 != (undefined4 *)&DAT_ram_f0063f0c);
    iVar2 = iVar2 + 4;
  } while (iVar2 != 8);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044344 @ 0xf0044344 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044344(int param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined4 local_30 [5];
  
  puVar2 = local_30;
  local_30[0] = 0x80;
  local_30[1] = 0x90;
  local_30[2] = 0xa0;
  local_30[3] = 0xb0;
  FUN_ram_f0042ee4(0x500,0x3fd61e1);
  FUN_ram_f0042ee4(0x6c,0x800060fe);
  FUN_ram_f0042ee4(0xc0,0x8f092300);
  do {
    uVar1 = *puVar2;
    puVar2 = puVar2 + 1;
    FUN_ram_f0042ee4(0x70,uVar1);
    FUN_ram_f0042ee4(0xcc,0x8000);
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    (*_DAT_ram_00014ea4)(0x404,0x38000,1,_DAT_ram_00014ea4);
    FUN_ram_f0042ee4(0x70,0);
    FUN_ram_f0042ee4(0xcc,0);
  } while (puVar2 != local_30 + 4);
  uVar1 = (*_DAT_ram_00014eb0)(0x46c,_DAT_ram_00014eb0);
  *(undefined4 *)(param_1 + 0x80) = uVar1;
  uVar1 = (*_DAT_ram_00014eb0)(0x470,_DAT_ram_00014eb0);
  *(undefined4 *)(param_1 + 0x84) = uVar1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044404 @ 0xf0044404 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044404(int param_1,byte param_2)

{
  undefined4 *puVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  uint local_70 [21];
  
  uVar2 = (uint)param_2;
  local_70[0xc] = 0x4403e8;
  local_70[0xd] = 0x4403ea;
  local_70[0xe] = 0x4403eb;
  local_70[0xf] = 0x4403e4;
  local_70[0x10] = 0x4403e6;
  local_70[6] = 0x5400038;
  local_70[7] = 0x540003a;
  local_70[8] = 0x540003b;
  local_70[9] = 0x5400034;
  local_70[10] = 0x5400036;
  local_70[0] = 0x10000116;
  local_70[1] = 0x1000011c;
  local_70[2] = 0x10000124;
  local_70[0x13] = 0x75b02800;
  local_70[3] = 0x1000012d;
  local_70[0x11] = 0x4403e7;
  local_70[0xb] = 0x5400037;
  local_70[5] = 0x10000148;
  local_70[4] = 0x10000139;
  local_70[0x12] = 0x75b0e800;
  FUN_ram_f0042ee4(0x500,0x3fd61e1);
  FUN_ram_f0042ee4(0x6c,0x81b06e07);
  FUN_ram_f0042ee4(0x48,0xf8300727);
  FUN_ram_f0042ee4(0xa0,0x900f777e);
  FUN_ram_f0042ee4(0x70,0x5000c00);
  if (DAT_ram_f006503d == '\x01') {
    _DAT_ram_6020080c = 0x300594;
    (*_DAT_ram_0001480c)();
  }
  FUN_ram_f0042ee4(200,0x820240c0);
  (*_DAT_ram_00014ea4)(0x41c,0xf0000000,1,_DAT_ram_00014ea4);
  FUN_ram_f0042ee4(200,0x20240c0);
  uVar3 = uVar2;
  while( true ) {
    if (uVar2 < uVar3) break;
    iVar5 = 0;
    do {
      if (uVar3 == 0) {
        FUN_ram_f0042ee4(0x534,*(undefined4 *)((int)local_70 + iVar5 + 0x30));
      }
      else {
        FUN_ram_f0042ee4(0x534,*(uint *)((int)local_70 + iVar5 + 0x30) & 0xfffffdff);
      }
      FUN_ram_f0042ee4(0x514,local_70[uVar3 + 0x12]);
      if (uVar3 == 0) {
        _DAT_ram_60205c3c = *(uint *)((int)local_70 + iVar5 + 0x18);
      }
      else {
        _DAT_ram_60205c3c = *(uint *)((int)local_70 + iVar5 + 0x18) & 0xffffffef;
      }
      _DAT_ram_60205c0c = *(undefined4 *)((int)local_70 + iVar5);
      _DAT_ram_60205c08 = 1;
      (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
      (*_DAT_ram_00014ebc)(0x60205d34,0x78000000,0x32,_DAT_ram_00014ebc);
      _DAT_ram_60205c08 = 2;
      iVar5 = iVar5 + 4;
      (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
      (*_DAT_ram_00014ebc)(0x60205d34,0x78000000,0x32,_DAT_ram_00014ebc);
      _DAT_ram_60205c08 = 0;
    } while (iVar5 != 0x18);
    uVar3 = uVar3 + 1 & 0xff;
  }
  uVar3 = uVar2 * 3;
  while( true ) {
    uVar4 = uVar3 & 0xff;
    uVar3 = uVar4 + 1;
    if (((uVar2 + 1) * 3 & 0xff) <= uVar4) break;
    puVar1 = *(undefined4 **)(&DAT_ram_f0063e7c + (uVar4 + 6) * 4);
    *(undefined4 *)(param_1 + (uVar4 + 0x22) * 4) = **(undefined4 **)(&DAT_ram_f0063e7c + uVar4 * 4)
    ;
    *(undefined4 *)(param_1 + (uVar4 + 0x28) * 4) = *puVar1;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044768 @ 0xf0044768 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044768(int param_1)

{
  _DAT_ram_60205c00 = _DAT_ram_60205c00 & 0x6fefff | 0x101000;
  _DAT_ram_6020080c = _DAT_ram_6020080c & 0xffffffe0 | 0x17;
  FUN_ram_f0042ee4(0x54,0x63ff0003);
  _DAT_ram_60205620 = _DAT_ram_60205620 | 1;
  (*_DAT_ram_00014eb4)(200,0x820240c0,_DAT_ram_00014eb4);
  (*_DAT_ram_0001480c)(3,_DAT_ram_0001480c);
  (*_DAT_ram_00014ea4)(0x41c,0x70000000,10);
  (*_DAT_ram_00014eb4)(200,0x20240c0,_DAT_ram_00014eb4);
  _DAT_ram_60205c60 = 0x400027f0;
  (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
  (*_DAT_ram_00014ebc)(0x60205c74,0x70000000,10);
  _DAT_ram_60205c60 = _DAT_ram_60205c60 & 0xffffff0f;
  _DAT_ram_60205c00 = _DAT_ram_60205c00 & 0xffefffff;
  _DAT_ram_6020080c = _DAT_ram_6020080c & 0xffffffe0;
  FUN_ram_f0042ee4(0x54,0x3ff0003);
  (*_DAT_ram_00014eb4)(200,0x20040c0,_DAT_ram_00014eb4);
  *(uint *)(param_1 + 0x118) = _DAT_ram_60205c60;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044898 @ 0xf0044898 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044898(int param_1)

{
  char cVar1;
  int iVar2;
  undefined4 *puVar3;
  char cVar4;
  int iStack_70;
  undefined4 *puStack_6c;
  undefined4 local_58 [8];
  undefined4 local_38 [7];
  
  local_38[2] = 0x5000ca0;
  local_38[3] = 0x5000cb0;
  local_38[0] = 0x5000c80;
  local_38[1] = 0x5000c90;
  if (_DAT_ram_f0064f98 == 0x8127) {
    local_58[4] = 0x800b7b3e;
  }
  else {
    local_58[4] = 0x800b7b7e;
  }
  local_58[3] = 0x900b777e;
  local_58[0] = 0x900b777e;
  local_58[1] = 0x900b777e;
  local_58[2] = 0x900b777e;
  local_38[5] = 0x24000001;
  local_38[4] = 0x24000000;
  local_58[5] = local_58[4];
  local_58[6] = local_58[4];
  local_58[7] = local_58[4];
  (*_DAT_ram_00014d28)(1,1,0,0,_DAT_ram_00014d28);
  _DAT_ram_60204e00 = 0x6020001;
  _DAT_ram_60204e04 = 0;
  _DAT_ram_60205804 = 0;
  FUN_ram_f0042ee4(200,0x20040c0);
  FUN_ram_f0042ee4(0x500,0x3fd61e1);
  FUN_ram_f0042ee4(0x6c,0x81bd6c7f);
  FUN_ram_f0042ee4(0x44,0x8c0082d0);
  FUN_ram_f0042ee4(0x48,0xf8b00727);
  FUN_ram_f0042ee4(0x534,0x4400c7);
  _DAT_ram_60205c3c = 0x5000027;
  if (DAT_ram_f006503d == '\x01') {
    _DAT_ram_6020080c = 0x300594;
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
  }
  puStack_6c = local_38 + 4;
  iStack_70 = 0;
  cVar1 = '\0';
  do {
    iVar2 = 0;
    puVar3 = (undefined4 *)(param_1 + (iStack_70 + 0x2e) * 4);
    cVar4 = '\0';
    do {
      FUN_ram_f0042ee4(0x70,*(undefined4 *)((int)local_38 + iVar2));
      if (cVar1 == '\0') {
        FUN_ram_f0042ee4(0xa0,*(undefined4 *)((int)local_58 + iVar2 + 0x10));
      }
      else {
        FUN_ram_f0042ee4(0xa0,*(undefined4 *)((int)local_58 + iVar2));
      }
      FUN_ram_f0042ee4(200,0x820240c0);
      (*_DAT_ram_00014ea4)(0x41c,0xf0000000,1,_DAT_ram_00014ea4);
      FUN_ram_f0042ee4(200,0x20240c0);
      _DAT_ram_60204600 = 0x2a0140;
      _DAT_ram_60204604 = 0x1000400;
      _DAT_ram_60205804 = 0x4980;
      _DAT_ram_60205c00 = 0x1010;
      _DAT_ram_60205c08 = 0x4f400;
      if ((cVar4 == '\x03') || (cVar1 == '\x01')) {
        _DAT_ram_60205c0c = 0x2954;
      }
      else {
        _DAT_ram_60205c0c = 0x2965;
      }
      _DAT_ram_60205404 = 0x10017;
      _DAT_ram_60205400 = 0x100;
      _DAT_ram_60204010 = 0x40;
      _DAT_ram_60200600 = *puStack_6c;
      _DAT_ram_60204e00 = 0x9100005;
      (*_DAT_ram_0001480c)(2,_DAT_ram_0001480c);
      (*_DAT_ram_00014ebc)(0x60204e34,0x7c000000,5,_DAT_ram_00014ebc);
      *puVar3 = _DAT_ram_60204e08;
      puVar3[1] = _DAT_ram_60204e0c;
      puVar3[2] = _DAT_ram_60204e10;
      _DAT_ram_60204e00 = 0x6020001;
      _DAT_ram_60204e04 = 0;
      _DAT_ram_60205804 = 0;
      cVar4 = cVar4 + '\x01';
      FUN_ram_f0042ee4(200,0x20040c0);
      iVar2 = iVar2 + 4;
      puVar3 = puVar3 + 3;
    } while (cVar4 != '\x04');
    cVar1 = cVar1 + '\x01';
    puStack_6c = puStack_6c + 1;
    iStack_70 = iStack_70 + 0xc;
  } while (cVar1 != '\x02');
  _DAT_ram_60204e00 = 0x2100001;
  _DAT_ram_60205c3c = 0x5000000;
  _DAT_ram_60205400 = 0;
  FUN_ram_f0042ee4(0x6c,0);
  FUN_ram_f0042ee4(0x44,0x10082d0);
  FUN_ram_f0042ee4(0x48,0x300727);
  FUN_ram_f0042ee4(0x70,0);
  FUN_ram_f0042ee4(0xa0,0x6f5fe);
  FUN_ram_f0042ee4(200,0x20040c0);
  FUN_ram_f0042ee4(0xa8,0x40000);
  (*_DAT_ram_00014d28)(0,0,0,0,_DAT_ram_00014d28);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044c94 @ 0xf0044c94 =====


void FUN_ram_f0044c94(int param_1,int param_2,int param_3,int param_4)

{
  char cVar1;
  uint uVar2;
  char cVar3;
  uint *puVar4;
  
  if (param_1 == 5000000) {
    puVar4 = (uint *)&DAT_ram_f0063d90;
    cVar3 = '\0';
    do {
      uVar2 = *puVar4;
      cVar1 = cVar3 + '\x01';
      puVar4 = puVar4 + 1;
      if (param_3 * 5000 + 5000000U < uVar2) {
        uVar2 = (uint)(byte)(cVar3 - 1);
        goto LAB_ram_f0044cdc;
      }
      cVar3 = cVar1;
    } while (cVar1 != '\t');
    uVar2 = 9;
LAB_ram_f0044cdc:
    (&DAT_ram_f006f210)[uVar2 * 0x124] = 1;
    while( true ) {
      uVar2 = uVar2 + 1 & 0xff;
      if ((7 < uVar2) ||
         ((param_3 + (param_4 + -1) * param_2) * 5000 + 5000000U <
          *(uint *)(&DAT_ram_f0063d90 + uVar2 * 4))) break;
      (&DAT_ram_f006f210)[uVar2 * 0x124] = 1;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044d2c @ 0xf0044d2c =====


void FUN_ram_f0044d2c(void)

{
  undefined1 *puVar1;
  
  puVar1 = &DAT_ram_f006f210;
  do {
    *puVar1 = 1;
    puVar1 = puVar1 + 0x124;
  } while (puVar1 != (undefined1 *)0xf006fb30);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044d48 @ 0xf0044d48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044d48(void)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)(*_DAT_ram_00014e9c)(_DAT_ram_00014e9c);
  if (puVar1 == (undefined4 *)0x0) {
    FUN_ram_f0043f84();
    _DAT_ram_f006512c = (*_DAT_ram_00014eb0)(0x88,_DAT_ram_00014eb0);
  }
  else {
    (*_DAT_ram_00014e8c)(_DAT_ram_00014e8c);
    _DAT_ram_60205c60 = *puVar1;
    _DAT_ram_60205c50 = puVar1[2];
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044d9c @ 0xf0044d9c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044d9c(void)

{
  undefined4 uVar1;
  
  if (_DAT_ram_f006514c == 0x6625) {
    if (DAT_ram_f006504c != '\0') {
      return;
    }
    if (DAT_ram_f006503c == '\0') {
      FUN_ram_f0042ee4(0xd8,0x3911fed1);
      uVar1 = 0x391c3911;
    }
    else {
      FUN_ram_f0042ee4(0xd8,0x3910fed1);
      uVar1 = 0x391c3910;
    }
  }
  else {
    FUN_ram_f0042ee4(0xd8,0x3911fed1);
    uVar1 = 0x39113911;
  }
  FUN_ram_f0042ee4(0xe0,uVar1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0044e24 @ 0xf0044e24 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044e24(char param_1)

{
  if (param_1 == '\0') {
    _DAT_ram_601200fc = _DAT_ram_601200fc & 0xfffffffc;
  }
  else {
    _DAT_ram_601200fc = _DAT_ram_601200fc & 0xfffffffc | 2;
  }
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_phy.c:0x808 <<<
// ===== FUN_ram_f0044e50 @ 0xf0044e50 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0044e50(uint param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  FUN_ram_f0044e24(0);
  if (param_1 < 0x25e721) {
    FUN_ram_f0042ee4(0x6c,0x80007004);
    uVar1 = (*_DAT_ram_00014eb0)(0xb4,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0xb4,uVar1 & 0xc3ffffff | 0x2c000000);
    uVar1 = (param_1 - 0x24a6d0) / 1000 & 0xff;
    if (uVar1 == 0x52) {
      uVar1 = 0x4f;
    }
    else if (0x4f < uVar1) {
      (*_DAT_ram_00014800)(0,0,0xf0063c54,0x808,_DAT_ram_00014800);
    }
    iVar2 = (*_DAT_ram_00014ec8)(uVar1,_DAT_ram_00014ec8);
    uVar1 = (*_DAT_ram_00014eb0)(0xb0,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0xb0,iVar2 << 0x14 | uVar1 & 0xf80fffff);
    if ((_DAT_ram_f0065148 < 0x8a01) && (_DAT_ram_f006514c != 0x6625)) {
      FUN_ram_f0042ee4(0xb4,0xedf0c010);
      (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
      (*_DAT_ram_00014ea4)(0x424,0xf0000000,1,_DAT_ram_00014ea4);
      uVar4 = 0xdf0c010;
      uVar3 = 0xb4;
    }
    else {
      FUN_ram_f0042ee4(0xb4,0xc130c010);
      (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
      (*_DAT_ram_00014ea4)(0x424,0xf0000000,1,_DAT_ram_00014ea4);
      FUN_ram_f0042ee4(0xb4,0x130c010);
      FUN_ram_f0042ee4(0xbc,0x6666b6c0);
      uVar4 = 0x16b54f84;
      uVar3 = 4;
    }
    FUN_ram_f0042ee4(uVar3,uVar4);
    FUN_ram_f0042ee4(0x6c,0);
    FUN_ram_f0044e24(0);
  }
  else {
    uVar1 = (*_DAT_ram_00014eb0)(0x500,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0x500,uVar1 | 2);
    FUN_ram_f0042ee4(0x590,0x1000040);
    uVar1 = (*_DAT_ram_00014eb0)(0x59c,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0x59c,uVar1 | 0xf00);
    FUN_ram_f0042ee4(0x590,0x1000840);
    FUN_ram_f0042ee4(0x590,0x1000040);
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    FUN_ram_f0042ee4(0x590,0x40);
    FUN_ram_f0042ee4(0x590,0x4040);
    FUN_ram_f0042ee4(0x590,0x40);
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    FUN_ram_f0042ee4(0x590,0x8c0);
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    FUN_ram_f0042ee4(0x590,0x40);
    FUN_ram_f0042ee4(0x590,0x4040);
    FUN_ram_f0042ee4(0x590,0x40);
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    FUN_ram_f0042ee4(0x590,0x1040);
    FUN_ram_f0042ee4(0x590,0x40);
    (*_DAT_ram_0001480c)(2,_DAT_ram_0001480c);
    uVar1 = (*_DAT_ram_00014eb0)(0x5a8,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0x5a8,uVar1 | 0xf00000);
    FUN_ram_f0042ee4(0x590,0x20010040);
    FUN_ram_f0042ee4(0x590,0x20000040);
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    FUN_ram_f0042ee4(0x590,0x20020040);
    FUN_ram_f0042ee4(0x590,0x20000040);
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    FUN_ram_f0042ee4(0x590,0x20040040);
    FUN_ram_f0042ee4(0x590,0x20000040);
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    FUN_ram_f0042ee4(0x590,0x40);
    FUN_ram_f0042ee4(0x590,0x80040);
    FUN_ram_f0042ee4(0x590,0x40);
    (*_DAT_ram_0001480c)(2,_DAT_ram_0001480c);
    FUN_ram_f0044e24(1);
  }
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_phy.c:0xba2 <<<
// ===== FUN_ram_f004518c @ 0xf004518c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f004518c(uint param_1,undefined4 param_2,undefined1 param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar2 = FUN_ram_f0043270(param_1,param_3);
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else {
    FUN_ram_f0043ee4();
    FUN_ram_f0044e50(param_1);
    FUN_ram_f00432dc(iVar2);
    if ((param_1 == 0x25e720) && (DAT_ram_f006503a != '\0')) {
      _DAT_ram_60205404 = _DAT_ram_60205404 | 0x8000;
    }
    else {
      _DAT_ram_60205404 = _DAT_ram_60205404 & 0xffff7fff;
    }
    uVar1 = param_1 / 1000;
    if (uVar1 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf0063c54,0xba2,_DAT_ram_00014800);
    }
    _DAT_ram_60200a24 =
         (((2500000 / uVar1 & 0x7fe) >> 1) + (2500000 / uVar1 & 1) & 0x3ff) << 10 |
         _DAT_ram_60200a24 & 0xfff003ff;
    if (param_1 < 0x25e721) {
      if (0x52 < (uVar1 - 0x62 & 0xff)) {
        (*_DAT_ram_00014800)(0,0,0xf0063c54,2999,_DAT_ram_00014800);
      }
      _DAT_ram_60200624 = uVar1 - 0x62 & 0x7f | _DAT_ram_60200624 & 0xffffff80;
    }
    uVar3 = 1;
  }
  return uVar3;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_phy.c:0x1321 <<<
// ===== FUN_ram_f00452b4 @ 0xf00452b4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00452b4(int param_1,char param_2,undefined4 param_3)

{
  int iVar1;
  
  if ((_DAT_ram_f00650a4 != 4) && (param_2 == '\0')) {
    (*_DAT_ram_00014800)(0,0,0xf0063c54,0x1321,_DAT_ram_00014800);
  }
  iVar1 = FUN_ram_f0043ed0(param_1,param_2,param_3);
  if ((param_1 == 0x24ba58) && (iVar1 == 0xe)) {
    _DAT_ram_f0065130 = 0x25e720;
  }
  else {
    _DAT_ram_f0065130 = param_1 + iVar1 * 5000;
  }
  _DAT_ram_f0065134 = param_3;
  if (4000000 < _DAT_ram_f0065130) {
    FUN_ram_f0049c24(0x20);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0045348 @ 0xf0045348 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0045348(void)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)&DAT_ram_f006f090;
  do {
    *puVar1 = _DAT_ram_60204e08;
    puVar1 = puVar1 + 3;
  } while (puVar1 != (undefined4 *)0xf006f0f0);
  FUN_ram_f0049c24(4);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0045380 @ 0xf0045380 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0045380(uint param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  undefined1 auStack_a0 [68];
  undefined1 auStack_5c [64];
  
  uVar7 = param_1 & 0x40;
  iVar1 = (*_DAT_ram_00014830)(param_1,_DAT_ram_00014830);
  (*_DAT_ram_00014d28)(1,1,0,0,_DAT_ram_00014d28);
  iVar2 = FUN_ram_f0020000();
  iVar4 = iVar2;
  do {
    while( true ) {
      uVar6 = (uint)DAT_ram_f0064fce;
      if (7 < uVar6) goto LAB_ram_f0045686;
      iVar4 = uVar6 * 0x124;
      iVar8 = iVar4 + -0xff90f10;
      if ((&DAT_ram_f006f210)[iVar4] != '\0') break;
      (*_DAT_ram_00014814)(2,0xf0063cd0,uVar6,_DAT_ram_00014814);
      iVar4 = DAT_ram_f0064fce + 1;
      DAT_ram_f0064fce = (byte)iVar4;
    }
    iVar3 = (*_DAT_ram_00014830)(iVar4,_DAT_ram_00014830);
    FUN_ram_f004518c(*(undefined4 *)((uint)DAT_ram_f0064fce * 4 + -0xff9c1a4),1,0);
    FUN_ram_f0043fac(auStack_5c,0);
    FUN_ram_f0044008(auStack_a0,0);
    iVar3 = (*_DAT_ram_00014814)
                      (2,0xf0063ce4,DAT_ram_f0064fce,iVar3 * 1000 + 0x7fffU >> 0xf,uVar7 >> 6,
                       _DAT_ram_00014814);
    if ((DAT_ram_f0064fcf == 0) || (iVar2 == 0)) {
      if (uVar7 == 0) {
        FUN_ram_f0044204(iVar8);
        FUN_ram_f0043fac(auStack_5c,1);
        uVar5 = FUN_ram_f0044008(auStack_a0,1);
        (*_DAT_ram_00014830)(uVar5,_DAT_ram_00014830);
        FUN_ram_f0044344(iVar8);
        FUN_ram_f0043fac(auStack_5c,1);
        uVar5 = FUN_ram_f0044008(auStack_a0,1);
        (*_DAT_ram_00014830)(uVar5,_DAT_ram_00014830);
      }
      FUN_ram_f0044404(iVar8,DAT_ram_f0064fcf);
      FUN_ram_f0043fac(auStack_5c,1);
      uVar5 = FUN_ram_f0044008(auStack_a0,1);
      (*_DAT_ram_00014830)(uVar5,_DAT_ram_00014830);
      iVar3 = iVar2;
      if (iVar2 == 0) {
        DAT_ram_f0064fcf = 1;
      }
    }
    if ((DAT_ram_f0064fcf != 1) && (iVar3 = iVar2, iVar2 != 0)) {
LAB_ram_f00455fe:
      uVar6 = (uint)DAT_ram_f0064fcf;
      if (uVar6 == 0) {
        DAT_ram_f0064fcf = 1;
      }
      else if (uVar6 == 1) {
        uVar6 = 0xf0064000;
        DAT_ram_f0064fce = DAT_ram_f0064fce + 1;
        DAT_ram_f0064fcf = 0;
      }
      (*_DAT_ram_00014814)(2,0xf0063d04,uVar6,_DAT_ram_00014814);
      if (uVar7 == 0) {
        uVar5 = 0xf006fa44;
      }
      else {
        uVar5 = 0xf006fd88;
      }
      iVar4 = FUN_ram_f002672c(uVar5,100,FUN_ram_f002672c);
      goto LAB_ram_f0045686;
    }
    (*_DAT_ram_00014830)(iVar3,_DAT_ram_00014830);
    FUN_ram_f0044404(iVar8,DAT_ram_f0064fcf);
    FUN_ram_f0043fac(auStack_5c,1);
    FUN_ram_f0044008(auStack_a0,1);
    FUN_ram_f0044898(iVar8);
    FUN_ram_f0043fac(auStack_5c,1);
    uVar5 = FUN_ram_f0044008(auStack_a0,1);
    (*_DAT_ram_00014830)(uVar5,_DAT_ram_00014830);
    FUN_ram_f0044768(iVar8);
    FUN_ram_f0043fac(auStack_5c,1);
    uVar5 = FUN_ram_f0044008(auStack_a0,1);
    (*_DAT_ram_00014830)(uVar5,_DAT_ram_00014830);
    (&DAT_ram_f006f211)[iVar4] = 1;
    if (iVar2 != 0) goto LAB_ram_f00455fe;
    DAT_ram_f0064fce = DAT_ram_f0064fce + 1;
    DAT_ram_f0064fcf = 0;
    iVar8 = (*_DAT_ram_00014828)(2,_DAT_ram_00014828);
    iVar4 = 0;
  } while (iVar8 == 0);
  iVar4 = FUN_ram_f002672c(0xf006fa44,1);
LAB_ram_f0045686:
  iVar4 = (*_DAT_ram_00014830)(iVar4,_DAT_ram_00014830);
  (*_DAT_ram_00014814)(2,0xf0063d14,(iVar4 - iVar1) * 1000 + 0x7fffU >> 0xf,_DAT_ram_00014814);
  if ((7 < DAT_ram_f0064fce) && (iVar2 == 0)) {
    DAT_ram_f0064fce = 0;
    if (uVar7 == 0) {
      uVar5 = 0xf006fa44;
    }
    else {
      uVar5 = 0xf006fd88;
    }
    FUN_ram_f00266d8(uVar5,FUN_ram_f00266d8);
    (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x10000);
  }
  (*_DAT_ram_00014d28)(0,0,0,0,_DAT_ram_00014d28);
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_phy.c:0xc21,0xc55 <<<
// ===== FUN_ram_f004571c @ 0xf004571c =====


/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f004571c(int param_1,byte param_2,int param_3,int param_4)

{
  bool bVar1;
  int iVar2;
  uint uVar3;
  uint *puVar4;
  uint uVar5;
  uint uVar6;
  code *pcVar7;
  int iVar8;
  uint uVar9;
  uint local_28;
  ushort local_24 [4];
  
  if ((_DAT_ram_f00650a4 != 4) && (param_2 == 0)) {
    (*_DAT_ram_00014800)(0,0,0xf0063c54,0xc21,_DAT_ram_00014800);
  }
  if (param_1 == _DAT_ram_f006513c) {
    if ((DAT_ram_f0065039 == param_2) && (param_3 == _DAT_ram_f0065140)) {
      return 1;
    }
    bVar1 = param_3 != _DAT_ram_f0065140;
  }
  else {
    bVar1 = true;
  }
  iVar2 = FUN_ram_f0043ed0(param_1,param_2,param_3);
  uVar5 = _DAT_ram_60120080;
  if ((param_1 == 0x24ba58) && (iVar2 == 0xe)) {
    uVar9 = 0x25e720;
  }
  else {
    uVar9 = param_1 + iVar2 * 5000;
  }
  _DAT_ram_60120080 = _DAT_ram_60120080 | 0x300;
  iVar2 = FUN_ram_f004518c(uVar9,param_3 == 0,param_4);
  if (iVar2 == 0) {
    _DAT_ram_60120080 = uVar5 & 0x300 | _DAT_ram_60120080 & 0xfffffcff;
    return 0;
  }
  DAT_ram_f0065039 = param_2;
  _DAT_ram_f0065138 = uVar9;
  _DAT_ram_f0065144 = param_4;
  if (_DAT_ram_f0064f8c < 100) {
    iVar2 = _DAT_ram_f0064f8c * 10;
    iVar8 = _DAT_ram_f0064f8c * 5;
    *(undefined1 *)(iVar2 + -0xff91358) = 0x81;
    *(undefined1 *)(iVar2 + -0xff91357) = 2;
    local_24[0] = (ushort)param_2 | (ushort)(param_4 << 8);
    local_24[1] = 1;
    local_28 = uVar9;
    (*(code *)&SUB_ram_0006a08c)((iVar8 + 1) * 2 + -0xff91358,local_24 + 1,2);
    (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91354,&local_28,4,&SUB_ram_0006a08c);
    (*(code *)&SUB_ram_0006a08c)(_DAT_ram_f0064f8c * 10 + -0xff91350,local_24,2,&SUB_ram_0006a08c);
    _DAT_ram_f0064f8c = _DAT_ram_f0064f8c + 1;
    if (_DAT_ram_f0064f8c < 100) goto LAB_ram_f00458e0;
  }
  _DAT_ram_f0064f8c = 0;
LAB_ram_f00458e0:
  pcVar7 = _DAT_ram_00014ac4;
  _DAT_ram_f0064f6c = _DAT_ram_f0064f6c + 1;
  if ((param_1 == 0x24ba58) && (0xe < param_2)) {
    (*_DAT_ram_00014ac4)(1);
  }
  else {
    FUN_ram_f003aadc(param_1,param_2,param_4);
    (*pcVar7)();
  }
  if (param_3 == 2) {
    (*_DAT_ram_00014800)(0,0,0xf0063c54,0xc55);
  }
  if ((_DAT_ram_f0065140 != param_3) || (bVar1 != false)) {
    FUN_ram_f0042f80(param_3,param_1);
    iVar2 = param_3;
    if (param_3 == 0) {
      FUN_ram_f0044d9c(0);
      uVar3 = (*_DAT_ram_00014eb0)(0xa0);
      FUN_ram_f0042ee4(0xa0,uVar3 & 0xfffffff | 0x80000000);
      _DAT_ram_60200600 = _DAT_ram_60200600 & 0xfffffffc;
      pcVar7 = _DAT_ram_0001496c;
    }
    else {
      FUN_ram_f0044d9c(param_3);
      uVar3 = (*_DAT_ram_00014eb0)(0xa0);
      FUN_ram_f0042ee4(0xa0,uVar3 & 0xfffffff | 0x90000000);
      if (param_3 == 1) {
        _DAT_ram_60200600 = _DAT_ram_60200600 & 0xfffffffc | 3;
      }
      else {
        _DAT_ram_60200600 = _DAT_ram_60200600 & 0xfffffffc | 1;
      }
      (*_DAT_ram_0001496c)(1);
      pcVar7 = _DAT_ram_0001495c;
      if (param_3 != 1) {
        iVar2 = 3;
      }
    }
    (*pcVar7)(iVar2);
    _DAT_ram_f006513c = param_1;
    _DAT_ram_f0065140 = param_3;
  }
  FUN_ram_f004a200(uVar9);
  if (uVar9 < 0x25e721) {
    if ((bVar1 != false) && (puVar4 = (uint *)(*_DAT_ram_00014e9c)(), puVar4 != (uint *)0x0)) {
      (*_DAT_ram_00014e8c)();
      _DAT_ram_60205c60 = *puVar4;
      _DAT_ram_60205c50 = puVar4[2];
    }
    if (uVar9 == 0x25e720) {
      uVar9 = 0xe;
    }
    else {
      uVar9 = (uVar9 - 0x24ba58) / 5000 & 0xff;
    }
    uVar9 = FUN_ram_f0048954(uVar9);
  }
  else {
    FUN_ram_f0044048(uVar9,param_3,bVar1);
    puVar4 = (uint *)&DAT_ram_f0063d90;
    uVar3 = 0;
    do {
      uVar6 = uVar3;
      uVar3 = uVar6 + 1 & 0xff;
      if (uVar9 < *puVar4) {
        uVar6 = 0;
        break;
      }
      puVar4 = puVar4 + 1;
    } while (uVar3 != 8);
    uVar9 = (uint)(char)(&DAT_ram_f006fa3c)[uVar6];
  }
  _DAT_ram_60205c60 = _DAT_ram_60205c60 & 0x7fffffbd | 0x80000002;
  _DAT_ram_60205c6c = (uVar9 & 0xff) << 8 | _DAT_ram_60205c6c & 0xffff00ff;
  _DAT_ram_6020561c = _DAT_ram_6020561c & 0xffffff00 | 0x2c;
  _DAT_ram_60205600 = _DAT_ram_60205600 & 0xfffeffff;
  iVar2 = (*_DAT_ram_f0065154)(0x60205000,0x60205000,_DAT_ram_f0065154);
  if (iVar2 != 0) {
    _DAT_ram_8005007c = 0x110000;
    _DAT_ram_60204818 = 0;
    _DAT_ram_60204804 = 0x625ace;
    _DAT_ram_602048d8 = 0x6662d6;
    _DAT_ram_602048e4 = 0x51111;
  }
  _DAT_ram_60120080 = uVar5 & 0x300 | _DAT_ram_60120080 & 0xfffffcff;
  FUN_ram_f0021430();
  FUN_ram_f0042ee4(0x44,0x10002d0);
  _DAT_ram_60120080 = _DAT_ram_60120080 | 0x200;
  FUN_ram_f0042ee4(0x3c,0xfeff);
  FUN_ram_f0042ee4(0x3c,0xffff);
  if (_DAT_ram_f0064f98 == 0x8127) {
    uVar5 = (*_DAT_ram_00014eb0)(0xc0,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0xc0,uVar5 & 0xffffffcf);
  }
  else if ((0x8a00 < _DAT_ram_f0065148) || (_DAT_ram_f006514c == 0x6625)) {
    uVar5 = (*_DAT_ram_00014eb0)(0xc0,_DAT_ram_00014eb0);
    FUN_ram_f0042ee4(0xc0,uVar5 | 0x30);
  }
  if (DAT_ram_f006503d != '\x01') {
    _DAT_ram_60120080 = _DAT_ram_60120080 & 0xfffffdff;
    return 1;
  }
  _DAT_ram_60120080 = _DAT_ram_60120080 & 0xfffffdff;
  _DAT_ram_60205c60 = _DAT_ram_60205c60 & 0xffff00ff | 0x3100;
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0045c9c @ 0xf0045c9c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0045c9c(void)

{
  if (((_DAT_ram_f006513c != 0) && (DAT_ram_f0065039 != '\0')) && (_DAT_ram_f0065140 != 2)) {
    _DAT_ram_f006513c = 0;
    FUN_ram_f004571c();
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0045ce4 @ 0xf0045ce4 =====


undefined4
FUN_ram_f0045ce4(uint param_1,int param_2,undefined4 param_3,undefined4 param_4,uint param_5)

{
  undefined4 uVar1;
  byte bVar2;
  uint uVar3;
  int iVar4;
  
  if (param_1 < 0x25e721) {
    if (param_1 != 0x25e720) {
      iVar4 = 0x24ba58;
      goto LAB_ram_f0045d36;
    }
    iVar4 = 0x24ba58;
    uVar3 = 0xe;
  }
  else {
    if (param_1 < 5000000) {
      if (param_1 < 4000000) {
        return 0;
      }
      iVar4 = 4000000;
    }
    else {
      iVar4 = 5000000;
    }
LAB_ram_f0045d36:
    param_5 = (param_1 - iVar4) % 5000;
    uVar3 = (param_1 - iVar4) / 5000 & 0xff;
  }
  if (param_2 == 1) {
    bVar2 = (char)uVar3 - 2;
  }
  else {
    if (param_2 != 3) goto LAB_ram_f0045d54;
    bVar2 = (char)uVar3 + 2;
  }
  uVar3 = (uint)bVar2;
LAB_ram_f0045d54:
  uVar1 = FUN_ram_f004571c(iVar4,uVar3,param_2,0,param_5);
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0045d70 @ 0xf0045d70 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0045d70(void)

{
  FUN_ram_f0046300();
  DAT_ram_f006503b = 0;
  FUN_ram_f0044d48();
  _DAT_ram_f006513c = 0;
  _DAT_ram_f0065140 = 2;
  FUN_ram_f004a410();
  FUN_ram_f0045348();
  FUN_ram_f0049de4();
  FUN_ram_f0046800();
  _DAT_ram_f0065150 = (*_DAT_ram_00014eb0)(0xd8,_DAT_ram_00014eb0);
  if (_DAT_ram_f006514c == 0x6625) {
    FUN_ram_f004931c();
  }
  FUN_ram_f004322c();
  DAT_ram_f006503a = 1;
  FUN_ram_f004571c(0x24ba58,7,0,0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0045e34 @ 0xf0045e34 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0045e34(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  int iStack_94;
  uint *puStack_78;
  uint local_58 [15];
  
  uVar1 = _DAT_ram_60205404;
  local_58[0xb] = 0xb0;
  local_58[8] = 0x80;
  local_58[9] = 0x90;
  local_58[10] = 0xa0;
  if (_DAT_ram_f006514c == 0x6625) {
    local_58[4] = -0x7ff20842;
    local_58[7] = 0x800db7be;
    local_58[5] = -0x7ff20842;
    local_58[6] = -0x7ff008c2;
  }
  else {
    if ((_DAT_ram_f0065148 < 0x8a01) || (_DAT_ram_f006514c != 0x6627)) {
      local_58[4] = -0x7ff00802;
      local_58[7] = 0x800ff73e;
    }
    else {
      local_58[4] = -0x7ff00842;
      local_58[7] = 0x800fb7be;
    }
    local_58[5] = local_58[4] + -0x40;
    local_58[6] = local_58[4];
  }
  local_58[3] = 0x2070000;
  local_58[0] = 0x2040000;
  local_58[1] = 0x2050000;
  local_58[2] = 0x2060000;
  local_58[0xd] = 0x24000001;
  local_58[0xc] = 0x24000000;
  (*_DAT_ram_00014d28)(1,1,0,0,_DAT_ram_60205404,_DAT_ram_00014d28);
  _DAT_ram_60204e00 = 0x6020001;
  _DAT_ram_60204e04 = 0;
  _DAT_ram_60205804 = 0;
  FUN_ram_f0042ee4(200,0x20040c0);
  FUN_ram_f0042ee4(0x6c,0x81bd6c7f);
  FUN_ram_f0042ee4(0x44,0x8c0082d0);
  FUN_ram_f0042ee4(0x48,0xf8b00727);
  if ((_DAT_ram_f0065148 < 0x8a01) && (_DAT_ram_f006514c != 0x6625)) {
    FUN_ram_f0042ee4(0xd4,0x87808d);
  }
  else {
    FUN_ram_f0042ee4(0xd4,0x878089);
  }
  iStack_94 = 0;
  puStack_78 = local_58 + 0xc;
  iVar4 = 0;
  do {
    puVar3 = (undefined4 *)(&DAT_ram_f006f090 + iStack_94 * 4);
    iVar2 = 0;
    do {
      _DAT_ram_60204e00 = 0x6020001;
      _DAT_ram_60204e04 = 0;
      _DAT_ram_60205804 = 0;
      FUN_ram_f0042ee4(200,0x20040c0);
      FUN_ram_f0042ee4(0x70,*(undefined4 *)((int)local_58 + iVar2 + 0x20));
      FUN_ram_f0042ee4(0xa0,iVar4 << 0x1c | *(uint *)((int)local_58 + iVar2 + 0x10));
      if (_DAT_ram_f006514c == 0x6625) {
        FUN_ram_f0042ee4(0xa8,*(undefined4 *)((int)local_58 + iVar2));
      }
      _DAT_ram_60205c3c = 0x5000027;
      if (DAT_ram_f006503d == '\x01') {
        _DAT_ram_6020080c = 0x300594;
        (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
      }
      FUN_ram_f00494b0(iVar4);
      _DAT_ram_60204600 = 0x2a0140;
      _DAT_ram_60204604 = 0x1000400;
      _DAT_ram_60205804 = 0x4980;
      _DAT_ram_60205c00 = 0x1010;
      _DAT_ram_60205c08 = 0x4f400;
      _DAT_ram_60205c0c = 0x2965;
      _DAT_ram_60205404 = 0x10017;
      _DAT_ram_60205400 = 0x100;
      _DAT_ram_60204010 = 0x40;
      _DAT_ram_60200600 = *puStack_78;
      _DAT_ram_60204e00 = 0x9100005;
      (*_DAT_ram_0001480c)(2,_DAT_ram_0001480c);
      (*_DAT_ram_00014ebc)(0x60204e34,0x7c000000,5,_DAT_ram_00014ebc);
      *puVar3 = _DAT_ram_60204e08;
      puVar3[1] = _DAT_ram_60204e0c;
      puVar3[2] = _DAT_ram_60204e10;
      iVar2 = iVar2 + 4;
      puVar3 = puVar3 + 3;
    } while (iVar2 != 0x10);
    iStack_94 = iStack_94 + 0xc;
    puStack_78 = puStack_78 + 1;
    iVar4 = iVar4 + 1;
  } while (iVar4 != 2);
  _DAT_ram_60200600 = 0x3000440;
  _DAT_ram_60204600 = 0x2a0100;
  _DAT_ram_60204604 = 0x100;
  _DAT_ram_60205804 = 0;
  _DAT_ram_60205c00 = 0x1080;
  _DAT_ram_60205c08 = 0x3330000;
  _DAT_ram_60205c0c = 0x2880;
  _DAT_ram_60205400 = 0;
  _DAT_ram_60204010 = 0;
  _DAT_ram_60204e04 = 0;
  _DAT_ram_60204e2c = 0xa4924924;
  _DAT_ram_60204e00 = 0x2100001;
  _DAT_ram_60205c3c = 0x5000000;
  _DAT_ram_60205404 = uVar1;
  FUN_ram_f0042ee4(0x6c,0);
  FUN_ram_f0042ee4(0x44,0x10082d0);
  FUN_ram_f0042ee4(0x48,0x300727);
  FUN_ram_f0042ee4(0x70,0);
  FUN_ram_f0042ee4(0xa0,0x6f5fe);
  FUN_ram_f0042ee4(200,0x20040c0);
  FUN_ram_f0042ee4(0xd4,0);
  FUN_ram_f0042ee4(0xa8,0x40000);
  (*_DAT_ram_00014d28)(0,0,0,0,_DAT_ram_00014d28);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00462c4 @ 0xf00462c4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f00462c4(void)

{
  int iVar1;
  char local_c [8];
  
  iVar1 = (*_DAT_ram_0209022c)(local_c,_DAT_ram_0209022c);
  if (iVar1 == 0) {
    local_c[0] = '\x19';
  }
  if (_DAT_ram_602001fc != 0) {
    local_c[0] = (char)_DAT_ram_f0064f90;
  }
  return (int)local_c[0];
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046300 @ 0xf0046300 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046300(void)

{
  if ((_DAT_ram_f00650ac == 0) ||
     (DAT_ram_f006503e = *(byte *)(_DAT_ram_f00650ac + 0xc), DAT_ram_f006503e == 0)) {
    DAT_ram_f006503e = 0x33;
  }
  if ((byte)(DAT_ram_f006503e - 0x2f) < 0x10) {
    DAT_ram_f006503f = 0x1c;
  }
  else if (DAT_ram_f006503e < 0x3f) {
    DAT_ram_f006503f = 4;
  }
  else {
    DAT_ram_f006503f = 0x34;
  }
  DAT_ram_f006fa10 = FUN_ram_f00462c4();
  DAT_ram_f0065040 = 1;
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_sensor.c:0x5ec <<<
// ===== FUN_ram_f004655c @ 0xf004655c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_ram_f004655c(void)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  uVar1 = _DAT_ram_81080000;
  _DAT_ram_81080000 = _DAT_ram_81080000 | 0xc0000;
  uVar2 = 0;
  do {
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
    if (-1 < (int)(_DAT_ram_81080000 << 0xf)) {
      uVar3 = _DAT_ram_81080000 & 0x7f;
      if (uVar2 < 1000) {
        _DAT_ram_81080000 = uVar1;
        return uVar3;
      }
      goto LAB_ram_f00465a2;
    }
    uVar2 = uVar2 + 1;
  } while (uVar2 != 1000);
  uVar3 = 0;
LAB_ram_f00465a2:
  (*_DAT_ram_00014800)(0,0,0xf00643ac,0x5ec,_DAT_ram_00014800);
  _DAT_ram_81080000 = uVar1;
  return uVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00465cc @ 0xf00465cc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00465cc(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (param_1 < 0x78) {
    if (param_1 < 0x69) {
      DAT_ram_f0065046 = 0x3f;
    }
    else {
      DAT_ram_f0065046 = 0x28;
    }
  }
  else {
    DAT_ram_f0065046 = 0x22;
  }
  if ((_DAT_ram_f006513c == 0x24ba58) &&
     (iVar1 = FUN_ram_f0043ed0(0x24ba58,DAT_ram_f0065039,_DAT_ram_f0065140), iVar1 == 0xe)) {
    iVar1 = 0x25e720;
  }
  else {
    iVar1 = _DAT_ram_f006513c;
    iVar2 = FUN_ram_f0043ed0(_DAT_ram_f006513c,DAT_ram_f0065039,_DAT_ram_f0065140);
    iVar1 = iVar1 + iVar2 * 5000;
  }
  FUN_ram_f004a200(iVar1);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046744 @ 0xf0046744 =====


void FUN_ram_f0046744(int param_1)

{
  if (param_1 < 0x56) {
    DAT_ram_f0064e1c = (char)param_1 + '(';
    DAT_ram_f0064e1a = 1;
    if (param_1 < 0) {
      DAT_ram_f0064e19 = 0;
      DAT_ram_f0064e1a = 1;
      return;
    }
  }
  else {
    DAT_ram_f0064e1a = 0;
  }
  DAT_ram_f0064e1b = (char)param_1 + -0x28;
  DAT_ram_f0064e19 = 1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046800 @ 0xf0046800 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046800(void)

{
  char *pcVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  
  FUN_ram_f0046744((int)DAT_ram_f006fa10);
  pcVar1 = &DAT_ram_f0064e18;
  _DAT_ram_600001fc =
       (int)DAT_ram_f0064e1c << 0x18 | _DAT_ram_600001fc & 0xffff | (uint)DAT_ram_f0064e1b << 0x10;
  uVar4 = 0x7d;
  uVar3 = 0xffffffd8;
  do {
    if (*pcVar1 < '\x01') {
      if (*pcVar1 != '\0') {
        pcVar1[2] = '\0';
        pcVar1[1] = '\x01';
      }
    }
    else {
      pcVar1[2] = '\x01';
      pcVar1[1] = '\0';
    }
    if (pcVar1[2] != '\0') {
      iVar2 = (int)pcVar1[4];
      uVar4 = iVar2 * (uint)(iVar2 < (int)uVar4) | uVar4 * ((int)uVar4 <= iVar2);
    }
    if (pcVar1[1] != '\0') {
      iVar2 = (int)pcVar1[3];
      uVar3 = iVar2 * (uint)((int)uVar3 <= iVar2) | uVar3 * (iVar2 < (int)uVar3);
    }
    pcVar1 = pcVar1 + 0xc;
  } while (pcVar1 != &DAT_ram_f0064e48);
  DAT_ram_f006fa12 = (undefined1)uVar4;
  DAT_ram_f006fa11 = (undefined1)uVar3;
  _DAT_ram_80105000 = 0;
  _DAT_ram_80105004 = 0;
  _DAT_ram_80105008 = 0;
  _DAT_ram_8010500c = 0;
  _DAT_ram_f006fa18 = (*_DAT_ram_00014830)(_DAT_ram_00014830);
  DAT_ram_f006fa14 = 1;
  DAT_ram_f006fa16 = 0;
  DAT_ram_f006fa15 = 0;
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_test.c:0x666 <<<
// ===== FUN_ram_f00468c8 @ 0xf00468c8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00468c8(void)

{
  undefined4 uVar1;
  byte bVar2;
  uint uVar3;
  
  uVar3 = *(uint *)(_DAT_ram_f0065160 + 0xfc);
  if (3 < uVar3) {
    (*_DAT_ram_00014800)(0,0,0xf00643cc,0x666,_DAT_ram_00014800);
  }
  bVar2 = *(byte *)(_DAT_ram_f0065160 + 0xfa);
  if (bVar2 == 0x20) {
    bVar2 = 8;
    uVar3 = 3;
  }
  else {
    if (uVar3 < 2) {
      if (((bVar2 < 4) || ((bVar2 = bVar2 + 2, bVar2 < 0xe && (bVar2 != 4)))) && (bVar2 != 5))
      goto LAB_ram_f0046960;
      uVar1 = 0x675;
    }
    else {
      if (bVar2 < 9) goto LAB_ram_f0046960;
      uVar1 = 0x67b;
    }
    (*_DAT_ram_00014800)(0,0,0xf00643cc,uVar1,_DAT_ram_00014800);
  }
LAB_ram_f0046960:
  (*_DAT_ram_00014ea8)(uVar3,bVar2,_DAT_ram_00014ea8);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046978 @ 0xf0046978 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046978(void)

{
  if ((_DAT_ram_f0065160 != (int *)0x0) && (*_DAT_ram_f0065160 - 8U < 2)) {
    *_DAT_ram_f0065160 = 0;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046990 @ 0xf0046990 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046990(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = _DAT_ram_f0065160;
  puVar2 = (undefined4 *)&DAT_ram_60205700;
  do {
    *(undefined4 *)((int)puVar2 + iVar1 + -0x602056c4) = *puVar2;
    puVar2 = puVar2 + 1;
  } while (puVar2 != (undefined4 *)0x60205770);
  *(uint *)(iVar1 + 0xac) = _DAT_ram_60205604 & 0xff;
  *(uint *)(iVar1 + 0xb0) = _DAT_ram_60205600 & 3;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00469d8 @ 0xf00469d8 =====


void FUN_ram_f00469d8(void)

{
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00469dc @ 0xf00469dc =====


void FUN_ram_f00469dc(void)

{
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00469e0 @ 0xf00469e0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00469e0(void)

{
  byte bVar1;
  int iVar2;
  undefined1 uVar3;
  int *piVar4;
  
  iVar2 = _DAT_ram_f0065160;
  piVar4 = (int *)(_DAT_ram_f0065160 + 0x138);
  if (*piVar4 == 0) {
    bVar1 = *(byte *)(_DAT_ram_f0065160 + 0x13e);
    if ((bVar1 & 1) != 0) {
      (*_DAT_ram_00014af0)(4,_DAT_ram_00014af0);
      (*_DAT_ram_00014dac)(0,0,0,_DAT_ram_00014dac);
      *(undefined1 *)(iVar2 + 0x13c) = 0;
      goto LAB_ram_f0046a7a;
    }
    _DAT_ram_60140000 = 0xf9a;
    _DAT_ram_60140038 = 0x1c320100;
    (*_DAT_ram_00014af0)(0x62,_DAT_ram_00014af0);
    (*_DAT_ram_00014dac)(bVar1 & 1,1,iVar2 + 0x13e,_DAT_ram_00014dac);
LAB_ram_f0046a76:
    uVar3 = 1;
  }
  else {
    (*_DAT_ram_00014af0)(4,_DAT_ram_00014af0);
    (*_DAT_ram_00014dac)(0,0,0,_DAT_ram_00014dac);
    if (*piVar4 != 2) goto LAB_ram_f0046a76;
    uVar3 = 0;
  }
  *(undefined1 *)(iVar2 + 0x13c) = uVar3;
LAB_ram_f0046a7a:
  if (*piVar4 == 0) {
    if ((*(byte *)(iVar2 + 0x144) & 1) == 0) {
      uVar3 = 1;
    }
    else {
      uVar3 = 0;
    }
  }
  else if (*piVar4 == 1) {
    uVar3 = 0;
  }
  else {
    uVar3 = 1;
  }
  *(undefined1 *)(iVar2 + 0x13d) = uVar3;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046aa0 @ 0xf0046aa0 =====


uint FUN_ram_f0046aa0(int param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  uVar2 = (param_1 - 0xbU) / 9;
  uVar3 = 3;
  uVar1 = 0;
  do {
    uVar1 = uVar1 | (uVar2 & 1) << (uVar3 & 0x1f);
    uVar3 = uVar3 - 1;
    uVar2 = uVar2 >> 1;
  } while (uVar3 != 0xffffffff);
  uVar3 = 7;
  do {
    uVar1 = uVar1 | (uVar2 & 1) << (uVar3 & 0x1f);
    uVar3 = uVar3 - 1;
    uVar2 = uVar2 >> 1;
  } while (uVar3 != 3);
  return uVar1 & 0xffff;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046ad8 @ 0xf0046ad8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046ad8(void)

{
  uint uVar1;
  
  _DAT_ram_60205600 = _DAT_ram_60205600 & 0xfffdffff;
  uVar1 = (*_DAT_ram_00014eb0)(0xd4,_DAT_ram_00014eb0);
  FUN_ram_f0042ee4(0xd4,uVar1 & 0x7fffffff);
  FUN_ram_f0042ee4(0xd8,_DAT_ram_f0065150);
  uVar1 = (*_DAT_ram_00014eb0)(0x70,_DAT_ram_00014eb0);
  FUN_ram_f0042ee4(0x70,uVar1 & 0xfefff0ff);
  _DAT_ram_60204010 = _DAT_ram_60204010 & 0xdfffffff;
  _DAT_ram_60205400 = _DAT_ram_60205400 & 0xffffffcc;
  _DAT_ram_60205804 = 0;
  _DAT_ram_60205404 = _DAT_ram_60205404 & 0xfffffff8;
  _DAT_ram_60200824 = _DAT_ram_60200824 & 0xf00f00ff | 0xe400;
  _DAT_ram_60205600 = _DAT_ram_60205600 & 0xfff7ffff;
  _DAT_ram_6020080c = _DAT_ram_6020080c & 0xffffffc0;
  _DAT_ram_60205c3c = _DAT_ram_60205c3c & 0xffffffc0;
  _DAT_ram_60205c94 = _DAT_ram_f0065168 & 0xffff0000 | _DAT_ram_60205c94 & 0xffff;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046c10 @ 0xf0046c10 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046c10(void)

{
  int iVar1;
  
  if (_DAT_ram_f0065160 != (int *)0x0) {
    iVar1 = *_DAT_ram_f0065160;
    if ((((iVar1 == 10) || (iVar1 == 4)) || (iVar1 == 5)) || (iVar1 == 6)) {
      FUN_ram_f0046ad8();
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046c44 @ 0xf0046c44 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046c44(void)

{
  DAT_ram_f0065042 = *(undefined1 *)(_DAT_ram_f0065160 + 0x125);
  FUN_ram_f0049c24(2);
  FUN_ram_f0049c24(0x20);
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_test.c:0x9ca <<<
// ===== FUN_ram_f0046c78 @ 0xf0046c78 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046c78(undefined4 *param_1,byte param_2)

{
  int iVar1;
  byte bVar2;
  
  iVar1 = _DAT_ram_f0065160;
  if (_DAT_ram_f0065160 != 0) {
    for (bVar2 = 0; bVar2 < param_2; bVar2 = bVar2 + 1) {
      if (param_1 == (undefined4 *)0x0) {
        (*_DAT_ram_00014800)(0,0,0xf00643cc,0x9ca,_DAT_ram_00014800);
      }
      if (*(uint *)(iVar1 + 0x164) < 1000) {
        (*(code *)&SUB_ram_0006a08c)
                  (*(int *)(_DAT_ram_f0065160 + 0x1c) + *(int *)(iVar1 + 0x168),param_1 + 4,0x20);
        *(int *)(iVar1 + 0x164) = *(int *)(iVar1 + 0x164) + 1;
        *(int *)(iVar1 + 0x168) = *(int *)(iVar1 + 0x168) + 0x20;
      }
      *(int *)(iVar1 + 0x160) = *(int *)(iVar1 + 0x160) + 1;
      param_1 = (undefined4 *)*param_1;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0046d0c @ 0xf0046d0c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0046d0c(void)

{
  bool bVar1;
  int iVar2;
  char cVar3;
  ushort uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  undefined4 *puVar8;
  int iVar9;
  
  iVar2 = _DAT_ram_f0065160;
  puVar8 = (undefined4 *)(_DAT_ram_f0065160 + 0xf4);
  _DAT_ram_60205400 = 0;
  _DAT_ram_60204010 = 0;
  _DAT_ram_60205804 = 0;
  _DAT_ram_6020080c = 0x300597;
  uVar5 = FUN_ram_f00468c8();
  uVar6 = uVar5 >> 6 & 0xff;
  if (((uVar5 & 0xc0) == 0xc0) || ((uVar5 & 0xc0) == 0x80)) {
    uVar7 = uVar5 & 0x3f;
  }
  else {
    uVar7 = uVar5 & 0xf;
  }
  _DAT_ram_60202408 = uVar7 | _DAT_ram_60202408 & 0xffffff80;
  _DAT_ram_60202404 = 0xffff0fff;
  uVar7 = *(uint *)(iVar2 + 0x130);
  _DAT_ram_60205404 =
       *(int *)(iVar2 + 0x110) << 8 | _DAT_ram_60205404 & 0xfffffcc8 | uVar7 & 0xff | uVar6 << 4;
  if ((uVar7 - 4 < 2) || ((uVar7 == 3 && (uVar6 == 0)))) {
    bVar1 = false;
    iVar9 = 0xd;
  }
  else if ((*(byte *)(iVar2 + 0xfa) == 0x20) ||
          ((_DAT_ram_60200838 >> (*(byte *)(iVar2 + 0xfa) + 0x11 & 0x1f) & 1) == 0)) {
    bVar1 = true;
    iVar9 = 0xe;
  }
  else {
    bVar1 = true;
    iVar9 = 0xf;
  }
  uVar6 = (*_DAT_ram_00014eb0)(0x70,_DAT_ram_00014eb0);
  FUN_ram_f0042ee4(0x70,uVar6 & 0xfffff0ff | iVar9 << 8 | 0x1000000);
  if (*(char *)(iVar2 + 0x129) == '\0') {
    uVar6 = _DAT_ram_60200824 & 0xffff00ff;
    if (!bVar1) {
      uVar6 = uVar6 | 0xe400;
    }
    uVar7 = (uint)*(char *)(iVar2 + 0xf9);
    if ((char)(*(char *)(iVar2 + 0xf9) + '@') < '\0') {
      _DAT_ram_60200824 = uVar6;
      uVar7 = FUN_ram_f0048840(*puVar8,uVar5,*(int *)(iVar2 + 0x110) == 1);
    }
    if (DAT_ram_f0065053 == '\0') {
      cVar3 = (char)uVar7;
      if (_DAT_ram_f006513c == 0x24ba58) {
        if (DAT_ram_f006503c == '\0') {
          cVar3 = cVar3 + '\x06';
        }
        else {
          cVar3 = cVar3 + '\x03';
        }
      }
      else {
        cVar3 = cVar3 + '\x02';
      }
      uVar7 = (uint)cVar3;
    }
    _DAT_ram_60200824 = uVar6 & 0xf80fffff | 0x8000000 | (uVar7 & 0x7f) << 0x14 | 0x80000000;
    if (*(char *)(iVar2 + 0xfa) == ' ') {
      _DAT_ram_60205600 = _DAT_ram_60205600 | 0xa0000;
      uVar5 = (*_DAT_ram_00014eb0)(0xd4,_DAT_ram_00014eb0);
      FUN_ram_f0042ee4(0xd4,uVar5 | 0x80000000);
      uVar4 = (*_DAT_ram_00014eb0)(0xd8,_DAT_ram_00014eb0);
      uVar5 = uVar4 | 0x391c0000;
    }
    else {
      _DAT_ram_60205600 = _DAT_ram_60205600 & 0xfffdffff | 0x80000;
      uVar5 = (*_DAT_ram_00014eb0)(0xd4,_DAT_ram_00014eb0);
      FUN_ram_f0042ee4(0xd4,uVar5 & 0x7fffffff);
      uVar4 = (*_DAT_ram_00014eb0)(0xd8,_DAT_ram_00014eb0);
      uVar5 = (uint)uVar4;
    }
    FUN_ram_f0042ee4(0xd8,uVar5);
  }
  if (bVar1) {
    if (*(int *)(iVar2 + 0x110) == 1) {
      _DAT_ram_60205804 = 0x91;
    }
    else {
      _DAT_ram_60205804 = 0x90;
    }
    _DAT_ram_60204010 = _DAT_ram_60204010 | 0x20000000;
    _DAT_ram_60205400 = _DAT_ram_60205400 & 0xfffffffc | 1;
  }
  else {
    _DAT_ram_60205400 = _DAT_ram_60205400 & 0xffffffcf | 0x10;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0047000 @ 0xf0047000 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0047000(void)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  undefined1 auStack_30 [28];
  
  piVar1 = _DAT_ram_f0065160;
  if ((_DAT_ram_f0065160 != (int *)0x0) && (_DAT_ram_f00650a4 == 4)) {
    uVar5 = _DAT_ram_60205600 & 0xffe7ffff;
    if (*(char *)((int)_DAT_ram_f0065160 + 0x129) == '\0') {
      if (-1 < (char)(*(char *)((int)_DAT_ram_f0065160 + 0xf9) + '@')) {
        (*(code *)&SUB_ram_0006a0a0)(auStack_30,(int)*(char *)((int)_DAT_ram_f0065160 + 0xf9),0x18);
        FUN_ram_f00484ec(auStack_30,0,0,0x3f,0x3f,0x3f);
        uVar2 = FUN_ram_f00468c8();
        uVar4 = piVar1[0x3d];
        if (((uVar4 < 0x25e721) && (iVar3 = FUN_ram_f0048a48(uVar4), iVar3 != 0)) &&
           (((uVar2 & 0xc0) == 0 || (*(char *)((int)piVar1 + 0xf9) < *(char *)(iVar3 + 0x72))))) {
          if ((_DAT_ram_60200824 & 0x40) == 0) {
            uVar2 = _DAT_ram_60200824 & 0x7f;
          }
          else {
            uVar2 = (uint)(char)((byte)_DAT_ram_60200824 | 0x80);
          }
          _DAT_ram_60200824 =
               uVar2 + *(byte *)(iVar3 + 0x71) & 0xff | _DAT_ram_60200824 & 0xffffff00;
        }
      }
    }
    else {
      _DAT_ram_60205608 = (uint)*(byte *)(_DAT_ram_f0065160 + 0x4a);
      uVar5 = uVar5 | 0x180000;
    }
    iVar3 = *_DAT_ram_f0065160;
    _DAT_ram_60205600 = uVar5;
    if (((iVar3 == 10) || (iVar3 == 4)) || (iVar3 == 6)) {
      FUN_ram_f0046d0c();
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0047138 @ 0xf0047138 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0047138(int param_1)

{
  undefined1 uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  byte *pbVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  ushort *puVar9;
  
  iVar2 = _DAT_ram_f0065160;
  if (_DAT_ram_f0065160 == 0) {
    uVar3 = 0xc0000001;
  }
  else {
    iVar8 = *(int *)(param_1 + 8);
    if (*(short *)(_DAT_ram_f0065160 + 0x13c) != 0) {
      puVar9 = *(ushort **)(param_1 + 0x14);
      if ((((((*(ushort *)(iVar8 + 0xe) & 1) != 0) || (*(ushort *)(iVar8 + 8) < 0x18)) ||
           ((*puVar9 & 0xc) == 4)) ||
          ((*(char *)(_DAT_ram_f0065160 + 0x13c) != '\0' &&
           (iVar4 = (*(code *)&SUB_ram_0006a074)(puVar9 + 2,_DAT_ram_f0065160 + 0x13e,6), iVar4 != 0
           )))) || ((*(char *)(iVar2 + 0x13d) != '\0' &&
                    (iVar4 = (*(code *)&SUB_ram_0006a074)(puVar9 + 5,iVar2 + 0x144,6), iVar4 != 0)))
         ) {
        return 0;
      }
    }
    *(int *)(iVar2 + 0x158) = *(int *)(iVar2 + 0x158) + 1;
    if (_DAT_ram_f006513c == 0x24ba58) {
      pbVar5 = &DAT_ram_f006504d;
    }
    else {
      pbVar5 = &DAT_ram_f006504e;
    }
    if (*(uint *)(iVar2 + 0x184) < 0xffffff01) {
      uVar6 = (uint)*pbVar5 + ((*(uint *)(iVar8 + 0x44) & 0xffff) >> 8);
      uVar7 = uVar6 & 0xff;
      *(int *)(iVar2 + 0x15c) = *(int *)(iVar2 + 0x15c) + 1;
      *(uint *)(iVar2 + 0x184) = *(uint *)(iVar2 + 0x184) + uVar7;
      uVar1 = (undefined1)uVar6;
      *(undefined1 *)(iVar2 + 0x18a) = uVar1;
      if (*(byte *)(iVar2 + 0x188) < uVar7) {
        *(undefined1 *)(iVar2 + 0x188) = uVar1;
      }
      if ((*(byte *)(iVar2 + 0x189) == 0) || (uVar7 < *(byte *)(iVar2 + 0x189))) {
        *(undefined1 *)(iVar2 + 0x189) = uVar1;
      }
    }
    (*(code *)&SUB_ram_0006a0d8)(param_1);
    uVar3 = 1;
  }
  return uVar3;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0047248 @ 0xf0047248 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0047248(char param_1)

{
  int iVar1;
  int iVar2;
  undefined1 *puVar3;
  int iVar4;
  undefined4 uVar5;
  undefined1 auStack_30 [28];
  
  iVar1 = _DAT_ram_f0065160;
  puVar3 = auStack_30;
  FUN_ram_f003bc90();
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e4c0,0,0x50);
  _DAT_ram_60200600 = _DAT_ram_60200600 & 0xffdfffff;
  (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_f0065160 + 0x14c,0,0x40);
  iVar2 = _DAT_ram_f0065160;
  _DAT_ram_f006513c = 0;
  _DAT_ram_f0065140 = 2;
  iVar4 = *(int *)(_DAT_ram_f0065160 + 0x110);
  if (iVar4 == 2) {
LAB_ram_f00472dc:
    uVar5 = 3;
  }
  else {
    if (iVar4 != 3) {
      uVar5 = 0;
      if (iVar4 == 0) goto LAB_ram_f00472e2;
      if (*(uint *)(_DAT_ram_f0065160 + 0xf4) < 0x24f4f0) goto LAB_ram_f00472dc;
    }
    uVar5 = 1;
  }
LAB_ram_f00472e2:
  FUN_ram_f0045ce4(*(uint *)(_DAT_ram_f0065160 + 0xf4),uVar5);
  (*_DAT_ram_00014e38)(1,*(undefined2 *)(iVar2 + 0x10c),_DAT_ram_00014e38);
  if (param_1 != '\0') {
    FUN_ram_f002672c(_DAT_ram_f0065160 + 4,2000);
  }
  if ((char)(*(char *)(iVar1 + 0xf9) + '@') < '\0') {
    puVar3 = &DAT_ram_f0064e48;
  }
  else {
    (*(code *)&SUB_ram_0006a0a0)(auStack_30,(int)*(char *)(iVar1 + 0xf9),0x18);
  }
  FUN_ram_f0049ce4(puVar3);
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_test.c:0x702,0x71b <<<
// ===== FUN_ram_f004735c @ 0xf004735c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004735c(byte *param_1)

{
  byte bVar1;
  uint *puVar2;
  undefined2 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int iVar6;
  uint uVar7;
  uint *puVar8;
  uint uVar9;
  uint uVar10;
  undefined4 uVar11;
  uint uVar12;
  uint uVar13;
  int iVar14;
  undefined2 *puVar15;
  uint *puVar16;
  int local_50;
  uint local_4c;
  uint uStack_48;
  int local_44;
  undefined2 local_30;
  undefined2 local_2e;
  undefined2 local_2a;
  uint local_24 [2];
  
  puVar8 = _DAT_ram_f0065160;
  local_24[0] = *(uint *)(param_1 + 4);
  bVar1 = *param_1;
  puVar16 = _DAT_ram_f0065160 + 0x3d;
  (*_DAT_ram_00014814)(2,0xf00643e8,bVar1,local_24[0],_DAT_ram_00014814);
  if (bVar1 == 0x16) {
    *(undefined1 *)((int)puVar8 + 0x11d) = (undefined1)local_24[0];
    return;
  }
  if (0x16 < bVar1) {
    if (bVar1 == 0x1f) {
      *(undefined1 *)((int)puVar8 + 0x129) = (undefined1)local_24[0];
      return;
    }
    if (bVar1 < 0x20) {
      if (bVar1 == 0x1b) {
        *(undefined1 *)((int)puVar8 + 0x125) = (undefined1)local_24[0];
        return;
      }
      if (0x1b < bVar1) {
        if (bVar1 == 0x1d) {
          *(undefined1 *)((int)puVar8 + 0x127) = (undefined1)local_24[0];
          return;
        }
        if (bVar1 < 0x1e) {
          *(undefined1 *)((int)puVar8 + 0x126) = (undefined1)local_24[0];
          FUN_ram_f0049074((local_24[0] & 0xff) != 0,0,0,FUN_ram_f0049074);
          return;
        }
        *(undefined1 *)(puVar8 + 0x4a) = (undefined1)local_24[0];
        return;
      }
      if (bVar1 != 0x19) {
        if (0x19 < bVar1) {
          *(undefined1 *)(puVar8 + 0x49) = (undefined1)local_24[0];
          return;
        }
        if (bVar1 != 0x17) {
          return;
        }
        *(undefined1 *)((int)puVar8 + 0x11e) = (undefined1)local_24[0];
        return;
      }
      puVar8[0x48] = local_24[0];
      _DAT_ram_6020080c = _DAT_ram_6020080c & 0xfff0ffff;
      if (local_24[0] == 1) {
        _DAT_ram_6020080c = _DAT_ram_6020080c | 0xc0000;
LAB_ram_f0047b68:
        _DAT_ram_6020484c = _DAT_ram_6020484c & 0x7fffffff;
        _DAT_ram_f0065160[0x38] = local_24[0];
        return;
      }
      if (local_24[0] == 2) {
        _DAT_ram_6020080c = _DAT_ram_6020080c | 0x30000;
        _DAT_ram_60204870 = _DAT_ram_60204870 | 0x3000000;
        goto LAB_ram_f0047b68;
      }
      puVar8 = (uint *)&DAT_ram_60204870;
      uVar12 = _DAT_ram_f0065160[0x39];
      uVar9 = _DAT_ram_60204870 & 0xfcffffff;
    }
    else {
      if (0x45 < bVar1) {
        if (bVar1 != 0x46) {
          if (bVar1 != 0x80) {
            return;
          }
          DAT_ram_f0064fcd = (undefined1)local_24[0];
          FUN_ram_f004898c();
          return;
        }
        if (3 < local_24[0]) {
          return;
        }
        puVar8[0x4e] = local_24[0];
LAB_ram_f0047cf2:
        FUN_ram_f00469e0();
        return;
      }
      if (0x43 < bVar1) {
        if (bVar1 == 0x44) {
          puVar8 = (uint *)((int)puVar8 + 0x13e);
        }
        else {
          puVar8 = puVar8 + 0x51;
        }
        if (*(short *)(param_1 + 2) == 0) {
          (*(code *)&SUB_ram_0006a08c)(puVar8,local_24,4);
          return;
        }
        if (*(short *)(param_1 + 2) != 4) {
          return;
        }
        (*(code *)&SUB_ram_0006a08c)(puVar8 + 1,local_24,2);
        goto LAB_ram_f0047cf2;
      }
      if (bVar1 == 0x41) {
        if (7 < local_24[0]) {
          return;
        }
        puVar8[0x4c] = local_24[0];
        return;
      }
      if (bVar1 == 0x42) {
        *(undefined1 *)(puVar8 + 0x4d) = (undefined1)local_24[0];
        DAT_ram_f006503a = (undefined1)local_24[0];
        return;
      }
      if (bVar1 != 0x40) {
        return;
      }
      uVar9 = (uint)*(ushort *)(param_1 + 2);
      if (0x74 < uVar9) {
        return;
      }
      (*(code *)&SUB_ram_0006a08c)((int)_DAT_ram_f0065160 + uVar9 + 0x3c,local_24,4);
      if (uVar9 != 0x74) {
        return;
      }
      puVar8 = (uint *)&DAT_ram_60205700;
      iVar6 = 0;
      do {
        *puVar8 = _DAT_ram_f0065160[iVar6 + 0xf];
        puVar8 = puVar8 + 1;
        iVar6 = iVar6 + 1;
      } while (puVar8 != (uint *)0x60205770);
      _DAT_ram_60205604 = (uint)(byte)_DAT_ram_f0065160[0x2b] | _DAT_ram_60205604 & 0xffffff00;
      puVar8 = (uint *)&DAT_ram_60205600;
      uVar12 = _DAT_ram_f0065160[0x2c] & 3;
      uVar9 = _DAT_ram_60205600 & 0xfffffffc;
    }
    uVar9 = uVar9 | uVar12;
LAB_ram_f0047b90:
    *puVar8 = uVar9;
    return;
  }
  if (bVar1 == 8) {
    if (0x8f7 < local_24[0] - 0x13) {
      return;
    }
    puVar8[0x42] = local_24[0];
    uVar3 = FUN_ram_f0046aa0(local_24[0]);
    *(undefined2 *)(puVar8 + 0x43) = uVar3;
    return;
  }
  if (bVar1 < 9) {
    if (bVar1 == 3) {
      uVar9 = local_24[0] & 0xff;
      if ((int)local_24[0] < 0) {
        if ((7 < uVar9) && (uVar9 != 0x20)) {
          return;
        }
      }
      else if (0xb < uVar9) {
        return;
      }
      *(undefined1 *)((int)puVar8 + 0xfa) = (undefined1)local_24[0];
      return;
    }
    if (3 < bVar1) {
      if (bVar1 == 6) {
        if (0xfe7 < local_24[0] - 0x18) {
          return;
        }
        puVar8[0x40] = local_24[0];
        return;
      }
      if (6 < bVar1) {
        puVar8[0x41] = local_24[0];
        return;
      }
      if (bVar1 != 4) {
        return;
      }
      if (3 < local_24[0]) {
        return;
      }
      puVar8[0x3f] = local_24[0];
      return;
    }
    if (bVar1 != 1) {
      if (bVar1 != 2) {
        return;
      }
      *(undefined1 *)((int)puVar8 + 0xf9) = (undefined1)local_24[0];
      return;
    }
    if (*_DAT_ram_f0065160 != 0) {
      *_DAT_ram_f0065160 = 0;
      FUN_ram_f00266d8(_DAT_ram_f0065160 + 1);
      (*_DAT_ram_00014b38)(0,0,_DAT_ram_00014b38);
      FUN_ram_f003718c(0,0x1ff);
      (*(code *)&SUB_ram_0006affc)(0);
      (*(code *)&SUB_ram_0006affc)(1);
      FUN_ram_f0046ad8();
      _DAT_ram_6013005c = _DAT_ram_6013005c & 0xbfffffff;
    }
    puVar2 = _DAT_ram_f0065160;
    if (local_24[0] != 5) {
      if (5 < local_24[0]) {
        if (local_24[0] == 8) {
LAB_ram_f00479aa:
          *_DAT_ram_f0065160 = local_24[0];
          FUN_ram_f0047248(0,0);
          return;
        }
        if (local_24[0] < 9) {
          if (local_24[0] != 6) {
            if (local_24[0] != 7) {
              return;
            }
            goto LAB_ram_f00479aa;
          }
          *_DAT_ram_f0065160 = 6;
          uVar9 = FUN_ram_f00468c8();
          if ((uVar9 & 0xc0) == 0) {
            uVar9 = 5;
          }
          else {
            uVar9 = 2;
          }
          puVar8[0x4c] = uVar9;
        }
        else {
          if (local_24[0] == 9) {
            *_DAT_ram_f0065160 = 9;
            FUN_ram_f0047248(0,0);
            FUN_ram_f0046c44();
            return;
          }
          if (local_24[0] != 10) {
            return;
          }
          *_DAT_ram_f0065160 = 10;
        }
LAB_ram_f0047990:
        FUN_ram_f0047248(0,1);
        FUN_ram_f0046d0c();
        return;
      }
      if (local_24[0] == 2) {
        *_DAT_ram_f0065160 = 2;
        FUN_ram_f0047248(1,0);
        (*_DAT_ram_00014b38)(1,1,_DAT_ram_00014b38);
        return;
      }
      if (2 < local_24[0]) {
        if (local_24[0] == 3) {
          (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_f0065160 + 0x3d,0,0x58);
          puVar2[0x40] = 0x400;
          puVar2[0x41] = 1000;
          puVar2[0x42] = 0xa0;
          uVar3 = FUN_ram_f0046aa0();
          *(undefined2 *)(puVar2 + 0x43) = uVar3;
          (*(code *)&SUB_ram_0006a0a0)(&local_30,0,10);
          local_2a = 1;
          local_2e = 0xffff;
          local_30 = 0xffff;
          iVar6 = 0;
          do {
            FUN_ram_f004bed0(iVar6,&local_30);
            iVar6 = iVar6 + 1;
          } while (iVar6 != 0xe);
          (*_DAT_ram_00014dd0)(0,9,_DAT_ram_00014dd0);
          (*_DAT_ram_00014e28)(0,_DAT_ram_00014e28);
          (*_DAT_ram_00014e1c)(0,_DAT_ram_00014e1c);
          (*_DAT_ram_00014e24)(0,_DAT_ram_00014e24);
          (*_DAT_ram_00014e20)(0,_DAT_ram_00014e20);
          (*_DAT_ram_00014e18)(0,_DAT_ram_00014e18);
          (*_DAT_ram_00014958)(0,_DAT_ram_00014958);
          *(undefined1 *)(puVar2 + 0x51) = 1;
          *(undefined1 *)((int)puVar2 + 0x13e) = 1;
          FUN_ram_f00469e0();
          puVar2[0x4b] = 0x24ba58;
          *(undefined1 *)((int)puVar2 + 0xf9) = 0x40;
          puVar2[0x3d] = 0x24cde0;
          return;
        }
        if (local_24[0] != 4) {
          return;
        }
        *_DAT_ram_f0065160 = 4;
        puVar8[0x4c] = 3;
        goto LAB_ram_f0047990;
      }
      if (local_24[0] != 1) {
        return;
      }
      *_DAT_ram_f0065160 = 1;
      FUN_ram_f0047248(1);
      if (4000000 < *puVar16) {
        FUN_ram_f0049c24(0x20);
      }
      puVar8 = _DAT_ram_f0065160;
      uVar4 = FUN_ram_f00468c8();
      if (puVar8[0x44] == 1) {
        uVar5 = 3;
      }
      else {
        uVar5 = 2;
      }
      FUN_ram_f0034034(uVar5);
      uVar5 = FUN_ram_f0034044((char)puVar8[0x45] != '\0');
      uVar9 = puVar8[0x41];
      if (uVar9 - 1 < 3) {
        uVar9 = uVar9 & 0xffff;
        uStack_48 = uVar9;
      }
      else {
        uStack_48 = 4;
      }
      uVar9 = (*_DAT_ram_00014830)(uVar5,uVar9,_DAT_ram_00014830);
      uVar9 = uVar9 << 0x10 ^ uVar9;
      local_50 = 0;
      local_4c = 0;
      local_44 = 0;
      do {
        if ((local_44 == uStack_48 - 1) && (0x100 < puVar8[0x40])) {
          iVar6 = FUN_ram_f0025c48();
          uVar12 = puVar8[0x40] - 0x1e & 0xffff;
        }
        else {
          iVar6 = FUN_ram_f0025c48(0x100);
          uVar12 = 0xe2;
        }
        if (iVar6 == 0) {
          if (local_50 == 0) {
            (*_DAT_ram_00014800)(0,0,0xf00643cc,0x71b,_DAT_ram_00014800);
            goto LAB_ram_f004789e;
          }
          break;
        }
        iVar14 = 0x1a;
        uVar13 = 0;
        while( true ) {
          uVar10 = uVar13 & 0xffff;
          uVar7 = uVar9 * 3 + uVar13;
          puVar15 = *(undefined2 **)(iVar6 + 8);
          uVar13 = uVar13 + 1;
          if (uVar12 >> 2 <= uVar10) break;
          *(uint *)((int)puVar15 + iVar14 & 0xfffffffc) = uVar9;
          iVar14 = iVar14 + 4;
          uVar9 = uVar7;
        }
        (*(code *)&SUB_ram_0006a0a0)(puVar15,0,0x1a);
        (*(code *)&SUB_ram_0006a08c)(puVar15 + 2,(int)puVar8 + 0x13e,6);
        (*(code *)&SUB_ram_0006a08c)(puVar15 + 5,puVar8 + 0x51,6);
        uVar12 = puVar8[0x40];
        if (uVar12 < 0x1e) {
          if (uVar12 < 0xe) {
            (*_DAT_ram_00014800)(0,0,0xf00643cc,0x702,_DAT_ram_00014800);
          }
          uVar5 = 10;
          uVar12 = puVar8[0x40] - 0xe;
        }
        else {
          *puVar15 = 0x88;
          puVar15[0xc] = 0x20;
          uVar12 = uVar12 - 0x1e;
          *(undefined1 *)(puVar15 + 8) = 0xab;
          *(undefined1 *)((int)puVar15 + 0x11) = 0xcd;
          uVar5 = 0x1a;
        }
        uVar11 = _DAT_ram_f0065164;
        if ((*(byte *)(puVar15 + 2) & 1) != 0) {
          uVar11 = 0;
        }
        FUN_ram_f0034540(iVar6,uVar11,_DAT_ram_f006389c,*(int *)(iVar6 + 8),uVar5,
                         *(int *)(iVar6 + 8) + 0x1a,uVar12 & 0xffff,&LAB_ram_f0047d1c,1,1,uVar4,0,
                         local_50);
        local_4c = local_4c + 1 & 0xffff;
        local_44 = local_44 + 1;
        local_50 = iVar6;
      } while (local_4c < uStack_48);
      _DAT_ram_f0065160[0x53] = local_4c;
      FUN_ram_f0037b58(local_50);
LAB_ram_f004789e:
      (*_DAT_ram_00014b38)(1,0,_DAT_ram_00014b38);
      puVar8 = (uint *)&DAT_ram_6013005c;
      uVar9 = _DAT_ram_6013005c | 0x40000000;
      goto LAB_ram_f0047b90;
    }
    *_DAT_ram_f0065160 = 5;
    FUN_ram_f0047248(0,1);
    _DAT_ram_60205c3c = _DAT_ram_60205c3c & 0xffffffc0 | 0x27;
    _DAT_ram_f0065168 = _DAT_ram_60205c94;
    _DAT_ram_60205c94 = _DAT_ram_60205c94 & 0xffff | 0x40400000;
    puVar8 = (uint *)&DAT_ram_6020080c;
    local_24[0] = _DAT_ram_6020080c & 0xffffffc0 | 0x17;
  }
  else {
    if (bVar1 != 0x12) {
      if (0x12 < bVar1) {
        if (bVar1 == 0x14) {
          puVar8[0x46] = local_24[0];
          return;
        }
        if (bVar1 < 0x15) {
          *(undefined1 *)((int)puVar8 + 0x115) = (undefined1)local_24[0];
          return;
        }
        *(undefined1 *)(puVar8 + 0x47) = (undefined1)local_24[0];
        return;
      }
      if (bVar1 == 0xf) {
        if (3 < local_24[0]) {
          return;
        }
        puVar8[0x44] = local_24[0];
        return;
      }
      if (bVar1 != 0x10) {
        if (bVar1 != 9) {
          return;
        }
        *(undefined1 *)(puVar8 + 0x3e) = (undefined1)local_24[0];
        return;
      }
      *(undefined1 *)(puVar8 + 0x45) = (undefined1)local_24[0];
      return;
    }
    *puVar16 = local_24[0];
    if (local_24[0] < 0x3d0901) {
      return;
    }
    puVar8 = (uint *)&DAT_ram_f0065130;
  }
  *puVar8 = local_24[0];
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0047db8 @ 0xf0047db8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0047db8(int param_1)

{
  char cVar1;
  undefined1 uVar2;
  int *piVar3;
  int *piVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint *puVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  int iVar13;
  
  piVar4 = _DAT_ram_f0065160;
  if (_DAT_ram_f0065160 == (int *)0x0) {
    return;
  }
  iVar13 = *(int *)(param_1 + 8);
  uVar11 = (uint)*(byte *)(iVar13 + 0x3c);
  if (((_DAT_ram_f0065160[0x47] & 0xffffffU) != 0) && (2 < uVar11 - 0x15)) {
LAB_ram_f00481d4:
    (*(code *)&SUB_ram_0006a0d8)(param_1);
    return;
  }
  cVar1 = *(char *)(iVar13 + 0x38);
  if (cVar1 != '\x02') {
    if (cVar1 == '\x05') {
      if (*(int *)(iVar13 + 0x3c) - 0x24cde0U < 0x11941) {
        _DAT_ram_f0065160[0x3d] = *(int *)(iVar13 + 0x3c);
      }
    }
    else if (cVar1 == '\x01') {
      FUN_ram_f004735c(iVar13 + 0x3c);
    }
    goto LAB_ram_f00481d4;
  }
  uVar2 = *(undefined1 *)(iVar13 + 0x36);
  uVar10 = *(uint *)(iVar13 + 0x40);
  *(uint *)(iVar13 + 0x38) = uVar11;
  FUN_ram_f003bc90();
  piVar3 = _DAT_ram_f0065160;
  if (uVar11 == 0x26) {
    uVar5 = _DAT_ram_f0065160[0xe];
  }
  else {
    if (0x26 < uVar11) {
      if (uVar11 == 0x2a) {
        if (0x28 < uVar10) goto LAB_ram_f0048184;
        if (uVar10 == 0) {
          uVar11 = _DAT_ram_60204870 & 0xfffffff | 0x30000000;
          puVar8 = (uint *)(_DAT_ram_f0065160 + 0x2d);
          iVar12 = 0;
          do {
            _DAT_ram_60204870 = uVar11;
            *puVar8 = _DAT_ram_60204874 & 0x7fffff;
            puVar8 = puVar8 + 1;
            iVar12 = iVar12 + 1;
            uVar11 = uVar11 + 0x10000000;
          } while (iVar12 != 0xb);
        }
        piVar4 = _DAT_ram_f0065160 + 0x2d;
      }
      else {
        if (0x2a < uVar11) {
          if (uVar11 == 0x2d) goto LAB_ram_f0048184;
          if (uVar11 == 0x2e) {
            uVar11 = 0;
            if (piVar4[0x57] != 0) {
              uVar11 = (uint)piVar4[0x61] / (uint)piVar4[0x57];
              if (0xdc < uVar11) {
                uVar11 = 0xdc;
              }
              uVar11 = (uVar11 >> 1) - 0x6e & 0xff;
            }
            *(uint *)(iVar13 + 0x3c) = uVar11;
            if (*(byte *)((int)piVar4 + 0x189) < 0xdd) {
              uVar10 = (((int)(uint)*(byte *)((int)piVar4 + 0x189) >> 1) + -0x6e) * 0x100 & 0xffff;
            }
            else {
              uVar10 = 0;
            }
            *(uint *)(iVar13 + 0x3c) = uVar10 | uVar11;
            if (*(byte *)(piVar4 + 0x62) < 0xdd) {
              uVar5 = (((int)(uint)*(byte *)(piVar4 + 0x62) >> 1) - 0x6eU & 0xff) << 0x10;
            }
            else {
              uVar5 = 0;
            }
            uVar5 = uVar5 | uVar10 | uVar11;
            *(uint *)(iVar13 + 0x3c) = uVar5;
            if (*(byte *)((int)piVar4 + 0x18a) < 0xdd) {
              uVar11 = (((int)(uint)*(byte *)((int)piVar4 + 0x18a) >> 1) + -0x6e) * 0x1000000;
            }
            else {
              uVar11 = 0;
            }
            uVar5 = uVar11 | uVar5;
          }
          else {
            if (uVar11 != 0x2b) goto LAB_ram_f0048180;
            uVar11 = FUN_ram_f004655c();
            _DAT_ram_f0065160[0x3a] =
                 (int)(char)((char)(((short)((short)uVar11 - (ushort)DAT_ram_f006503e) * 0xa5) /
                                   0x6d) + DAT_ram_f006503f) << 0x10 | uVar11;
            uVar5 = _DAT_ram_f0065160[0x3a];
          }
          goto LAB_ram_f0048182;
        }
        if (uVar11 == 0x28) {
          if (((uint)piVar4[0x5a] < 4) || (piVar4[0x5a] - 4U < uVar10)) goto LAB_ram_f0048184;
          piVar4 = (int *)_DAT_ram_f0065160[7];
        }
        else if (uVar11 < 0x29) {
          if (0x74 < uVar10) goto LAB_ram_f0048184;
          if (uVar10 == 0) {
            FUN_ram_f0046990();
          }
          piVar4 = _DAT_ram_f0065160 + 0xf;
        }
        else {
          if (0x13 < uVar10) goto LAB_ram_f0048184;
          if (uVar10 == 0) {
            uVar9 = _DAT_ram_60200604 >> 0x10;
            uVar7 = _DAT_ram_60200608 >> 0x10;
            uVar6 = _DAT_ram_60200604 & 0xffff;
            uVar5 = _DAT_ram_60200608 & 0xffff;
            uVar11 = _DAT_ram_6020060c & 0xffff;
            _DAT_ram_f0065160[0x60] = _DAT_ram_6020060c >> 0x10;
            piVar3[0x5b] = uVar6;
            piVar3[0x5c] = uVar9;
            piVar3[0x5d] = uVar5;
            piVar3[0x5e] = uVar7;
            piVar3[0x5f] = uVar11;
          }
          piVar4 = piVar4 + 0x5b;
        }
      }
LAB_ram_f004809e:
      (*(code *)&SUB_ram_0006a08c)(iVar13 + 0x3c,(int)piVar4 + uVar10,4);
      goto LAB_ram_f0048184;
    }
    if (uVar11 == 0x22) {
      if ((short)_DAT_ram_f0065160[0x4f] == 0) {
        uVar5 = _DAT_ram_f006e4e0 - _DAT_ram_f006e4c0;
      }
      else {
        uVar5 = piVar4[0x56];
      }
    }
    else {
      if (uVar11 < 0x23) {
        if (uVar11 == 0x20) {
          iVar12 = piVar4[0x54];
        }
        else {
          if (uVar11 < 0x21) {
            if (uVar11 != 0) goto LAB_ram_f0048180;
            uVar5 = 0x1000002;
            goto LAB_ram_f0048182;
          }
          iVar12 = piVar4[0x55];
        }
LAB_ram_f0047eba:
        *(int *)(iVar13 + 0x3c) = iVar12;
        goto LAB_ram_f0048184;
      }
      if (uVar11 == 0x24) {
        if (uVar10 == 0x28) {
          iVar12 = piVar4[0x5a];
          goto LAB_ram_f0047eba;
        }
        if (uVar10 < 0x29) {
          if (uVar10 != 0x25) {
            if (uVar10 == 0x27) {
              if (*_DAT_ram_f0065160 == 9) {
                uVar5 = 0;
              }
              else {
                uVar5 = 0x78;
              }
              goto LAB_ram_f0048182;
            }
            goto LAB_ram_f0048180;
          }
        }
        else if (uVar10 != 0x29) {
          if (uVar10 == 0x2a) {
            if (_DAT_ram_f0065160[0x38] == 1) {
              uVar5 = 0x20;
            }
            else {
              if (_DAT_ram_f0065160[0x38] != 2) goto LAB_ram_f0048180;
              uVar5 = 0x2c;
            }
          }
          else {
LAB_ram_f0048180:
            uVar5 = 0;
          }
          goto LAB_ram_f0048182;
        }
        uVar5 = 0x18;
      }
      else {
        uVar5 = _DAT_ram_f006e4c0;
        if (0x24 < uVar11) {
          if (0x14 < uVar10) goto LAB_ram_f0048184;
          piVar4 = _DAT_ram_f0065160 + 8;
          goto LAB_ram_f004809e;
        }
      }
    }
  }
LAB_ram_f0048182:
  *(uint *)(iVar13 + 0x3c) = uVar5;
LAB_ram_f0048184:
  *(undefined1 *)(iVar13 + 0x34) = 0x10;
  *(undefined1 *)(iVar13 + 0x35) = uVar2;
  *(undefined2 *)(iVar13 + 0x30) = 0x10;
  *(undefined1 *)(param_1 + 0x2f) = 1;
  *(undefined2 **)(param_1 + 0x28) = (undefined2 *)(iVar13 + 0x30);
  *(undefined2 *)(iVar13 + 0x32) = 1;
  FUN_ram_f0039750(param_1);
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_test.c:0x315 <<<
// ===== FUN_ram_f00481e8 @ 0xf00481e8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00481e8(void)

{
  undefined1 auStack_38 [12];
  undefined1 auStack_2c [6];
  undefined1 local_26;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18;
  undefined1 local_17;
  undefined1 local_16;
  undefined1 local_15;
  
  _DAT_ram_f0065160 = (undefined4 *)FUN_ram_f0025b48(2,0x18c);
  if (_DAT_ram_f0065160 == (undefined4 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf00643cc,0x315,_DAT_ram_00014800);
  }
  (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_f0065160,0,0x18c);
  FUN_ram_f00266a0(_DAT_ram_f0065160 + 1,&LAB_ram_f0048418,0);
  _DAT_ram_f0065160[7] = 0x204398;
  *_DAT_ram_f0065160 = 3;
  local_1c = 3;
  local_20 = 1;
  FUN_ram_f004735c(&local_20);
  (*_DAT_ram_00014a4c)(1,_DAT_ram_00014a4c);
  (*_DAT_ram_00014a50)(0,_DAT_ram_00014a50);
  (*_DAT_ram_00014970)(1,0,_DAT_ram_00014970);
  (*_DAT_ram_00014960)(0x7f07ff,1,_DAT_ram_00014960);
  (*_DAT_ram_00014ab0)(0,_DAT_ram_00014ab0);
  (*_DAT_ram_00014ad4)(0x400,_DAT_ram_00014ad4);
  (*_DAT_ram_00014af4)(0,1,_DAT_ram_00014af4);
  if ((_DAT_ram_60120070 & 0x10) == 0) {
    (*_DAT_ram_00014b00)(1,_DAT_ram_00014b00);
  }
  (*_DAT_ram_00014988)(0x20000000,0,_DAT_ram_00014988);
  FUN_ram_f003bc90();
  (*(code *)&SUB_ram_0006a0a0)(&DAT_ram_f006e4c0,0,0x50);
  (*(code *)&SUB_ram_0006a0a0)(_DAT_ram_f0065160 + 0x53,0,0x40);
  DAT_ram_f0064fd0 = 0;
  DAT_ram_f0064fd1 = 0;
  _DAT_ram_f0065164 = FUN_ram_f0025d90(0);
  *(undefined1 *)(_DAT_ram_f0065164 + 0x28a) = 0;
  *(undefined1 *)(_DAT_ram_f0065164 + 9) = 1;
  local_26 = 0;
  local_18 = 1;
  local_16 = 1;
  local_17 = 1;
  local_15 = 1;
  (*_DAT_ram_00014e4c)
            (*(undefined1 *)(_DAT_ram_f0065164 + 0x28a),auStack_2c,&local_18,auStack_38,
             _DAT_ram_00014e4c);
  *(undefined1 *)(_DAT_ram_020a0068 + 6) = 1;
  _DAT_ram_020a0068[3] = 1;
  *_DAT_ram_020a0068 = 0;
  _DAT_ram_6020080c = _DAT_ram_6020080c | 0x300000;
  _DAT_ram_f0065160[0x39] = _DAT_ram_60204870 & 0x3000000;
  if ((_DAT_ram_f00650a4 == 4) && (_DAT_ram_f006514c == 0x6625)) {
    FUN_ram_f0044d2c();
    FUN_ram_f0049c24(0x10);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004844c @ 0xf004844c =====


void FUN_ram_f004844c(int param_1)

{
  int iVar1;
  byte *pbVar2;
  int iVar3;
  uint uVar4;
  
  DAT_ram_f0065057 = 0x3f;
  DAT_ram_f0065056 = 0x3f;
  iVar1 = 1;
  do {
    iVar3 = 0;
    pbVar2 = (byte *)(param_1 + iVar1 * 0xc + 1);
    do {
      uVar4 = (uint)*pbVar2;
      iVar3 = iVar3 + 1;
      if (uVar4 != 0) {
        DAT_ram_f0065056 =
             *pbVar2 * ((int)uVar4 < (int)(char)DAT_ram_f0065056) |
             DAT_ram_f0065056 * ((int)(char)DAT_ram_f0065056 <= (int)uVar4);
      }
      pbVar2 = pbVar2 + 2;
    } while (iVar3 != 6);
    iVar1 = iVar1 + 1;
  } while (iVar1 != 3);
  pbVar2 = (byte *)(param_1 + 0x25);
  iVar1 = 0;
  do {
    uVar4 = (uint)*pbVar2;
    iVar1 = iVar1 + 1;
    if (uVar4 != 0) {
      DAT_ram_f0065057 =
           *pbVar2 * ((int)uVar4 < (int)(char)DAT_ram_f0065057) |
           DAT_ram_f0065057 * ((int)(char)DAT_ram_f0065057 <= (int)uVar4);
    }
    pbVar2 = pbVar2 + 2;
  } while (iVar1 != 6);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00484bc @ 0xf00484bc =====


bool FUN_ram_f00484bc(char param_1,char param_2)

{
  char *pcVar1;
  
  if (param_2 == '\0') {
    pcVar1 = &DAT_ram_f0065056;
  }
  else {
    if (param_2 != '\x01') {
      return false;
    }
    pcVar1 = &DAT_ram_f0065057;
  }
  return 8 < (int)param_1 + (int)*pcVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00484ec @ 0xf00484ec =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00484ec(char *param_1,char param_2,char param_3,char param_4,char param_5,char param_6
                     )

{
  bool bVar1;
  bool bVar2;
  char cVar3;
  byte bVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  byte *pbVar8;
  char cVar9;
  char cVar10;
  byte bVar11;
  int iVar12;
  char *pcVar13;
  char *pcVar14;
  undefined1 *puVar15;
  int iVar16;
  uint uVar17;
  uint local_74;
  uint local_70;
  char cStack_50;
  char local_4f [11];
  undefined1 auStack_44 [7];
  undefined1 local_3d;
  undefined1 local_3b;
  
  iVar6 = (int)param_5;
  iVar7 = (int)param_6;
  if (DAT_ram_f0065053 == '\0') {
    if (_DAT_ram_f006513c == 0x24ba58) {
      if (DAT_ram_f006503c == '\0') {
        cVar9 = '\x06';
      }
      else {
        cVar9 = '\x03';
      }
    }
    else {
      cVar9 = '\x02';
    }
  }
  else {
    cVar9 = '\0';
  }
  bVar4 = cVar9 + 0x18;
  if (_DAT_ram_f006513c == 0x24ba58) {
    cVar10 = DAT_ram_f006504a;
    cVar3 = DAT_ram_f0065049;
    if (DAT_ram_f0065039 == '\x01') {
      bVar1 = true;
    }
    else {
      bVar1 = DAT_ram_f0065039 == '\v';
      if (!bVar1) {
        if (DAT_ram_f0065039 == '\x05') {
          uVar17 = _DAT_ram_f0065140 ^ 3;
        }
        else {
          if (DAT_ram_f0065039 != '\a') {
            bVar2 = false;
            goto LAB_ram_f004867e;
          }
          uVar17 = _DAT_ram_f0065140 ^ 1;
        }
        bVar2 = uVar17 == 0;
        goto LAB_ram_f004867e;
      }
    }
    bVar2 = true;
    goto LAB_ram_f004867e;
  }
  cVar10 = DAT_ram_f0065055;
  cVar3 = DAT_ram_f0065054;
  if (_DAT_ram_f006513c == 5000000) {
    if ((DAT_ram_f0065039 == '$') || (DAT_ram_f0065039 == '@')) {
      bVar1 = true;
    }
    else {
      bVar1 = DAT_ram_f0065039 == 'd';
      if ((!bVar1) && ((DAT_ram_f0065039 != '(' || (_DAT_ram_f0065140 != 3))))
      goto LAB_ram_f0048628;
    }
    bVar2 = true;
  }
  else {
    bVar1 = false;
LAB_ram_f0048628:
    if (DAT_ram_f0065039 == '<') {
      uVar17 = _DAT_ram_f0065140 ^ 1;
    }
    else {
      if (DAT_ram_f0065039 != 'h') {
        bVar2 = false;
        goto LAB_ram_f004867e;
      }
      uVar17 = _DAT_ram_f0065140 ^ 3;
    }
    bVar2 = uVar17 == 0;
  }
LAB_ram_f004867e:
  local_70 = (uint)cVar3;
  local_74 = (uint)cVar10;
  iVar16 = (int)DAT_ram_f0065047;
  iVar12 = (int)DAT_ram_f0065046;
  uVar17 = iVar16 * (uint)(iVar16 < iVar12) | iVar12 * (uint)(iVar12 <= iVar16);
  if (((_DAT_ram_f0065174 == 1) && (_DAT_ram_f006513c == 5000000)) && (DAT_ram_f0065039 == 'd')) {
    uVar17 = 0x1a;
  }
  uVar5 = uVar17 * ((int)uVar17 < iVar6) | iVar6 * (uint)(iVar6 <= (int)uVar17);
  uVar17 = uVar17 * ((int)uVar17 < iVar7) | iVar7 * (uint)(iVar7 <= (int)uVar17);
  pcVar13 = &cStack_50;
  pcVar14 = param_1;
  while( true ) {
    iVar6 = (int)DAT_ram_f0065004 + (int)*pcVar14;
    bVar11 = (char)uVar17 * ((int)uVar17 < iVar6) | (char)iVar6 * (iVar6 <= (int)uVar17);
    if (bVar1) {
      bVar11 = bVar11 * ((char)bVar11 < DAT_ram_f0065048) |
               DAT_ram_f0065048 * (DAT_ram_f0065048 <= (char)bVar11);
    }
    cVar10 = cVar9;
    if (param_2 != '\0') {
      cVar10 = cVar9 + param_3;
    }
    pcVar13[1] = cVar10 + bVar11;
    pcVar13 = pcVar13 + 2;
    if (pcVar13 == local_4f + 3) break;
    pcVar14 = pcVar14 + 1;
  }
  pcVar14 = param_1 + 0x12;
  puVar15 = auStack_44;
  cVar10 = '\x01';
  do {
    uVar17 = uVar5;
    if (cVar10 == '\x03') {
      pcVar13 = pcVar14;
      if ((bVar2) && ((int)local_74 < (int)uVar5)) {
        uVar17 = local_74;
      }
    }
    else {
      pcVar13 = param_1 + 0xc;
      if (cVar10 == '\x01') {
        pcVar13 = param_1 + 6;
      }
      if ((bVar1) && ((int)local_70 < (int)uVar5)) {
        uVar17 = local_70;
      }
    }
    pbVar8 = puVar15 + 1;
    iVar6 = 0;
    do {
      iVar7 = (int)DAT_ram_f0065004 + (int)pcVar13[iVar6];
      bVar11 = (char)uVar17 * ((int)uVar17 < iVar7) | (char)iVar7 * (iVar7 <= (int)uVar17);
      cVar3 = cVar9;
      if ((param_2 != '\0') && ((char)bVar11 < param_4)) {
        cVar3 = cVar9 + param_3;
      }
      bVar11 = cVar3 + bVar11;
      *pbVar8 = bVar11;
      if ((('\x02' < (char)iVar6) && (_DAT_ram_60205700 == 0)) && (bVar4 <= bVar11)) {
        if (bVar4 < (byte)(bVar11 - 8)) {
          *pbVar8 = bVar11 - 8;
        }
        else {
          *pbVar8 = bVar4;
        }
      }
      iVar6 = iVar6 + 1;
      pbVar8 = pbVar8 + 2;
    } while (iVar6 != 6);
    cVar10 = cVar10 + '\x01';
    puVar15 = puVar15 + 0xc;
  } while (cVar10 != '\x04');
  local_3d = local_3b;
  FUN_ram_f004844c(&cStack_50);
  _DAT_ram_6013009c = _DAT_ram_6013009c & 0xff80ffff | 0x80000;
  (*_DAT_ram_00014dcc)(0,&cStack_50,_DAT_ram_00014dcc);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048840 @ 0xf0048840 =====


int FUN_ram_f0048840(uint param_1,byte param_2,char param_3)

{
  char cVar1;
  int iVar2;
  undefined1 *puVar3;
  
  if (param_1 < 4000000) {
    if ((param_2 & 0xc0) == 0) {
      if ((param_2 & 0xf) < 2) {
        iVar2 = (int)DAT_ram_f0064e49;
      }
      else {
        iVar2 = (int)DAT_ram_f0064e48;
      }
    }
    else if ((param_2 & 0xc0) == 0x40) {
      iVar2 = (int)DAT_ram_f0064e4e;
    }
    else {
      if (param_3 == '\0') {
        puVar3 = &DAT_ram_f0064e54;
        cVar1 = DAT_ram_f0064e54;
      }
      else {
        puVar3 = &DAT_ram_f0064e5a;
        cVar1 = DAT_ram_f0064e5a;
      }
      iVar2 = (int)cVar1;
      param_2 = param_2 & 0x3f;
      if (1 < param_2) {
        if (param_2 < 4) {
          iVar2 = (int)(char)puVar3[1];
        }
        else if (param_2 == 4) {
          iVar2 = (int)(char)puVar3[2];
        }
        else if (param_2 == 5) {
          iVar2 = (int)(char)puVar3[3];
        }
        else if (param_2 == 6) {
          iVar2 = (int)(char)puVar3[4];
        }
        else {
          iVar2 = (int)(char)puVar3[5];
        }
      }
    }
  }
  else {
    iVar2 = 0;
  }
  return iVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00488dc @ 0xf00488dc =====


int FUN_ram_f00488dc(byte param_1)

{
  int iVar1;
  
  if (DAT_ram_f0065044 < param_1) {
    if (DAT_ram_f0065045 < param_1) {
      iVar1 = 2;
    }
    else {
      iVar1 = 1;
    }
  }
  else {
    iVar1 = 0;
  }
  return (int)(char)(&DAT_ram_f006fda0)[iVar1];
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048914 @ 0xf0048914 =====


void FUN_ram_f0048914(byte param_1,char param_2)

{
  int iVar1;
  
  if (DAT_ram_f0065044 < param_1) {
    if (DAT_ram_f0065045 < param_1) {
      iVar1 = 2;
    }
    else {
      iVar1 = 1;
    }
  }
  else {
    iVar1 = 0;
  }
  (&DAT_ram_f006fda0)[iVar1] = (&DAT_ram_f006fda0)[iVar1] + param_2;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048954 @ 0xf0048954 =====


int FUN_ram_f0048954(byte param_1)

{
  int iVar1;
  
  if (DAT_ram_f0065044 < param_1) {
    if (DAT_ram_f0065045 < param_1) {
      iVar1 = 2;
    }
    else {
      iVar1 = 1;
    }
  }
  else {
    iVar1 = 0;
  }
  return (int)*(short *)(&DAT_ram_f006fa34 + iVar1 * 2);
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004898c @ 0xf004898c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004898c(void)

{
  _DAT_ram_60205c60 = _DAT_ram_60205c60 & 0x7fffffbd;
  _DAT_ram_60205c68 =
       _DAT_ram_60205c68 & 0xf800ff80 | (uint)_DAT_ram_f0065070 << 0x10 | (uint)_DAT_ram_f006506e;
  _DAT_ram_60205c64 = _DAT_ram_60205c64 & 0xffc00000 | 0x108a8a;
  _DAT_ram_60205c70 = _DAT_ram_60205c70 & 0xfff0ffff | 0xd0000;
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_txpwr.c:0x822 <<<
// ===== FUN_ram_f0048a48 @ 0xf0048a48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f0048a48(int param_1)

{
  uint uVar1;
  int iVar2;
  
  if (param_1 == 0x25e720) {
    uVar1 = 0xe;
  }
  else {
    uVar1 = (param_1 - 0x24ba58U) / 5000 & 0xff;
  }
  if ((uVar1 - 1 & 0xff) < 0xe) {
    iVar2 = ((int)(uVar1 - 1) >> 1) * 0x74 + -0xff905a4;
  }
  else {
    (*_DAT_ram_00014800)(0,0,0xf006440c,0x822,_DAT_ram_00014800);
    iVar2 = 0;
  }
  return iVar2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048ab4 @ 0xf0048ab4 =====


undefined4 FUN_ram_f0048ab4(int param_1)

{
  undefined4 uVar1;
  
  if (param_1 - 5U < 0x18) {
    uVar1 = *(undefined4 *)((param_1 - 5U) * 4 + -0xff9b870);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048acc @ 0xf0048acc =====


int FUN_ram_f0048acc(uint param_1)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = param_1 & 7;
  param_1 = param_1 & 0xfffffff8;
  if (uVar2 != 0) {
    uVar2 = uVar2 - 8;
    param_1 = param_1 + 8;
  }
  if (param_1 >> 3 < 4) {
    iVar1 = 0;
  }
  else {
    iVar1 = (param_1 >> 3) - 4;
  }
  return iVar1 * 0x20 + (uVar2 + 7) * 2;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048af4 @ 0xf0048af4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f0048af4(int param_1,undefined4 param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  
  uVar1 = FUN_ram_f0048acc(param_2);
  uVar3 = uVar1 >> 5;
  _DAT_ram_60205c0c = _DAT_ram_60205c0c & 0xfffffe00;
  uVar1 = (uVar1 & 0x1e) >> 1;
  if (8 < uVar3) {
    iVar4 = 0;
    goto LAB_ram_f0048c38;
  }
  uVar3 = 9 - uVar3;
  if (uVar3 < 3) {
    uVar1 = uVar1 + (2 - uVar3) * 8;
    iVar4 = 1;
  }
  else {
    if (_DAT_ram_f006514c == 0x6625) {
      if (param_1 == 0x24ba58) {
        if (uVar3 != 3) {
          uVar3 = uVar3 ^ 4;
          uVar2 = 0x26;
          uVar5 = 0x2e;
          goto LAB_ram_f0048c06;
        }
        uVar5 = 0x38;
      }
      else if (DAT_ram_f006503d == '\x01') {
        if (uVar3 == 3) {
          uVar5 = 0x36;
        }
        else if (uVar3 == 4) {
          uVar5 = 0x2d;
        }
        else if (uVar3 == 5) {
          uVar5 = 0x24;
        }
        else if (uVar3 == 6) {
          uVar5 = 0x1b;
        }
        else {
          if (uVar3 != 7) {
            uVar3 = uVar3 ^ 8;
            uVar2 = 0xe;
            uVar5 = 0x11;
            goto LAB_ram_f0048c06;
          }
          uVar5 = 0x16;
        }
      }
      else {
        if (uVar3 == 3) {
          iVar4 = 0x40;
        }
        else if (uVar3 == 4) {
          iVar4 = 0x33;
        }
        else if (uVar3 == 5) {
          iVar4 = 0x28;
        }
        else if (uVar3 == 6) {
          iVar4 = 0x20;
        }
        else if (uVar3 == 7) {
          iVar4 = 0x1a;
        }
        else {
          iVar4 = 0x14;
          if (uVar3 != 8) {
            iVar4 = 0x10;
          }
        }
        uVar5 = iVar4 - 3;
      }
    }
    else if (uVar3 == 3) {
      uVar5 = 0x33;
    }
    else {
      uVar3 = uVar3 ^ 4;
      uVar2 = 0x20;
      uVar5 = 0x28;
LAB_ram_f0048c06:
      if (uVar3 != 0) {
        uVar5 = uVar2;
      }
    }
    _DAT_ram_60205c0c = _DAT_ram_60205c0c | 0x100 | uVar5;
    iVar4 = 0;
  }
  uVar3 = 9;
LAB_ram_f0048c38:
  return uVar3 * 0x20 + uVar1 * 2 + iVar4;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048c48 @ 0xf0048c48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0048c48(void)

{
  int iVar1;
  
  iVar1 = FUN_ram_f0048af4();
  _DAT_ram_60205c00 = _DAT_ram_60205c00 & 0x6ffff0 | 0x10000f | iVar1 << 0x17;
  _DAT_ram_60205620 = _DAT_ram_60205620 | 1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048c9c @ 0xf0048c9c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f0048c9c(void)

{
  int iVar1;
  
  _DAT_ram_60205404 = _DAT_ram_60205404 & 0xffffefc8 | 0x1007;
  _DAT_ram_60205400 = 0x100;
  _DAT_ram_60205604 = 0xc;
  _DAT_ram_60205a00 = 1;
  iVar1 = 0;
  do {
    (*_DAT_ram_0001480c)(0x21,_DAT_ram_0001480c);
    iVar1 = iVar1 + 1;
    if ((_DAT_ram_60205a18 >> 0x1e & 1) == 0) {
      return 1;
    }
  } while (iVar1 != 0x14);
  _DAT_ram_60200000 = 0x80021000;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048d20 @ 0xf0048d20 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0048d20(void)

{
  uint uVar1;
  
  uVar1 = FUN_ram_f0048acc();
  _DAT_ram_60205604 = uVar1 >> 1 & 0xff | _DAT_ram_60205604 & 0xffffff00;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048d54 @ 0xf0048d54 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0048d54(void)

{
  bool bVar1;
  bool bVar2;
  uint *puVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  int aiStack_f8 [28];
  int local_88 [29];
  
  (*(code *)&SUB_ram_0006a08c)(local_88,0xf0064638);
  (*(code *)&SUB_ram_0006a08c)(aiStack_f8,0xf00645c8,0x70);
  bVar2 = false;
  puVar3 = (uint *)&DAT_ram_60205700;
  bVar1 = false;
  do {
    uVar6 = *puVar3;
    puVar3 = puVar3 + 1;
    if (uVar6 >> 0x18 == 3) {
LAB_ram_f0048dac:
      if (bVar2) {
        bVar1 = true;
        goto LAB_ram_f0048dc2;
      }
      bVar1 = true;
    }
    else {
      if (uVar6 >> 0x18 == 4) {
        bVar2 = true;
      }
      if (bVar1) goto LAB_ram_f0048dac;
    }
  } while (puVar3 != (uint *)&DAT_ram_6020576c);
  bVar1 = false;
LAB_ram_f0048dc2:
  uVar6 = _DAT_ram_60205700 >> 0x10;
  if ((0x400 < uVar6) && (!bVar1)) {
    uVar6 = uVar6 - 0x800;
  }
  iVar4 = 0;
  do {
    puVar3 = (uint *)(&DAT_ram_60205700 + iVar4);
    uVar5 = *puVar3 >> 0x10;
    if ((0x400 < uVar5) && (!bVar1)) {
      uVar5 = uVar5 - 0x800;
    }
    if (!bVar1) {
      uVar5 = ((int)((uVar5 - uVar6) * *(int *)((int)aiStack_f8 + iVar4)) >> 5) + uVar6;
    }
    *puVar3 = (uVar5 & 0x7ff) * 0x10000 +
              ((((*puVar3 & 0xffff) - 0x100) * *(int *)((int)local_88 + iVar4) >> 5) + 0x100 & 0x1ff
              );
    iVar4 = iVar4 + 4;
  } while (iVar4 != 0x70);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048e48 @ 0xf0048e48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0048e48(int param_1,byte param_2,int param_3)

{
  int iVar1;
  
  if (_DAT_ram_f006514c == 0x6625) {
    if (param_1 != 0x24ba58) {
      if (DAT_ram_f006503c == '\0') {
        param_3 = param_3 + 0xb;
        goto LAB_ram_f0048ee6;
      }
      if (DAT_ram_f006503d == '\x01') {
        if (param_2 == 0) {
          param_3 = param_3 + 0xd;
          goto LAB_ram_f0048ee6;
        }
        if (param_2 < 2) {
          param_3 = param_3 + 0x11;
          goto LAB_ram_f0048ee6;
        }
        if (param_2 < 7) goto LAB_ram_f0048f04;
      }
      else {
        if (param_2 < 2) goto LAB_ram_f0048eba;
        if (param_2 < 4) goto LAB_ram_f0048ecc;
      }
      iVar1 = param_3 + 0xb;
      goto LAB_ram_f0048f08;
    }
    if (DAT_ram_f006503c != '\0') {
LAB_ram_f0048eba:
      param_3 = param_3 + 0xf;
LAB_ram_f0048ee6:
      FUN_ram_f0048d20(param_3);
      return;
    }
    if (2 < param_2) {
LAB_ram_f0048ecc:
      iVar1 = param_3 + 0xd;
      goto LAB_ram_f0048f08;
    }
  }
  else {
    if (param_2 < 2) {
      param_3 = param_3 + 0x13;
      goto LAB_ram_f0048ee6;
    }
    if (param_2 < 4) {
      iVar1 = param_3 + 0x11;
      goto LAB_ram_f0048f08;
    }
  }
LAB_ram_f0048f04:
  iVar1 = param_3 + 0xf;
LAB_ram_f0048f08:
  FUN_ram_f0048d20(iVar1,param_2,param_3,FUN_ram_f0048d20);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0048f14 @ 0xf0048f14 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined1 FUN_ram_f0048f14(int param_1,undefined1 param_2,byte param_3)

{
  int iVar1;
  uint uVar2;
  uint *puVar3;
  uint uVar4;
  int iVar5;
  undefined1 uVar6;
  int iVar7;
  
  iVar7 = 0;
  uVar4 = 0x78;
  do {
    FUN_ram_f0048c48(param_1,uVar4);
    iVar1 = FUN_ram_f0048c9c();
    if (iVar1 == 0) {
      iVar1 = 0;
      uVar6 = 0;
      goto LAB_ram_f0048fa2;
    }
    puVar3 = (uint *)&DAT_ram_6020576c;
    iVar1 = 0;
    do {
      iVar5 = iVar1 + 1;
      if ((*puVar3 & 0xffff) < 0x16a) {
        if (iVar1 == 0) {
          uVar4 = uVar4 + 2;
        }
        break;
      }
      puVar3 = puVar3 + -1;
      iVar1 = iVar5;
    } while (iVar5 != 0x1c);
    uVar2 = FUN_ram_f0048ab4(iVar1);
    if (uVar4 < uVar2) {
      uVar4 = 0;
    }
    else {
      uVar4 = uVar4 - uVar2;
    }
    if (iVar1 - 1U < 4) goto LAB_ram_f0048fa0;
    iVar7 = iVar7 + 1;
  } while (iVar7 != 6);
  if (iVar1 - 1U < 8) {
LAB_ram_f0048fa0:
    uVar6 = 1;
  }
  else {
    uVar6 = 0;
  }
LAB_ram_f0048fa2:
  FUN_ram_f0048d54();
  if ((_DAT_ram_f006514c != 0x6625) || (param_1 == 5000000)) {
    uVar4 = uVar4 + (int)DAT_ram_f0065052;
  }
  FUN_ram_f0048e48(param_1,param_2,uVar4);
  if (iVar1 - 5U < 2) {
    _DAT_ram_60205604 = (_DAT_ram_60205604 & 0xffffff00) + 4;
  }
  else if (iVar1 - 7U < 2) {
    _DAT_ram_60205604 = (_DAT_ram_60205604 & 0xffffff00) + 6;
  }
  if ((((DAT_ram_f0065040 != '\x02') && ((_DAT_ram_60205604 & 0xff) < 0xa0)) &&
      (param_1 == 0x24ba58)) && (DAT_ram_f006503d == '\x01')) {
    if (param_3 < 2) {
      (*_DAT_ram_00014814)(2,0xf006442c,_DAT_ram_00014814);
    }
    uVar6 = 0;
  }
  return uVar6;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0049074 @ 0xf0049074 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0049074(int param_1,char param_2,char param_3)

{
  if (param_1 == 1) {
    _DAT_ram_60205600 = _DAT_ram_60205600 & 0xfffcffef | 0x10010;
  }
  else if (param_1 == 0) {
    _DAT_ram_60205600 = _DAT_ram_60205600 & 0xfffcffef | 0x20010;
  }
  else {
    _DAT_ram_6020560c =
         _DAT_ram_6020560c & 0x80808080 | 0x3f3f0000 | (int)param_2 & 0x7fU |
         ((int)param_3 & 0x7fU) << 8;
    if (_DAT_ram_f006514c == 0x6625) {
      _DAT_ram_60205600 = _DAT_ram_60205600 & 0xfffc9d6f | 0x4290;
    }
    else {
      _DAT_ram_60205600 = _DAT_ram_60205600 & 0xfffcfdef | 0x210;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004912c @ 0xf004912c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004912c(void)

{
  if ((DAT_ram_f0064e49 == '\0') ||
     (DAT_ram_f0064e49 = DAT_ram_f0065051, _DAT_ram_f0065074 != 0x2866)) {
    DAT_ram_f0064e49 = DAT_ram_f0064e48;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004931c @ 0xf004931c =====


void FUN_ram_f004931c(void)

{
  undefined1 *puVar1;
  
  FUN_ram_f00266a0(0xf006fa44,&LAB_ram_f0049f44,2);
  FUN_ram_f00266a0(0xf0070180,&LAB_ram_f0049f44,3);
  FUN_ram_f00266a0(0xf006fd88,&LAB_ram_f0049f44,4);
  puVar1 = &DAT_ram_f006f211;
  do {
    *puVar1 = 0;
    puVar1 = puVar1 + 0x124;
  } while (puVar1 != (undefined1 *)0xf006fb31);
  DAT_ram_f0064fce = 0;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00494b0 @ 0xf00494b0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00494b0(void)

{
  FUN_ram_f0042ee4(200,0x820240c0);
  (*_DAT_ram_0001480c)(3,_DAT_ram_0001480c);
  (*_DAT_ram_00014ea4)(0x41c,0xf0000000,1,_DAT_ram_00014ea4);
  FUN_ram_f0042ee4(200,0x20240c0);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0049504 @ 0xf0049504 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0049504(int param_1,int param_2,byte param_3)

{
  undefined4 *puVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 *puVar5;
  byte bVar6;
  undefined4 *puVar7;
  uint uVar8;
  byte bVar9;
  int iVar10;
  
  uVar8 = (uint)param_3;
  puVar7 = (undefined4 *)&DAT_ram_f0064538;
  do {
    puVar1 = (undefined4 *)*puVar7;
    puVar5 = puVar7 + 1;
    puVar7 = puVar7 + 2;
    *puVar1 = *puVar5;
  } while (puVar7 != (undefined4 *)0xf00645c8);
  if (param_2 == 0x24ba58) {
    FUN_ram_f0042ee4(0x70,0x8a);
    FUN_ram_f0042ee4(0xac,0);
    uVar2 = (*_DAT_ram_00014eb0)(0xd4,_DAT_ram_00014eb0);
    uVar2 = uVar2 | 0x80000000;
    uVar3 = 0xd4;
  }
  else {
    FUN_ram_f0042ee4(0x70,0x1000e84);
    uVar2 = 0x130000;
    uVar3 = 0xac;
  }
  FUN_ram_f0042ee4(uVar3,uVar2);
  FUN_ram_f0042ee4(0x54,0x43ff0003);
  FUN_ram_f0042ee4(0x6c,0x81bcec3f);
  FUN_ram_f0042ee4(0x48,0xfcb00727);
  _DAT_ram_60204604 = 0x400100;
  _DAT_ram_60205c0c = 0x46880;
  if (_DAT_ram_f006514c == 0x6625) {
    _DAT_ram_60205600 = _DAT_ram_60205600 & 0xffffff6c | 0x10;
    if (param_2 != 0x24ba58) {
      if (uVar8 == 0) {
        uVar3 = 0x75287528;
      }
      else if (uVar8 == 1) {
        uVar3 = 0x74aa74aa;
      }
      else {
        if ((((uVar8 != 2) && (uVar8 != 3)) && (uVar8 != 4)) && ((uVar8 != 5 && (uVar8 != 6)))) {
          if (uVar8 == 7) {
            FUN_ram_f0042ee4(0x55c,0x73f073f0);
          }
          goto LAB_ram_f0049672;
        }
        uVar3 = 0x742e742e;
      }
      FUN_ram_f0042ee4(0x55c,uVar3);
    }
  }
  else {
    _DAT_ram_60205600 = _DAT_ram_60205600 & 0xffffffec | 0x10;
  }
LAB_ram_f0049672:
  FUN_ram_f0044d9c();
  _DAT_ram_60205c60 = _DAT_ram_60205c60 & 0x7fffff3f | 0x40;
  if (DAT_ram_f006503d == '\x01') {
    _DAT_ram_6020080c = _DAT_ram_6020080c & 0xffffffc0 | 0x17;
    (*_DAT_ram_0001480c)(1,_DAT_ram_0001480c);
  }
  iVar10 = 0;
  _DAT_ram_60205c64 = _DAT_ram_60205c64 | 0x40000000;
  bVar9 = 0;
  do {
    iVar4 = FUN_ram_f0048f14(param_2,uVar8,iVar10);
    bVar6 = bVar9 + 1;
    if (iVar4 != 0) break;
    iVar10 = iVar10 + 1;
    bVar9 = bVar6;
  } while (bVar6 != 3);
  iVar10 = 0;
  if (param_2 != 0x24ba58) {
    uVar8 = 0;
  }
  if (bVar9 < 3) {
    puVar1 = (undefined4 *)(param_1 + uVar8 * 0x74);
    puVar5 = (undefined4 *)&DAT_ram_60205700;
    puVar7 = puVar1;
    do {
      uVar3 = *puVar5;
      puVar5 = puVar5 + 1;
      *puVar7 = uVar3;
      puVar7 = puVar7 + 1;
    } while (puVar5 != (undefined4 *)0x60205770);
    uVar2 = _DAT_ram_60205604 & 0xff;
    *(char *)(puVar1 + 0x1c) = (char)_DAT_ram_60205604;
    (*_DAT_ram_00014814)(2,0xf00644ac,uVar8,uVar2,_DAT_ram_00014814);
    *(undefined1 *)((int)puVar1 + 0x73) = 0x3f;
    *(undefined1 *)((int)puVar1 + 0x71) = 0;
    *(undefined1 *)((int)puVar1 + 0x72) = 0x1c;
  }
  else {
    param_1 = param_1 + uVar8 * 0x74;
    do {
      *(undefined4 *)(param_1 + iVar10) = 0;
      iVar10 = iVar10 + 4;
    } while (iVar10 != 0x70);
    *(undefined1 *)(param_1 + 0x72) = 0x3f;
    *(undefined1 *)(param_1 + 0x71) = 0;
    *(undefined1 *)(param_1 + 0x73) = 0x20;
    (*_DAT_ram_00014814)(2,0xf00644c8,uVar8,*(undefined1 *)(param_1 + 0x70),_DAT_ram_00014814);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f00497b4 @ 0xf00497b4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f00497b4(int param_1)

{
  uint uVar1;
  uint uVar2;
  uint *puVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  uint unaff_s3;
  
  uVar2 = _DAT_ram_f0065130;
  (*_DAT_ram_00014d28)(1,1,0,0,_DAT_ram_00014d28);
  if (param_1 == 0x24ba58) {
    (*_DAT_ram_00014814)(2,0xf00644e0,_DAT_ram_00014814);
    iVar5 = 0x24cde0;
    iVar6 = 0;
    do {
      FUN_ram_f004518c(iVar5,1,0);
      FUN_ram_f0049504(0xf006fa5c,0x24ba58,iVar6);
      iVar6 = iVar6 + 1;
      iVar5 = iVar5 + 10000;
    } while (iVar6 != 7);
  }
  else {
    (*_DAT_ram_00014814)
              (2,0xf00644e8,_DAT_ram_f0065130,_DAT_ram_f0065140,DAT_ram_f0065039,_DAT_ram_00014814);
    FUN_ram_f004518c(uVar2,1,0);
    FUN_ram_f0044048(uVar2,_DAT_ram_f0065140,1);
    puVar3 = (uint *)&DAT_ram_f0063d90;
    uVar1 = 0;
    do {
      uVar4 = uVar1;
      uVar1 = uVar4 + 1 & 0xff;
      if (uVar2 < *puVar3) break;
      puVar3 = puVar3 + 1;
      unaff_s3 = uVar4;
    } while (uVar1 != 8);
    FUN_ram_f0049504(0xf006fda8,5000000,unaff_s3);
  }
  _DAT_ram_60205404 = _DAT_ram_60205404 & 0xffffefc8 | 0x10;
  FUN_ram_f0042ee4(0x6c,0);
  FUN_ram_f0042ee4(0x48,0x300727);
  FUN_ram_f0042ee4(0x54,0x3ff0003);
  FUN_ram_f0042ee4(0x70,0);
  uVar2 = (*_DAT_ram_00014eb0)(0xd4,_DAT_ram_00014eb0);
  FUN_ram_f0042ee4(0xd4,uVar2 & 0x7fffffff);
  FUN_ram_f0042ee4(0xac,0);
  _DAT_ram_60204604 = 0x100;
  _DAT_ram_60205c0c = 0x2880;
  _DAT_ram_60205c00 = _DAT_ram_60205c00 & 0x6ffff0;
  _DAT_ram_60205c60 = _DAT_ram_60205c60 & 0xffffff3f;
  _DAT_ram_6020080c = _DAT_ram_6020080c & 0xffffffc0;
  _DAT_ram_60205c64 = _DAT_ram_60205c64 & 0xbfffffff;
  (*_DAT_ram_00014d28)(0,0,0,0,_DAT_ram_00014d28);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004999c @ 0xf004999c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f004999c(void)

{
  uint uVar1;
  
  uVar1 = _DAT_ram_60120080;
  _DAT_ram_60120080 = _DAT_ram_60120080 | 0x300;
  if (_DAT_ram_f00650a4 == 4) {
    FUN_ram_f0046c10();
  }
  FUN_ram_f0044e24(0);
  (*_DAT_ram_00014d28)(1,1,0,0,_DAT_ram_00014d28);
  if ((DAT_ram_f0065041 & 8) != 0) {
    (*_DAT_ram_00014ecc)(_DAT_ram_00014ecc);
    _DAT_ram_f0065158 = _DAT_ram_f0065158 + 1;
    _DAT_ram_600001fc = _DAT_ram_600001fc & 0xffffff00 | _DAT_ram_f0065158;
  }
  if ((DAT_ram_f0065041 & 0xc) != 0) {
    FUN_ram_f0045e34();
  }
  if ((DAT_ram_f0065041 & 10) != 0) {
    FUN_ram_f00497b4(0x24ba58);
  }
  if (_DAT_ram_f006514c == 0x6625) {
    if (((DAT_ram_f0065041 & 0x50) != 0) && (FUN_ram_f0045380(), (DAT_ram_f0065041 & 0x40) != 0)) {
      _DAT_ram_f0065158 = _DAT_ram_f0065158 + 1;
      _DAT_ram_600001fc = _DAT_ram_600001fc & 0xffffff00 | _DAT_ram_f0065158;
    }
    if ((DAT_ram_f0065041 & 0x20) != 0) {
      FUN_ram_f00497b4(5000000);
    }
  }
  FUN_ram_f0044e24(0);
  (*_DAT_ram_00014d28)(0,0,0,0,_DAT_ram_00014d28);
  DAT_ram_f0065041 = 0;
  (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x20000000);
  FUN_ram_f0045c9c();
  if (_DAT_ram_f00650a4 == 4) {
    FUN_ram_f0046978();
  }
  _DAT_ram_60120080 = uVar1 & 0x300 | _DAT_ram_60120080 & 0xfffffcff;
  return 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0049b54 @ 0xf0049b54 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0049b54(int param_1)

{
  do {
    _DAT_ram_f0065170 = _DAT_ram_f006516c;
    _DAT_ram_f006516c = param_1;
    if (param_1 == 2) {
      (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x20000001);
    }
    else if (param_1 == 3) {
      param_1 = FUN_ram_f004999c();
    }
  } while (_DAT_ram_f006516c != param_1);
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_txpwr.c:0xd2e <<<
// ===== FUN_ram_f0049bac @ 0xf0049bac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0049bac(uint param_1)

{
  if ((param_1 & 0xffff0000) != 0x8000000) {
    (*_DAT_ram_00014800)(0,0,0xf006440c,0xd2e,_DAT_ram_00014800);
  }
  if ((param_1 & 0xffff) == 1) {
    if (_DAT_ram_f006516c == 2) {
      FUN_ram_f00266d8(0xf0070168);
      FUN_ram_f0049b54(3);
    }
    else {
      (*(code *)&SUB_ram_0006aa24)(0,0x6f,0x20000000);
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0049c24 @ 0xf0049c24 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0049c24(byte param_1)

{
  DAT_ram_f0065041 = param_1 | DAT_ram_f0065041;
  if ((DAT_ram_f0065041 & 2) != 0) {
    FUN_ram_f00266d8(0xf0070198);
  }
  if ((DAT_ram_f0065041 & 0x20) != 0) {
    FUN_ram_f00266d8(0xf0070180);
  }
  if (_DAT_ram_f006516c == 0) {
    FUN_ram_f0049b54(2);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0049ce4 @ 0xf0049ce4 =====


void FUN_ram_f0049ce4(void)

{
  if (DAT_ram_f006504b != '\0') {
    DAT_ram_f006504b = '\0';
    FUN_ram_f004898c();
    FUN_ram_f0049c24(2);
    FUN_ram_f0049c24(0x20);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f0049de4 @ 0xf0049de4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f0049de4(void)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  
  FUN_ram_f00266a0(0xf0070198,&LAB_ram_f0049f44,1);
  puVar3 = &DAT_ram_f006facc;
  do {
    uVar2 = _DAT_ram_60205604;
    puVar3[1] = 0;
    *puVar3 = (char)uVar2;
    puVar3[2] = 0x3f;
    puVar3[3] = 0x20;
    puVar3 = puVar3 + 0x74;
  } while (puVar3 != (undefined1 *)0xf006fdf8);
  if (_DAT_ram_f006514c == 0x6625) {
    puVar4 = (undefined4 *)&DAT_ram_f00646a8;
    do {
      puVar1 = puVar4 + 1;
      puVar5 = (undefined4 *)*puVar4;
      puVar4 = puVar4 + 2;
      *puVar5 = *puVar1;
    } while (puVar4 != (undefined4 *)0xf0064790);
  }
  else {
    puVar4 = (undefined4 *)&DAT_ram_f00646a8;
    do {
      puVar5 = (undefined4 *)*puVar4;
      puVar1 = puVar4 + 1;
      puVar4 = puVar4 + 2;
      *puVar5 = *puVar1;
    } while (puVar4 != (undefined4 *)0xf0064790);
  }
  if (_DAT_ram_f006514c == 0x6625) {
    puVar4 = (undefined4 *)&DAT_ram_60205700;
    do {
      *puVar4 = 0x100;
      puVar4 = puVar4 + 1;
    } while (puVar4 != (undefined4 *)0x60205770);
    _DAT_ram_60205610 = _DAT_ram_60205610 & 0xffff7fff;
    DAT_ram_f006fe18 = (undefined1)_DAT_ram_60205604;
    DAT_ram_f006fe19 = 0;
    DAT_ram_f006fe1a = 0x3f;
    DAT_ram_f006fe1b = 0x20;
    DAT_ram_f006fe8f = 0x20;
    DAT_ram_f006fe8d = 0;
    DAT_ram_f006fe8e = 0x3f;
    DAT_ram_f006fe8c = DAT_ram_f006fe18;
  }
  FUN_ram_f0049074(2,0x3f,0x3f);
  DAT_ram_f0065052 = *(undefined1 *)(_DAT_ram_f00650ac + 0x27);
  DAT_ram_f0065053 = *(undefined1 *)(_DAT_ram_f00650ac + 0x28);
  FUN_ram_f0049c24(2);
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_txpwr.c:0x652 <<<
// ===== FUN_ram_f004a020 @ 0xf004a020 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: ram */

void FUN_ram_f004a020(char param_1,char param_2,char param_3,char param_4)

{
  bool bVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  int iVar5;
  undefined1 *puVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  uint uVar13;
  int iVar14;
  uint uVar15;
  int iVar16;
  undefined1 uStack_50;
  char local_4f;
  char local_4d;
  undefined1 local_43 [12];
  undefined1 local_37 [12];
  undefined1 local_2b [15];
  
  iVar14 = (int)param_1;
  iVar12 = (int)param_2;
  iVar9 = (int)param_3;
  iVar16 = (int)param_4;
  if (_DAT_ram_f006513c != 0x24ba58) {
    DAT_ram_f0064e60 = param_1;
    DAT_ram_f0064e61 = param_2;
    DAT_ram_f0064e62 = param_3;
    DAT_ram_f0064e63 = param_4;
    return;
  }
  if (DAT_ram_f0065039 == '\x01') {
    bVar1 = true;
  }
  else {
    bVar1 = DAT_ram_f0065039 == '\v';
    if (!bVar1) {
      if (DAT_ram_f0065039 == '\x05') {
        uVar8 = _DAT_ram_f0065140 ^ 3;
      }
      else {
        if (DAT_ram_f0065039 != '\a') {
          bVar2 = false;
          goto LAB_ram_f004a0ae;
        }
        uVar8 = _DAT_ram_f0065140 ^ 1;
      }
      bVar2 = uVar8 == 0;
      goto LAB_ram_f004a0ae;
    }
  }
  bVar2 = true;
LAB_ram_f004a0ae:
  DAT_ram_f0064e60 = param_1;
  DAT_ram_f0064e61 = param_2;
  DAT_ram_f0064e62 = param_3;
  DAT_ram_f0064e63 = param_4;
  iVar4 = FUN_ram_f0043ed0(0x24ba58,DAT_ram_f0065039,_DAT_ram_f0065140);
  iVar5 = _DAT_ram_f006513c;
  if (iVar4 == 0xe) {
    iVar5 = 0x25e720;
  }
  else {
    iVar4 = FUN_ram_f0043ed0(_DAT_ram_f006513c,DAT_ram_f0065039,_DAT_ram_f0065140,_DAT_ram_f006513c,
                             FUN_ram_f0043ed0);
    iVar5 = iVar5 + iVar4 * 5000;
  }
  iVar7 = (int)DAT_ram_f0065047;
  iVar4 = (int)DAT_ram_f0065046;
  uVar8 = iVar7 * (uint)(iVar7 < iVar4) | iVar4 * (uint)(iVar4 <= iVar7);
  uVar15 = iVar14 * (uint)(iVar14 < (int)uVar8) | uVar8 * ((int)uVar8 <= iVar14);
  if (bVar1) {
    iVar14 = (int)DAT_ram_f0065048;
    uVar15 = uVar15 * ((int)uVar15 < iVar14) | iVar14 * (uint)(iVar14 <= (int)uVar15);
  }
  uVar13 = iVar12 * (uint)(iVar12 < (int)uVar8) | uVar8 * ((int)uVar8 <= iVar12);
  if (bVar1) {
    iVar12 = (int)DAT_ram_f0065049;
    uVar13 = uVar13 * ((int)uVar13 < iVar12) | iVar12 * (uint)(iVar12 <= (int)uVar13);
  }
  uVar10 = iVar9 * (uint)(iVar9 < (int)uVar8) | uVar8 * ((int)uVar8 <= iVar9);
  if (bVar1) {
    iVar9 = (int)DAT_ram_f0065049;
    uVar10 = uVar10 * ((int)uVar10 < iVar9) | iVar9 * (uint)(iVar9 <= (int)uVar10);
  }
  if ((!bVar2) || (uVar11 = (int)DAT_ram_f006504a, (int)uVar10 <= (int)DAT_ram_f006504a)) {
    uVar11 = iVar16 * (uint)(iVar16 < (int)uVar8) | uVar8 * ((int)uVar8 <= iVar16);
  }
  iVar9 = FUN_ram_f0048a48(iVar5);
  if (iVar9 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf006440c,0x652,_DAT_ram_00014800);
  }
  iVar12 = (int)*(char *)(iVar9 + 0x73);
  cVar3 = *(char *)(iVar9 + 0x71);
  iVar9 = (int)*(char *)(iVar9 + 0x72);
  local_4f = (char)uVar15 + cVar3;
  uVar13 = uVar13 * ((int)uVar13 < iVar12) | iVar12 * (uint)(iVar12 <= (int)uVar13);
  uVar8 = uVar11 * ((int)uVar11 < iVar12) | iVar12 * (uint)(iVar12 <= (int)uVar11);
  uVar15 = uVar10 * ((int)uVar10 < iVar12) | iVar12 * (uint)(iVar12 <= (int)uVar10);
  if ((int)uVar13 < iVar9) {
    uVar13 = (uint)(char)(cVar3 + (char)uVar13);
  }
  if ((int)uVar15 < iVar9) {
    uVar15 = (uint)(char)(cVar3 + (char)uVar15);
  }
  if ((int)uVar8 < iVar9) {
    uVar8 = (uint)(char)(cVar3 + (char)uVar8);
  }
  puVar6 = local_43;
  do {
    *puVar6 = (char)uVar13;
    puVar6[0xc] = (char)uVar15;
    puVar6[0x18] = (char)uVar8;
    puVar6 = puVar6 + 2;
  } while (puVar6 != local_37);
  local_4d = local_4f;
  (*_DAT_ram_00014dcc)(1,&uStack_50,_DAT_ram_00014dcc);
  return;
}

// >>> MOD: wifi/mgmt/mt6582/rlm_txpwr.c:0x455 <<<
// ===== FUN_ram_f004a200 @ 0xf004a200 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_ram_f004a200(int param_1)

{
  char cVar1;
  char cVar2;
  char cVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  uint uVar9;
  undefined1 *puVar10;
  
  if (param_1 - 0x24cde0U < 0x11941) {
    if (param_1 == 0x25e720) {
      uVar9 = 0xe;
    }
    else {
      uVar9 = (param_1 - 0x24ba58U) / 5000 & 0xff;
      if (0xe < uVar9) {
        return (param_1 - 0x24ba58U) % 5000;
      }
    }
    iVar4 = FUN_ram_f00488dc(uVar9);
    DAT_ram_f0065047 = *(undefined1 *)(uVar9 + 0xf006d7b7);
    puVar5 = (undefined4 *)FUN_ram_f0048a48(param_1);
    if (DAT_ram_f006503c == '\0') {
      _DAT_ram_60205414 = 0x20222022;
    }
    else {
      _DAT_ram_60205414 = 0x1e201e20;
    }
    puVar10 = &DAT_ram_f0064e48;
  }
  else {
    _DAT_ram_60205414 = 0x1e1e1e1e;
    puVar5 = (undefined4 *)0xf006fe1c;
    if (param_1 == _DAT_ram_f0065130) {
      puVar5 = (undefined4 *)0xf006fda8;
    }
    iVar4 = 0;
    puVar10 = (undefined1 *)0xf0064e64;
  }
  if (puVar5 == (undefined4 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf006440c,0x455,_DAT_ram_00014800);
  }
  puVar7 = (undefined4 *)&DAT_ram_60205700;
  puVar6 = puVar5;
  do {
    uVar8 = *puVar6;
    puVar6 = puVar6 + 1;
    *puVar7 = uVar8;
    puVar7 = puVar7 + 1;
  } while (puVar7 != (undefined4 *)0x60205770);
  _DAT_ram_60205604 = (uint)*(byte *)(puVar5 + 0x1c) | _DAT_ram_60205604 & 0xffffff00;
  _DAT_ram_60205418 = _DAT_ram_60205418 & 0xffffff80 | 0x21;
  FUN_ram_f0049074(2,0x3f,(int)*(char *)((int)puVar5 + 0x72));
  _DAT_ram_60200824 = _DAT_ram_60200824 & 0xffff00ff | 0xe400;
  cVar1 = *(char *)((int)puVar5 + 0x73);
  cVar2 = *(char *)((int)puVar5 + 0x71);
  cVar3 = *(char *)((int)puVar5 + 0x72);
  FUN_ram_f004a020((int)DAT_ram_f0064e60,(int)DAT_ram_f0064e61,(int)DAT_ram_f0064e62,
                   (int)DAT_ram_f0064e63);
  FUN_ram_f00484ec(puVar10,1,(int)cVar2,(int)cVar3,(int)cVar1,0x3f);
  if (param_1 - 0x24cde0U < 0x11941) {
    _DAT_ram_60200824 = iVar4 - cVar2 & 0xffU | _DAT_ram_60200824 & 0xffffff00;
  }
  else {
    _DAT_ram_60200824 = _DAT_ram_60200824 & 0xffffff00 | 0xf8;
  }
  uVar9 = _DAT_ram_f00650a4;
  if (_DAT_ram_f00650a4 == 4) {
    uVar9 = FUN_ram_f0047000();
  }
  return uVar9;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004a410 @ 0xf004a410 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004a410(void)

{
  short sVar1;
  byte bVar2;
  int iVar3;
  undefined4 *puVar4;
  short *psVar5;
  undefined4 *puVar6;
  byte *pbVar7;
  ushort uVar8;
  undefined4 uVar9;
  
  FUN_ram_f00266a0(0xf0070168,&LAB_ram_f0049f44,0);
  iVar3 = _DAT_ram_f00650ac;
  DAT_ram_f0065047 = 0x3f;
  DAT_ram_f006504b = 0;
  _DAT_ram_f006516c = 0;
  _DAT_ram_f0065170 = 0;
  DAT_ram_f0065046 = 0x3f;
  DAT_ram_f0065041 = 0;
  DAT_ram_f0065048 = 0x3f;
  DAT_ram_f0065049 = 0x3f;
  DAT_ram_f006504a = 0x3f;
  DAT_ram_f0065054 = 0x3f;
  DAT_ram_f0065055 = 0x3f;
  if ((_DAT_ram_f00650ac == 0) || (*(char *)(_DAT_ram_f00650ac + 6) == '\0')) {
    if (_DAT_ram_f006514c == 0x6625) {
      _DAT_ram_f006506e = 0x41;
      _DAT_ram_f0065070 = 0x388;
    }
    else {
      _DAT_ram_f006506e = 0x40;
      _DAT_ram_f0065070 = 0x374;
    }
  }
  else {
    _DAT_ram_f006506e = *(undefined2 *)(_DAT_ram_f00650ac + 8);
    _DAT_ram_f0065070 = *(undefined2 *)(_DAT_ram_f00650ac + 10);
  }
  if (_DAT_ram_f006514c != 0x6625) goto LAB_ram_f004a60c;
  if ((_DAT_ram_f00650ac == 0) || (*(char *)(_DAT_ram_f00650ac + 0x2a) == '\0')) {
    puVar4 = (undefined4 *)&DAT_ram_f0063d30;
    puVar6 = (undefined4 *)&DAT_ram_f0070148;
    do {
      uVar9 = *puVar4;
      puVar4 = puVar4 + 3;
      *puVar6 = uVar9;
      puVar6 = puVar6 + 1;
    } while (puVar4 != (undefined4 *)&DAT_ram_f0063d90);
    if (iVar3 != 0) goto LAB_ram_f004a5ba;
  }
  else {
    _DAT_ram_f0070148 = *(undefined4 *)(_DAT_ram_f00650ac + 0x2c);
    _DAT_ram_f007014c =
         CONCAT22(*(undefined2 *)(_DAT_ram_f00650ac + 0x30),
                  *(undefined2 *)(_DAT_ram_f00650ac + 0x2c));
    _DAT_ram_f0070150 =
         CONCAT22(*(undefined2 *)(_DAT_ram_f00650ac + 0x30),
                  *(undefined2 *)(_DAT_ram_f00650ac + 0x2c));
    _DAT_ram_f0070154 =
         CONCAT22(*(undefined2 *)(_DAT_ram_f00650ac + 0x30),
                  *(undefined2 *)(_DAT_ram_f00650ac + 0x2c));
    _DAT_ram_f0070158 =
         CONCAT22(*(undefined2 *)(_DAT_ram_f00650ac + 0x32),
                  *(undefined2 *)(_DAT_ram_f00650ac + 0x2c));
    _DAT_ram_f007015c =
         CONCAT22(*(undefined2 *)(_DAT_ram_f00650ac + 0x32),
                  *(undefined2 *)(_DAT_ram_f00650ac + 0x2c));
    _DAT_ram_f0070160 =
         CONCAT22(*(undefined2 *)(_DAT_ram_f00650ac + 0x34),
                  *(undefined2 *)(_DAT_ram_f00650ac + 0x2c));
    _DAT_ram_f0070164 =
         CONCAT22(*(undefined2 *)(_DAT_ram_f00650ac + 0x34),
                  *(undefined2 *)(_DAT_ram_f00650ac + 0x2c));
LAB_ram_f004a5ba:
    if (*(char *)(iVar3 + 0x1e) != '\0') {
      DAT_ram_f006504c = 1;
      _DAT_ram_f0065072 = *(undefined2 *)(iVar3 + 0x20);
      _DAT_ram_f006fda4 = *(undefined2 *)(iVar3 + 0x22);
      _DAT_ram_f006fda6 = *(undefined2 *)(iVar3 + 0x24);
      goto LAB_ram_f004a60c;
    }
  }
  DAT_ram_f006504c = 0;
  _DAT_ram_f006fda6 = 0;
  _DAT_ram_f006fda4 = 0;
LAB_ram_f004a60c:
  psVar5 = (short *)&DAT_ram_f006fa34;
  DAT_ram_f0065050 = 0;
  DAT_ram_f0065043 = 0;
  _DAT_ram_f006fa34 = 0;
  _DAT_ram_f006fa36 = 0;
  _DAT_ram_f006fa38 = 0;
  if ((iVar3 != 0) && (*(char *)(iVar3 + 0x16) != '\0')) {
    pbVar7 = (byte *)(iVar3 + 0x12);
    do {
      bVar2 = *pbVar7;
      uVar8 = (ushort)bVar2;
      if ((char)bVar2 < '\0') {
        uVar8 = bVar2 | 0xff00;
      }
      sVar1 = uVar8 + *psVar5;
      *psVar5 = sVar1;
      if (sVar1 < 0x80) {
        if (sVar1 < -0x80) {
          *psVar5 = -0x80;
        }
      }
      else {
        *psVar5 = 0x7f;
      }
      psVar5 = psVar5 + 1;
      pbVar7 = pbVar7 + 1;
    } while (psVar5 != (short *)0xf006fa3a);
    bVar2 = *(byte *)(iVar3 + 0x15);
    if ((bVar2 & 4) == 0) {
      DAT_ram_f0065043 = bVar2 & 7;
    }
    else {
      DAT_ram_f0065043 = bVar2 | 0xf8;
    }
  }
  DAT_ram_f0065044 = 4;
  DAT_ram_f0065045 = 8;
  _DAT_ram_f006fa3c = 0;
  DAT_ram_f006fda2 = 0;
  _DAT_ram_f006fda0 = 0;
  _DAT_ram_f006fa40 = 0;
  _DAT_ram_60205600 = _DAT_ram_60205600 | 0x30;
  _DAT_ram_6020560c = 0x3f3f1c3f;
  _DAT_ram_60205404 = _DAT_ram_60205404 | 0x40000;
  _DAT_ram_60205414 = 0x20222022;
  _DAT_ram_60205418 = _DAT_ram_60205418 & 0xffffff80 | 0x21;
  FUN_ram_f004898c();
  FUN_ram_f0049c24(1);
  DAT_ram_f006504d = 7;
  DAT_ram_f006504e = 3;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004a784 @ 0xf004a784 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004a784(void)

{
  uint uVar1;
  
  *(undefined1 *)(_DAT_ram_020a0064 + 0x209a) = 0x1b;
  uVar1 = *(byte *)(_DAT_ram_020a0064 + 0x209a) & 2;
  if ((*(byte *)(_DAT_ram_020a0064 + 0x209a) & 2) != 0) {
    uVar1 = 1;
  }
  *(uint *)(_DAT_ram_020a0064 + 0x209c) = uVar1;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004a7ac @ 0xf004a7ac =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004a7ac(int param_1)

{
  if (*(char *)(param_1 + 0xb5) != '\0') {
    *(undefined1 *)(param_1 + 0xb5) = 0;
    if (*(char *)(param_1 + 0x346) == '\0') {
      _DAT_ram_60310000 = 0x800000;
      (*_DAT_ram_00014984)(0x800000,0,_DAT_ram_00014984);
    }
    else {
      _DAT_ram_60310004 = 0x400;
      _DAT_ram_6031000c = _DAT_ram_6031000c & 0xfffffbff;
    }
    (*_DAT_ram_00014b98)(*(undefined1 *)(param_1 + 0x344),0,_DAT_ram_00014b98);
    (*_DAT_ram_00014ba0)(*(undefined1 *)(param_1 + 0x344),0,_DAT_ram_00014ba0);
    (*_DAT_ram_00014b9c)(*(undefined1 *)(param_1 + 0x344),0,_DAT_ram_00014b9c);
    (*_DAT_ram_00014ba4)(*(undefined1 *)(param_1 + 0x344),0,_DAT_ram_00014ba4);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004a888 @ 0xf004a888 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004a888(int param_1)

{
  if (*(char *)(param_1 + 0x346) == '\0') {
    _DAT_ram_60310000 = 0x400800;
    if (*(int *)(param_1 + 0xc) != 1) {
      _DAT_ram_60310000 = 0x400000;
    }
    (*_DAT_ram_00014984)(_DAT_ram_60310000,0,_DAT_ram_00014984);
  }
  else {
    _DAT_ram_60310004 = 0x200;
    _DAT_ram_6031000c = _DAT_ram_6031000c & 0xfffffdff;
  }
  (*_DAT_ram_00014bbc)(*(undefined1 *)(param_1 + 0x346),0,_DAT_ram_00014bbc);
  (*_DAT_ram_00014bd0)(*(undefined1 *)(param_1 + 0x346),0,0,_DAT_ram_00014bd0);
  (*_DAT_ram_00014b78)
            (*(undefined1 *)(param_1 + 0x346),*(undefined2 *)(param_1 + 0x6a),
             *(undefined1 *)(param_1 + 0x72),0,0,0);
  if (*(int *)(param_1 + 0xc) - 1U < 2) {
    (*_DAT_ram_00014964)(0,_DAT_ram_00014964);
    (*_DAT_ram_00014c98)(1,_DAT_ram_00014c98);
    FUN_ram_f003718c(7,0x80);
  }
  return;
}

// >>> MOD: wifi/nic/nic.c:0x6fe,0x710 <<<
// ===== FUN_ram_f004aac8 @ 0xf004aac8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004aac8(int param_1)

{
  short sVar1;
  int iVar2;
  uint uVar3;
  short sVar4;
  
  if (param_1 == 0) {
    (*_DAT_ram_00014800)(0,0,0xf0064834,0x6fe,_DAT_ram_00014800);
  }
  (*_DAT_ram_00014cb0)
            (*(undefined1 *)(param_1 + 0x346),*(undefined1 *)(param_1 + 0x347),_DAT_ram_00014cb0);
  iVar2 = (*_DAT_ram_00014950)
                    (*(undefined1 *)(param_1 + 0x348),*(undefined1 *)(param_1 + 0x346),
                     _DAT_ram_00014950);
  if (iVar2 != 0) {
    (*_DAT_ram_00014800)(0,0,0xf0064834,0x710,_DAT_ram_00014800);
  }
  FUN_ram_f004bed0(7);
  (*_DAT_ram_00014b78)
            (*(undefined1 *)(param_1 + 0x346),*(undefined2 *)(param_1 + 0x6a),
             *(undefined1 *)(param_1 + 0x72),1,0,1);
  (*_DAT_ram_00014bd0)(*(undefined1 *)(param_1 + 0x346),1,1,_DAT_ram_00014bd0);
  (*_DAT_ram_00014c98)(5000,_DAT_ram_00014c98);
  *(undefined2 *)(param_1 + 0x34a) = 5000;
  FUN_ram_f002ad0c();
  (*_DAT_ram_00014bbc)(*(undefined1 *)(param_1 + 0x346),1,_DAT_ram_00014bbc);
  if (*(char *)(param_1 + 0x346) == '\0') {
    _DAT_ram_60310000 = 0x400800;
    if (*(int *)(param_1 + 0xc) != 1) {
      _DAT_ram_60310000 = 0x400000;
    }
    (*_DAT_ram_00014988)(_DAT_ram_60310000,0,_DAT_ram_00014988);
  }
  else {
    _DAT_ram_60310004 = 0x200;
    _DAT_ram_6031000c = _DAT_ram_6031000c | 0x200;
  }
  uVar3 = 10000 / *(ushort *)(param_1 + 0x6a);
  sVar1 = 0;
  do {
    sVar4 = sVar1;
    uVar3 = uVar3 >> 1;
    sVar1 = sVar4 + 1;
  } while (uVar3 != 0);
  *(short *)(param_1 + 0x6c) = sVar4;
  return;
}

// >>> MOD: wifi/nic/nic.c:0x56b <<<
// ===== FUN_ram_f004ac48 @ 0xf004ac48 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004ac48(int param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  
  (*_DAT_ram_00014cb0)
            (*(undefined1 *)(param_1 + 0x346),*(undefined1 *)(param_1 + 0x347),_DAT_ram_00014cb0);
  iVar3 = *(int *)(param_1 + 0xc);
  uVar1 = 1;
  if (iVar3 == 1) {
    *(undefined1 *)(param_1 + 0x75) = 1;
    uVar1 = 0;
LAB_ram_f004ac90:
    *(undefined1 *)(param_1 + 0x74) = uVar1;
  }
  else {
    if (iVar3 == 0) {
      if ((uint)*(byte *)(param_1 + 0x72) * (uint)*(ushort *)(param_1 + 0x6a) < 0x7d1) {
        *(undefined1 *)(param_1 + 0x75) = 0;
        goto LAB_ram_f004ac90;
      }
    }
    else if (iVar3 != 2) {
      (*_DAT_ram_00014800)(0,0,0xf0064834,0x56b,_DAT_ram_00014800);
      goto LAB_ram_f004acb4;
    }
    *(undefined1 *)(param_1 + 0x74) = 1;
    *(undefined1 *)(param_1 + 0x75) = 1;
  }
LAB_ram_f004acb4:
  if (*(short *)(param_1 + 0x6a) == 0) {
    *(undefined2 *)(param_1 + 0x6a) = 100;
  }
  (*_DAT_ram_00014b78)
            (*(undefined1 *)(param_1 + 0x346),*(undefined2 *)(param_1 + 0x6a),
             *(undefined1 *)(param_1 + 0x72),*(undefined1 *)(param_1 + 0x75),
             *(undefined1 *)(param_1 + 0x74),1);
  (*_DAT_ram_00014bd0)(*(undefined1 *)(param_1 + 0x346),1,1,_DAT_ram_00014bd0);
  (*_DAT_ram_00014b94)(1,_DAT_ram_00014b94);
  FUN_ram_f002ad0c();
  (*_DAT_ram_00014bbc)(*(undefined1 *)(param_1 + 0x346),1,_DAT_ram_00014bbc);
  if (*(char *)(param_1 + 0x346) == '\0') {
    uVar2 = 0x400000;
    uVar4 = 0;
  }
  else {
    if (*(char *)(param_1 + 0x346) != '\x01') {
      return;
    }
    uVar4 = 0x200;
    uVar2 = 0;
  }
  (*_DAT_ram_00014984)(uVar2,uVar4,_DAT_ram_00014984);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004aea8 @ 0xf004aea8 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004aea8(int param_1)

{
  code *pcVar1;
  int iVar2;
  undefined4 uVar3;
  
  if (*(char *)(param_1 + 0xb5) == '\0') {
    *(undefined1 *)(param_1 + 0xb5) = 1;
    (*_DAT_ram_00014c6c)
              (*(undefined1 *)(param_1 + 0x27c),*(undefined1 *)(param_1 + 0x27d),
               *(undefined1 *)(param_1 + 0x27e),*(undefined2 *)(param_1 + 0x280),
               *(undefined1 *)(param_1 + 0x79),_DAT_ram_00014c6c);
    (*_DAT_ram_00014b98)(*(undefined1 *)(param_1 + 0x344),1,_DAT_ram_00014b98);
    pcVar1 = _DAT_ram_00014c70;
    iVar2 = FUN_ram_f0020000();
    if (iVar2 == 0) {
      uVar3 = 100;
    }
    else {
      uVar3 = 0x14;
    }
    (*pcVar1)(1,5,1,uVar3,0);
    (*_DAT_ram_00014ba0)(*(undefined1 *)(param_1 + 0x344),1,_DAT_ram_00014ba0);
    (*_DAT_ram_00014b9c)(*(undefined1 *)(param_1 + 0x344),1,_DAT_ram_00014b9c);
    (*_DAT_ram_00014ba4)(*(undefined1 *)(param_1 + 0x344),1,_DAT_ram_00014ba4);
    if (*(char *)(param_1 + 0x344) == '\0') {
      _DAT_ram_60310000 = 0x800000;
      (*_DAT_ram_00014988)(0x800000,0,_DAT_ram_00014988);
    }
    else {
      _DAT_ram_60310004 = 0x400;
      _DAT_ram_6031000c = _DAT_ram_6031000c | 0x400;
    }
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004afa4 @ 0xf004afa4 =====


/* WARNING: Removing unreachable block (ram,0xf004b0f0) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004afa4(int param_1)

{
  undefined4 uVar1;
  
  *(undefined4 *)(param_1 + 0x4c) = 0x209fefc;
  *(undefined4 *)(param_1 + 0x50) = 0x209ff74;
  *(undefined4 *)(param_1 + 0x54) = 0x209ff9c;
  *(undefined4 *)(param_1 + 0x58) = 0x20a003c;
  *(undefined4 *)(param_1 + 0x5c) = 0x20a21f8;
  *(undefined4 *)(param_1 + 0x60) = _DAT_ram_020a2628;
  *(undefined4 *)(param_1 + 100) = _DAT_ram_020a262c;
  *(undefined4 *)(param_1 + 0x68) = _DAT_ram_020a2630;
  *(undefined4 *)(param_1 + 0x6c) = 0x20a2470;
  *(undefined4 *)(param_1 + 0x70) = 0x20a2638;
  *(undefined4 *)(param_1 + 0x74) = _DAT_ram_020a2634;
  uVar1 = _DAT_ram_020a27f0;
  *(undefined4 *)(param_1 + 0x24) = 0x20a21f8;
  *(undefined4 *)(param_1 + 0x78) = uVar1;
  *(undefined1 **)(param_1 + 0x7c) = &DAT_ram_020a2628;
  *(undefined4 *)(param_1 + 0x80) = 0x20a2470;
  *(undefined4 *)(param_1 + 0x84) = 0x20a2460;
  *(undefined4 *)(param_1 + 0x88) = 0x20a27f4;
  *(undefined4 *)(param_1 + 0x8c) = 0x20a281c;
  *(undefined4 *)(param_1 + 0x90) = 0x20a2894;
  *(undefined4 *)(param_1 + 0x94) = 0x20a2934;
  *(undefined1 *)(param_1 + 0x98) = 0x1e;
  *(undefined1 *)(param_1 + 0x99) = 10;
  *(undefined1 *)(param_1 + 0x9a) = 0x28;
  *(undefined1 *)(param_1 + 0x9b) = 1;
  *(undefined4 *)(param_1 + 0x20) = _DAT_ram_020a2628;
  *(undefined2 *)(param_1 + 8) = 0x50;
  *(undefined4 *)(param_1 + 0xa0) = 0x6a000;
  *(undefined4 *)(param_1 + 0xa4) = 0x1794;
  *(undefined1 **)(param_1 + 0xa8) = &DAT_ram_0209f800;
  *(undefined4 *)(param_1 + 0xac) = 0x324c;
  *(undefined4 *)(param_1 + 0xb4) = 0x2318;
  *(undefined4 *)(param_1 + 0xb0) = 0x200000;
  *_DAT_ram_0209fef0 = 5000;
  (*_DAT_ram_00014e80)(0x800,0x80,0,_DAT_ram_00014e80);
  return;
}

// >>> MOD: wifi/nic/nic_dma.c:0x143 <<<
// ===== FUN_ram_f004b140 @ 0xf004b140 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_ram_f004b140(int *param_1)

{
  int iVar1;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0064858,0x143,_DAT_ram_00014800);
  }
  iVar1 = *(int *)param_1[1];
  if (iVar1 == *param_1) {
    iVar1 = 0;
  }
  else {
    param_1[1] = iVar1;
    if (*param_1 == 0) {
      *param_1 = iVar1;
    }
    param_1[2] = param_1[2] + 1;
  }
  return iVar1;
}

// >>> MOD: wifi/nic/nic_dma.c:0x164 <<<
// ===== FUN_ram_f004b180 @ 0xf004b180 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * FUN_ram_f004b180(int *param_1)

{
  int *piVar1;
  int iVar2;
  
  if (param_1 == (int *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0064858,0x164,_DAT_ram_00014800);
  }
  piVar1 = (int *)*param_1;
  if (((piVar1 == (int *)0x0) || ((piVar1[1] & 1U) != 0)) || ((piVar1[1] & 2U) != 0)) {
    piVar1 = (int *)0x0;
  }
  else {
    if ((int *)param_1[1] == piVar1) {
      iVar2 = 0;
    }
    else {
      iVar2 = *piVar1;
    }
    *param_1 = iVar2;
    param_1[2] = param_1[2] + -1;
  }
  return piVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004b1d0 @ 0xf004b1d0 =====


void FUN_ram_f004b1d0(void)

{
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004b1d4 @ 0xf004b1d4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004b1d4(void)

{
  int iVar1;
  
  iVar1 = 0;
  do {
    (*_DAT_ram_00014b3c)(iVar1,0,_DAT_ram_020a0078,_DAT_ram_00014b3c);
    iVar1 = iVar1 + 1;
  } while (iVar1 != 9);
  return;
}

// >>> MOD: wifi/nic/nic_dma.c:0x499,0x4a8 <<<
// ===== FUN_ram_f004b1fc @ 0xf004b1fc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f004b1fc(int param_1,uint param_2,undefined4 *param_3)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  if (1 < (param_2 & 0xff)) {
    (*_DAT_ram_00014800)(0,0,0xf0064858,0x499,_DAT_ram_00014800);
  }
  puVar2 = (undefined4 *)0x0;
  puVar3 = (undefined4 *)0x0;
  while (puVar4 = puVar3, iVar1 = FUN_ram_f004b180(param_1 + (param_2 & 0xff) * 0xc), iVar1 != 0) {
    puVar3 = *(undefined4 **)(iVar1 + 0x14);
    *(byte *)(iVar1 + 4) = *(byte *)(iVar1 + 4) | 2;
    if (puVar3 == (undefined4 *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf0064858,0x4a8,_DAT_ram_00014800);
    }
    *puVar3 = 0;
    if (puVar2 == (undefined4 *)0x0) {
      puVar2 = puVar3;
    }
    if (puVar4 != (undefined4 *)0x0) {
      *puVar4 = puVar3;
    }
  }
  if ((puVar2 != (undefined4 *)0x0) && (param_3 != (undefined4 *)0x0)) {
    *param_3 = puVar2;
  }
  return 0;
}

// >>> MOD: wifi/nic/nic_dma.c:0x4dc,0x4e1,0x4e5 <<<
// ===== FUN_ram_f004b290 @ 0xf004b290 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f004b290(int param_1,uint param_2,undefined4 *param_3)

{
  char *pcVar1;
  int iVar2;
  int iVar3;
  char cVar4;
  
  param_2 = param_2 & 0xff;
  if (1 < param_2) {
    (*_DAT_ram_00014800)(0,0,0xf0064858,0x4dc,_DAT_ram_00014800);
  }
  for (; param_3 != (undefined4 *)0x0; param_3 = (undefined4 *)*param_3) {
    iVar2 = FUN_ram_f004b140(param_1 + param_2 * 0xc);
    if (iVar2 == 0) {
      (*_DAT_ram_00014800)(0,0,0xf0064858,0x4e1,_DAT_ram_00014800);
    }
    iVar3 = param_3[2];
    if ((iVar3 + 0x30U & 1) != 0) {
      (*_DAT_ram_00014800)(0,0,0xf0064858,0x4e5,_DAT_ram_00014800);
    }
    *(uint *)(iVar2 + 0xc) = iVar3 + 0x30U;
    *(undefined4 **)(iVar2 + 0x14) = param_3;
    if (*(char *)(param_1 + 0x34) != '\0') {
      cVar4 = '\0';
      iVar3 = 2;
      do {
        pcVar1 = (char *)(iVar2 + 4 + iVar3);
        iVar3 = iVar3 + 1;
        cVar4 = cVar4 + *pcVar1;
      } while (iVar3 != 0x10);
      *(byte *)(iVar2 + 5) = ~(cVar4 + 1U);
    }
    *(undefined1 *)(iVar2 + 4) = 1;
    (*_DAT_ram_000149d4)(param_1,param_2,*(undefined1 *)((int)param_3 + 0xe),_DAT_ram_000149d4);
  }
  return 0;
}

// >>> MOD: wifi/nic/nic_dma.c:0x1ca,0x1ef,0x1fa <<<
// ===== FUN_ram_f004b3d4 @ 0xf004b3d4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004b3d4(char param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  
  puVar1 = (undefined4 *)FUN_ram_f0025b14();
  if (puVar1 != (undefined4 *)0x0) {
    iVar4 = 0;
    *(undefined1 *)(puVar1 + 3) = 0;
    *(undefined1 *)((int)puVar1 + 0xe) = 0xff;
    puVar3 = puVar1;
    do {
      puVar2 = (undefined4 *)FUN_ram_f0025b14();
      iVar4 = iVar4 + 1;
      if (puVar2 == (undefined4 *)0x0) {
        (*_DAT_ram_00014800)(0,0,0xf0064858,0x1ca,_DAT_ram_00014800);
      }
      *(undefined1 *)((int)puVar2 + 0xe) = 0xff;
      *(undefined1 *)(puVar2 + 3) = 0;
      *puVar3 = puVar2;
      puVar3 = puVar2;
    } while (iVar4 != 0x1b);
    *puVar2 = 0;
    puVar3 = puVar1;
    if (param_1 != '\0') {
      do {
        puVar3[2] = 0x202318;
        puVar3 = (undefined4 *)*puVar3;
      } while (puVar3 != (undefined4 *)0x0);
    }
    FUN_ram_f004b290(_DAT_ram_0209fed8,0,puVar1);
    puVar1 = (undefined4 *)FUN_ram_f0025b14();
    if (puVar1 == (undefined4 *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf0064858,0x1ef,_DAT_ram_00014800);
    }
    *(undefined1 *)((int)puVar1 + 0xe) = 0xff;
    *(undefined1 *)(puVar1 + 3) = 1;
    iVar4 = 0;
    puVar3 = puVar1;
    do {
      puVar2 = (undefined4 *)FUN_ram_f0025b14();
      iVar4 = iVar4 + 1;
      if (puVar2 == (undefined4 *)0x0) {
        (*_DAT_ram_00014800)(0,0,0xf0064858,0x1fa,_DAT_ram_00014800);
      }
      *(undefined1 *)((int)puVar2 + 0xe) = 0xff;
      *(undefined1 *)(puVar2 + 3) = 1;
      *puVar3 = puVar2;
      puVar3 = puVar2;
    } while (iVar4 != 3);
    *puVar2 = 0;
    if (param_1 != '\0') {
      iVar4 = 0x202318;
      for (puVar3 = puVar1; iVar4 = iVar4 + 0x680, puVar3 != (undefined4 *)0x0;
          puVar3 = (undefined4 *)*puVar3) {
        puVar3[2] = iVar4;
      }
    }
    FUN_ram_f004b290(_DAT_ram_0209fed8,1,puVar1);
  }
  return;
}

// >>> MOD: wifi/nic/nic_dma.c:0x2f6,0x310 <<<
// ===== FUN_ram_f004b520 @ 0xf004b520 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004b520(char param_1)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *unaff_s1;
  int iVar4;
  
  puVar1 = (undefined4 *)FUN_ram_f0025b14();
  if (puVar1 == (undefined4 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0064858,0x2f6,_DAT_ram_00014800);
  }
  if (param_1 == '\0') {
    iVar2 = 0xc;
  }
  else {
    iVar2 = 8;
  }
  *(undefined1 *)(puVar1 + 3) = 2;
  puVar3 = puVar1;
  for (iVar4 = 0; iVar4 < iVar2 + -1; iVar4 = iVar4 + 1) {
    unaff_s1 = (undefined4 *)FUN_ram_f0025b14();
    if (unaff_s1 == (undefined4 *)0x0) {
      (*_DAT_ram_00014800)(0,0,0xf0064858,0x310,_DAT_ram_00014800);
    }
    *(undefined1 *)(unaff_s1 + 3) = 2;
    *puVar3 = unaff_s1;
    puVar3 = unaff_s1;
  }
  *unaff_s1 = 0;
  if (param_1 != '\0') {
    iVar2 = 0x20c098;
    for (puVar3 = puVar1; puVar3 != (undefined4 *)0x0; puVar3 = (undefined4 *)*puVar3) {
      puVar3[2] = iVar2;
      iVar2 = iVar2 + 0x1080;
    }
  }
  (*(code *)&SUB_ram_0006ae98)(puVar1);
  (*_DAT_ram_00014b00)(0,_DAT_ram_00014b00);
  return;
}

// >>> MOD: wifi/nic/nic_dma.c:0x24f,0x275 <<<
// ===== FUN_ram_f004b5e4 @ 0xf004b5e4 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004b5e4(int param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  
  if (param_2 == (undefined4 *)0x0) {
    (*_DAT_ram_00014800)(0,0,0xf0064858,0x24f,_DAT_ram_00014800);
  }
  if (param_1 == 0) {
    FUN_ram_f0037f14(param_2);
  }
  else {
    do {
      puVar1 = (undefined4 *)*param_2;
      *param_2 = 0;
      if (*(byte *)(param_2[2] + 0x33) >> 6 == 1) {
        if (param_2[2] == 0) {
          (*_DAT_ram_00014800)(0,0,0xf0064858,0x275,_DAT_ram_00014800);
        }
        *(char *)((int)param_2 + 0xe) = (char)((*(byte *)(param_2[2] + 0x33) & 0x3c) >> 2);
        FUN_ram_f0028190(param_2);
      }
      else {
        FUN_ram_f0037f14(param_2);
      }
      param_2 = puVar1;
    } while (puVar1 != (undefined4 *)0x0);
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004b684 @ 0xf004b684 =====


void FUN_ram_f004b684(int param_1)

{
  *(undefined1 *)(param_1 + 0x28a) = 0x7f;
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004b690 @ 0xf004b690 =====


undefined1 FUN_ram_f004b690(char param_1,char param_2,byte param_3)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = (uint)param_3;
  if (param_2 != '\0') {
    return 0;
  }
  if (param_1 == '\0') {
    if (5 < uVar2) {
      return 0x7f;
    }
    iVar1 = -0xff9b774;
  }
  else {
    if (param_1 != '\x01') {
      return 0x7f;
    }
    if (3 < uVar2) {
      return 0x7f;
    }
    iVar1 = -0xff9b76c;
  }
  return *(undefined1 *)(iVar1 + uVar2);
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004b6cc @ 0xf004b6cc =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f004b6cc(byte param_1)

{
  bool bVar1;
  uint uVar2;
  uint uVar3;
  undefined1 auStack_58 [32];
  undefined1 auStack_38 [12];
  undefined1 auStack_2c [12];
  undefined4 local_20;
  undefined2 local_1c;
  
  if (param_1 < 0x14) {
    (*(code *)&SUB_ram_0006a0a0)(auStack_58,0);
    (*(code *)&SUB_ram_0006a0a0)(auStack_38,0,10);
    (*(code *)&SUB_ram_0006a0a0)(auStack_2c,0,10);
    local_20 = 0;
    local_1c = 0;
    uVar2 = (*_DAT_ram_00014e4c)(param_1,auStack_2c,&local_20,auStack_38,_DAT_ram_00014e4c);
    uVar3 = (*_DAT_ram_00014e78)(param_1,auStack_58,_DAT_ram_00014e78);
    bVar1 = (uVar3 & uVar2) == 0;
  }
  else {
    bVar1 = false;
  }
  return bVar1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004b748 @ 0xf004b748 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f004b748(undefined1 param_1,undefined1 param_2)

{
  int iVar1;
  int iVar2;
  undefined1 auStack_44 [12];
  undefined1 auStack_38 [12];
  undefined4 local_2c;
  undefined2 local_28;
  undefined4 local_24 [2];
  
  (*(code *)&SUB_ram_0006a0a0)(auStack_38,0,10);
  local_2c = 0;
  local_28 = 0;
  (*(code *)&SUB_ram_0006a0a0)(auStack_44,0,10);
  local_24[0] = 0;
  (*_DAT_ram_00014e54)(param_2,auStack_38,&local_2c,auStack_44,_DAT_ram_00014e54);
  (*_DAT_ram_00014e68)(param_2,local_24,_DAT_ram_00014e68);
  iVar1 = (*_DAT_ram_00014e4c)(param_1,auStack_38,&local_2c,auStack_44,_DAT_ram_00014e4c);
  iVar2 = (*_DAT_ram_00014e7c)(param_1,local_24,_DAT_ram_00014e7c);
  return iVar2 == 0 && iVar1 == 0;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004b7e0 @ 0xf004b7e0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004b7e0(byte param_1,char param_2)

{
  uint uVar1;
  int iVar2;
  undefined1 auStack_60 [32];
  undefined1 auStack_40 [3];
  undefined1 local_3d;
  undefined1 auStack_34 [12];
  undefined1 auStack_28 [12];
  
  iVar2 = _DAT_ram_020a0064;
  uVar1 = (uint)param_1;
  if (uVar1 < 0x14) {
    if (param_2 != '\0') {
      (*_DAT_ram_00014e54)(uVar1,auStack_34,auStack_28,auStack_40,_DAT_ram_00014e54);
      local_3d = 0;
      (*(code *)&SUB_ram_0006a0a0)(auStack_60,0,0x20);
      (*_DAT_ram_00014e4c)(uVar1,auStack_34,auStack_28,auStack_40,_DAT_ram_00014e4c);
      (*_DAT_ram_00014e78)(uVar1,auStack_60,_DAT_ram_00014e78);
    }
    iVar2 = (uVar1 + 0x417) * 8 + iVar2;
    *(undefined1 *)(iVar2 + 5) = 3;
    *(undefined1 *)(iVar2 + 4) = 0;
    *(undefined4 *)(iVar2 + 8) = 0;
  }
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004b86c @ 0xf004b86c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_ram_f004b86c(int param_1,byte param_2,int param_3)

{
  char cVar1;
  bool bVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  undefined1 auStack_68 [16];
  undefined1 auStack_58 [8];
  undefined1 auStack_50 [8];
  bool local_48 [5];
  char local_43;
  undefined1 local_42;
  undefined1 local_41;
  undefined1 local_40;
  byte local_3f;
  int local_3c;
  char local_38;
  char local_37;
  undefined1 local_33;
  undefined1 auStack_30 [8];
  undefined4 uStack_28;
  undefined2 local_24;
  
  uVar4 = (uint)param_2;
  uStack_28 = _DAT_ram_f0064884;
  local_24 = (undefined2)_DAT_ram_f0064888;
  if (0x13 < uVar4) {
    return false;
  }
  if (*(int *)(*(int *)(&DAT_ram_020a0068 + (uint)*(byte *)(param_1 + 10) * 4) + 0x10) == 5) {
    iVar5 = 0x10;
    iVar6 = 0x10;
  }
  else if (*(char *)(param_1 + 3) == '\0') {
    iVar6 = 0x18;
    iVar5 = 0x10;
  }
  else {
    iVar6 = 0x10;
    iVar5 = 0x18;
  }
  if (*(char *)(param_1 + 0xb) == '\x02') {
    if (uVar4 == 0) {
      (*(code *)&SUB_ram_0006a08c)(_DAT_ram_020a0064 + 0x12ec,param_1 + 0x10 + iVar5,8);
      param_3 = _DAT_ram_020a0064 + 0x12e4;
    }
    else {
      if (param_3 == 0) goto LAB_ram_f004b932;
      (*(code *)&SUB_ram_0006a08c)(param_3 + 0x34d,param_1 + 0x10 + iVar5,8);
      param_3 = param_3 + 0x345;
    }
    (*(code *)&SUB_ram_0006a08c)(param_3,param_1 + 0x10 + iVar6,8);
  }
LAB_ram_f004b932:
  (*_DAT_ram_00014e54)(uVar4,&local_3c,auStack_30,local_48,_DAT_ram_00014e54);
  local_48[2] = false;
  if (*(byte *)(param_1 + 0xb) < 9) {
    local_3f = *(byte *)(param_1 + 0xb);
  }
  iVar3 = param_1 + 0x10;
  local_41 = *(undefined1 *)(param_1 + 0xc);
  (*(code *)&SUB_ram_0006a08c)(&local_3c,param_1 + 4,6);
  (*(code *)&SUB_ram_0006a08c)(auStack_68,iVar3,0x10);
  (*(code *)&SUB_ram_0006a08c)(auStack_58,iVar3 + iVar5,8);
  (*(code *)&SUB_ram_0006a08c)(auStack_50,iVar3 + iVar6,8);
  cVar1 = *(char *)(param_1 + 1);
  local_42 = 1;
  if (cVar1 == '\0') {
    bVar2 = *(char *)(param_1 + 10) == '\x01';
    local_43 = cVar1;
    if (bVar2) {
      (*(code *)&SUB_ram_0006a08c)(&local_3c,_DAT_ram_020a006c + 0x4b,6);
    }
    local_40 = 0;
    local_48[0] = bVar2;
  }
  else {
    iVar5 = (*(code *)&SUB_ram_0006a074)(param_1 + 4,&uStack_28,6);
    cVar1 = *(char *)(param_1 + 10);
    if (iVar5 == 0) {
      local_48[0] = false;
      if (cVar1 == '\x01') {
        (*(code *)&SUB_ram_0006a08c)(&local_3c,_DAT_ram_020a006c + 0x4b,6);
      }
      local_43 = '\0';
      local_48[0] = cVar1 == '\x01';
    }
    else {
      if (cVar1 == '\0') {
        (*_DAT_ram_00014d94)(0x36,_DAT_ram_00014d94);
      }
      local_48[0] = true;
      if ((*(char *)(param_1 + 2) != '\0') && (local_43 = '\x01', *(char *)(param_1 + 10) == '\x02')
         ) {
        local_43 = '\0';
      }
    }
    if (*(char *)(param_1 + 10) == '\x01') {
      local_40 = 0x21;
    }
    else {
      local_40 = 0x20;
    }
  }
  local_48[3] = true;
  local_48[4] = true;
  local_48[1] = true;
  iVar5 = *(int *)(_DAT_ram_020a0064 + (uVar4 + 0x417) * 8 + 8);
  if (iVar5 != 0) {
    if (*(char *)(iVar5 + 0x37) == '\0') {
      local_33 = 0;
    }
    else {
      local_33 = 1;
    }
  }
  if (((*(char *)(param_1 + 10) == '\0') &&
      (((cVar1 = *(char *)(param_1 + 0xb), cVar1 == '\x01' || (cVar1 == '\x05')) || (cVar1 == '\a'))
      )) && (((local_3c == -1 && (local_38 == -1)) && (local_37 == -1)))) {
    local_48[1] = false;
  }
  iVar5 = (*_DAT_ram_00014e4c)(uVar4,&local_3c,auStack_30,local_48,_DAT_ram_00014e4c);
  iVar6 = (*_DAT_ram_00014e78)(uVar4,auStack_68,_DAT_ram_00014e78);
  (*_DAT_ram_00014e54)(uVar4,&local_3c,auStack_30,local_48,_DAT_ram_00014e54);
  return iVar6 == 0 && iVar5 == 0;
}

// >>> MOD: wifi/nic/nic_privacy.c:0x288 <<<
// ===== FUN_ram_f004bb2c @ 0xf004bb2c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f004bb2c(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  undefined1 auStack_60 [32];
  undefined1 local_40;
  undefined1 local_3f;
  undefined1 auStack_34 [6];
  undefined1 local_2e;
  undefined4 local_28;
  undefined2 local_24;
  
  uVar4 = (uint)*(byte *)(param_1 + 0x14);
  iVar1 = *(int *)(&DAT_ram_020a0068 + uVar4 * 4);
  if ((uVar4 == 0) && (*(int *)(param_1 + 0x10) == 0x41)) {
    iVar2 = (*(code *)&SUB_ram_0006a074)(param_1 + 10,_DAT_ram_020a0064 + 0xfb3,6);
    if (iVar2 != 0) {
      return 0;
    }
    (*_DAT_ram_00014d94)(0x36,_DAT_ram_00014d94);
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
    (*_DAT_ram_00014e54)(uVar6,auStack_34,&local_28,&local_40,_DAT_ram_00014e54);
    iVar2 = (*(code *)&SUB_ram_0006a074)(param_1 + 10,auStack_34,6);
    uVar5 = uVar6;
    if (iVar2 == 0) goto LAB_ram_f004bc10;
  }
  uVar5 = 0x7f;
LAB_ram_f004bc10:
  if (uVar6 == uVar4 + 1) {
    while( true ) {
      if (uVar4 < uVar3) break;
      if (*(char *)(_DAT_ram_020a0064 + (uVar3 + 0x417) * 8 + 4) == '\0') goto LAB_ram_f004bc46;
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
  (*(code *)&SUB_ram_0006a0a0)(auStack_60,0,0x20);
  if (*(int *)(param_1 + 0x10) == 0x11) {
    if (*(int *)(iVar1 + 0x14) == 1) {
      *(undefined1 *)(param_1 + 0x289) = 0;
      (*_DAT_ram_00014e54)(0,auStack_34,&local_28,&local_40,_DAT_ram_00014e54);
      (*(code *)&SUB_ram_0006a0a0)(&local_40,0,10);
    }
    else {
      *(undefined1 *)(param_1 + 0x289) = 1;
      (*_DAT_ram_00014e58)(0,auStack_60,&local_28,auStack_34,_DAT_ram_00014e58);
      (*_DAT_ram_00014e54)(0,auStack_34,&local_28,&local_40,_DAT_ram_00014e54);
    }
  }
  else {
    (*_DAT_ram_00014e58)(uVar3,auStack_60,_DAT_ram_00014e58);
    (*_DAT_ram_00014e54)(uVar3,auStack_34,&local_28,&local_40,_DAT_ram_00014e54);
    if (((*(int *)(param_1 + 0x10) == 0x41) && (*(int *)(_DAT_ram_020a0064 + 0xf7c) == 0)) &&
       (*(char *)(_DAT_ram_020a0064 + 0x12e3) != '\0')) {
      local_40 = 1;
    }
  }
  (*(code *)&SUB_ram_0006a08c)(auStack_34,param_1 + 10,6);
  iVar2 = _DAT_ram_020a0064;
  local_2e = 0;
  local_24 = 0;
  local_28 = 0;
  iVar1 = *(int *)(iVar1 + 0x10);
  if (((iVar1 == 0) || (iVar1 == 2)) &&
     ((*(char *)(param_1 + 0x14) == '\0' && (iVar1 = FUN_ram_f002a480(), iVar1 == 0)))) {
    local_3f = 1;
  }
  iVar1 = (*_DAT_ram_00014e4c)(uVar3,auStack_34,&local_28,&local_40,_DAT_ram_00014e4c);
  if (iVar1 != 0) {
    (*_DAT_ram_00014800)(0,0,0xf006486c,0x288,_DAT_ram_00014800);
    return 0;
  }
  iVar2 = iVar2 + (uVar3 + 0x417) * 8;
  *(undefined1 *)(iVar2 + 4) = 1;
  *(int *)(iVar2 + 8) = param_1;
  *(undefined1 *)(iVar2 + 5) = *(undefined1 *)(param_1 + 0x14);
  *(char *)(param_1 + 0x28a) = (char)uVar3;
  if ((*(char *)(param_1 + 0x14) == '\0') && (iVar1 = FUN_ram_f002a480(), iVar1 == 0)) {
    (*_DAT_ram_00014e78)(uVar3,auStack_60,_DAT_ram_00014e78);
  }
  return 1;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004be1c @ 0xf004be1c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f004be1c(char param_1,char param_2)

{
  if (param_1 == '\0') {
    _DAT_ram_6016005c = _DAT_ram_6016005c & 0xfff7ffff;
  }
  else {
    _DAT_ram_6016005c = _DAT_ram_6016005c | 0x80000;
  }
  if (param_2 == '\0') {
    _DAT_ram_60150008 = _DAT_ram_60150008 & 0x7fffffff;
  }
  else {
    _DAT_ram_6016005c = _DAT_ram_6016005c | 0x80000;
    _DAT_ram_60150008 = _DAT_ram_60150008 | 0x80000000;
  }
  return 0;
}

// >>> MOD: wifi/nic/HAL/mt6582/hal.c:0x143 <<<
// ===== FUN_ram_f004be5c @ 0xf004be5c =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_ram_f004be5c(void)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = 0;
  do {
    _DAT_ram_6032ff14 = iVar1 << 0x18 | 0x30000;
    _DAT_ram_6032ff10 = 0;
    uVar2 = 0;
    while ((_DAT_ram_6032ff14 & 0x10000) != 0) {
      uVar2 = uVar2 + 1;
      (*_DAT_ram_0001480c)(4,_DAT_ram_0001480c);
      if (10000 < uVar2) {
        (*_DAT_ram_00014800)(0,0,0xf0064898,0x143,_DAT_ram_00014800);
      }
    }
    iVar1 = iVar1 + 1;
  } while (iVar1 != 0x20);
  return;
}

// >>> MOD: (sin ASSERT) <<<
// ===== FUN_ram_f004bed0 @ 0xf004bed0 =====


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_ram_f004bed0(byte param_1,ushort *param_2)

{
  uint uVar1;
  uint *puVar2;
  uint uVar3;
  uint *puVar4;
  uint uVar5;
  undefined1 *puVar6;
  uint *puVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  
  uVar1 = (uint)param_1;
  puVar2 = (uint *)((uint)(param_1 >> 2) * 4 + 0x60120010);
  uVar5 = (uVar1 & 3) << 3;
  *puVar2 = ~(0xff << uVar5) & *puVar2;
  *puVar2 = (uint)(byte)param_2[4] << uVar5 & 0xff << uVar5 | *puVar2;
  if (uVar1 < 9) {
    if (uVar1 < 7) {
      if (uVar1 == 2) {
LAB_ram_f004bf76:
        iVar10 = 0;
        uVar8 = 0xffff;
      }
      else {
        if (uVar1 < 3) {
          if (uVar1 == 0) goto LAB_ram_f004bf76;
          if (uVar1 != 1) goto LAB_ram_f004bf68;
        }
        else {
          if (uVar1 == 4) goto LAB_ram_f004bf76;
          if (3 < uVar1) {
            if (uVar1 == 5) {
              iVar10 = 8;
              iVar9 = 4;
              goto LAB_ram_f004bfa2;
            }
            goto LAB_ram_f004bf68;
          }
        }
        iVar10 = 0x10;
        uVar8 = 0xffff0000;
      }
      uVar11 = uVar1 << 2;
    }
    else {
      iVar10 = 0;
      iVar9 = 0;
LAB_ram_f004bfa2:
      uVar11 = iVar9 + (uVar1 - 1) * 4;
      uVar5 = (uVar1 - 5) * 8 + iVar10;
      iVar10 = 0x10;
      uVar8 = 0xffff0000;
    }
  }
  else {
    if (uVar1 == 0xb) {
LAB_ram_f004bfbe:
      iVar10 = 0;
      uVar8 = 0xffff;
    }
    else {
      if (uVar1 < 0xc) {
        if (uVar1 == 9) goto LAB_ram_f004bfbe;
        if (uVar1 != 10) {
LAB_ram_f004bf68:
          uVar5 = 0;
          iVar10 = 0x10;
          uVar8 = 0xffff0000;
          uVar11 = 0;
          goto LAB_ram_f004bfd8;
        }
      }
      else if (uVar1 != 0xc) {
        if (uVar1 != 0xd) goto LAB_ram_f004bf68;
        goto LAB_ram_f004bfbe;
      }
      iVar10 = 0x10;
      uVar8 = 0xffff0000;
    }
    uVar5 = (int)((uVar1 - 9) * 8) % 0x20;
    uVar11 = (uVar1 - 9) * 4;
  }
LAB_ram_f004bfd8:
  uVar3 = 0xf << (uVar11 & 0x1f);
  uVar12 = 0xff << (uVar5 & 0x1f);
  if (uVar1 == 6) {
    return 0;
  }
  if (uVar1 < 7) {
    if (uVar1 < 4) {
      if (1 < uVar1) {
        puVar7 = (uint *)&DAT_ram_6013000c;
        puVar2 = (uint *)&DAT_ram_60120040;
        goto LAB_ram_f004c08a;
      }
      puVar7 = (uint *)&DAT_ram_6013000c;
      puVar2 = (uint *)&DAT_ram_60120040;
      puVar4 = (uint *)&DAT_ram_60120028;
    }
    else {
      puVar2 = (uint *)&DAT_ram_60120044;
      puVar7 = (uint *)0x0;
LAB_ram_f004c08a:
      puVar4 = puVar2 + -5;
      if (puVar7 != (uint *)0x0) {
        puVar7 = puVar7 + -1;
      }
    }
    *puVar4 = *puVar4 & ~uVar8;
    *puVar4 = *puVar4 | (uint)param_2[1] << iVar10 & uVar8;
    if (puVar7 != (uint *)0x0) {
      *puVar7 = *puVar7 & ~uVar8;
      *puVar7 = *puVar7 | (uint)param_2[2] << iVar10 & uVar8;
    }
    if ((uVar1 != 7) && (uVar1 != 8)) goto LAB_ram_f004c0de;
  }
  else {
    if (10 < uVar1) {
      if (uVar1 < 0xd) {
        puVar6 = &DAT_ram_60120028;
      }
      else {
        if (uVar1 != 0xd) {
          return 2;
        }
        puVar6 = &DAT_ram_6012002c;
      }
      puVar6 = puVar6 + 4;
      puVar2 = (uint *)0x60130010;
LAB_ram_f004c13e:
      puVar7 = (uint *)(puVar6 + 0xc);
      _DAT_ram_60120024 = (uint)param_2[3] << (uVar11 & 0x1f) & uVar3 | ~uVar3 & _DAT_ram_60120024;
      *puVar7 = *puVar7 & ~uVar8;
      *puVar7 = *puVar7 | (uint)param_2[1] << iVar10 & uVar8;
      _DAT_ram_60120040 = (uint)*param_2 << (uVar5 & 0x1f) & uVar12 | ~uVar12 & _DAT_ram_60120040;
      if ((uVar1 - 9 & 0xff) < 2) {
        puVar2 = (uint *)&DAT_ram_60130014;
      }
      *puVar2 = *puVar2 & ~uVar8;
      *puVar2 = *puVar2 | (uint)param_2[2] << iVar10 & uVar8;
      return 0;
    }
    if (8 < uVar1) {
      puVar2 = (uint *)&DAT_ram_6013000c;
      puVar6 = &DAT_ram_60120028;
      goto LAB_ram_f004c13e;
    }
    if ((uVar1 != 7) && (uVar1 != 8)) {
      return 2;
    }
    puVar2 = (uint *)&DAT_ram_60120040;
  }
  puVar2 = puVar2 + 1;
LAB_ram_f004c0de:
  *puVar2 = *puVar2 & ~uVar12;
  *puVar2 = *puVar2 | (uint)*param_2 << (uVar5 & 0x1f) & uVar12;
  _DAT_ram_60120020 = (uint)param_2[3] << (uVar11 & 0x1f) & uVar3 | ~uVar3 & _DAT_ram_60120020;
  return 0;
}

