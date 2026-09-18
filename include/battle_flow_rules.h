#ifndef BATTLE_FLOW_RULES_H
#define BATTLE_FLOW_RULES_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

u32 sub_804473C(BattleObj *arg0, u8 *arg1);

u32 sub_80448A8(BattleObj *arg0, u8 *arg1);

u32 sub_8044A40();

u32 sub_8044F4C(BattleObj *, BattleObj *);

u16 sub_8045098(BattleObj *source, BattleObj *target); // 道具伤害: source[0xA4] 选基础值/随机幅度, 属性响应倍增或减半, 钳位 [0,999]; 推进 RNG 状态

void sub_804519C(BattleObj *source, BattleObj *target);

u8 sub_8045328(BattleObj *, BattleObj *, u8); // 2026-09-13 gpnux: 命中判定; arg0=行动发起方, arg1=目标对象, arg2=基准权重(调用点 0x50). 2026-09-11 zcode-engine: sub_8046480 调用点反汇编证据 (lsls/lsrs/cmp #1)

u16 sub_80453D8(void);

u16 sub_804542C(void);

u8 sub_80454A4(u16);

u16 sub_80455A0(u8, u8);

void sub_8045688(u8, u8, u8);

void sub_80457AC();

s8 sub_8045860(u8, u8 *);

u8 sub_8045940(BattleObj *obj, u8 *buf);

u8 sub_8045A10(BattleObj *, u8);

u8 sub_8045A74(u8 *, u8 *, u8, u8, u8);

void sub_8045B90(BattleObj *obj, u8 index);

void sub_8045BF4(BattleObj *obj);

void sub_8045D00(BattleObj *, u8, u16, s8 *); // 池目标候选收集: mode 0/4=自身 1/2=同族/外形 3=敌侧 5=我方 6=无

void sub_8045EB8(u8 *);

u8 sub_8045F10(BattleObj *, u16);

void sub_8045F94(BattleObj *obj, u16 arg1);

void sub_8046060(BattleObj *obj, u16 arg1);

void sub_804612C(BattleObj *obj, u16 arg1, u16 arg2);

u8 sub_804621C(BattleObj *, u8 *, u8);

u32 sub_80462E4(BattleObj *, u8 *, u16);

u32 sub_8046480(BattleObj *arg0, u8 *buf, u8 mode);

u8 sub_8046558(BattleObj *source, u8 *out, u8 mask, u16 classMask); // 按类别掩码及 +0xAC 匹配收集池槽号；out 至少容纳5字节(slot<=10)或7字节

void sub_804666C();

void sub_80466F0();

void sub_8046C50();

u8 sub_8046CD4(BattleObj *source, u8 *out); // out可为NULL，仅计数；否则容量至少7字节(slot<=10)或5字节

u8 sub_8046E18(u8 *, s32, s32); // 2026-09-11 zcode-engine: 宽参+窄局部 (经验71), 匹配调用方传参无截断证据

u16 sub_8046F0C(BattleObj *obj); // 2026-09-11 zcode-engine: 调用点返回值按 u16 用 (lsls/lsrs #0x10), 无已匹配调用者

u16 sub_8047024(BattleObj *obj, u8 kind);

u8 sub_80471AC(void);

u32 sub_80472E8(BattleObj *obj, u16 skillId, u16 suppliedPower);

void sub_804753C(BattleObj *obj, u8 kind, u8 amount); // 战斗属性BUFF增长: statMods[kind] += base*amount/100, 上限=参考属性/10*5 (kind1 上限用atc/基数用def); kind0 另写 gUnk_0300090A

u8 sub_80476DC();

u8 sub_8047B1C(BattleObj *obj);

u8 sub_8047D28(BattleObj *obj, u8 mask);

u8 sub_8047DC8(BattleObj *obj);

s32 sub_8047FCC(u16);

s32 sub_80480EC(void);

void sub_80481B8();

u8 sub_8048310(void);

u8 sub_8048458(BattleObj *obj);

u8 sub_80485A4(BattleObj *obj, u8 mode);

u8 sub_8048690(BattleObj *arg0, BattleObj *arg1, u8 arg2);

u8 sub_8048764(u8 *);

u8 sub_804877C(u8);

u8 sub_80487A4(u8);

u8 sub_80487CC(u8);

u16 sub_8048818(u8, u8);

u8 sub_8048868(u8, u8);

u8 sub_80488CC(u8 *, u8);

u8 sub_8048934(BattleObj *arg0, u8 arg1);

u8 sub_8048984(u8 *, u8);

u8 sub_80489A4(u8 *, u8);

u16 sub_80489C8(u8 *, u16);

u8 sub_80489E8(BattleObj *base, u8 *output, u8 mode, u16 flags);

u8 sub_8048A68(BattleObj *arg0);

void sub_8048A88(u8 *, s8, s8);

void sub_8048ACC(u8 *, u8, u8);

void sub_8048B30(u8, u8, u16);

void sub_8048B5C(u8 *, u8);

u8 sub_8048B88(BattleObj *arg0);

u8 sub_8048BAC(BattleObj *arg0);

void sub_8048BD0(BattleObj *arg0);

u8 sub_8048C30(BattleObj *obj);

u8 sub_8048C80(BattleObj *obj);

u8 sub_8048CEC(BattleObj *obj);

void sub_8048D40(BattleObj *arg0);

u32 sub_8048D64(BattleObj *arg0, u16 arg1);

u8 sub_8048D84(u8 *, u8 *);

void sub_8048DA4();

void sub_8048F0C();

u8 sub_8048FB8(void);

void sub_80492C0();

void sub_80494F0();

u32 sub_80497B0(u16 *arg0, u16 arg1);

u32 sub_80498E0();

u32 sub_8049958(u16 *dest);

u8 sub_8049AD8(u8 arg0);

void sub_8049B70();

u8 sub_8049C1C(u8 *); // 2026-09-11 zcode-engine: void*→u8* (定义侧 arg0[0] 字节读写), 无已匹配调用者

u8 sub_8049D58(u8); // 唯一调用点 sub_8018070: 入参/返回均 u8

u8 sub_8049DF8(void *, void *);

void sub_804A148();

u8 sub_804A368(void *);

void sub_804AA2C(u16 mask);

void sub_804AB10(void);

void sub_804AB40();

void sub_804ABD0(void);

u32 sub_804ABF8(u16 *dest, u8 arg1);

void sub_804AC60(void);

u16 *sub_804ACC0(u8);

void sub_804AD24();

#endif // BATTLE_FLOW_RULES_H
