#ifndef _SAVE_H
#define _SAVE_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

s32 Save_Fsm(u8);
u8 Save_FillSlot0(u8);
void Save_FillSlot1(u8);
void Save_FillSlot2(u8);
void Save_FillSlot3(u8);

#endif