#ifndef BATTLE_STAGE_ACTOR_H
#define BATTLE_STAGE_ACTOR_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

u8 sub_80264C0(BattleObj *, BattleObj *);

u32 sub_802698C(BattleObj *);

u32 sub_8026D08(BattleObj *);

u8 sub_8026F88(BattleObj *, BattleObj *);

u8 sub_802723C(BattleObj *);

u8 sub_802761C(BattleObj *); // 战斗对象特效/背景覆盖演出状态机 (gUnk_03000820 0起手/1-7等待/9全体); 唯一入口=0x0839CD5C 指针表 idx9 (sub_803F444 对 slot 0xB..0x70 的对象间接调用, 传参为 BattleObj)

u8 sub_8027760(BattleObj *, BattleObj *); // 战斗对象"换位靠近"演出状态机; 同 sub_80282EC 族双参 (参1 = 同池目标对象), 尾部帧计数+sub_803F658

u8 sub_8027A20(BattleObj *);

u32 sub_8027D9C(BattleObj *, BattleObj *);

u8 sub_8028098(BattleObj *);

u8 sub_80282EC(BattleObj *, BattleObj *);

u8 sub_80285A0(BattleObj *, BattleObj *); // 战斗对象"归位到目标"演出状态机; 同 sub_8027760 族双参, 用 sub_804B834/sub_804B8E8 槽位恢复

u8 sub_80287EC(BattleObj *);

u32 sub_8028AD8(BattleObj *);

u8 sub_8029268(BattleObj *, BattleObj *); // 战斗对象"移动到目标"演出状态机; 双参, 插值 headB.f_2B/f_2C

u8 sub_8029510(BattleObj *); // 战斗对象"强化/特效"演出状态机; 单参, 用 sub_804B834/sub_804B8E8 槽位恢复

u8 sub_8029784(BattleObj *); // 战斗对象"强化/特效"演出状态机变体; 同 sub_8029510 但 case0 CE80 末参=1, case1 空

u8 sub_80299C8(BattleObj *, BattleObj *); // 战斗对象"归位到目标"演出状态机 (双参; 用 sub_804B834/sub_804B8E8 槽位恢复)

u8 sub_8029BF8(BattleObj *);

u32 sub_802A154(BattleObj *); // 战斗对象"文字框/换装长演出"状态机; 单参, 0x0839CD5C 指针表 idx0x1A handler, 用 sub_801A2AC + gUnk_03000867/68 双窗口插值 + gUnk_0839DF90 演出序号

u8 sub_802A86C(BattleObj *); // 战斗对象"长演出/换装"状态机; 单参, 0x0839CD5C idx0x18 handler, 与 sub_8029BF8 近乎逐字同族 (0 → 0x12..0x21 → 9), 差异 f_B6=0xA9/sub_8020CC4 0x2E8/case28 参数序

u8 sub_802ADC4(BattleObj *); // 战斗对象"召唤/贝斯手演出"状态机; 单参, 0x0839CD5C idx0x19 handler, 同 sub_802B608 族 (0 → 0x12..0x19 → 9), 中段 sub_801768C 插值 gUnk_03000867 + sub_804B96C/sub_804C4D8 装备层切换, case9 result=2

u8 sub_802B0F0(BattleObj *, BattleObj *); // 战斗对象双参"绕行归位/多段位移"演出状态机 (gUnk_03000820: 0 → 0x12..0x1E → 9); 参1 = 目标对象, case22/28 用 sub_801768C 把 posX/posY/pad_C1 插值

u8 sub_802B608(BattleObj *); // 战斗对象"召唤/技能释放"演出状态机; 单参, 用 sub_801A348/sub_8019B98/sub_804BDD8/sub_804BE90

u8 sub_802B8BC(BattleObj *); // 战斗对象"闪现/瞬移演出"状态机; 同 sub_802F480 族, case0 类型 0xB, case20 多 Sfx(0x36), case22 多 Sfx_StopTrack

u8 sub_802BB24(BattleObj *, BattleObj *); // 战斗对象"召唤伙伴/传送"演出状态机; 双参, case13 按 arg1 位置播 sub_8020CC4

u8 sub_802BD94(BattleObj *); // 战斗对象"蓄力/多段演出"状态机; 单参, 用 sub_804BF14(9参)/sub_804C728

u8 sub_802C0EC(BattleObj *);

u8 sub_802C490(BattleObj *);

u8 sub_802C714(BattleObj *); // 战斗对象"多段演出/蓄力"状态机; 同 sub_802BD94 族单参, f_B6=2/case22 Sfx(0x4E,1,1)/case23 阈值0x18

u8 sub_802C9E8(BattleObj *, BattleObj *); // 战斗对象"多段演出/绕到目标+位移"状态机 (双参, 参1=同池目标对象); 0x0839CDE4 指针表 handler

u8 sub_802CE90(BattleObj *); // 战斗对象"冲刺/跳跃攻击"演出状态机; 同 sub_802C714 族单参, 两段 sub_8020CC4 + sub_801768C/sub_804BE90

u8 sub_802D1FC(BattleObj *); // 战斗对象"放声/蓄力"演出状态机 (gUnk_03000820: 0 → 0x12..0x17 → 9); 用 headB 等待 + 位置锁定后播 sub_8020CC4 锚点动画

u8 sub_802D454(BattleObj *, BattleObj *); // 战斗对象双参"蓄力/放声"演出状态机 (gUnk_03000820: 0 → 0x12..0x19 → 9); 参1 = 同池目标对象 (headB.kindFlags 等待)

u8 sub_802D728(BattleObj *, BattleObj *); // 战斗对象双参"目标重定位/多段插值"演出状态机 (gUnk_03000820: 0 → 0x12..0x17 → 9); 参1 = 目标对象, 用 sub_801768C 把 headB.f_2B/f_2C 插值到目标坐标

u8 sub_802DA78(BattleObj *); // 战斗对象"多段连击/旋转攻击"演出状态机; 同 sub_802CE90 族单参, headA.frameIdx %0x1F 循环 + 两段 sub_8020CC4

u8 sub_802DE04(BattleObj *); // 战斗对象"吸血/回复"演出状态机 (gObjActStep: 0 → 0x12 → 0x14/0x15 → 9; 0x0839CD5C 表 idx 0x28 handler); case20 hp += sub_8048D64(obj, maxHp>>1) (至多回一半, 按 maxHp-hp 夹取), case9 返回 2; 尾部 gObjActStepTimer++

u32 sub_802DFDC(BattleObj *arg);

u8 sub_802E234(BattleObj *); // 战斗对象"横移演出"状态机; 单参, 0x0839CE04 指针表 handler

u32 sub_802E49C(BattleObj *, BattleObj *);

u8 sub_802E6C8(BattleObj *); // 战斗对象"多段演出/蓄力"状态机; 单参, 0x0839CE10 指针表 handler, 用 sub_804BF14(9参)/sub_804C728

u8 sub_802EAC4(BattleObj *); // 战斗对象"多段演出/蓄力"状态机; 单参, 同 sub_802E6C8 族 (0 → 0x1C..0x1F → 0x15..0x19 → 9), case28 sub_8020CC4 0x314/case21 0xB4/Sfx 0x59/0x5A/case24 0x78

u8 sub_802EDD8(BattleObj *); // 战斗对象"多段演出/蓄力"状态机; 单参, 0x0839CD5C 指针表 idx0x2E handler, 同 sub_802E6C8 族 (0 → 0x1C..0x1F → 0x15..0x18 → 9), case28 sub_8020CC4 0x316/case21 0xB4/Sfx 0x59/0x5A/0x55/case23 sub_8044514(0x1E)

u8 sub_802F100(BattleObj *);

u32 sub_802F480(BattleObj *arg);

u8 sub_802F6D8(BattleObj *); // 战斗对象"换装/形态切换"演出状态机; 单参, 用 sub_804B96C/sub_804C4D8/Sfx_TrackBusy

u8 sub_802F9EC(BattleObj *);

u8 sub_802FE98(BattleObj *);

u32 sub_803029C(BattleObj *, BattleObj *);

u8 sub_8030664(BattleObj *); // 战斗对象"双头换装/形态切换"演出状态机; 单参, 用双 headA/headB + sub_8020CC4 锚点

u32 sub_80309B0(BattleObj *arg);

u8 sub_8030C08(BattleObj *obj);

u8 sub_8030D9C(BattleObj *obj);

u8 sub_8030F30(BattleObj *obj);

u8 sub_80310C4(BattleObj *obj);

u8 sub_8031258(BattleObj *obj);

u8 sub_80313EC(BattleObj *obj);

#endif // BATTLE_STAGE_ACTOR_H
