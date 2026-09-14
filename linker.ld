ENTRY(__start)

gNumMusicPlayers = 5;
gMaxLines = 0x1E;

MEMORY {
    ewram (w!x) : ORIGIN = 0x02000000, LENGTH = 256K
    iwram (w!x) : ORIGIN = 0x03000000, LENGTH = 32K
    rom (rx) : ORIGIN = 0x08000000, LENGTH = 8M
}

gMapNpcSlotGroups = 0x08091948;
gObjSizeTable = 0x08393A24;
gCosTable = 0x0839C26C;   /* float[360] 余弦表 (淡出波) */
gSinTable = 0x0839BCCC;   /* float[360] 正弦表 (淡出波) */
gSkillLearnTable = 0x08093418;
gClassStatCurveTable = 0x080921F0;
gStatGrowthCurveTables = 0x080923D8;
gUnk_08092248 = 0x08092248;
gLevelUpExpTable = 0x08092248;
gMsgPoolMain = 0x080936A0;
gUnk_08095028 = 0x08095028;
gItemNames = 0x08095028;
gFacingEventOffsets = 0x08059794;    /* 朝向矩形偏移表 u16[32] (CheckFacingEvent) */
gUnk_08095828 = 0x08095828;
gCharacterNames = 0x08095828;
gUnk_080987C4 = 0x080987C4;
gMapSceneDescriptors = 0x08088D80;
gMapViewportBoundsTable = 0x08089BC4;
gUnk_0808EA0C = 0x0808EA0C;
gUnk_080BABA0 = 0x080BABA0;
gBgLoadTable = 0x0839D8EC;      /* BgLoadEntry[17]: 战斗背景组表 {pal,map,chunkBase,chunkCount} (sub_8019B98) */
gBgLoadChunks = 0x087ED544;     /* LZ77 图块指针表, 由 BgLoadEntry.chunkBase 索引 (sub_8019B98) */
gUnk_082893EC = 0x082893EC;
gUnk_087E94F8 = 0x087E94F8;    /* = &gUnk_087E94FC[-1]: 特殊事件表基址 (CheckFacingEvent) */
gUnk_087E96B4 = 0x087E96B4;
gChoiceNpcCmdPtrs = 0x087E96C8;    /* 84 项指针表 → Chara_SetCmdPtr 命令流 (至 0x087E9818) (zcode-engine) */
gUnk_087E9AA0 = 0x087E9AA0;
gUnk_087EA020 = 0x087EA020;
gUnk_087EA0FC = 0x087EA0FC;
gUnk_087EA110 = 0x087EA110;
gUnk_087EA124 = 0x087EA124;
gUnk_087EA138 = 0x087EA138;
gUnk_087EA33C = 0x087EA33C;
gUnk_087EA368 = 0x087EA368;
gUnk_087EA38C = 0x087EA38C;
gUnk_087ED5CC = 0x087ED5CC;    /* u32 数据表 (sub_801CBA4 tail 用) */
gUnk_087ED5F8 = 0x087ED5F8;    /* u32 数据表 (sub_801CBA4 tail 用) */
gUnk_087EA394 = 0x087EA394;
gUnk_087EB1F4 = 0x087EB1F4;
gUnk_087EB214 = 0x087EB214;
gUnk_087EB22C = 0x087EB22C;
gUnk_087EB278 = 0x087EB278;
gOptionsMenuLabels = 0x087EB278;
gScriptSetTable = 0x087ED6D4;

D_87EB2A8 = 0x087EB2A8;
gUnk_0839CE7C = 0x0839CE7C;
gUnk_0839B2D4 = 0x0839B2D4;
gUnk_0839B2E0 = 0x0839B2E0;
gUnk_0839B462 = 0x0839B462;
gUnk_0839BB4C = 0x0839BB4C;    /* tile 动画帧表 (sub_804EFDC 用) */
gUnk_0839CFBA = 0x0839CFBA;
gUnk_0839D348 = 0x0839D348;
gUnk_0839CC4C = 0x0839CC4C;
gUnk_0839CC4C_entries = 0x0839CC4C;
gUnk_0839CE38 = 0x0839CE38;
gUnk_0839CEFC = 0x0839CEFC;
gUnk_083987EC = 0x083987EC;
gUnk_083989B0 = 0x083989B0;    /* 菜单/对话图标表 A (sub_8018E34 用) */
gUnk_083989CB = 0x083989CB;    /* 菜单/对话图标表 B (sub_8018E34 用) */
gUnk_083989DC = 0x083989DC;    /* 菜单/对话图标表 C (sub_8018E34 用) */
gUnk_083989FC = 0x083989FC;
gUnk_0839C80C = 0x0839C80C;
gUnk_08393A3C = 0x08393A3C;
gUnk_08393A40 = 0x08393A40;
gUnk_08393A48 = 0x08393A48;    /* u8 成员数×? 坐标表 X (sub_8032948 case25 用) */
gUnk_08393A4D = 0x08393A4D;    /* u8 成员数×? 坐标表 Y (sub_8032948 case25 用) */
gUnk_08393B28_entries = 0x08393B28;   /* 同址别名: u16 视图 (offset 0x10 字段, sub_804D1B4) */
gUnk_08393B28 = 0x08393B28;    /* ObjAnimEntry[] 992×0x14B 战斗对象动画/特效资源表 (code_0.h) */
gUnk_0839D5BC = 0x0839D5BC;
gUnk_0839DBF6 = 0x0839DBF6;
gUnk_083988A8 = 0x083988A8;    /* 24-byte 动画条目表 (sub_801CBA4 用) */
gUnk_0839DF5C = 0x0839DF5C;   /* 强度斜坡 {1,2,3,4,4,3,2,1} (sub_801CF90 渐变 LUT) */
gUnk_0861A5E4 = 0x0861A5E4;
gUnk_0861C664 = 0x0861C664;
gUnk_08619FA4 = 0x08619FA4;
gUnk_0861A004 = 0x0861A004;   /* 数字字体表内空块 tile (表项3): sub_801D568/801D710 前导 tile DMA 源 */
gUnk_0861C744 = 0x0861C744;   /* 战斗对象 5 槽 tile 属性 LUT (sub_801D214 尾调 sub_804C2FC 源) */
gUnk_0861A484 = 0x0861A484;   /* 数字字体表内空块 tile (表项39): sub_801D710 kind!=0 图形路径源 */
gUnk_08619AA4 = 0x08619AA4;
gUnk_08393A74 = 0x08393A74;
gUnk_08393A54 = 0x08393A54;
gUnk_08619FE4 = 0x08619FE4;   /* 数字字体表内表项2: sub_801D710 kind!=0 图形路径源 */
gUnk_08393A30 = 0x08393A30;   /* u8 表 6行×4列, sub_801B570 用 (progress.md 待登记项) */
gUnk_083939F8 = 0x083939F8;
gUnk_087ED394 = 0x087ED394;
gUnk_0861A4A4 = 0x0861A4A4;
gUnk_0809C834 = 0x0809C834;
gUnk_087ED6A8 = 0x087ED6A8;
gUnk_0861AAA4 = 0x0861AAA4;
gUnk_0861C664 = 0x0861C664;
gUnk_0861C764 = 0x0861C764;
gUnk_087EA580 = 0x087EA580;
gUnk_0839DF90 = 0x0839DF90;   /* u8 (x,y) 对表, sub_802A154 用 */
gUnk_0839B2A4 = 0x0839B2A4;
gCharaBaseData = 0x087EA580;
gUnk_0839B2B0 = 0x0839B2B0;   /* 12 字节步长表 (sub_801DB3C 分支 arg2<=2) */
gUnk_087E8430 = 0x087E8430;   /* 精灵图块 LZ77 指针表 (LoadSpriteSheetGfx) */
gUnk_080B9DFC = 0x080B9DFC;   /* 精灵 OBJ 调色板数组 [][32] (LoadSpriteSheetPal) */
gUnk_087EA1A0 = 0x087EA1A0;
gUnk_0862D574 = 0x0862D574;
gSpriteObjPalettes = 0x080B9DFC;
gScriptOpcodeHandlers = 0x0862D434;
gObjPalFadeInSteps = 0x08289B6E;   /* OBJ 调色板 10 帧渐显步进表 (Op_SysEffect subop7, 每步 0x20) */
gObjPalFadeInFinal = 0x083936A8;   /* 渐显末帧 16 色 → OBJ bank10; 亦为 MapScene_Load 场景调色板备选 (0x08393688 表 +0x20) */
gFlashFxPaletteTable = 0x0808B1B4; /* = gMenuEntPaletteFrames 项 0x7C (闪光帧组 16 色视图, Op_SysEffect 用) */
gInvPageItemIds = 0x0839CFAA;   /* 16 项: 菜单页号 -> 道具 id (见 data_805769C.h) */
gUnk_087ED904 = 0x087ED904;
gBattleIntroCmd = 0x08619A60;
gBattleIntroPal = 0x08619430;
gSaveSignature = 0x08098199;   /* 12B "LUNAR1_12_09" 存档签名 */
gUnk_080981E6 = 0x080981E6;    /* u16[] 存档块长度表 {2, 0x5A, 8, 0} */
gUnk_080981EE = 0x080981EE;    /* u8[] 6B/项 出生/传送参数表 {npcSetId, tileX, tileY, facing, moveCmd u16} */
gUnk_08097DB0 = 0x08097DB0;    /* 动画 tile 表 u16[] (palette 0x800) */
gUnk_08097EF0 = 0x08097EF0;    /* 动画 tile 表 u16[] (palette 0x900) */
gUnk_08098030 = 0x08098030;    /* 动画 tile 表 u16[] (palette 0xA00) */
gUnk_0809888B = 0x0809888B;    /* 角色显示参数表 u8[] */
gUnk_080981A5 = 0x080981A5;    /* 数字字形/物品名辅助表 */
gUnk_0830FC04 = 0x0830FC04;    /* 装备名表 (u8[] 0xFF 终止项, 名字绘制源) */
gUnk_087EB1E8 = 0x087EB1E8;    /* 存档块指针表 {&gUnk_03004D48, gSaveTimers, gSaveFlags, ...} */
gDialogPortraitTilemapPtrs = 0x087E9818; /* four dialog portrait tilemap positions */
gUnk_087EB2E0 = 0x087EB2E0;    /* 角色头像 LZ77 指针表 (存档/队伍菜单用) */
gUnk_087EBDF0 = 0x087EBDF0;    /* BG tileset LZ77 指针表[4] → VRAM 0x06008000 + idx*0x1000 */
gUnk_087EBE00 = 0x087EBE00;    /* 精灵/瓦片 LZ77 指针表[9] (sub_801A3C4 按 f_18&0xF 分派) */
gMPlayInfos2 = 0x087EDC80;    /* 音轨 MusicPlayerInfo 指针表[4] (Sfx_* 用) */
gSongHeaderTable = 0x087ED910;    /* 歌曲头指针表 (BGM/SFX 共用, MPlayStart 第二参) */
gUnk_0809E4E4 = 0x0809E4E4;    /* 角色调色板/图形表 [N][32] (sub_8016068 DMA 源) */
gUnk_08098308 = 0x08098308;    /* 数字/字形 tile 属性表 (sub_80161F4 VRAM 填充源) */
gUnk_080936A0 = 0x080936A0;    /* 字节表, sub_8018750 扫描 0xFF 计数 */
gUnk_08089B90 = 0x08089B90;
gUnk_0808B7D4 = 0x0808B7D4;
gUnk_08095A1C = 0x08095A1C;
gUnk_08095C94 = 0x08095C94;
gUnk_08095F14 = 0x08095F14;
gUnk_080963C0 = 0x080963C0;
gUnk_08096B2C = 0x08096B2C;
gUnk_080972C0 = 0x080972C0;
gUnk_08097344 = 0x08097344;
gUnk_080973A8 = 0x080973A8;
gUnk_08097514 = 0x08097514;
gUnk_08097B60 = 0x08097B60;
gUnk_08098630 = 0x08098630;
gUnk_0809863B = 0x0809863B;
gUnk_0809C834 = 0x0809C834;
gUnk_0809E644 = 0x0809E644;
gUnk_080A0B58 = 0x080A0B58;
gUnk_080A12D0 = 0x080A12D0;
gUnk_083937E8 = 0x083937E8;

SECTIONS {
    ewram (NOLOAD) : ALIGN(4) {
        . = 0x00000000; gUnk_02000000 = .;
        . = 0x00004000; gUnk_02004000 = .;
        . = 0x00005000; gUnk_02005000 = .;
        . = 0x005058; gUnk_02005058 = .; /* 菜单 tile 写入缓冲 */
        . = 0x00005380; gUnk_02005380 = .;
        . = 0x0053A8; gUnk_020053A8 = .; /* 数字绘制 dest (副菜单) */
        . = 0x0053B4; gUnk_020053B4 = .; /* 数字绘制 dest (主菜单) */
        . = 0x00005800; gWindowBgBuf = .;
        . = 0x00005C44; gMsgLineBuf = .;   /* u16[29] Msg_RenderLine 输出行缓冲 */
        . = 0x005C82; gUnk_02005C82 = .; /* 选项菜单文字源基址 (Msg_DrawPoolSegment dest, zcode-engine) */
        . = 0x005C84; gUnk_02005C84 = .; /* 选项菜单 u16[16] 瓦片缓冲 A (zcode-engine) */
        . = 0x005CC4; gUnk_02005CC4 = .; /* 选项菜单 u16[16] 瓦片缓冲 B (zcode-engine) */
        . = 0x00016000; gUnk_02016000 = .;
        . = 0x00016200; gUnk_02016200 = .;
        . = 0x020000; gCutsceneGfxBuf = .;
        . = 0x000352C0; gUnk_020352C0 = .;
        . = 0x00035B04; gUnk_02035B04 = .;
        . = 0x00036EC0; gUnk_02036EC0 = .;
        . = 0x0003EE00; gCharaWalkCmdBuf = .;
        . = 0x03F000; gSpritePoolBackup = .;
        . = 0x03FE00; gQueue34C0Backup = .;
        . = 0x00040000;
    } >ewram

    iwram (NOLOAD) : ALIGN(4) {
        . = 0x00000000; gUnk_03000000 = .;
        . = 0x00000002; gUnk_03000002 = .;
        . = 0x00000004; gBlendFadeStep = .;
        . = 0x000008; gLogoAnimDirection = .;
        . = 0x00000A; gLogoSpriteNodes = .;
        . = 0x00000C; gLogoAnimTimer = .;

        . = 0x00000010; gMenuEntAnimFlags = .;
        . = 0x00000014; gMenuEntAnimThreshold = .;
        . = 0x00000018; gMenuEntAnimShift = .;
        . = 0x00000020; gMenuEntAnimCounter = .;
        . = 0x00000028; gMenuEntPalDest = .;
        . = 0x00000038; gMenuEntAnimFrameTbl = .;
        . = 0x00000048; gMenuCursorSprite = .;

        . = 0x00000058; gUiSprites = .;
        . = 0x000000BC; gUiSpritesAux = .;
        . = 0x00000184; gUnk_03000184 = .;
        . = 0x00000185; gUnk_03000185 = .;

        . = 0x00000186; gMenuCursorGrp = .;

        . = 0x00000187; gMenuCursorSel = .;
        . = 0x00000188; gMenuCursorStack = .;
        . = 0x00000198; gUnk_03000198 = .;
        . = 0x00000199; gUnk_03000199 = .;
        . = 0x00000199; gInvCursor2 = .;

        . = 0x000001A0; gUnk_030001A0 = .;
        . = 0x000001A0; gInvViewState = .;
        . = 0x000001AA; gInvPageUpItems = .;
        . = 0x000001AC; gInvPageDownItems = .;
        . = 0x000001AE; gUnk_030001AE = .;
        . = 0x000001AF; gUnk_030001AF = .;
        . = 0x000001B0; gUnk_030001B0 = .;
        . = 0x000001B1; gUnk_030001B1 = .;


        . = 0x000001B4; gItemUseCtx = .;
        . = 0x000001B9; gItemPagePrevId = .;
        . = 0x000001BA; gItemPageNextId = .;
        . = 0x000001BC; gStatArrowIds = .;
        . = 0x000001C3; gItemUseId = .;
        . = 0x000001C4; gItemUseMpCost = .;
        . = 0x000001C5; gItemUseEffectType = .;
        . = 0x000001C6; gItemUseHealHp = .;

        . = 0x000001C8; gMenuResultCode = .;


        . = 0x000001D0; gMsgTable = .;

        . = 0x000001F0; gScreenIdleIconIds = .;
        . = 0x00000200; gScreenIdleIconCursor = .;
        . = 0x00000204; gUnk_03000204 = .;
        . = 0x00000208; gUnk_03000208 = .;
        . = 0x00000210; gUnk_03000210 = .;

        . = 0x00000220; gSaveCurSlot = .;
        . = 0x00000221; gSaveModeFlag = .;
        . = 0x00000224; gSkillMenuTmpA = .;
        . = 0x00000225; gSkillMenuTmpB = .;
        . = 0x00000226; gPartyMenuIdx = .;
        . = 0x00000227; gSkillMenuPage = .;
        . = 0x00000229; gMenuItemId = .;
        . = 0x0000022A; gItemPocketIdx = .;
        . = 0x0000022B; gInvUiMode = .;
        . = 0x0000022C; gMenuItemIcon = .;
        . = 0x00000230; gCardAlbumCursor = .;
        . = 0x00000230; gSoundTestSfxId = .;
        . = 0x00000232; gCardAlbumPage = .;
        . = 0x00000232; gSoundTestBgmId = .;
        . = 0x00000233; gCardCursorY = .;
        . = 0x00000233; gSoundTestPlayingBgmIdPlusOne = .;
        . = 0x00000234; gTitleFadeStep = .;
        . = 0x00000234; gTitleAttractCountdown = .;
        . = 0x00000236; gCardRecvId = .;
        . = 0x00000238; gCardSendId = .;
        . = 0x00000240; gUnk_03000240 = .;
        . = 0x00000244; gObjPoolPtr = .;
        . = 0x00000248; gUnk_03000248 = .;
        . = 0x00000310; gKeysHeld = .;
        . = 0x00000312; gGstate312 = .;

        . = 0x00000314; gGstate314 = .;
        . = 0x00000316; gKeyIgnoreTimer = .;
        . = 0x00000317; gUnk_03000317 = .;
        . = 0x00000318; gUnk_03000318 = .;
        . = 0x00000324; gGstate324 = .;
        . = 0x00000328; gBattleRngSeed = .;     
        . = 0x0000032C; gUnk_0300032C = .;
        . = 0x0000032E; gGstate32E = .;
        . = 0x00000330; gGstate330 = .;
        . = 0x00000340; gGstate340 = .;
        . = 0x00000344; gUnk_03000344 = .;  /* VBlank 战斗流水线结果暂存 (sub_8018070 写) */
        . = 0x00000348; gDialogCtx = .;
        . = 0x00000384; gFlashFlags = .;   
        . = 0x00000386; gWaveAngle = .;
        . = 0x00000390; gWaveRowOffset = .;
        . = 0x000004D0; gWaveTablePtr = .;
        . = 0x000004D4; gWaveAngleVel = .;
        . = 0x000004D5; gWaveAmp = .;
        . = 0x000004D6; gWaveRowStep = .;
        . = 0x000004D7; gWaveMode = .; 
        . = 0x000004D8; gWaveBgHofsTbl = .;
        . = 0x000004E8; gWaveBgVofsTbl = .;
        . = 0x000004F8; gBgLoadSlot = .;  
        . = 0x00000500; gBgScrollBackup = .;
        . = 0x00000510; gBattleUiFlags = .;
        . = 0x00000512; gUnk_03000512 = .;
        . = 0x00000513; gBgLoadChunkIdx = .;
        . = 0x00000514; gUnk_03000514 = .;
        . = 0x00000518; gObjFlagsA = .;
        . = 0x00000598; gObjFlagsB = .;
        . = 0x00000618; gUnk_03000618 = .;
        . = 0x0000061A; gUnk_0300061A = .;
        . = 0x0000061C; gUnk_0300061C = .;
        . = 0x0000061E; gUnk_0300061E = .;
        . = 0x00000620; gUnk_03000620 = .;
        . = 0x00000622; gUnk_03000622 = .;
        . = 0x00000624; gUnk_03000624 = .;
        . = 0x0000062C; gUnk_0300062C = .;
        . = 0x00000630; gUnk_03000630 = .;
        . = 0x00000638; gUnk_03000638 = .;
        . = 0x00000668; gUnk_03000668 = .;
        . = 0x00000669; gUnk_03000669 = .;
        . = 0x00000670; gUnk_03000670 = .;
        . = 0x0000068C; gDmgPopupSlot = .;
        . = 0x0000068D; gDmgPopupPhase = .;
        . = 0x0000068E; gDmgPopupLevel = .;
        . = 0x00000690; gTaskPoolHead = .;   /* 待选池链表头 (UnkNode 哨兵, ListNode_Init) */
        . = 0x000006A0; gTaskPoolNodes = .;
        . = 0x000006F0; gTaskPoolCount = .;   /* 待选池计数 (u8, sub_801DC20++/sub_801DD04--) */
        . = 0x000006F8; gFxQueueObjs = .;
        . = 0x00000714; gFxQueueWriteIdx = .;
        . = 0x00000715; gFxQueueReadIdx = .;
        . = 0x00000716; gUnk_03000716 = .;
        . = 0x00000718; gUnk_03000718 = .;
        . = 0x0000071C; gUnk_0300071C = .;
        . = 0x00000730; gUnk_03000730 = .; gUnk_03000730_arr = .;
        . = 0x0000073C; gUnk_0300073C = .;
        . = 0x0000073D; gUnk_0300073D = .;
        . = 0x00000744; gUnk_03000744 = .;
        . = 0x00000748; gUnk_03000748 = .;
        . = 0x00000758; gUnk_03000758 = .;
        . = 0x00000763; gUnk_03000763 = .;
        . = 0x00000765; gUnk_03000765 = .;
        . = 0x00000768; gUnk_03000768 = .;
        . = 0x00000769; gUnk_03000769 = .;
        . = 0x0000076A; gUnk_0300076A = .;
        . = 0x0000076B; gUnk_0300076B = .;
        . = 0x0000076C; gUnk_0300076C = .;
        . = 0x0000076E; gUnk_0300076E = .;
        . = 0x00000770; gMenuListCount = .;
        . = 0x00000778; gMenuListItems = .;
        . = 0x00000781; gMenuListTop = .;
        . = 0x00000782; gMenuListCursor = .;
        . = 0x00000784; gMenuListRowBits = .;
        . = 0x000788; gMenuSlotStates = .;
        . = 0x0007BA; gMenuMasterCursor = .;
        . = 0x00000808; gMenuList2Count = .;
        . = 0x00000809; gMenuList2Top = .;
        . = 0x0000080A; gMenuList2Cursor = .;
        . = 0x0000080C; gMenuObjLoadSlots = .;
        . = 0x00000811; gMenuObjLoadCount = .;
        . = 0x00000812; gMenuObjLoadIdx = .;
        . = 0x00000813; gMenuObjLoadPhase = .;
        . = 0x00000814; gMenuSelSlot0 = .;
        . = 0x00000815; gMenuSelSlot1 = .;
        . = 0x00000816; gMenuWindowPhase = .;
        . = 0x00000818; gMenuWindowFlags = .;
        . = 0x00000820; gObjActStep = .;
        . = 0x00000822; gObjActSavedF2A = .;
        . = 0x00000824; gObjActSavedPal = .;
        . = 0x00000825; gObjActStepTimer = .;
        . = 0x00000826; gObjActResult = .;
        . = 0x00000828; gObjActSavedX = .;
        . = 0x00000829; gObjActSavedY = .;
        . = 0x00000830; gTargetSlotList = .;
        . = 0x0000083C; gTargetSlotCount = .;
        . = 0x0000083D; gObjActGroupCount = .;
        . = 0x00000840; gObjActGroupSlots = .;
        . = 0x00000844; gActWaitBusy0 = .;
        . = 0x00000845; gActWaitBusy1 = .;
        . = 0x00000856; gActWaitBusy2 = .;
        . = 0x00000857; gActWaitCnt0 = .;
        . = 0x00000858; gActEventCount = .;
        . = 0x0000085A; gActWaitFrames = .;
        . = 0x0000085C; gActWaitCnt1 = .;
        . = 0x00000865; gObjActDoneCount = .;
        . = 0x00000867; gSceneFadeOut = .;
        . = 0x00000868; gSceneFadeIn = .;
        . = 0x0000086A; gObjActBranch = .;
        . = 0x0000086B; gObjActParam = .;
        . = 0x0000086E; gObjActMoveFromX = .;
        . = 0x0000086F; gObjActMoveFromY = .;
        . = 0x00000882; gActHitDmgAmount = .;
        . = 0x00000884; gObjActSfxLatch = .;
        . = 0x00000886; gActWaitSfxId = .;
        . = 0x00000888; gActWaitSfxParam = .;
        . = 0x00000889; gSceneTransStep = .;
        . = 0x0000088B; gUnk_0300088B = .;
        . = 0x0000088C; gUnk_0300088C = .;
        . = 0x00000890; gUnk_03000890 = .;
        . = 0x00000898; gUnk_03000898 = .;
        . = 0x0000089A; gUnk_0300089A = .;
        . = 0x0000089C; gUnk_0300089C = .;
        . = 0x000008A0; gUnk_030008A0 = .;
        . = 0x000008A4; gUnk_030008A4 = .;
        . = 0x000008A5; gUnk_030008A5 = .;
        . = 0x000008EC; gStatRecalcPool = .;
        . = 0x000008F0; gStatRecalcKind = .;
        . = 0x000008F1; gFxReqTimer = .;
        . = 0x000008F2; gFxReqKind = .;
        . = 0x000008F3; gFxReqFrames = .;
        . = 0x00000906; gFxReqAnimIdx = .;
        . = 0x00000908; gSkillHealAmount = .;
        . = 0x00000910; gBattleIntroState = .;
        . = 0x00000911; gBattleIntroTimer = .;
        . = 0x00000918; gBattleIntroObj = .;
        . = 0x00000948; gBattleIntroPhase = .;
        . = 0x00000949; gBattleDlgObjSlot = .;
        . = 0x0000094A; gBattleDlgCellIdx = .;
        . = 0x0000094B; gBattleDlgTileCol = .;
        . = 0x0000094C; gBattleDlgDigitCol = .;
        . = 0x0000094D; gBattleDlgAnimFrame = .;
        . = 0x00000950; gBattleDlgSegOff = .;
        . = 0x00000954; gBattleDlgNextState = .;
        . = 0x00000956; gBattleDlgShowVal = .;
        . = 0x00000958; gBattleDlgLvUpMask = .;
        . = 0x0000095A; gBattleDlgShowTarget = .;
        . = 0x00000960; gBattleDlgLearnList = .;
        . = 0x00000968; gBattleDlgLearnCount = .;
        . = 0x00000969; gBattleDlgLearnIdx = .;
        . = 0x0000096C; gBattleDlgLearnGfx = .;
        . = 0x00000970; gBattleDlgPool = .;
        . = 0x00000974; gBattleDlgPartySlots = .;
        . = 0x00000979; gBattleDlgPartyCount = .;
        . = 0x0000097A; gBattleDlgPartyIdx = .;
        . = 0x0000097B; gBattleDlgFlashState = .;
        . = 0x0000097C; gBattleDlgFlashTimer = .;
        . = 0x0000097D; gBattleDlgFlashPal = .;
        . = 0x0000097E; gBattleIntroFadeFlag = .;
        . = 0x000009BE; gResultsDropCount = .;
        . = 0x000009BF; gResultsDropSelIdx = .;
        . = 0x000009C0; gResultsDropTablePtr = .;
        . = 0x000009C4; gResultsStepDone = .;
        . = 0x000009C5; gResultsViewKind = .;
        . = 0x000009C8; gResultsStatePtr = .;
        . = 0x000009D0; gWipeDesc = .;
        . = 0x000009D8; gWipeSavedH = .;
        . = 0x00000AD8; gWipeSubframe = .;
        . = 0x00000AD9; gWipeOamStart = .;
        . = 0x00000ADA; gWipeOamEnd = .;
        . = 0x00000ADB; gWipeMinY = .;
        . = 0x00000ADC; gWipeMaxY = .;
        . = 0x00000ADD; gWipeProgress = .;
        . = 0x00000ADE; gWipeCtl = .;

        . = 0x00000AE0; gObjPalSlotUsed = .;

        . = 0x00000AE2; gBgPalSlotUsed = .;
        . = 0x00000AE4; gUnk_03000AE4 = .;
        . = 0x00000AE5; gUnk_03000AE5 = .;
        . = 0x00000AE8; gObjPalAnim = .;
        . = 0x00000BE8; gBgPalAnim = .;
        . = 0x00000CE8; gUnk_03000CE8 = .;
        
        . = 0x00000D38; gObjTargetCache = .;
        . = 0x00000D48; gInvPendingApply = .;
        . = 0x00000D88; gInvPageDeltas = .;
        . = 0x00000DC8; gObjInvBackup = .;
        . = 0x00000DDC; gInvPageDeltaCount = .;
        . = 0x00000DDD; gInvPendingApplyCount = .;
        . = 0x000DDE; gItemUseFxState = .; gItemUseFx = .; /* ItemUseFx 结构体基址视图 */
        . = 0x000DE0; gItemUseFxLoopFrom = .;
        . = 0x000DE2; gItemUseFxLoopTo = .;
        . = 0x000DE4; gItemUseFxLoopCount = .;
        . = 0x000DE5; gItemUseFxLoopsDone = .;
        . = 0x000DE6; gItemUseFxSavedF2A = .;
        . = 0x000DE8; gItemUseFxSavedPalSlot = .;
        . = 0x000DF0; gBattleFxObjs = .; gBattleFx = .; /* BattleFx 结构体基址视图 */
        . = 0x000E04; gBattleFxObjCount = .;
        . = 0x000E05; gBattleFxState = .;
        . = 0x000E08; gBattleDrops = .; gBattleDropsBlk = .; /* BattleDrops 结构体基址视图 */
        . = 0x000E30; gBattleDropCount = .;
        . = 0x00000E68; gScriptReturnSetId = .;
        . = 0x00000E69; gScriptPendingEntry = .;
        . = 0x00000E6C; gScriptCursor = .;
        . = 0x000E70; gScriptVmFlags = .;
        . = 0x000E72; gScriptDialogPhase = .;
        . = 0x000E73; gDialogTimer = .;
        . = 0x00000E74; gUnk_03000E74 = .;
        . = 0x000E76; gDialogTextCursor = .;
        . = 0x000E78; gScriptCallStackDepth = .;
        . = 0x000E80; gScriptCallStack = .;
        . = 0x00000EA0; gScriptStreamCursorStack = .;
        . = 0x00000EC0; gScriptStreamSetIdStack = .;
        . = 0x00000EC8; gScriptStreamEntry = .;
        . = 0x00000EC9; gScriptStreamSetId = .;
        . = 0x00000ECA; gScriptStreamDepth = .;
        . = 0x000ECB; gDialogWindowTileX = .;
        . = 0x000ECC; gDialogWindowTileY = .;
        . = 0x000ECD; gDialogTextX = .;
        . = 0x000ECE; gDialogTextY = .;
        . = 0x00000ED8; gScriptLocalSlots = .; /* 脚本 VM 8 个 u16 局部槽 (原 gUnk_03000ED8 视图废弃) */
        . = 0x00000EE8; gTileDmaAllocTable = .;
        . = 0x00000F24; gTileDmaCount = .;
        . = 0x00000F26; gTileDmaLastBlockIdx = .;
        . = 0x00000F28; gTextDrawPos = .;
        . = 0x00000F2A; gTileAnimFrameIdx = .;
        . = 0x00000F2B; gTileAnimCharIdx = .;
        . = 0x00000F2C; gScriptKeyState = .;
        . = 0x00000F2E; gScriptKeysPressed = .;
        . = 0x00000F30; gScriptCurSetId = .;
        . = 0x00000F38; gSoundTaskFlags = .;
        . = 0x00000F3A; gPlayingSongId = .;
        . = 0x00000F3C; gBgmVolume = .;
        . = 0x00000F3E; gFadeFromVolume = .;
        . = 0x00000F40; gFadeDuration = .;
        . = 0x00000F41; gFadeCounter = .;
        . = 0x00000F42; gSfxTrackActiveBits = .;
        . = 0x00000F43; gSfxTrackLoopBits = .;
        . = 0x00000F48; gSfxTrackSongIds = .;
        . = 0x00000F50; gSfxTrackFadeBits = .;
        . = 0x00000F58; gMPlayTrack_0 = .;
        . = 0x00001408; gMPlayTrack_1 = .;
        . = 0x00001458; gMPlayTrack_2 = .;
        . = 0x000014A8; gMPlayTrack_3 = .;
        . = 0x000014F8; gMPlayTrack_4 = .;

        . = 0x00001544;
        . = ALIGN(4);
        *libgcc.a:dp-bit.o(.bss);
        *libgcc.a:fp-bit.o(.bss);
        . = ALIGN(4); src/m4a.o(.bss*);
        
        . = 0x000018F0; gSwitchFlags = .;
        . = 0x00001940; gSioRecvWord = .;
        . = 0x00001944; gMainGameState = .;
        . = 0x00001944; gGameState = .;
        . = 0x00001948; gGameTimer = .;
        . = 0x00001950; gUnk_03001950 = .;
        . = 0x00001988; gHBlankScrollCounter = .;
        . = 0x00001990; gLzContext = .;
        . = 0x000019B0; gHeldKeysRaw = .;
        . = 0x000019C0; gHBlankWaveV = .;
        . = 0x00001AC0; gMainTaskSlot = .;
        . = 0x00001AC0; gMainLoopMode = .;
        . = 0x00001AD0; gUnk_03001AD0 = .;    
        . = 0x00001AD0; gSioRecvPacket = .;
        . = 0x00001B50; gSioLinkState = .;
        . = 0x00001B54; gHBlankWaveRow = .;
        . = 0x00001B60; gHBlankWaveH = .;
        . = 0x00001C60; gEventFlags = .;
        . = 0x00001CA0; gNewKeysRaw = .;
        . = 0x00001CB0; gUnk_03001CB0 = .;
        . = 0x00001CB0; gSioSendPacket = .;
        . = 0x00001D30; gSioRetryTimer = .;
        . = 0x00001D80; gIntrMainBuf = .;

        . = 0x00001EE0; gUnk_03001EE0 = .;
        . = 0x0000259C; gVBlankPipelineMode = .;
        . = 0x000025A0; gFrameCounter = .;
        . = 0x000025A4; gRandCursor = .;
        . = 0x000025A8; gCardExchangeStatus = .;
        . = 0x000025B0; gPlayerMoveDir = .;

        . = 0x000025B4; gCameraPosX = .;
        . = 0x000025B8; gUnk_030025B8 = .;
        . = 0x000025BC; gCurSpriteW = .;
        . = 0x000025C0; gFollowerHistX = .;
        . = 0x000025D4; gEncounterCounter = .;
        . = 0x000025D8; gDialogueActive = .;
        . = 0x000025E0; gFollowerHistY = .;
        . = 0x000025F0; gCutsceneActive = .;
        . = 0x000025F0; gTitleIntroState = .;
        . = 0x000025F4; gSceneEntryFlag = .;
        . = 0x000025F8; gCameraTargetX = .;
        . = 0x000025FC; gCameraPosY = .;
        . = 0x00002600; gScenePhase = .;
        . = 0x00002604; gLogoEffectState = .;
        . = 0x00002608; gTitleFadeTimer = .;
        . = 0x0000260C; gWarpAnimState = .;
        . = 0x00002C30; gCurSpriteH = .;
        . = 0x00002C34; gBattleResultType = .;
        . = 0x00002C38; gSilverAmount = .;
        . = 0x00002C3C; gCameraTargetY = .;
        . = 0x00002C40; gUnk_03002C40 = .;
        . = 0x00002C44; gUnk_03002C44 = .;  /* 同址别名: gPartyFollowFlags 视图 */
        . = 0x00002C44; gPartyFollowFlags = .;
        . = 0x00002C48; gAfterBattleCounter = .;
        . = 0x00002C4C; gSceneLoadToggle = .;
        . = 0x00002C50; gInputLockFrames = .;
        . = 0x00002C58; gFollowerHistDir = .;
        . = 0x00002C60; gCutsceneAnimFlags = .;
        . = 0x00002C80; gSpriteRenderQueue = .;

        . = 0x00002E80; gActors = .;              /* Actor[24] —— 场上实体主数组 */
        . = 0x00003150; gEffectActor = .;         /* == gActors[18] 别名 */
        . = 0x00003178; gDialogArrowActors = .;   /* == &gActors[19] 别名 */       
   
        . = 0x00003240; gPendingPalId = .;
        . = 0x00003250; gVramBufferPointers = .;
        
        . = 0x000032D0; gPendingSpriteLoad = .;
        . = 0x000032D4; gPendingGfxId = .;
        . = 0x000032E0; gCutsceneAnimPals = .;

        . = 0x00003360; gVramTransferCounts = .;
        . = 0x00003380; gPalTransferQueue = .;
        . = 0x00003480; gPendingCharaSwitch = .;
        . = 0x00003490; gCutsceneAnimSlots = .;
        . = 0x000034B0; gPendingPalSlot = .; 
        . = 0x000034C0; gOamAffineBuf = .;     
        . = 0x000035C0; gOamBuffer = .;
        . = 0x000039C0; gVramTransferQueue = .;
        
        . = 0x00003AC0; gSpriteNodePool = .;

        . = 0x000044C0; gCutsceneAnimScripts = .;
        . = 0x00004540; gPendingGfxSlot = .;

        . = 0x00004550; gBlendCoefficients = .;
        . = 0x00004560; gWin0HWaveTable = .;
        . = 0x004604; gWindowTransitionProgress = .;
        . = 0x00004608; gSceneTransitionArg = .;
        . = 0x0000460C; gCameraDrawMode = .;
        . = 0x00004610; gHBlankEffectMode = .;
        . = 0x00004614; gMoveCmdSetId = .;
        . = 0x00004618; gUnk_03004618 = .;
        . = 0x00461C; gCameraPanStartY = .; /* 摄像机平移起始 Y (Op_CameraPan) */
        . = 0x00004624; gScreenIdleEventFlags = .;
        . = 0x00004628; gSceneBlendMode = .;
        . = 0x0000462C; gChoiceListPtr = .;
        . = 0x004630; gCameraPanTargetX = .; /* 摄像机平移目标 X (Op_CameraPan) */
        . = 0x00004634; gMapScriptSetId = .;
        . = 0x00004638; gSpawnTileY = .;
        . = 0x0000463C; gMapZoneType = .;
        . = 0x00004640; gChoiceListLen = .;
        . = 0x00004644; gZoneCheckTileYs = .;
        . = 0x00004648; gBG3ScrollY = .;

        . = 0x0000464C; gCameraMinY = .;
        . = 0x0000464C; gUnk_0300464C = .;
        . = 0x00004650; gCameraMinX = .;
        . = 0x00004650; gUnk_03004650 = .;
        . = 0x00004654; gMapZoneEntryIdx = .;
        . = 0x00004658; gBlendControl = .;
        . = 0x0000465C; gScreenFadeFlags = .;
        . = 0x00004660; gIntroBgTileSetIndex = .;
        . = 0x00004668; gWindowTransitionProgressSnapshot = .;
        . = 0x0000466C; gChoiceCursor = .;
        . = 0x00004670; gSlotGfxId = .;

        . = 0x0000467C; gObjGraphicsSetId = .;
        . = 0x00004680; gDrawCamEaseActive = .;
        . = 0x00004684; gDrawCamY = .;
        . = 0x00004688; gUnk_03004688 = .;
        . = 0x0000468C; gSpawnTileX = .;
        . = 0x00004694; gPendingPortraitPalette = .;
        . = 0x00004698; gBg1ScrollMode = .;
        . = 0x0000469C; gChoiceSel = .;
        . = 0x0046A0; gAnimSlots = .;
        . = 0x000047A0; gMapZoneHeader = .;
        . = 0x000047A4; gSpawnFacingDir = .;
        . = 0x000047A8; gScreenFadeProgress = .;
        . = 0x000047AC; gUnk_030047AC = .;
        . = 0x000047B0; gCurrentMapId = .;
        . = 0x0047B4; gCameraPanDuration = .; /* 摄像机平移总帧数 (0=未平移) */
        . = 0x000047B8; gChoiceDestY = .;
        . = 0x000047BC; gChoiceGroupIdx = .;
        . = 0x000047C0; gDrawCamX = .;
        . = 0x000047C4; gMapWidthPx = .;
        . = 0x000047C4; gUnk_030047C4 = .;
        . = 0x000047C8; gIntroBgTransferStage = .;
        . = 0x000047CC; gPendingPortraitGfx = .;
        . = 0x000047D0; gSlotPalId = .;
        . = 0x0047DC; gCameraPanStartX = .; /* 摄像机平移起始 X (Op_CameraPan) */
        . = 0x000047E0; gChoiceSubIdx = .;
        . = 0x000047E4; gMapNpcSetId = .;
        . = 0x000047EC; gMapHeightPx = .;
        . = 0x000047EC; gUnk_030047EC = .;
        . = 0x000047F0; gScreenFadeStep = .;
        . = 0x000047F4; gPaletteFxPhase = .;
        . = 0x00004800; gViewportFlags = .;
        . = 0x00004820; gEncounterEnabled = .;
        . = 0x00004824; gChoiceDestX = .;
        . = 0x00004828; gBG2ScrollY = .;
        . = 0x004830; gCameraPanTargetY = .; /* 摄像机平移目标 Y (Op_CameraPan) */
        . = 0x00004834; gScreenFadeParam = .;
        . = 0x00004838; gZoneCheckTileXs = .;
        . = 0x0000483C; gPendingPortraitSlot = .;
        . = 0x00004840; gSceneSubState = .;
        . = 0x00004840; gScreenTransitionState = .;
        . = 0x004844; gCameraPanStep = .; /* 摄像机平移当前步数 */

        . = 0x00004848; gBG2ScrollX = .;
        . = 0x0000484C; gBG3ScrollX = .;
        . = 0x00004850; gEnvScriptSetId = .;

        . = 0x00004854; gCameraSnapFlag = .;
        . = 0x00004860; gUnk_03004860 = .;
        . = 0x00004870; gChestFlags = .;
        . = 0x00004890; gChestObjects = .;
        . = 0x00004910; gPaletteFxMode = .;
        . = 0x00004914; gPaletteFxPending = .;
        . = 0x00004918; gPaletteFxTimer = .;
        . = 0x00004920; gMapObjGfxSetId = .;
        . = 0x00004930; gStaticMapObjects = .;
        . = 0x0000496C; gUnk_0300496C = .;
        . = 0x00004970; gUnk_03004970 = .;
        
        . = 0x00004980; gInventory = .;
        . = 0x00004A80; gEquipBonusDef = .;
        . = 0x00004A88; gBattleFormationIds = .;
        . = 0x00004A90; gEquipBonusAtkBase = .;
        . = 0x00004A94; gEquipBonusRes = .;
        . = 0x00004A98; gEquipBonusMen = .;
        . = 0x00004AA0; gPartyMemberIds = .;
        . = 0x00004AA8; gEquipBonusDef2 = .;
        . = 0x00004AAC; gEquipBonusAtk = .;
        . = 0x00004AB0; gEquipBonusAgl = .;
        . = 0x00004AB4; gEquipBonusNoa = .;
        . = 0x00004AB8; gEquipBonusLuc = .;
        . = 0x00004AC0; gPartyStats = .;
        . = 0x00004D40; gUnk_03004D40 = .;
        . = 0x00004D44; gSaveFsmState = .;
        . = 0x00004D48; gUnk_03004D48 = .;
        . = 0x00004D4C; gUnk_03004D4C = .;
        . = 0x00004D50; gActiveSaveSlot = .;
        . = 0x00004D60; gSaveTimers = .;

        . = 0x00004DBC; gUnk_03004DBC = .;
        . = 0x00004DC0; gBgTileReloadFlag = .; gUnk_03004DC0 = .; /* == gBgTileReloadFlag (别名视图) */
        . = 0x00004DC4; gSaveBusyA = .;
        . = 0x00004DC8; gSaveFlags = .;

        . = 0x00004DD0; gSaveSramBlock = .;
        . = 0x00004DD4; gSaveUiParam = .;
        . = 0x00004DD8; gSaveBusyB = .;
        . = 0x00004DDC; gSavedDispCnt = .;   /* 存档菜单进入前的 REG_DISPCNT 备份 */
        . = 0x00004DE0; gSavedBldCnt = .;    /* 同上, REG_BLDCNT 备份 */
        . = 0x00004DE4; gUnk_03004DE4 = .;
        . = 0x00004DE8; gUnk_03004DE8 = .;
        . = 0x00004DF0; gSioState = .;
        . = 0x00004DF0; gSioCommState = .;
        . = 0x00004E48; gUnk_03004E48 = .;
        . = 0x00004F20; gSioSession = .;
        . = 0x00004F80; gSioXferCtx = .;
        . = 0x00004F90; gObjSlotFxCmd = .;
        . = 0x00004FA0; src/m4a.o(common_data);
        . = 0x00007FF8; gUnk_03007FF8 = .;
        . = 0x00008000;
    } >iwram

    .text : ALIGN(4) {
        asm/crt0.o(.text);
        src/engine_core.o(.text);
        src/scene_mgr.o(.text);
        src/sprite_engine.o(.text);
        src/vram_transfer.o(.text);
        src/map_view.o(.text);
        src/anim_slot.o(.text);
        src/player_stats.o(.text);
        src/menu_ui.o(.text);
        src/save.o(.text);
        src/menu.o(.text);
        src/text_engine.o(.text);
        src/sio_link.o(.text);
        src/battle_obj_core.o(.text);
        src/scene_obj_fx.o(.text);
        src/event_actor.o(.text);
        src/scene_interact.o(.text);
        src/event_hub.o(.text);
        src/cutscene_mgr.o(.text);
        src/obj_state.o(.text);
        src/battle_engine.o(.text);
        src/battle_anim.o(.text);
        src/battle_rewards.o(.text);
        src/obj_pool.o(.text);
        src/battle_fx.o(.text);
        src/script_vm.o(.text);
        src/sound.o(.text);

        asm/m4a_asm.o(.text);
        src/m4a.o(.text);
        asm/libagbsyscall.o(.text);
        src/agb_sram.o(.text);
        *libgcc.a:_call_via_rX.o(.text);
        *libgcc.a:_divsi3.o(.text);
        *libgcc.a:_dvmd_tls.o(.text);
        *libgcc.a:_fixunsdfsi.o(.text);
        *libgcc.a:_fixunssfsi.o(.text);
        *libgcc.a:_modsi3.o(.text);
        *libgcc.a:_udivsi3.o(.text);
        *libgcc.a:_umodsi3.o(.text);
        *libgcc.a:dp-bit.o(.text);
        *libgcc.a:fp-bit.o(.text);
        *libgcc.a:_lshrdi3.o(.text);
        *libgcc.a:_muldi3.o(.text);
        *libgcc.a:_negdi2.o(.text);
        *libc.a:memcpy.o(.text);
    }

    .rodata ALIGN(4) : SUBALIGN(4) {
        src/data_805769C.o(.rodata);
        data/data.o(.rodata);
        . = ORIGIN(rom) + 0x61C784;
        src/data_script.o(.rodata);
        src/m4a_tables.o(.rodata);
        data/sound_data.o(.rodata);
        src/agb_sram.o(.rodata);
        . = ORIGIN(rom) + 0x7E83F0;

        src/data_87E83F0.o(.rodata);
        data/data1a.o(.rodata);
        . = ORIGIN(rom) + 0x7EA1A0;
        src/data_087EA1A0.o(.rodata);
        data/data1b.o(.rodata);
        . = ORIGIN(rom) + 0x7ED6D4;
        src/data_087ED6D4.o(.rodata);
        data/data1c.o(.rodata);

    } >rom =0xFF



    
}

gIntroBgPalettes = 0x08086C1C;
gBgPalBackdropWhite = 0x08087216;
gUiSpritesAuxDesc = 0x087EB2C4;
gIntroBgTiles = 0x087E9828;
gIntroBgMaps = 0x087E98C0;

/* s32 返回类型别名 (sub_8009D34 需要无截断的返回值读取, 见 code_8005020.c) */
Sprite_EnqueueRender_S32 = Sprite_EnqueueRender;

/* ==== MOD-01 语义命名新增地址 (SECTIONS 外绝对符号) ==== */

/* ==== Title & Card Menu ROM Assets ==== */
gTitleLogoGfx_GameArts = 0x080963C0;
gTitleLogoGfx_Esp = 0x08096B2C;
gSaveMenuStringTable = 0x080936A0;
gTitleMenuDesc = 0x08098622;
gCardAlbumMenuDesc = 0x08098630;
gCardDetailMenuDesc = 0x0809863B;
gCardBgMapTable = 0x08089B90;
gOptionsMenuDesc = 0x08098630;
gGalleryMenuDesc = 0x0809863B;
