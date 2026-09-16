#ifndef BATTLE_TYPES_H
#define BATTLE_TYPES_H

/* Shared BattleObj / ObjHead / animation-table types split out from code_0.h. */

#include "gba/types.h"
#include "iwram.h"

struct ObjFadeSeq {
    u8 pad0[0xB4];
    u16 f_b4;
    u16 f_b6;
};

/* 战斗对象/精灵命令流控制块 — 0xC8 池对象 (0x02037028) 与独立 IWRAM 实例
 * (0x03000254 处 +0x14, 0x03000918) 的公共头部, 0x30 字节。
 * 仅在战斗主循环 (BattleTask_Run, gMainLoopCallbacks[1]) 内驱动;
 * ObjGfxLoad_Step 按 kind 分步把 gUnk_087EBE00 的 LZ77 块解压到 VRAM/WRAM。
 * 断点现场 (r0=0x03000254): +0x10=0x0856B440 脚本头, +0x18=0x0C03 (kind=3)。
 * 字段命名规则: 已验证语义的字段给语义名, 其余保留偏移名 (见 MOD-04)。 */
typedef struct ObjHead
{
    u16 *cmdBase0;    /* +0x00 命令流0 (u16 命令流; 跳转表辅助; sub_801B570 逆序遍历消费) */
    u16 *cmdBase1;    /* +0x04 命令流1: [0]=跳转表项数 N, +4 起 N 项 u16 偏移表, 条目=u16 对(值,帧号) */
    u16 *jumpTable0;  /* +0x08 流0 的 u16 偏移表 (= cmdBase0 + 2) */
    u16 *jumpTable1;  /* +0x0C 流1 的 u16 偏移表 (= cmdBase1 + 2) */
    const u16 *scriptPtr; /* +0x10 脚本头 (ROM): [0]=cmdBase0 字节偏移(实测恒 4), [1]=cmdBase1 字节偏移(可变 0x14..0x224) */
    const u8 *palBitsPtr; /* +0x14 调色板数据 (DMA 源, sub_804C2FC) */
    u16 kindFlags;    /* +0x18 bits0-3=kind(0x800=DMA禁用/0x200=跳过帧构建/0x8000=激活后清除) */
    u16 f_1A;         /* +0x1A 未验证 */
    u16 frameIdx;     /* +0x1C 当前帧/跳转查找游标 (Obj_FindJumpEntry 入参) */
    u16 f_1E;         /* +0x1E 重置时拷自 +0x24 (未验证) */
    u16 gfxTotal;     /* +0x20 分步装载总数 (ObjGfxLoad_Step 计数上限) */
    u16 gfxPos;       /* +0x22 当前装载片号 (作为 gUnk_087EBE00 索引基 + f_26) */
    u16 vramBank;     /* +0x24 case1 的 OBJ VRAM 槽 (<<?12) / 调色板槽号源 */
    u16 gfxBaseIdx;   /* +0x26 LZ77 块索引基址 (gUnk_087EBE00[gfxBaseIdx+gfxPos]) */
    u8 f_28;          /* +0x28 未验证 (重置清零, ObjGfxLoad_Copy 拷贝) */
    u8 palSlot;       /* +0x29 调色板槽 (sub_804C2FC 实参) */
    u8 f_2A;          /* +0x2A 未验证 */
    u8 f_2B;          /* +0x2B 精灵 X (sub_801B81C arg1 → obj[0x37]; 滑动起点, 上限 0xB4) */
    u8 f_2C;          /* +0x2C 精灵 Y (sub_801B81C arg2 → obj[0x38]; 滑动起点) */
    u8 f_2D;          /* +0x2D 未验证 */
    u8 f_2E;          /* +0x2E 未验证 */
    u8 f_2F;          /* +0x2F 装载完成后回填 +0x29 的值 */
} ObjHead;

/* BattleObj.animPtr 的前 16B u16 索引块视图。
 * +0x00/+0x02/+0x04/+0x06 是主动画索引，+0x08..0x0E 是 fxKind==1 时按 animSubIdx 选择的副索引。
 * 其它 animPtr 字段 (如 +0x1A/+0x20/+0x23/+0x29) 不在此视图中建模。 */
typedef struct ObjAnimIdxBlk
{
    u16 mainAnim[4];
    u16 subIdx[4];
} ObjAnimIdxBlk;

/* 0x083988A8 动画条目表 (24-byte 条目, sub_801CBA4/sub_80401AC 用) */
typedef struct
{
    u8 unk_00[0x14];
    u16 animId;
    u8 unk_16[2];
} AnimEntry24;

extern AnimEntry24 gUnk_083988A8[];

/* 战斗对象 (0xC8 字节) — 池 0x02037028 的 12 个槽 (步长 0xC8, +0xBE 槽号 ≤0xB, 0xFF=空)
 * 与 IWRAM 独立实例 gUnk_03000248 (0x03000248..0x03000310 = 恰好 0xC8) 共用布局。
 * 2026-09-11 zcode 分析, 证据:
 *  - sub_8020F4C(0x03000248) 按 0xC8 对象初始化 (field_BB/BC/B0/BE/36);
 *  - sub_801FA10(obj,kind) 写 +0xB0 bits0-3, 并按 kind 调 sub_801B81C(obj + 0xC, ...);
 *  - 帧/命令分发 call site (code.s 0x080180F8+): 对同一对象 r4 先后
 *      sub_801B8AC(r4+0xC, r4[0x39]) 与 sub_801B8AC(r4+0x3C, r4[0x69]),
 *    且 kind 分别读 `[r4,#0x24]` 与 `[r4,#0x54]` = (0xC+0x18) 与 (0x3C+0x18)
 *    → **对象内置两个 ObjHead** (各 0x30 字节, +0x0C 与 +0x3C);
 *  - sub_8020A0C 用 sub_801B81C((u8*)arg0 + 0x3C, ...) 装配第二个头;
 *  - sub_802151C/sub_802192C/sub_801FAB8/sub_802103C 对 0x03000248 访问 +0x24/+0x37/+0x38/+0xB0/+0xBD/+0xBE;
 *  - BattleTask_Run 尾部 ListNode_InitKey(obj, obj[0x38]) 把对象挂 0x03000318 行动链 → +0x00 是链表头。
 * 语义: +0x00 12B UnkNode (key=+0x38 值), 两个 ObjHead 图形/脚本头, +0xB0 状态字, +0xBE 槽号。 */
typedef struct BattleObj
{
    UnkNode node;                          /* +0x00 key/prev/next (key 由 +0x38=headA.f_2C 值填充) */
    ObjHead headA;                         /* +0x0C 主头 (sub_801B81C(obj+0xC), 801B8AC(obj+0xC)) */
    ObjHead headB;                         /* +0x3C 次头 (sub_801B81C(obj+0x3C), 801B8AC(obj+0x3C)) */
    u16 hp;                                /* +0x6C 当前 HP (E3: sub_80200E8 ← PlayerStats.hp; 801E4D4/E690/8020BC0: hp-=dmgAmount, (s16)≤0 清 0 并死亡入队 030006F8; 801D12C/D19C 与 maxHp 判等选状态; 读 8047024/804A368/802DE04) */
    u16 maxHp;                             /* +0x6E 最大 HP (E3: ← stats.max_hp (sub_80200E8); 804A368 中毒伤 = maxHp/10) */
    u16 mp;                                /* +0x70 当前 MP (E2: ← stats.mp (sub_80200E8); 与 hp/maxHp 同批读 (8047024/804A368)) */
    u16 maxMp;                             /* +0x72 最大 MP (E2: ← stats.max_mp (sub_80200E8)) */
    u16 atc;                               /* +0x74 攻击 = stats.base_atc+equip_atc (E3: sub_80200E8 字节对求和) */
    u16 def;                               /* +0x76 防御 = base_def+equip_def (同上) */
    u16 agl;                               /* +0x78 敏捷 = base_agl+equip_agl (同上) */
    u16 men;                               /* +0x7A 精神 = base_men+equip_men (同上) */
    u16 res;                               /* +0x7C 抗性 = base_res+equip_res (同上) */
    u16 statMods[5];                       /* +0x7E..0x87 能力修正值 [0..4]↔atc/def/agl/men/res: sub_8046F0C case5-9 与 +0x74..0x7C 成对相加截断 u16; sub_8048D40(战斗开始/我方) 与 sub_80200E8(装载) 连清 5×u16; sub_8048CEC 以 [0]/[1] 非零作状态标记 */
    u8 *animPtr;                             /* +0x88 动画/图形数据块指针 ([+2]/[+8+idx*2]/[+0x1A]/[+0x20] 为 u16 索引入口; u8 [+0x23]/[+0x24] 为 f_C3 源 (sub_801CA08 case3/4); 多处 *(u8**) 消费; 低 16 位在 slot≤6 复用为计数/阈值 (sub_80209C8 每次+=4, sub_8048B5C 写 0x20/arg1, sub_801DC20 清 0, sub_801CF90 作渐变阈值 0-32)) */
    u8 field_8C;                           /* +0x8C (sub_80200E8 不写; sub_802192C 以 u32 视图读 +0x98 跨本区) */
    u8 equipSlots[6];                      /* +0x8D..0x92 ← stats.equip_slot1..6 (E3: sub_80200E8 逐字节搬运; 0xB3=空槽哨兵, sub_8048B5C 判 equip5/6 是否为空) */
    u8 pad_93[0x99 - 0x93];                /* +0x93..0x98 未验证 (sub_80200E8 不写) */
    u8 skills[8];                          /* +0x99..0xA0 ← stats.skills[i]-1, 0xFF=空/无效 (E3: sub_80200E8 8 字节循环, 0xFF 与 0x26 视为空) */
    u8 pad_A1;                             /* +0xA1 选中技能槽索引: 0-7 索引 skills[8] (+0x99), >7 时按原值当技能 id。
                                            * E2 消费者: sub_80489A4/sub_8048764 (val<=7 → skills[val] 否则 val) 与
                                            * sub_8045B90 (直接 skills[arg1]) 三处同源访问, 均由本字段取值 */
    u8 substate;                           /* +0xA2 子状态 (sub_801D12C 写状态机值; sub_801CF90 逐帧消费) */
    u8 flashLevel;                         /* +0xA3 闪烁/混色档 (sub_8020D50 写 sub_804BBDC 混色结果并置 state bit7; sub_801CF90 读取) */
    u8 pad_A4[0xA9 - 0xA4];                /* +0xA4..0xA8 未验证 */
    u8 noa;                                /* +0xA9 ← stats.noa (E2: sub_80200E8) */
    u8 lv;                                 /* +0xAA 等级 ← stats.lv (E2: sub_80200E8) */
    u8 variantClass;                       /* +0xAB 形态/类别: 0=基础动画, 非0→动画取变色变体(基索引+2, 801CA08/801CE80), 8=死亡(全场效果跳过 801DEDC/DF90/E4D4/E690), ==5→flag|0x20 (801CA08), 背景对象复用: ==4→调色基址取 gUnk_03000744 (801ED40/801EE6C); 出场清 0 (801B964/80200E8/801DD04), 战斗脚本可写 7 (8033E2C/8038920/8039724) */
    u8 pad_AC[0xB0 - 0xAC];                /* +0xAC..0xAF (+0xAC ← sub_80200E8 arg2; sub_8022710 以 obj+0xAC+r6 变址读) */
    u16 state;                             /* +0xB0 bits0-3=kind, bits4-7=子态(0x10/0x20/0x60), 0x400=不入链, 0x2000=跳跃; bit0=逐帧清 (801CF90), bit1=请求重建渐变LUT (801CF90 清), bit7=调色板取 flashLevel (801CF90) */
    u16 dmgAmount;                         /* +0xB2 待结算伤害/数值累计 (E3: 中毒=maxHp/10 (804A368) → sub_801D568 弹 3 位数字 (显示夹 ≤999; 9999 上限 803B484) → hp-=本值 (801E4D4/E690/8020BC0); 战斗脚本族 8040EE8/8041308/80416F0/80419E0/8041EDC/80422B8/8042784/8042B90/8042E70/8043554/8043938/8043B5C/8043F90 直写; 入队清 0 后逐帧 +1 作概率权重 (801FF40/8020AE4: (n+1)*40 vs rand%101, n>4 才可被选)) */
    u16 f_B4;                              /* +0xB4 动画表项 +0xC 装载 (801CA08 case3/4, 801CE80 case1/5); 战斗对象运算大量读写 (语义未定) */
    u16 f_B6;                              /* +0xB6 同上 = 表项 +0xE */
    u16 statusAil;                         /* +0xB8 异常状态位域 (bit0=中毒: 804A368 ands 1 → 中毒结算分支; 另读 801EEE4/801F3FC/804A148/804AA2C) */
    u8 pad_BA[0xBB - 0xBA];                /* +0xBA 未验证 */
    u8 memberIdx;                          /* +0xBB 出场 = sub_80487CC(memberId) = 队伍位次 0-5 (0x03004A88[] 查表; 0xA1/0xA7 特例 → 2); 801FF40 副代表选取按本值判重; 敌群路径写 r8-5; 动画流可写 (均 801B964) */
    u8 fxKind;                             /* +0xBC 效果/攻击型别 (s8): 0/1 选 animPtr 偏移模式 (801DEDC/DF90/E4D4/E690/802093C); 出场 =0xFF (801B964); sub_804CEE0 写 0/1, 80466F0 写 */
    u8 f_BD;                               /* +0xBD 目标对象的 obj 池索引 (0-4 玩家侧候选/0xFF 空): 801EA70 写入 == 本槽目标选取结果, 801F884/802192C/804E2AC 按 GetObjPool()+f_BD*0xC8 取目标对象; 出场清 0 (801B964); 常见值 0-5 (8023820 ×7, 80230BC=5, 802103C=arg1, 80466F0); >4 时 sub_8020DF0 转全体扫描 */
    u8 slot;                               /* +0xBE 槽号 (≤0xB 玩家侧, ≤0x70 敌方, 0x71+ 特效/特殊; 0xFF=空) */
    u8 posX;                               /* +0xBF 屏幕坐标 X (E3: 出场 ← 阵型表 tbl[id*4+2] (801B964); 801D568 弹数字锚 X-16; headB 装配 (801DB3C/8020A0C/80210C0); 802B608 拷到 0x03000828; 移动族大量读写) */
    u8 posY;                               /* +0xC0 屏幕坐标 Y (同上: tbl[id*4+3]; 弹数字 Y-8; 0x03000829) */
    u8 pad_C1;                             /* +0xC1 出场清 0 (801B964) */
    u8 animSubIdx;                         /* +0xC2 动画副索引 (animPtr+8+idx*2 选表项; 801DF90/801E690/803F444) */
    u8 f_C3;                               /* +0xC3 动画附属参数 (801CA08 case3/4 写 animPtr[0x23]/[0x24]; 出场=0x10 (80200E8); 80264C0/803E58C/803ED34/8040690/80419E0/8042E70 等战斗对象逐帧大量读取; 语义未定) */
    u8 pad_C4[0xC8 - 0xC4];                /* +0xC4..0xC7 (出场 +0xC4=0x10 (80200E8); 8020228/802031C 写) */
} BattleObj;

/* 0x08393B28 战斗对象动画/特效资源表 (E0: 992 项 × 0x14B, 0x08393B28..0x083988A8 全部
 * animScriptPtr 为合法 ROM 命令流指针; 项内两个 ROM 指针 + 图形参数)。BattleObj.animPtr 数据块中的
 * u16 索引 (+0/+2/+6/+8+idx*2/+0x1A/+0x20) 均指向本表 (801CA08/801CE80/801EA70/801F884 族)。
 * animScriptPtr/palettePtr = ROM 数据指针, gfxBaseIdx/gfxTotal/field_C/field_E = 装配参数
 * (sub_801B81C headA/headB; field_C/E 另复制到 obj->f_B4/f_B6, 801CA08 case3/4, 801CE80 case1)。
 *   animScriptPtr (+0x00) -> ObjHead.scriptPtr; 指向 u16 动画/命令流头。
 *   palettePtr    (+0x04) -> ObjHead.palBitsPtr; 指向调色板 DMA 源。
 *   gfxBaseIdx    (+0x08) -> ObjHead.gfxBaseIdx; gUnk_087EBE00 的 LZ77 图形块索引基址。
 *   gfxTotal      (+0x0A) -> ObjHead.gfxTotal; 分步装载图形片数。
 * targetMode (+0x10, 原 field_10) = 目标作用模式, E2 三组独立消费者:
 *   - sub_801F884 目标匹配键 (敌方 0xB..0x70): 0=取 +0xAC 原值, 1=0(调用者不过滤),
 *     2=低 nibble(==2→1), 3=高 nibble(==0x20→0x10);
 *   - sub_801DEDC/DF90/E4D4/E690 效果弹数字: 0=单体, 1=全场;
 *   - slot≥0x71 特殊对象目标选取族 (sub_804D1B4..804DCD8): 0=f_BD=随机存活候选, 1=f_BD=0。
 * 表值分布 (E0): targetMode 958×0, 24×1, 1×2, 9×3。 */
typedef struct ObjAnimEntry
{
    const u16 *animScriptPtr; /* +0x00 ObjHead.scriptPtr 源 */
    const u8 *palettePtr; /* +0x04 ObjHead.palBitsPtr 源 */
    u16 gfxBaseIdx; /* +0x08 gUnk_087EBE00 索引基址 */
    u16 gfxTotal; /* +0x0A 分步装载图形片数 */
    u16 field_C; /* 复制到 obj->f_B4 (语义未定) */
    u16 field_E; /* 复制到 obj->f_B6 (语义未定) */
    u16 targetMode;
    u16 pad_12;
} ObjAnimEntry;

extern const ObjAnimEntry gUnk_08393B28[];

#endif // BATTLE_TYPES_H
