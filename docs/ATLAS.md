# 函数图谱与模块边界提案

> 自动生成: `python3 scripts/atlas.py`。产物: 本文件 + `modules.draft.yaml` + `.scratch/atlas.json`
>
> **硬约束**: 模块 = 按原始顺序的连续函数区间, 不重排任何函数 (linker.ld 的 .text 顺序决定)

- 函数 **1059**, 已匹配 **910** (85.9%), 未匹配 **149** / **60,215** 条指令
- 亲和边 18222 条 (m=17550), 调度表 **6** 张, λ=0.3 → **31 个模块**

## 0. ROM 自己的模块骨架: 函数指针调度表

全 ROM 264 张指针数组里, 254 张是编译器生成的 switch 跳转表(目标都在同一函数内),
只有下面这些是**真正的跨函数调度表** —— 一张表 = 一个处理器家族 = 一个模块的铁证:

| 表地址 | 项数 | 不同函数 | 目标范围 | 判读 |
|---|---|---|---|---|
| 0x0839CD5C | 89 | 87 | 0x080257D8–0x0804DCD8 | **引擎对象/角色状态处理器表** |
| 0x0862D434 | 80 | 80 | 0x0804F280–0x080533D4 | **脚本 opcode 处理器表** |
| 0x0839D4CC | 60 | 59 | 0x08032548–0x08044738 | 引擎子处理器表 |
| 0x087E83F0 | 16 | 16 | 0x08001354–0x080177AC | 中断/初始化向量表 |
| 0x0839CEC4 | 14 | 14 | 0x08040690–0x08043F90 | 引擎子处理器表 |
| 0x0805769C | 13 | 5 | 0x0800065C–0x08016FC0 | 中断/初始化向量表 |

## 1. 方法验证 —— 现有 5 个 .o 的边界, detector 是否独立复现

| 现有边界 | 地址 | 最近的检测切点 | 相差函数数 |
|---|---|---|---|
| code_80002A0 | code_8005020 | 0x08005020 | 0x08004EB8 | **6** |
| code_8005020 | save | 0x080109F8 | 0x08010978 | **1** |
| save | code_8010F10 | 0x08010F10 | 0x08010978 | **6** |
| code_8010F10 | code_801A3C4 | 0x0801A3C4 | 0x0801A3A8 | **1** |
| code_801A3C4 | code_8020D50 | 0x08020D50 | 0x080210C0 | **17** |
| code_8020D50 | code_80264C0 | 0x080264C0 | 0x0802576C | **5** |
| code_80264C0 | code_8044394 | 0x08044394 | 0x080446A4 | **13** |
| code_8044394 | code_804F0B8 | 0x0804F0B8 | 0x0804F250 | **5** |

## 1b. λ 扫描 (每段罚项 → 模块数; 找肘部)

| λ | 1.2 | 0.9 | 0.7 | 0.55 | 0.45 | 0.38 | 0.3 | 0.24 | 0.18 | 0.13 | 0.09 | 0.06 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 模块数 | 16 | 21 | 24 | 25 | 28 | 31 | 31 | 34 | 40 | 42 | 52 | 53 |
| 目标值 | 30.49 | 35.96 | 40.55 | 44.3 | 46.97 | 49.07 | 51.55 | 53.47 | 55.63 | 57.69 | 59.58 | 61.14 |

## 2. 模块清单 (按未匹配指令数 = 工作量排序)

| 模块 | 分区 | 地址区间 | 函数 | 未匹配 | 未匹配指令 | 调度表 | 语义锚点 | 关键工作内存 |
|---|---|---|---|---|---|---|---|---|
| **M16** | ENGINE/OBJECT | 0x08032548–0x0803F658 | 61 | 23 | 11,817 | 0x0839D4CC(59) | — | 0x3000820, 0x3000822 |
| **M07** | ENGINE/SYS | 0x0800ACA4–0x08010770 | 37 | 14 | 6,990 | — | BattleIntro_Setup, FullHealCharacter, ItemGetUsePower | 0x3000188, 0x3004aa0 |
| **M13** | ENGINE/CORE | 0x080210C0–0x080256E4 | 26 | 14 | 6,411 | — | MenuSlot_ResetAll, SetHead | 0x300076c, 0x3000781 |
| **M17** | ENGINE/OBJECT | 0x0803FF54–0x08044680 | 32 | 14 | 5,631 | 0x0839CEC4(14) | — | 0x3000820, 0x3000825 |
| **M08** | ENGINE/SYS | 0x08010978–0x08016C2C | 85 | 9 | 4,864 | — | Hud_DrawHp, Hud_DrawLv, Hud_DrawMp | 0x2005800, 0x3000187 |
| **M12** | ENGINE/CORE | 0x0801B920–0x08021064 | 89 | 11 | 4,021 | — | ResetSceneObjects, sub_8020B54 | 0x300068d, 0x30006f8 |
| **M03** | BOOT/INTR | 0x08004EB8–0x08008DF8 | 55 | 9 | 3,908 | — | AnimSlot_Active, AnimSlot_BankReload, AnimSlot_LoadSet | 0x300467c, 0x40000d4 |
| **M28** | SCRIPT | 0x0804F250–0x0805305C | 77 | 7 | 3,535 | 0x0862D434(80) | BattleDrops_Clear, BgTiles_LoadSet, FlushTileDma | 0x2016200, 0x3000e70 |
| **M19** | ENGINE/CORE | 0x080448A8–0x08048B30 | 65 | 18 | 3,430 | — | — | 0x3004ac0, 0x3004a88 |
| **M14** | ENGINE/OBJECT | 0x0802576C–0x080313EC | 62 | 4 | 1,888 | 0x0839CD5C(87) | — | 0x3000825, 0x3000820 |
| **M21** | ENGINE/CORE | 0x08048D64–0x0804AD24 | 25 | 7 | 1,782 | — | — | 0x300094d, 0x3000949 |
| **M10** | ENGINE/CORE | 0x0801768C–0x0801A36C | 72 | 5 | 1,581 | — | BattleFx_DispOff, BattleFx_Init, BattleFx_Stop | 0x3000324, 0x3000384 |
| **M11** | ENGINE/CORE | 0x0801A3A8–0x0801B8FC | 18 | 3 | 1,488 | — | ObjGfxLoad_Copy, ObjGfxLoad_Step, sub_801B954 | 0x40000d4, 0x3000518 |
| **M01** | BOOT/INTR | 0x080012B8–0x08004D38 | 66 | 3 | 822 | 0x087E83F0(16) | Anim_BuildOamChain, Anim_PlayCustom, Anim_StepChara | 0x3002e80, 0x3003ac0 |
| **M09** | ENGINE/SYS | 0x08016C44–0x08017640 | 16 | 2 | 540 | — | Sio_BuildPacket, Sio_ClearSlot, Sio_IsHost | 0x3004df0, 0x4000208 |
| **M05** | ENGINE/SYS | 0x080091A4–0x08009E80 | 19 | 1 | 477 | — | ChestFlags_Test, MenuEnt_ClearStates, MenuEnt_GetState | 0x3000010, 0x3004910 |
| **M27** | ENGINE/CORE | 0x0804DD70–0x0804F244 | 28 | 1 | 374 | — | BattleDrops_Roll, BattleFxObjs_Add, BattleFx_GetObjCount | 0x3000d88, 0x3000ddc |
| **M23** | ENGINE/CORE | 0x0804B224–0x0804C6B0 | 41 | 2 | 292 | — | — | 0x3000ae8, 0x3000be8 |
| **M24** | ENGINE/CORE | 0x0804C728–0x0804D0F8 | 18 | 1 | 249 | — | — | 0x3000d38, 0x3000be8 |
| **M22** | ENGINE/CORE | 0x0804AD54–0x0804B1F8 | 9 | 1 | 115 | — | — | 0x3000ade, 0x3000948 |
| **M00** | BOOT/INTR | 0x080002A0–0x0800128C | 34 | 0 | 0 | 0x0805769C(5) | AgbMain, Display_RestartAfterLoad, Display_ShutdownSequence | 0x40000d4, 0x300259c |
| **M02** | BOOT/INTR | 0x08004D8C–0x08004E88 | 8 | 0 | 0 | — | Chara_AnyMoving, Chara_SetCmdPtr, Chara_SetPosDir | 0x3002e80, 0x30032d0 |
| **M04** | ENGINE/SYS | 0x08008E44–0x08009184 | 8 | 0 | 0 | — | BgMap_FillRow, ChestFlags_ClearAll, ChestFlags_Toggle | 0x3004870, 0x3004890 |
| **M06** | ENGINE/SYS | 0x08009F48–0x0800AC08 | 29 | 0 | 0 | — | Chara_ClearTempStatus, Chara_GetFormGfx, EquipItem | 0x3004ac0, 0x3004aa8 |
| **M15** | ENGINE/OBJECT | 0x08031580–0x080323B4 | 10 | 0 | 0 | 0x0839CD5C(87) | — | 0x3000868, 0x3000889 |
| **M18** | ENGINE/OBJECT | 0x080446A4–0x0804473C | 8 | 0 | 0 | 0x0839D4CC(59) | — | 0x3000826, 0x3000884 |
| **M20** | ENGINE/CORE | 0x08048B5C–0x08048D40 | 8 | 0 | 0 | — | — | — |
| **M25** | ENGINE/OBJECT | 0x0804D1B4–0x0804D708 | 8 | 0 | 0 | 0x0839CD5C(87) | — | — |
| **M26** | ENGINE/OBJECT | 0x0804D798–0x0804DCD8 | 8 | 0 | 0 | 0x0839CD5C(87) | — | — |
| **M29** | SCRIPT | 0x08053078–0x080533B4 | 19 | 0 | 0 | 0x0862D434(80) | Op_ChestOpen, Op_ClearFlagsList, Op_ClearSwitchTail | 0x2016000, 0x2016200 |
| **M30** | SCRIPT | 0x080533D4–0x08053884 | 18 | 0 | 0 | — | Bgm_Continue, Bgm_FadeIn, Bgm_FadeOut | 0x3000f38, 0x3000f42 |

## 3. 参数化同构家族 (一个任务包, 做一个中一串)

| 大小 | 未匹配 | 指令/函数 | 成员 | 所属模块 |
|---|---|---|---|---|

## 4. 每个模块的函数明细

### M00 [BOOT/INTR]  0x080002A0–0x0800128C  (34 函数 / 0 未匹配 / 0 指令)

调度表: 0x0805769C(5)
锚点: AgbMain, Display_RestartAfterLoad, Display_ShutdownSequence, DummyIntr3, DummyIntr4, DummyIntr5, EventFlags_ClearAll, EventFlags_Reset, EventFlags_Set, EventFlags_Test
工作内存: 0x40000d4, 0x300259c, 0x4000004, 0x30018f0
ROM 表: 0x80576d0, 0x8057750, 0x805769c, 0x87e83f0

```
  地址        指令  ll.cfg 名                     函数
 0x080002a0   174  VBlank_UpdateGameScreen      VBlank_UpdateGameScreen
 0x080004f8    58  VBlank_UpdateScreenSimple    VBlank_UpdateScreenSimple
 0x080005a8    64  HBlankWave_ApplyLineScroll   HBlankWave_ApplyLineScroll
 0x0800065c   194  VBlankIntr                   VBlankIntr
 0x080008cc    64  Display_ShutdownSequence     Display_ShutdownSequence
 0x0800096c   152  Sio_LinkTask                 Sio_LinkTask
 0x08000b58   108  System_SoftReset             System_SoftReset
 0x08000c98    89  HBlankWave_BuildTables       HBlankWave_BuildTables
 0x08000d5c    91  LZ_UncompressChunk           LZ_UncompressChunk
 0x08000e1c    53  Intr_SetMode                 Intr_SetMode
 0x08000ed8    51  Display_RestartAfterLoad     Display_RestartAfterLoad
 0x08000f54    18  System_ResetToLogo           System_ResetToLogo
 0x08000f90     1  nullsub_5                    nullsub_5
 0x08000f94     5  VBlankWait_PumpSound         VBlankWait_PumpSound
 0x08000fa4    17  VBlankWaitExit_PumpSound     VBlankWaitExit_PumpSound
 0x08000fd0    18  LZ_InitContext               LZ_InitContext
 0x08000ff8    10  Rand_TableNext               Rand_TableNext
 0x08001014    11  EventFlags_ClearAll          EventFlags_ClearAll
 0x08001030    13  EventFlags_Test              EventFlags_Test
 0x08001050    14  EventFlags_Set               EventFlags_Set
 0x08001070    14  EventFlags_Reset             EventFlags_Reset
 0x08001090    11  SwitchFlags_ClearAll         SwitchFlags_ClearAll
 0x080010ac    13  SwitchFlags_Test             SwitchFlags_Test
 0x080010cc    14  SwitchFlags_Set              SwitchFlags_Set
 0x080010ec    14  SwitchFlags_Reset            SwitchFlags_Reset
 0x0800110c    11  SwitchFlags_ClearRange       SwitchFlags_ClearRange
 0x08001128    59  System_Init                  System_Init
 0x080011f0    16  ReadKeysRaw                  ReadKeysRaw
 0x0800121c    16  ReadKeys                     ReadKeys
 0x08001248     1  DummyIntr3                   DummyIntr3
 0x0800124c    21  Intr_HandleHBlank            Intr_HandleHBlank
 0x08001284     1  DummyIntr4                   DummyIntr4
 0x08001288     1  DummyIntr5                   DummyIntr5
 0x0800128c    14  AgbMain                      AgbMain
```

### M01 [BOOT/INTR]  0x080012B8–0x08004D38  (66 函数 / 3 未匹配 / 822 指令)

调度表: 0x087E83F0(16)
锚点: Anim_BuildOamChain, Anim_PlayCustom, Anim_StepChara, BattleTransition_Enter, Chara_FreeSprite, Chara_InitDialogArrow, Chara_InitEffect, Chara_InitEffectAtPlayer, Chara_InitFromDesc, Chara_MoveBy
工作内存: 0x3002e80, 0x3003ac0, 0x3001944, 0x3002600
ROM 表: 0x8058834, 0x87e8430, 0x80b9dfc, 0x805881c

```
  地址        指令  ll.cfg 名                     函数
 0x080012b8    50  VBlank_UpdateScreenMode5     VBlank_UpdateScreenMode5
 0x08001354   157  SceneTransition_Load         SceneTransition_Load
 0x08001538   163  NewGame_Init                 NewGame_Init
 0x08001708    98  Scene_EnterMap               Scene_EnterMap
 0x08001828    58  BattleTransition_Enter       BattleTransition_Enter
 0x080018d4   146  Scene_Reload                 Scene_Reload
 0x08001a7c   111  Scene_EnterDoor              Scene_EnterDoor
 0x08001bd0    94  Scene_RestoreAfterBattle     Scene_RestoreAfterBattle
 0x08001d08   384  Task_MapExplore              Task_MapExplore
 0x08002154   237  Sprites_UpdateFrame          Sprites_UpdateFrame
 0x08002380    80  Sprite_SetupDialogArrow      Sprite_SetupDialogArrow
 0x0800243c   323  Sprite_EnqueueRender         Sprite_EnqueueRender
 0x0800271c   313  Sprite_UpdateCharaAnim       Sprite_UpdateCharaAnim
 0x080029d8   165  Anim_PlayCustom              Anim_PlayCustom
 0x08002b54   220  Anim_BuildOamChain           Anim_BuildOamChain
 0x08002d54    53  CheckEncounter               CheckEncounter
 0x08002ddc   142  LogoBlendEffect_Update       LogoBlendEffect_Update
 0x08002f6c    67  LogoAssets_Load              LogoAssets_Load
 0x08003088    12  Task_DispatchGameState       Task_DispatchGameState
 0x080030b0    34  SceneTransition_RequestMap   SceneTransition_RequestMap
 0x08003114     6  Task_DialogueFrame           Task_DialogueFrame
 0x08003128    20  Task_BattleMenuFrame         Task_BattleMenuFrame
 0x08003168    41  Scene_ReloadViaMenu          Scene_ReloadViaMenu
 0x080031e4     6  Task_TitleMenuFrame          Task_TitleMenuFrame
 0x080031f8     5  Task_TextFrame               Task_TextFrame
 0x08003208    27  Scene_ResetResources         Scene_ResetResources
 0x08003254     6  Anim_StepChara               Anim_StepChara
 0x08003264    39  PalTransfer_Flush            PalTransfer_Flush
 0x080032bc    64  OAM_FlushFromQueue           OAM_FlushFromQueue
 0x08003348    76  Sprites_ReleaseAll           Sprites_ReleaseAll
 0x080033e8    49  Sprites_LoadMapNPCs          Sprites_LoadMapNPCs
 0x0800345c   360  Chara_InitFromDesc           Chara_InitFromDesc
 0x0800375c    57  Chara_InitDialogArrow        Chara_InitDialogArrow
 0x080037dc    51  Chara_InitEffect             Chara_InitEffect
 0x0800384c    54  Chara_InitEffectAtPlayer     Chara_InitEffectAtPlayer
 0x080038cc    47  PendingSpriteLoad_Flush      PendingSpriteLoad_Flush
*0x08003958   197  Chara_SetWalkPath            Chara_SetWalkPath
 0x08003b08   143  Chara_ProcessCmdStream       Chara_ProcessCmdStream
*0x08003c54   339  Chara_StepMove               Chara_StepMove
 0x08003f40   183  CheckFacingEvent             CheckFacingEvent
*0x080040e4   286  Party_FollowAnim             Party_FollowAnim
 0x08004358    51  Followers_ResetHistory       Followers_ResetHistory
 0x080043d4    57  Followers_SyncToTail         Followers_SyncToTail
 0x0800445c   267  Party_FollowStep             Party_FollowStep
 0x080046dc    64  CutsceneAnim_Load            CutsceneAnim_Load
 0x0800478c   156  CutsceneAnim_PlayFrame       CutsceneAnim_PlayFrame
 0x08004980    28  MapGroup_Lookup              MapGroup_Lookup
 0x080049c8    26  Chara_SetTilePos             Chara_SetTilePos
 0x08004a00    32  Chara_MoveBy                 Chara_MoveBy
 0x08004a44    15  VramTransfer_AllocSlot       VramTransfer_AllocSlot
 0x08004a6c    16  PalTransfer_AllocSlot        PalTransfer_AllocSlot
 0x08004a94    19  PalTransfer_Enqueue          PalTransfer_Enqueue
 0x08004ac0    11  VramTransfer_Clear           VramTransfer_Clear
 0x08004adc    34  VramTransfer_Flush           VramTransfer_Flush
 0x08004b2c    22  VramTransfer_Enqueue         VramTransfer_Enqueue
 0x08004b60    19  PalTransfer_Clear            PalTransfer_Clear
 0x08004b8c    21  SpritePool_Clear             SpritePool_Clear
 0x08004bbc    15  Queue34C0_Clear              Queue34C0_Clear
 0x08004be0    12  RenderQueue_Clear            RenderQueue_Clear
 0x08004bfc    19  Sprite_AllocNode             Sprite_AllocNode
 0x08004c28    48  Sprite_InitChainNode         Sprite_InitChainNode
 0x08004c8c    17  LoadSpriteSheetGfx           LoadSpriteSheetGfx
 0x08004cb8    15  LoadSpriteSheetPal           LoadSpriteSheetPal
 0x08004ce8    19  LoadArrowObjTiles            LoadArrowObjTiles
 0x08004d20    10  Chara_SetGfxPal              Chara_SetGfxPal
 0x08004d38    37  Chara_FreeSprite             Chara_FreeSprite
```

### M02 [BOOT/INTR]  0x08004D8C–0x08004E88  (8 函数 / 0 未匹配 / 0 指令)

锚点: Chara_AnyMoving, Chara_SetCmdPtr, Chara_SetPosDir, Chara_StartMoving, GetPendingSpriteLoad, Party_SetFollowMode, SetSlotGfxId, SetSlotPalId
工作内存: 0x3002e80, 0x30032d0, 0x3002c44, 0x3004670

```
  地址        指令  ll.cfg 名                     函数
 0x08004d8c     9  Chara_SetCmdPtr              Chara_SetCmdPtr
 0x08004da4    19  Chara_StartMoving            Chara_StartMoving
 0x08004dd0    24  Chara_AnyMoving              Chara_AnyMoving
 0x08004e04     6  Party_SetFollowMode          Party_SetFollowMode
 0x08004e14    17  SetSlotGfxId                 SetSlotGfxId
 0x08004e48    17  SetSlotPalId                 SetSlotPalId
 0x08004e7c     3  GetPendingSpriteLoad         GetPendingSpriteLoad
 0x08004e88    21  Chara_SetPosDir              Chara_SetPosDir
```

### M03 [BOOT/INTR]  0x08004EB8–0x08008DF8  (55 函数 / 9 未匹配 / 3,908 指令)

锚点: AnimSlot_Active, AnimSlot_BankReload, AnimSlot_LoadSet, AnimSlot_Parse, AnimSlot_ParseLoop, AnimSlot_Pause, AnimSlot_PlayOnce, AnimSlot_Resume, AnimSlot_Step, AnimSlots_Release
工作内存: 0x300467c, 0x40000d4, 0x30046a0, 0x3004550
ROM 表: 0x8088d80, 0x8087216, 0x87e9aa0, 0x87ea020

```
  地址        指令  ll.cfg 名                     函数
 0x08004eb8    17  Chara_GetDrawZ               Chara_GetDrawZ
 0x08004edc    36  Chara_GetDrawX               Chara_GetDrawX
 0x08004f3c    20  Sprite_FreeChain             Sprite_FreeChain
 0x08004f64    31  Sprite_WriteOam              Sprite_WriteOam
 0x08004fa8    18  Chara_StartScriptAnim        Chara_StartScriptAnim
 0x08004fd0    32  Chara_AnimWaitDone           Chara_AnimWaitDone
*0x08005020   174  sub_8005020                  sub_8005020
 0x080051d0    58  ScreenFade_Apply             ScreenFade_Apply
 0x0800526c    52  ScreenFade_Update            ScreenFade_Update
*0x080052f8    85  sub_80052F8                  sub_80052F8
*0x080053b4   216  sub_80053B4                  sub_80053B4
*0x080055e8   612  sub_80055E8                  sub_80055E8
 0x08005b2c    58  MapTile_At                   MapTile_At
 0x08005bb4    89  MapTile_CollisionBits        MapTile_CollisionBits
*0x08005c70   717  Viewport_UpdateScroll        Viewport_UpdateScroll
 0x080064ac    54  BgMap_FillPattern            BgMap_FillPattern
 0x08006520    92  MapBg_LoadFull               MapBg_LoadFull
*0x0800661c   993  MapScene_Load                MapScene_Load
*0x080071ec    78  MapScene_LoadNpcSlotIds      MapScene_LoadNpcSlotIds
 0x0800729c    73  MapScene_InitSprites         MapScene_InitSprites
*0x08007350   636  MapScene_LoadEventAnimations MapScene_LoadEventAnimations
 0x08007964    42  AnimSlot_Parse               AnimSlot_Parse
 0x080079bc    45  AnimSlot_ParseLoop           AnimSlot_ParseLoop
 0x08007a1c    91  AnimSlot_Step                AnimSlot_Step
 0x08007adc   111  sub_8007ADC                  sub_8007ADC
 0x08007bd0   133  sub_8007BD0                  sub_8007BD0
 0x08007d5c   174  MapBg_LoadInterior           MapBg_LoadInterior
 0x08007fb8   128  Logo_LoadAssets              Logo_LoadAssets
 0x08008124    65  ChoiceMenu_BuildList         ChoiceMenu_BuildList
 0x080081c0    60  BattleIntro_Cursor           BattleIntro_Cursor
*0x08008254   397  ChoiceMenu_HandleInput       ChoiceMenu_HandleInput
 0x08008620    87  DialogPortrait_Set           DialogPortrait_Set
 0x080086fc    60  Viewport_UpdateEffects       Viewport_UpdateEffects
 0x08008788   107  IntroBg_Load                 IntroBg_Load
 0x080088b4    22  ScreenFade_Start             ScreenFade_Start
 0x080088f4    37  AnimSlot_BankReload          AnimSlot_BankReload
 0x08008978    38  Win0H_WaveDmaByVCount        Win0H_WaveDmaByVCount
 0x080089e0    35  ScreenFx_SetMode             ScreenFx_SetMode
 0x08008a3c    15  AnimSlots_Release            AnimSlots_Release
 0x08008a60    15  AnimSlots_StepAll            AnimSlots_StepAll
 0x08008a80    35  BgTiles_LoadUiSet            BgTiles_LoadUiSet
 0x08008b14    26  BgScroll_LoadFromTable       BgScroll_LoadFromTable
 0x08008b5c    25  PlayerSheets_Load            PlayerSheets_Load
 0x08008ba4    28  AnimSlot_LoadSet             AnimSlot_LoadSet
 0x08008be4     9  AnimSlot_Pause               AnimSlot_Pause
 0x08008bfc     9  AnimSlot_Resume              AnimSlot_Resume
 0x08008c14     6  AnimSlot_Active              AnimSlot_Active
 0x08008c24    29  ReloadSpriteSheet            ReloadSpriteSheet
 0x08008c70    32  ReloadAllSpriteSheets        ReloadAllSpriteSheets
 0x08008cc0    35  ChoiceMenu_ResolveDest       ChoiceMenu_ResolveDest
 0x08008d18    33  DialogPortrait_FlushPending  DialogPortrait_FlushPending
 0x08008d78    30  Camera_GetDrawOffset         Camera_GetDrawOffset
 0x08008dcc     3  Script_SetEnvSet             Script_SetEnvSet
 0x08008dd8    10  BgPal_ResetFirst             BgPal_ResetFirst
 0x08008df8    34  AnimSlot_PlayOnce            AnimSlot_PlayOnce
```

### M04 [ENGINE/SYS]  0x08008E44–0x08009184  (8 函数 / 0 未匹配 / 0 指令)

锚点: BgMap_FillRow, ChestFlags_ClearAll, ChestFlags_Toggle, ChestObject_BuildSprite, ChestObject_Open, ChestObjects_LoadForMap, LoadDigitFontObjTiles, MapBg_FlushPending
工作内存: 0x3004870, 0x3004890, 0x40000d4, 0x3003ac0
ROM 表: 0x8088400, 0x8088c40, 0x8088c00

```
  地址        指令  ll.cfg 名                     函数
 0x08008e44    35  BgMap_FillRow                BgMap_FillRow
 0x08008e94    45  MapBg_FlushPending           MapBg_FlushPending
 0x08008f28    77  ChestObjects_LoadForMap      ChestObjects_LoadForMap
 0x08008fd0    78  ChestObject_BuildSprite      ChestObject_BuildSprite
 0x0800908c    52  ChestObject_Open             ChestObject_Open
 0x08009114    25  LoadDigitFontObjTiles        LoadDigitFontObjTiles
 0x08009168    12  ChestFlags_ClearAll          ChestFlags_ClearAll
 0x08009184    14  ChestFlags_Toggle            ChestFlags_Toggle
```

### M05 [ENGINE/SYS]  0x080091A4–0x08009E80  (19 函数 / 1 未匹配 / 477 指令)

锚点: ChestFlags_Test, MenuEnt_ClearStates, MenuEnt_GetState, MenuEnt_Lock, MenuEnt_ParseAll, MenuEnt_ParseDesc, MenuEnt_ParseRange, MenuEnt_Unlock, PaletteEffects_Update, PaletteFx_Apply
工作内存: 0x3000010, 0x3004910, 0x40000d4, 0x3000020
ROM 表: 0x87ea138, 0x808a234, 0x80baba0, 0x808ea0c

```
  地址        指令  ll.cfg 名                     函数
 0x080091a4    14  ChestFlags_Test              ChestFlags_Test
 0x080091c4   166  PaletteEffects_Update        PaletteEffects_Update
 0x08009370    71  PaletteTransfer_Update       PaletteTransfer_Update
 0x08009428    77  PaletteFx_Apply              PaletteFx_Apply
 0x080094fc   105  PaletteFx_Step               PaletteFx_Step
*0x08009600   477  PaletteFx_Transform          PaletteFx_Transform
 0x08009a5c    14  MenuEnt_ClearStates          MenuEnt_ClearStates
 0x08009a7c    32  MenuEnt_ParseAll             MenuEnt_ParseAll
 0x08009ac4    28  MenuEnt_ParseRange           MenuEnt_ParseRange
 0x08009b04     9  MenuEnt_Unlock               MenuEnt_Unlock
 0x08009b1c     9  MenuEnt_Lock                 MenuEnt_Lock
 0x08009b34     6  MenuEnt_GetState             MenuEnt_GetState
 0x08009b44    10  Palette_Backup               Palette_Backup
 0x08009b64    14  Palette_FillWhite            Palette_FillWhite
 0x08009b84    40  MenuEnt_ParseDesc            MenuEnt_ParseDesc
 0x08009bf0    54  StaticObjGfx_LoadPair        StaticObjGfx_LoadPair
 0x08009c84    78  StaticObjs_Spawn             StaticObjs_Spawn
 0x08009d34   157  StaticObjs_StepAll           StaticObjs_StepAll
 0x08009e80    91  StaticObj_BuildChain         StaticObj_BuildChain
```

### M06 [ENGINE/SYS]  0x08009F48–0x0800AC08  (29 函数 / 0 未匹配 / 0 指令)

锚点: Chara_ClearTempStatus, Chara_GetFormGfx, EquipItem, ExpToLevel, FullHealParty, Inventory_AddItem, ItemFindSlot, ItemGetValue, LevelToExp, PartyForm_ApplyBonus
工作内存: 0x3004ac0, 0x3004aa8, 0x3004ab8, 0x3004ab0
ROM 表: 0x87ea580, 0x8092248, 0x8093418, 0x80921f0

```
  地址        指令  ll.cfg 名                     函数
 0x08009f48    17  StaticObjs_Reset             StaticObjs_Reset
 0x08009f70    78  sub_8009F70                  sub_8009F70
 0x0800a048    73  Stats_BuildSkillList         Stats_BuildSkillList
 0x0800a0e4    86  Chara_GetFormGfx             Chara_GetFormGfx
 0x0800a1b4   245  sub_800A1B4                  sub_800A1B4
 0x0800a3c8   165  sub_800A3C8                  sub_800A3C8
 0x0800a534    85  sub_800A534                  sub_800A534
 0x0800a664   126  Stats_RebuildEquipBonuses    Stats_RebuildEquipBonuses
 0x0800a79c    83  Stats_RecalcEquip            Stats_RecalcEquip
 0x0800a86c    22  ExpToLevel                   ExpToLevel
 0x0800a8a0    21  LevelToExp                   LevelToExp
 0x0800a8d0    39  ItemFindSlot                 ItemFindSlot
 0x0800a924    22  Party_InitStats              Party_InitStats
 0x0800a958    10  ItemGetValue                 ItemGetValue
 0x0800a970     3  sub_800A970                  sub_800A970
 0x0800a978     3  sub_800A978                  sub_800A978
 0x0800a980    27  FullHealParty                FullHealParty
 0x0800a9c0    60  EquipItem                    EquipItem
 0x0800aa60    15  Inventory_AddItem            Inventory_AddItem
 0x0800aa84    13  sub_800AA84                  sub_800AA84
 0x0800aaa4     9  Silver_Add                   Silver_Add
 0x0800aac0    10  Silver_Sub                   Silver_Sub
 0x0800aadc    12  sub_800AADC                  sub_800AADC
 0x0800aaf8    14  sub_800AAF8                  sub_800AAF8
 0x0800ab18    15  sub_800AB18                  sub_800AB18
 0x0800ab3c    27  Party_AnyEquip               Party_AnyEquip
 0x0800ab7c    29  Chara_ClearTempStatus        Chara_ClearTempStatus
 0x0800abbc    20  Stats_ClearEquipBonus        Stats_ClearEquipBonus
 0x0800ac08    66  PartyForm_ApplyBonus         PartyForm_ApplyBonus
```

### M07 [ENGINE/SYS]  0x0800ACA4–0x08010770  (37 函数 / 14 未匹配 / 6,990 指令)

锚点: BattleIntro_Setup, FullHealCharacter, ItemGetUsePower, MenuHp_Update, MenuState_Reset, MenuUi_DrawItemList, MenuUi_SetEntityPos, MenuUi_SpawnAuxSprites, Msg_RenderLine, PartyUi_InitEntities
工作内存: 0x3000188, 0x3004aa0, 0x3000186, 0x3000187
ROM 表: 0x8095028, 0x87ea580, 0x87eb250, 0x8095828

```
  地址        指令  ll.cfg 名                     函数
 0x0800aca4    15  FullHealCharacter            FullHealCharacter
*0x0800acc8   248  sub_800ACC8                  sub_800ACC8
 0x0800b14c   132  SceneBg_Reload               SceneBg_Reload
 0x0800b2d0    26  MenuState_Reset              MenuState_Reset
 0x0800b314    39  MenuHp_Update                MenuHp_Update
*0x0800b374  1133  sub_800B374                  sub_800B374
 0x0800bee4    53  Msg_RenderLine               Msg_RenderLine
 0x0800bf5c    70  PartyUi_InitEntities         PartyUi_InitEntities
 0x0800bff8    99  sub_800BFF8                  sub_800BFF8
 0x0800c0d8    76  BattleIntro_Setup            BattleIntro_Setup
*0x0800c194   152  sub_800C194                  sub_800C194
*0x0800c2f8  3129  sub_800C2F8                  sub_800C2F8
 0x0800e170    90  MenuUi_SetEntityPos          MenuUi_SetEntityPos
*0x0800e244   374  sub_800E244                  sub_800E244
 0x0800e668    64  sub_800E668                  sub_800E668
 0x0800e71c    70  UiSprite_BeginSlide          UiSprite_BeginSlide
 0x0800e7bc   146  UiSprites_Update             UiSprites_Update
*0x0800e8f8   232  sub_800E8F8                  sub_800E8F8
*0x0800eae4    84  sub_800EAE4                  sub_800EAE4
 0x0800eb98    85  MenuUi_SpawnAuxSprites       MenuUi_SpawnAuxSprites
*0x0800ec54   508  sub_800EC54                  sub_800EC54
*0x0800f128   280  sub_800F128                  sub_800F128
 0x0800f3ac   101  sub_800F3AC                  sub_800F3AC
 0x0800f4a8   195  MenuUi_DrawItemList          MenuUi_DrawItemList
 0x0800f670    69  sub_800F670                  sub_800F670
*0x0800f70c   218  sub_800F70C                  sub_800F70C
 0x0800fa24   100  sub_800FA24                  sub_800FA24
*0x0800fb2c   304  sub_800FB2C                  sub_800FB2C
 0x0800fdec   127  sub_800FDEC                  sub_800FDEC
 0x0800ff10   214  sub_800FF10                  sub_800FF10
*0x08010170    92  sub_8010170                  sub_8010170
 0x0801026c    70  ItemGetUsePower              ItemGetUsePower
*0x08010300   120  sub_8010300                  sub_8010300
 0x08010434    79  WarpTable_Check              WarpTable_Check
*0x080104f8   116  sub_80104F8                  sub_80104F8
 0x08010624   139  sub_8010624                  sub_8010624
 0x08010770   200  sub_8010770                  sub_8010770
```

### M08 [ENGINE/SYS]  0x08010978–0x08016C2C  (85 函数 / 9 未匹配 / 4,864 指令)

锚点: Hud_DrawHp, Hud_DrawLv, Hud_DrawMp, InvUi_DrawCursors, InvUi_Main, Inv_FindFirstHeld, Inv_FindHeldItemOnPage, Inv_FindPrevHeld, Inv_NextNonZero, Inv_PrevNonZero
工作内存: 0x2005800, 0x3000187, 0x3004980, 0x3004aa0
ROM 表: 0x8098199, 0x8095028, 0x839cfaa, 0x87eb2e0

```
  地址        指令  ll.cfg 名                     函数
 0x08010978    55  ScreenIdleIcons_BuildList    ScreenIdleIcons_BuildList
 0x080109f8   192  Save_Fsm                     Save_Fsm
 0x08010bec   101  Save_FillSlot0               Save_FillSlot0
 0x08010ccc    80  Save_FillSlot1               Save_FillSlot1
 0x08010d80    80  Save_FillSlot2               Save_FillSlot2
 0x08010e58    78  Save_FillSlot3               Save_FillSlot3
*0x08010f10   264  sub_8010F10                  sub_8010F10
 0x0801114c   119  sub_801114C                  sub_801114C
 0x08011268   154  sub_8011268                  sub_8011268
 0x080113cc    56  sub_80113CC                  sub_80113CC
 0x08011454  1371  TitleMenu_ProcessFrame       TitleMenu_ProcessFrame
*0x08012530   252  sub_8012530                  sub_8012530
*0x08012790  1654  TitleMenu_UpdateUi           TitleMenu_UpdateUi
 0x08013870    83  TitleMenu_DrawOptions        TitleMenu_DrawOptions
 0x08013934   197  OptionsMenu_DrawEntries      OptionsMenu_DrawEntries
 0x08013b0c   108  sub_8013B0C                  sub_8013B0C
*0x08013c00   334  sub_8013C00                  sub_8013C00
 0x08013f3c    66  Save_LoadSlot0               Save_LoadSlot0
 0x08013fe8    69  Save_LoadContinue            Save_LoadContinue
 0x08014084    34  SaveTimer_CountUsed          SaveTimer_CountUsed
 0x080140d0    38  SaveTimer_Inc                SaveTimer_Inc
 0x08014124    40  SaveTimer_Dec                SaveTimer_Dec
*0x0801417c   296  sub_801417C                  sub_801417C
 0x08014488    92  sub_8014488                  sub_8014488
 0x0801455c   139  sub_801455C                  sub_801455C
*0x080146a8   390  sub_80146A8                  sub_80146A8
*0x08014a68  1003  sub_8014A68                  sub_8014A68
 0x0801543c    76  sub_801543C                  sub_801543C
 0x080154e8   171  sub_80154E8                  sub_80154E8
*0x08015658   470  sub_8015658                  sub_8015658
 0x08015af0    52  sub_8015AF0                  sub_8015AF0
 0x08015b90    42  InvUi_DrawCursors            InvUi_DrawCursors
*0x08015c18   201  InvUi_Main                   InvUi_Main
 0x08015e1c    51  sub_8015E1C                  sub_8015E1C
 0x08015e88     7  Save_ResetReadState          Save_ResetReadState
 0x08015ea0    16  Save_StartWrite              Save_StartWrite
 0x08015ed0    29  sub_8015ED0                  sub_8015ED0
 0x08015f14    26  SaveUi_DrawSlots             SaveUi_DrawSlots
 0x08015f50    15  SaveTimer_Get                SaveTimer_Get
 0x08015f74    14  SaveFlag_Set                 SaveFlag_Set
 0x08015f94    14  SaveFlag_Get                 SaveFlag_Get
 0x08015fb4    52  SaveUi_Open                  SaveUi_Open
 0x08016038    16  sub_8016038                  sub_8016038
 0x080160cc    10  sub_80160CC                  sub_80160CC
 0x080160f4    36  sub_80160F4                  sub_80160F4
 0x08016178    58  sub_8016178                  sub_8016178
 0x0801624c     8  Num_Draw16                   Num_Draw16
 0x08016260    29  Hud_DrawLv                   Hud_DrawLv
 0x080162a8    40  Hud_DrawHp                   Hud_DrawHp
 0x08016308    40  Hud_DrawMp                   Hud_DrawMp
 0x08016368    50  Text_PutGlyph                Text_PutGlyph
 0x080163cc    39  TextBlocks_Render            TextBlocks_Render
 0x08016424    15  Math_DivLoop                 Math_DivLoop
 0x08016444    11  Msg_ShowEndMark              Msg_ShowEndMark
 0x08016460    44  Msg_DrawPoolSegment          Msg_DrawPoolSegment
 0x080164c0    24  Msg_BuildSegmentIndex        Msg_BuildSegmentIndex
 0x080164f8     7  Msg_Show                     Msg_Show
 0x08016508    30  Msg_ShowById                 Msg_ShowById
 0x0801654c    50  Text_WriteOrClear            Text_WriteOrClear
 0x080165b8     6  Menu_GetFocus                Menu_GetFocus
 0x080165c8    43  Text_ClearRect               Text_ClearRect
 0x08016628    38  MenuUi_SetExclusive          MenuUi_SetExclusive
 0x0801667c    18  MenuUi_HideAll               MenuUi_HideAll
 0x080166a4    37  Text_DrawChar                Text_DrawChar
 0x080166fc    39  sub_80166FC                  sub_80166FC
 0x08016758    53  sub_8016758                  sub_8016758
 0x080167d4    16  MenuUi_MoveCursor            MenuUi_MoveCursor
 0x080167f8    21  Party_SlotOfMember           Party_SlotOfMember
 0x0801682c    27  SkillMenu_SaveCursor         SkillMenu_SaveCursor
 0x08016878    15  ItemUse_Execute              ItemUse_Execute
 0x080168a8    23  ItemUse_SetCtx               ItemUse_SetCtx
 0x080168ec    23  SkillMenu_RestoreCursor      SkillMenu_RestoreCursor
 0x08016930    27  SkillMenu_GetSkill           SkillMenu_GetSkill
 0x08016978    20  Inv_FindFirstHeld            Inv_FindFirstHeld
 0x080169ac    24  Inv_FindPrevHeld             Inv_FindPrevHeld
 0x080169ec    16  Inv_FindHeldItemOnPage       Inv_FindHeldItemOnPage
 0x08016a14    38  Save_SyncShadow              Save_SyncShadow
 0x08016a6c    20  Inv_SeekFirst                Inv_SeekFirst
 0x08016aa0    21  Inv_PrevNonZero              Inv_PrevNonZero
 0x08016ad4    41  Inv_NextNonZero              Inv_NextNonZero
 0x08016b30    57  sub_8016B30                  sub_8016B30
 0x08016bb0    16  SaveUi_OpenLoad              SaveUi_OpenLoad
 0x08016be0    24  Text_WriteChars              Text_WriteChars
 0x08016c10    14  Text_FillHidden              Text_FillHidden
 0x08016c2c    10  Text_TileAt                  Text_TileAt
```

### M09 [ENGINE/SYS]  0x08016C44–0x08017640  (16 函数 / 2 未匹配 / 540 指令)

锚点: Sio_BuildPacket, Sio_ClearSlot, Sio_IsHost, Sio_SetReady, Sio_SetXferCtx, Sio_Shutdown
工作内存: 0x3004df0, 0x4000208, 0x4000128, 0x3004f20

```
  地址        指令  ll.cfg 名                     函数
 0x08016c44    23  sub_8016C44                  sub_8016C44
 0x08016c88    61  sub_8016C88                  sub_8016C88
*0x08016d24   114  sub_8016D24                  sub_8016D24
 0x08016e30    34  Sio_BuildPacket              Sio_BuildPacket
 0x08016e80    78  sub_8016E80                  sub_8016E80
 0x08016f30    59  sub_8016F30                  sub_8016F30
 0x08016fc0   112  sub_8016FC0                  sub_8016FC0
 0x080170bc     7  Sio_SetReady                 Sio_SetReady
 0x080170d0    26  Sio_Shutdown                 Sio_Shutdown
 0x08017120    87  sub_8017120                  sub_8017120
*0x080171e4   426  sub_80171E4                  sub_80171E4
 0x08017588    21  Sio_IsHost                   Sio_IsHost
 0x080175c0    25  sub_80175C0                  sub_80175C0
 0x08017600    12  Sio_SetXferCtx               Sio_SetXferCtx
 0x0801761c    15  Sio_ClearSlot                Sio_ClearSlot
 0x08017640    38  sub_8017640                  sub_8017640
```

### M10 [ENGINE/CORE]  0x0801768C–0x0801A36C  (72 函数 / 5 未匹配 / 1,581 指令)

锚点: BattleFx_DispOff, BattleFx_Init, BattleFx_Stop, BattleFx_UpdateTable, BattleTask_Run, BattleUiFlag_Clear, BattleUiFlag_Get, BattleUiFlag_Reset, BattleUiFlag_Set, Bg0_InitClear
工作内存: 0x3000324, 0x3000384, 0x40000d4, 0x3000348
ROM 表: 0x80936a0, 0x861a4a4, 0x861a7e4, 0x87ed394

```
  地址        指令  ll.cfg 名                     函数
 0x0801768c   116  sub_801768C                  sub_801768C
*0x080177ac   692  BattleTask_Run               BattleTask_Run
 0x08017fa4    76  sub_8017FA4                  sub_8017FA4
 0x08018070   233  sub_8018070                  sub_8018070
 0x080182a8   201  sub_80182A8                  sub_80182A8
 0x080184a8   235  sub_80184A8                  sub_80184A8
 0x0801869c    39  sub_801869C                  sub_801869C
 0x08018744     4  sub_8018744                  sub_8018744
 0x08018750    25  sub_8018750                  sub_8018750
 0x0801878c     3  sub_801878C                  sub_801878C
 0x08018798     6  sub_8018798                  sub_8018798
 0x080187a8     3  sub_80187A8                  sub_80187A8
 0x080187b4     3  sub_80187B4                  sub_80187B4
 0x080187c0     7  sub_80187C0                  sub_80187C0
 0x080187d4     7  sub_80187D4                  sub_80187D4
 0x080187e8     3  sub_80187E8                  sub_80187E8
 0x080187f4     3  sub_80187F4                  sub_80187F4
 0x08018800     5  ListNode_Init                ListNode_Init
 0x0801880c     5  ListNode_InitKey             ListNode_InitKey
 0x08018818    16  ListNode_InsertSorted        ListNode_InsertSorted
 0x08018838     3  sub_8018838                  sub_8018838
 0x08018844    10  Rng_LcgNext                  Rng_LcgNext
 0x08018864     2  GetObjPool                   GetObjPool
 0x0801886c     2  GetCtx_0248                  GetCtx_0248
 0x08018874     2  GetBuf_37410                 GetBuf_37410
 0x0801887c    11  sub_801887C                  sub_801887C
 0x0801889c    11  sub_801889C                  sub_801889C
 0x080188bc    38  sub_80188BC                  sub_80188BC
 0x08018928   125  sub_8018928                  sub_8018928
 0x08018a58   158  sub_8018A58                  sub_8018A58
 0x08018bf8   164  sub_8018BF8                  sub_8018BF8
 0x08018d9c    50  sub_8018D9C                  sub_8018D9C
 0x08018e34    40  sub_8018E34                  sub_8018E34
*0x08018ea8   129  sub_8018EA8                  sub_8018EA8
*0x08018fc0   184  sub_8018FC0                  sub_8018FC0
 0x08019148    51  Bg0_InitClear                Bg0_InitClear
*0x080191cc   151  sub_80191CC                  sub_80191CC
 0x08019304    26  DialogCtx_Clear3             DialogCtx_Clear3
*0x0801933c   425  sub_801933C                  sub_801933C
 0x08019748    28  DialogCtx_SetPair            DialogCtx_SetPair
 0x08019784   249  BattleFx_UpdateTable         BattleFx_UpdateTable
 0x080199e0    94  sub_80199E0                  sub_80199E0
 0x08019ad0    76  sub_8019AD0                  sub_8019AD0
 0x08019b98   233  sub_8019B98                  sub_8019B98
 0x08019df8     4  BattleUiFlag_Clear           BattleUiFlag_Clear
 0x08019e04     7  BattleUiFlag_Set             BattleUiFlag_Set
 0x08019e18     3  BattleUiFlag_Get             BattleUiFlag_Get
 0x08019e24     7  BattleUiFlag_Reset           BattleUiFlag_Reset
 0x08019e38     7  Disp_ObjOff                  Disp_ObjOff
 0x08019e4c     9  Disp_ObjOn                   Disp_ObjOn
 0x08019e60    39  sub_8019E60                  sub_8019E60
 0x08019ecc     7  Disp_Bg1Off                  Disp_Bg1Off
 0x08019ee0    17  DialogCtx_SetHead            DialogCtx_SetHead
 0x08019f08    56  sub_8019F08                  sub_8019F08
 0x08019f78   114  sub_8019F78                  sub_8019F78
 0x0801a05c     9  DialogCtx_GetField_C         DialogCtx_GetField_C
 0x0801a0f0    28  DialogCtx_Flush              DialogCtx_Flush
 0x0801a13c     4  FlashFlag_Clear              FlashFlag_Clear
 0x0801a148     3  FlashFlag_Get                FlashFlag_Get
 0x0801a154     7  FlashFlag_Reset              FlashFlag_Reset
 0x0801a168    42  BattleFx_Init                BattleFx_Init
 0x0801a1dc    19  BattleFx_Stop                BattleFx_Stop
 0x0801a218    29  BattleFx_DispOff             BattleFx_DispOff
 0x0801a270    23  sub_801A270                  sub_801A270
 0x0801a2ac    25  sub_801A2AC                  sub_801A2AC
 0x0801a2ec    20  sub_801A2EC                  sub_801A2EC
 0x0801a324     4  BgLoad_Reset                 BgLoad_Reset
 0x0801a330     4  BgLoad_Finish                BgLoad_Finish
 0x0801a33c     3  BgLoad_GetPos                BgLoad_GetPos
 0x0801a348     6  sub_801A348                  sub_801A348
 0x0801a35c     6  sub_801A35C                  sub_801A35C
 0x0801a36c    26  BgScrolls_WriteAll           BgScrolls_WriteAll
```

### M11 [ENGINE/CORE]  0x0801A3A8–0x0801B8FC  (18 函数 / 3 未匹配 / 1,488 指令)

锚点: ObjGfxLoad_Copy, ObjGfxLoad_Step, sub_801B954
工作内存: 0x40000d4, 0x3000518, 0x202b2c0, 0x30035c0
ROM 表: 0x8393a30, 0x8393a24, 0x8393a18, 0x87ebe00

```
  地址        指令  ll.cfg 名                     函数
 0x0801a3a8    12  sub_801A3A8                  sub_801A3A8
 0x0801a3c4   193  ObjGfxLoad_Step              ObjGfxLoad_Step
 0x0801a5ec    75  ObjGfxLoad_Copy              ObjGfxLoad_Copy
 0x0801a684    52  sub_801A684                  sub_801A684
 0x0801a6f4   175  sub_801A6F4                  sub_801A6F4
*0x0801a884   520  sub_801A884                  sub_801A884
*0x0801ad0c   448  sub_801AD0C                  sub_801AD0C
*0x0801b0b8   520  sub_801B0B8                  sub_801B0B8
 0x0801b570   129  sub_801B570                  sub_801B570
 0x0801b688    77  sub_801B688                  sub_801B688
 0x0801b760    21  sub_801B760                  sub_801B760
 0x0801b790    18  sub_801B790                  sub_801B790
 0x0801b7b8    42  sub_801B7B8                  sub_801B7B8
 0x0801b81c    45  sub_801B81C                  sub_801B81C
 0x0801b878    23  sub_801B878                  sub_801B878
 0x0801b8ac    27  sub_801B8AC                  sub_801B8AC
 0x0801b8e8    10  sub_801B8E8                  sub_801B8E8
 0x0801b8fc    17  sub_801B8FC                  sub_801B8FC
```

### M12 [ENGINE/CORE]  0x0801B920–0x08021064  (89 函数 / 11 未匹配 / 4,021 指令)

锚点: ResetSceneObjects, sub_8020B54
工作内存: 0x300068d, 0x30006f8, 0x300068c, 0x3000670
ROM 表: 0x8393b28, 0x861c664, 0x839cc4c, 0x861a004

```
  地址        指令  ll.cfg 名                     函数
 0x0801b920    22  sub_801B920                  sub_801B920
 0x0801b954     3  sub_801B954                  sub_801B954
 0x0801b95c     3  sub_801B95C                  sub_801B95C
*0x0801b964   540  sub_801B964                  sub_801B964
*0x0801be34   565  sub_801BE34                  sub_801BE34
*0x0801c484   515  sub_801C484                  sub_801C484
 0x0801ca08   165  sub_801CA08                  sub_801CA08
*0x0801cba4   298  sub_801CBA4                  sub_801CBA4
 0x0801ce80   108  sub_801CE80                  sub_801CE80
 0x0801cf90   191  sub_801CF90                  sub_801CF90
 0x0801d12c    55  sub_801D12C                  sub_801D12C
 0x0801d19c    59  sub_801D19C                  sub_801D19C
 0x0801d214   135  sub_801D214                  sub_801D214
 0x0801d378   112  sub_801D378                  sub_801D378
 0x0801d468   114  sub_801D468                  sub_801D468
 0x0801d568   187  sub_801D568                  sub_801D568
 0x0801d710   278  sub_801D710                  sub_801D710
 0x0801d984   122  sub_801D984                  sub_801D984
 0x0801daa0    62  sub_801DAA0                  sub_801DAA0
 0x0801db3c   103  sub_801DB3C                  sub_801DB3C
 0x0801dc20   100  sub_801DC20                  sub_801DC20
 0x0801dd04    78  sub_801DD04                  sub_801DD04
 0x0801ddb0    66  sub_801DDB0                  sub_801DDB0
 0x0801de44    58  sub_801DE44                  sub_801DE44
 0x0801dedc    80  sub_801DEDC                  sub_801DEDC
 0x0801df90    78  sub_801DF90                  sub_801DF90
 0x0801e040   173  sub_801E040                  sub_801E040
*0x0801e1d8   130  sub_801E1D8                  sub_801E1D8
 0x0801e30c   208  sub_801E30C                  sub_801E30C
 0x0801e4d4   205  sub_801E4D4                  sub_801E4D4
 0x0801e690   203  sub_801E690                  sub_801E690
*0x0801e848   236  sub_801E848                  sub_801E848
 0x0801ea70   221  sub_801EA70                  sub_801EA70
 0x0801ec3c    88  sub_801EC3C                  sub_801EC3C
 0x0801ed40   130  sub_801ED40                  sub_801ED40
 0x0801ee6c    53  sub_801EE6C                  sub_801EE6C
*0x0801eee4   504  sub_801EEE4                  sub_801EEE4
*0x0801f3fc   320  sub_801F3FC                  sub_801F3FC
 0x0801f76c    46  sub_801F76C                  sub_801F76C
 0x0801f884   178  sub_801F884                  sub_801F884
 0x0801fa10    65  sub_801FA10                  sub_801FA10
*0x0801fab8   419  sub_801FAB8                  sub_801FAB8
 0x0801febc    48  sub_801FEBC                  sub_801FEBC
 0x0801ff40   196  sub_801FF40                  sub_801FF40
 0x080200e8   155  sub_80200E8                  sub_80200E8
 0x08020228   115  sub_8020228                  sub_8020228
*0x0802031c   344  sub_802031C                  sub_802031C
*0x08020648   150  sub_8020648                  sub_8020648
 0x08020798     3  sub_8020798                  sub_8020798
 0x080207a4     5  sub_80207A4                  sub_80207A4
 0x080207b4    17  sub_80207B4                  sub_80207B4
 0x080207dc    46  sub_80207DC                  sub_80207DC
 0x08020840    46  sub_8020840                  sub_8020840
 0x080208a4    51  sub_80208A4                  sub_80208A4
 0x08020914    19  sub_8020914                  sub_8020914
 0x0802093c    27  sub_802093C                  sub_802093C
 0x08020974    39  sub_8020974                  sub_8020974
 0x080209c8    18  sub_80209C8                  sub_80209C8
 0x080209ec    15  sub_80209EC                  sub_80209EC
 0x08020a0c    50  sub_8020A0C                  sub_8020A0C
 0x08020a7c    24  sub_8020A7C                  sub_8020A7C
 0x08020ab0    22  sub_8020AB0                  sub_8020AB0
 0x08020ae4    13  sub_8020AE4                  sub_8020AE4
 0x08020b04    30  sub_8020B04                  sub_8020B04
 0x08020b48     3  sub_8020B48                  sub_8020B48
 0x08020b54    22  sub_8020B54                  sub_8020B54
 0x08020b90    18  sub_8020B90                  sub_8020B90
 0x08020bc0    24  sub_8020BC0                  sub_8020BC0
 0x08020bf0    26  sub_8020BF0                  sub_8020BF0
 0x08020c2c    16  sub_8020C2C                  sub_8020C2C
 0x08020c58    49  sub_8020C58                  sub_8020C58
 0x08020cc4    66  sub_8020CC4                  sub_8020CC4
 0x08020d50    38  sub_8020D50                  sub_8020D50
 0x08020da0    29  sub_8020DA0                  sub_8020DA0
 0x08020de4     4  sub_8020DE4                  sub_8020DE4
 0x08020df0    42  sub_8020DF0                  sub_8020DF0
 0x08020e54     2  sub_8020E54                  sub_8020E54
 0x08020e5c     3  sub_8020E5C                  sub_8020E5C
 0x08020e68     3  sub_8020E68                  sub_8020E68
 0x08020e74    11  sub_8020E74                  sub_8020E74
 0x08020e90    11  sub_8020E90                  sub_8020E90
 0x08020eac    11  sub_8020EAC                  sub_8020EAC
 0x08020ec8    14  sub_8020EC8                  sub_8020EC8
 0x08020eec     9  sub_8020EEC                  sub_8020EEC
 0x08020f08    26  sub_8020F08                  sub_8020F08
 0x08020f4c    39  sub_8020F4C                  sub_8020F4C
 0x08020fb8    49  sub_8020FB8                  sub_8020FB8
 0x0802103c    17  sub_802103C                  sub_802103C
 0x08021064    34  sub_8021064                  sub_8021064
```

### M13 [ENGINE/CORE]  0x080210C0–0x080256E4  (26 函数 / 14 未匹配 / 6,411 指令)

锚点: MenuSlot_ResetAll, SetHead
工作内存: 0x300076c, 0x3000781, 0x2035ac0, 0x3000768
ROM 表: 0x8393a74, 0x8393a30, 0x861a7a4, 0x839b2a4

```
  地址        指令  ll.cfg 名                     函数
 0x080210c0    50  sub_80210C0                  sub_80210C0
 0x08021130    38  MenuSlot_ResetAll            MenuSlot_ResetAll
 0x08021184   115  sub_8021184                  sub_8021184
*0x080212b4   250  sub_80212B4                  sub_80212B4
 0x0802151c   201  sub_802151C                  sub_802151C
 0x08021700    57  sub_8021700                  sub_8021700
 0x08021788    59  sub_8021788                  sub_8021788
*0x0802181c   125  sub_802181C                  sub_802181C
*0x0802192c  1177  sub_802192C                  sub_802192C
*0x08022458   109  sub_8022458                  sub_8022458
 0x08022550   126  sub_8022550                  sub_8022550
*0x08022710   986  sub_8022710                  sub_8022710
*0x08022f2c   188  sub_8022F2C                  sub_8022F2C
*0x080230bc   262  sub_80230BC                  sub_80230BC
*0x08023320   103  sub_8023320                  sub_8023320
*0x08023414   443  sub_8023414                  sub_8023414
*0x08023820  1226  sub_8023820                  sub_8023820
*0x080244bc   164  sub_80244BC                  sub_80244BC
*0x08024618    93  sub_8024618                  sub_8024618
*0x080246e8   148  sub_80246E8                  sub_80246E8
 0x08024820    97  sub_8024820                  sub_8024820
*0x08024940  1137  sub_8024940                  sub_8024940
 0x0802550c     3  sub_802550C                  sub_802550C
 0x08025518   113  sub_8025518                  sub_8025518
 0x08025638     8  sub_8025638                  sub_8025638
 0x080256e4    59  sub_80256E4                  sub_80256E4
```

### M14 [ENGINE/OBJECT]  0x0802576C–0x080313EC  (62 函数 / 4 未匹配 / 1,888 指令)

调度表: 0x0839CD5C(87)
工作内存: 0x3000825, 0x3000820, 0x3000822, 0x3000824
ROM 表: 0x8393b28, 0x8393a48, 0x8393b20, 0x8393a4d

```
  地址        指令  ll.cfg 名                     函数
 0x0802576c    46  sub_802576C                  sub_802576C
 0x080257d8   161  sub_80257D8                  sub_80257D8
*0x08025994   424  sub_8025994                  sub_8025994
*0x08025da8   273  sub_8025DA8                  sub_8025DA8
 0x080260bc   366  sub_80260BC                  sub_80260BC
*0x080264c0   486  sub_80264C0                  sub_80264C0
 0x0802698c   328  sub_802698C                  sub_802698C
 0x08026d08   242  sub_8026D08                  sub_8026D08
 0x08026f88   221  sub_8026F88                  sub_8026F88
 0x0802723c   328  sub_802723C                  sub_802723C
 0x0802761c   122  sub_802761C                  sub_802761C
 0x08027760   229  sub_8027760                  sub_8027760
 0x08027a20   294  sub_8027A20                  sub_8027A20
 0x08027d9c   263  sub_8027D9C                  sub_8027D9C
 0x08028098   198  sub_8028098                  sub_8028098
 0x080282ec   230  sub_80282EC                  sub_80282EC
 0x080285a0   189  sub_80285A0                  sub_80285A0
 0x080287ec   242  sub_80287EC                  sub_80287EC
*0x08028ad8   705  sub_8028AD8                  sub_8028AD8
 0x08029268   251  sub_8029268                  sub_8029268
 0x08029510   219  sub_8029510                  sub_8029510
 0x08029784   198  sub_8029784                  sub_8029784
 0x080299c8   200  sub_80299C8                  sub_80299C8
 0x08029bf8   467  sub_8029BF8                  sub_8029BF8
 0x0802a154   638  sub_802A154                  sub_802A154
 0x0802a86c   468  sub_802A86C                  sub_802A86C
 0x0802adc4   258  sub_802ADC4                  sub_802ADC4
 0x0802b0f0   470  sub_802B0F0                  sub_802B0F0
 0x0802b608   219  sub_802B608                  sub_802B608
 0x0802b8bc   197  sub_802B8BC                  sub_802B8BC
 0x0802bb24   203  sub_802BB24                  sub_802BB24
 0x0802bd94   287  sub_802BD94                  sub_802BD94
 0x0802c0ec   316  sub_802C0EC                  sub_802C0EC
 0x0802c490   200  sub_802C490                  sub_802C490
 0x0802c714   238  sub_802C714                  sub_802C714
 0x0802c9e8   421  sub_802C9E8                  sub_802C9E8
 0x0802ce90   296  sub_802CE90                  sub_802CE90
 0x0802d1fc   196  sub_802D1FC                  sub_802D1FC
 0x0802d454   239  sub_802D454                  sub_802D454
 0x0802d728   295  sub_802D728                  sub_802D728
 0x0802da78   317  sub_802DA78                  sub_802DA78
 0x0802de04   146  sub_802DE04                  sub_802DE04
 0x0802dfdc   191  sub_802DFDC                  sub_802DFDC
 0x0802e234   195  sub_802E234                  sub_802E234
 0x0802e49c   179  sub_802E49C                  sub_802E49C
 0x0802e6c8   351  sub_802E6C8                  sub_802E6C8
 0x0802eac4   262  sub_802EAC4                  sub_802EAC4
 0x0802edd8   263  sub_802EDD8                  sub_802EDD8
 0x0802f100   302  sub_802F100                  sub_802F100
 0x0802f480   191  sub_802F480                  sub_802F480
 0x0802f6d8   257  sub_802F6D8                  sub_802F6D8
 0x0802f9ec   415  sub_802F9EC                  sub_802F9EC
 0x0802fe98   348  sub_802FE98                  sub_802FE98
 0x0803029c   319  sub_803029C                  sub_803029C
 0x08030664   276  sub_8030664                  sub_8030664
 0x080309b0   191  sub_80309B0                  sub_80309B0
 0x08030c08   122  sub_8030C08                  sub_8030C08
 0x08030d9c   122  sub_8030D9C                  sub_8030D9C
 0x08030f30   122  sub_8030F30                  sub_8030F30
 0x080310c4   122  sub_80310C4                  sub_80310C4
 0x08031258   122  sub_8031258                  sub_8031258
 0x080313ec   122  sub_80313EC                  sub_80313EC
```

### M15 [ENGINE/OBJECT]  0x08031580–0x080323B4  (10 函数 / 0 未匹配 / 0 指令)

调度表: 0x0839CD5C(87)
工作内存: 0x3000868, 0x3000889, 0x3000825, 0x3000867

```
  地址        指令  ll.cfg 名                     函数
 0x08031580   122  sub_8031580                  sub_8031580
 0x08031714   122  sub_8031714                  sub_8031714
 0x080318a8   122  sub_80318A8                  sub_80318A8
 0x08031a3c   122  sub_8031A3C                  sub_8031A3C
 0x08031bd0   122  sub_8031BD0                  sub_8031BD0
 0x08031d64   122  sub_8031D64                  sub_8031D64
 0x08031ef8   122  sub_8031EF8                  sub_8031EF8
 0x0803208c   122  sub_803208C                  sub_803208C
 0x08032220   122  sub_8032220                  sub_8032220
 0x080323b4   122  sub_80323B4                  sub_80323B4
```

### M16 [ENGINE/OBJECT]  0x08032548–0x0803F658  (61 函数 / 23 未匹配 / 11,817 指令)

调度表: 0x0839D4CC(59)
工作内存: 0x3000820, 0x3000822, 0x3000824, 0x3000825
ROM 表: 0x8393a48, 0x8393a4d, 0x839df67, 0x839df64

```
  地址        指令  ll.cfg 名                     函数
 0x08032548   175  sub_8032548                  sub_8032548
 0x0803272c   189  sub_803272C                  sub_803272C
 0x08032948   375  sub_8032948                  sub_8032948
 0x08032d74    80  sub_8032D74                  sub_8032D74
*0x08032ea0   534  sub_8032EA0                  sub_8032EA0
*0x080334b8   387  sub_80334B8                  sub_80334B8
*0x08033988   371  sub_8033988                  sub_8033988
 0x08033e2c   519  sub_8033E2C                  sub_8033E2C
 0x08034440   122  sub_8034440                  sub_8034440
 0x080345ac   122  sub_80345AC                  sub_80345AC
 0x08034718   121  sub_8034718                  sub_8034718
 0x080348a8   286  sub_80348A8                  sub_80348A8
 0x08034bfc   140  sub_8034BFC                  sub_8034BFC
 0x08034d94   122  sub_8034D94                  sub_8034D94
 0x08034f00   177  sub_8034F00                  sub_8034F00
 0x08035130   178  sub_8035130                  sub_8035130
*0x08035360   453  sub_8035360                  sub_8035360
 0x0803586c   221  sub_803586C                  sub_803586C
 0x08035b04   221  sub_8035B04                  sub_8035B04
 0x08035d9c   222  sub_8035D9C                  sub_8035D9C
 0x08036034   221  sub_8036034                  sub_8036034
 0x080362cc   221  sub_80362CC                  sub_80362CC
 0x08036564   321  sub_8036564                  sub_8036564
 0x080368fc   178  sub_80368FC                  sub_80368FC
 0x08036b30   288  sub_8036B30                  sub_8036B30
 0x08036ea4   145  sub_8036EA4                  sub_8036EA4
 0x08037078   252  sub_8037078                  sub_8037078
*0x08037388   434  sub_8037388                  sub_8037388
 0x08037868   312  sub_8037868                  sub_8037868
 0x08037c40   145  sub_8037C40                  sub_8037C40
 0x08037e14   143  sub_8037E14                  sub_8037E14
 0x08037fe8   143  sub_8037FE8                  sub_8037FE8
 0x080381bc   143  sub_80381BC                  sub_80381BC
 0x08038390   143  sub_8038390                  sub_8038390
 0x08038568   151  sub_8038568                  sub_8038568
 0x0803874c   143  sub_803874C                  sub_803874C
*0x08038920   297  sub_8038920                  sub_8038920
 0x08038c84   157  sub_8038C84                  sub_8038C84
 0x08038e44   169  sub_8038E44                  sub_8038E44
 0x08039024   222  sub_8039024                  sub_8039024
 0x080392c0    74  sub_80392C0                  sub_80392C0
 0x080393e0   286  sub_80393E0                  sub_80393E0
*0x08039724   438  sub_8039724                  sub_8039724
*0x08039c38   728  sub_8039C38                  sub_8039C38
*0x0803a478   362  sub_803A478                  sub_803A478
*0x0803a8d0   575  sub_803A8D0                  sub_803A8D0
*0x0803af60   453  sub_803AF60                  sub_803AF60
*0x0803b484   681  sub_803B484                  sub_803B484
*0x0803bbec   630  sub_803BBEC                  sub_803BBEC
*0x0803c328  1025  sub_803C328                  sub_803C328
*0x0803ce0c   325  sub_803CE0C                  sub_803CE0C
*0x0803d20c   324  sub_803D20C                  sub_803D20C
*0x0803d60c   787  sub_803D60C                  sub_803D60C
*0x0803decc   576  sub_803DECC                  sub_803DECC
*0x0803e58c   738  sub_803E58C                  sub_803E58C
*0x0803ed34   481  sub_803ED34                  sub_803ED34
*0x0803f21c   128  sub_803F21C                  sub_803F21C
 0x0803f328    93  sub_803F328                  sub_803F328
*0x0803f444   164  sub_803F444                  sub_803F444
 0x0803f5b4    70  sub_803F5B4                  sub_803F5B4
*0x0803f658   926  sub_803F658                  sub_803F658
```

### M17 [ENGINE/OBJECT]  0x0803FF54–0x08044680  (32 函数 / 14 未匹配 / 5,631 指令)

调度表: 0x0839CEC4(14)
工作内存: 0x3000820, 0x3000825, 0x3000822, 0x3000824
ROM 表: 0x83988a8, 0x839d4c4, 0x839cc4c, 0x8393b28

```
  地址        指令  ll.cfg 名                     函数
 0x0803ff54   140  sub_803FF54                  sub_803FF54
 0x080401ac   293  sub_80401AC                  sub_80401AC
 0x080405a4    93  sub_80405A4                  sub_80405A4
*0x08040690   861  sub_8040690                  sub_8040690
*0x08040ee8   399  sub_8040EE8                  sub_8040EE8
*0x08041308   351  sub_8041308                  sub_8041308
*0x080416f0   280  sub_80416F0                  sub_80416F0
*0x080419e0   503  sub_80419E0                  sub_80419E0
*0x08041edc   313  sub_8041EDC                  sub_8041EDC
 0x08042200    75  sub_8042200                  sub_8042200
*0x080422b8   466  sub_80422B8                  sub_80422B8
*0x08042784   283  sub_8042784                  sub_8042784
 0x08042ab4    89  sub_8042AB4                  sub_8042AB4
*0x08042b90   238  sub_8042B90                  sub_8042B90
*0x08042e70   716  sub_8042E70                  sub_8042E70
*0x08043554   337  sub_8043554                  sub_8043554
*0x08043938   168  sub_8043938                  sub_8043938
*0x08043b5c   365  sub_8043B5C                  sub_8043B5C
*0x08043f90   351  sub_8043F90                  sub_8043F90
 0x08044394    57  sub_8044394                  sub_8044394
 0x08044414     4  sub_8044414                  sub_8044414
 0x08044420     3  sub_8044420                  sub_8044420
 0x0804442c    29  sub_804442C                  sub_804442C
 0x0804448c     4  sub_804448C                  sub_804448C
 0x08044498     3  sub_8044498                  sub_8044498
 0x080444a4    32  sub_80444A4                  sub_80444A4
 0x080444e8    16  sub_80444E8                  sub_80444E8
 0x08044514    29  sub_8044514                  sub_8044514
 0x08044574    35  sub_8044574                  sub_8044574
 0x080445e0     2  sub_80445E0                  sub_80445E0
 0x080445e8    64  sub_80445E8                  sub_80445E8
 0x08044680    15  sub_8044680                  sub_8044680
```

### M18 [ENGINE/OBJECT]  0x080446A4–0x0804473C  (8 函数 / 0 未匹配 / 0 指令)

调度表: 0x0839D4CC(59)
工作内存: 0x3000826, 0x3000884
ROM 表: 0x839dbf6

```
  地址        指令  ll.cfg 名                     函数
 0x080446a4     9  sub_80446A4                  sub_80446A4
 0x080446bc    49  sub_80446BC                  sub_80446BC
 0x08044728     2  sub_8044728                  sub_8044728
 0x0804472c     2  sub_804472C                  sub_804472C
 0x08044730     2  sub_8044730                  sub_8044730
 0x08044734     2  sub_8044734                  sub_8044734
 0x08044738     2  sub_8044738                  sub_8044738
 0x0804473c    56  sub_804473C                  sub_804473C
```

### M19 [ENGINE/CORE]  0x080448A8–0x08048B30  (65 函数 / 18 未匹配 / 3,430 指令)

工作内存: 0x3004ac0, 0x3004a88, 0x30008ec, 0x30008f0
ROM 表: 0x8093418, 0x839d5bc, 0x839d81a, 0x83988a8

```
  地址        指令  ll.cfg 名                     函数
 0x080448a8    81  sub_80448A8                  sub_80448A8
*0x08044a40   422  sub_8044A40                  sub_8044A40
 0x08044f4c   147  sub_8044F4C                  sub_8044F4C
*0x08045098    71  sub_8045098                  sub_8045098
*0x0804519c   164  sub_804519C                  sub_804519C
 0x08045328    82  sub_8045328                  sub_8045328
 0x080453d8    39  sub_80453D8                  sub_80453D8
 0x0804542c    55  sub_804542C                  sub_804542C
 0x080454a4   116  sub_80454A4                  sub_80454A4
 0x080455a0    70  sub_80455A0                  sub_80455A0
 0x08045688   106  sub_8045688                  sub_8045688
 0x080457ac    83  sub_80457AC                  sub_80457AC
 0x08045860   103  sub_8045860                  sub_8045860
 0x08045940    85  sub_8045940                  sub_8045940
 0x08045a10    46  sub_8045A10                  sub_8045A10
 0x08045a74   138  sub_8045A74                  sub_8045A74
 0x08045b90    46  sub_8045B90                  sub_8045B90
 0x08045bf4   109  sub_8045BF4                  sub_8045BF4
 0x08045d00   203  sub_8045D00                  sub_8045D00
 0x08045eb8    43  sub_8045EB8                  sub_8045EB8
 0x08045f10    46  sub_8045F10                  sub_8045F10
 0x08045f94    83  sub_8045F94                  sub_8045F94
 0x08046060    83  sub_8046060                  sub_8046060
 0x0804612c   110  sub_804612C                  sub_804612C
 0x0804621c    96  sub_804621C                  sub_804621C
 0x080462e4   203  sub_80462E4                  sub_80462E4
 0x08046480   102  sub_8046480                  sub_8046480
*0x08046558   136  sub_8046558                  sub_8046558
 0x0804666c    62  sub_804666C                  sub_804666C
*0x080466f0   639  sub_80466F0                  sub_80466F0
 0x08046c50    63  sub_8046C50                  sub_8046C50
*0x08046cd4   158  sub_8046CD4                  sub_8046CD4
*0x08046e18   114  sub_8046E18                  sub_8046E18
 0x08046f0c   104  sub_8046F0C                  sub_8046F0C
 0x08047024   158  sub_8047024                  sub_8047024
*0x080471ac   152  sub_80471AC                  sub_80471AC
*0x080472e8   117  sub_80472E8                  sub_80472E8
*0x0804753c   163  sub_804753C                  sub_804753C
*0x080476dc   432  sub_80476DC                  sub_80476DC
 0x08047b1c   190  sub_8047B1C                  sub_8047B1C
 0x08047d28    77  sub_8047D28                  sub_8047D28
*0x08047dc8   121  sub_8047DC8                  sub_8047DC8
 0x08047fcc    25  sub_8047FCC                  sub_8047FCC
*0x080480ec    96  sub_80480EC                  sub_80480EC
*0x080481b8   162  sub_80481B8                  sub_80481B8
*0x08048310   156  sub_8048310                  sub_8048310
*0x08048458   157  sub_8048458                  sub_8048458
*0x080485a4    74  sub_80485A4                  sub_80485A4
*0x08048690    96  sub_8048690                  sub_8048690
 0x08048764    11  sub_8048764                  sub_8048764
 0x0804877c    17  sub_804877C                  sub_804877C
 0x080487a4    16  sub_80487A4                  sub_80487A4
 0x080487cc    32  sub_80487CC                  sub_80487CC
 0x08048818    33  sub_8048818                  sub_8048818
 0x08048868    45  sub_8048868                  sub_8048868
 0x080488cc    50  sub_80488CC                  sub_80488CC
 0x08048934    35  sub_8048934                  sub_8048934
 0x08048984    14  sub_8048984                  sub_8048984
 0x080489a4    16  sub_80489A4                  sub_80489A4
 0x080489c8    15  sub_80489C8                  sub_80489C8
 0x080489e8    63  sub_80489E8                  sub_80489E8
 0x08048a68    15  sub_8048A68                  sub_8048A68
 0x08048a88    30  sub_8048A88                  sub_8048A88
 0x08048acc    41  sub_8048ACC                  sub_8048ACC
 0x08048b30    13  sub_8048B30                  sub_8048B30
```

### M20 [ENGINE/CORE]  0x08048B5C–0x08048D40  (8 函数 / 0 未匹配 / 0 指令)

ROM 表: 0x839cc4c, 0x839d5bc

```
  地址        指令  ll.cfg 名                     函数
 0x08048b5c    21  sub_8048B5C                  sub_8048B5C
 0x08048b88    15  sub_8048B88                  sub_8048B88
 0x08048bac    15  sub_8048BAC                  sub_8048BAC
 0x08048bd0    24  sub_8048BD0                  sub_8048BD0
 0x08048c30    38  sub_8048C30                  sub_8048C30
 0x08048c80    50  sub_8048C80                  sub_8048C80
 0x08048cec    42  sub_8048CEC                  sub_8048CEC
 0x08048d40    18  sub_8048D40                  sub_8048D40
```

### M21 [ENGINE/CORE]  0x08048D64–0x0804AD24  (25 函数 / 7 未匹配 / 1,782 指令)

工作内存: 0x300094d, 0x3000949, 0x3000910, 0x300094a
ROM 表: 0x839b2e0, 0x839d348, 0x861a7a4, 0x8095028

```
  地址        指令  ll.cfg 名                     函数
 0x08048d64    15  sub_8048D64                  sub_8048D64
 0x08048d84    15  sub_8048D84                  sub_8048D84
 0x08048da4   121  sub_8048DA4                  sub_8048DA4
 0x08048f0c    67  sub_8048F0C                  sub_8048F0C
*0x08048fb8   231  sub_8048FB8                  sub_8048FB8
*0x080492c0   216  sub_80492C0                  sub_80492C0
*0x080494f0   227  sub_80494F0                  sub_80494F0
 0x080497b0   139  sub_80497B0                  sub_80497B0
 0x080498e0    49  sub_80498E0                  sub_80498E0
 0x08049958   147  sub_8049958                  sub_8049958
 0x08049ad8    67  sub_8049AD8                  sub_8049AD8
*0x08049b70    73  sub_8049B70                  sub_8049B70
 0x08049c1c   116  sub_8049C1C                  sub_8049C1C
 0x08049d58    60  sub_8049D58                  sub_8049D58
*0x08049df8   335  sub_8049DF8                  sub_8049DF8
 0x0804a148   243  sub_804A148                  sub_804A148
*0x0804a368   642  sub_804A368                  sub_804A368
 0x0804aa2c   105  sub_804AA2C                  sub_804AA2C
 0x0804ab10    15  sub_804AB10                  sub_804AB10
*0x0804ab40    58  sub_804AB40                  sub_804AB40
 0x0804abd0    15  sub_804ABD0                  sub_804ABD0
 0x0804abf8    43  sub_804ABF8                  sub_804ABF8
 0x0804ac60    36  sub_804AC60                  sub_804AC60
 0x0804acc0    43  sub_804ACC0                  sub_804ACC0
 0x0804ad24    20  sub_804AD24                  sub_804AD24
```

### M22 [ENGINE/CORE]  0x0804AD54–0x0804B1F8  (9 函数 / 1 未匹配 / 115 指令)

工作内存: 0x3000ade, 0x3000948, 0x3000910, 0x3000ad8
ROM 表: 0x8619a60, 0x8619430, 0x8393a24

```
  地址        指令  ll.cfg 名                     函数
 0x0804ad54     6  sub_804AD54                  sub_804AD54
 0x0804ad60    44  sub_804AD60                  sub_804AD60
 0x0804ade0     7  sub_804ADE0                  sub_804ADE0
 0x0804adf8    17  sub_804ADF8                  sub_804ADF8
 0x0804ae2c   126  sub_804AE2C                  sub_804AE2C
*0x0804af60   115  sub_804AF60                  sub_804AF60
 0x0804b080   158  sub_804B080                  sub_804B080
 0x0804b1ec     4  sub_804B1EC                  sub_804B1EC
 0x0804b1f8    13  sub_804B1F8                  sub_804B1F8
```

### M23 [ENGINE/CORE]  0x0804B224–0x0804C6B0  (41 函数 / 2 未匹配 / 292 指令)

工作内存: 0x3000ae8, 0x3000be8, 0x40000d4, 0x3000ae0
ROM 表: 0x861aaa4, 0x861c764

```
  地址        指令  ll.cfg 名                     函数
 0x0804b224    36  sub_804B224                  sub_804B224
 0x0804b288   131  sub_804B288                  sub_804B288
 0x0804b3c0    74  sub_804B3C0                  sub_804B3C0
 0x0804b458    57  sub_804B458                  sub_804B458
 0x0804b4d0    70  sub_804B4D0                  sub_804B4D0
*0x0804b56c   116  sub_804B56C                  sub_804B56C
 0x0804b654   161  sub_804B654                  sub_804B654
 0x0804b7b0    61  sub_804B7B0                  sub_804B7B0
 0x0804b834    87  sub_804B834                  sub_804B834
 0x0804b8e8    61  sub_804B8E8                  sub_804B8E8
 0x0804b96c   235  sub_804B96C                  sub_804B96C
 0x0804bb64    55  sub_804BB64                  sub_804BB64
*0x0804bbdc   176  sub_804BBDC                  sub_804BBDC
 0x0804bd54    61  sub_804BD54                  sub_804BD54
 0x0804bdd8    87  sub_804BDD8                  sub_804BDD8
 0x0804be90    61  sub_804BE90                  sub_804BE90
 0x0804bf14   235  sub_804BF14                  sub_804BF14
 0x0804c10c    55  sub_804C10C                  sub_804C10C
 0x0804c184     5  sub_804C184                  sub_804C184
 0x0804c194    12  sub_804C194                  sub_804C194
 0x0804c1b4    21  sub_804C1B4                  sub_804C1B4
 0x0804c1e4    21  sub_804C1E4                  sub_804C1E4
 0x0804c214    25  sub_804C214                  sub_804C214
 0x0804c250    17  sub_804C250                  sub_804C250
 0x0804c278    17  sub_804C278                  sub_804C278
 0x0804c2a0    38  sub_804C2A0                  sub_804C2A0
 0x0804c2f0     3  sub_804C2F0                  sub_804C2F0
 0x0804c2fc    45  sub_804C2FC                  sub_804C2FC
 0x0804c364    29  sub_804C364                  sub_804C364
 0x0804c3a4    29  sub_804C3A4                  sub_804C3A4
 0x0804c3e4    22  sub_804C3E4                  sub_804C3E4
 0x0804c420    22  sub_804C420                  sub_804C420
 0x0804c45c    43  sub_804C45C                  sub_804C45C
 0x0804c4d8    48  sub_804C4D8                  sub_804C4D8
 0x0804c53c     3  sub_804C53C                  sub_804C53C
 0x0804c548    51  sub_804C548                  sub_804C548
 0x0804c5b8    29  sub_804C5B8                  sub_804C5B8
 0x0804c5f8    29  sub_804C5F8                  sub_804C5F8
 0x0804c638    23  sub_804C638                  sub_804C638
 0x0804c674    23  sub_804C674                  sub_804C674
 0x0804c6b0    46  sub_804C6B0                  sub_804C6B0
```

### M24 [ENGINE/CORE]  0x0804C728–0x0804D0F8  (18 函数 / 1 未匹配 / 249 指令)

工作内存: 0x3000d38, 0x3000be8
ROM 表: 0x8393b28, 0x839d5bc

```
  地址        指令  ll.cfg 名                     函数
 0x0804c728    48  sub_804C728                  sub_804C728
 0x0804c78c    91  sub_804C78C                  sub_804C78C
 0x0804c890    34  sub_804C890                  sub_804C890
 0x0804c8e0   101  sub_804C8E0                  sub_804C8E0
 0x0804c9b4    54  sub_804C9B4                  sub_804C9B4
 0x0804ca2c    49  sub_804CA2C                  sub_804CA2C
 0x0804caa0    49  sub_804CAA0                  sub_804CAA0
 0x0804cb18    49  sub_804CB18                  sub_804CB18
 0x0804cb8c    49  sub_804CB8C                  sub_804CB8C
 0x0804cc00    49  sub_804CC00                  sub_804CC00
 0x0804cc78    49  sub_804CC78                  sub_804CC78
 0x0804ccec    49  sub_804CCEC                  sub_804CCEC
 0x0804cd60    49  sub_804CD60                  sub_804CD60
 0x0804cdd4    49  sub_804CDD4                  sub_804CDD4
 0x0804ce48    49  sub_804CE48                  sub_804CE48
 0x0804cebc    16  sub_804CEBC                  sub_804CEBC
*0x0804cee0   249  sub_804CEE0                  sub_804CEE0
 0x0804d0f8    89  sub_804D0F8                  sub_804D0F8
```

### M25 [ENGINE/OBJECT]  0x0804D1B4–0x0804D708  (8 函数 / 0 未匹配 / 0 指令)

调度表: 0x0839CD5C(87)
ROM 表: 0x8393b28

```
  地址        指令  ll.cfg 名                     函数
 0x0804d1b4    78  sub_804D1B4                  sub_804D1B4
 0x0804d260    79  sub_804D260                  sub_804D260
 0x0804d310    64  sub_804D310                  sub_804D310
 0x0804d3a0    78  sub_804D3A0                  sub_804D3A0
 0x0804d44c    79  sub_804D44C                  sub_804D44C
 0x0804d4fc    83  sub_804D4FC                  sub_804D4FC
 0x0804d5b4   159  sub_804D5B4                  sub_804D5B4
 0x0804d708    64  sub_804D708                  sub_804D708
```

### M26 [ENGINE/OBJECT]  0x0804D798–0x0804DCD8  (8 函数 / 0 未匹配 / 0 指令)

调度表: 0x0839CD5C(87)
ROM 表: 0x8393b28

```
  地址        指令  ll.cfg 名                     函数
 0x0804d798    74  sub_804D798                  sub_804D798
 0x0804d840    80  sub_804D840                  sub_804D840
 0x0804d8f4   128  sub_804D8F4                  sub_804D8F4
 0x0804da04    82  sub_804DA04                  sub_804DA04
 0x0804dabc    75  sub_804DABC                  sub_804DABC
 0x0804db64    86  sub_804DB64                  sub_804DB64
 0x0804dc24    80  sub_804DC24                  sub_804DC24
 0x0804dcd8    68  sub_804DCD8                  sub_804DCD8
```

### M27 [ENGINE/CORE]  0x0804DD70–0x0804F244  (28 函数 / 1 未匹配 / 374 指令)

锚点: BattleDrops_Roll, BattleFxObjs_Add, BattleFx_GetObjCount, BattleFx_Reset, BattleFx_Update, CheckObjectKindSlot, ItemUseFx_Reset, ItemUseFx_RunConsumable, ItemUseFx_RunWeapon, ItemUseFx_Update
工作内存: 0x3000d88, 0x3000ddc, 0x3004980, 0x3000e04
ROM 表: 0x839cfaa, 0x87ea580, 0x839ce38, 0x839cefc

```
  地址        指令  ll.cfg 名                     函数
 0x0804dd70    12  sub_804DD70                  sub_804DD70
 0x0804dd90    51  sub_804DD90                  sub_804DD90
 0x0804de20    46  sub_804DE20                  sub_804DE20
 0x0804de8c    58  sub_804DE8C                  sub_804DE8C
 0x0804df14    44  sub_804DF14                  sub_804DF14
 0x0804df74    43  sub_804DF74                  sub_804DF74
 0x0804dfd8   129  sub_804DFD8                  sub_804DFD8
 0x0804e0e4   145  ItemUseFx_RunConsumable      ItemUseFx_RunConsumable
*0x0804e2ac   374  ItemUseFx_RunWeapon          ItemUseFx_RunWeapon
 0x0804e6dc    69  sub_804E6DC                  sub_804E6DC
 0x0804e76c    63  sub_804E76C                  sub_804E76C
 0x0804e7ec    49  BattleFxObjs_Add             BattleFxObjs_Add
 0x0804e85c   161  BattleFx_Update              BattleFx_Update
 0x0804e9dc   232  BattleDrops_Roll             BattleDrops_Roll
 0x0804ec04   295  sub_804EC04                  sub_804EC04
 0x0804eec4    23  sub_804EEC4                  sub_804EEC4
 0x0804ef00    33  sub_804EF00                  sub_804EF00
 0x0804ef50    26  sub_804EF50                  sub_804EF50
 0x0804ef90    33  sub_804EF90                  sub_804EF90
 0x0804efdc    54  sub_804EFDC                  sub_804EFDC
 0x0804f050    19  sub_804F050                  sub_804F050
 0x0804f07c     4  ItemUseFx_Reset              ItemUseFx_Reset
 0x0804f088    22  ItemUseFx_Update             ItemUseFx_Update
 0x0804f0b8    40  sub_804F0B8                  sub_804F0B8
 0x0804f10c    50  sub_804F10C                  sub_804F10C
 0x0804f17c    68  sub_804F17C                  sub_804F17C
 0x0804f210    20  BattleFx_Reset               BattleFx_Reset
 0x0804f244     3  BattleFx_GetObjCount         BattleFx_GetObjCount
```

### M28 [SCRIPT]  0x0804F250–0x0805305C  (77 函数 / 7 未匹配 / 3,535 指令)

调度表: 0x0862D434(80)
锚点: BattleDrops_Clear, BgTiles_LoadSet, FlushTileDma, Op_AddPartyMember, Op_AnimSlotPause, Op_AnimSlotResume, Op_BgmFadeIn, Op_BgmFadeOut, Op_BgmPlay, Op_BgmStop
工作内存: 0x2016200, 0x3000e70, 0x2016000, 0x3000f24
ROM 表: 0x862d574, 0x87ed904, 0x87ed6d4, 0x83936a8

```
  地址        指令  ll.cfg 名                     函数
 0x0804f250    19  BattleDrops_Clear            BattleDrops_Clear
*0x0804f280   345  sub_804F280                  sub_804F280
 0x0804f64c    85  Op_CameraPan                 Op_CameraPan
 0x0804f768    67  Op_RemovePartyMember         Op_RemovePartyMember
 0x0804f7f8    98  Op_AddPartyMember            Op_AddPartyMember
 0x0804f8d8    58  Op_ScriptBattle              Op_ScriptBattle
 0x0804f974    62  Op_IfAllFlagsJump            Op_IfAllFlagsJump
 0x0804fa04    62  Op_IfAllFlagsClearJump       Op_IfAllFlagsClearJump
 0x0804fa94    62  Op_IfAnyFlagJump             Op_IfAnyFlagJump
 0x0804fb24   473  Op_SysEffect                 Op_SysEffect
 0x08050014    43  ScriptPump_Run               ScriptPump_Run
 0x0805008c   118  ScriptPump_ServiceFrame      ScriptPump_ServiceFrame
*0x080501b8   295  sub_80501B8                  sub_80501B8
*0x08050434   243  sub_8050434                  sub_8050434
*0x0805063c   100  sub_805063C                  sub_805063C
*0x08050720  1018  sub_8050720                  sub_8050720
 0x080511a0    54  Op_ScriptReturn              Op_ScriptReturn
 0x08051230    57  Op_ScriptStop                Op_ScriptStop
 0x080512c4    80  sub_80512C4                  sub_80512C4
 0x080513a0    58  sub_80513A0                  sub_80513A0
*0x0805144c   574  sub_805144C                  sub_805144C
 0x08051a1c    88  Op_OpenWindow                Op_OpenWindow
 0x08051aec    98  sub_8051AEC                  sub_8051AEC
*0x08051be4   960  sub_8051BE4                  sub_8051BE4
 0x08052574     3  Script_GetFlags              Script_GetFlags
 0x08052580    34  Script_ResetVM               Script_ResetVM
 0x080525e8    65  ScriptSet_Load               ScriptSet_Load
 0x080526a0    48  ScriptPump_JumpToEntry       ScriptPump_JumpToEntry
 0x08052728    15  Script_Abort                 Script_Abort
 0x08052758    14  BgTiles_LoadSet              BgTiles_LoadSet
 0x08052780    18  TileDma_Reset                TileDma_Reset
 0x080527ac    28  sub_80527AC                  sub_80527AC
 0x080527f4     5  TileDma_GetCtx               TileDma_GetCtx
 0x08052808    29  Op_LoadTileGfx               Op_LoadTileGfx
 0x08052858    11  Op_ScriptJump                Op_ScriptJump
 0x08052878    30  Script_Call                  Script_Call
 0x080528c4     1  Op_Nop                       Op_Nop
 0x080528c8    37  Op_DialogSetup               Op_DialogSetup
 0x0805291c    66  Op_CloseWindow               Op_CloseWindow
 0x080529b8    36  Op_WaitFrames                Op_WaitFrames
 0x08052a14    16  Op_BgmPlay                   Op_BgmPlay
 0x08052a38    10  Op_BgmStop                   Op_BgmStop
 0x08052a50    15  Op_BgmVolume                 Op_BgmVolume
 0x08052a70    12  Op_BgmFadeIn                 Op_BgmFadeIn
 0x08052a8c    12  Op_BgmFadeOut                Op_BgmFadeOut
 0x08052aa8    17  Op_SfxPlay                   Op_SfxPlay
 0x08052acc    12  Op_SfxStop                   Op_SfxStop
 0x08052ae8    32  Op_RandomJump                Op_RandomJump
 0x08052b34    30  Op_ScriptCallAlt             Op_ScriptCallAlt
 0x08052b80    15  Op_WaitCharsStop             Op_WaitCharsStop
 0x08052ba0    28  Op_LoadCharaGfx              Op_LoadCharaGfx
 0x08052be0    16  Op_LoadCharaPal              Op_LoadCharaPal
 0x08052c04    15  Op_WaitSpriteLoad            Op_WaitSpriteLoad
 0x08052c24    41  Op_SceneChangeFade           Op_SceneChangeFade
 0x08052c90    28  Op_SceneChangePlain          Op_SceneChangePlain
 0x08052cd0    12  Op_WaitSceneIdle             Op_WaitSceneIdle
 0x08052cf0    32  Op_LoadMap                   Op_LoadMap
 0x08052d4c    26  Op_IfEventFlagJump           Op_IfEventFlagJump
 0x08052d8c    15  Op_SetEventFlag              Op_SetEventFlag
 0x08052dac    15  Op_ClearEventFlag            Op_ClearEventFlag
 0x08052dcc    26  Op_IfSwitchJump              Op_IfSwitchJump
 0x08052e0c    15  Op_SetSwitch                 Op_SetSwitch
 0x08052e2c    15  Op_ClearSwitch               Op_ClearSwitch
 0x08052e4c    11  Op_CameraSnap                Op_CameraSnap
 0x08052e6c     8  Op_CameraFollow              Op_CameraFollow
 0x08052e80    12  Op_WaitCameraPan             Op_WaitCameraPan
 0x08052e9c    17  Op_LoadCutsceneAnim          Op_LoadCutsceneAnim
 0x08052ec0    40  Op_RestartCharaAnim          Op_RestartCharaAnim
 0x08052f20    16  Op_WaitCharaAnim             Op_WaitCharaAnim
 0x08052f44    44  Op_IfPartyMemberJump         Op_IfPartyMemberJump
 0x08052fac    13  Op_LoadAnimSet               Op_LoadAnimSet
 0x08052fc8    12  Op_AnimSlotResume            Op_AnimSlotResume
 0x08052fe4    12  Op_AnimSlotPause             Op_AnimSlotPause
 0x08053000    17  Op_WaitAnimSlotIdle          Op_WaitAnimSlotIdle
 0x08053024    13  Op_MenuLoadAnims             Op_MenuLoadAnims
 0x08053040    12  Op_MenuUnlock                Op_MenuUnlock
 0x0805305c    12  Op_MenuLock                  Op_MenuLock
```

### M29 [SCRIPT]  0x08053078–0x080533B4  (19 函数 / 0 未匹配 / 0 指令)

调度表: 0x0862D434(80)
锚点: Op_ChestOpen, Op_ClearFlagsList, Op_ClearSwitchTail, Op_EquipItem, Op_FullHealParty, Op_GiveTakeItem, Op_IfItemQtyJump, Op_IfMoneyJump, Op_IfSaveFlagJump, Op_IfSaveLoadedJump
工作内存: 0x2016000, 0x2016200, 0x3002604, 0x3004980

```
  地址        指令  ll.cfg 名                     函数
 0x08053078    17  Op_WaitMenuReady             Op_WaitMenuReady
 0x0805309c    10  Op_FullHealParty             Op_FullHealParty
 0x080530b4    14  Op_EquipItem                 Op_EquipItem
 0x080530d4    22  Op_GiveTakeItem              Op_GiveTakeItem
 0x08053104    24  Op_SilverAddSub              Op_SilverAddSub
 0x08053138    20  Op_IfItemQtyJump             Op_IfItemQtyJump
 0x0805316c    12  Op_ChestOpen                 Op_ChestOpen
 0x0805318c    12  Op_SaveUiTrigger             Op_SaveUiTrigger
 0x080531a8    22  Op_IfSaveLoadedJump          Op_IfSaveLoadedJump
 0x080531e4    12  Op_SaveTimerA                Op_SaveTimerA
 0x08053200    12  Op_SaveTimerB                Op_SaveTimerB
 0x0805321c    22  Op_IfSaveFlagJump            Op_IfSaveFlagJump
 0x08053254    12  Op_SaveOp                    Op_SaveOp
 0x08053270    48  Op_SetFlagsList              Op_SetFlagsList
 0x080532dc    48  Op_ClearFlagsList            Op_ClearFlagsList
 0x08053348    10  Op_ClearSwitchTail           Op_ClearSwitchTail
 0x08053360    25  Op_IfMoneyJump               Op_IfMoneyJump
 0x080533a0     8  Op_StartLogoFade             Op_StartLogoFade
 0x080533b4    12  Op_WaitLogoFade              Op_WaitLogoFade
```

### M30 [SCRIPT]  0x080533D4–0x08053884  (18 函数 / 0 未匹配 / 0 指令)

锚点: Bgm_Continue, Bgm_FadeIn, Bgm_FadeOut, Bgm_Play, Bgm_SetVolume, Bgm_Stop, Op_SetCharacterLevel, Sfx_GetLoopFlag, Sfx_Play, Sfx_PlayFade
工作内存: 0x3000f38, 0x3000f42, 0x3000f43, 0x3000f40
ROM 表: 0x87edc80, 0x87ed910

```
  地址        指令  ll.cfg 名                     函数
 0x080533d4    13  Op_SetCharacterLevel         Op_SetCharacterLevel
 0x080533f0    72  SoundMain_Frame              SoundMain_Frame
 0x080534b4   100  SoundTracks_Frame            SoundTracks_Frame
 0x0805359c    22  Sound_Init                   Sound_Init
 0x080535e8     3  Sound_GetFlags               Sound_GetFlags
 0x080535f4     9  Sound_VSyncOff               Sound_VSyncOff
 0x0805360c     9  Sound_VSyncOn                Sound_VSyncOn
 0x08053628    36  Bgm_Play                     Bgm_Play
 0x08053688     6  Bgm_Stop                     Bgm_Stop
 0x0805369c    13  Bgm_SetVolume                Bgm_SetVolume
 0x080536c0    13  Bgm_FadeIn                   Bgm_FadeIn
 0x080536ec    15  Bgm_FadeOut                  Bgm_FadeOut
 0x08053720     6  Bgm_Continue                 Bgm_Continue
 0x08053734     9  Sfx_TrackBusy                Sfx_TrackBusy
 0x0805374c    46  Sfx_Play                     Sfx_Play
 0x080537c0    46  Sfx_PlayFade                 Sfx_PlayFade
 0x08053838    30  Sfx_StopTrack                Sfx_StopTrack
 0x08053884     9  Sfx_GetLoopFlag              Sfx_GetLoopFlag
```
