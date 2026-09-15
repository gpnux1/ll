#include "code_0.h"
#include "data_805769C.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"
#include "sound.h"

/* BattleObj.animPtr 动画索引块视图 (与 ObjAnimEntry 配对): u16 成员均为 gUnk_08393B28
 * 索引。slot≥0x71 特殊对象的块来自 gUnk_0839ABCC[field_BE*0x40] (sub_802031C, 0x40 字节/项):
 * +0x00/+0x02/+0x04/+0x06 为主动画索引 (sub_801CE80 kind0/1/2/6; fxKind==0 路径用 +0x02),
 * +0x08 的 4 项为动画副索引表 —— fxKind==1 时按 obj->animSubIdx 选取 (同 sub_801CE80 kind5),
 * 选中表项的 targetMode 决定 f_BD 赋值模式 (0=随机存活候选, 1=0)。 */

/* gUnk_08393B28 (ObjAnimEntry, 见 code_0.h): 本文件族 = slot≥0x71 特殊对象的目标选取
 * handler, 经 sub_804DD70 分派表 gUnk_0839CE38[slot-0x71] 进入 (调用者 sub_801EA70/sub_8020C58)。
 * targetMode==0 → f_BD=values[] 中随机存活候选; ==1 → f_BD=0。 */

// @ 0x0804D1B4
void sub_804D1B4(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 15)
        obj->fxKind = 1;
    else
        obj->fxKind = 0;
    switch ((s8)obj->fxKind)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            obj->animSubIdx = 0;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[0]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D260
void sub_804D260(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 10)
        obj->fxKind = 1;
    else
        obj->fxKind = 0;
    switch ((s8)obj->fxKind)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            obj->animSubIdx = 0;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[0]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D310
void sub_804D310(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    u8 *flag;
    ObjAnimEntry *entry;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 10)
    {
        flag = &obj->fxKind;
        *flag = 1;
    }
    else
    {
        flag = &obj->fxKind;
        *flag = 0;
    }
    *flag = 0;
    entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D3A0
void sub_804D3A0(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 13)
        obj->fxKind = 1;
    else
        obj->fxKind = 0;
    switch ((s8)obj->fxKind)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            obj->animSubIdx = 0;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[0]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D44C
void sub_804D44C(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 10)
        obj->fxKind = 1;
    else
        obj->fxKind = 0;
    switch ((s8)obj->fxKind)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            obj->animSubIdx = 0;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[0]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D4FC
void sub_804D4FC(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;
    s8 *flag;
    unsigned int v;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 10)
    {
        flag = &obj->fxKind;
        obj->fxKind = 1;
    }
    else
    {
        flag = &obj->fxKind;
        obj->fxKind = 0;
    }
    v = *flag;
    switch (v)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            v &= Rng_LcgNext();
            obj->animSubIdx = v;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D5B4
void sub_804D5B4(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 value;
    u8 count;
    u8 rand;
    ObjAnimEntry *entry;
    u8 i;
    unsigned int victory;

    victory = 0;
    count = sub_80489E8(pool, values, 1, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 10)
        obj->fxKind = 1;
    else
        obj->fxKind = victory;
    if (count == 2)
    {
        victory = 1;
        for (i = 0; i < 2; i++)
        {
            if (obj->pad_AC[0] == pool[values[i]].pad_AC[0])
                continue;
            if ((s8)pool[values[i]].fxKind != 1)
                break;
            if (pool[values[i]].animSubIdx != (count = 1)) /* count 仅作 1 的载体, 下行起即被重算 */
                break;
            obj->fxKind = 3;
            break;
        }
    }
    count = sub_80489E8(pool, values, 0, 0x6F);
    switch ((s8)obj->fxKind)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            if (victory == 1 && Rng_LcgNext() % 0x64 <= 0x31)
                obj->animSubIdx = victory;
            else
                obj->animSubIdx = 0;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D708
// 概率判定+drop道具。⚠ 2026-09-03 还原 INCLUDE_ASM: 原 C 代码比 ROM 少 4 字节
// (ROM 尾部死 store `movs r0,#0; strb r0,[obj+0xBC]` 被 C 编译器优化掉),
// 直接导致全局 +4 位移 bug。待用 do-while 屏障/中间变量复现死 store 后重匹配。
void sub_804D708(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    u8 *flag;
    ObjAnimEntry *entry;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 10)
    {
        flag = &obj->fxKind;
        *flag = 1;
    }
    else
    {
        flag = &obj->fxKind;
        *flag = 0;
    }
    *flag = 0;
    entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}

// @ 0x0804D798
void sub_804D798(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    u8 *flag;
    ObjAnimEntry *entry;
    unsigned int v;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 10)
    {
        flag = &obj->fxKind;
        *flag = 1;
    }
    else
    {
        flag = &obj->fxKind;
        *flag = 0;
    }
    *flag = 1;
    v = Rng_LcgNext();
    v &= 3;
    obj->animSubIdx = v;
    entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D840
void sub_804D840(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;
    s8 *flag;
    unsigned int v;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 <= 0x45)
    {
        flag = &obj->fxKind;
        obj->fxKind = 1;
    }
    else
    {
        flag = &obj->fxKind;
        obj->fxKind = 0;
    }
    v = *flag;
    switch (v)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            v &= Rng_LcgNext();
            obj->animSubIdx = v;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804D8F4
void sub_804D8F4(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;
    u16 gold;
    u8 lucky;
    unsigned int v;

    lucky = 0;
    gold = obj->maxHp / 10 << 2;
    count = sub_80489E8(pool, values, 0, 0x6F);
    if ((obj->state & 0x400) == 0 && obj->hp < gold)
    {
        obj->fxKind = 1;
        obj->state |= 0x400;
        lucky = 1;
    }
    else
    {
        if (Rng_LcgNext() % 0x65 <= 0x45)
            obj->fxKind = 1;
        else
            obj->fxKind = 0;
    }
    switch (v = (s8)obj->fxKind)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            if (lucky == 0)
                obj->animSubIdx = lucky;
            else
                obj->animSubIdx = v;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804DA04
void sub_804DA04(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;
    unsigned int kind;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 <= 0x45)
        obj->fxKind = 1;
    else
        obj->fxKind = 0;
    switch ((s8)obj->fxKind)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            kind = Rng_LcgNext() % 3;
            obj->animSubIdx = kind;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804DABC
void sub_804DABC(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    u8 v;
    u8 *flag;
    u8 *subIdx;
    ObjAnimEntry *entry;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 <= 0x45)
    {
        flag = &obj->fxKind;
        *flag = 1;
    }
    else
    {
        flag = &obj->fxKind;
        *flag = 0;
    }
    *flag = 1;
    v = Rng_LcgNext() & 3;
    subIdx = &obj->animSubIdx;
    *subIdx = v;
    if (v == 1)
        *subIdx = 0;
    entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[*subIdx]];
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804DB64
void sub_804DB64(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    s8 *flag;
    unsigned int v;
    ObjAnimEntry *entry;
    unsigned int kind;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 <= 0x45)
    {
        flag = &obj->fxKind;
        obj->fxKind = 1;
    }
    else
    {
        flag = &obj->fxKind;
        obj->fxKind = 0;
    }
    v = *flag;
    switch (v)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            kind = Rng_LcgNext() % 5;
            obj->animSubIdx = kind;
            if ((u32)obj->animSubIdx == 2)
                obj->animSubIdx = v;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804DC24
void sub_804DC24(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;
    s8 *flag;
    unsigned int v;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x64 <= 0x3B)
    {
        flag = &obj->fxKind;
        obj->fxKind = 1;
    }
    else
    {
        flag = &obj->fxKind;
        obj->fxKind = 0;
    }
    v = *flag;
    switch (v)
    {
        case 0:
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->mainAnim[1]];
            break;
        case 1:
            v &= Rng_LcgNext();
            obj->animSubIdx = v;
            entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
            break;
    }
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
        case 1:
            obj->f_BD = 0;
            break;
    }
}
// @ 0x0804DCD8
void sub_804DCD8(BattleObj *obj, BattleObj *pool)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 rand;
    ObjAnimEntry *entry;
    s8 *flag;
    u8 zero;

    count = sub_80489E8(pool, values, 0, 0x6F);
    if (Rng_LcgNext() % 0x65 < count * 10)
    {
        flag = &obj->fxKind;
        obj->fxKind = 1;
    }
    else
    {
        flag = &obj->fxKind;
        obj->fxKind = 0;
    }
    zero = 0;
    obj->fxKind = 1;
    obj->animSubIdx = zero;
    entry = &gUnk_08393B28[((ObjAnimIdxBlk *)obj->animPtr)->subIdx[obj->animSubIdx]];
    switch (entry->targetMode)
    {
        case 0:
            rand = Rng_LcgNext();
            value = values[rand % count];
            obj->f_BD = value;
            break;
            while (value) break; /* 调度屏障: 触发 agbcc global-alloc 复用 r1 存 0 + 拷地址到 r2 (零行为, 两路皆 break) */
        case 1:
            obj->f_BD = 0;
            break;
    }
}
