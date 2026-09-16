#ifndef GUARD_SCRIPT_VM_H
#define GUARD_SCRIPT_VM_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* ========================================================================== */
/* 脚本虚拟机 (Script VM) 操作码枚举 (0x00 .. 0x4F, 共 80 项)                 */
/* 分发表位于 ROM 0x0862D434 (gScriptOpcodeHandlers)                           */
/* ========================================================================== */

enum ScriptOpcode
{
    OP_DIALOG_MESSAGE       = 0x00, /* 0x08050720 */
    OP_SCRIPT_JUMP          = 0x01, /* 0x08052858 */
    OP_SCRIPT_CALL          = 0x02, /* 0x08052878 */
    OP_SCRIPT_RETURN        = 0x03, /* 0x080511A0 */
    OP_CHARA_CONTROL        = 0x04, /* 0x0804F280 */
    OP_NOP                  = 0x05, /* 0x080528C4: reserved; handler does not advance PC */
    OP_SCRIPT_STOP          = 0x06, /* 0x08051230 */
    OP_WAIT_CHARS_STOP      = 0x07, /* 0x08052B80 */
    OP_LOAD_CHARA_GFX       = 0x08, /* 0x08052BA0 */
    OP_LOAD_CHARA_PAL       = 0x09, /* 0x08052BE0 */
    OP_WAIT_SPRITE_LOAD     = 0x0A, /* 0x08052C04 */
    OP_SCENE_CHANGE_FADE    = 0x0B, /* 0x08052C24 */
    OP_SCENE_CHANGE_PLAIN   = 0x0C, /* 0x08052C90 */
    OP_WAIT_SCENE_IDLE      = 0x0D, /* 0x08052CD0 */
    OP_LOAD_MAP             = 0x0E, /* 0x08052CF0 */
    OP_IF_EVENT_FLAG_JUMP   = 0x0F, /* 0x08052D4C */
    OP_SET_EVENT_FLAG       = 0x10, /* 0x08052D8C */
    OP_CLEAR_EVENT_FLAG     = 0x11, /* 0x08052DAC */
    OP_IF_SWITCH_JUMP       = 0x12, /* 0x08052DCC */
    OP_SET_SWITCH           = 0x13, /* 0x08052E0C */
    OP_CLEAR_SWITCH         = 0x14, /* 0x08052E2C */
    OP_SCRIPT_STREAM_LZ     = 0x15, /* 0x080512C4 */
    OP_SCRIPT_RETURN_CHUNK  = 0x16, /* 0x080513A0 */
    OP_DIALOG_TEXT          = 0x17, /* 0x0805144C */
    OP_CAMERA_SNAP          = 0x18, /* 0x08052E4C */
    OP_CAMERA_FOLLOW        = 0x19, /* 0x08052E6C */
    OP_CAMERA_PAN           = 0x1A, /* 0x0804F64C */
    OP_WAIT_CAMERA_PAN      = 0x1B, /* 0x08052E80 */
    OP_REMOVE_PARTY_MEMBER  = 0x1C, /* 0x0804F768 */
    OP_ADD_PARTY_MEMBER     = 0x1D, /* 0x0804F7F8 */
    OP_LOAD_CUTSCENE_ANIM   = 0x1E, /* 0x08052E9C */
    OP_RESTART_CHARA_ANIM   = 0x1F, /* 0x08052EC0 */
    OP_WAIT_CHARA_ANIM      = 0x20, /* 0x08052F20 */
    OP_IF_PARTY_MEMBER_JUMP = 0x21, /* 0x08052F44 */
    OP_SCRIPT_BATTLE        = 0x22, /* 0x0804F8D8 */
    OP_DIALOG_SETUP         = 0x23, /* 0x080528C8 */
    OP_OPEN_WINDOW          = 0x24, /* 0x08051A1C */
    OP_CLOSE_WINDOW         = 0x25, /* 0x0805291C */
    OP_WAIT_FRAMES          = 0x26, /* 0x080529B8 */
    OP_LOAD_ANIM_SET        = 0x27, /* 0x08052FAD */
    OP_ANIM_SLOT_RESUME     = 0x28, /* 0x08052FC9 */
    OP_ANIM_SLOT_PAUSE      = 0x29, /* 0x08052FE5 */
    OP_WAIT_ANIM_SLOT_IDLE  = 0x2A, /* 0x08053001 */
    OP_MENU_LOAD_ANIMS      = 0x2B, /* 0x08053025 */
    OP_MENU_UNLOCK          = 0x2C, /* 0x08053041 */
    OP_MENU_LOCK            = 0x2D, /* 0x0805305C */
    OP_WAIT_MENU_READY      = 0x2E, /* 0x08053078 */
    OP_FULL_HEAL_PARTY      = 0x2F, /* 0x0805309C */
    OP_EQUIP_ITEM           = 0x30, /* 0x080530B4 */
    OP_GIVE_TAKE_ITEM       = 0x31, /* 0x080530D4 */
    OP_SILVER_ADD_SUB       = 0x32, /* 0x08053104 */
    OP_DIALOG_CHOICE        = 0x33, /* 0x08051BE4 */
    OP_BGM_PLAY             = 0x34, /* 0x08052A14 */
    OP_BGM_STOP             = 0x35, /* 0x08052A38 */
    OP_BGM_VOLUME           = 0x36, /* 0x08052A50 */
    OP_BGM_FADE_IN          = 0x37, /* 0x08052A70 */
    OP_BGM_FADE_OUT         = 0x38, /* 0x08052A8C */
    OP_SFX_PLAY             = 0x39, /* 0x08052AA8 */
    OP_SFX_STOP             = 0x3A, /* 0x08052ACC */
    OP_IF_ITEM_QTY_JUMP     = 0x3B, /* 0x08053138 */
    OP_CHEST_OPEN           = 0x3C, /* 0x0805316C */
    OP_SAVE_UI_TRIGGER      = 0x3D, /* 0x0805318C */
    OP_IF_SAVE_LOADED_JUMP  = 0x3E, /* 0x080531A8 */
    OP_SAVE_TIMER_A         = 0x3F, /* 0x080531E4 */
    OP_SAVE_TIMER_B         = 0x40, /* 0x08053200 */
    OP_IF_SAVE_FLAG_JUMP    = 0x41, /* 0x0805321C */
    OP_SAVE_OP              = 0x42, /* 0x08053254 */
    OP_SET_FLAGS_LIST       = 0x43, /* 0x08053270 */
    OP_CLEAR_FLAGS_LIST     = 0x44, /* 0x080532DC */
    OP_IF_ALL_FLAGS_JUMP    = 0x45, /* 0x0804F974 */
    OP_IF_ALL_FLAGS_CLR_JMP = 0x46, /* 0x0804FA04 */
    OP_IF_ANY_FLAG_JUMP     = 0x47, /* 0x0804FA94 */
    OP_CLEAR_SWITCH_TAIL    = 0x48, /* 0x08053348 */
    OP_IF_MONEY_JUMP        = 0x49, /* 0x08053360 */
    OP_START_LOGO_FADE      = 0x4A, /* 0x080533A0 */
    OP_WAIT_LOGO_FADE       = 0x4B, /* 0x080533B4 */
    OP_SET_CHARACTER_LEVEL  = 0x4C, /* 0x080533D4 */
    OP_SYS_EFFECT           = 0x4D, /* 0x0804FB24 */
    OP_RANDOM_JUMP          = 0x4E, /* 0x08052AE8 */
    OP_SCRIPT_CALL_ALT      = 0x4F, /* 0x08052B34 */
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

extern u16 (*const gScriptOpcodeHandlers[])(u32 *);
extern u8 *gUnk_087ED904[];
extern u8 gUnk_0862D574[];
extern const u32 gScriptSetTable[];

/* 脚本虚拟机运行与异步服务请求标志 (0x03000E70) */
#define SCRIPT_VM_FLAG_RUNNING           0x0001 /* bit0: 脚本当前正在运行 */
#define SCRIPT_VM_FLAG_WINDOW_BG_REQ     0x0010 /* bit4: 窗口/对话框 BG0 拷贝加载请求 */
#define SCRIPT_VM_FLAG_DIALOG_STATE      0x0020 /* bit5: 对话框翻页/状态标志 */
#define SCRIPT_VM_FLAG_TILE_DMA_REQ      0x0040 /* bit6: 图块 DMA 传输待刷新 */
#define SCRIPT_VM_FLAG_RELOAD_BG_SET     0x0100 /* bit8: 关窗后重载地图图块/色块 */
#define SCRIPT_VM_FLAG_LZ_STREAMING      0x0200 /* bit9: 异步流式 LZ 解压进行中 (暂停脚本执行) */
#define SCRIPT_VM_FLAG_ENTRY_JUMP_REQ    0x0400 /* bit10: 异步 LZ 解压完成后 PC 跳转入口表 */


/* ========================================================================== */
/* 脚本虚拟机执行函数与控制例程                                               */
/* ========================================================================== */

void ScriptPump_Run();
void ScriptPump_ServiceFrame();
u16 Script_GetFlags();
void Script_ResetVM();
void Script_Abort(u8);

void BgTiles_LoadSet(u16);
void TileDma_Reset();
s16 sub_80527AC(void);
u32 TileDma_GetCtx(u32 *);
u32 Op_LoadTileGfx(u8);

/* Generated module API declarations (was include/code_0.h). */

void sub_804F280();

#define Op_CharaControl sub_804F280

u32 Op_CameraPan(u32 *);

u32 Op_RemovePartyMember(u32 *);

u32 Op_AddPartyMember(u32 *);

u32 Op_ScriptBattle(u32 *);

u32 Op_IfAllFlagsJump(u32 *);

u32 Op_IfAllFlagsClearJump(u32 *);

u32 Op_IfAnyFlagJump(u32 *);

u32 Op_SysEffect(u32 *);

void ScriptPump_Run();

void ScriptPump_ServiceFrame();

void sub_80501B8();

void sub_8050434();

void sub_805063C();

void sub_8050720();

#define Op_DialogMessage sub_8050720

u8 Op_ScriptReturn(u32 *arg0);

u32 Op_ScriptStop(u32 *);

u32 sub_80512C4(u32 *);

#define Op_ScriptStreamLZ sub_80512C4

u32 sub_80513A0(u32 *);

#define Op_ScriptReturnChunk sub_80513A0

void sub_805144C();

#define Op_DialogText sub_805144C

u32 Op_OpenWindow(u32 *);

s16 sub_8051AEC(s16, s16, s16, s16, u8);  /** 同 sub_801768C 插值家族: 第5参必须 u8 (switch 内 cast (s8)); 结果复用 arg1 做累加器 (default 路径 r0=arg1 直达尾部) **/

void sub_8051BE4();

#define Op_DialogChoice sub_8051BE4

u16 Script_GetFlags();

void Script_ResetVM();

void ScriptSet_Load(u8, u8, u8);

void ScriptPump_JumpToEntry(u8, u8);

void Script_Abort(u8);

void BgTiles_LoadSet(u16);

void TileDma_Reset();

s16 sub_80527AC(void); // FlushTileDma: 把待传图块经 DMA3 从 0x0203DE00 刷到 VRAM 0x0600B800 并等完成

#define FlushTileDma sub_80527AC

u32 TileDma_GetCtx(u32 *);

u32 Op_LoadTileGfx(u8);

u32 Op_ScriptJump(u32 *); // ScriptGotoEntry: 脚本指针跳到 gUnk_02016200 + gUnk_02016000[data[1]]

#define ScriptGotoEntry Op_ScriptJump

u32 Script_Call(u32 *);

void Op_Nop();

u32 Op_DialogSetup(u32 *);

u32 Op_CloseWindow(u32 *);

u8 Op_WaitFrames();

u32 Op_BgmPlay(u32 *);

u32 Op_BgmStop(u32 *);

u32 Op_BgmVolume(u32 *);

u32 Op_BgmFadeIn(u32 *);

u32 Op_BgmFadeOut(u32 *);

u32 Op_SfxPlay(u32 *);

u32 Op_SfxStop(u32 *);

u32 Op_RandomJump(u32 *);

u32 Op_ScriptCallAlt(u32 *);

u32 Op_WaitCharsStop(u32 *);

u32 Op_LoadCharaGfx(u32 *);

u32 Op_LoadCharaPal(u32 *);

u32 Op_WaitSpriteLoad(u32 *);

u32 Op_SceneChangeFade(u32 *);

u32 Op_SceneChangePlain(u32 *);

u32 Op_WaitSceneIdle(u32 *);

u32 Op_LoadMap(u32 *);

u32 Op_IfEventFlagJump(u32 *);

u32 Op_SetEventFlag(u32 *);

u32 Op_ClearEventFlag(u32 *);

u32 Op_IfSwitchJump(u32 *);

u32 Op_SetSwitch(u32 *);

u32 Op_ClearSwitch(u32 *);

u32 Op_CameraSnap(u32 *);

s32 Op_CameraFollow(u32 *);

u32 Op_WaitCameraPan(u32 *);

u32 Op_LoadCutsceneAnim(u32 *);

u32 Op_RestartCharaAnim(u32 *);

u32 Op_WaitCharaAnim(u32 *);

u32 Op_IfPartyMemberJump(u32 *);

u32 Op_LoadAnimSet(u32 *);

u32 Op_AnimSlotResume(u32 *);

u32 Op_AnimSlotPause(u32 *);

u32 Op_WaitAnimSlotIdle(u32 *);

u32 Op_MenuLoadAnims(u32 *);

u32 Op_MenuUnlock(u32 *);

u32 Op_MenuLock(u32 *);

u32 Op_WaitMenuReady(u32 *);

u32 Op_FullHealParty(u32 *);

u32 Op_EquipItem(u32 *);

u32 Op_GiveTakeItem(u32 *);

u32 Op_SilverAddSub(u32 *);

u32 Op_IfItemQtyJump(u32 *);

u32 Op_ChestOpen(u32 *);

u32 Op_SaveUiTrigger(u32 *);

u32 Op_IfSaveLoadedJump(u32 *);

u32 Op_SaveTimerA(u32 *);

u32 Op_SaveTimerB(u32 *);

u32 Op_IfSaveFlagJump(u32 *);

u32 Op_SaveOp(u32 *);

u32 Op_SetFlagsList(u32 *);

u32 Op_ClearFlagsList(
    u32 *); // ScriptClearFlags: 把脚本里 data[1]>>1 个 u16 标志号逐个清位(<=0x1FF 走 0x03001C60 位图, 否则 -0x200 走 0x030018F0 位图)

#define ScriptClearFlags Op_ClearFlagsList

u32 Op_ClearSwitchTail(u32 *);

u32 Op_IfMoneyJump(u32 *);

u32 Op_StartLogoFade(u32 *);

u32 Op_WaitLogoFade(u32 *);

u32 Op_SetCharacterLevel(u32 *);

#endif /* GUARD_SCRIPT_VM_H */
