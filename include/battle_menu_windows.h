#ifndef BATTLE_MENU_WINDOWS_H
#define BATTLE_MENU_WINDOWS_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void MenuSlot_ResetAll();

void sub_8021184(s8, u8 *); // 战斗对象槽号/状态同步: arg1+0xBE 槽号→idx, switch((s8)arg0) case 0/3/6/7 更新 gUnk_030007xx 系列

void sub_80212B4();

u8 sub_802151C(u8, BattleObj *);

u8 sub_8021700(void);

void sub_8021788(u8 arg0);

void sub_802181C();

u8 sub_802192C(void *, void *, u8 *);

u8 sub_8022458(u8); // 唯一调用点 sub_8018070: 入参 0x7F, 返回 u8

void sub_8022550();

void sub_8022710();

void sub_8022F2C();

void sub_80230BC();

void sub_8023320();

void sub_8023414();

void sub_8023820();

void sub_80244BC();

void sub_8024618();

void sub_80246E8();

void sub_8024820();

void sub_8024940();

void sub_802550C(u8);

void sub_8025518();

void sub_8025638();

void sub_8025650();

void sub_80256E4();

void sub_802576C(u8 *);

u8 sub_80257D8(BattleObj *, BattleObj *); // 战斗对象"抓取目标并位移"演出状态机 (gUnk_03000820: 0→3→4→5→6→7→9); arg1=同池被抓取协作对象, case5 用 sub_801768C 把 arg1->posX 在帧窗口 0x64..0x69 插值

void sub_8025994();

u32 sub_8025DA8(BattleObj *, BattleObj *);

u32 sub_80260BC(BattleObj *); // 战斗对象"多段闪光/抖动演出(变体)"状态机; 同 sub_802698C 结构, 细分步骤 0x12..0x18; case0x14 装配 arg5=0xD; case0x15 Sfx_Play(0x55); case0x16>0x17 先 Sfx_StopTrack(1); case6 复位后 headA.kindFlags|=0x100 转 0x18

#endif // BATTLE_MENU_WINDOWS_H
