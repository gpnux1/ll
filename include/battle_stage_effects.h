#ifndef BATTLE_STAGE_EFFECTS_H
#define BATTLE_STAGE_EFFECTS_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

u8 sub_803FF54(BattleObj *obj);

u32 sub_80401AC(void);

u8 sub_80405A4(BattleObj *obj);

u8 sub_8040690(BattleObj *obj, BattleObj *arg1);

u8 sub_8040EE8(BattleObj *obj, BattleObj *arg1);

void sub_8041308();

void sub_80416F0();

void sub_80419E0();

void sub_8041EDC();

u8 sub_8042200(BattleObj *obj);

void sub_80422B8();

void sub_8042784();

u8 sub_8042AB4(BattleObj *obj);

void sub_8042B90();

void sub_8042E70();

void sub_8043554();

void sub_8043938();

void sub_8043B5C();

void sub_8043F90();

#endif // BATTLE_STAGE_EFFECTS_H
