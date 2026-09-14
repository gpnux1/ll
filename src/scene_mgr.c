#include "code_0.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "globals.h"
#include "data_87E83F0.h"
#include "data_805769C.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "sound.h"

extern void IntrMain();

// 切换场景加载
//  @ 0x08001354
void SceneTransition_Load(void)
{
    u16 i;
    u8 temp_r6;
    s32 temp_r7;

    Display_ShutdownSequence();
    temp_r6 = gVBlankPipelineMode;
    gVBlankPipelineMode = 0;
    temp_r7 = gSceneTransitionArg;
    gHBlankEffectMode = 0;
    MenuEnt_ClearStates();
    gCameraTargetX = gSpawnTileX * 8;
    gCameraTargetY = gSpawnTileY * 8;
    if (gCameraSnapFlag != 0)
    {
        gCameraPosX = gCameraTargetX;
        gCameraPosY = gCameraTargetY;
    }
    SpritePool_Clear();
    Queue34C0_Clear();
    RenderQueue_Clear();

    for (i = 0; i < 128; i++)
    {
        gOamBuffer[i].attrs[0] = 0;
        gOamBuffer[i].attrs[1] = 0;
    }

    VramTransfer_Clear();
    PalTransfer_Clear();
    AnimSlots_Release();
    StaticObjs_Reset();
    Sprites_ReleaseAll();
    gPlayerMoveDir = gSpawnFacingDir;
    Party_FollowAnim();
    Party_FollowStep();
    MapScene_Load(gMapNpcSetId);
    MapScene_InitSprites(gMapNpcSetId);
    Sprites_LoadMapNPCs(gMapNpcSetId);
    BgScroll_LoadFromTable(gMoveCmdSetId);
    ChestObjects_LoadForMap(gMapNpcSetId);
    StaticObjGfx_LoadPair(gMapObjGfxSetId);
    StaticObjs_Spawn(gMapObjGfxSetId);

    gVBlankPipelineMode = temp_r6;

    if ((Script_GetFlags() & 1) == 0)
    {
        ScriptPump_JumpToEntry(0xFF, 2);
        ScriptPump_Run();
    }

    Sprites_UpdateFrame();

    REG_WIN0H = 0xF0;
    REG_WIN0V = 0xA0;
    REG_DISPCNT |= DISPCNT_WIN0_ON;
    REG_WININ = 0;
    REG_WINOUT = 0;

    if ((Script_GetFlags() & 1) == 0)
    {
        Palette_Backup();
        ScreenFx_SetMode(3);
    }

    gInputLockFrames = 2;
    gGameState = GAME_STATE_MAP_EXPLORE;
    gScenePhase = 0;
    Display_RestartAfterLoad();

    if (gSceneEntryFlag != 0xFF || gSceneTransitionArg == 0)
        return;

    if (temp_r7 == gSceneTransitionArg)
    {
        Bgm_SetVolume(0);
        VBlankWaitExit_PumpSound();
        Bgm_Continue();
        VBlankWaitExit_PumpSound();
        Bgm_FadeIn(4);
    }
    else
    {
        Bgm_SetVolume(0);
        VBlankWaitExit_PumpSound();
        Bgm_Stop();
        VBlankWaitExit_PumpSound();
        Bgm_Play(gSceneTransitionArg, 0);
        VBlankWaitExit_PumpSound();
        Bgm_FadeIn(4);
    }
}

// New Game
//  @ 0x08001538
void NewGame_Init(void)
{
    u8 i;
    u16 i1;

    gSceneEntryFlag = 0xFF;
    gDialogueActive = 0;
    gUnk_03000000 = 0;
    gUnk_03000002 = 0;
    gScenePhase = 0;
    gSceneLoadToggle = 0;
    gTitleFadeTimer = 0;
    gPartyFollowFlags = 0;
    gWarpAnimState = 0;
    gScreenIdleEventFlags[0] = 0;
    gScreenIdleEventFlags[1] = 0;

    gVBlankPipelineMode = 1;
    gMapNpcSetId = 0x82;
    gSpawnTileX = 0;
    gSpawnTileY = 0;
    gSpawnFacingDir = 0;
    gMoveCmdSetId = 0;
    gPartyMemberIds[0] = 0;
    gPartyMemberIds[1] |= 0xFF;
    gPartyMemberIds[2] |= 0xFF;
    gPartyMemberIds[3] |= 0xFF;
    gPartyMemberIds[4] |= 0xFF;
    gPartyMemberIds[5] |= 0xFF;
    gBattleFormationIds[0] = 0;
    gBattleFormationIds[1] |= 0xFF;
    gBattleFormationIds[2] |= 0xFF;
    gBattleFormationIds[3] |= 0xFF;
    gBattleFormationIds[4] |= 0xFF;
    gBattleFormationIds[5] |= 0xFF;

    Party_InitStats();

    for (i = 0; i < 11; i++)
    {
        Stats_RebuildEquipBonuses(i);
        Stats_RecalcEquip(i);
    }

    gInventory[0xDD] = 2;
    gSilverAmount = 300;
    gGameTimer = 0;
    gCameraTargetX = 0x60;
    gCameraTargetY = 0x50;

    Script_ResetVM();

    REG_WIN0H = 0xF0;
    REG_WIN0V = 0xA0;
    REG_DISPCNT |= DISPCNT_WIN0_ON;
    REG_WININ = 0;
    REG_WINOUT = 0;

    if (gTitleIntroState == TITLE_INTRO_DISABLED)
    {
        gMapScriptSetId = 1;
        gEnvScriptSetId = 1;
        ScriptSet_Load(1, 0, 1);

        for (i1 = 0; i1 < 16; i1++)
        {
            VBlankWaitExit_PumpSound();
        }
    }
    else
    {
        ScriptSet_Load(0, 0, 1);
        for (i1 = 0; i1 < 4; i1++)
        {
            VBlankWaitExit_PumpSound();
        }
    }
    ScriptPump_JumpToEntry(1, 2);
    gGameState = GAME_STATE_MAP_EXPLORE;
    gScenePhase = 0;
}

// @ 0x08001708
void Scene_EnterMap(void)
{
    u16 i;

    if (gScreenTransitionState == 0 && gScenePhase == 1)
    {
        gCameraTargetX = gSpawnTileX << 3;
        gCameraTargetY = gSpawnTileY << 3;

        if (gCameraSnapFlag != 0)
        {
            gCameraPosX = gCameraTargetX;
            gCameraPosY = gCameraTargetY;
        }

        BgScroll_LoadFromTable(gMoveCmdSetId);
        gPlayerMoveDir = gSpawnFacingDir;
        Party_FollowStep();

        for (i = 0; i < 8; i++)
        {
            gFollowerHistX[i] = gActors[0].x;
            gFollowerHistY[i] = gActors[0].y;
            gFollowerHistDir[i] = gActors[0].facingDir;
        }

        gActors[1].x = gActors[0].x;
        gActors[1].y = gActors[0].y;
        gActors[1].facingDir = gActors[0].facingDir;

        ScreenFx_SetMode(3);
        gInputLockFrames = 2;
        gGameState = GAME_STATE_MAP_EXPLORE;
        gScenePhase = 0;
    }
    else
    {

        if (gScenePhase != 1)
        {
            gSceneLoadToggle = (gSceneLoadToggle + 1) & 1;
            gScenePhase = 1;
            Palette_Backup();
            ScreenFx_SetMode(4);
        }
    }

    Sprites_UpdateFrame();
}

// @ 0x08001828
void BattleTransition_Enter(void)
{
    u16 i;
    u8 val;
    val = gScreenTransitionState;

    if (val == 0 && gScenePhase == 1)
    {
        Display_ShutdownSequence();
        gVBlankPipelineMode = val;
        gScenePhase = val;

        if (gAfterBattleCounter != 0)
        {
            gAfterBattleCounter++;
            sub_8017FA4(gUnk_030025B8);
        }
        else
        {
            sub_8017FA4(gEncounterEnabled);
        }

        gMainLoopMode = MAIN_LOOP_BATTLE;
        gVBlankPipelineMode = 2;
    }
    else
    {
        if (gScenePhase != 1)
        {
            gScenePhase = 1;
            Palette_Backup();
            ScreenFx_SetMode(4);
            Sfx_Play(0x16, 0, 0);
            Bgm_FadeOut(0x2E);
        }
        Sprites_UpdateFrame();
    }
}

// @ 0x080018D4
void Scene_Reload(void)
{
    s16 i;

    REG_IME = 0;
    MenuEnt_ClearStates();
    gVBlankPipelineMode = 0;
    RenderQueue_Clear();

    for (i = 0; i < 0x80; i++)
    {
        gOamBuffer[i].attrs[0] = 0;
        gOamBuffer[i].attrs[1] = 0;
    }

    VramTransfer_Clear();
    PalTransfer_Clear();
    if (gAfterBattleCounter != 0)
    {
        gAfterBattleCounter++;

        if (gBattleResultType == 2)
        {
            BgPal_ResetFirst();
            REG_WIN0H = 0xF0;
            REG_WIN0V = 0xA0;
            REG_DISPCNT |= DISPCNT_WIN0_ON;
            REG_WININ = 0;
            REG_WINOUT = 0;
            gVBlankPipelineMode = 1;
            gGameState = GAME_STATE_MAP_EXPLORE;
            gScenePhase = 0;
            REG_IME = 1;
            Display_RestartAfterLoad();
            return;
        }
    }
    Party_FollowStep();
    gVBlankPipelineMode = 1;
    REG_IME = 1;
    MapScene_Load(gMapNpcSetId);

    if (!(1 & Script_GetFlags()))
    {
        ScriptSet_Load(gEnvScriptSetId, 0, 1);
    }

    ReloadAllSpriteSheets();
    BgScroll_LoadFromTable(gMoveCmdSetId);
    StaticObjGfx_LoadPair(gMapObjGfxSetId);

    gGameState = GAME_STATE_MAP_EXPLORE;
    gScenePhase = 0;
    REG_WIN0H = 0xF0;
    REG_WIN0V = 0xA0;
    REG_DISPCNT |= DISPCNT_WIN0_ON;
    REG_WININ = 0;
    REG_WINOUT = 0;

    if (gAfterBattleCounter == 0 || (gBattleResultType != 1))
    {
        Palette_Backup();
        ScreenFx_SetMode(3);
    }

    Display_RestartAfterLoad();

    if (gSceneTransitionArg != 0)
    {
        VBlankWaitExit_PumpSound();
        Bgm_Play(gSceneTransitionArg, 0);
        VBlankWaitExit_PumpSound();
        Bgm_FadeIn(4);
    }

    Sprites_UpdateFrame();
}

// @ 0x08001A7C
void Scene_EnterDoor(void)
{
    s32 var_r4;

    if (gScreenTransitionState == 0 && gScenePhase == 1)
    {
        Display_ShutdownSequence();
        MenuEnt_ClearStates();
        SpritePool_Clear();
        Queue34C0_Clear();
        RenderQueue_Clear();
        MapBg_LoadInterior(gChoiceGroupIdx);
        PlayerSheets_Load();
        Party_FollowAnim();
        gCameraPosX = 0;
        gCameraPosY = 0;
        gPlayerMoveDir = 4;
        Palette_Backup();
        ScreenFx_SetMode(3);
        gScenePhase = 0;
        gGameState = GAME_STATE_BATTLE_MENU;
        Party_SetFollowMode();
        ChoiceMenu_BuildList();

        ChoiceMenu_ResolveDest(*(u8 *)(gChoiceListPtr + gChoiceCursor) & 0xF);

        gCameraTargetX = gChoiceDestX;
        gCameraTargetY = gChoiceDestY;
        gActors[1].x = gChoiceDestX;
        gActors[1].y = gChoiceDestY - 8;
        gActors[1].stateFlags |= 0x10u;
        gActors[1].facingDir = 4;
        BattleIntro_Cursor();
        Display_RestartAfterLoad();
        gSceneTransitionArg = 0;

        var_r4 = EventFlags_Test(0x39) != 0 ? 6 : 5;

        VBlankWaitExit_PumpSound();
        Bgm_Play(var_r4, 0);
        VBlankWaitExit_PumpSound();
        Bgm_FadeIn(4);
        return;
    }

    if (gScenePhase != 1)
    {
        gSceneLoadToggle = (gSceneLoadToggle + 1) & 1;
        gScenePhase = 1;
        Palette_Backup();
        ScreenFx_SetMode(4);
        Bgm_FadeOut(0x2E);
    }

    Sprites_UpdateFrame();
}
// @ 0x08001BD0
void Scene_RestoreAfterBattle(void)
{
    s16 i;

    MenuEnt_ClearStates();
    RenderQueue_Clear();

    for (i = 0; i < 128; i++)
    {
        gOamBuffer[i].attrs[0] = 0;
        gOamBuffer[i].attrs[1] = 0;
    }

    VramTransfer_Clear();
    PalTransfer_Clear();
    AnimSlots_Release();
    StaticObjs_Reset();
    Script_ResetVM();

    gWarpAnimState = 0;
    gCameraSnapFlag = 0;
    gSceneEntryFlag = 0xFF;

    DmaCopy16(3, (void *)0x0203F000, gSpriteNodePool, 0xA00);
    DmaCopy16(3, (void *)0x0203FE00, (void *)0x030034C0, 0x100);

    Party_FollowStep();
    Followers_ResetHistory();
    MapScene_Load(gMapNpcSetId);
    ScriptSet_Load(gEnvScriptSetId, 0, 1);
    ReloadAllSpriteSheets();
    BgScroll_LoadFromTable(gMoveCmdSetId);
    StaticObjGfx_LoadPair(gMapObjGfxSetId);
    StaticObjs_Spawn(gMapObjGfxSetId);

    gGameState = GAME_STATE_MAP_EXPLORE;
    gScenePhase = 0;

    Palette_Backup();
    ScreenFx_SetMode(3);

    gInputLockFrames = 2;

    Display_RestartAfterLoad();

    if (gSceneTransitionArg != 0)
    {
        Bgm_Play(gSceneTransitionArg, 0);
        Sprites_UpdateFrame();
        VBlankWaitExit_PumpSound();
        Bgm_FadeIn(4);
    }
    Sprites_UpdateFrame();
}

// @ 0x08001D08
void Task_MapExplore(void)
{
    u8 eventId;
    u8 moveFlags;
    u8 moveSpeed;

    moveFlags = 0;

    gActors[0].stateFlags &= ~0x40;

    if ((Script_GetFlags() & 1) == 0)
    {
        gDialogueActive = 0;
        if (gTitleIntroState != TITLE_INTRO_DISABLED && (gScreenFadeFlags & 0x80) == 0 && !gScreenTransitionState)
        {
            gGameState = GAME_STATE_TITLE_MENU;
            gScenePhase = 5;
            return;
        }
    }
    else
    {
        gDialogueActive = 1;
        if (gTitleIntroState != TITLE_INTRO_DISABLED && (gScreenFadeFlags & 0x80) == 0 && !gScreenTransitionState && (gNewKeysRaw & 0x30F) != 0)
        {
            Script_Abort(1);
            gGameState = GAME_STATE_TITLE_MENU;
            gScenePhase = 5;
            return;
        }
    }

    if ((gHeldKeysRaw & ABXY_BUTTONS) == ABXY_BUTTONS)
    {
        // Reset Game
        Script_Abort(1);
        System_ResetToLogo();
        return;
    }

    if (gUnk_03004D4C == 0 && gWarpAnimState == 0)
    {
        if (!gDialogueActive && (gScreenFadeFlags & 0x80) == 0 && !gScreenTransitionState)
        {
            if (gInputLockFrames != 0)
            {
                gInputLockFrames--;
                gNewKeysRaw = 0;
                gHeldKeysRaw = 0;
            }

            if (gPendingCharaSwitch != 0xFF)
            {
                ScriptPump_JumpToEntry(gPendingCharaSwitch, 2);
                gPendingCharaSwitch = 0xFF;
            }
            else
            {
                if (gNewKeysRaw & A_BUTTON)
                {
                    eventId = CheckFacingEvent();
                    if (eventId != 0)
                    {
                        ScriptPump_JumpToEntry(eventId - 1, 2);
                    }
                }
                else if (gNewKeysRaw & B_BUTTON)
                {
                    sub_800ACC8();
                }
                if ((Script_GetFlags() & 1) != 0)
                {
                    gDialogueActive = 1;
                }

                // gDialogueActive = 1     表示当前正在对话，无法移动
                // gUnk_03004D4C = 0xD   菜单界面，无法移动

                if (gDialogueActive || gUnk_03004D4C)
                {
                    moveSpeed = 0;
                }
                else
                {
                    if (CheckEncounter() != 0)
                    {
                        // 遇敌后先进入淡出/战斗初始化状态, 再切换顶层 battle loop。
                        gGameState = GAME_STATE_BATTLE_ENTER;
                        return;
                    }

                    if (gHeldKeysRaw & DPAD_UP)
                    {
                        moveFlags = 1;
                        gActors[0].stateFlags |= 0x40;
                    }
                    else if (gHeldKeysRaw & DPAD_DOWN)
                    {
                        moveFlags = 2;
                        gActors[0].stateFlags |= 0x40;
                    }
                    if (gHeldKeysRaw & DPAD_LEFT)
                    {
                        moveFlags |= 4;
                        gActors[0].stateFlags |= 0x40;
                    }
                    else if (gHeldKeysRaw & DPAD_RIGHT)
                    {
                        moveFlags |= 8;
                        gActors[0].stateFlags |= 0x40;
                    }

                    if (gWalkMoveDirLut[moveFlags])
                    {
                        gPlayerMoveDir = gWalkMoveDirLut[moveFlags] - 1;
                        moveSpeed = 2;
                    }
                    else
                    {
                        moveSpeed = 0;
                    }
                }
                MovePlayer(&gCameraTargetX, &gCameraTargetY, gPlayerMoveDir + 1, moveSpeed);
            }
        }
    }
    else if (gUnk_03004D4C != 0)
    {
        sub_800ACC8();
    }
    else
    {
        // gWarpAnimState = 1  开始播放传送动画
        switch (gWarpAnimState)
        {
            case 1:
                gWarpAnimState = 2;
                break;
            case 2:
                CutsceneAnim_Load(0x78, 0, 0xA);
                VBlankWait_PumpSound();
                CutsceneAnim_Load(0x79, 1, 0xA);
                VBlankWait_PumpSound();
                gWarpAnimState = 3;
                break;
            case 3:
                Sfx_Play(0x19, 0, 0);
                Chara_InitEffectAtPlayer();
                Chara_StartScriptAnim(0x12, 0);
                gWarpAnimState = 4;
                break;
            case 4:
                if (Chara_AnimWaitDone(0x12) != 0)
                {
                    Sfx_Play(0x1A, 0, 0);
                    Chara_FreeSprite(0);
                    Chara_FreeSprite(1);
                    Chara_StartScriptAnim(0x12, 1);
                    gWarpAnimState = 5;
                }
                break;
            case 5:
                if (Chara_AnimWaitDone(0x12) != 0)
                {
                    gGameState = GAME_STATE_SCENE_REQUEST_MAP;
                    Chara_FreeSprite(0x12);
                    gWarpAnimState = 9;
                }
                break;
            case 6:
                Sfx_Play(0x1A, 0, 0);
                CutsceneAnim_Load(0x7A, 0, 0xA);
                VBlankWait_PumpSound();
                CutsceneAnim_Load(0x7B, 1, 0xA);
                VBlankWait_PumpSound();
                Chara_InitEffectAtPlayer();
                Chara_StartScriptAnim(0x12, 0);
                gWarpAnimState = 7;
                break;
            case 7:
                if (Chara_AnimWaitDone(0x12) != 0)
                {
                    Sfx_Play(0x19, 0, 0);
                    Chara_StartScriptAnim(0x12, 1);
                    gWarpAnimState = 8;
                }
                break;
            case 8:
                if (Chara_AnimWaitDone(0x12) != 0)
                {
                    gWarpAnimState = 0;
                    Chara_FreeSprite(0x12);
                }
                break;
            case 9:
                if ((0x80 & gScreenFadeFlags) || (gScreenTransitionState != 0))
                {
                    gWarpAnimState = 10;
                }
                break;

            case 10:
                if (!(0x80 & gScreenFadeFlags) && (gScreenTransitionState == 0))
                {
                    gWarpAnimState = 6;
                }
                break;
        }
    }

    if (gDialogueActive != 0)
    {
        gActors[0].stateFlags |= 0x40;
    }
    ScriptPump_Run();
    LogoBlendEffect_Update();
    Sprites_UpdateFrame();
}
