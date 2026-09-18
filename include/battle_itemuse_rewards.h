#ifndef BATTLE_ITEMUSE_REWARDS_H
#define BATTLE_ITEMUSE_REWARDS_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void sub_804DD70(BattleObj *ptr, u32 arg1);

u8 sub_804DD90(u8, u8); /** 勿改宽原型/K&R: u8原型+Sub6C结构形态才是 sub_8045EB8 的解 */

void sub_804DE20();

void sub_804DE8C();

u8 sub_804DF14(InvListEntry *);

void sub_804DF74(InvListEntry *, u8 *, u8);

void sub_804DFD8(u16 *, u8, u8, u8 *, u8, u8, u8);

u8 ItemUseFx_RunConsumable(BattleObj *arg0, u32 arg1);

u8 ItemUseFx_RunWeapon(u8 *, u32);

s8 sub_804E6DC(BattleObj *obj, u8 value);

s8 sub_804E76C(BattleObj *obj, u8 arg1, u8 arg2);

void BattleFxObjs_Add();

u32 BattleFx_Update(void);

u8 BattleDrops_Roll(u32 *arg0);

u8 sub_804EC04(u32 *arg0);
#define BattleCards_Roll sub_804EC04

void sub_804EEC4(void);

void sub_804EF00(u8);

void sub_804EF50(void);

u8 sub_804EF90(u8);

void sub_804EFDC(u8 *, u8, u8, u8 *, u8);

u8 sub_804F050(u8);

void ItemUseFx_Reset();

u8 ItemUseFx_Update(BattleObj *arg0, u32 arg1);

u8 sub_804F0B8(BattleObj *, s32); // CheckObjectKindSlot: 比较 equipSlots[4]/[5] (+0x91/+0x92) 两个候选 id, 返 1/2/0

#define CheckObjectKindSlot sub_804F0B8

s8 sub_804F10C(u8, u8);

u8 sub_804F17C(u8 *outSlots, u8, u8); // 收集版: 命中的槽下标写入 outSlots[0..n-1], 返 n

#endif // BATTLE_ITEMUSE_REWARDS_H
