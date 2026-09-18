#ifndef BATTLE_TASK_SERVICES_H
#define BATTLE_TASK_SERVICES_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void Sio_SetReady();

void Sio_Shutdown();

u32 sub_8017120();

s32 sub_80171E4();

u32 Sio_IsHost();

void sub_80175C0();

void Sio_SetXferCtx(u32 *, u32 *, u32, u32);

void Sio_ClearSlot();

void sub_8017640();

s16 sub_801768C(s16, s16, s16, s16, u8);  /** 前4参必须 s16 (定义侧用有符号 float 转换); 第5参必须 u8 不能 s8: s8 会给 code_80264C0.c 的调用点 (sub_80405A4/sub_8042AB4) 多加一条 ldrsb, 各 +4B -> 整 ROM 位移 8B **/

void BattleTask_Run();

void sub_8017FA4(s8);

void sub_8018070();

void sub_80182A8(u16 , u16 *);

u8 sub_80184A8(UnkNode *, u8 );

void sub_801869C();

void sub_8018744();

void sub_8018750();

u32 sub_801878C();

void sub_8018798(u8, s16);

u32 sub_80187A8();

u16 sub_80187B4();

void sub_80187C0(u16);

void sub_80187D4(u16);

u16 sub_80187E8();

u16 sub_80187F4();

void ListNode_Init(UnkNode *);

void ListNode_InitKey(UnkNode *, u8);

void ListNode_InsertSorted(UnkNode *, UnkNode *);

void sub_8018838(u32);

u32 Rng_LcgNext(void);

u32 GetObjPool();

u32 GetCtx_0248();

u32 GetBuf_37410();

void sub_801887C();

void sub_801889C();

void sub_80188BC();

void sub_8018928();

void sub_8018A58(u8);

void sub_8018BF8();

void sub_8018D9C();

u32 sub_8018E34();

void sub_8018EA8(); /* 3 位数图块显示: (值, x, y, 调色板, 标志); 唯一 C 调用者 sub_801CF90 */

void sub_8018FC0(u8, u8, u8, u8, u8, u8, u8); /* 图块绘制 (槽号, x, y, 调色板3参, 标志); 唯一 C 调用者 sub_801CF90 */

void Bg0_InitClear();

void sub_80191CC();

void DialogCtx_Clear3();

void sub_801933C();

void sub_80196D4();

void DialogCtx_SetPair(u32, u32, u32, u32, u32);

void BattleFx_UpdateTable();

void sub_80199E0();

void sub_8019AD0(u8, u16);

u8 sub_8019B98(u8, u8, u8, u8); // 返回 u8 (调用点 lsls #24 截断判断)

void BattleUiFlag_Clear();

void BattleUiFlag_Set(u16);

u16 BattleUiFlag_Get();

void BattleUiFlag_Reset(u16);

void Disp_ObjOff();

void Disp_ObjOn();

void sub_8019E60(); // BlankTilemap: 清空 VRAM 图块 0x2C0 并把 1024 项 tilemap 全填成该空白图块

#define BlankTilemap sub_8019E60

void Disp_Bg1Off();

void DialogCtx_SetHead(u8, u8, u8);

void sub_8019F08(u16 *, u16, u8, u8, u8, u8);

void sub_8019F78(u16 *, int, s8, int, u8, u8, u8, u8);

u8 DialogCtx_GetField_C(u8);

void BgMap_PalFillRect(); // K&R: ROM 调用点无截断 (被调入口截断由定义侧提供), 全原型会给 caller 加 lsls/lsrs 破坏已匹配的 sub_802576C

void DialogCtx_Flush();

void FlashFlag_Clear();

u16 FlashFlag_Get();

void FlashFlag_Reset(u16);

void BattleFx_Init(u8, u8, u8, u8);

void BattleFx_Stop();

void BattleFx_DispOff();

void sub_801A270();

void sub_801A2AC(int, int, int);

void sub_801A2EC();

void BgLoad_Reset();

void BgLoad_Finish();

u8 BgLoad_GetPos();

void sub_801A348();

void sub_801A35C();

void BgScrolls_WriteAll();

void sub_801A3A8(u8, u16, u16);

#endif // BATTLE_TASK_SERVICES_H
