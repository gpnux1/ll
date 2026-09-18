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

u8 sub_8041308(BattleObj *obj, BattleObj *arg1);

u8 sub_80416F0(BattleObj *obj, BattleObj *arg1);

u32 sub_80419E0(BattleObj *obj, BattleObj *arg1);

u8 sub_8041EDC(BattleObj *obj, BattleObj *arg1);

u8 sub_8042200(BattleObj *obj);

u8 sub_80422B8(BattleObj *obj, BattleObj *arg1);

u8 sub_8042784(BattleObj *obj, BattleObj *arg1);

u8 sub_8042AB4(BattleObj *obj);

u8 sub_8042B90(BattleObj *obj, BattleObj *arg1);

void sub_8042E70();

u32 sub_8043554(BattleObj *obj);

u32 sub_8043938(BattleObj *);

u32 sub_8043B5C(BattleObj *obj, BattleObj *arg1);

u32 sub_8043F90(BattleObj *obj);

#endif // BATTLE_STAGE_EFFECTS_H
