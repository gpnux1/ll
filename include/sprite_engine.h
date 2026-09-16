#ifndef SPRITE_ENGINE_H
#define SPRITE_ENGINE_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

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

void CutsceneAnim_PlayFrame(u16 animEntityId);

void MapGroup_Lookup();

void Chara_SetTilePos(u8, u8, u8, u8);

void Chara_MoveBy(u8, u8, u8, u8);

#endif // SPRITE_ENGINE_H
