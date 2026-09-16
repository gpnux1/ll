#ifndef GUARD_MENU_H
#define GUARD_MENU_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* One 32-bit SIO transfer. Only cardIdLo/cardIdHi carry payload here; the
 * upper halfword remains zero after RAM reset. Keep byte fields because the
 * matched title code performs two strb/ldrb accesses. */
typedef struct CardExchangePacket {
    u8 cardIdLo;
    u8 cardIdHi;
    u16 reserved;
} CardExchangePacket;

extern CardExchangePacket gSioSendPacket;
extern CardExchangePacket gSioRecvPacket;

// 标题、读档、卡片图鉴与卡片交换共用的主状态机。
enum TitleMenuPhase {
    TITLE_PHASE_INIT = 0,
    TITLE_PHASE_LOGO_LOAD = 1,
    TITLE_PHASE_LOGO_FADEIN = 2,
    TITLE_PHASE_LOGO_HOLD = 3,
    TITLE_PHASE_LOGO_FADEOUT = 4,
    TITLE_PHASE_TITLE_LOAD = 5,
    TITLE_PHASE_TITLE_FADEIN = 6,
    TITLE_PHASE_PRE_PROMPT_HOLD = 7,
    TITLE_PHASE_PRESS_START = 8,
    TITLE_PHASE_MAIN_MENU = 9,
    TITLE_PHASE_START_NEW_GAME = 10,
    TITLE_PHASE_LOAD_MENU_OPEN = 11,
    TITLE_PHASE_LOAD_READ_SLOTS = 12,
    TITLE_PHASE_LOAD_MENU_FADEIN = 13,
    TITLE_PHASE_LOAD_MENU = 14,
    TITLE_PHASE_SUBMENU_CLOSE = 15,
    TITLE_PHASE_RETURN_TO_MAIN_MENU = 16,
    TITLE_PHASE_START_LOADED_GAME = 17,
    TITLE_PHASE_OPTIONS_OPEN = 18,
    TITLE_PHASE_OPTIONS_FADEIN = 19,
    TITLE_PHASE_OPTIONS_MENU = 20,
    TITLE_PHASE_ATTRACT_START = 21,
    TITLE_PHASE_GALLERY_SCENE_LOAD = 22,
    TITLE_PHASE_GALLERY_SCENE_FADEIN = 23,
    TITLE_PHASE_GALLERY_EXIT_FADEOUT = 24,
    TITLE_PHASE_GALLERY_SCENE_LOOP = 25,
    TITLE_PHASE_GALLERY_RETURN_TO_OPTIONS = 26,
    TITLE_PHASE_CARD_EXCHANGE_PREP = 27,
    TITLE_PHASE_CARD_EXCHANGE_INIT = 28,
    TITLE_PHASE_CARD_EXCHANGE_FADEIN = 29,
    TITLE_PHASE_CARD_EXCHANGE_MENU = 30,
    TITLE_PHASE_CARD_EXCHANGE_CONNECT = 31,
    TITLE_PHASE_CARD_EXCHANGE_BLEND_DOWN = 32,
    TITLE_PHASE_CARD_EXCHANGE_BLEND_UP = 33,
    TITLE_PHASE_CARD_EXCHANGE_SEND_PACKET = 34,
    TITLE_PHASE_CARD_EXCHANGE_POLL = 35,
    TITLE_PHASE_CARD_EXCHANGE_CANCEL = 36,
    TITLE_PHASE_CARD_EXCHANGE_SAVE = 37,
    TITLE_PHASE_CARD_EXCHANGE_FINISH_FADE = 38,
};

/* gMenuCursorGrp selects one of these input/rendering pages inside
 * TitleMenu_UpdateUi. The same cursor stack is shared with other menus. */
enum TitleMenuPage {
    TITLE_PAGE_PRESS_START = 0,
    TITLE_PAGE_MAIN_MENU = 1,
    TITLE_PAGE_LOAD = 2,
    TITLE_PAGE_OPTIONS = 3,
    TITLE_PAGE_GALLERY = 4,
    TITLE_PAGE_CARD = 5,
};

/* This byte is both the publisher-logo index during startup and the boolean
 * marker for the title-screen attract/demo route after both logos finish. */
enum TitleIntroState {
    TITLE_INTRO_DISABLED = 0,
    TITLE_INTRO_GAME_ARTS_LOGO = 1,
    TITLE_INTRO_ESP_LOGO = 2,
    TITLE_INTRO_ATTRACT_MODE = 3,
};

// 卡片交换网络状态
enum CardExchangeStatus {
    CARD_EXCHANGE_IDLE = 0,
    CARD_EXCHANGE_SENDING = 1,
    CARD_EXCHANGE_TIMEOUT = 2,
    CARD_EXCHANGE_FAILED = 3,
    CARD_EXCHANGE_SUCCESS = 5,
    CARD_EXCHANGE_CONNECTING = 0xFF,
};

// 标题与菜单系统函数声明
void TitleMenu_ProcessFrame(void);

/* Generated module API declarations (was include/code_0.h). */

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

#endif // GUARD_MENU_H
