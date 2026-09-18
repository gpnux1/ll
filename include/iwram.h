#ifndef _IWRAM_H
#define _IWRAM_H

#include "gba/types.h"

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

extern u16 gUnk_03000000;
extern u16 gUnk_03000002;
extern u16 gBlendFadeStep;
extern u8 gLogoAnimDirection;
extern u8 gLogoSpriteNodes[2];
extern u16 gLogoAnimTimer;
/* ---- 菜单实体调色板动画 (MenuEnt_ParseDesc 写入, PaletteTransfer_Update 消费) */
extern u8 gMenuEntAnimFlags[4];
extern u8 gMenuEntAnimThreshold[4];
extern u8 gMenuEntAnimShift[4];
extern u16 gMenuEntAnimCounter[4];
extern u32 gMenuEntPalDest[4];
extern u8 *gMenuEntAnimFrameTbl[4];

typedef struct
{
    u8 statusFlags; /* bit0=显示 */
    u8 animTimer;
    u8 lerpFrame; /* 插值倒计时 8→0 */
    u8 oamSlotId;
    u16 x; /* attr1=(x-0x20)&0x1FF */
    u16 y; /* 直作 attr0 */
    u16 moveEndX; /* 目标 (ptr->x) */
    u16 moveEndY;
    u16 moveStartX; /* 滑动起点 */
    u16 moveStartY;
} MenuCursorSprite;
/* ---- 菜单光标精灵 (滑动选择光标, menu/menu_ui/text_engine 共享):
 * 与 UISpriteEntity 前 0x10B 同布局 (0x48 区仅 0x10 大小, 无 baseTileId/pad);
 * 目标位姿取 gUnk_087EB1F4/214/22C[gMenuCursorGrp][gMenuCursorSel] */
extern MenuCursorSprite gMenuCursorSprite;

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
/* ---- 道具详情翻页可用项缓存 (0x030001B9/0x030001BA, 2026-09-14 claude804AF60 定名):
 * sub_800FDEC 从 gItemUseCtx[0]/[4] 锚点分别向后/向前扫描"当前成员可用"的道具
 * (gCharaBaseData[i].resistFlags & 成员位 && (formRace & 0xF)==类型 && gInventory[i]!=0),
 * 命中即停; 无命中写 0, 初值 0xFF=无。消费者 sub_800C2F8 (翻页导航: 非 0xFF 才响键+播音)。 */
extern u8 gItemPagePrevId; /* 0x030001B9 (原 gUnk_030001B9): 上一个可用道具 id (0xFF=无, 0=无命中) */
extern u8 gItemPageNextId; /* 0x030001BA (原 gUnk_030001BA): 下一个可用道具 id (同上) */
/* 0x030001BC: 7 个装备属性差值指示字形 id (0xB=平/0xC=升/0xD=降);
 * 索引序 atk(0),def(1),agl(2),men(3),res(4),luc(5),noa(6)。
 * 由 sub_800FF10 填写, sub_800B374 读取后交给 Text 渲染。 */
extern u8 gStatArrowIds[];
/* ---- 道具使用效果参数 (0x030001C3..0x030001C6, 2026-09-14 claude804AF60 定名):
 * 选择道具时由 sub_800F128/sub_8010300 填写, 确认使用 sub_8010770 消费。
 * 效果参数源 = ROM 道具效果表 0x08093418 (5 字节/项: [1]&0xF=类型, [3]=HP回复量)。 */
extern u8 gItemUseId;         /* 0x030001C3 (原 gUnk_030001C3): 待使用道具 id (0x26=传送(WarpTable_Check), 0x3E=无角色道具) */
extern u8 gItemUseMpCost;     /* 0x030001C4 (原 gUnk_030001C4): 使用 MP 消耗 (sub_8010300 按成员计算; 0→判 MP 不足, 结果码 0x1D) */
extern u8 gItemUseEffectType; /* 0x030001C5 (原 gUnk_030001C5): 使用类型 = 表[1]&0xF (5=全队 HP 恢复) */
extern u8 gItemUseHealHp;     /* 0x030001C6 (原 gUnk_030001C6): HP 回复量 = 表[3] (0=恢复满) */
/* 0x030001C8: 菜单挂起结果/画面码槽 (u16): 各菜单 handler 写入 (0x14/0x15/0x16=画面切换,
 * 0x1A/0x1D/0x23/0x24/0x27=动作结果, 0x24=无效果), 主状态机 sub_800B374 轮询, 非 0 即分发。 */
extern u16 gMenuResultCode; /* 0x030001C8 (原 gUnk_030001C8) */

extern u8 *gMsgTable[];

extern u8 gSaveCurSlot;
extern u8 gSaveModeFlag;
extern u8 gSkillMenuTmpA;
extern u8 gSkillMenuTmpB;
extern u8 gPartyMenuIdx;
extern u8 gSkillMenuPage;
extern u8 gMenuItemId;    ///< 0x03000229 (原 gUnk_03000229) 道具界面光标处道具 id (InvUi_Main 存储; 80146A8 据此索引 gUnk_03004980 数量表)
extern u8 gUnk_03000204[];
extern u8 gUnk_03000208[];
extern u32 gUnk_03000210[];
extern u8 gItemPocketIdx; ///< 0x0300022A (原未登记) 道具分类页 (sub_8015658: sub_800AADC 类别映射, 4→5/>4→6; 8014A68 消费)
extern u8 gInvUiMode;     ///< 0x0300022B (原 gUnk_0300022B) 道具界面打开模式标志 (sub_801417C 存档菜单路径置位; 80146A8/8014A68 分支)
extern u32 gMenuItemIcon; ///< 0x0300022C (原 gUnk_0300022C) 光标道具图标 tile 基址 (InvUi_Main 存储; sub_8015658 清零/重绘)
/* Option sound-test state. BGM/SFX rows are displayed as ??? until a valid
 * save enables them. The playing BGM byte stores id+1 so zero means stopped. */
extern u16 gSoundTestSfxId;
extern u8 gSoundTestBgmId;
extern u8 gSoundTestPlayingBgmIdPlusOne;
/* Number of PRESS START update frames left before attract mode. */
extern u8 gTitleAttractCountdown;
extern u16 gCardRecvId;
extern u16 gCardSendId;
extern u8 gUnk_03000240;
extern u8 *gObjPoolPtr;
extern u32 gUnk_03000248;
/* ---- 输入/波浪 (battle_task_services.c): gKeysHeld=本帧按键(~REG_KEYINPUT, gGstate312=新按下),
 * gKeyIgnoreTimer=输入屏蔽倒计时(sub_8018744 装 10 帧); gWave*=BattleFx_Init 参数 */
extern u16 gKeysHeld;
extern u16 gGstate312;

/* gGstate314 = 按键连发状态字 (sub_80182A8 维护, sub_80187E8 返回):
 * 位 0-7  = 本帧按住键, 位 8-15 = 本帧新按/连发标志 (按下瞬间或 D-pad 连发阈值命中时置位,
 *           下一帧被 sub_80182A8 开头的字节回写清空)。
 * 位序同 REG_KEYINPUT 按键位: bit0=Up(0x40), bit1=Down(0x80), bit2=Left(0x20),
 * bit3=Right(0x10), bit4=A(0x01), bit5=B(0x02), bit6=L(0x200), bit7=R(0x100),
 * 高 8 位按同序对应"按下/连发"。
 * KeyRepeatState 与 GameOamData 同型 (打包 u16 + 命名位域), 供按名访问; 常量位名用
 * 下面的 KEYREPEAT_* 宏 (sub_80182A8 函数体已用它们命名掩码, 字节等价)。
 * 注意: 函数体必须经独立全局 gGstate314 访问 —— 原 ROM 对 0x03000314/316/317 是三条独立
 * 绝对地址常量, 用结构体成员访问 (基址+偏移) 会改变 GCC2.9 寻址/寄存器分配, 破坏字节匹配。 */
typedef union
{
    struct
    {
        u16 upHeld : 1;
        u16 downHeld : 1;
        u16 leftHeld : 1;
        u16 rightHeld : 1;
        u16 aHeld : 1;
        u16 bHeld : 1;
        u16 lHeld : 1;
        u16 rHeld : 1;
        u16 upPressed : 1;
        u16 downPressed : 1;
        u16 leftPressed : 1;
        u16 rightPressed : 1;
        u16 aPressed : 1;
        u16 bPressed : 1;
        u16 lPressed : 1;
        u16 rPressed : 1;
    } keys;
    u16 heldPress;
} KeyRepeatState;

#define KEYREPEAT_UP_HELD      0x0001
#define KEYREPEAT_DOWN_HELD    0x0002
#define KEYREPEAT_LEFT_HELD    0x0004
#define KEYREPEAT_RIGHT_HELD   0x0008
#define KEYREPEAT_A_HELD       0x0010
#define KEYREPEAT_B_HELD       0x0020
#define KEYREPEAT_L_HELD       0x0040
#define KEYREPEAT_R_HELD       0x0080
#define KEYREPEAT_UP_PRESSED   0x0100
#define KEYREPEAT_DOWN_PRESSED 0x0200
#define KEYREPEAT_LEFT_PRESSED 0x0400
#define KEYREPEAT_RIGHT_PRESSED 0x0800
#define KEYREPEAT_A_PRESSED    0x1000
#define KEYREPEAT_B_PRESSED    0x2000
#define KEYREPEAT_L_PRESSED    0x4000
#define KEYREPEAT_R_PRESSED    0x8000
#define KEYREPEAT_PRESSED_MASK 0xFF00

extern u16 gGstate314;
extern u8 gKeyIgnoreTimer;
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
/* ---- 逐行 BG 波浪滚动引擎 (battle_task_services.c, 与 SIO 无关): WaveTablePtr=运行时生成波形表指针
 * (常指 gUnk_02036EC0), WaveAngle/AngleVel=相位/步进(%360), WaveRowOffset=逐行偏移表,
 * WaveBgHofsTbl/VofsTbl=BG HOFS/VOFS 寄存器地址表 */
extern u16 gWaveAngle;
extern u16 gWaveRowOffset[];

extern u8 gWaveAngleVel;
extern u8 gWaveAmp;
extern u8 gWaveRowStep;
extern u8 gWaveMode;
extern u32 gWaveTablePtr;
extern u32 gWaveBgHofsTbl[4];
extern u32 gWaveBgVofsTbl[4];

extern u8 gBgLoadSlot;

typedef struct
{
    u16 bg0Hofs; /* +0x00 备份, 写回 REG_BG0HOFS (sub_801A2AC 族) */
    u16 bg0Vofs; /* +0x02 */
    u16 bg1Hofs; /* +0x04 */
    u16 bg1Vofs; /* +0x06 */
    u16 bg2Hofs; /* +0x08 */
    u16 bg2Vofs; /* +0x0A */
    u16 bg3Hofs; /* +0x0C */
    u16 bg3Vofs; /* +0x0E */
} BgScrollBackup;
/* ---- BG 滚动偏移备份 (field_0..E = BG0..BG3 的 HOFS/VOFS 成对备份,
 * sub_801A2AC 族写回 REG_BGxHOFS/VOFS; 波浪引擎经 gWaveBgHofsTbl/VofsTbl 逐行覆盖) */
extern BgScrollBackup gBgScrollBackup;
extern u16 gBattleUiFlags;
extern u8 gUnk_03000512;   /* 0x03000512: BgLoad 状态 (0..5), 见 sub_8019B98 */
extern u8 gBgLoadChunkIdx; /* 0x03000513: 已 DMA 进 VRAM 的图块块数 (sub_8019B98 case5) */
extern u8 gUnk_03000514;   /* 0x03000514: BgLoadEntry 组内已解压的图块索引 */

extern u8 gObjFlagsA[0x80];
extern u8 gObjFlagsB[0x80];

extern u16 gUnk_03000618;
extern u16 gUnk_0300061A;
extern u16 gUnk_0300061C;
extern u16 gUnk_0300061E;
extern u16 gUnk_03000620;
extern u16 gUnk_03000622;
extern u8 gUnk_03000624;
extern u8 gActSeqState;          /* 0x03000625: 战斗行动执行状态机 PC (sub_801BE34 / sub_801C484) */
/* ---- 伤害数字弹出 (sub_801D568 族: 弹槽 tile 基号=*4+0x158, 相位/级数控制滚动) */
extern u8 gDmgPopupSlot;
extern u8 gDmgPopupPhase;
extern u8 gDmgPopupLevel;
extern u32 gUnk_0300062C;
extern u8 gUnk_03000630;
extern u8 *gUnk_03000638[12];
extern u8 gUnk_03000668;
extern u8 gUnk_03000669;
extern struct BattleObj *gFxQueueObjs[]; /* 战斗效果/步进队列 (7 槽), 消费者 sub_801E040 */
/* ---- 战斗效果/步进队列游标 (队列=gUnk_030006F8[7]) */
extern u8 gFxQueueWriteIdx;
extern u8 gFxQueueReadIdx;
extern u8 gUnk_03000716;

/* 战斗"待选池"链节点 (16B, 步长 0x10): 前 12 字节就是 UnkNode, 故 gUnk_030006A0[i]
 * 可直接强转 UnkNode* 交给 ListNode_InitKey/InsertSorted (见 sub_801DC20)。
 * 数组 0x030006A0..0x030006F0 共 5 项 (索引 = obj 池槽 0..4, 由 sub_80489E8 mode0 产出);
 * 链表头 0x03000690 是 UnkNode 哨兵 (ListNode_Init, key=0xFF), 遍历条件恒为 key <= 0xFE。
 * data 指向 BattleObj*。消费者: sub_801DC20 (挂入), sub_801DD04 (摘链),
 * sub_801FF40 (挑选, 读 data 对象 memberIdx), sub_8020AE4 (逐帧 dmgAmount+1)。 */
typedef struct TaskPoolNode
{
    u8 key;
    u8 pad_1[3];
    struct TaskPoolNode *prev;
    struct TaskPoolNode *next;
    u32 data;
} TaskPoolNode;
extern TaskPoolNode gTaskPoolNodes[];
/* ---- 战斗"待选池" (等待被选中的对象池, 术语沿旧注释): sub_801DC20 挂入
 * (ListNode_InsertSorted 按 key 排序, 节点=TaskPoolNodes[池槽 0..4], Count++),
 * sub_801DD04 摘链 (Count--); 在池期间 dmgAmount 逐帧 +1 作蓄力权重 (sub_8020AE4),
 * sub_801FF40 按权重挑选对象出战。邻接 gFxQueueObjs=效果/步进队列 (死亡/特效入队,
 * gFxQueueReadIdx/WriteIdx 游标, 消费者 sub_801E040 按槽类重放登场) */
extern UnkNode gTaskPoolHead;  /* 待选池链表头: UnkNode 哨兵 (key=0xFF), 遍历取 ->next */
extern u8 gTaskPoolCount;       /* 待选池计数: sub_801DC20 挂入时 ++, sub_801DD04 摘链时 -- */
extern u32 gUnk_03000718;
extern u8 gUnk_0300071C;
extern u32 gUnk_03000730;
extern u8 gUnk_0300073C;
extern u8 gUnk_0300073D;
extern u8 gActHitSegCount;       /* 0x0300073E: 行动命中段计数 (与 obj->noa 比较) */
extern u8 gActDmgPhase;          /* 0x0300073F: 行动伤害结算相位 (0未结算/1待演出/2已结算) */
extern u8 gActDmgMode;           /* 0x03000740: 行动结算模式 (sub_801F3FC 传参) */
extern u16 gActDmgAmount;        /* 0x03000742: 本次行动累计伤害 */
extern u8 gUnk_03000744;
extern u8 gUnk_03000765;
extern u8 gUnk_08393A30[];
extern u8 gUnk_08393A48[]; ///< 0x08393A48 成员 X 坐标字符表 (sub_80257D8 case5 插值起点 / sub_8032948 存 gObjActSavedX); 同族 gUnk_08393A4D 为 Y
extern u8 gUnk_0839DF90[]; ///< sub_802A154 case30/36 用的 (x,y) 对表 (各 2 字节)
extern u8 gUnk_0839DF67[]; ///< 0x0839DF67 战斗对象演出坐标偏移表 {0,0x78,0x1E,0x5A} (sub_8039C38 case37 按 gObjActParam%4 取)
extern float gCosTable[];
extern float gSinTable[];
extern u8 gUnk_03000748[];
extern u8 gUnk_03000758[];
extern u8 gUnk_03000763;
extern u8 gUnk_03000764;         /* 0x03000764: 行动状态机结算标记 */
extern u8 gUnk_03000768;
extern u8 gUnk_03000769;
extern u8 gUnk_0300076B;
extern u16 gUnk_0300076C;
extern u16 gUnk_0300076E;
extern s8 gUnk_0300076A;
extern u16 gUnk_0300076C;
extern u16 gUnk_0300076E;
/* 菜单/演出选择列表窗口活动状态 (0x03000770..0x03000784, 2026-09-14 zcode-menulist-770 定名)。
 * 演出 handler sub_8023820 构建: 扫描候选槽 (0xFF 终止) 逐项追加到 gMenuListItems,
 * gMenuListCount 计数 (≤9, 数组 0x778..0x780 恰好 9 字节), 过滤通过项在 gMenuListRowBits
 * 置位; 多处复位序列 = 四字段全清 0。渲染 sub_80256E4: 3 行可见窗口, 行 = Top..Top+2 且
 * < Count, 行样式 = RowBits bit→高亮(2)/可选(0), 未置位→暗淡(1); 选中行 = Cursor。
 * sub_8021184 case6 从 gMenuSlotStates[idx][1]/[2] 同步 Top/Cursor (越界则吸附到末项)。
 * 0x771-0x777 与 0x783 为纯填充 (全 ROM 零访问); 第二平行窗口 = 0x03000808/809/80A (未名)。 */
extern u8 gMenuListCount;  ///< 0x03000770 (原 gUnk_03000770) 列表项数
extern u8 gMenuListItems[9]; ///< 0x03000778 (原未登记) 列表项 id (槽号)
extern u8 gMenuListTop;    ///< 0x03000781 (原 gUnk_03000781) 滚动窗口顶项
extern u8 gMenuListCursor; ///< 0x03000782 (原 gUnk_03000782) 选中项 (绝对下标)
extern u16 gMenuListRowBits; ///< 0x03000784 (原 gUnk_03000784) 行样式位图 (bit n = 项 n 高亮/可选)
extern u8 gMenuSlotStates[][5];
/* ---- 菜单第二选择列表窗口 (0x03000808..0x0300080A, 与 gMenuList* 平行; 2026-09-14 claude804AF60 定名):
 * 消费者全部字节级核对 —— 渲染 sub_802576C (3 行窗口: i = Top..Top+2 且 < Count,
 * 行样式 = (i==Cursor)?2:0, BgMap_PalFillRect(base, 0xB+style, 8, (i-Top)*2+8, 9, 2));
 * 同步 sub_8021184 case7 (从 gMenuSlotStates[idx][3]/[4] 取 Top/Cursor, 越界吸附末项,
 * 与 case6 对 gMenuList* 的处理相同); 逐项绘制 sub_8024940 (行数据取 0x030007C8[4B 步长])。
 * 0x80B 为纯填充 (全 ROM 零访问)。 */
extern u8 gMenuList2Count;  ///< 0x03000808 (原 gUnk_03000808) 第二列表项数
extern u8 gMenuList2Top;    ///< 0x03000809 (原 gUnk_03000809) 滚动窗口顶项
extern u8 gMenuList2Cursor; ///< 0x0300080A (原 gUnk_0300080A) 选中项 (绝对下标)
extern u8 gMenuMasterCursor; /* 0x030007BA (声明沿用原文件位次) */
/* ---- 菜单成员精灵装载队列 (0x0300080C..0x03000813): sub_802151C 开菜单时把
 * 候选池槽中 fxKind(+0xBC)==3 的成员写入 Slots[Count] 并把其 fxKind 清 0;
 * sub_8021700 逐帧消费: Phase0 = sub_80207DC(obj, posX, posY, f_1E, palSlot) 出精灵
 * → Phase1, Phase1 = 等 headA.kindFlags bit11 (0x800 装载中) 清零 → Idx++/Phase=0;
 * 全部装完 (Idx>=Count) 返回 1。装载期间菜单窗口等待。 */
extern u8 gMenuObjLoadSlots[5]; ///< 0x0300080C (原 gUnk_0300080C) 待装载 BattleObj 池槽号
extern u8 gMenuObjLoadCount;    ///< 0x03000811 (原 gUnk_03000811) 队列长度
extern u8 gMenuObjLoadIdx;      ///< 0x03000812 (原 gUnk_03000812) 当前装载下标
extern u8 gMenuObjLoadPhase;    ///< 0x03000813 (原 gUnk_03000813) 0=出精灵 1=等装载完成
/* ---- 菜单选择/窗口状态 (0x03000814..0x03000818):
 * SelSlot0/1 (s8): 全 ROM 仅写 -1 (sub_8025638 复位, sub_80230BC/8023820/8024940 关菜单时),
 * 唯一读者 sub_8022710 以 ldrsb 有符号读并判 >=0 —— 本版恒为无选择 (-1), 疑废弃/预留槽。
 * WindowPhase/Flags: 菜单窗口生命周期 —— sub_802151C 开菜单: Flags=0 + Phase=1;
 * sub_8021788 Phase1: Flags|=0x1000 (窗口 BG 开) → Phase=0, Phase0: DialogCtx 条件满足清
 * 0x1000; Phase2 (sub_802192C 关闭路径) → Phase=0; sub_8023820/8024940 演出分支亦写。 */
extern s8 gMenuSelSlot0;        ///< 0x03000814 (原 gUnk_03000814) 选择槽 0 (恒 -1)
extern s8 gMenuSelSlot1;        ///< 0x03000815 (原 gUnk_03000815) 选择槽 1 (恒 -1)
extern u8 gMenuWindowPhase;     ///< 0x03000816 (原 gUnk_03000816) 菜单窗口相位 (0=活动 1=开启中 2=关闭中)
extern u16 gMenuWindowFlags;    ///< 0x03000818 (原 gUnk_03000818) 窗口标志 (bit0x1000=窗口开启)
/* ---- BattleTask 物件演出引擎工作区 (sub_804442C 复位; handler=battle_stage_actor/battle_stage_dialogue/battle_stage_effects):
 * gObjActStep=步骤PC(0起手/1-2动画/5-8等待/9收尾/0x12-0x1F细分), SavedF2A/SavedPal=动画恢复参数,
 * StepTimer=步骤帧计数(插值t), Result=演出结果, SavedX/Y=原始坐标, GroupCount/GroupSlots=多对象
 * 组登记(低4位=池槽), ActWait*=等待窗互斥(全0=放行 sub_80444E8), SceneFadeOut/In=场景淡出/入量,
 * MoveFromX/Y=移动插值起点, SfxLatch=一次性音效闩, SceneTransStep=场景切换PC (sub_8044394族) */
extern u8 gObjActStep;
extern u16 gObjActSavedF2A;
extern u8 gObjActSavedPal;
extern u8 gObjActStepTimer;
extern u16 gObjActResult;
extern u8 gObjActSavedX;
extern u8 gObjActSavedY;
/* 当前动作候选目标槽位表 (0xC 项: 槽号; sub_80489E8 输出 — battle_stage_dialogue:634 mode1 敌侧,
 * sub_8032EA0 mode0 我侧, 消费者按 [i]*0xC8 取目标) */
extern u8 gTargetSlotList[];
extern u8 gTargetSlotCount;   /* 候选数 (sub_80489E8 返回值) */
extern u8 gObjActGroupCount;
extern u8 *gObjActGroupSlots;
extern u8 gActWaitBusy0;
extern u8 gActWaitBusy1;
extern u8 gActWaitBusy2;
extern u8 gActWaitCnt0;
extern u8 gActEventCount;
extern u16 gActWaitFrames;
extern u8 gActWaitCnt1;
/* 0x03000860 当前演出槽特效表项指针 (sub_80422B8 case0 按 obj->slot 命中
 * gObjActSlotFxTable 的项, 未命中取 [0]; case1/函数尾部读其 animIdx/frameIdx/timer。
 * 唯一消费者 = sub_80422B8 (code.s 仅 3 处引用, 全在本函数) */
extern const struct ObjActSlotFx *gObjActSlotFxCur;
extern u8 gUnk_03000864; ///< 0x03000864 (原 gUnk_03000864) 战斗对象逃跑判定结果 (0=成功, 1=失败; sub_803FF54 消费)
extern u8 gObjActDoneCount; ///< 0x03000865 (原 gUnk_03000865) 对象演出完成计数: sub_803FF54 步进器 case 在 obj->slot=0xFF/variantClass=7/gObjActStep=0x38 后 ++; sub_804448C 清 0 (BattleTask_Run 开场), getter sub_8044498 供 ObjGroup_AnyEvent 等待 !=0
extern u8 gSceneFadeOut;
extern u8 gSceneFadeIn;
extern u8 gObjActBranch;
extern u8 gObjActParam;
extern u16 gUnk_0300086C; ///< sub_803E58C 动画基准表项 (0x350/0x353/0x356, 加 1/2 变体)
extern u8 gObjActMoveFromX;
extern u8 gObjActMoveFromY; ///< sub_802D728 锚点动画源坐标 X/Y (obj->posX+0x1D / obj->posY-0x2F)
extern u8 gUnk_03000870[];  ///< 0x03000870 目标对象保存 X 坐标表 (sub_80401AC 逃跑演出用)
extern u8 gObjActSavedArgX; ///< 0x03000880 协作对象(arg1)演出前 posX 快照 (sub_803C328 case0 存/case42 还原)
extern u8 gObjActSavedArgY; ///< 0x03000881 协作对象(arg1)演出前 posY 快照 (同上)
extern u16 gActHitDmgAmount; ///< 0x03000882 (原 gUnk_03000882) 战斗脚本族 (sub_8040690/8042E70 等) 从 BattleObj.dmgAmount(+0xB2) 快照的当前伤害值; getter sub_8044420, 合击/连锁处理 (sub_801BE34/801C484) 累加进 0x03000742; 战斗脚本 case 起手清 0
extern u8 gObjActSfxLatch;
extern u16 gActWaitSfxId;   ///< 0x03000886 (原 gUnk_03000886) 演出等待结束音效号: sub_8044514 置默认 0x37, sub_8044574 由脚本参数给定; 等待结束时 Sfx_Play(本值,0,gActWaitSfxParam) (sub_803F658 合击状态机等)
extern u8 gActWaitSfxParam; ///< 0x03000888 (原 gUnk_03000888) 上述 Sfx_Play 第 3 参 (sub_8044574 arg2 / 默认 0)
extern u8 gSceneTransStep;
extern u8 gUnk_0300088B; ///< sub_80489E8 返回的参演角色数 (sub_8028AD8 登记)
extern u8 gUnk_0300088C; ///< 当前处理角色下标 (sub_8028AD8)
extern u8 gUnk_03000890[]; ///< 角色 obj 池槽号表 (sub_80489E8 输出, 按 0x8C 下标; sub_8028AD8)
extern u8 gUnk_03000898[]; ///< 暂存 posX (sub_8028AD8)
extern u8 gUnk_0300089A[]; ///< 暂存 posY (sub_8028AD8)
extern u8 gUnk_0300089C[]; ///< 暂存 headA.palSlot (sub_8028AD8)
extern u16 gUnk_030008A0[]; ///< 暂存 headA.f_1E (sub_8028AD8)
extern u8 gUnk_030008A4; ///< 当前相位/组下标 (sub_8028AD8)
extern u8 gUnk_030008A5; ///< sub_802A154 演出序号 (0..2, 索引 gUnk_0839DF90 的 (x,y) 对)
extern u32 gStatRecalcPool;
extern u8 gChoiceListLen;
/* 战斗结算/展示渲染状态簇 (0x03000949..0x0300097D, 2026-09-14 zcode-dlg-9xx 定名):
 * 主状态机 = sub_8048FB8 (按 gBattleIntroState 分派 22 态, battle task @0x08017E00 逐帧调用;
 * 尾部每帧调 sub_8048F0C 调色板闪光机)。渲染 = 命令表驱动: 段表 gUnk_0839B2E0 (u16,
 * 0xFFF0 分隔, sub_804AB40/80494F0 切段) + 0x02035AC0 DialogCtx BG map 写入
 * (sub_80492C0: 段内格光标 0x94A, 列光标 0x94B; 格 opcode 0x6E0-0x6E6 分派
 * tile 动画/数字/图形; 数字位光标 0x94C 持久, tile 动画帧号 0x94D)。
 * 业务流: 参战遍历 (0x974/0x979/0x97A, sub_80489E8 输出) → EXP 结算
 * (0x956=sub_80453D8 累加, 0x958=sub_80454A4 升级位掩码) → 升级能力值滚动
 * (0x956←0x95A 目标值, sub_8049AD8 逐 stat 恢复) → 升级习得列表
 * (0x960/0x968/0x969/0x96C, sub_8045860 收集 + sub_804ACC0 图形段)。
 * 复位: sub_8048DA4 清 0x03000948-0x0300097D。 */
extern u8 gBattleDlgObjSlot; ///< 0x03000949 当前展示对象 BattleObj pool 槽号 (sub_80494F0 case10 ++/case11 =0; pool[0x949].headA.palSlot → 0x97D)
extern u8 gBattleDlgCellIdx; ///< 0x0300094A 段命令格光标 (0x950 段内 u16 下标; 格动画完成时 ++, 切段清 0)
extern u8 gBattleDlgTileCol; ///< 0x0300094B BG map 列光标 (0x02035AC0 行内 tile x, 每格 ++; sub_80492C0 目的地偏移)
extern u8 gBattleDlgDigitCol; ///< 0x0300094C 多位数字位光标 (sub_80497B0 跨调用持久, 不切段不清)
extern u8 gBattleDlgAnimFrame; ///< 0x0300094D tile 动画帧号 (sub_804ABF8/9958/98E0; >3 或 0xF00 终止符结束)
extern u32 gBattleDlgSegOff; ///< 0x03000950 当前命令段字节偏移 (gUnk_0839B2E0 内, = 段起点*2; sub_804AB40 返回值)
extern u8 gBattleDlgNextState; ///< 0x03000954 结算子状态暂存 (sub_80494F0 返回值; sub_8048FB8 case20 → gBattleIntroState)
extern u16 gBattleDlgShowVal; ///< 0x03000956 正在显示/滚动的数值 (EXP=sub_80453D8, 金钱=sub_804542C, 升级后能力值←0x95A)
extern u8 gBattleDlgLvUpMask; ///< 0x03000958 升级成员位掩码 (sub_80454A4 发放 EXP 返回; bit i=成员 i 升级)
extern u16 gBattleDlgShowTarget; ///< 0x0300095A 数值滚动目标 (= 能力新值-旧值 后的实际新值, sub_8049AD8; 完成时 → 0x956)
extern u8 gBattleDlgLearnList[8]; ///< 0x03000960 升级习得 id 列表 (id-1; sub_8045860 推进 obj[0xAA] 学习等级收集)
extern s8 gBattleDlgLearnCount; ///< 0x03000968 习得列表长度 (sub_8045860 返回; 94F0 按 (s8) 比较)
extern u8 gBattleDlgLearnIdx; ///< 0x03000969 习得列表遍历光标
extern u16 *gBattleDlgLearnGfx; ///< 0x0300096C 当前习得项图形段指针 (sub_804ACC0 切 0x0839B462 段表返回; sub_8049B70 消费)
extern s8 gResultsDropCount;
extern s8 gResultsDropSelIdx;
extern u32 gResultsDropTablePtr;
extern u8 gResultsStepDone;
extern s8 gResultsViewKind;
extern u32 *gResultsStatePtr;
extern u8 gChoiceSubIdx;
/* ---- 战斗结算/属性计算 (sub_80494F0 结算总驱动; BattleDrops_Roll 写掉落表): 
 * Results*=结果屏掉落/视图状态, SkillHealAmount=技能回血量, StatRecalc*=能力重算
 * (sub_8048ACC 递归重算 statMods), FxReq*=能力变化演出请求(sub_8048B30, 第三参=
 * gUnk_08393B28 效果索引) */
extern u8 gStatRecalcKind;
extern u8 gFxReqTimer;
extern u8 gFxReqKind;
extern u8 gFxReqFrames;
extern u8 gFxReqCounter;
#define gUnk_03000904 gFxReqCounter
extern u16 gFxReqAnimIdx;
extern u16 gSkillHealAmount;
extern u8 gBattleIntroState;      ///< 战斗开场 BGM/淡入演出状态机 (sub_8049C1C): 0..4
extern u8 gBattleIntroTimer;      ///< 上述状态机的帧计数
extern u8 gBattleIntroObj[];      ///< 战斗开场对象的 ObjHead (sub_804AD60 用 sub_801B81C 装配)
extern u8 gBattleIntroPhase;      ///< 开场阶段标志 (0/1/2, sub_804ADE0/ADF8 设置)
extern u8 gBattleDlgPartySlots[5]; ///< 0x03000974 参战成员槽号表 (sub_80489E8(pool,·,0,0x7F) 输出, 0..4 号位)
extern u8 gBattleDlgPartyCount; ///< 0x03000979 参战成员数 (sub_80489E8 返回)
extern u8 gBattleDlgPartyIdx; ///< 0x0300097A 参战遍历光标 (sub_8048FB8 case1/2 逐个登场动画)
extern u8 gBattleDlgFlashState; ///< 0x0300097B 调色板闪光状态机状态 (sub_8048F0C: 0=空闲 1=启动 2/3=等待; sub_804AB10 置 1)
extern u8 gBattleDlgFlashTimer; ///< 0x0300097C 调色板闪光帧计数 (sub_8048F0C)
extern u8 gBattleDlgFlashPal; ///< 0x0300097D 闪光对象的 OBJ 调色板槽 (= pool[0x949].headA.palSlot; sub_804B96C/804C4D8 淡变)
extern u8 gBattleIntroFadeFlag;   ///< sub_804ADF8 复位 (淡入/开场演出标志)
extern u8 gTurnStep;              ///< 0x0300097F 回合状态机阶段 (sub_804A148 置 1, sub_804A368 分派 1..18)
extern u8 gTurnAilSlots[12];      ///< 0x03000988 异常状态槽号表 (sub_804A148/sub_804AA2C 收集)
extern u8 gTurnAilCount;          ///< 0x03000994 异常状态槽数
extern u8 gTurnAilIdx;            ///< 0x03000995 异常状态遍历光标 (sub_804AA2C 清 0, sub_804A368 消费)
extern u16 gTurnAilMask;          ///< 0x03000996 异常状态综合掩码 (sub_804A148 按 statusAil 累积, sub_804A368 判 bit)
extern u8 gTurnActSlots[12];      ///< 0x030009B0 回合行动待选槽号表 (sub_804A148 收集, sub_804A368 消费)
extern u8 gTurnActCount;          ///< 0x030009BC 回合行动待选槽数
extern u8 gTurnActIdx;            ///< 0x030009BD 回合行动遍历光标 (sub_804A148 清 0, sub_804A368 消费)

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
extern u16 gObjPalSlotUsed; ///< 0x03000AE0 OBJ 调色板槽占用位图 bit0-15 (gObjPalAnim@AE8 系统的 16 槽; 装载/占用置位 sub_804C2FC/804C364, 释放清位 sub_804C3A4, 找空槽扫描 sub_804B654/804B96C)
extern u16 gBgPalSlotUsed;  ///< 0x03000AE2 BG 调色板槽占用位图 bit0-15 (gBgPalAnim@BE8 系统的 16 槽; 装载/占用置位 sub_804C548/804C5B8, 释放清位 sub_804C5F8, 找空槽扫描 sub_804BBDC mode3/804BF14)
extern u8 gUnk_03000AE4; ///< 战斗动画子系统状态字节 (与 AE0/AE2 位图同簇): 全 ROM 唯一消费 = sub_804B288 复位 strb 0 清零; 无任何读者 (2026-09-14 E0 全 ROM 扫描: 字面池/基址偏移/字半字区间重叠/数据指针均无)。AE6/AE7 未登记
extern u8 gUnk_03000AE5; ///< 同 gUnk_03000AE4: 仅 sub_804B288 清零, 无读者 — 语义不可考, 勿臆测改名
/* 调色板动画条目 (16 字节 × 16 项 = 256 字节)。
 * 0x03000AE8 = OBJ 调色板动画表 (目的 0x05000200 = OBJ 调色板 RAM / 镜像 0x02036AC0);
 * 0x03000BE8 = BG 调色板动画表 (目的 0x05000000 = BG 调色板 RAM / 镜像 0x02036CC0); 布局相同。
 * ⚠ 2026-09-14 纠正: 旧命名 gBgPalAnim(AE8)/gObjPalAnim(BE8) 的 BG/OBJ 标签与 GBA 硬件互换
 * (E0: BG palette=0x05000000..0x1FF, OBJ palette=0x05000200..0x3FF; E2: AE8 系槽全部经
 * headA.palSlot 作精灵调色板装载/释放 — battle_object_engine.c:153/2672, battle_stage_dialogue.c:92 等), 已互换。
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
extern PaletteAnimEntry gObjPalAnim[];  /* 0x03000AE8 OBJ 调色板动画表 (目的 0x05000200 / 镜像 0x02036AC0; 旧名 gBgPalAnim) */
extern u16 gUnk_03000CE8; // 2026-09-11 zcode-engine 登记 (sub_804B288)
extern PaletteAnimEntry gBgPalAnim[];   /* 0x03000BE8 BG 调色板动画表 (目的 0x05000000 / 镜像 0x02036CC0; 旧名 gObjPalAnim) */
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
/* ---- 物件使用演出 (ItemUseFx_Update 按 obj[0xA4] 分派 ItemUseFx_RunConsumable/ItemUseFx_RunWeapon 两台同构
 * 状态机, 状态机 PC 共用 gItemUseFxState 0..13, 返回 1 = 演出结束; ItemUseFx_Reset 复位)。
 * E3 道具名 (gItemNames): RunWeapon 分支 ids 0x19..0x31 = 武器攻击道具 (アイスハンマー/
 * フレイムロッド 等, 其中 8 个有 dmgAmount 预设 0xA7..0xAD/2), RunConsumable 分支 ids
 * 0xDD..0xE4 = 回复/辅助消耗品 (ヒールガム/エンジェルティア 等)。
 *  - gItemUseFxState: 状态机 PC。0 起手保存动画参数 → 1 播动画(Sfx 0x17) → 2 等帧尾并
 *    用 SavedF2A/SavedPalSlot 恢复原动画 → 5/6 等音轨 → 7/8 从目标 headB 帧表
 *    (sub_801B8FC) 取循环区间 → 9 目标 headB+0x58 当前帧在 [LoopFrom,LoopTo) 内逐帧
 *    循环, 每圈 LoopsDone++, 播满 LoopCount 圈 → 10..12 等窗口/收尾 → 13 结束。
 *  - gItemUseFxLoopFrom/To (u16): headB 循环帧区间 (RunWeapon case8, 末记录 +2);
 *    gItemUseFxLoopCount (u8): 圈数 (case0 固定 3); gItemUseFxLoopsDone (u8): 已播圈数。
 *  - gItemUseFxSavedF2A (u16)/SavedPalSlot (u8): case0 保存的 obj+0x2A (headA.f_1E)
 *    与 obj+0x35 (headA.palSlot), 供 sub_801CBA4 恢复用。 */
extern u8 gItemUseFxState;    /* 0x03000DDE */
extern u16 gItemUseFxLoopFrom; /* 0x03000DE0 */
extern u16 gItemUseFxLoopTo;   /* 0x03000DE2 */
extern u8 gItemUseFxLoopCount; /* 0x03000DE4 */
extern u8 gItemUseFxLoopsDone; /* 0x03000DE5 */
extern u16 gItemUseFxSavedF2A;    /* 0x03000DE6 */
extern u8 gItemUseFxSavedPalSlot; /* 0x03000DE8 */
/* 布局视图 (0x03000DDE..0x03000DE8+2)。0x03000DDF 与 0x03000DE9..0x03000DEF 未用 (无字面量引用)。
 * ⚠ 匹配输入约定: 未匹配的 ItemUseFx_RunWeapon 候选 C 应使用扁平符号 (gItemUseFxState 等) ——
 * ROM 字面量 0x03000DDE/DE0/DE2/DE4/DE5 与之一一对应; 结构体成员访问会改写寻址形状 (基址+偏移)
 * 破坏字节匹配。结构体仅作布局文档/语义分组。 */
typedef struct ItemUseFx
{
    u8 state;        /* +0x00 = gItemUseFxState (PC 0..13) */
    u8 pad_1;
    u16 loopFrom;    /* +0x02 = gItemUseFxLoopFrom (目标 headB 循环帧起点) */
    u16 loopTo;      /* +0x04 = gItemUseFxLoopTo (循环帧终点) */
    u8 loopCount;    /* +0x06 = gItemUseFxLoopCount (圈数, 固定 3) */
    u8 loopsDone;    /* +0x07 = gItemUseFxLoopsDone (已播圈数) */
    u16 savedF2A;    /* +0x08 = gItemUseFxSavedF2A */
    u8 savedPalSlot; /* +0x0A = gItemUseFxSavedPalSlot */
    u8 pad_B;
} ItemUseFx;
extern ItemUseFx gItemUseFx; /* 0x03000DDE (结构体基址视图) */
/* ---- 多对象同步演出登记 (sub_801DC20 建对象时经 BattleFxObjs_Add 登记, 上限 5;
 * BattleFx_Update 播放: [0]=模板, 模板 obj+0x3C 起 0x30B (headB 块) 复制给其余对象,
 * 播完清全体 state 的 0x2000 跳跃位; BattleFx_Reset 清, sub_801BE34 驱动) */
extern u32 gBattleFxObjs[];     /* 0x03000DF0 (5 项) */
extern u8 gBattleFxObjCount;    /* 0x03000E04 (BattleFx_GetObjCount 返回值) */
extern u8 gBattleFxState;       /* 0x03000E05 (BattleFx_Update 状态机 PC 0/1/2/4) */
/* 布局视图 (0x03000DF0..0x03000E05+1)。0x03000E06/7 未用。匹配输入约定同上:
 * 已匹配函数用扁平符号, 候选 C 亦然 (ROM 字面量 = 各地址)。 */
typedef struct BattleFx
{
    u32 objs[5]; /* +0x00 = gBattleFxObjs[0..4], [0]=模板 */
    u8 objCount; /* +0x14 = gBattleFxObjCount */
    u8 state;    /* +0x15 = gBattleFxState (BattleFx_Update PC 0/1/2/4) */
    u8 pad_16[2];
} BattleFx;
extern BattleFx gBattleFx; /* 0x03000DF0 (结构体基址视图) */
/* ---- 战后掉落结果表 (BattleDrops_Roll: 按敌方/special 槽号查 LUT 0x0839D9B8[101]/0x0839DBB1[]
 * 两段式掷取 (60% 门 + 阈值), 任一我方装备效果字段5==4 (幸运系装备) 时道具 id +15
 * 升稀有档 (E3: 0xDDヒールガム→0xECドラゴンリング, 0xE3ポイズンクリーン→0xF2ドロボウのこころ),
 * 去重写入本表
 * 并经 Inventory_AddItem 直接进背包; *out=本表, 返回记录数。消费者 sub_80494F0 战斗结算流程:
 * 数量→gUnk_030009BE, 表指针→gUnk_030009C0) */
typedef struct BattleDropEntry
{
    u8 itemId; /* 0x03000E08[i]+0: 道具 id (E3: 传 Inventory_AddItem 索引 gInventory) */
    u8 count;  /* +1: 同 id 掷中次数 (E3: Inventory_AddItem 的增量) */
    u16 pad_2;
} BattleDropEntry;
extern BattleDropEntry gBattleDrops[]; /* 0x03000E08 (容量>=10, 死代码 BattleDrops_Clear 清 i<=9) */
extern u8 gBattleDropCount;            /* 0x03000E30 已用记录数 */
/* 布局视图 (0x03000E08..0x03000E30)。容量恰为 10: items[10] 末尾与 count 相邻 (E0 相邻性),
 * 且 BattleDrops_Roll 追加记录时无上界检查 (>10 个不同道具会覆写 count, 未观察到)。
 * 匹配输入约定同上: BattleDrops_Roll/BattleDrops_Clear 候选 C 用扁平符号
 * (ROM 字面量 0x03000E08/0x03000E30 与之一一对应)。 */
typedef struct BattleDrops
{
    BattleDropEntry items[10]; /* +0x00 = gBattleDrops[0..9] */
    u8 count;                  /* +0x28 = gBattleDropCount */
} BattleDrops;
extern BattleDrops gBattleDropsBlk; /* 0x03000E08 (结构体基址视图) */
/* ---- 战后卡片掉落结果表 (sub_804EC04 / BattleCards_Roll:
 * 遍历敌方怪物按怪物槽号掷取卡片/图鉴 entry, 查 sub_804E76C 装备属性 (字段 5==2 / 5==3) 提升掉落率,
 * 去重写入本表, 并逐项通过 SaveTimer_Inc 递增卡片图鉴计数。*out=本表, 返回记录数。
 * 容量 10 项, 布局与 BattleDropEntry 一致: {cardId@+0, count@+1, pad_2@+2}) */
typedef struct BattleCardDropEntry
{
    u8 cardId; /* 0x03000E38[i]+0: 卡片/怪物图鉴 id */
    u8 count;  /* +1: 掉落计数 */
    u16 pad_2;
} BattleCardDropEntry;
extern BattleCardDropEntry gBattleCardDrops[]; /* 0x03000E38 (容量 10) */
extern u8 gBattleCardDropCount;                /* 0x03000E60 已用卡片记录数 */
extern u8 gScriptReturnSetId; /* 0x03000E68 ScriptSet_Load 记挂的脚本集号; 脚本退场时还原到 gEnvScriptSetId */
extern u8 gScriptPendingEntry; /* 0x03000E69 mode==2 记挂的入口号, 解压完成后跳 entryTbl[本值] */
extern u32 gScriptCursor; /* 0x03000E6C: 脚本 VM PC 槽, 存当前 opcode 字节地址 (EWRAM 脚本区) */
extern u16 gScriptVmFlags;
extern u8 gScriptDialogPhase;        /* 0x03000E72: Op_DialogText 13 态状态机的当前态 (0..12) */
extern u8 gDialogTimer;              /* 0x03000E73: Op_DialogText 复用字节 —— 态6 光标闪烁相位 (0..0xF 自增回绕), 态11 等待帧数目标 */
extern u8 gUnk_03000E74;             /* 0x03000E74: 脚本等待帧计数 (Op_WaitFrames 与 Op_DialogText 态11 共用) */
extern u16 gDialogTextCursor;        /* 0x03000E76: Op_DialogText 文本 token 流字节游标 (每 token 前进 2) */
extern u8 gScriptCallStackDepth;
extern u32 gScriptCallStack[];
/* 脚本 VM 流式子脚本 (OP_SCRIPT_STREAM_LZ=0x15 / OP_SCRIPT_RETURN_CHUNK=0x16) 调用栈,
 * 与同集 gosub 栈 gScriptCallStackDepth/gScriptCallStack 平行; 容量各 8 项 (区域 0x20 字节):
 * 进子脚本时压入调用方上下文, 返回时弹出并恢复; 深度无边界检查 (Script_ResetVM 清零)。 */
extern u32 gScriptStreamCursorStack[]; /* 0x03000EA0: 每层保存的调用方字节码指针 (STREAM_LZ 写入, RETURN_CHUNK 恢复到 gScriptCursor) */
extern u8 gScriptStreamSetIdStack[];   /* 0x03000EC0: 每层保存的调用方脚本集号 (弹栈时还原 gScriptReturnSetId, 同时镜像 gScriptCurSetId) */
extern u8 gScriptStreamEntry;          /* 0x03000EC8: STREAM_LZ 记录的子脚本入口号槽 (仅写入; 匹配 asm 中存储先于新操作数读取, 全工程无读者) */
extern u8 gScriptStreamSetId;          /* 0x03000EC9: STREAM_LZ 记录的子脚本集号槽 (同上, 与 gScriptStreamEntry 成对) */
extern u8 gScriptStreamDepth;          /* 0x03000ECA: 流式子脚本嵌套深度 (=EA0/EC0 栈顶计数, 进 ++/出 --/复位清 0) */
extern u8 gDialogWindowTileX;
extern u8 gDialogWindowTileY;
extern u8 gDialogTextX;              /* 0x03000ECD: Op_DialogText 文本绘制列 (token 0x0900 复位为 data[2], 每字 +1) */
extern u8 gDialogTextY;              /* 0x03000ECE: Op_DialogText 文本绘制行 (token 0x0900 换行 +2, 每字占 2 行) */
/* 脚本 VM 局部槽 + 对话 tile DMA/绘制状态簇 (0x03000ED8..0x03000F30, 2026-09-14 zcode-tile-edx 定名):
 * - gScriptLocalSlots: 脚本 VM 8 个 u16 局部槽 (ScriptPump_JumpToEntry 全置 0xFFFF;
 *   ScriptPump_Run 每帧经 sub_80182A8(按键, 本表) 刷新按键等待槽)。
 * - TileDma 族: 对话 BG 动态 tile 装载 (LZ/块从 ROM 解到 0x0203DE00 暂存, FlushTileDma
 *   DMA 刷到 VRAM 0x0600B800, 64B/块)。gTileDmaAllocTable[30] = 已登记动态 tile 值
 *   (0xE0+i 重定向, sub_805063C 线性查重), gTileDmaCount = 登记数 (= 待刷块数)。
 * - gTextDrawPos/gTileAnimFrameIdx/gTileAnimCharIdx: 脚本文本/图块绘制坐标 (x|y<<8)
 *   与 tile 动画状态 (帧表 gUnk_0862D574 按 charIdx*18 选、frameIdx*2 取帧)。
 * - gScriptKeyState/gScriptKeysPressed: 脚本活动期间的按键状态与本帧新按下沿
 *   (ScriptPump_Run: keys=~REG_KEYINPUT, pressed=keys&~prev)。
 * - gScriptCurSetId: 当前调用栈层的脚本集 id (sub_80513A0 从 gScriptStreamSetIdStack[栈深] 取)。 */
extern u16 gScriptLocalSlots[8]; ///< 0x03000ED8 (原 gUnk_03000ED8 u8 视图废弃)
extern u16 gTileDmaAllocTable[30]; ///< 0x03000EE8 (原 gUnk_03000EE8; TileDma_Reset 清 0..0x1D)
extern u16 gTileDmaCount; ///< 0x03000F24 (原 gUnk_03000F24; 登记数 = 待刷块数)
extern u16 gTileDmaLastBlockIdx; ///< 0x03000F26 (原未登记; sub_80501B8 尾部写 = gTileDmaCount-1, 全 ROM 单写点, 强推断)
extern u16 gTextDrawPos; ///< 0x03000F28 (原未登记; sub_8050720 从脚本 data[2]/data[3] 组装 x|y<<8)
extern u8 gTileAnimFrameIdx; ///< 0x03000F2A (原 gUnk_03000F2A; sub_805063C 写帧后 ++, sub_805144C 归零)
extern u8 gTileAnimCharIdx; ///< 0x03000F2B (原未登记; sub_805144C 从脚本数据装载, 选 0x0862D574 动画表项)
extern u16 gScriptKeyState; ///< 0x03000F2C (原 gUnk_03000F2C)
extern u16 gScriptKeysPressed; ///< 0x03000F2E (原 gUnk_03000F2E)
extern u8 gScriptCurSetId; ///< 0x03000F30 (原 gUnk_03000F30)

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
extern u32 gGameTimer; // 3001948
extern u32 gUnk_03001950[14];
extern u16 gHBlankScrollCounter;

extern struct LzContext gLzContext; // 3001990

extern u16 gHeldKeysRaw;

extern u8 gHBlankWaveV[];

extern u8 gMainLoopMode;
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
extern u8 gSceneEntryFlag;
extern u16 gCameraTargetX;
extern s16 gCameraPosY;

extern u16 gScenePhase;
extern u8 gLogoEffectState;
extern u16 gTitleFadeTimer;
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
extern u16 gWindowTransitionProgress;
/* Iris-transition progress (0 = closed, 240 = fully open). */
extern u8 gSceneTransitionArg;
extern u8 gCameraDrawMode;
extern u16 gHBlankEffectMode;
extern u16 gMoveCmdSetId;
extern u8 gUnk_03004618;
extern u16 gCameraPanStartY;

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
extern u16 gCameraPanTargetX;
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
extern u16 gWindowTransitionProgressSnapshot;
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
extern u16 *gPendingPortraitPalette;
extern u16 gBg1ScrollMode;

#include "map_scene_runtime.h"

/* 当前地图的区域头表指针 (MapScene_Load 从 0x087EBB20[mapIdx] 装载):
 * {u32 cells; u32 type0..type4} — cells: {u8 count, {u8 xTile, u8 yTile, u8 type, u8 entryIdx}[count]}
 * typeN: MapZone_Trigger 各动作的记录表 (type0/1 记录 8B, type2 4B, type3 2B, type4 4B) */
extern u32 *gMapZoneHeader;
extern u8 gSpawnFacingDir;
/* Fade progress in scanline units; ScreenFade_Start initializes it to 0 or 0x1B0. */
extern s16 gScreenFadeProgress;
extern u16 gUnk_030047AC;
extern u16 gCurrentMapId;
extern u8 gCameraPanDuration;
extern u8 gChoiceDestY;
extern u8 gChoiceGroupIdx;

extern u16 gDrawCamX;
extern u16 gUnk_030047C4;
extern u16 gIntroBgTransferStage; // 0=无, 1=tiles 已暂存 0x02020000, 2=tilemap 已暂存
extern u8 *gPendingPortraitGfx;
// extern Unk_03004670 gSlotPalId;
extern u8 gSlotPalId[];
/* 每个精灵表槽位(0..11)当前使用的调色板编号, 0xFF = 该槽空。写: SetSlotPalId。 */
extern u16 gCameraPanStartX;

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
extern u16 gCameraPanTargetY;

/* Third ScreenFade_Start argument; currently only its initialization is observed. */
extern s16 gScreenFadeParam;
extern u8 gZoneCheckTileXs[4];

extern u8 gPendingPortraitSlot;
/* 0 = no upload; otherwise portrait position + 1. */

extern u8 gWin0HWaveTable[];
/* 81 packed WIN0H boundaries generated for the iris transition. */
/* Nonzero while ScreenFx_SetMode has an in-flight window/palette transition. */
extern u8 gScreenTransitionState;
extern u8 gCameraPanStep;

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
extern u16 gUnk_03004D48;

extern u8 gUnk_03004D4C;
extern u8 gActiveSaveSlot;
extern u8 gSaveTimers[];

extern u16 gUnk_03004DBC;
extern u8 gBgTileReloadFlag;
extern u8 gUnk_03004DC0;
extern u8 gSaveBusyA;
extern u8 gSaveFlags[];

extern u8 gSaveSramBlock;
extern u8 gSaveUiParam;
extern u8 gSaveBusyB;
extern u16 gSavedDispCnt; /* 0x03004DDC 存档菜单进入前的 REG_DISPCNT */
extern u16 gSavedBldCnt; /* 0x03004DE0 同上, REG_BLDCNT */
extern u16 gUnk_03004DE4;

extern u8 gSioState[];

/* SIO 多机通信会话状态 (0x03004DF0)。gSioState 的结构化视图 (同址别名 gUnk_03004DF0):
 * 现存已匹配子函数按 u8 下标访问, 本结构供新匹配使用; 字段语义名待 SIO 族匹配后统一 (recvAccumMap/recvDoneMap/swapPending/
 * frameHasPacket/peerReady 已落地 2026-09-14 zcode; unk_8/unk_A/unk_18 及缓冲方向待定)。 */
typedef struct
{
    u8 isParent; // 0x00 1=主机(parent) 0=从机 (Sio_IsHost/SetReady 视作 mode)
    u8 stage; // 0x01 连接阶段
    u8 recvAccumMap; // 0x02 收包位图累积
    u8 recvDoneMap; // 0x03 收到位图
    u8 swapPending; // 0x04 包双缓冲交换标志
    u8 frameHasPacket; // 0x05 帧完成标志 (本帧有包)
    u8 peerReady; // 0x06 对端就绪
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
} SioCommState;
extern SioCommState gSioCommState;

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

extern u8 gObjSlotFxCmd[];

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

/* iwram consumers also need menu title enum/constants; include after iwram types are defined. */
#include "menu.h"

#endif
