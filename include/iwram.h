#ifndef _IWRAM_H
#define _IWRAM_H

#include "gba/types.h"
#include "menu.h"

#define NULL 0

typedef struct UnkNode
{
    u8 key; // offset 0, 用于比较排序
    u8 pad[3];
    struct UnkNode *prev; // offset 4
    struct UnkNode *next; // offset 8
} UnkNode;

struct LzContext
{
    u8 *dest;
    u8 *src;
    u8 *flags;
    u32 unkC;
    u32 bitIndex;
    u32 size;
    u32 processedSize;
    u32 remainingSize;
};

struct LzHeader
{
    u32 uncompressedSize;
    u32 size;
    u8 data[1];
};

typedef struct LzHeader LzHeader;
#define Unk_LzData LzHeader

extern u16 gUnk_03000000;
extern u16 gUnk_03000002;
extern u16 gUnk_03000004;
extern u8 gLogoAnimDirection;
#define gUnk_03000008 gLogoAnimDirection
extern u8 gLogoSpriteNodes[2];
#define gUnk_0300000A gLogoSpriteNodes
extern u16 gLogoAnimTimer;
#define gUnk_0300000C gLogoAnimTimer
extern u8 gUnk_03000010[4];
extern u8 gUnk_03000014[4];
extern u8 gUnk_03000018[4];
extern u16 gUnk_03000020[4];
extern u32 gUnk_03000028[4];
extern u8 *gUnk_03000038[4];

typedef struct
{
    u8 field_0;
    u8 field_1;
    u8 field_2;
    u8 field_3;
    u16 field_4;
    u16 field_6;
    u16 field_8;
    u16 field_A;
    u16 field_C;
    u16 field_E;
} Unk_03000048;
extern Unk_03000048 gUnk_03000048;

typedef struct
{
    /* 0x00 */ u8 statusFlags; // 状态标志 (bit7=激活; bit0=tile 动画模式; bit1=固定 tile+0x60;
                               //           bit2/3=attr2 变体: 4=换字块-0x5000, 4|8=-0x6000, 其他=直写)
    /* 0x01 */ u8 animTimer; // 动画帧计数 (bit3 翻转, 每翻转切换 +0x10 图块)
    /* 0x02 */ u8 lerpFrame; // 移动插值倒计时 (8 -> 0; =0 静止)
    /* 0x03 */ u8 oamSlotId; // 关联的渲染层 SpriteNode 池槽
    /* 0x04 */ s16 x; // 当前实时显示 X 坐标
    /* 0x06 */ s16 y; // 当前实时显示 Y 坐标
    /* 0x08 */ s16 moveEndX; // 移动终点 (插值到位后贴合; 由 UiSprite_BeginSlide 写入)
    /* 0x0A */ s16 moveEndY;
    /* 0x0C */ s16 moveStartX; // 移动起点 (写入时保存的当前 x/y; 证据 UiSprite_BeginSlide: [4]->[0xC])
    /* 0x0E */ s16 moveStartY;
    /* 0x10 */ u16 baseTileId; // 基础图块起始 ID
    /* 0x12 */ u16 pad;
} UISpriteEntity; // Total Size: 0x14 (20 bytes)

/* 0x03000058 起: UI 精灵实体数组 (15 项, 前 5 项 = 队伍成员)。
 * 旧名 Unk_03000058 与本 typedef 重复, 已合并为一个。*/
extern UISpriteEntity gUiSprites[]; /* 实体[0..14] @0x03000058 */
extern UISpriteEntity gUiSpritesAux[]; /* == &gUiSprites[5] (偏移 0x64, 非队伍辅助实体) */

extern u8 gUnk_03000184;
extern u8 gUnk_03000185;
extern u8 gMenuCursorGrp;
extern u8 gMenuCursorSel;
extern u8 gMenuCursorStack[];
extern u8 gUnk_03000198;
extern u8 gInvCursor2;
extern u8 gInvViewState[];
extern u8 gUnk_030001AE;
extern u8 gUnk_030001AF;
extern u8 gUnk_030001B0;
extern u8 gUnk_030001B1; /* 0x030001B1: 道具目标向下搜索开关 (非 0 才搜"上一个") */
extern u8 gItemUseCtx[];
extern u8 gUnk_030001B9; /* 0x030001B9: 上一个可用目标 (0xFF=无) */
extern u8 gUnk_030001BA; /* 0x030001BA: 下一个可用目标 (0xFF=无) */
/* 0x030001BC: 7 个装备属性差值指示字形 id (0xB=平/0xC=升/0xD=降);
 * 索引序 atk(0),def(1),agl(2),men(3),res(4),luc(5),noa(6)。
 * 由 sub_800FF10 填写, sub_800B374 读取后交给 Text 渲染。 */
extern u8 gStatArrowIds[];
/* 0x030001C3: 当前菜单选中的道具/技能 id (sub_800F128 写入, sub_8010300/sub_8010770 读取; 0x3E=无角色, 0x26=传送) */
extern u8 gUnk_030001C3;
extern u8 gUnk_030001C4; /* 0x030001C4: sub_8010300 写入的 MP 消耗量, sub_8010770 扣减 */
extern u8 gUnk_030001C5; /* 0x030001C5: 道具表 [1]&0xF (使用类型) */
extern u8 gUnk_030001C6; /* 0x030001C6: 道具表 [3] (回复量) */
extern u16 gUnk_030001C8;

extern u8 *gMsgTable[];

extern u8 gSaveCurSlot;
extern u8 gSaveModeFlag;
extern u8 gSkillMenuTmpA;
extern u8 gSkillMenuTmpB;
extern u8 gPartyMenuIdx;
extern u8 gSkillMenuPage;
extern u8 gUnk_03000229;
extern u8 gUnk_03000204;
extern u8 gUnk_03000208;
extern u32 gUnk_03000210;
extern u8 gUnk_0300022B;
extern u32 gUnk_0300022C;
/* Option sound-test state. BGM/SFX rows are displayed as ??? until a valid
 * save enables them. The playing BGM byte stores id+1 so zero means stopped. */
extern u16 gSoundTestSfxId;
#define gCardAlbumCursor gSoundTestSfxId
#define gUnk_03000230 gSoundTestSfxId
extern u8 gSoundTestBgmId;
#define gCardAlbumPage gSoundTestBgmId
#define gUnk_03000232 gSoundTestBgmId
extern u8 gSoundTestPlayingBgmIdPlusOne;
#define gCardCursorY gSoundTestPlayingBgmIdPlusOne
#define gUnk_03000233 gSoundTestPlayingBgmIdPlusOne
/* Number of PRESS START update frames left before attract mode. */
extern u8 gTitleAttractCountdown;
#define gTitleFadeStep gTitleAttractCountdown
#define gUnk_03000234 gTitleAttractCountdown
extern u16 gCardRecvId;
#define gUnk_03000236 gCardRecvId
extern u16 gCardSendId;
#define gUnk_03000238 gCardSendId
extern u8 gUnk_03000240;
extern u8 *gObjPoolPtr;
extern u32 gUnk_03000248;
extern u16 gUnk_03000310;
extern u16 gGstate312;
extern u16 gGstate314;
extern u8 gUnk_03000316;
extern u8 gUnk_03000317;
extern UnkNode gUnk_03000318; // 战斗待机行动链表头 (ListNode_Init)
extern u16 gGstate324;
extern u8 gUnk_03000344; // VBlank 战斗流水线结果暂存: sub_8018070 入口置 0x7F, 出口写本帧返回值
extern u32 gBattleRngSeed;
extern u16 gUnk_0300032C;
extern u8 gGstate32E;
extern u16 gGstate330[];
extern u32 gGstate340;
typedef struct
{
    u8 padding0[8];
    u8 field_8;
    u8 field_9;
    u8 field_A;
    u8 field_B;
    u16 field_C;
    u16 field_E;
    u32 field_10;
    // u8 padding1[4];
} Unk_03000348;
extern Unk_03000348 gDialogCtx[];

extern u16 gFlashFlags;
extern u16 gUnk_03000386;
extern u16 gUnk_03000390[];

extern u8 gUnk_030004D4;
extern u8 gUnk_030004D5;
extern u8 gUnk_030004D6;
extern u8 gUnk_030004D7;
extern u32 gUnk_030004D0;
extern u32 gUnk_030004D8[4];
extern u32 gUnk_030004E8[4];

extern u8 gUnk_030004F8;

typedef struct
{
    u16 field_0;
    u16 field_2;
    u16 field_4;
    u16 field_6;
    u16 field_8;
    u16 field_A;
    u16 field_C;
    u16 field_E;
} Unk_03000500;
extern Unk_03000500 gUnk_03000500;
extern u16 gBattleUiFlags;
extern u8 gUnk_03000512;
extern u8 gUnk_03000514;

extern u8 gObjFlagsA[0x80];
extern u8 gObjFlagsB[0x80];

extern u16 gUnk_03000618;
extern u16 gUnk_0300061A;
extern u16 gUnk_0300061C;
extern u16 gUnk_0300061E;
extern u16 gUnk_03000620;
extern u16 gUnk_03000622;
extern u8 gUnk_03000624;
extern u8 gUnk_0300068C;
extern u8 gUnk_0300068D;
extern u8 gUnk_0300068E;
extern u32 gUnk_0300062C;
extern u8 gUnk_03000630;
extern u8 *gUnk_03000638[12];
extern u8 gUnk_03000668;
extern u8 gUnk_03000669;
extern struct BattleObj *gUnk_030006F8[]; /* 战斗效果/步进队列 (7 槽), 消费者 sub_801E040 */
extern u8 gUnk_03000714;
extern u8 gUnk_03000715;
extern u8 gUnk_03000716;

/* 战斗"待选池"链节点 (16B, 步长 0x10): 前 12 字节就是 UnkNode, 故 gUnk_030006A0[i]
 * 可直接强转 UnkNode* 交给 ListNode_InitKey/InsertSorted (见 sub_801DC20)。
 * 数组 0x030006A0..0x030006F0 共 5 项 (索引 = obj 池槽 0..4, 由 sub_80489E8 mode0 产出);
 * 链表头 0x03000690 是 UnkNode 哨兵 (ListNode_Init, key=0xFF), 遍历条件恒为 key <= 0xFE。
 * data 指向 BattleObj*。消费者: sub_801DC20 (挂入), sub_801DD04 (摘链),
 * sub_801FF40 (挑选, 读 data 对象 memberIdx), sub_8020AE4 (逐帧 dmgAmount+1)。 */
typedef struct Unk_030006A0
{
    u8 key;
    u8 pad_1[3];
    struct Unk_030006A0 *prev;
    struct Unk_030006A0 *next;
    u32 data;
} Unk_030006A0;
extern Unk_030006A0 gUnk_030006A0[];
extern UnkNode gUnk_03000690;  /* 待选池链表头: UnkNode 哨兵 (key=0xFF), 遍历取 ->next */
extern u8 gUnk_030006F0;       /* 待选池计数: sub_801DC20 挂入时 ++, sub_801DD04 摘链时 -- */
extern u32 gUnk_03000718;
extern u8 gUnk_0300071C;
extern u32 gUnk_03000730;
extern u8 gUnk_0300073C;
extern u8 gUnk_0300073D;
extern u8 gUnk_03000744;
extern u8 gUnk_03000765;
extern u8 gUnk_08393A30[];
extern u8 gUnk_0839DF90[]; ///< sub_802A154 case30/36 用的 (x,y) 对表 (各 2 字节)
extern float gCosTable[];
extern float gSinTable[];
extern u8 gUnk_03000748[];
extern u8 gUnk_03000758[];
extern u8 gUnk_03000763;
extern u8 gUnk_03000768;
extern u8 gUnk_03000769;
extern u8 gUnk_0300076B;
extern u16 gUnk_0300076C;
extern u16 gUnk_0300076E;
extern s8 gUnk_0300076A;
extern u16 gUnk_0300076C;
extern u16 gUnk_0300076E;
extern u8 gUnk_03000770;
extern u8 gUnk_03000781;
extern u8 gUnk_03000782;
extern u16 gUnk_03000784;
extern u8 gMenuSlotStates[][5];
#define gUnk_03000788 gMenuSlotStates
extern u8 gUnk_03000808;
extern u8 gUnk_03000809;
extern u8 gUnk_0300080A;
extern u8 gMenuMasterCursor;
#define gUnk_030007BA gMenuMasterCursor
extern u8 gUnk_0300080C[];
extern u8 gUnk_03000811;
extern u8 gUnk_03000812;
extern u8 gUnk_03000813;
extern s8 gUnk_03000814;
extern s8 gUnk_03000815;
extern u8 gUnk_03000816;
extern u16 gUnk_03000818;
extern u8 gUnk_03000820;
extern u16 gUnk_03000822;
extern u8 gUnk_03000824;
extern u8 gUnk_03000825;
extern u16 gUnk_03000826;
extern u8 gUnk_03000828;
extern u8 gUnk_03000829;
extern u8 gUnk_03000830[]; ///< 0xC 字节 ID 缓冲 (0x830..0x83B), sub_80489E8 的输出表
extern u8 gUnk_0300083C;  ///< 上表的项数
extern u8 gUnk_0300083D;
extern u8 *gUnk_03000840;
extern u8 gUnk_03000844;
extern u8 gUnk_03000845;
extern u8 gUnk_03000856;
extern u8 gUnk_03000857;
extern u8 gUnk_03000858;
extern u16 gUnk_0300085A;
extern u8 gUnk_0300085C;
extern u8 gUnk_03000865;
extern u8 gUnk_03000867;
extern u8 gUnk_03000868;
extern u8 gUnk_0300086A;
extern u8 gUnk_0300086B;
extern u16 gUnk_0300086C; ///< sub_803E58C 动画基准表项 (0x350/0x353/0x356, 加 1/2 变体)
extern u8 gUnk_0300086E;
extern u8 gUnk_0300086F; ///< sub_802D728 锚点动画源坐标 X/Y (obj->posX+0x1D / obj->posY-0x2F)
extern u16 gUnk_03000882;
extern u8 gUnk_03000884;
extern u16 gUnk_03000886;
extern u8 gUnk_03000888;
extern u8 gUnk_03000889;
extern u8 gUnk_0300088B; ///< sub_80489E8 返回的参演角色数 (sub_8028AD8 登记)
extern u8 gUnk_0300088C; ///< 当前处理角色下标 (sub_8028AD8)
extern u8 gUnk_03000890[]; ///< 角色 obj 池槽号表 (sub_80489E8 输出, 按 0x8C 下标; sub_8028AD8)
extern u8 gUnk_03000898[]; ///< 暂存 posX (sub_8028AD8)
extern u8 gUnk_0300089A[]; ///< 暂存 posY (sub_8028AD8)
extern u8 gUnk_0300089C[]; ///< 暂存 headA.palSlot (sub_8028AD8)
extern u16 gUnk_030008A0[]; ///< 暂存 headA.f_1E (sub_8028AD8)
extern u8 gUnk_030008A4; ///< 当前相位/组下标 (sub_8028AD8)
extern u8 gUnk_030008A5; ///< sub_802A154 演出序号 (0..2, 索引 gUnk_0839DF90 的 (x,y) 对)
extern u32 gUnk_030008EC;
extern u8 gChoiceListLen;
extern u8 gUnk_0300094A;
extern u8 gUnk_0300094B;
extern u8 gUnk_0300094C;
extern u8 gUnk_0300094D;
extern u8 gUnk_03000949; // 2026-09-11 zcode-engine 登记 (sub_8048DA4)
extern u32 gUnk_03000950;
extern u8 gUnk_03000954; // 2026-09-11 zcode-engine 登记 (sub_8048DA4)
extern u16 gUnk_03000956;
extern u8 gUnk_03000958;
extern u16 gUnk_0300095A; // 2026-09-11 zcode-engine 登记 (sub_8048DA4)
extern u8 gUnk_03000960[];
extern u8 gUnk_03000968;
extern u8 gUnk_03000969;
extern s8 gUnk_030009BE;
extern s8 gUnk_030009BF;
extern u32 gUnk_030009C0;
extern u8 gUnk_030009C4;
extern s8 gUnk_030009C5;
extern u32 *gUnk_030009C8;
extern u8 gChoiceSubIdx;
extern u8 gUnk_030008F0;
extern u8 gUnk_030008F1;
extern u8 gUnk_030008F2;
extern u8 gUnk_030008F3;
extern u16 gUnk_03000906;
extern u16 gUnk_03000908;
extern u8 gBattleIntroState;      ///< 战斗开场 BGM/淡入演出状态机 (sub_8049C1C): 0..4
extern u8 gBattleIntroTimer;      ///< 上述状态机的帧计数
extern u8 gBattleIntroObj[];      ///< 战斗开场对象的 ObjHead (sub_804AD60 用 sub_801B81C 装配)
extern u8 gBattleIntroPhase;      ///< 开场阶段标志 (0/1/2, sub_804ADE0/ADF8 设置)
extern u16 *gUnk_0300096C;
extern u8 gUnk_03000974[]; // 2026-09-11 zcode-engine 登记 (sub_8048DA4)
extern u8 gUnk_03000979; // 2026-09-11 zcode-engine 登记 (sub_8048DA4)
extern u8 gUnk_0300097A; // 2026-09-11 zcode-engine 登记 (sub_8048DA4)
extern u8 gUnk_0300097B;
extern u8 gUnk_0300097C;
extern u8 gUnk_0300097D;
extern u8 gBattleIntroFadeFlag;   ///< sub_804ADF8 复位 (淡入/开场演出标志)

/* 战斗转场/擦除 (wipe) 效果 (sub_804AE2C 逐帧更新, sub_804B1EC 复位, sub_804B1F8 启动):
 * gWipeDesc 指向转场源对象 (sub_804B1F8 的首参); 其 +0x2D/+0x2E 给出受影响的 OAM 下标范围
 * (sub_804AE2C 取 gWipeOamStart = +0x2D, gWipeOamEnd = +0x2D - +0x2E)。
 * gWipeCtl: bit0=运行中, bit1=已捕获 OAM, bits4-7=效果类型 (0x10=上/下擦除)。 */
typedef struct
{
    u8 pad[0x2D];
    u8 field_2D;   /* +0x2D 受影响 OAM 下标上界 (gWipeOamStart) */
    u8 field_2E;   /* +0x2E 受影响 OAM 下标基准 (gWipeOamStart - gWipeOamEnd = 计数) */
} WipeDesc;
extern WipeDesc *gWipeDesc;
extern u16 gWipeSavedH[];   ///< 逐 OAM 保存的原始 HPos
extern u8 gWipeSubframe;    ///< 5 帧子计数 (归零时推进 gWipeProgress)
extern u8 gWipeOamStart;    ///< 转场影响的 OAM 起始下标
extern u8 gWipeOamEnd;      ///< 转场影响的 OAM 结束下标
extern u8 gWipeMinY;        ///< 受影响 OAM 的最小 VPos
extern u8 gWipeMaxY;        ///< 受影响 OAM 的最大 VPos+高度
extern u8 gWipeProgress;    ///< 擦除推进量 (遮挡高度)
extern u16 gWipeCtl;        ///< 转场控制字
extern u16 gUnk_03000AE0;
extern u16 gUnk_03000AE2;
extern u8 gUnk_03000AE4; // 2026-09-11 zcode-engine 登记 (sub_804B288)
extern u8 gUnk_03000AE5; // 2026-09-11 zcode-engine 登记 (sub_804B288)
/* 调色板动画条目 (16 字节 × 16 项 = 256 字节)。
 * 0x03000AE8 = BG 调色板动画表 (目的 0x05000200 / 镜像 0x02036AC0);
 * 0x03000BE8 = OBJ 调色板动画表 (目的 0x05000000 / 镜像 0x02036CC0); 布局相同。
 * ctrl 低 4 位 = opcode: 0=空(0xFF), 1=流式(sub_804B3C0), 2=精灵动画(sub_804B458),
 * 3=淡变(sub_804B4D0); bit4=0x10 方向, bit5=0x20 禁止颜色重置, bit6=0x40 循环方向。
 * RGB 增量 (dR/dG/dB) 与 shift 供 sub_804B56C 做 src + delta*weight>>shift 插值。 */
typedef struct
{
    u8 ctrl;       /* +0x0 控制字/opcode */
    u8 palSlot;    /* +0x1 目标调色板槽 (<<4 或 <<5 索引); 作有符号槽号时由使用处强转 (s8) */
    u8 period;     /* +0x2 周期/总帧数 */
    u8 counter;    /* +0x3 当前帧计数器 */
    u8 span;       /* +0x4 低4位=宽, 高4位=帧数 */
    u8 pad5;       /* +0x5 */
    u16 frameIdx;  /* +0x6 当前帧索引 */
    u8 dir;        /* +0x8 往返方向 */
    u8 pad9[3];    /* +0x9..0xB */
    s8 dR;         /* +0xC R 增量 */
    s8 dG;         /* +0xD G 增量 */
    s8 dB;         /* +0xE B 增量 */
    u8 shift;      /* +0xF 插值移位/除数 */
} PaletteAnimEntry;
extern PaletteAnimEntry gBgPalAnim[];   /* 0x03000AE8 BG 调色板动画表 (目的 0x05000200 / 镜像 0x02036AC0) */
extern u16 gUnk_03000CE8; // 2026-09-11 zcode-engine 登记 (sub_804B288)
extern PaletteAnimEntry gObjPalAnim[];  /* 0x03000BE8 OBJ 调色板动画表 (目的 0x05000000 / 镜像 0x02036CC0) */
extern u8 gObjTargetCache[]; ///< 每 obj 池槽 (0..10) 缓存的目标对象索引 (f_BD): 0xFF=未选, sub_804CEBC 初始化, sub_804CA2C 族惰性赋值
typedef struct
{
    u8 itemId;   /* +0x0 道具 id (0 = 空) */
    u8 count;    /* +0x1 数量 */
    u8 field_2;
    u8 field_3;
} InvListEntry;
/* 道具/背包延迟写入系统 (sub_804DE8C / sub_804EF50 族):
 *  - gInvPageDeltas/gInvPageDeltaCount: 当前道具页的"已改动道具"工作表 (由
 *    sub_804DE8C 从 gInvPageItemIds 快照当前数量), sub_804EF00 可回滚单项。
 *  - gObjInvBackup: 每个 obj 池槽 (0..4) 被替换时的原始 {道具 id, 数量} 备份。
 *  - gInvPendingApply/gInvPendingApplyCount: sub_804DE20 收集的待提交项, sub_804EEC4 写回 gInventory。
 *  - sub_804EF50 把 gInvPageDeltas 中 itemId>0xDC 的真实道具数量写回 gInventory (背包数量表)。 */
extern InvListEntry gInvPendingApply[];
extern InvListEntry gInvPageDeltas[];
extern InvListEntry gObjInvBackup[];
extern u8 gInvPageDeltaCount;
extern u8 gInvPendingApplyCount;
extern u8 gUnk_03000DDE;
/* 0x03000DE6/0x03000DE8: 物件状态机 (sub_804E0E4) 在切场景前保存的
 * obj+0x2A (u16) 与 obj+0x35 (u8), 供 sub_801CBA4 恢复用。 */
extern u16 gUnk_03000DE6;
extern u8 gUnk_03000DE8;
extern u32 gUnk_03000DF0[];
extern u8 gUnk_03000E04;
extern u8 gUnk_03000E05;
typedef struct
{
    u8 field_0;
    u8 field_2;
    u16 padding;
} Unk_03000E08;
extern Unk_03000E08 gUnk_03000E08[];
extern u8 gUnk_03000E30;
extern u8 gScriptReturnSetId; /* 0x03000E68 ScriptSet_Load 记挂的脚本集号; 脚本退场时还原到 gEnvScriptSetId */
extern u8 gScriptPendingEntry; /* 0x03000E69 mode==2 记挂的入口号, 解压完成后跳 entryTbl[本值] */
extern u32 gScriptCursor; /* 0x03000E6C: 脚本 VM PC 槽, 存当前 opcode 字节地址 (EWRAM 脚本区) */
extern u16 gScriptVmFlags;
#define gUnk_03000E70 gScriptVmFlags
extern u8 gScriptDialogPhase;
#define gUnk_03000E72 gScriptDialogPhase
extern u8 gUnk_03000E74;
extern u8 gScriptCallStackDepth;
#define gUnk_03000E78 gScriptCallStackDepth
extern u32 gScriptCallStack[];
#define gUnk_03000E80 gScriptCallStack
extern u32 gUnk_03000EA0[];
extern u8 gUnk_03000EC0[];
extern u8 gUnk_03000EC8;
extern u8 gUnk_03000EC9;
extern u8 gUnk_03000ECA;
extern u8 gDialogWindowTileX;
#define gUnk_03000ECB gDialogWindowTileX
extern u8 gDialogWindowTileY;
#define gUnk_03000ECC gDialogWindowTileY
extern u8 gUnk_03000ED8;
extern u16 gScriptLocalSlots[];
extern u16 gUnk_03000EE8[];
extern u16 gUnk_03000F24;
extern u8 gUnk_03000F2A;
extern u16 gUnk_03000F2C;
extern u8 gUnk_03000F30;
extern u16 gUnk_03000F2E;

extern u16 gSoundTaskFlags;
extern u16 gPlayingSongId;
extern u16 gBgmVolume;
extern s16 gFadeFromVolume;
extern u8 gFadeDuration;
extern u8 gFadeCounter;
extern u8 gSfxTrackActiveBits;
extern u8 gSfxTrackLoopBits;
extern u16 gSfxTrackSongIds[4];
extern u8 gSfxTrackFadeBits;

extern u8 gSwitchFlags[0x50];

extern s32 gSioRecvWord;
extern u8 gGameState;
#define gMainGameState gGameState
extern u32 gGameTimer; // 3001948
extern u32 gUnk_03001950[14];
extern u16 gHBlankScrollCounter;

extern struct LzContext gLzContext; // 3001990

extern u16 gHeldKeysRaw;

extern u8 gHBlankWaveV[];

extern u8 gMainLoopMode;
#define gMainTaskSlot gMainLoopMode
// 0x03001AD0: gSioRecvPacket (declared in menu.h)
extern u32 gSioLinkState;
extern u8 gHBlankWaveRow;
extern u8 gHBlankWaveH[];

extern u8 gEventFlags[0x40];
extern u16 gNewKeysRaw;
// 0x03001CB0: gSioSendPacket (declared in menu.h)

extern u32 gSioRetryTimer;
extern u32 gIntrMainBuf[512];

/* 场上实体/角色记录 (0x28 = 40 B) —— 原名 CharacterObject, 已改: 它不是"角色"
 * (RPG 属性在另一个结构体 PlayerStats @0x40, 含 lv/hp/atc/skills/equip_slotN),
 * 而是脚本可寻址的**活动可绘制实体**: 玩家/NPC/对话箭头/特效/敌人共用同一格式。
 *   证据: gActors[data[1]] 直接由脚本 opcode 参数索引 (src/code_804F0B8.c)。
 *         宝箱不是此格式，而是独立的 0x08 字节 ChestObject 记录 (见下方定义)。
 * 字段构成 = 位置 + 朝向 + 动画 + 调色板 + 精灵链句柄, 无一项是 RPG 数值。
 */
typedef struct
{
    /* 0x00 */ u8 sprNodeIdx; ///< → gSpriteNodePool[] 主精灵链句柄 (0 = 无)
    /* 0x01 */ u8 renderFlags; ///< bit0 = 渲染使能 (`sprNodeIdx && (renderFlags&1)` 才更新);
                               ///<   bit1 = 已初始化 (三个 init 路径均置 2); 还作为
                               ///<   Sprite_EnqueueRender 的第 5 个实参传入
    /* 0x02 */ u8 gfxSetId; ///< 图形集编号 (与 paletteId 同值初始化: 箭头=9 / NPC=5 / 特效=0xA),
                            ///<   用作瓦片基址索引 (×72)
    /* 0x03 */ u8 paletteId; ///< OBJ 调色板号 (→ attr2 的 bit12-15)
    /* 0x04 */ u8 facingDir; ///< 当前朝向 0..7 (由 gWalkMoveDirLut 从 D-pad 码映射),
                             ///<   也是 gWalkDirectionMapping 的下标
    /* 0x05 */ u8 animTimer; ///< 逐帧递增; (animTimer>>3)&3 = 动画相位 → gWalkAnimFrameMapping 下标
    /* 0x06 */ s16 x; ///< 像素坐标 (地图格 ×8: `x = tileX * 8`)
    /* 0x08 */ s16 y;
    /* 0x0A */ u8 field_A; ///< 仅见 `= 0` 初始化; 读者在未匹配的 asm 里
    /* 0x0B */ u8 field_B; ///< 同上
    /* 0x0C */ u8 field_C; ///< 同上
    /* 0x0D */ u8 field_D; ///< 同上
    /* 0x0E */ u8 targetFacing; ///< 目标朝向 (命令/随机行走写入, 永远 `&= 7` 后
                                ///<   `facingDir = targetFacing` 拷回; ++/-- = 左转/右转)
    /* 0x0F */ u8 field_F; ///< 命令第 2 操作数 (live C 只写不读)
    /* 0x10 */ u8 stepTimer; ///< Chara_StepMove 返 1 且有脚本时每帧 ++; 命令里置 `op+1` / 1
    /* 0x11 */ u8 field_11; ///< 命令第 3 操作数; idle 分支会置 0x10 (live C 只写不读)
    /* 0x12 */ u8 stateFlags; ///< bit0 = z 随摄像机偏移 (Chara_GetDrawZ 判它);
                              ///<   bit4 (0x10) 由命令 2 与 Scene_EnterDoor 置;
                              ///<   bit5 (0x20) 由命令 0xFD 置;   bit6 (0x40) = 玩家正在移动;
                              ///<   命令 0xFD/0xFF 会 `&= 0x7F` / `&= 0x7B` 清位
    /* 0x13 */ u8 field_13; ///< 仅见 `= 0x80` / `= 0` 初始化
    /* 0x14 */ u16 field_14; ///< 计时器: `> 0xFE` 判完, 哨兵值 0xFF
    /* 0x16 */ u8 animIdx; ///< 当前动画编号, 0xFF = 无动画
    /* 0x17 */ u8 cmdPc; ///< **命令流程序计数器**: `temp = cmdStream + cmdPc; cmd = *temp++`,
                         ///<   各命令按长度 `cmdPc += 2/3/4`, 0xFE = 归零重播
    /* 0x18 */ u8 subSprNodeIdx; ///< **第二条精灵链句柄**: `= Sprite_AllocNode()`,
                                 ///<   `&gSpriteNodePool[subSprNodeIdx]` 用于释放/取子对象
    /* 0x19 */ u8 field_19; ///< 仅见 `= 0` 与 sub_804F280 的 strb 写
    /* 0x1A */ s16 z; ///< 深度/排序键: Chara_GetDrawZ 返回它 (bit0 时叠加摄像机偏移),
                      ///<   作为 Sprite_EnqueueRender 的第 4 个实参 (z)
    /* 0x1C */ u16 field_1C; ///< sub_804F280 用 strh 写 (= z << 4), 待定标
    /* 0x1E */ u16 field_1E; ///< sub_804F280 用 strh 写 (差值×16 / 表值), 待定标
    /* 0x20 */ u16 field_20; ///< sub_804F280 用 strh 写; CutsceneAnim_PlayFrame 用 ldrh 读
    /* 0x22 */ u16 field_22; ///< 全 ROM 未观察到访问
    /* 0x24 */ u8 *cmdStream; ///< **脚本命令流指针** (NULL = 无脚本);
                              ///<   非空时 Sprites_UpdateFrame 会 stepTimer++ 并调 Chara_ProcessCmdStream
} Actor; /* sizeof == 0x28, 已用 agbcc 实编译对账 asm 里的 idx*40 步长 */

/* 与 gActors 紧邻的前一个同类型数组 (0x03001EE0, 100 项, 尾部正好 = gActors 基址)。
 * 用途未定: sub_8004FD0 对 arg0<100 走 gActors[], 对 arg0>=100 走本数组[arg0]。*/
extern Actor gUnk_03001EE0[];

extern u8 gVBlankPipelineMode;
extern u32 gFrameCounter;
extern u8 gRandCursor;


extern u32 gCardExchangeStatus;
#define gUnk_030025A8 gCardExchangeStatus
extern u8 gPlayerMoveDir;
extern s16 gCameraPosX;
extern u8 gUnk_030025B8;
extern u8 gCurSpriteW;
extern u8 gSpriteWidth;
extern u16 gFollowerHistX[8];
extern u16 gEncounterCounter;
extern u8 gDialogueActive;
extern u16 gFollowerHistY[8];
extern u8 gTitleIntroState;
#define gCutsceneActive gTitleIntroState
extern u8 gSceneEntryFlag;
extern u16 gCameraTargetX;
extern s16 gCameraPosY;

extern u16 gScenePhase;
extern u8 gLogoEffectState;
extern u16 gTitleFadeTimer;
#define gUnk_03002608 gTitleFadeTimer
extern u8 gWarpAnimState;

extern u8 gSpriteHeight;
extern u8 gCurSpriteH;
extern u8 gBattleResultType;
extern u32 gSilverAmount;
extern u16 gCameraTargetY;
extern s16 gUnk_03002C40;
extern u8 gPartyFollowFlags;
extern u8 gAfterBattleCounter;
extern u16 gSceneLoadToggle;
extern u8 gInputLockFrames;
extern u8 gFollowerHistDir[8];
extern u8 gCutsceneAnimFlags[];
// extern struct Unk_03003AC0 *gUnk_03002C80[128];

/* 主实体数组: **恰好 24 项** (0x03002E80 + 24*0x28 = 0x03003240 = gPendingPalId ✓)
 * 与遍历上界 `for (i = 0; i <= 23; i++)` 双向印证。
 *   [0]      = 玩家 (Task_MapExplore 里 D-pad 处理写 gActors[0].field_12)
 *   [2..18]  = 队伍跟随者 (Party_FollowStep 从 i=2 起, 上界 19)
 *   [18]     = gEffectActor      (特效实体, = gActors+720)
 *   [19..]   = gDialogArrowActors (对话箭头, = gActors+760, 按数组用)
 * → 下面两个符号不是独立对象, 而是本数组固定槽位的**别名**。*/
extern Actor gActors[];

// extern Unk_03003150 gEffectActor;
// extern Unk_03003150 gDialogArrowActors[];

extern Actor gEffectActor; /* == gActors[18] */
extern Actor gDialogArrowActors[]; /* == &gActors[19] */
extern u16 gPendingPalId;

extern u32 gVramBufferPointers[];

extern u8 gPendingSpriteLoad;
/* 待处理精灵装载请求位图: bit0=图块(由 SetSlotGfxId 置), bit1=调色板(由 SetSlotPalId 置);
 * 由 PendingSpriteLoad_Flush 消费并清零, 查询用 GetPendingSpriteLoad()。 */
#define PENDING_SPRITE_GFX 1
#define PENDING_SPRITE_PAL 2
extern u16 gPendingGfxId;
extern u8 *gCutsceneAnimPals[];

extern u8 gVramTransferCounts[32];

typedef struct
{
    u8 field_0;
    u8 field_1;
    u8 pad[2];
    u32 field_4;
} Unk_03003380;

extern Unk_03003380 gPalTransferQueue[32];

extern u8 gPendingCharaSwitch;
extern u8 gCutsceneAnimSlots[];
extern u8 gPendingPalSlot;

typedef struct
{
    u16 field_0;
    u16 field_2;
    u16 field_4;
    u16 field_6;
} Unk_030034C0;

extern Unk_030034C0 gOamAffineBuf[];

typedef union
{
    struct
    {
        u32 VPos : 8;
        u32 AffineMode : 2;
        u32 ObjMode : 2;
        u32 Mosaic : 1;
        u32 ColorMode : 1;
        u32 Shape : 2;
        u32 HPos : 9;
        u32 AffineParamNo_L : 3;
        u32 HFlip : 1;
        u32 VFlip : 1;
        u32 Size : 2;

        u16 CharNo : 10;
        u16 Priority : 2;
        u16 Pltt : 4;
        u16 AffineParam;
    } fields;

    u32 attrs[2];

} GameOamData;
extern GameOamData gOamBuffer[128];

typedef struct
{
    void *src;
    void *dest;
} Unk_030039C0;

extern Unk_030039C0 gVramTransferQueue[32];

/* ===== OAM 属性字段访问宏 =====
 * 语义与目标汇编一致: 清位用 &, 写回用 + (不是 |), 与 SET_OAM_FIELD 展开对应:
 *   field = (field & ~mask) + (val & mask)
 * 生成形态: movs mask半字; ands; (val 截断); adds —— 见 UiSprites_Update */
#define SET_OAM_FIELD(field, mask, val) ((field) = ((field) & ~(mask)) + ((val) & (mask)))

#define GET_OAM_FIELD(field, mask)      ((field) & (mask))

#define OAM0_Y_MASK                     0x00FF // u32 VPos:8
#define OAM0_AFFINE_MODE                0x0300 // u32 AffineMode:2
#define OAM0_OBJ_MODE                   0x0C00 // u32 ObjMode:2
#define OAM0_MOSAIC                     0x1000 // u32 Mosaic:1
#define OAM0_COLOR_MODE                 0x2000 // u32 ColorMode:1
#define OAM0_SHAPE                      0xC000 // u32 Shape:2

#define OAM1_X_MASK                     0x01FF // u32 HPos:9
#define OAM1_AFFINE_PTR                 0x0E00 // u32 AffineParamNo:3 (仿射模式下)
#define OAM1_AFFINE_OR_FLIP             0x3E00 // u32 AffineParamNo|HFlip|VFlip (bit9-13, 非仿射时含翻转位)
#define OAM1_HFLIP                      0x1000 // u32 HFlip:1
#define OAM1_VFLIP                      0x2000 // u32 VFlip:1
#define OAM1_SIZE                       0xC000 // u32 Size:2

#define OAM2_CHAR_MASK                  0x03FF // u16 CharNo:10
#define OAM2_PRIORITY                   0x0C00 // u16 Priority:2
#define OAM2_PALETTE                    0xF000 // u16 Pltt:4

#define GET_OAM_Y(field)                ((field) & (OAM0_Y_MASK))
#define CLR_OAM_Y(field)                ((field) & (~OAM0_Y_MASK))
#define SET_OAM_Y(field, val)           SET_OAM_FIELD(field, OAM0_Y_MASK, val)

#define GET_OAM_X(field)                ((field) & (OAM1_X_MASK))
#define CLR_OAM_X(field)                ((field) & (~OAM1_X_MASK))
#define SET_OAM_X(field, val)           SET_OAM_FIELD(field, OAM1_X_MASK, val)

/* 定点插值: 8 步走完 dist (UiSprites_Update 的移动插值, asrs #3) */
#define LERP_POS(start, dist, step)     ((start) + (((dist) * (step)) >> 3))

/* 精灵链节点 (0x14 = 20 B) —— 名字保留, 它确实是 "node":
 *   gSpriteRenderQueue[128] = 链头指针数组; 本结构自带 next 字段 → 链表节点;
 *   一个多块拼接的大精灵 = 一串节点, 每节点渲染成 **一个 OBJ 矩形**
 *   (sub_8004F64 逐节点写 gOamBuffer[*oamIdx] 并推进游标, 循环次数 = flags & 0x7F)。
 *   可用池容量 112 (Sprite_AllocNode 返值判 `v <= 0x6F`), 声明 128 为对齐到 0x2C80/0x3AC0 边界。
 * 字段 = 3 个 GBA OAM 属性字 + 逻辑坐标 + 链指针, 与 OAM 布局不同 (OAM 项只 8 B),
 * 所以叫 OamEntry 反而错 —— SpriteNode 是更准确的抽象层名字。
 */
typedef struct SpriteNode
{
    /* 0x00 */ u8 flags;
    ///< bits 0-6: 该链的 OBJ 段数 (渲染时循环次数), 证据 `flags & 0x7F`
    ///< bit 7   : 隐藏/跳过渲染, 证据 sub_8004F64 的 `(s8)flags < 0` 直接返回 next
    ///< 值 0   : 空闲池块 (RenderQueue_Clear 用 `flags == 0` 判空闲)
    ///< 注: 旧注释"bit 0=active, bits 1-7=chain count" 是错的
    /* 0x01 */ u8 animStep;

    /* 0x02 */ u16 attr0; ///< GBA OAM Attribute 0 (Y pos, shape, mode, affine flags)
                          ///< Bits 0-7: Y coordinate
                          ///< Bits 8-9: Affine mode
                          ///< Bits 10-11: OBJ mode (normal/transparent/window)
                          ///< Bit 12: Mosaic enable
                          ///< Bit 13: Color mode (0=16 color, 1=256 color)
                          ///< Bits 14-15: Shape (square/horizontal/vertical)

    /* 0x04 */ u16 attr1; ///< GBA OAM Attribute 1 (X pos, size, flip, affine param)
                          ///< Bits 0-8: X coordinate
                          ///< Bits 9-11: Affine parameter number (lower 3 bits)
                          ///< Bit 12: Horizontal flip
                          ///< Bit 13: Vertical flip
                          ///< Bits 14-15: Size (depends on shape)

    /* 0x06 */ u16 attr2; ///< GBA OAM Attribute 2 (tile ID, palette, priority)
                          ///< Bits 0-9: Character/tile number
                          ///< Bits 10-11: Display priority
                          ///< Bits 12-15: Palette number

    /* 0x08 */ s16 x; ///< Logical/screen X position (before OAM conversion)
    /* 0x0A */ s16 y; ///< Logical/screen Y position (before OAM conversion)

    /* 0x0C */ struct SpriteNode *next; ///< Pointer to next sprite in chain (multi-tile sprites)

    /* 0x10 */ u16 tileOffsetX; ///< Tile offset X for multi-tile sprites
    /* 0x12 */ u16 tileOffsetY; ///< Tile offset Y for multi-tile sprites

} SpriteNode; // Size: 0x14 (20 bytes)

extern struct SpriteNode *gSpriteRenderQueue[128]; // 3002C80
extern struct SpriteNode gSpriteNodePool[128]; // 3003AC0

/*
struct Unk_03003AC0
{
    u8 field_0;
    u8 animFrame;
    u16 attr0;
    u16 attr1;
    u16 attr2;
    s16 x;
    s16 y;
    struct Unk_03003AC0 *field_C;
    u16 field_10;
    u16 field_12;
};
extern struct SpriteNode gSpriteNodePool[128]; // 3003AC0 (上方声明, 此处为旧注释残留别名)
*/

extern u32 gCutsceneAnimScripts[];

extern u8 gPendingGfxSlot;
extern u16 gBlendCoefficients;
extern u16 gUnk_03004604;
/* Iris-transition progress (0 = closed, 240 = fully open). */
#define gWindowTransitionProgress gUnk_03004604
extern u8 gSceneTransitionArg;
extern u8 gCameraDrawMode;
extern u16 gHBlankEffectMode;
extern u16 gMoveCmdSetId;
extern u8 gUnk_03004618;
extern u16 gUnk_0300461C;
#define gCameraPanStartY gUnk_0300461C

/* 已看过的开场整屏图位图: bit i ↔ gScreenIdleIconPageMap[i] (地图 ID);
 * bit 13 (地图 0x78) 由事件标志 0xFD 解锁 (ScreenIdleIcons_BuildList) */
extern u8 gScreenIdleEventFlags[];

/* 场景混合特效模式 (MapScene_Load 从 gMapSceneDescriptors[].bgLoadMode 装入);
 * PaletteEffects_Update 按 8/11/17 三值驱动 gBlendCoefficients 动画 */
extern u8 gSceneBlendMode;

/* ScreenIdleIcons_BuildList 产物: 已看地点 ID 列表 (16 项, 0 结尾) + 游标 */
extern u8 gScreenIdleIconIds[];
extern u8 gScreenIdleIconCursor;

extern u8 *gChoiceListPtr;
extern u16 gUnk_03004630;
#define gCameraPanTargetX gUnk_03004630
extern u8 gMapScriptSetId; /* 0x03004634 当前地图环境脚本集号 (来自 MapSceneDescriptor.scriptSetId, MapScene_Load 消费) */
extern u8 gSpawnTileY;
/* MapZone_FindAt 命中的区域动作号 (0..4, 0xFF=未命中); MapZone_Trigger 按它分发 */
extern u8 gMapZoneType;

/* MapZone_FindAt 算出的 16x16 足迹覆盖的至多 4 个瓦片坐标 (0xFF=无效槽) */
extern u8 gZoneCheckTileYs[4];
extern u16 gBG3ScrollY;
extern u16 gUnk_0300464C;
extern u16 gUnk_03004650;
/* 命中区域在其动作记录表内的下标 (cells 条目第 4 字节) */
extern u8 gMapZoneEntryIdx;
extern u16 gBlendControl;
/* Active screen fade flags; bit 7 marks completion and gates gameplay input. */
extern u16 gScreenFadeFlags;
extern u16 gIntroBgTileSetIndex; // 0=主 tile组(0x06000000), 1=备用8-tile组(0x06000C00) — IntroBg 暂存态
/* Per-frame snapshot consumed while building each transition scanline. */
extern u16 gUnk_03004668;
#define gWindowTransitionProgressSnapshot gUnk_03004668
extern u8 gChoiceCursor;

typedef struct
{
    u8 field_0;
    u8 field_1;
} Unk_03004670;

// extern Unk_03004670 gSlotGfxId;
extern u8 gSlotGfxId[];
/* 每个精灵表槽位(0..11)当前使用的图形编号, 0xFF = 该槽空。
 * 写: SetSlotGfxId; 读: ReloadSpriteSheet / ReloadAllSpriteSheets。 */

/* 图形资源集/模式 ID。bit7 = 1 时不重载角色精灵与数字字体(见
 * ReloadSpriteSheet / ReloadAllSpriteSheets / LoadDigitFontObjTiles / LoadArrowObjTiles);
 * 0xFC/0xFD/0xFE/0xFF 是保留值, `<= 0xFC` 走正常分支。
 * ⚠ 这里按 u16 声明但多处只按字节读写(asm 侧是 ldrb), 改类型会影响代码生成, 不要顺手改。 */
extern u16 gObjGraphicsSetId;
#define GFXSET_NO_SPRITE_LOAD 0x80 /* bit7 */
extern u16 gDrawCamY;
extern u16 gUnk_03004688;
extern u8 gSpawnTileX;
extern u16 *gUnk_03004694;
#define gPendingPortraitPalette gUnk_03004694
extern u16 gBg1ScrollMode;

#include "anim_slot.h"

/* 当前地图的区域头表指针 (MapScene_Load 从 0x087EBB20[mapIdx] 装载):
 * {u32 cells; u32 type0..type4} — cells: {u8 count, {u8 xTile, u8 yTile, u8 type, u8 entryIdx}[count]}
 * typeN: MapZone_Trigger 各动作的记录表 (type0/1 记录 8B, type2 4B, type3 2B, type4 4B) */
extern u32 *gMapZoneHeader;
extern u8 gSpawnFacingDir;
/* Fade progress in scanline units; ScreenFade_Start initializes it to 0 or 0x1B0. */
extern s16 gScreenFadeProgress;
extern u16 gUnk_030047AC;
extern u16 gCurrentMapId;
extern u8 gUnk_030047B4;
#define gCameraPanDuration gUnk_030047B4
extern u8 gChoiceDestY;
extern u8 gChoiceGroupIdx;

extern u16 gDrawCamX;
extern u16 gUnk_030047C4;
extern u16 gIntroBgTransferStage; // 0=无, 1=tiles 已暂存 0x02020000, 2=tilemap 已暂存
extern u8 *gUnk_030047CC;
#define gPendingPortraitGfx gUnk_030047CC
// extern Unk_03004670 gSlotPalId;
extern u8 gSlotPalId[];
/* 每个精灵表槽位(0..11)当前使用的调色板编号, 0xFF = 该槽空。写: SetSlotPalId。 */
extern u16 gUnk_030047DC;
#define gCameraPanStartX gUnk_030047DC

extern u8 gMapNpcSetId;
extern u16 gUnk_030047EC;
/* Signed per-frame change applied to gScreenFadeProgress. */
extern s16 gScreenFadeStep;
extern u16 gPaletteFxPhase; /* 0x030047F4: 调色板特效相位计数 (PaletteEffects_Update 读/清, Op_SysEffect subop4-3 复位) */

/* gViewportFlags 下标语义 (写点: Viewport_UpdateEffects / Op_SysEffect / BgMap_FillRow) */
enum ViewportFlagIdx
{
    VF_EFFECT_EN     = 0,  /* 特效使能位: bit0/1=视口抖动 plane, bit2=白闪 */
    VF_SHAKE1_OFF    = 1,  /* plane1 抖动偏移 = Rand_TableNext() & VF_SHAKE_MASK */
    VF_SHAKE2_OFF    = 2,  /* plane2 抖动偏移 = 同上 */
    VF_FLASH_CNT     = 3,  /* 白闪帧计数 0..0xF (Viewport_UpdateEffects bit2 分支) */
    VF_SHAKE_MASK    = 4,  /* 抖动随机掩码 (Op_SysEffect subop0: arg 1/2/其它 → 1/3/7) */
    VF_WHITEOUT_CNT  = 10, /* 白化淡入/出帧计数 (Op_SysEffect subop0xCA) */
    VF_FADE_LONG_CNT = 11, /* SYSFX_FADE_LONG 长渐变帧计数 (1..0x3E) */
    VF_SAVEUI_STEP   = 12, /* 存档 UI OBJ 调色板装载步号 0..3 (Op_SysEffect subop8) */
    VF_BGMAP_FILL    = 13, /* 行填充请求 (BgMap_FillRow 置位, 主循环消费清零) */
    VF_FADE_PHASE    = 14, /* SYSFX_FADE_2PH 渐变相位闩 (0=第一相, 1=收尾) */
    VF_FADE_FRAME    = 15, /* 调色板渐变帧计数 (subop4-1/4-4 与 subop7 共用) */
};
extern u16 gViewportFlags[];
extern u8 gEncounterEnabled;
extern u8 gChoiceDestX;

extern u16 gBG2ScrollY;
extern u16 gUnk_03004830;
#define gCameraPanTargetY gUnk_03004830

/* Third ScreenFade_Start argument; currently only its initialization is observed. */
extern s16 gScreenFadeParam;
extern u8 gZoneCheckTileXs[4];

extern u8 gUnk_0300483C;
/* 0 = no upload; otherwise portrait position + 1. */
#define gPendingPortraitSlot gUnk_0300483C

extern u8 gWin0HWaveTable[];
/* 81 packed WIN0H boundaries generated for the iris transition. */
#define gWindowTransitionScanlineTable gWin0HWaveTable
/* Nonzero while ScreenFx_SetMode has an in-flight window/palette transition. */
extern u8 gScreenTransitionState;
#define gSceneSubState gScreenTransitionState
extern u8 gUnk_03004844;
#define gCameraPanStep gUnk_03004844

extern u16 gBG2ScrollX;
extern u16 gBG3ScrollX;

extern u8 gEnvScriptSetId; /* 0x03004850 当前环境脚本集号 (脚本退场 Script_SetEnvSet 还原; 进存档) */
extern u8 gCameraSnapFlag;
extern u8 gUnk_03004860;
extern u8 gChestFlags[32];

typedef struct
{
    u8 flags; /* bit0 = opened; bit7 = special chest gate */
    u8 mapEntryIndex; /* index into gChestFlags */
    u8 spriteNodeIdx; /* head of the chest's sprite chain */
    u8 interactionId; /* item/script interaction identifier */
    s16 x; /* 像素坐标 (CheckFacingEvent 目标形状 = ldrsh 寄存器偏移读, 需 s16; Thumb 无 ldrsh 立即数形式) */
    s16 y;
} ChestObject;

extern ChestObject gChestObjects[16]; /* 0x03004890, 16 个场景宝箱记录 */

extern u8 gPaletteFxMode; /* 0x03004910: palette effect mode + 1 */
extern u8 gPaletteFxPending; /* 0x03004914: pending palette upload flag */
extern u8 gPaletteFxTimer; /* 0x03004918: palette effect frame counter */
extern u8 gMapObjGfxSetId;

typedef struct
{
    u8 field_0;
    u8 field_1;
    u8 field_2;
    u8 field_3;
    u16 animTimer;
    u16 x;
    u16 y;
    u16 z;
    u16 field_C;
    u8 field_E;
    u8 field_F;
    u8 *dataPtr;
} StaticMapObject;

extern StaticMapObject gStaticMapObjects[3]; // gUnk_03004930

extern u8 gInventory[];

extern u8 gEquipBonusDef;
extern u8 gBattleFormationIds[];
extern u8 gEquipBonusAtkBase;
extern u8 gEquipBonusRes;
extern u8 gEquipBonusMen;
extern u8 gPartyMemberIds[];
extern u8 gEquipBonusDef2;
extern u8 gEquipBonusAtk;
extern u8 gEquipBonusAgl;
extern u8 gEquipBonusNoa;
extern u8 gEquipBonusLuc;

extern u8 gUnk_03004D40;

typedef struct
{
    /** 0x00 */ u8 lv;
    /** 0x01 */ u8 field_1;
    /** 0x02 */ u16 hp;
    /** 0x04 */ u16 mp;
    /** 0x06 */ u16 atc;
    /** 0x08 */ u16 def;
    /** 0x0A */ u16 agl;
    /** 0x0C */ u16 men;
    /** 0x0E */ u16 res;
    /** 0x10 */ u8 noa;
    /** 0x11 */ u8 luc;
    /** 0x12 */ u16 max_hp;
    /** 0x14 */ u16 max_mp;
    /** 0x16 */ u8 base_atc;
    /** 0x17 */ u8 base_def;
    /** 0x18 */ u8 base_agl;
    /** 0x19 */ u8 base_men;
    /** 0x1A */ u8 base_res;
    /** 0x1B */ u8 base_noa;
    /** 0x1C */ u8 base_luc;

    /** 0x1D */ u8 equip_atc;
    /** 0x1E */ u8 equip_def;
    /** 0x1F */ u8 equip_agl;
    /** 0x20 */ u8 equip_men;
    /** 0x21 */ u8 equip_res;
    /** 0x22 */ u8 equip_noa;
    /** 0x23 */ u8 equip_luc;

    /** 0x24 */ u8 equip_slot1;
    /** 0x25 */ u8 equip_slot2;
    /** 0x26 */ u8 equip_slot3;
    /** 0x27 */ u8 equip_slot4;
    /** 0x28 */ u8 equip_slot5;
    /** 0x29 */ u8 equip_slot6;

    /** 0x2A */ u8 skills[8];
    /** 0x32 */ u8 field_unk[6];

    /** 0x38 */ u32 exp;
    /** 0x3C */ u32 next_exp;
} PlayerStats;

extern PlayerStats gPartyStats[];

extern u8 gSaveFsmState;
#define gUnk_03004D44 gSaveFsmState
extern u16 gUnk_03004D48;

extern u8 gUnk_03004D4C;
extern u8 gActiveSaveSlot;
#define gUnk_03004D50 gActiveSaveSlot
extern u8 gSaveTimers[];

extern u16 gUnk_03004DBC;
extern u8 gBgTileReloadFlag;
extern u8 gUnk_03004DC0;
extern u8 gSaveBusyA;
extern u8 gSaveFlags[];

extern u8 gSaveSramBlock;
#define gUnk_03004DD0 gSaveSramBlock
extern u8 gSaveUiParam;
extern u8 gSaveBusyB;
extern u16 gSavedDispCnt; /* 0x03004DDC 存档菜单进入前的 REG_DISPCNT */
extern u16 gSavedBldCnt; /* 0x03004DE0 同上, REG_BLDCNT */
extern u16 gUnk_03004DE4;

extern u8 gSioState[];

/* SIO 多机通信会话状态 (0x03004DF0)。gSioState 的结构化视图 (同址别名 gUnk_03004DF0):
 * 现存已匹配子函数按 u8 下标访问, 本结构供新匹配使用; 字段语义名待 SIO 族匹配后统一。 */
typedef struct
{
    u8 isParent; // 0x00 1=主机(parent) 0=从机 (Sio_IsHost/SetReady 视作 mode)
    u8 stage; // 0x01 连接阶段
    u8 unk_2; // 0x02 收包位图累积
    u8 unk_3; // 0x03 收到位图
    u8 unk_4; // 0x04 包双缓冲交换标志
    u8 unk_5; // 0x05 帧完成标志 (本帧有包)
    u8 unk_6; // 0x06 对端就绪
    u8 errorFlags; // 0x07 SIO Error 位
    u8 unk_8;
    u8 sioInterrupted; // 0x09 串行 IRQ 已处理
    u8 unk_A;
    u8 counter; // 0x0B
    u8 pad_C[0x14 - 0xC];
    s32 unk_14; // 0x14 发送推进计数
    s32 unk_18; // 0x18 接收列计数 (-1=复位换缓冲)
    void *unk_1C; // 0x1C 发送双缓冲 A
    void *unk_20; // 0x20 发送双缓冲 B
    void *unk_24; // 0x24 接收缓冲 A (每槽 16×u16)
    void *unk_28; // 0x28 接收缓冲 B
    void *unk_2C; // 0x2C 收包双缓冲 (sub_8016E80 交换)
    void *unk_30; // 0x30
} Unk_03004DF0;
extern Unk_03004DF0 gUnk_03004DF0;

typedef struct
{
    u16 field_0;
    u16 field_2;
    u8 pad4[0x14];
} Unk_03004F20_entry;

typedef struct
{
    u16 unk0;
    u16 unk2;
    u8 pad0[0x18 - 4];
    Unk_03004F20_entry unk18[2];
    u32 field_48; /* 0x48 状态位域 (原 field_48..4B 四个 u8, 无单独引用) */
    u8 field_4C;
    u8 field_4D;
    u8 pad1[0x5E - 0x4E];
    u8 unk5E;
} Unk_03004F20;
extern Unk_03004F20 gSioSession;

typedef struct
{
    u32 *field_0;
    u32 *field_4;
    u16 field_8;
    u16 field_A;
    u16 field_C;
    u16 field_E;
} Unk_03004F80;

extern Unk_03004F80 gSioXferCtx;

extern u8 gUnk_03004F90[];

extern u16 gUnk_03007FF8;

typedef struct SaveInfo
{
    /** 0x00 */ char str[12]; // 12
    /** 0x0C */ u32 gameTimer; // 4
    /** 0x10 */ u32 silverAmount; // 4
    /** 0x14 */ u8 field_030047E4; // 1
    /** 0x15 */ u8 field_0300468C; // 1
    /** 0x16 */ u8 field_03004638; // 1
    /** 0x17 */ u8 field_030047A4; // 1
    /** 0x18 */ u16 field_03004614; // 2
    /** 0x1A */ PlayerStats field_03004AC0[10]; // 0x280 (10 * 0x40)
    /** 0x29A */ u16 field_03004624; // 2
    /** 0x29C */ u8 field_03004AA0[6]; // 6
    /** 0x2A2 */ u8 field_03004980[256]; // 0x100
    /** 0x3A2 */ u8 field_03001C60[64]; // 0x40
    /** 0x3E2 */ u8 field_030018F0[64]; // 0x40
    /** 0x422 */ Actor field_03002E80[18]; // 0x2D0 (18 * 0x28)
    /** 0x6F2 */ u8 field_0203F000[0xE00]; // 0xE00
    /** 0x14F2 */ u8 field_0203FE00[0x100]; // 0x100

    /** 0x15F2 */ u16 field_030025F8; // 2
    /** 0x15F4 */ u16 field_03002C3C; // 2
    /** 0x15F6 */ u8 field_030025B0; // 1
    /** 0x15F7 */ u8 field_03004A88[6]; // 6
    /** 0x15FD */ u8 field_03004870[32]; // 0x20
    /** 0x161D */ ChestObject chestObjects[16]; // 0x80 (16 * 8)
    /** 0x169D */ u8 field_03004670[12]; // 0xC
    /** 0x16A9 */ u8 field_030047D0[12]; // 0xC
    /** 0x16B5 */ u8 field_03004850; // 1
    /** 0x16B6 */ u8 field_03002C44; // 1
} SaveInfo; // 总大小: 0x16B7 字节

/* ==== 视口/摄像机滚动 (Viewport_UpdateScroll, 原 sub_8005C70) ==== */
extern s16 gCameraMinY; /* 0x0300464C: 摄像机 Y 下界 */
extern s16 gCameraMinX; /* 0x03004650: 摄像机 X 下界 */
extern u8 gDrawCamEaseActive; /* 0x03004680: 缓动进行中; gDrawCamY += gScrollEaseDeltas[gDrawCamX++] */
extern u16 gDrawCamY; /* 0x03004684: 绘制用摄像机 Y (Chara_GetDrawY 用它算屏幕 Y) */
extern u16 gDrawCamX; /* 0x030047C0: 绘制用摄像机 X; 缓动时兼作帧计数器 */
extern u16 gMapWidthPx; /* 0x030047C4: 地图宽(像素); 摄像机 X 上界 = 本值 - 240 */
extern u16 gMapHeightPx; /* 0x030047EC: 地图高(像素); 摄像机 Y 上界 = 本值 - 160 */

/* ==== 分层选项数据库 (gChoiceDataBase @0x080876A2) ==== */
/* 组/子组索引 -> 一个 0xFF 结尾的选项列表; 见 ChoiceMenu_BuildList / ChoiceMenu_HandleInput */
extern u8 gChoiceGroupIdx; /* 0x030047BC */
extern u8 gChoiceSubIdx; /* 0x030047E0 */
extern u8 *gChoiceListPtr; /* 0x0300462C */
extern u8 gChoiceListLen; /* 0x03004640 */
extern u8 gChoiceCursor; /* 0x0300466C */
extern u8 gChoiceSel; /* 0x0300469C: 当前选中项的低 nibble */

#endif

/* 过场动画 VRAM 槽基址 (CutsceneAnim_Load 写入) */
extern u32 gCutsceneAnimVram[];

/* ==== 敌人/角色基础数据表 (0x087EA580, 248 有效项 × 12B + 8 全零) ====
 * +0x00 u16 击败经验       +0x02 u16 击败金币 (≈经验/2)
 * +0x04 高4位=外形类别 (0xD/0xE/0xF=特殊), 低4位=属性族 (0-7)
 * +0x05     掉落物品 id (0=无; 1-17 范围)
 * +0x06     HP
 * +0x07     攻击特化 (多数 0; 1-22 常见, 60-80 罕见)
 * +0x08     防御 (0/1/2 为主)
 * +0x0A     AI 行为表下标 (gUnk_0839CEFC + idx*3, bit 域: sub_804DD90 查询)
 * +0x0B     属性/抗性位段 (0x80|0x20|0x01 等, 0xFF=全体)
 * 消费者: Chara_GetFormGfx(+4&F), PartyForm_ApplyBonus(+4&F0),
 *         sub_800AAF8(+0/2 u16 拼), sub_804DD90(+0x0A×3→gUnk_0839CEFC)
 * 注: 追加块在 #endif 后, agbcc 对 typedef 重复声明报 conflicting types, 必须自带 guard */
#ifndef _ENEMY_CHARA_STAT_H
#define _ENEMY_CHARA_STAT_H
typedef struct
{
    /* 0x00 */ u16 expReward;
    /* 0x02 */ u16 goldReward;
    /* 0x04 */ u8 formRace; /* 高4=外形, 低4=属性族 */
    /* 0x05 */ u8 dropItemId;
    /* 0x06 */ u8 hp;
    /* 0x07 */ u8 attack;
    /* 0x08 */ u8 defense;
    /* 0x09 */ u8 pad_09;
    /* 0x0A */ u8 aiTableIdx;
    /* 0x0B */ u8 resistFlags;
} EnemyCharaStat;
extern const EnemyCharaStat gCharaBaseData[];

/* ==== 视口/摄像机滚动 (Viewport_UpdateScroll, 原 sub_8005C70) ==== */
extern s16 gCameraMinY; /* 0x0300464C: 摄像机 Y 下界 */
extern s16 gCameraMinX; /* 0x03004650: 摄像机 X 下界 */
extern u8 gDrawCamEaseActive; /* 0x03004680: 缓动进行中; gDrawCamY += gScrollEaseDeltas[gDrawCamX++] */
extern u16 gDrawCamY; /* 0x03004684: 绘制用摄像机 Y (Chara_GetDrawY 用它算屏幕 Y) */
extern u16 gDrawCamX; /* 0x030047C0: 绘制用摄像机 X; 缓动时兼作帧计数器 */
extern u16 gMapWidthPx; /* 0x030047C4: 地图宽(像素); 摄像机 X 上界 = 本值 - 240 */
extern u16 gMapHeightPx; /* 0x030047EC: 地图高(像素); 摄像机 Y 上界 = 本值 - 160 */

/* ==== 分层选项数据库 (gChoiceDataBase @0x080876A2) ==== */
/* 组/子组索引 -> 一个 0xFF 结尾的选项列表; 见 ChoiceMenu_BuildList / ChoiceMenu_HandleInput */
extern u8 gChoiceGroupIdx; /* 0x030047BC */
extern u8 gChoiceSubIdx; /* 0x030047E0 */
extern u8 *gChoiceListPtr; /* 0x0300462C */
extern u8 gChoiceListLen; /* 0x03004640 */
extern u8 gChoiceCursor; /* 0x0300466C */
extern u8 gChoiceSel; /* 0x0300469C: 当前选中项的低 nibble */

#endif
