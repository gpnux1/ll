#ifndef GUARD_MENU_SLOT_H
#define GUARD_MENU_SLOT_H

#include "gba/types.h"

#define MENU_SLOT_COUNT 10
#define MENU_SLOT_BYTES 5

// Byte offsets within each 5-byte slot state:
#define MENU_SLOT_OFFSET_CURSOR       0 // Current active cursor/selection index
#define MENU_SLOT_OFFSET_WIN1_START   1 // Primary scroll window top/start index
#define MENU_SLOT_OFFSET_WIN1_CURSOR  2 // Primary scroll window cursor position
#define MENU_SLOT_OFFSET_WIN2_START   3 // Secondary scroll window top/start index
#define MENU_SLOT_OFFSET_WIN2_CURSOR  4 // Secondary scroll window cursor position

typedef struct MenuSlotState {
    u8 cursor;
    u8 win1Start;
    u8 win1Cursor;
    u8 win2Start;
    u8 win2Cursor;
} MenuSlotState;

// Reset all 10 menu slot states and the master cursor
void MenuSlot_ResetAll(void);

// Sync menu slot state (case 0: restore master, case 3: cursor, case 6: win1, case 7: win2)
void sub_8021184(s8 mode, u8 *obj);

#endif // GUARD_MENU_SLOT_H
