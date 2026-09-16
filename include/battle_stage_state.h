#ifndef BATTLE_STAGE_STATE_H
#define BATTLE_STAGE_STATE_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void sub_8044394(BattleObj *obj);

void sub_8044414();

u16 sub_8044420();

void sub_804442C(u8);

void sub_804448C();

u8 sub_8044498();

void sub_80444A4(BattleObj *);

u8 sub_80444E8(void);

void sub_8044514(s16);

void sub_8044574(s16, u16, u8);

u8 *sub_80445E0();

void sub_80445E8(BattleObj *arg0, u8 arg1);

u8 sub_8044680(BattleObj *arg0);

u8 sub_80446A4(BattleObj *arg0);

void sub_80446BC(BattleObj *arg0);

s32 sub_8044728();

s32 sub_804472C();

s32 sub_8044730();

s32 sub_8044734();

s32 sub_8044738();

#endif // BATTLE_STAGE_STATE_H
