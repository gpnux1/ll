#ifndef BATTLE_SPECIAL_TARGETS_H
#define BATTLE_SPECIAL_TARGETS_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void sub_804D1B4(BattleObj *obj, BattleObj *pool); // obj槽位概率填充: 守卫+表驱动随机

void sub_804D260(BattleObj *obj, BattleObj *pool); // obj槽位概率填充 (x10, sub_804D1B4 孪生)

void sub_804D310(BattleObj *obj, BattleObj *pool);

void sub_804D3A0(BattleObj *obj, BattleObj *pool); // obj槽位概率填充 (x13, 同族孪生)

void sub_804D44C(BattleObj *obj, BattleObj *pool); // obj槽位概率填充 (x10, 同族孪生)

void sub_804D4FC(BattleObj *obj, BattleObj *pool);

void sub_804D5B4(BattleObj *obj, BattleObj *pool);

void sub_804D708(BattleObj *obj, BattleObj *pool); // pool = sub_80489E8 的对象池基址 (BattleObj[] 步长 0xC8)

void sub_804D798(BattleObj *obj, BattleObj *pool);

void sub_804D840(BattleObj *obj, BattleObj *pool);

void sub_804D8F4(BattleObj *obj, BattleObj *pool);

void sub_804DA04(BattleObj *obj, BattleObj *pool);

void sub_804DABC(BattleObj *obj, BattleObj *pool);

void sub_804DB64(BattleObj *obj, BattleObj *pool);

void sub_804DC24(BattleObj *obj, BattleObj *pool);

void sub_804DCD8(BattleObj *obj, BattleObj *pool);

#endif // BATTLE_SPECIAL_TARGETS_H
