#ifndef BATTLE_PALETTE_WIPE_H
#define BATTLE_PALETTE_WIPE_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void sub_804AD54();

void sub_804AD60(void);

void sub_804ADE0();

void sub_804ADF8();

void sub_804AE2C();

void sub_804AF60();

u8 sub_804B080(BattleObj *obj, u8 index, u16 flags);

void sub_804B1EC();

void sub_804B1F8(WipeDesc *);

void sub_804B224(u16 *);

void sub_804B288();

void sub_804B3C0(PaletteAnimEntry *, u8, u16 *, u16 *); // opcode1 流式调色板动画一帧

void sub_804B458(PaletteAnimEntry *, u8, u16 *, u16 *); // opcode2 帧步进调色板动画一帧

void sub_804B4D0(PaletteAnimEntry *, u8, u16 *, u16 *); // opcode3 淡变调色板动画一帧

void sub_804B56C(u16 *, u16 *, u8, s8 *); // 16 色插值: src + delta*weight>>shift, clamp 0..31 (delta+3=shift)

s32 sub_804B654(u8 arg0, u8 arg1, s8 *arg2, u8 arg3, u8 arg4, u8 arg5);

void sub_804B7B0(u8, u8);

s8 sub_804B834(u8, u8, u8, s8, u8);

void sub_804B8E8(u8, u8);

s32 sub_804B96C(u8, u8, s8, s8, s8, u8, u8, s8, u8);

void sub_804BB64(u8, u8);

s32 sub_804BBDC(u8, u8, s8, s8, s8, u8, s8, u8); // BG 调色板流式动画 (opcode1, 与 sub_804B654 同构)

void sub_804BD54(u8, u8);

s8 sub_804BDD8(u8, u8, u8, s8, u8);

void sub_804BE90(u8, u8);

s32 sub_804BF14(u8, u8, s8, s8, s8, u8, u8, s8, u8);

void sub_804C10C(u8, u8);

void sub_804C184();

void *sub_804C194(u8);

void sub_804C1B4(u8, u8, u8);

void sub_804C1E4(u8, u8, u8);

u8 sub_804C214(u8, u8);

void sub_804C250(u8, u8);

void sub_804C278(u8, u8);

void sub_804C2A0(u16 *, u16 *, u8, u8, u8);

u16 sub_804C2F0();

void sub_804C2FC(u32, u8, u8);

void sub_804C364(u8, u8);

void sub_804C3A4(u8, u8);

void sub_804C3E4(u8);

void sub_804C420(u8);

void sub_804C45C(void);

void sub_804C4D8(u8, u8, u8); // 三个形参入口均 lsls/lsrs #0x18 → u8

u16 sub_804C53C();

void sub_804C548(u32, u8, u8);

void sub_804C5B8(u8, u8);

void sub_804C5F8(u8, u8);

void sub_804C638(u8);

void sub_804C674(u8);

void sub_804C6B0(void);

void sub_804C728(u8, u8, u8); // 三个形参入口均 lsls/lsrs #0x18 → u8

void sub_804C78C();

void sub_804C890(BattleObj *); // 对 obj 池槽 0..4 中 sub_8045F10(o,0x20)==2 的对象置 f_BD=随机同伴, fxKind=0

u8 sub_804C8E0(BattleObj *, u8); // obj池槽位移除元素+随机取回 (返回 u8)

void sub_804C9B4();

void sub_804CA2C(BattleObj *obj);

void sub_804CAA0(BattleObj *);

void sub_804CB18(BattleObj *obj);

void sub_804CB8C(BattleObj *);

void sub_804CC00(BattleObj *);

void sub_804CC78(BattleObj *);

void sub_804CCEC(BattleObj *);

void sub_804CD60(BattleObj *);

void sub_804CDD4(BattleObj *);

void sub_804CE48(BattleObj *);

void sub_804CEBC();

void sub_804CEE0();

void sub_804D0F8(BattleObj *obj); // obj槽位填充: 守卫+移除匹配obj[0xAC]+随机取回

#endif // BATTLE_PALETTE_WIPE_H
