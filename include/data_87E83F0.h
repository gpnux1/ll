#ifndef _DATA_87E83F0_H
#define _DATA_87E83F0_H

#include "gba/types.h"

typedef void (*MainLoopCallback)(void);
typedef void (*GameStateCallback)(void);

/* AgbMain selects one top-level loop. Normal gameplay then selects one state
 * callback from gGameStateCallbacks. Keep these as byte-sized values in IWRAM. */
enum MainLoopMode
{
    MAIN_LOOP_GAME = 0,
    MAIN_LOOP_BATTLE = 1,
};

enum GameState
{
    GAME_STATE_NEW_GAME = 0,
    GAME_STATE_MAP_EXPLORE,
    GAME_STATE_SCENE_LOAD,
    GAME_STATE_SCENE_REQUEST_MAP,
    GAME_STATE_SCENE_ENTER_MAP,
    GAME_STATE_BATTLE_ENTER,
    GAME_STATE_SCENE_RELOAD,
    GAME_STATE_DIALOGUE,
    GAME_STATE_ENTER_DOOR,
    GAME_STATE_BATTLE_MENU,
    GAME_STATE_RELOAD_VIA_MENU,
    GAME_STATE_TITLE_MENU,
    GAME_STATE_RESTORE_AFTER_BATTLE,
    GAME_STATE_TEXT,
};

extern const MainLoopCallback gMainLoopCallbacks[];
extern const GameStateCallback gGameStateCallbacks[];

extern const u8 *const gGfxSpriteSheetWalk_PtrTable[];

extern const u32 gUnk_087E860C[];
extern const u32 gUnk_087E8D84[];

typedef struct
{
    u8 field_0;
    u8 field_1;
    u8 tileX;
    u8 tileY;
} Unk_087E94FC;

extern const Unk_087E94FC gUnk_087E94FC[];

/* 同址别名: = &gUnk_087E94FC[-1] (off_87E8D84 blob 末尾的指针字 0x080838F8)。
 * CheckFacingEvent 以它为特殊事件表基址: &gUnk_087E94F8[gUnk_03004618 * 4]
 * = &gUnk_087E94FC[gUnk_03004618 - 1] (gUnk_03004618 = MapGroup_Lookup 的 i+1)。 */
extern const u8 gUnk_087E94F8[];

#endif
