#ifndef BATTLE_STAGE_DIALOGUE_H
#define BATTLE_STAGE_DIALOGUE_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

u32 sub_8032548(BattleObj *arg0, u8 *arg1);

u32 sub_803272C(BattleObj *arg0, u8 *arg1);

u32 sub_8032948(BattleObj *, u8 *); // 战斗对话/演出状态机变体 (gUnk_03000820: 0 → 1 → 2 → 0x12..0x1B → 9); 多段 sub_8020974 装载 + sub_804BF14/sub_804C728 窗口 + sub_801768C 位移

u8 sub_8032D74(BattleObj *obj);

void sub_8032EA0();

void sub_80334B8();

u32 sub_8033988(BattleObj *arg);

u32 sub_8033E2C(BattleObj *arg);

u32 sub_8034440(BattleObj *arg);

u32 sub_80345AC(BattleObj *arg);

u32 sub_8034718(BattleObj *arg, BattleObj *arg1);

u32 sub_80348A8(BattleObj *arg);

u32 sub_8034BFC(BattleObj *arg);

u32 sub_8034D94(BattleObj *arg);

u32 sub_8034F00(BattleObj *arg);

u32 sub_8035130(BattleObj *arg);

void sub_8035360();

u32 sub_803586C(BattleObj *arg);

u32 sub_8035B04(BattleObj *arg);

u32 sub_8035D9C(BattleObj *arg);

u32 sub_8036034(BattleObj *arg);

u32 sub_80362CC(BattleObj *arg);

u32 sub_8036564(BattleObj *, BattleObj *);

u32 sub_80368FC(BattleObj *arg0);

u32 sub_8036B30(BattleObj *); // 战斗对象"文字框演出"状态机 (gUnk_03000820: 0 → 1 → 2 → 5 → 0x12..0x16 → 0x1A → 0x1B → 9); 用 sub_801A2AC/sub_8019B98/sub_804BDD8/sub_804BE90 + gUnk_03000867 窗口插值

u32 sub_8036EA4(BattleObj *, BattleObj *);

u32 sub_8037078(BattleObj *, BattleObj *);

void sub_8037388();

u32 sub_8037868(BattleObj *);

u32 sub_8037C40(BattleObj *, BattleObj *);

u32 sub_8037E14(BattleObj *obj);

u32 sub_8037FE8(BattleObj *obj);

u32 sub_80381BC(BattleObj *obj);

u32 sub_8038390(BattleObj *obj);

u32 sub_8038568(BattleObj *arg, BattleObj *arg1);

u32 sub_803874C(BattleObj *obj);

void sub_8038920();

u32 sub_8038C84(BattleObj *arg0, u8 *arg1);

u32 sub_8038E44(BattleObj *arg0, u8 *arg1); // NPC对话状态机变体 (同 sub_8038C84 骨架, 起手 bl sub_80187E8; case1 sub_803E58C mode2+开场动画0x3C8; case3 帧==0x24 发音效等0x1E / 帧>0x43 发音效等0x28→5)

u32 sub_8039024(BattleObj *, BattleObj *);

u8 sub_80392C0(BattleObj *obj);

u32 sub_80393E0(BattleObj *);

u32 sub_8039724(BattleObj *obj);

void sub_8039C38();

void sub_8039C6C();

void sub_803A478();

void sub_803A8D0();

void sub_803AF60();

void sub_803B484();

void sub_803BBEC();

void sub_803C328();

void sub_803CE0C();

void sub_803D20C();

void sub_803D60C();

void sub_803DECC();

u8 sub_803E58C(BattleObj *, u8 *, u8); // 对话对象"到位/插值"演出服务 (gUnk_0300086B: 0 → 3..23); mode 选锚点算法 (0=相对 arg1,1=调色,2/3=对象池中点,4=朝向); 返回 1 表示收尾

u8 sub_803ED34();

void sub_803F21C();

u8 sub_803F328(u8 arg0);

void sub_803F444();

void sub_803F5B4(BattleObj *);

void sub_803F658(BattleObj *);

#endif // BATTLE_STAGE_DIALOGUE_H
