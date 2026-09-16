#ifndef PLAYER_STATS_H
#define PLAYER_STATS_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

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

void Inventory_AddItem(u8, u8); // AddInventoryItem: add item to inventory (cap 99)

void sub_800AA84(u8, u8); // RemoveInventoryItem: remove item from inventory (floor 0)

#define AddInventoryItem    Inventory_AddItem

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

#endif // PLAYER_STATS_H
