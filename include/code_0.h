#ifndef _CODE_0_H
#define _CODE_0_H

#include "gba/types.h"
#include "iwram.h"

struct ObjFadeSeq {
    u8 pad0[0xB4];
    u16 f_b4;
    u16 f_b6;
};

extern void sub_8000170();

void VBlankIntr(void);

u8 Rand_TableNext();

s32 Sio_LinkTask();
void EventFlags_ClearAll();
u8 EventFlags_Test(u16);
void EventFlags_Set(u16);
void EventFlags_Reset(u16);
void SwitchFlags_ClearAll();
u8 SwitchFlags_Test(u16);
void SwitchFlags_Set(u16);
void SwitchFlags_Reset(u16);
void SwitchFlags_ClearRange();
void Intr_HandleHBlank();
void nullsub_5();
void DummyIntr3();
void DummyIntr4();

void VBlank_UpdateScreenMode5();
void SceneTransition_Load();
void NewGame_Init();
void Scene_EnterMap();
void BattleTransition_Enter();
void Scene_Reload();
void Scene_EnterDoor();
void Scene_RestoreAfterBattle();
void Task_MapExplore();
void Sprites_UpdateFrame();
void Sprite_SetupDialogArrow(u8);
u8 Sprite_EnqueueRender(s16, s16, u8, s16, u8);
void Sprite_UpdateCharaAnim(u8);
void Anim_PlayCustom(u8);
void Anim_BuildOamChain();
u8 CheckEncounter();
void LogoBlendEffect_Update();
void LogoAssets_Load();
void Task_DispatchGameState();
void SceneTransition_RequestMap();
void Task_DialogueFrame();
void Task_BattleMenuFrame();
void Scene_ReloadViaMenu();
void Task_TitleMenuFrame();
void Task_TextFrame();
void Scene_ResetResources();
void Anim_StepChara(u8);
void PalTransfer_Flush();
void OAM_FlushFromQueue();
void Sprites_ReleaseAll();
void Sprites_LoadMapNPCs(u8);
void Chara_InitFromDesc(u8, void *);
void Chara_InitDialogArrow(u8);
void Chara_InitEffect(u8);
void Chara_InitEffectAtPlayer();
void PendingSpriteLoad_Flush();
void Chara_SetWalkPath();
void Chara_ProcessCmdStream(u16);
u16 Chara_StepMove(u16);
u8 CheckFacingEvent();
void Party_FollowAnim();
void Followers_ResetHistory();
void Followers_SyncToTail();
void Party_FollowStep();
void CutsceneAnim_Load(u16 animId, u8 slot, u8 slotSel); /* slotSel≥100 → 额外置 flags bit6 */
void CutsceneAnim_PlayFrame();
void MapGroup_Lookup();
void Chara_SetTilePos(u8, u8, u8, u8);
void Chara_MoveBy(u8, u8, u8, u8);
u16 VramTransfer_AllocSlot();
u8 PalTransfer_AllocSlot();
void PalTransfer_Enqueue(u8, u32, s8, u8);
void VramTransfer_Clear();
void VramTransfer_Flush();

void VramTransfer_Enqueue(u16, void *, void *, u8);
void PalTransfer_Clear();
void SpritePool_Clear();
void Queue34C0_Clear();
void RenderQueue_Clear();
u8 Sprite_AllocNode();
SpriteNode *Sprite_InitChainNode(SpriteNode *, u8, u16, u16, u16);
void LoadSpriteSheetGfx(u8 slot, u16 gfxId); // LZ77 解压缩精灵图块 → OBJ 图块槽 slot
void LoadSpriteSheetPal(u8 slot, u16 palId); // DMA3 装 16 色调色板 → OBJ PLTT 槽 slot
void LoadArrowObjTiles(s8); // 按 bit7 选 2/4 块箭头图块, DMA3 装入 OBJ 图块槽 146
void Chara_SetGfxPal(u8, u8, u8);
void Chara_FreeSprite(u8);
void Chara_SetCmdPtr(u8, u8 *);
void Chara_StartMoving(u8);
u8 Chara_AnyMoving();
void Party_SetFollowMode();
void SetSlotGfxId(u8 slot, u16 gfxId); // 记 gSlotGfxId[slot]=gfxId 并置 PENDING_SPRITE_GFX
void SetSlotPalId(u8 slot, u16 palId); // 记 gSlotPalId[slot]=palId 并置 PENDING_SPRITE_PAL
u8 GetPendingSpriteLoad(); // 返回 gPendingSpriteLoad 位图
void Chara_SetPosDir(u8, s32, s32, u8);
u16 Chara_GetDrawZ(Actor *);
s16 Chara_GetDrawX(Actor *);
void Sprite_FreeChain(struct SpriteNode *);
SpriteNode *Sprite_WriteOam(u16 *, SpriteNode *);
void Chara_StartScriptAnim(u8, u8);
s32 Chara_AnimWaitDone(u8);
void sub_8005020();
/* VBlank sprite/palette transfer plus WIN0H iris-transition table generation. */
#define VBlank_UpdateSpriteAndWindow sub_8005020
/* Apply the active fade level to BLDCNT/BLDALPHA/BLDY for the current scanline. */
void ScreenFade_Apply();
/* Per-frame blend register refresh and fade progress update. */
void ScreenFade_Update();
void sub_80052F8();
void sub_80053B4(u16, u16);
void sub_80055E8(u16 *, u16 *, u8, u8); // 按方向码 1..8 (gWalkDirVectors) 步进相机目标坐标, 含瓦片碰撞+滑动+实体阻挡检查
#define MovePlayer sub_80055E8
u16 *MapTile_At(s16, s16);
u16 MapTile_CollisionBits(u16 *, u16, u16);
void Viewport_UpdateScroll();
void BgMap_FillPattern(u16);
void MapBg_LoadFull(u8);
void MapScene_Load();
void MapScene_LoadNpcSlotIds(u8);
void MapScene_InitSprites(u8);
void MapScene_LoadEventAnimations(u8);
u8 *AnimSlot_Parse(u16, u8 *);
u8 *AnimSlot_ParseLoop(u16, u8 *);
void AnimSlot_Step(s16); // UpdateSpriteAnim: 推进精灵动画槽帧计数, 并把当前帧图块拷进 0x02006000 图块缓存
#define UpdateSpriteAnim AnimSlot_Step
s32 sub_8007ADC(u16, u16); // 算 (x,y) 16x16 足迹覆盖的至多 4 个瓦片坐标, 在 gMapZoneHeader 的 cells 表查区域; 命中写
                           // gMapZoneType/gMapZoneEntryIdx 返回 1
#define MapZone_FindAt sub_8007ADC
s32 sub_8007BD0(
    void); // 按 gMapZoneType 0..4 分发: 0=换图(装载点+state3) 1=图内传送(state4) 2=state8 3=首次进入触发脚本 4=朝向触发脚本
#define MapZone_Trigger sub_8007BD0
void MapBg_LoadInterior(u8);
void Logo_LoadAssets(u8);
u32 ChoiceMenu_BuildList(); // 返回类型非 void 但体内无 return —— 原 ROM 即如此(占住 r0 使寄存器整体上移)
void BattleIntro_Cursor();
void ChoiceMenu_HandleInput(u16);
void DialogPortrait_Set(u8 portraitId, u8 position);
void Viewport_UpdateEffects();
void IntroBg_Load(u8);
void ScreenFade_Start(u16, s16, s16);
void AnimSlot_BankReload();
void Win0H_WaveDmaByVCount();
/* 逐扫描线水波效果族 (源数据 = gWaveSineTable @0x080576D0, 见 data_805769C.h) */
void HBlankWave_BuildTables(u16 mode);
void HBlankWave_ApplyLineScroll(u16 scanline);
u32 LZ_UncompressChunk(void); // @0x08000D5C 分块 LZ 流式解压 gLzContext, 返回 0 = 全部完成
void LZ_InitContext(u8 *dest, struct Unk_LzData *arg1, u32 arg2);
void ScreenFx_SetMode(u16);
void AnimSlots_Release();
void AnimSlots_StepAll();
void BgTiles_LoadUiSet(u8);
void BgScroll_LoadFromTable(u16);
void PlayerSheets_Load();
void AnimSlot_LoadSet(u8, u8); // LoadSpriteAnimSet: 把 gUnk_087EA1A0[setId] 一组精灵动画模型装入 gUnk_030046A0[startSlot..]
#define LoadSpriteAnimSet AnimSlot_LoadSet
void AnimSlot_Pause(u8);
void AnimSlot_Resume(u8);
u8 AnimSlot_Active(u8);
void ReloadSpriteSheet(u8 slot); // 按 slot 重载单个精灵表(图块+调色板)
void ReloadAllSpriteSheets(); // 循环 slot 0..11 全部重载
void ChoiceMenu_ResolveDest(u8);
void DialogPortrait_FlushPending(void);
u16 Camera_GetDrawOffset();
void Script_SetEnvSet(u8);
void BgPal_ResetFirst();
void AnimSlot_PlayOnce(u16, u8 *);
void BgMap_FillRow(u8);
void MapBg_FlushPending();
void ChestObjects_LoadForMap(u8);
void ChestObject_BuildSprite(u8);
void ChestObject_Open(u8);
void LoadDigitFontObjTiles(); // 10 个数字字形 → OBJ 图块槽 150, + 2 组 OBJ 调色板
void ChestFlags_ClearAll();
void ChestFlags_Toggle(u8);
u8 ChestFlags_Test(u8);
void PaletteEffects_Update();
void PaletteTransfer_Update();
void PaletteFx_Apply(u8);
void PaletteFx_Step();
void PaletteFx_Transform();
void MenuEnt_ClearStates();
void MenuEnt_ParseAll(u8);
void MenuEnt_ParseRange(u8, u8);
void MenuEnt_Unlock(u8);
void MenuEnt_Lock(u8);
u8 MenuEnt_GetState(u8);
void Palette_Backup();
void Palette_FillWhite();
u8 *MenuEnt_ParseDesc(u8, u8 *);
void StaticObjGfx_LoadPair(u8);
void StaticObjs_Spawn(u8);
void StaticObjs_StepAll(void);
void StaticObj_BuildChain(u8, u8 *);
void StaticObjs_Reset();
u16 sub_8009F70();
void Stats_BuildSkillList(u8 *, u8, u8);
u8 Chara_GetFormGfx(u8);
void sub_800A1B4(u8);
void sub_800A3C8(u8, u8);
void sub_800A534(u8);
void Stats_RebuildEquipBonuses(u8);
void Stats_RecalcEquip(u8);
u8 ExpToLevel(s32);
u32 LevelToExp(u8);
u8 ItemFindSlot(u8, u8);
void Party_InitStats();
u8 ItemGetValue(u8);
void sub_800A970(void *);
void sub_800A978(void *);
void FullHealParty();
void EquipItem(u8, u8, u8);
void sub_800AA60(u8, u8); // AddInventoryItem: add item to inventory (cap 99)
void sub_800AA84(u8, u8); // RemoveInventoryItem: remove item from inventory (floor 0)
#define AddInventoryItem    sub_800AA60
#define RemoveInventoryItem sub_800AA84
void Silver_Add(s32);
void Silver_Sub(s32);
u8 sub_800AADC(u8);
u16 sub_800AAF8(u8);
u16 sub_800AB18(u8);
u8 Party_AnyEquip();
void Chara_ClearTempStatus(u8);
void Stats_ClearEquipBonus();
void PartyForm_ApplyBonus(u8, u8, u8, u8);
void FullHealCharacter(u8);
void sub_800ACC8();
void SceneBg_Reload();
void MenuState_Reset();
void MenuHp_Update();
void sub_800B374();
void Msg_RenderLine(u8 *, u8);
void PartyUi_InitEntities(u8);
void sub_800BFF8(s16, u16 *, u32);
void BattleIntro_Setup();
void sub_800C194();
void sub_800C2F8();

void MenuUi_SetEntityPos(u8, u8, u8);
void sub_800E244();
void sub_800E668(u8);
void UiSprite_BeginSlide();
void UiSprites_Update();
void sub_800E8F8();
void sub_800EAE4(u16 *, u16, u8);
void MenuUi_SpawnAuxSprites(u8);
void sub_800EC54();
void sub_800F128();
void sub_800F3AC();
void MenuUi_DrawItemList();
void sub_800F670();
void sub_800F70C();
u8 sub_800FA24();
void sub_800FB2C();
void sub_800FDEC();
void sub_800FF10(u8, u8, u8);
s32 sub_8010170(u8, u8);
u8 ItemGetUsePower(u8, u8);
u8 sub_8010300(u8);
u8 WarpTable_Check(void);
void sub_80104F8();
void sub_8010624();
void sub_8010770(u8);
void ScreenIdleIcons_BuildList();

void sub_8010F10(u8, u8, u8, u8);
void sub_801114C();
void sub_8011268();
u8 sub_80113CC(void);
void TitleMenu_ProcessFrame();
void sub_8012530();
void TitleMenu_UpdateUi();
void TitleMenu_DrawOptions();
void OptionsMenu_DrawEntries();
void sub_8013B0C(u16);
void sub_8013C00();
void Save_LoadSlot0();
void Save_LoadContinue();
void SaveTimer_CountUsed();
void SaveTimer_Inc(u8);
void SaveTimer_Dec(u8);
void sub_801417C();
void sub_8014488();
void sub_801455C();
void sub_80146A8();
void sub_8014A68();
void sub_801543C(u8);
void sub_80154E8(u8);
void sub_8015658();
void sub_8015AF0();
void InvUi_DrawCursors();
void InvUi_Main();
void sub_8015E1C();
void Save_ResetReadState();
void Save_StartWrite();
s32 sub_8015ED0(u8);
void SaveUi_DrawSlots();
u32 SaveTimer_Get(u8);
void SaveFlag_Set(u8);
s32 SaveFlag_Get(u8);
void SaveUi_Open(u8);
void sub_8016038(u8);
void sub_8016068();
void sub_80160CC();
void sub_80160F4();
void sub_8016178(u16);
void sub_80161F4();
void Num_Draw16(s16, u16 *);
void Hud_DrawLv(u8, u8, u8);
void Hud_DrawHp(u8, u8, u8);
void Hud_DrawMp(u8, u8, u8);
void Text_PutGlyph(u16 *, u16, u8);
void TextBlocks_Render(u8 *);
u8 Math_DivLoop(s32 *, s32);
void Msg_ShowEndMark(u16 *, u16, u8);
u16 *Msg_DrawPoolSegment(u16 *, u16, u8);
void Msg_BuildSegmentIndex();
void Msg_Show(u16);
void Msg_ShowById(u16, u8);
s32 Text_WriteOrClear(u8 *, u8, u8);
u8 Menu_GetFocus();
void Text_ClearRect(u8, u8, u8, u8);
void MenuUi_SetExclusive(u8, u8);
void MenuUi_HideAll();
void Text_DrawChar(u8, u8, u8, u8);
void sub_80166FC(u8, u8, u8, u8);
void sub_8016758(u8, u8, u8);
void MenuUi_MoveCursor(u8, u8);
u8 Party_SlotOfMember(u8);
void SkillMenu_SaveCursor();
s32 ItemUse_Execute();
void ItemUse_SetCtx();
void SkillMenu_RestoreCursor();
u8 SkillMenu_GetSkill(u8);
u8 Inv_FindFirstHeld();
u8 Inv_FindPrevHeld();
u8 Inv_FindHeldItemOnPage(u8);
void Save_SyncShadow();
void Inv_SeekFirst();
u8 Inv_PrevNonZero();
u8 Inv_NextNonZero(u8);
u8 sub_8016B30(u8, u8);
void SaveUi_OpenLoad();
void Text_WriteChars(u16 *, u8 *, u8);
void Text_FillHidden(u16 *, u8 *);
u16 *Text_TileAt(u8, u8);
void sub_8016C44();
void sub_8016C88();
u32 sub_8016D24();
void Sio_BuildPacket(u8 *);
u8 sub_8016E80(u8 *); /** 收包; 确实返回 gSioState[3], 调用方忽略 (删 return 会少两条指令) */
void sub_8016F30();
void sub_8016FC0();
void Sio_SetReady();
void Sio_Shutdown();
u32 sub_8017120();
s32 sub_80171E4();
u32 Sio_IsHost();
void sub_80175C0();
void Sio_SetXferCtx(u32 *, u32 *, u32, u32);
void Sio_ClearSlot();
void sub_8017640();
s16 sub_801768C(s16, s16, s16, s16, u8);  /** 前4参必须 s16 (定义侧用有符号 float 转换); 第5参必须 u8 不能 s8: s8 会给 code_80264C0.c 的调用点 (sub_80405A4/sub_8042AB4) 多加一条 ldrsb, 各 +4B -> 整 ROM 位移 8B **/

void BattleTask_Run();
void sub_8017FA4(s8);
void sub_8018070();
void sub_80182A8();
void sub_80184A8();
void sub_801869C();
void sub_8018744();
void sub_8018750();
u32 sub_801878C();
void sub_8018798(u8, s16);
u32 sub_80187A8();
u16 sub_80187B4();
void sub_80187C0(u16);
void sub_80187D4(u16);
u16 sub_80187E8();
u16 sub_80187F4();
void ListNode_Init(UnkNode *);
void ListNode_InitKey(UnkNode *, u8);
void ListNode_InsertSorted(UnkNode *, UnkNode *);
void sub_8018838(u32);
u16 Rng_LcgNext();
u32 GetObjPool();
u32 GetCtx_0248();
u32 GetBuf_37410();
void sub_801887C();
void sub_801889C();
void sub_80188BC();
void sub_8018928();
void sub_8018A58(u8);
void sub_8018BF8();
void sub_8018D9C();
u32 sub_8018E34();
void sub_8018EA8();
void sub_8018FC0();
void Bg0_InitClear();
void sub_80191CC();
void DialogCtx_Clear3();
void sub_801933C();
void sub_80196D4();
void DialogCtx_SetPair(u32, u32, u32, u32, u32);
void BattleFx_UpdateTable();
void sub_80199E0();
void sub_8019AD0(u8, u16);
void sub_8019B98();
void BattleUiFlag_Clear();
void BattleUiFlag_Set(u16);
u16 BattleUiFlag_Get();
void BattleUiFlag_Reset(u16);
void Disp_ObjOff();
void Disp_ObjOn();
void sub_8019E60(); // BlankTilemap: 清空 VRAM 图块 0x2C0 并把 1024 项 tilemap 全填成该空白图块
#define BlankTilemap sub_8019E60
void Disp_Bg1Off();
void DialogCtx_SetHead(u8, u8, u8);
void sub_8019F08(u16 *, u16, u8, u8, u8, u8);
void sub_8019F78(u16 *, int, s8, int, u8, u8, u8, u8);
u8 DialogCtx_GetField_C(u8);
void BgMap_PalFillRect(); // K&R: ROM 调用点无截断 (被调入口截断由定义侧提供), 全原型会给 caller 加 lsls/lsrs 破坏已匹配的 sub_802576C
void DialogCtx_Flush();
void FlashFlag_Clear();
u16 FlashFlag_Get();
void FlashFlag_Reset(u16);
void BattleFx_Init(u8, u8, u8, u8);
void BattleFx_Stop();
void BattleFx_DispOff();
void sub_801A270();
void sub_801A2AC(int, int, int);
void sub_801A2EC();
void BgLoad_Reset();
void BgLoad_Finish();
u8 BgLoad_GetPos();
void sub_801A348();
void sub_801A35C();
void BgScrolls_WriteAll();
void sub_801A3A8(u8, u16, u16);
void ObjGfxLoad_Step();
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
    UnkNode node;                          /* +0x00 key/prev/next (key 由 +0x38 值填充) */
    ObjHead headA;                         /* +0x0C 主头 (sub_801B81C(obj+0xC), 801B8AC(obj+0xC)) */
    ObjHead headB;                         /* +0x3C 次头 (sub_801B81C(obj+0x3C), 801B8AC(obj+0x3C)) */
    u16 f_6C;                              /* +0x6C 剩余计数值 ((s16) 比较, 每帧扣 f_B2, 扣空登记 030006F8 池) */
    u16 f_6E;                              /* +0x6E (sub_801D12C/D19C 与 f_6C 判等) */
    u8 pad_70[0x88 - 0x70];                /* +0x70..+0x87 未验证 */
    u8 *animPtr;                           /* +0x88 动画/图形数据块指针 ([+2]/[+8+idx*2]/[+0x1A]/[+0x20] 为 u16 索引入口; 多处 *(u8**) 消费) */
    u8 pad_8C[0xA2 - 0x8C];                /* +0x8C..+0xA1 未验证 */
    u8 f_A2;                               /* +0xA2 子状态 (sub_801D12C 写入) */
    u8 pad_A3[0xAB - 0xA3];                /* +0xA3..+0xAA 未验证 */
    u8 f_AB;                               /* +0xAB tint 分流: ==4 时取 gUnk_03000744 (sub_801ED40/sub_801EE6C) */
    u8 pad_AC[0xB0 - 0xAC];                /* +0xAC..+0xAF 未验证 */
    u16 state;                             /* +0xB0 bits0-3=kind, bits4-7=子态(0x10/0x20/0x60), 0x400=不入链, 0x2000=跳跃 */
    u16 f_B2;                              /* +0xB2 扣减步长 ((s16) 读; 原注释"步长等") */
    u16 f_B4;                              /* +0xB4 (sub_801CE80/event_hub 写入) */
    u16 f_B6;                              /* +0xB6 (sub_801CE80 写入) */
    u8 pad_B8[0xBB - 0xB8];                /* +0xB8..+0xBA 未验证 */
    u8 f_BB;                               /* +0xBB 辅助 */
    u8 f_BC;                               /* +0xBC 辅助 */
    u8 f_BD;                               /* +0xBD (sub_802103C 写入 arg1) */
    u8 slot;                               /* +0xBE 槽号 (≤0xB; 0xFF=空) */
    u8 f_BF;                               /* +0xBF 朝向/参数 */
    u8 f_C0;                               /* +0xC0 朝向/参数 */
    u8 pad_C1;                             /* +0xC1 */
    u8 f_C2;                               /* +0xC2 动画副索引 (animPtr+8+idx*2 选表项; sub_801E690/sub_801E848) */
    u8 f_C3;                               /* +0xC3 (sub_802093C 写入) */
    u8 pad_C4[0xC8 - 0xC4];                /* +0xC4..+0xC7 事件值等 (未逐一验证) */
} BattleObj;

void ObjGfxLoad_Copy(ObjHead *, ObjHead *);
void sub_801A684(ObjHead *);
void sub_801A6F4();
u8 sub_801A884(ObjHead *, u8, u8 *);
void sub_801AD0C(ObjHead *);
u8 sub_801B0B8(ObjHead *, u8);
void sub_801B570(ObjHead *);
void sub_801B688(u8); // 唯一调用点 sub_8018070: r0 = 本帧 u8 结果 (asm 体内还读 [sp,#4], 实参可能不止 1 个)
void sub_801B760(u16);
u8 sub_801B790(u16);
void sub_801B7B8();
void sub_801B81C(ObjHead *, u8, u8, u16, u8, u32, u32, u16, u16, u16);
u8 sub_801B878(ObjHead *, u8, u8 *);
u8 sub_801B8AC(ObjHead *, u8);
u16 *sub_801B8E8(u16 *, u16);
u16 *sub_801B8FC(ObjHead *, u8, u16);
void sub_801B920();
u8 sub_801B954(ObjHead *head);
u16 sub_801B95C(ObjHead *head);
void sub_801B964();
u8 sub_801BE34(void *);
u8 sub_801C484(void *);
void sub_801CA08(BattleObj *, u8, u16, u8, u8);
void sub_801CBA4(BattleObj *, u8, u16, u8, u8);
void sub_801CE80(BattleObj *, u8, u16, u8, u8);
void sub_801CF90();
void sub_801D12C(BattleObj *, u8);
u16 sub_801D19C(BattleObj *, u8);
u8 sub_801D214(u8 *, u8); // 唯一调用点 sub_8018070: (gObjPoolPtr, 本帧结果) -> u8
u8 sub_801D378(u8 *, u8);
void sub_801D468();
void sub_801D568(BattleObj *);
void sub_801D710();
u8 sub_801D984(u8); // OAM 缓冲自绘: 按 0x0300068C 循环把 0x03000670[i] 逐字段写入 gOamBuffer[r6] (r6 递减), 返回递减后的槽号
u32 sub_801DAA0(); // PollSceneTimer: 场景计时状态机 (0x0300068E 0..0x22++), 走完→重置+返回1
void sub_801DB3C(BattleObj *, u8, u16);
void sub_801DC20(BattleObj *, u8);
void sub_801DD04(BattleObj *, u8, u16);
void sub_801DDB0(BattleObj *, u8);
void sub_801DE44(); // ResetSceneObjects: 重置 3 个标志 + 7 项表 + sub_804C2FC(表0), 再对对象列表逐项调 sub_801D710
#define ResetSceneObjects sub_801DE44
void sub_801DEDC(BattleObj *, BattleObj *);
void sub_801DF90(BattleObj *, BattleObj *);
u8 sub_801E040(void);
u8 sub_801E1D8(void);
void sub_801E30C();
u32 sub_801E4D4(BattleObj *, BattleObj *); // ROM 中无调用者(死代码); 返回 7 项标志数组中是否存在回绕项
u32 sub_801E690(BattleObj *, BattleObj *); // ROM 中无调用者(死代码); 同 E4D4, 查表入口改 animPtr+2 / animPtr+8+f_C2*2
u8 sub_801E848();
void sub_801EA70();
u32 sub_801EC3C(BattleObj *, u8); // 返回字节值 (0x20 / (x&0x1F)<<3 / 小常量); 调用方需 (u8) 截断
void sub_801ED40(BattleObj *, u8);
void sub_801EE6C(BattleObj *);
u8 sub_801EEE4();
void sub_801F3FC();
void sub_801F76C();
void sub_801F884();
void sub_801FA10(BattleObj *, u8);
void sub_801FAB8();
void sub_801FEBC(BattleObj *, u16, u8);
s8 sub_801FF40(u8);
void sub_80200E8(u8 *, u8 *, u8);
void sub_8020228(u8 *, u8 *, u8);
void sub_802031C();
void sub_8020648();
u8 sub_8020798();
void sub_80207A4();
u8 sub_80207B4(void *);
void sub_80207DC(BattleObj *obj, u8 bf, u8 c0, u16 f2a, u8 f35);
void sub_8020840(BattleObj *obj, u8 bf, u8 c0, u16 f2a, u8 f35);
void sub_80208A4(BattleObj *);
void sub_8020914(BattleObj *);
void sub_802093C(BattleObj *);
void sub_8020974(ObjHead *, u16, u16, u8, u16); // 入口截断定类: r1/r2/栈参 u16, r3 u8, r0 = 对象头
void sub_80209C8(BattleObj *);
void sub_80209EC(BattleObj *);
void sub_8020A0C(BattleObj *, u8);
u8 sub_8020A7C(BattleObj *);
u8 sub_8020AB0(void);
void sub_8020AE4();
void sub_8020B04();
u32 sub_8020B48();
void sub_8020B54();
void sub_8020B90(BattleObj *);
u8 sub_8020BC0(BattleObj *);
u8 sub_8020BF0(BattleObj *);
u8 sub_8020C2C(void);
void sub_8020C58(BattleObj *, u32);
void sub_8020CC4(void *, u8, u8, u16, u8, u16, u16);
void sub_8020D50(void *, u8);
void sub_8020DA0(void *, u8);
void sub_8020DE4();
void sub_8020DF0(u8 *);
u32 *sub_8020E54();
u8 sub_8020E5C();
u32 sub_8020E68();
void sub_8020E74();
void sub_8020E90(u8 *);
u8 sub_8020EAC(u8 *);
void sub_8020EC8();
void sub_8020EEC(u8);
void sub_8020F08();
void sub_8020F4C();
void sub_8020FB8(void *, u16, u16, u16, u8);
void sub_802103C(u8 *, u8, u16);
void sub_8021064(u8);
void sub_80210C0(void *, u8);
void MenuSlot_ResetAll();
void sub_8021184(u8, u8 *); // 战斗对象槽号/状态同步: arg1+0xBE 槽号→idx, switch((s8)arg0) case 0/3/6/7 更新 gUnk_030007xx 系列
void sub_80212B4();
u8 sub_802151C(void *, void *);
u8 sub_8021700(void);
void sub_8021788(u8 arg0);
void sub_802181C();
u8 sub_802192C(void *, void *, u8 *);
u8 sub_8022458(u8); // 唯一调用点 sub_8018070: 入参 0x7F, 返回 u8
void sub_8022550();
void sub_8022710();
void sub_8022F2C();
void sub_80230BC();
void sub_8023320();
void sub_8023414();
void sub_8023820();
void sub_80244BC();
void sub_8024618();
void sub_80246E8();
void sub_8024820();
void sub_8024940();
void sub_802550C(u8);
void sub_8025518();
void sub_8025638();
void sub_8025650();
void sub_80256E4();
void sub_802576C(u8 *);
void sub_80257D8();
void sub_8025994();
void sub_8025DA8();
void sub_80260BC();
void sub_80264C0();
void sub_802698C();
void sub_8026D08();
void sub_8026F88();
void sub_802723C();
u8 sub_802761C();
void sub_8027760();
void sub_8027A20();
void sub_8027D9C();
void sub_8028098();
void sub_80282EC();
void sub_80285A0();
void sub_80287EC();
void sub_8028AD8();
void sub_8029268();
void sub_8029510();
void sub_8029784();
void sub_80299C8();
void sub_8029BF8();
void sub_802A154();
void sub_802A86C();
void sub_802ADC4();
void sub_802B0F0();
void sub_802B608();
void sub_802B8BC();
void sub_802BB24();
void sub_802BD94();
void sub_802C0EC();
void sub_802C490();
void sub_802C714();
void sub_802C9E8();
void sub_802CE90();
void sub_802D1FC();
void sub_802D454();
void sub_802D728();
void sub_802DA78();
void sub_802DE04();
u32 sub_802DFDC();
void sub_802E234();
void sub_802E49C();
void sub_802E6C8();
void sub_802EAC4();
void sub_802EDD8();
void sub_802F100();
u32 sub_802F480();
void sub_802F6D8();
void sub_802F9EC();
void sub_802FE98();
void sub_803029C();
void sub_8030664();
u32 sub_80309B0();
u8 sub_8030C08(u8 *);
u8 sub_8030D9C(u8 *);
u8 sub_8030F30(u8 *);
u8 sub_80310C4(u8 *);
u8 sub_8031258(u8 *);
u8 sub_80313EC(u8 *);
u8 sub_8031580(u8 *);
u8 sub_8031714(u8 *);
u8 sub_80318A8(u8 *);
u8 sub_8031A3C(u8 *);
u8 sub_8031BD0(u8 *);
u8 sub_8031D64(u8 *);
u8 sub_8031EF8(u8 *);
u8 sub_803208C(u8 *);
u8 sub_8032220(u8 *);
u8 sub_80323B4(u8 *);
u32 sub_8032548();
u32 sub_803272C();
void sub_8032948();
u8 sub_8032D74();
void sub_8032EA0();
void sub_80334B8();
void sub_8033988();
void sub_8033E2C();
u32 sub_8034440();
u32 sub_80345AC();
u32 sub_8034718();
u32 sub_80348A8();
u32 sub_8034BFC();
u32 sub_8034D94();
u32 sub_8034F00();
u32 sub_8035130();
void sub_8035360();
u32 sub_803586C();
u32 sub_8035B04();
u32 sub_8035D9C();
u32 sub_8036034();
u32 sub_80362CC();
void sub_8036564();
u32 sub_80368FC();
void sub_8036B30();
void sub_8036EA4();
void sub_8037078();
void sub_8037388();
void sub_8037868();
void sub_8037C40();
u32 sub_8037E14();
u32 sub_8037FE8();
u32 sub_80381BC();
u32 sub_8038390();
u32 sub_8038568();
u32 sub_803874C();
void sub_8038920();
u32 sub_8038C84();
void sub_8038E44();
void sub_8039024();
u8 sub_80392C0();
void sub_80393E0();
void sub_8039724();
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
u8 sub_803E58C();
u8 sub_803ED34();
void sub_803F21C();
u8 sub_803F328(u8 arg0);
void sub_803F444();
void sub_803F5B4();
void sub_803F658();
u8 sub_803FF54(u8 *);
u8 sub_80401AC(void);
u8 sub_80405A4(u8 *);
void sub_8040690();
void sub_8040EE8();
void sub_8041308();
void sub_80416F0();
void sub_80419E0();
void sub_8041EDC();
u8 sub_8042200(u8 *);
void sub_80422B8();
void sub_8042784();
u8 sub_8042AB4(u8 *);
void sub_8042B90();
void sub_8042E70();
void sub_8043554();
void sub_8043938();
void sub_8043B5C();
void sub_8043F90();
void sub_8044394(u8 *);
void sub_8044414();
u16 sub_8044420();
void sub_804442C(u8);
void sub_804448C();
u8 sub_8044498();
void sub_80444A4();
u8 sub_80444E8(void);
void sub_8044514(s16);
void sub_8044574(s16, u16, u8);
u8 *sub_80445E0();
void sub_80445E8(u8 *, u8);
u8 sub_8044680(u8 *);
u8 sub_80446A4(u8 *);
void sub_80446BC(u8 *);
s32 sub_8044728();
s32 sub_804472C();
s32 sub_8044730();
s32 sub_8044734();
s32 sub_8044738();
u32 sub_804473C();
u32 sub_80448A8();
u32 sub_8044A40();
u32 sub_8044F4C();
u32 sub_8045098();
void sub_804519C();
u8 sub_8045328(); // 2026-09-11 zcode-engine: sub_8046480 调用点反汇编证据 (lsls/lsrs/cmp #1), 无已匹配调用者
u16 sub_80453D8(void);
u16 sub_804542C(void);
u8 sub_80454A4(u16);
u16 sub_80455A0(u8, u8);
void sub_8045688(u8, u8, u8);
void sub_80457AC();
s8 sub_8045860(u8, u8 *);
void sub_8045940();
u8 sub_8045A10(u8 *, u8);
u8 sub_8045A74(u8 *, u8 *, u8, u8, u8);
void sub_8045B90(u8 *, u8);
void sub_8045BF4();
void sub_8045D00();
void sub_8045EB8(u8 *);
u8 sub_8045F10(u8 *, u16);
void sub_8045F94(u8 *, u16);
void sub_8046060(u8 *, u16);
void sub_804612C(u8 *, u16, u16);
void sub_804621C();
u32 sub_80462E4();
u32 sub_8046480(u8 *, u8 *, u8);
void sub_8046558(); // 2026-09-11 zcode-engine 回退: 函数未匹配, 原型保持原样
void sub_804666C();
void sub_80466F0();
void sub_8046C50();
void sub_8046CD4();
u8 sub_8046E18(u8 *, s32, s32); // 2026-09-11 zcode-engine: 宽参+窄局部 (经验71), 匹配调用方传参无截断证据
u16 sub_8046F0C(); // 2026-09-11 zcode-engine: 调用点返回值按 u16 用 (lsls/lsrs #0x10), 无已匹配调用者
u16 sub_8047024();
u8 sub_80471AC();
u32 sub_80472E8();
void sub_804753C();
u8 sub_80476DC();
u8 sub_8047B1C();
u8 sub_8047D28(u8 *, u8);
u8 sub_8047DC8();
s32 sub_8047FCC(u16);
void sub_80480EC();
void sub_80481B8();
void sub_8048310();
void sub_8048458();
void sub_80485A4();
void sub_8048690();
u8 sub_8048764(u8 *);
u8 sub_804877C(u8);
u8 sub_80487A4(u8);
u8 sub_80487CC(u8);
u16 sub_8048818(u8, u8);
u8 sub_8048868(u8, u8);
u8 sub_80488CC(u8 *, u8);
u8 sub_8048934(u8 *, u8);
u8 sub_8048984(u8 *, u8);
u8 sub_80489A4(u8 *, u8);
u16 sub_80489C8(u8 *, u16);
u8 sub_80489E8(u8 *, u8 *, u8, u16);
u8 sub_8048A68(u8 *);
void sub_8048A88(u8 *, s8, s8);
void sub_8048ACC(u8 *, u8, u8);
void sub_8048B30(u8, u8, u16);
void sub_8048B5C(u8 *, u8);
u8 sub_8048B88(u8 *);
u8 sub_8048BAC(u8 *);
void sub_8048BD0(u8 *);
u8 sub_8048C30(u8 *);
u8 sub_8048C80(u8 *);
u8 sub_8048CEC(u8 *);
void sub_8048D40(u8 *);
u16 sub_8048D64(u8 *, u16);
u8 sub_8048D84(u8 *, u8 *);
void sub_8048DA4();
void sub_8048F0C();
u8 sub_8048FB8(void);
void sub_80492C0();
void sub_80494F0();
u32 sub_80497B0(u16 *arg0, u16 arg1);
u32 sub_80498E0();
void sub_8049958();
void sub_8049AD8();
void sub_8049B70();
u8 sub_8049C1C(u8 *); // 2026-09-11 zcode-engine: void*→u8* (定义侧 arg0[0] 字节读写), 无已匹配调用者
u8 sub_8049D58(u8); // 唯一调用点 sub_8018070: 入参/返回均 u8
u8 sub_8049DF8(void *, void *);
void sub_804A148();
u8 sub_804A368(void *);
void sub_804AA2C();
void sub_804AB10(void);
void sub_804AB40();
void sub_804ABD0(void);
u32 sub_804ABF8(u16 *dest, u8 arg1);
void sub_804AC60(void);
u16 *sub_804ACC0(u8);
void sub_804AD24();
void sub_804AD54();
void sub_804AD60(void);
void sub_804ADE0();
void sub_804ADF8();
void sub_804AE2C();
void sub_804AF60();
void sub_804B080();
void sub_804B1EC();
void sub_804B1F8(u32);
void sub_804B224(u16 *);
void sub_804B288();
void sub_804B3C0();
typedef struct
{
    s8 field_0;
    s8 field_1;
    u8 field_2;
    u8 field_3;
    u8 field_4;
    u8 field_5;
    u16 field_6;
    u8 field_8;
} Unk_804B458;
void sub_804B458(Unk_804B458 *, u8, u16 *, u16 *);
void sub_804B4D0();
void sub_804B56C();
s32 sub_804B654();
void sub_804B7B0();
void sub_804B834();
void sub_804B8E8(u8, u8);
void sub_804B96C();
void sub_804BB64(u8, u8);
u8 sub_804BBDC(u8, u32, u32, u32, u32, u32, u32, u32);
void sub_804BD54(u8, u8);
void sub_804BDD8();
void sub_804BE90(u8, u8);
void sub_804BF14();
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
void sub_804C890();
u8 sub_804C8E0(u8 *, u8); // obj池槽位移除元素+随机取回 (返回 u8)
void sub_804C9B4();
void sub_804CA2C(u8 *);
void sub_804CAA0(u8 *);
void sub_804CB18(u8 *);
void sub_804CB8C(u8 *);
void sub_804CC00(u8 *);
void sub_804CC78(u8 *);
void sub_804CCEC(u8 *);
void sub_804CD60(u8 *);
void sub_804CDD4(u8 *);
void sub_804CE48(u8 *);
void sub_804CEBC();
void sub_804CEE0();
void sub_804D0F8(u8 *); // obj槽位填充: 守卫+移除匹配obj[0xAC]+随机取回
void sub_804D1B4(u8 *, u8 *); // obj槽位概率填充: 守卫+表驱动随机
void sub_804D260(u8 *, u8 *); // obj槽位概率填充 (x10, sub_804D1B4 孪生)
void sub_804D310();
void sub_804D3A0(u8 *, u8 *); // obj槽位概率填充 (x13, 同族孪生)
void sub_804D44C(u8 *, u8 *); // obj槽位概率填充 (x10, 同族孪生)
void sub_804D4FC();
void sub_804D5B4();
void sub_804D708();
void sub_804D798();
void sub_804D840();
void sub_804D8F4();
void sub_804DA04();
void sub_804DABC();
void sub_804DB64();
void sub_804DC24();
void sub_804DCD8();
void sub_804DD70(u8 *, u32);
u8 sub_804DD90(u8, u8); /** 勿改宽原型/K&R: u8原型+Sub6C结构形态才是 sub_8045EB8 的解 */
void sub_804DE20();
void sub_804DE8C();
u8 sub_804DF14(Unk_03000DEntry *);
void sub_804DF74(Unk_03000DEntry *, u8 *, u8);
void sub_804DFD8(u16 *, u8, u8, u8 *, u8, u8, u8);
u8 sub_804E0E4(u8 *, u32);
u8 sub_804E2AC(u8 *, u32);
s8 sub_804E6DC(u8 *, u8);
s8 sub_804E76C(u8 *, u8, u8);
void sub_804E7EC();
u8 sub_804E85C(void);
void sub_804E9DC();
void sub_804EC04();
void sub_804EEC4(void);
void sub_804EF00(u8);
void sub_804EF50(void);
u8 sub_804EF90(u8);
void sub_804EFDC(u8 *, u8, u8, u8 *, u8);
u8 sub_804F050(u8);
void sub_804F07C();
u8 sub_804F088(u8 *, u32);
u8 sub_804F0B8(u8 *, s32); // CheckObjectKindSlot: 比较对象 +0x91/+0x92 两个候选 id, 返 1/2/0
#define CheckObjectKindSlot sub_804F0B8
s8 sub_804F10C(u8, u8);
u8 sub_804F17C(u8 *, u8, u8);
void SioBattle_ResetState();
u8 SioBattle_GetState();
void SioBattle_ClearSlots();
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
void System_ResetToLogo(void);
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

#endif
