#ifndef MENU_UI_H
#define MENU_UI_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

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

void sub_8010624(u8, u8);

void sub_8010770(u8);

void ScreenIdleIcons_BuildList();

#endif // MENU_UI_H
