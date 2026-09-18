#ifndef BATTLE_OBJECT_ENGINE_H
#define BATTLE_OBJECT_ENGINE_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void ObjGfxLoad_Step();

void ObjGfxLoad_Copy(ObjHead *, ObjHead *);

void sub_801A684(ObjHead *);

void sub_801A6F4();

u8 sub_801A884(ObjHead *, u8, u8 *);

void sub_801AD0C(ObjHead *);

u8 sub_801B0B8(ObjHead *, u8);

void sub_801B570(ObjHead *);

void sub_801B688(u8); // 唯一调用点 sub_8018070: r0 = 本帧 u8 结果 (asm 体内还读 [sp,#4], 实参可能不止 1 个)

void sub_801B760(u16);

u8 sub_801B790(u16);

void sub_801B7B8();

void sub_801B81C(ObjHead *, u8, u8, u16, u8, u32, u32, u16, u16, u16);

u8 sub_801B878(ObjHead *, u8, u8 *);

u8 sub_801B8AC(ObjHead *, u8);

u16 *sub_801B8E8(u16 *, u16);

u16 *sub_801B8FC(ObjHead *, u8, u16);

void sub_801B920();

u8 sub_801B954(ObjHead *);

u16 sub_801B95C(ObjHead *);

void sub_801B964();

u8 sub_801BE34(BattleObj *pool);

u8 sub_801C484(BattleObj *pool);

void sub_801CA08(BattleObj *, u8, u16, u8, u8);

void sub_801CBA4(BattleObj *, u8, u16, u8, u8);

void sub_801CE80(BattleObj *, u8, u16, u8, u8);

void sub_801CF90(BattleObj *, u8);

void sub_801D12C(BattleObj *, u8);

u16 sub_801D19C(BattleObj *, u8);

u8 sub_801D214(BattleObj *, u8); // 唯一调用点 sub_8018070: (gObjPoolPtr, 本帧结果) -> u8; 5 槽 tile 属性装载 + DMA 上传

u8 sub_801D378(BattleObj *, u8 );

void sub_801D468();

void sub_801D568(BattleObj *);

void sub_801D710(BattleObj *, u8); // 弹出重绘/重登记: kind==0 重画 f_B2 数字(0x158+4*count tile区), kind!=0 画固定图形; 尾部重记 gUnk_03000670[count] 并 count++

u8 sub_801D984(u8); // OAM 缓冲自绘: 按 0x0300068C 循环把 0x03000670[i] 逐字段写入 gOamBuffer[r6] (r6 递减), 返回递减后的槽号

u32 sub_801DAA0(); // PollSceneTimer: 场景计时状态机 (0x0300068E 0..0x22++), 走完→重置+返回1

void sub_801DB3C(BattleObj *, u8, u16);

void sub_801DC20(BattleObj *, u8);

void sub_801DD04(BattleObj *, u8, u16);

void sub_801DDB0(BattleObj *, u8);

void sub_801DE44(); // ResetSceneObjects: 重置 3 个标志 + 7 项表 + sub_804C2FC(表0), 再对对象列表逐项调 sub_801D710

#define ResetSceneObjects sub_801DE44

void sub_801DEDC(BattleObj *, BattleObj *);

void sub_801DF90(BattleObj *, BattleObj *);

u8 sub_801E040(void);

u8 sub_801E1D8();

u32 sub_801E30C(BattleObj *, BattleObj *); // ROM 中无调用者(死代码); 同族 E4D4/E690, 索引源=装备道具配对表/技能表 + [r6(UB残留 sb)+3] 间接字节

u32 sub_801E4D4(BattleObj *, BattleObj *); // ROM 中无调用者(死代码); 返回 7 项标志数组中是否存在回绕项

u32 sub_801E690(BattleObj *, BattleObj *); // ROM 中无调用者(死代码); 同 E4D4, 查表入口改 animPtr+2 / animPtr+8+animSubIdx*2

u8 sub_801E848();

void sub_801EA70();

u32 sub_801EC3C(BattleObj *, u8); // 返回字节值 (0x20 / (x&0x1F)<<3 / 小常量); 调用方需 (u8) 截断

void sub_801ED40(BattleObj *, u8);

void sub_801EE6C(BattleObj *);

u8 sub_801EEE4();

u8 sub_801F3FC();

u8 sub_801F76C(BattleObj *); // 返回战斗对象动作类别 0-3 (唯一调用者 801EA70 按 u8 使用返回值; 体内按 r3 返回 0/1/2/3)

u8 sub_801F884(BattleObj *); // 目标匹配键: f_BD 指向对象的 +0xAC 按槽号/fxKind/gUnk_08393B28.targetMode 取原值或 nibble 映射 (0=调用者不过滤)

void sub_801FA10(BattleObj *, u8);

void sub_801FAB8();

void sub_801FEBC(BattleObj *, u16, u8);

s8 sub_801FF40(u8);

void sub_80200E8(BattleObj *, PlayerStats *, u8); // 战斗对象数值装载: arg1=&gPartyStats[n] (调用点 sub_801B964, lsls#6 步长 0x40), arg2=ROM 表 0x08393B14[memberId] → obj+0xAC

void sub_8020228(u8 *, BattleObj *, u8); // 敌方对象属性装载: 按 slot 索引 gUnk_083987EC (0x2C/项) 装 HP/MP/五维/lv/animPtr; slot>0x70 清 statMods 转 sub_802031C; 尾部 +0xAC=arg2

void sub_802031C(u8 *, BattleObj *); // 特殊槽(slot>=0x71)属性装载: 按 obj->slot 索引 gUnk_0839ABCC (0x40/项) 装 animPtr/HP/MP/五维/noa/f_C3/pad_C4; 再按 (slot-0x71) 0..0x10 分派 sub_8020648 施加能力加成 (部分类型额外置 obj->state|=4); 首参在体内未被读取 (调用方 sub_8020228 的 arg0 透传)

void sub_8020648(u8, BattleObj *, u16, u16, u16, u16, u16, u16, u16); // 特殊对象能力倍率施加: scale=max(lv, lvFloor) 夹 0x32 写入 obj->lv, 六个增量分别 ×scale 截 u16 后饱和加进 maxHp+hp (arg3 同值双写) / atc / def / agl / men / res

u8 sub_8020798();

void sub_80207A4();

u8 sub_80207B4(void *);

void sub_80207DC(BattleObj *obj, u8 bf, u8 c0, u16 f2a, u8 f35);

void sub_8020840(BattleObj *obj, u8 bf, u8 c0, u16 f2a, u8 f35);

void sub_80208A4(BattleObj *);

void sub_8020914(BattleObj *);

void sub_802093C(BattleObj *);

void sub_8020974(ObjHead *, u16, u16, u8, u16); // 入口截断定类: r1/r2/栈参 u16, r3 u8, r0 = 对象头

void sub_80209C8(BattleObj *);

void sub_80209EC(BattleObj *);

void sub_8020A0C(BattleObj *, u8);

u8 sub_8020A7C(BattleObj *);

u8 sub_8020AB0();

void sub_8020AE4();

void sub_8020B04(BattleObj *arg0);

u32 sub_8020B48();

void sub_8020B54();

void sub_8020B90(BattleObj *);

u8 sub_8020BC0(BattleObj *);

u8 sub_8020BF0();

u8 sub_8020C2C(void);

void sub_8020C58(BattleObj *, u32);

void sub_8020CC4(void *, u8, u8, u16, u8, u16, u16);

void sub_8020D50(BattleObj *, u8);

void sub_8020DA0(BattleObj *, u8);

void sub_8020DE4();

void sub_8020DF0(BattleObj *);

u32 *sub_8020E54();

u8 sub_8020E5C();

u32 sub_8020E68();

void sub_8020E74();

void sub_8020E90(BattleObj *);

u8 sub_8020EAC(BattleObj *);

void sub_8020EC8();

void sub_8020EEC(u8);

void sub_8020F08();

void sub_8020F4C(BattleObj *);

void sub_8020FB8(BattleObj *, s32, s32, u16, u8);

void sub_802103C(BattleObj *, u8, u16);

void sub_8021064(u8);

void sub_80210C0(BattleObj *, u8);

#endif // BATTLE_OBJECT_ENGINE_H
