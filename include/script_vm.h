#ifndef GUARD_SCRIPT_VM_H
#define GUARD_SCRIPT_VM_H

#include "gba/types.h"

/* ========================================================================== */
/* 脚本虚拟机 (Script VM) 操作码枚举 (0x00 .. 0x4F, 共 80 项)                 */
/* 分发表位于 ROM 0x0862D434 (gScriptOpcodeHandlers)                           */
/* ========================================================================== */

enum ScriptOpcode
{
    OP_DIALOG_MESSAGE       = 0x00, /* 0x08050720: 对话消息处理 */
    OP_SCRIPT_JUMP          = 0x01, /* 0x08052858: 脚本无条件跳转 (ScriptGotoEntry) */
    OP_SCRIPT_CALL          = 0x02, /* 0x08052878: 脚本子例程调用 */
    OP_SCRIPT_RETURN        = 0x03, /* 0x080511A0: 脚本返回 */
    OP_CHARA_CONTROL        = 0x04, /* 0x0804F280: 角色运动/移动控制 */
    OP_NOP                  = 0x05, /* 0x080528C4: 空操作 (跳过 1 字节) */
    OP_SCRIPT_STOP          = 0x06, /* 0x08051230: 脚本停止/挂起 */
    OP_BGM_PLAY             = 0x07, /* 0x08052B80: 播放 BGM */
    OP_BGM_STOP             = 0x08, /* 0x08052BA0: 停止 BGM */
    OP_BGM_VOLUME           = 0x09, /* 0x08052BE0: 调节 BGM 音量 */
    OP_BGM_FADE_IN          = 0x0A, /* 0x08052C04: BGM 淡入 */
    OP_BGM_FADE_OUT         = 0x0B, /* 0x08052C24: BGM 淡出 */
    OP_SFX_PLAY             = 0x0C, /* 0x08052C90: 播放音效 */
    OP_SFX_STOP             = 0x0D, /* 0x08052CD0: 停止音效 */
    OP_RANDOM_JUMP          = 0x0E, /* 0x08052CF0: 随机概率跳转 */
    OP_SCRIPT_CALL_ALT      = 0x0F, /* 0x08052D4C: 备用脚本调用 */
    OP_WAIT_CHARS_STOP      = 0x10, /* 0x08052D8C: 等待所有角色停止移动 */
    OP_LOAD_CHARA_GFX       = 0x11, /* 0x08052DAC: 装载角色图块图形 */
    OP_LOAD_CHARA_PAL       = 0x12, /* 0x08052DCC: 装载角色调色板 */
    OP_WAIT_SPRITE_LOAD     = 0x13, /* 0x08052E0C: 等待精灵传输完成 */
    OP_SCENE_CHANGE_FADE    = 0x14, /* 0x08052E2C: 带淡出切场景 */
    OP_SCRIPT_STREAM_LZ     = 0x15, /* 0x080512C4: LZ 数据流解压加载 */
    OP_SCRIPT_RETURN_CHUNK  = 0x16, /* 0x080513A0: 脚本返回解压块 */
    OP_DIALOG_TEXT          = 0x17, /* 0x0805144C: 渲染对话文本 */
    OP_SCENE_CHANGE_PLAIN   = 0x18, /* 0x08052E4C: 直接切场景 */
    OP_WAIT_SCENE_IDLE      = 0x19, /* 0x08052E6C: 等待场景就绪 */
    OP_CAMERA_PAN           = 0x1A, /* 0x0804F64C: 相机平移/缓动 */
    OP_LOAD_MAP             = 0x1B, /* 0x08052E80: 地图装载 */
    OP_REMOVE_PARTY_MEMBER  = 0x1C, /* 0x0804F768: 移除队伍成员 */
    OP_ADD_PARTY_MEMBER     = 0x1D, /* 0x0804F7F8: 添加队伍成员 */
    OP_IF_EVENT_FLAG_JUMP   = 0x1E, /* 0x08052E9C: 事件标志满足时跳转 */
    OP_SET_EVENT_FLAG       = 0x1F, /* 0x08052EC0: 置位事件标志 */
    OP_CLEAR_EVENT_FLAG     = 0x20, /* 0x08052F20: 清除事件标志 */
    OP_IF_SWITCH_JUMP       = 0x21, /* 0x08052F44: 脚本开关满足时跳转 */
    OP_SCRIPT_BATTLE        = 0x22, /* 0x0804F8D8: 触发脚本剧情战 */
    OP_DIALOG_SETUP         = 0x23, /* 0x080528C8: 对话框初始化 */
    OP_OPEN_WINDOW          = 0x24, /* 0x08051A1C: 打开 UI 窗口 */
    OP_CLOSE_WINDOW         = 0x25, /* 0x0805291C: 关闭 UI 窗口 */
    OP_WAIT_FRAMES          = 0x26, /* 0x080529B8: 脚本等待指定帧数 */
    OP_SET_SWITCH           = 0x27, /* 0x08052FAC: 置位指定开关 */
    OP_CLEAR_SWITCH         = 0x28, /* 0x08052FC8: 复位指定开关 */
    OP_CAMERA_SNAP          = 0x29, /* 0x08052FE4: 相机瞬间对齐目标 */
    OP_CAMERA_FOLLOW        = 0x2A, /* 0x08053000: 相机跟随指定对象 */
    OP_WAIT_CAMERA_PAN      = 0x2B, /* 0x08053024: 等待相机平移完成 */
    OP_LOAD_CUTSCENE_ANIM   = 0x2C, /* 0x08053040: 过场动画装载 */
    OP_RESTART_CHARA_ANIM   = 0x2D, /* 0x0805305C: 重启角色动作 */
    OP_WAIT_CHARA_ANIM      = 0x2E, /* 0x08053078: 等待角色动作完成 */
    OP_IF_PARTY_MEMBER_JUMP = 0x2F, /* 0x0805309C: 指定同伴在队时跳转 */
    OP_LOAD_ANIM_SET        = 0x30, /* 0x080530B4: 动画槽资源装载 */
    OP_ANIM_SLOT_RESUME     = 0x31, /* 0x080530D4: 恢复指定动画槽 */
    OP_ANIM_SLOT_PAUSE      = 0x32, /* 0x08053104: 暂停指定动画槽 */
    OP_DIALOG_CHOICE        = 0x33, /* 0x08051BE4: 弹出选项分支 */
    OP_WAIT_ANIM_SLOT_IDLE  = 0x34, /* 0x08052A14: 等待动画槽空闲 */
    OP_MENU_LOAD_ANIMS      = 0x35, /* 0x08052A38: 菜单动画装载 */
    OP_MENU_UNLOCK          = 0x36, /* 0x08052A50: 解锁菜单操作 */
    OP_MENU_LOCK            = 0x37, /* 0x08052A70: 锁定菜单操作 */
    OP_WAIT_MENU_READY      = 0x38, /* 0x08052A8C: 等待菜单状态就绪 */
    OP_FULL_HEAL_PARTY      = 0x39, /* 0x08052AA8: 全体队伍满血满状态 */
    OP_EQUIP_ITEM           = 0x3A, /* 0x08052ACC: 装备道具 */
    OP_IF_ITEM_QTY_JUMP     = 0x3B, /* 0x08053138: 道具数量判定跳转 */
    OP_CHEST_OPEN           = 0x3C, /* 0x0805316C: 宝箱开启标记 */
    OP_SAVE_UI_TRIGGER      = 0x3D, /* 0x0805318C: 触发存档 UI 界面 */
    OP_IF_SAVE_LOADED_JUMP  = 0x3E, /* 0x080531A8: 读档状态成立跳转 */
    OP_SAVE_TIMER_A         = 0x3F, /* 0x080531E4: 存档定时器控制 A */
    OP_SAVE_TIMER_B         = 0x40, /* 0x08053200: 存档定时器控制 B */
    OP_IF_SAVE_FLAG_JUMP    = 0x41, /* 0x0805321C: 存档标志判定跳转 */
    OP_SAVE_OP              = 0x42, /* 0x08053254: 执行存档操作 */
    OP_SET_FLAGS_LIST       = 0x43, /* 0x08053270: 批量置位标志列表 */
    OP_CLEAR_FLAGS_LIST     = 0x44, /* 0x080532DC: 批量清零标志列表 */
    OP_IF_ALL_FLAGS_JUMP    = 0x45, /* 0x0804F974: 标志全为 1 跳转 */
    OP_IF_ALL_FLAGS_CLR_JMP = 0x46, /* 0x0804FA04: 标志全为 0 跳转 */
    OP_IF_ANY_FLAG_JUMP     = 0x47, /* 0x0804FA94: 标志任意为 1 跳转 */
    OP_CLEAR_SWITCH_TAIL    = 0x48, /* 0x08053348: 尾部开关清零 */
    OP_IF_MONEY_JUMP        = 0x49, /* 0x08053360: 金钱数量判定跳转 */
    OP_START_LOGO_FADE      = 0x4A, /* 0x080533A0: 启动 Logo 淡出 */
    OP_WAIT_LOGO_FADE       = 0x4B, /* 0x080533B4: 等待 Logo 淡出完成 */
    OP_SET_CHARACTER_LEVEL  = 0x4C, /* 0x080533D4: 设定角色等级 */
    OP_SYS_EFFECT           = 0x4D, /* 0x0804FB24: 通用系统/视口特效 */
    OP_GIVE_TAKE_ITEM       = 0x4E, /* 0x08052AE8: 获得/消耗道具 */
    OP_SILVER_ADD_SUB       = 0x4F, /* 0x08052B34: 增加/扣除银币 (金钱) */
};

/* ========================================================================== */
/* 脚本 VM 通用系统/屏幕特效 op 0x4D 的配套定义                               */
/* ========================================================================== */

/* 外部调色板数据声明 */
extern const u16 gFlashFxPaletteTable[]; /* 0x0808B1B4 = gMenuEntPaletteFrames 项 0x7C (linker 别名, 闪光帧组) */
extern const u8 gObjPalFadeInSteps[];    /* 0x08289B6E: OBJ 调色板 10 帧渐显步进表, 每步 0x20 */
extern const u16 gObjPalFadeInFinal[];   /* 0x083936A8: 渐显末帧 16 色 (→ OBJ bank10) */

/* opcode 0x4D 子命令 (insn[1]); insn[2]=arg 区分 执行族(arg!=0) / 复位族(arg==0) */
enum SysFxSubOp
{
    SYSFX_SHAKE      = 0x00, /* 视口抖动: 开(掩码 1/3/7 按 arg) / 关 */
    SYSFX_FLASH      = 0x01, /* 白闪: 开 / 关 */
    SYSFX_BGLAYER    = 0x02, /* BG 层显示(mapId==0x63→BG2, 否则 BG3): 开 / 关 */
    SYSFX_CAMEASE    = 0x03, /* 相机滚动缓动 (gDrawCamEaseActive): 开 / 关 */
    SYSFX_PALFX_SEQ  = 0x04, /* 调色板渐变序列(arg=子步 1..5) / 复位计数 [14][15] */
    SYSFX_SAVE_OPEN  = 0x05, /* 存档菜单(Save_Fsm(1)) / 读档 UI(SaveUi_OpenLoad) */
    SYSFX_WAIT_A     = 0x06, /* 等按 A → Script_Abort+System_ResetToLogo (仅复位族) */
    SYSFX_OBJPAL_IN  = 0x07, /* OBJ 调色板 10 帧渐显 / 复位计数 */
    SYSFX_SAVEUI_PAL = 0x08, /* 存档 UI OBJ 调色板分块装载步进机 (仅复位族) */
    SYSFX_HEAL       = 0x4D, /* FullHealCharacter(arg) */
    SYSFX_BGM_RESUME = 0x64, /* Bgm_Continue (仅复位族; 实测案例 4D 64 00) */
    SYSFX_MAPBG      = 0xC8, /* MapBg_LoadFull(arg+0x81) / DISPCNT|=0x100 开 BG0 */
    SYSFX_INTROBG    = 0xC9, /* IntroBg_Load(arg) / IntroBg_Load(0) */
    SYSFX_WHITEOUT   = 0xCA, /* 白化淡入(arg=1)/淡出(arg=2) / 复位计数 [10] */
};

/* SYSFX_SAVEUI_PAL 的装载步 (flags[VF_SAVEUI_STEP], 0..3 循环) */
enum SysFxSaveUiStep
{
    SYSFX_SAVEUI_FILL_ROW0  = 0, /* BgMap_FillRow(0)+MenuEnt_ParseRange(3,0x17)+解锁 */
    SYSFX_SAVEUI_WAIT_ROW0  = 1, /* 等 MenuEnt_GetState(3)==0 */
    SYSFX_SAVEUI_FILL_ROW1  = 2, /* BgMap_FillRow(1)+MenuEnt_ParseRange(3,0x18)+解锁 */
    SYSFX_SAVEUI_WAIT_ROW1  = 3, /* 等完成后步号归 0 */
};

/* SYSFX_SHAKE 的抖动掩码档位 (arg) */
enum SysFxShakeMask
{
    SYSFX_SHAKE_MASK_PLANE1 = 1, /* 仅 plane1 抖动 */
    SYSFX_SHAKE_MASK_BOTH   = 2, /* 双 plane 抖动 (掩码 3); 其它 arg → 掩码 7 */
};

/* SYSFX_PALFX_SEQ 的子步 (arg, 1..5) */
enum SysFxPalStep
{
    SYSFX_PAL_BLACKIN  = 1, /* BLDY 渐黑 ramp (帧数[15]>>3) */
    SYSFX_FLASH_SETUP  = 2, /* 关 OBJ + BLDCNT=0xC10 + DMA 闪光调色板→OBJ bank4 */
    SYSFX_BG_PAL_CLEAR = 3, /* gCutsceneGfxBuf 清零 → BG PLTT 16..63 */
    SYSFX_FADE_2PH     = 4, /* 两相渐变 (帧数[15]>>2, 相位闩[14]) */
    SYSFX_FADE_LONG    = 5, /* 长渐变 (计数[11] 1..0x3E) */
};

/* ========================================================================== */
/* 外部数据表与分发表声明                                                     */
/* ========================================================================== */

extern u16 (*gScriptOpcodeHandlers[])(u32 *);
extern u8 *gUnk_087ED904[];
extern u8 gUnk_0862D574[];
extern u8 gUnk_03004980[];

/* ========================================================================== */
/* 脚本虚拟机执行函数与控制例程                                               */
/* ========================================================================== */

void ScriptPump_Run();
void sub_805008C();
u16 Script_GetFlags();
void Script_ResetVM();
void Script_Abort(u8);

void BgTiles_LoadSet(u16);
void TileDma_Reset();
s16 sub_80527AC(void);
u32 TileDma_GetCtx(u32 *);
u32 Op_LoadTileGfx(u8);

#endif /* GUARD_SCRIPT_VM_H */
