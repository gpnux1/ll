#ifndef MAP_ZONE_H
#define MAP_ZONE_H

#include "gba/types.h"
#include "iwram.h"
#include "map_scene_runtime.h"

s32 sub_8007ADC(s16, s16);
// gMapZoneType/gMapZoneEntryIdx 返回 1
#define MapZone_FindAt sub_8007ADC
s32 sub_8007BD0(void);
#define MapZone_Trigger sub_8007BD0

#endif // MAP_ZONE_H
