#ifndef SCENE_MGR_H
#define SCENE_MGR_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void SceneTransition_Load();

void NewGame_Init();

void Scene_EnterMap();

void BattleTransition_Enter();

void Scene_Reload();

void Scene_EnterDoor();

void Scene_RestoreAfterBattle();

void Task_MapExplore();

#endif // SCENE_MGR_H
