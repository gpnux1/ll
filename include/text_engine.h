#ifndef TEXT_ENGINE_H
#define TEXT_ENGINE_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

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

#endif // TEXT_ENGINE_H
