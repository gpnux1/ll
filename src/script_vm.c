#include "battle_types.h"
#include "script_vm.h"
#include "map_scene_runtime.h"
#include "battle_task_services.h"
#include "engine_core.h"
#include "map_view.h"
#include "menu.h"
#include "player_stats.h"
#include "save.h"
#include "sound.h"
#include "sprite_engine.h"
#include "text_engine.h"
#include "vram_transfer.h"
#include "data_87E83F0.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"

// @ 0x0804F280
INCLUDE_ASM("asm/nonmatchings", sub_804F280);
// @ 0x0804F64C
u32 Op_CameraPan(u32 *pScriptCursor)
{
    u8 *pBytecode = (u8 *)*pScriptCursor;

    gCameraPanDuration = pBytecode[1];
    gCameraPanStep = 0;
    switch (gCameraDrawMode)
    {
    case 2:
        gCameraPanStartX = gDrawCamX;
        gCameraPanStartY = 0;
        break;

    case 5:
        if (!gDrawCamEaseActive)
        {
            gCameraPanStartX = gCameraPosX;
        }
        else
        {
            gCameraPanStartX = gDrawCamX;
        }
        gCameraPanStartY = gCameraPosY;
        break;

    case 8:
        if (!gDrawCamEaseActive)
        {
            gCameraPanStartY = gCameraPosY;
        }
        else
        {
            gCameraPanStartY = gDrawCamY;
        }
        gCameraPanStartX = gCameraPosX;
        break;

    default:
        gCameraPanStartX = gCameraPosX;
        gCameraPanStartY = gCameraPosY;
        break;
    }
    gCameraPanTargetX = pBytecode[2] + (pBytecode[3] << 8);
    gCameraPanTargetY = pBytecode[4] + (pBytecode[5] << 8);
    *pScriptCursor += 6;
    return 1;
}
// @ 0x0804F768
u32 Op_RemovePartyMember(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u16 i;
    u8 temp;

    pBytecode = (u8 *)*pScriptCursor;
    for (i = 0; i <= 4; i++)
    {
        if (gPartyMemberIds[i] == pBytecode[1])
        {
            gPartyMemberIds[i] = 0xFF;
            for (; i <= 4; i++)
            {
                temp = gPartyMemberIds[i];
                gPartyMemberIds[i] = gPartyMemberIds[i + 1];
                gPartyMemberIds[i + 1] = temp;
            }
            break;
        }
    }

    for (i = 0; i <= 4; i++)
    {
        if (gBattleFormationIds[i] == pBytecode[1])
        {
            gBattleFormationIds[i] = 0xFF;
            break;
        }
    }

    *pScriptCursor += 2;
    return 1;
}
// @ 0x0804F7F8
// 脚本 opcode: 队伍添加 (Op_RemovePartyMember 的镜像)。
//   1) 把 data[1] 按升序插入 gPartyMemberIds[0..4] (已存在则跳过; 插入点后整体后移, 末位溢出丢弃);
//   2) Chara_ClearTempStatus(data[1]); gPartyMemberIds[5] = 0xFF;
//   3) 编队槽: 取 gPartyStats[id-1].field_unk[5] 记录的旧槽,
//      旧槽为空(0xFF) → 直接占回; 否则先查该 id 是否已在 gBattleFormationIds (在 → 仅回填槽号),
//      再找首个空槽(0xFF) → 写入 id 并回填槽号; 全满 → 不动。
u32 Op_AddPartyMember(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 i;
    u8 temp;
    u8 val;
    u8 memberId;
    u8 newId;
    u8 idx;

    pBytecode = (u8 *)*pScriptCursor;
    i = 0;
    if (gPartyMemberIds[0] == pBytecode[1])
        goto after;
    do
    {
        // 死赋值(下一次迭代即被覆盖): 拉长 newId 伪寄存器寿命, 使全局分配把 ptr 给 r6、
        // newId 给 r7 (缺则 ptr 落 r7、stats 基址溢出从 ip 变 r7, 差 2 条指令)。
        newId = gPartyMemberIds[i];
        if (gPartyMemberIds[i] > pBytecode[1])
        {
            val = pBytecode[1];
            for (; i <= 4; i++)
            {
                temp = gPartyMemberIds[i];
                gPartyMemberIds[i] = val;
                val = temp;
            }
            goto after;
        }
        i++;
        if (i > 4)
            goto after;
    } while (gPartyMemberIds[i] != pBytecode[1]);
after:
    Chara_ClearTempStatus(pBytecode[1]);
    gPartyMemberIds[5] = 0xFF;

    memberId = pBytecode[1];
    idx = memberId;
    if (memberId != 0)
        idx = memberId - 1;

    if (gBattleFormationIds[gPartyStats[idx].field_unk[5]] == 0xFF)
    {
        gBattleFormationIds[gPartyStats[idx].field_unk[5]] = memberId;
    }
    else
    {
        i = 0;
        newId = memberId;
        do
        {
            if (gBattleFormationIds[i] == memberId)
            {
                gPartyStats[idx].field_unk[5] = i;
                goto end;
            }
            i++;
        } while (i <= 4);
        i = 0;
        do
        {
            if (gBattleFormationIds[i] == 0xFF)
            {
                gBattleFormationIds[i] = newId;
                gPartyStats[idx].field_unk[5] = i;
                goto end;
            }
            i++;
        } while (i <= 4);
    }
end:
    *pScriptCursor += 2;
    return 1;
}
// @ 0x0804F8D8
// 脚本 opcode: 按 gAfterBattleCounter 状态机分派。
//   state==3: 若 (sub_80187B4()&0x40)!=0 或 data[1]==0 → *ptr+=4;
//             否则 *ptr = gUnk_02016200 + gUnk_02016000[data[1]]; 清 state 返 1。
//   state==0: 初始化 gAfterBattleCounter=1 / gBattleResultType=data[3] /
//             gUnk_030025B8=data[2]+0xBA|0x1C(按 data[2] 符号) / gGameState=GAME_STATE_BATTLE_ENTER; 返 0。
// 注: 用 `goto setup` 把 setup 块强制放成分支目标(冷路径)才匹配 `beq setup` 布局;
//     非 goto 写法分支被反转成 `bne` 使 setup 落 fall-through (差 127B)。待 flag 重构。
// 注: 跳转表需 `u16 idx = data[1] * 2` 中间量, 否则 ldr 基址被调度提到 lsls 前。
u32 Op_ScriptBattle(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 state;
    u16 idx;

    pBytecode = (u8 *)*pScriptCursor;
    state = gAfterBattleCounter;
    if (state == 0)
        goto setup;
    if (state == 3)
    {
        if ((sub_80187B4() & 0x40) != 0 || pBytecode[1] == 0)
            *pScriptCursor += 4;
        else
        {
            idx = pBytecode[1] * 2;
            *pScriptCursor = (u32)(gUnk_02016200 + *(u16 *)((u32)gUnk_02016000 + idx));
        }
        gAfterBattleCounter = 0;
        return 1;
    }
    return 0;
setup:
    gAfterBattleCounter = 1;
    gBattleResultType = pBytecode[3];
    if ((s8)pBytecode[2] < 0)
        gUnk_030025B8 = pBytecode[2] + 0xBA;
    else
        gUnk_030025B8 = pBytecode[2] + 0x1C;
    gGameState = GAME_STATE_BATTLE_ENTER;
    return 0;
}
// @ 0x0804F974
// 脚本条件跳转: data[1] 为 flag 表字节数, 循环 count/2 个 u16 flag id
// (id<=0x1FF 查 EventFlags_Test, 否则查 SwitchFlags_Test(id-0x200)), 任一为真 →
// 指针跳到 0x02016200 + jtbl[data[2]], 全假 → 指针推进 *ptr + t + 3; 恒返 1。
// 注: ① 跳转路径必须写 `u16 *jtbl = (u16*)0x02016000;` 提升变量 + 字面量基址,
//     `*ptr = jtbl[data[2]] + 0x02016200;` — 符号形式或加和顺序会让 GCC 把基址加法
//     跨跳合并进公共尾部 (基址落 r1, 目标要 r2, 差 5B)。
// ② 推进路径必须用嵌套块内新变量 `u32 step = t + 3; *ptr = *ptr + step;` —
//     让 t+3 独立于 *ptr 装载 (目标 mov r1,r8; adds r1,#3; ldr r0,[r6]; adds r0,r0,r1)。
u32 Op_IfAllFlagsJump(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 t;
    u8 n;
    u16 *jtbl;
    u16 i;
    u16 v;
    u8 res;

    pBytecode = (u8 *)*pScriptCursor;
    t = pBytecode[1];
    n = t >> 1;
    jtbl = (u16 *)0x02016000;
    for (i = 0; n > i; i++)
    {
        v = pBytecode[i * 2 + 3] | (pBytecode[i * 2 + 4] << 8);
        if (v > 0x1FF)
            res = SwitchFlags_Test(v - 0x200);
        else
            res = EventFlags_Test(v);
        if (res == 0)
            break;
    }
    if (res != 0)
        *pScriptCursor = jtbl[pBytecode[2]] + 0x02016200;
    else
    {
        u32 step = t + 3;
        *pScriptCursor = *pScriptCursor + step;
    }
    return 1;
}
// @ 0x0804FA04
u32 Op_IfAllFlagsClearJump(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 t;
    u8 n;
    u32 off;
    u16 i;
    u16 v;
    u16 *jtbl;
    u8 res;

    pBytecode = (u8 *) (*pScriptCursor);
    t = pBytecode[1];
    n = t >> 1;
    for (i = 0; n > i; i++)
    {
        v = pBytecode[(i * 2) + 3] | (pBytecode[(i * 2) + 4] << 8);
        if (v > 0x1FF)
        {
            res = SwitchFlags_Test(v - 0x200);
        }
        else
        {
            res = EventFlags_Test(v);
        }
        if (res != 0)
        {
            break;
        }
    }

    jtbl = (u16 *) 0x02016000;
    if (res == 0)
    {
        *pScriptCursor = 0x02016200 + jtbl[pBytecode[2]];
    }
    else
    {
        off = t + 3;
        *pScriptCursor = (*pScriptCursor) + off;
    }
    return 1;
}

// @ 0x0804FA94
// 脚本条件跳转 (任一 flag 置位版, 镜像 Op_IfAllFlagsJump): data[1] 为 flag 表字节数, 循环 count/2 个
// u16 flag id (id<=0x1FF 查 EventFlags_Test, 否则查 SwitchFlags_Test(id-0x200)), 任一为真 →
// 指针跳到 gUnk_02016200 + gUnk_02016000[data[2]], 全假 → 指针推进 data + count + 3; 恒返 1。
// 注: t 全程存 r8 (推进路径 mov r1,r8)、0x1FF 存 sb; 跳转路径必须写 jtbl 提升变量 + 字面量基址
// 在前的和 (`jtbl = (u16*)0x02016000;` 提到 if 前, `*ptr = 0x02016200 + jtbl[data[2]]`) ——
// 直写/符号形式会让 GCC 把基址加法跨跳合并进公共尾部 (基址落 r1, 目标要 r2, 差 5B)。
// `n > i` / `v > 0x1FF` 的操作数序对应 cmp r7,r4 / cmp r1,sb, 勿翻转。
// res 无初值 = 原始代码如此 (零循环时读 r1 残值, 目标同样无初始化指令)。
u32 Op_IfAnyFlagJump(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 t;
    u8 n;
    u32 off;
    u16 i;
    u16 v;
    u16 *jtbl;
    u8 res;

    pBytecode = (u8 *) (*pScriptCursor);
    t = pBytecode[1];
    n = t >> 1;
    for (i = 0; n > i; i++)
    {
        v = pBytecode[(i * 2) + 3] | (pBytecode[(i * 2) + 4] << 8);
        if (v > 0x1FF)
        {
            res = SwitchFlags_Test(v - 0x200);
        }
        else
        {
            res = EventFlags_Test(v);
        }
        if (res != 0)
        {
            break;
        }
    }

    jtbl = (u16 *) 0x02016000;
    if (res != 0)
    {
        *pScriptCursor = 0x02016200 + jtbl[pBytecode[2]];
    }
    else
    {
        off = t + 3;
        *pScriptCursor = (*pScriptCursor) + off;
    }
    return 1;
}
// @ 0x0804FB24
// 脚本 VM 万能系统/屏幕特效 op (opcode 0x4D, 固定 3 字节 `4D <subop> <arg>`, MOD-08):
// insn[2]!=0 = 执行/开启族, insn[2]==0 = 复位/关闭/续行族; 未完成返 0 等帧,
// 完成则 *pScriptCursor += 3 返 1。ROM 数据: gFlashFxPaletteTable[0]→OBJ bank4、
// gObjPalFadeInSteps[0..9]→OBJ 1..15 色 10 帧渐显、gObjPalFadeInFinal→OBJ bank10 末帧、
// gCutsceneGfxBuf 清零→BG PLTT 16..63。subop4 的两处常量 0 写入是与复位族
// case 3/0xCA 共享 0 伪寄存器的字节精确技巧, 勿改为 insn[2] (progress.md 2026-09-07)。
u32 Op_SysEffect(u32 *pScriptCursor)
{
    u8 *insn;
    u16 fadeStep;
    u16 *palDst;

    insn = (u8 *)(*pScriptCursor);
    if (insn[2] != 0)
    {
        switch (insn[1])
        {
        case SYSFX_SHAKE:
            switch (insn[2])
            {
            case SYSFX_SHAKE_MASK_PLANE1:
                gViewportFlags[VF_SHAKE_MASK] = insn[2];
                break;
            case SYSFX_SHAKE_MASK_BOTH:
                gViewportFlags[VF_SHAKE_MASK] = 3;
                break;
            default:
                gViewportFlags[VF_SHAKE_MASK] = 7;
                break;
            }
            gViewportFlags[VF_EFFECT_EN] |= 3;
            break;

        case SYSFX_FLASH:
            gViewportFlags[VF_EFFECT_EN] |= 4;
            break;

        case SYSFX_BGLAYER:
            if (gCurrentMapId == 0x63)
            {
                REG_DISPCNT |= 0x0400;
            }
            else
            {
                REG_DISPCNT |= 0x0800;
            }
            break;

        case SYSFX_CAMEASE:
            gDrawCamEaseActive = 1;
            break;

        case SYSFX_PALFX_SEQ:
            switch (insn[2] - 1)
            {
            case SYSFX_PAL_BLACKIN - 1:
                gViewportFlags[VF_FADE_FRAME]++;
                fadeStep = gViewportFlags[VF_FADE_FRAME] >> 3;
                gBlendCoefficients = ((fadeStep << 9) + 0xF) - fadeStep;
                if (fadeStep > 0xE)
                {
                    gBlendCoefficients = 0x1F00;
                }
                else
                {
                    return 0;
                }
                break;

            case SYSFX_FLASH_SETUP - 1:
                REG_DISPCNT &= 0xFDFF;
                gBlendControl = 0xC10;
                gBlendCoefficients = 0xF0F;
                DmaCopy16(3, (const void *)gFlashFxPaletteTable, (void *)0x05000080, 0x20);
                break;

            case SYSFX_BG_PAL_CLEAR - 1:
                /* 0x02020000 (=gCutsceneGfxBuf) 必须写字面量: 符号形式是地址常量,
                 * GCC2 会把指针存 r4 跨循环复用 (差 4B), 见 ScriptPump_JumpToEntry 同款坑 */
                palDst = (u16 *)0x02020000;
                for (fadeStep = 0; fadeStep <= 0x5F; fadeStep++)
                {
                    *(palDst++) = 0;
                }
                DmaCopy16(3, (const void *)0x02020000, (void *)0x05000020, 0x60);
                break;

            case SYSFX_FADE_2PH - 1:
                gViewportFlags[VF_FADE_FRAME]++;
                fadeStep = gViewportFlags[VF_FADE_FRAME] >> 2;
                if (gViewportFlags[VF_FADE_PHASE] == 0)
                {
                    gBlendCoefficients = 0x1F00 | (fadeStep & 0xF);
                    if (fadeStep == 0xF)
                    {
                        gViewportFlags[VF_FADE_PHASE]++;
                    }
                    return 0;
                }
                gPaletteFxPhase = 0;
                break;

            case SYSFX_FADE_LONG - 1:
                gViewportFlags[VF_FADE_LONG_CNT]++;
                if (gViewportFlags[VF_FADE_LONG_CNT] > 0x3E)
                {
                    gBlendCoefficients = 0;
                    break;
                }
                if (gViewportFlags[VF_FADE_LONG_CNT] > 0x1F)
                {
                    gBlendCoefficients = 0x3F - gViewportFlags[VF_FADE_LONG_CNT];
                    return 0;
                }
                gBlendCoefficients = gViewportFlags[VF_FADE_LONG_CNT];
                return 0;
            }
            break;

        case SYSFX_SAVE_OPEN:
            if (Save_Fsm(1) != 0)
            {
                return 0;
            }
            break;

        case SYSFX_OBJPAL_IN:
            if (gViewportFlags[VF_FADE_FRAME] <= 9)
            {
                DmaCopy16(3, (const void *)(gObjPalFadeInSteps + (gViewportFlags[VF_FADE_FRAME] * 0x20)), (void *)(0x05000002 + (gViewportFlags[VF_FADE_FRAME] * 0x20)), 0x1E);
            }
            else
            {
                DmaCopy16(3, (const void *)gObjPalFadeInFinal, (void *)0x05000140, 0x20);
            }
            gViewportFlags[VF_FADE_FRAME]++;
            if (gViewportFlags[VF_FADE_FRAME] <= 10)
            {
                return 0;
            }
            break;

        case SYSFX_HEAL:
            FullHealCharacter(insn[2]);
            break;

        case SYSFX_MAPBG:
            ((void (*)(u32))MapBg_LoadFull)(insn[2] + 0x81);
            break;

        case SYSFX_INTROBG:
            IntroBg_Load(insn[2]);
            break;

        case SYSFX_WHITEOUT:
            gViewportFlags[VF_WHITEOUT_CNT]++;
            if (insn[2] == 1)
            {
                if (gViewportFlags[VF_WHITEOUT_CNT] > 0xE)
                {
                    gBlendCoefficients = 0xF1F;
                    gViewportFlags[VF_WHITEOUT_CNT] = 0;
                    break;
                }
                gBlendCoefficients = (gViewportFlags[VF_WHITEOUT_CNT] << 8) + 0x1F;
                gBlendControl = 0x1E41;
                return 0;
            }
            else
            {
                if (gViewportFlags[VF_WHITEOUT_CNT] > 0xE)
                {
                    gBlendCoefficients = 0x1F;
                    gBlendControl = 0x1E01;
                    gViewportFlags[VF_WHITEOUT_CNT] = 0;
                    break;
                }
                gBlendCoefficients = ((0xF - gViewportFlags[VF_WHITEOUT_CNT]) << 8) + 0x1F;
                return 0;
            }
        }
    }
    else
    {
        switch (insn[1])
        {
        case SYSFX_SHAKE:
            gViewportFlags[VF_EFFECT_EN] &= 0xFFFC;
            break;

        case SYSFX_FLASH:
            gViewportFlags[VF_EFFECT_EN] &= 0xFFFB;
            REG_DISPCNT &= 0xFDFF;
            break;

        case SYSFX_BGLAYER:
            if (gCurrentMapId == 0x63)
            {
                REG_DISPCNT &= 0xFBFF;
            }
            else
            {
                REG_DISPCNT &= 0xF7FF;
            }
            break;

        case SYSFX_CAMEASE:
            gDrawCamEaseActive = 0;
            break;

        case SYSFX_PALFX_SEQ:
        case SYSFX_OBJPAL_IN:
            gViewportFlags[VF_FADE_FRAME] = 0;
            gViewportFlags[VF_FADE_PHASE] = 0;
            break;

        case SYSFX_SAVE_OPEN:
            gUnk_03004D48 |= 1;
            SaveUi_OpenLoad();
            break;

        case SYSFX_WAIT_A:
            if (gNewKeysRaw & 1)
            {
                Script_Abort(1);
                System_ResetToLogo();
            }
            return;

        case SYSFX_SAVEUI_PAL:
            {
                u16 *flags = gViewportFlags;
                switch (flags[VF_SAVEUI_STEP])
                {
                case SYSFX_SAVEUI_FILL_ROW0:
                    BgMap_FillRow(0);
                    MenuEnt_ParseRange(3, 0x17);
                    MenuEnt_Unlock(3);
                    flags[VF_SAVEUI_STEP]++;
                    return 0;

                case SYSFX_SAVEUI_WAIT_ROW0:
                    if (MenuEnt_GetState(3) != 0)
                    {
                        return 0;
                    }
                    flags[VF_SAVEUI_STEP]++;
                    return 0;

                case SYSFX_SAVEUI_FILL_ROW1:
                    BgMap_FillRow(1);
                    MenuEnt_ParseRange(3, 0x18);
                    MenuEnt_Unlock(3);
                    flags[VF_SAVEUI_STEP]++;
                    return 0;

                case SYSFX_SAVEUI_WAIT_ROW1:
                    if (MenuEnt_GetState(3) != 0)
                    {
                        return 0;
                    }
                    flags[VF_SAVEUI_STEP] = 0;
                    break;
                }
                break;
            }

        case SYSFX_HEAL:
            FullHealCharacter(insn[2]);
            break;

        case SYSFX_BGM_RESUME:
            Bgm_Continue();
            break;

        case SYSFX_MAPBG:
            REG_DISPCNT |= 0x0100;
            break;

        case SYSFX_INTROBG:
            IntroBg_Load(0);
            break;

        case SYSFX_WHITEOUT:
            gViewportFlags[VF_WHITEOUT_CNT] = 0;
            break;
        }
    }
    *pScriptCursor += 3;
    return 1;
}

// @ 0x08050014
void ScriptPump_Run(void)
{
    u16 keys;

    if ((gScriptVmFlags & 1) != 0 && (gScriptVmFlags & 0x200) == 0)
    {
        keys = ~REG_KEYINPUT;
        gScriptKeysPressed = keys & ~gScriptKeyState;
        gScriptKeyState = keys;
        sub_80182A8(gScriptKeyState, gScriptLocalSlots);
        while (gScriptOpcodeHandlers[*(u8 *)gScriptCursor](&gScriptCursor) == 1)
        {
        }
    }
}

// @ 0x0805008C
// 脚本泵的逐帧后台服务: 在脚本活动期间 (bit0=脚本中, bit9=暂停) 按 gScriptVmFlags 的请求位
// 刷新 BG0 滚动/瓦片/LZ 块; LZ 解压完成后把脚本指针跳到解压缓冲 gUnk_02016200 的入口表项。
// 注: ① op==0||0x17 且 bgRequest==0 才走第一 DMA 块, 否则(含非 0/0x17 op)重读 gScriptVmFlags
//     若 bit4 置位走第二 DMA 块 —— `goto block2` 使编译器生成 bne 直跳 E2, 差 1B;
//     ② LZ 表须 `u16 *entryTbl = gUnk_02016000;` 中间指针 (permuter 发现, 修池加载序, 差 4B);
//     ③ bgRequest 必须内联 (抽变量多 1B)。fncheck OK 300B。
void ScriptPump_ServiceFrame(void)
{
    u8 op;
    u16 bgRequest;
    u16 *entryTbl;

    if ((gScriptVmFlags & 1) != 0 && (gScriptVmFlags & 0x200) == 0)
    {
        op = *(u8 *)gScriptCursor;
        if (op == 0 || op == 0x17)
        {
            bgRequest = gScriptVmFlags & 0x10;
            if (bgRequest == 0)
            {
                REG_BG0HOFS = bgRequest;
                REG_BG0VOFS = bgRequest;
                DmaSetUnchecked(3, 0x02005800, 0x0600F800, 0x80000400);
            }
            else
            {
                goto block2;
            }
        }
        if (gScriptVmFlags & 0x10)
        {
        block2:
            REG_BG0HOFS = 0;
            REG_BG0VOFS = 0;
            DmaSetUnchecked(3, 0x02005800, 0x0600F800, 0x80000400);
        }
    }
    if ((gScriptVmFlags & 0x40) != 0 && FlushTileDma() < 0)
        gScriptVmFlags &= ~0x40;
    if ((gScriptVmFlags & 0x100) != 0)
    {
        BgTiles_LoadSet(0);
        gScriptVmFlags &= ~0x100;
    }
    if ((gScriptVmFlags & 0x200) != 0)
    {
        if (LZ_UncompressChunk() == 0)
        {
            if ((gScriptVmFlags & 0x400) != 0)
            {
                entryTbl = gUnk_02016000;
                gScriptCursor = (u32)(gUnk_02016200 + entryTbl[gScriptPendingEntry]);
                gScriptVmFlags &= ~0x400;
            }
            gScriptVmFlags &= ~0x200;
        }
    }
}
// @ 0x080501B8
INCLUDE_ASM("asm/nonmatchings", sub_80501B8);
// @ 0x08050434
INCLUDE_ASM("asm/nonmatchings", sub_8050434);
// @ 0x0805063C
INCLUDE_ASM("asm/nonmatchings", sub_805063C);
// @ 0x08050720
INCLUDE_ASM("asm/nonmatchings", sub_8050720);
// @ 0x080511A0
u8 Op_ScriptReturn(u32 *pScriptCursor)
{
    u8 ret = 1;
    u8 i;
    if (gScriptCallStackDepth != 0)
    {
        gScriptCallStackDepth--;
        *pScriptCursor = gScriptCallStack[gScriptCallStackDepth];
    }
    else
    {
        *pScriptCursor = *pScriptCursor + 1;
        Script_SetEnvSet(gScriptReturnSetId);
        gScriptVmFlags &= 0xFFFE;
        i = 0;
        while (i < gScriptStreamDepth)
        {
            gScriptStreamCursorStack[i] = 0;
            gScriptStreamSetIdStack[i] = 0;
            i++;
        }
        gScriptStreamDepth = 0;
        ret = 0;
    }
    return ret;
}
// @ 0x08051230
u32 Op_ScriptStop(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 i;
    u8 state;
    u32 action;

    pBytecode = (u8 *)*pScriptCursor;
    if ((gScriptVmFlags & 0x200) == 0)
    {
        pBytecode++;
        Script_SetEnvSet(gScriptReturnSetId);
        state = *pBytecode != 0 ? 1 : 3;
        action = state;
        switch (action)
        {
            default:
            case 1:
                gScriptCursor = (u32)gUnk_02016200;
            case 3:
                break;
        }

        gScriptVmFlags &= ~1;
        for (i = 0; i < gScriptStreamDepth; i++)
        {
            gScriptStreamCursorStack[i] = 0;
            gScriptStreamSetIdStack[i] = 0;
        }

        action = 0;
        gScriptStreamDepth = action;
    }
    return 0;
}
// @ 0x080512C4
u32 sub_80512C4(u32 *ptr)
{
    u8 *data;
    u8 entry;
    int chunkSize;
    u8 songId;
    struct LzHeader *lzData;
    u32 uncompSize;
    u16 *entryTbl;
    vu16 *ioReg;

    data = (u8 *)(*ptr);
    gScriptStreamSetIdStack[gScriptStreamDepth] = gScriptReturnSetId;
    gScriptStreamEntry = entry;
    gScriptStreamSetId = songId;
    data++;
    songId = *data;
    data++;
    entry = *data;
    data++;
    gScriptStreamCursorStack[gScriptStreamDepth] = (u32)data;
    chunkSize = 0x400;
    gScriptStreamDepth++;
    gScriptReturnSetId = songId;
    lzData = (struct LzHeader *)gScriptSetTable[songId];
    uncompSize = lzData->uncompressedSize;
    ioReg = (vu16 *)0x04000000;
    if ((*ioReg & 0x80) == 0x80)
    {
        LZ_InitContext((u8 *)0x02016000, lzData, uncompSize);
        LZ_UncompressChunk();
    }
    else
    {
        ioReg = (vu16 *)0x02016000;
        LZ_InitContext((u8 *)ioReg, lzData, chunkSize);
        gScriptVmFlags |= 0x200;
    }
    entryTbl = (u16 *)0x02016000;
    gScriptPendingEntry = entry;
    gScriptVmFlags |= 0x400;
    gScriptCursor = (u32)(0x02016200 + entryTbl[entry]);
    gScriptVmFlags |= 2;
    songId = 0;
    return songId;
}
// @ 0x080513A0
u32 sub_80513A0(u32 *ptr)
{
    struct LzHeader *lz;
    u32 savedCursor;
    u32 size;
    int chunkSize;
    u8 setId;
    vu16 *ioReg;

    gScriptStreamDepth--;
    chunkSize = 0x400;
    gScriptCurSetId = gScriptStreamSetIdStack[gScriptStreamDepth];
    setId = gScriptStreamSetIdStack[gScriptStreamDepth];
    gScriptReturnSetId = setId;
    lz = (struct LzHeader *)gScriptSetTable[setId];
    size = lz->uncompressedSize;
    ioReg = (vu16 *)0x04000000;
    if ((*ioReg & 0x80) == 0x80)
    {
        LZ_InitContext((u8 *)0x02016000, lz, size);
        LZ_UncompressChunk();
    }
    else
    {
        ioReg = (vu16 *)0x02016000;
        LZ_InitContext((u8 *)ioReg, lz, chunkSize);
        gScriptVmFlags |= 0x200;
    }
    gScriptCursor = 0x02016200;
    savedCursor = gScriptStreamCursorStack[gScriptStreamDepth];
    gScriptCursor = savedCursor;
    return 0;
}
// @ 0x0805144C
INCLUDE_ASM("asm/nonmatchings", sub_805144C);
// @ 0x08051A1C
typedef union {
    BgCnt bg;
    u32 word;
} BgUnion;

/* 打开脚本窗口: 用 0xB000 填充窗口背景缓冲并清 VRAM 0x0600F800, 等第二次 DMA,
 * 开 BG0, 把 BG0CNT 配成 CharBase 2 / ScreenBase 31 (终值 0x1F08), 置 gScriptVmFlags bit4,
 * 游标前进 1。
 * 末尾掩码链 = BgCnt 逐字段赋值; ScBasep=31 填满掩码, 故 GCC2 把 `(x & ~0x1F00) | 0x1F00`
 * 简化成 `| 0x1F00` (无前导 AND)。 */
u32 Op_OpenWindow(u32 *pScriptCursor)
{
    BgUnion bg0cnt;

    DmaFill16(3, 0xB000, gWindowBgBuf, 0x800);
    DmaWait(3);
    DmaFill16(3, 0, (void *)0x0600F800, 0x800);
    {
        vu32 *dmaRegs = (vu32 *)REG_ADDR_DMA3;
        u32 status = dmaRegs[2];
        u32 mask = DMA_ENABLE << 16;
        u32 value = *pScriptCursor;

        if ((s32)status < 0)
        {
            do
            {
                status = dmaRegs[2];
            } while (status & mask);
        }
        REG_DISPCNT |= DISPCNT_BG0_ON;
        bg0cnt.bg.Priority = 0;
        bg0cnt.bg.CharBasep = 2;
        bg0cnt.bg.Dummy_5_4 = 0;
        bg0cnt.bg.Mosaic = 0;
        bg0cnt.bg.ColorMode = 0;
        bg0cnt.bg.ScBasep = 31;
        bg0cnt.bg.Loop = 0;
        bg0cnt.bg.Size = 0;
        REG_BG0CNT = bg0cnt.word;
        gScriptVmFlags |= 0x10;
        *pScriptCursor = value + 1;
    }
    return 1;
}
// @ 0x08051AEC
s16 sub_8051AEC(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 mode)
{
    switch ((s8)mode)
    {
        case 0:
            break;
        case 1:
            arg1 = ((float)arg1) * ((((float)arg3) * 10.0f) / ((float)arg2) / 10.0f);
            break;
        case 2:
            arg1 = ((float)arg1) * (((((-10.0f) * ((float)arg3)) / ((float)arg2)) + 20.0f) / 10.0f);
            break;
    }

    return arg0 + ((arg1 * arg3) / arg2);
}
// @ 0x08051BE4
INCLUDE_ASM("asm/nonmatchings", sub_8051BE4);

// @ 0x08052574
u16 Script_GetFlags(void)
{
    return gScriptVmFlags;
}

// @ 0x08052580
void Script_ResetVM(void)
{
    u8 i;

    gScriptCursor = (u32)gUnk_02016200;
    gScriptVmFlags = 0;
    gScriptDialogPhase = 0;
    gDialogWindowTileX = 1;
    gDialogWindowTileY = 0xC;
    gScriptCallStackDepth = 0;
    for (i = 0; i <= 7; i++)
    {
        gScriptCallStack[i] = 0;
    }
    gScriptStreamDepth = 0;
}
// @ 0x080525E8
void ScriptSet_Load(u8 setId, u8 entry, u8 mode)
{
    struct LzHeader *lzData;
    u32 uncompSize;
    u16 *jtbl;

    gScriptReturnSetId = setId;
    lzData = (struct LzHeader *)gScriptSetTable[setId];
    uncompSize = lzData->uncompressedSize;
    if (REG_DISPCNT & 0x80)
    {
        LZ_InitContext((u8 *)0x02016000, lzData, uncompSize);
        LZ_UncompressChunk();
    }
    else
    {
        LZ_InitContext((u8 *)0x02016000, lzData, 0x400);
        gScriptVmFlags |= 0x200;
    }
    jtbl = (u16 *)0x02016000;
    switch (mode)
    {
        case 1:
        default:
            gScriptCursor = 0x02016200;
            break;
        case 2:
            gScriptPendingEntry = entry;
            gScriptVmFlags |= 0x400;
            gScriptCursor = 0x02016200 + jtbl[entry];
            break;
    }
}
// @ 0x080526A0
// 脚本 VM 启动/跳转: 按 arg1 模式设置脚本指针 gScriptCursor, 然后清局部槽并置运行标志。
//   arg1==2 -> 跳到脚本区第 arg0 项入口 (gUnk_02016200 + gUnk_02016000[arg0])
//   arg1==3 -> 保持脚本指针不变
//   其它    -> 复位到脚本区基址 gUnk_02016200
// 之后把 8 个 u16 局部槽 (gScriptLocalSlots) 全置 0xFFFF, 置运行标志 bit0, 清 gScriptDialogPhase。
// 注: base 0x02016200 / 表基址 0x02016000 必须写字面量, 换 gUnk_02016200/gUnk_02016000 符号
//     会改变 case2 块寄存器分配 (差 9B); `= -1` 是窄化 store 的 ldrh/orr/strh 展开形状,
//     写 `|= 0xFFFF` 或经局部指针/强转视图访问都会被折叠成直接 store (字节错)。
void ScriptPump_JumpToEntry(u8 arg0, u8 arg1)
{
    u8 i;
    u16 *jtbl;

    jtbl = (u16 *)0x02016000;
    switch (arg1)
    {
    default:
    case 1:
        gScriptCursor = 0x02016200;
        break;
    case 2:
        gScriptCursor = 0x02016200 + jtbl[arg0];
        break;
    case 3:
        break;
    }
    for (i = 0; i <= 7; i++)
    {
        gScriptLocalSlots[i] = -1;
    }
    gScriptVmFlags = gScriptVmFlags | 1;
    gScriptDialogPhase = 0;
}

// @ 0x08052728
void Script_Abort(u8 arg0)
{

    switch (arg0)
    {
        default:
        case 1:
            *(u32 *)0x03000E6C = 0x02016200;
            *(u16 *)0x03000E70 &= ~1;
            break;
        case 3:
            *(u16 *)0x03000E70 &= ~1;
            break;
    }
}

/* 注意: 原代码里这个 if 是个空转 —— 两个分支结果都是 arg0 = 0。
 * 不能删: 删了 GCC2 就不会生成 cmp/beq + movs 这三条,
 * 直接 `arg0 = 0;` 只会留一条 movs。 */
// @ 0x08052758
void BgTiles_LoadSet(u16 arg0)
{
    if (arg0 != 0)
    {
        arg0 = 0;
    }

    LZ77UnCompVram(gUnk_087ED904[arg0], (void *)0x0600B800);
}

// @ 0x08052780
void TileDma_Reset(void)
{
    u8 i;

    for (i = 0; i <= 0x1D; i++)
    {
        gTileDmaAllocTable[i] = 0;
    }

    gTileDmaCount = 0;
}
// 把待传的图块数据从 EWRAM 暂存区 0x0203DE00 用 DMA3 刷到 VRAM 0x0600B800。
// gTileDmaCount = 待传块数, 每块 64 字节(= 16 个 u32); 无待传项时不发 DMA。
// 调用方按 (s16)返回值 < 0 判定已刷新。同族写法见 DialogCtx_Flush。
// @ 0x080527AC
s16 sub_80527AC(void)
{
    if (gTileDmaCount != 0)
    {
        DmaCopy32(3, 0x0203DE00, 0x0600B800, gTileDmaCount * 64);
        DmaWait(3);
    }
    return -1;
}
// @ 0x080527F4
u32 TileDma_GetCtx(u32 *arg0)
{
    *arg0 = gTileDmaAllocTable;
    return gTileDmaCount;
}

// @ 0x08052808
u32 Op_LoadTileGfx(u8 arg0)
{
    sub_8050434((u32)(arg0 * 18) + (u32)gUnk_0862D574 + gTileAnimFrameIdx * 2, 0x6F1E);
    if (gTileDmaCount != 0)
    {
        gScriptVmFlags |= 0x40;
        return 1;
    }
    return 0;
}
// 脚本 opcode: 无条件把脚本指针改到脚本区 0x02016200 里第 data[1] 项的入口。
// gUnk_02016000[] = 项偏移表(u16), gUnk_02016200 = 脚本数据基址。
// 同族写法见 Script_Call / Op_IfEventFlagJump。
// @ 0x08052858
u32 Op_ScriptJump(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    *pScriptCursor = (u32)(gUnk_02016200 + gUnk_02016000[pBytecode[1]]);
    return 1;
}
// @ 0x08052878
u32 Script_Call(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u16 ofs;

    pBytecode = (u8 *)*pScriptCursor;
    if (gScriptCallStackDepth <= 7)
    {
        gScriptCallStack[gScriptCallStackDepth] = (u32)(pBytecode + 2);
        ofs = *(u16 *)((u32)gUnk_02016000 + pBytecode[1] * 2);
        gScriptCallStackDepth++;
        *pScriptCursor = ofs + (u32)gUnk_02016200;
    }
    else
    {
        *pScriptCursor = (u32)(pBytecode + 2);
    }
    return 1;
}
// @ 0x080528C4
void Op_Nop() { }
// @ 0x080528C8
u32 Op_DialogSetup(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 a1, a2, a3, a4, a5, a6;

    pBytecode = (u8 *)*pScriptCursor;
    a1 = *(++pBytecode);
    a2 = *(++pBytecode);
    a3 = *(++pBytecode);
    a4 = *(++pBytecode);
    a5 = *(++pBytecode);
    a6 = *(pBytecode + 1);
    sub_8019F08(gWindowBgBuf, a1, a2, a3, a4, a5);
    *pScriptCursor += 7;
    if (a6 == 1)
    {
        return 0;
    }
    return 1;
}
// @ 0x0805291C
u32 Op_CloseWindow(u32 *pScriptCursor)
{
    DmaFill16(3, 0xB000, gWindowBgBuf, 0x800);
    DmaWait(3);
    DmaFill16(3, 0, (void *)0x0600F800, 0x800);
    {
        vu32 *dmaRegs = (vu32 *)REG_ADDR_DMA3;
        u32 status = dmaRegs[2];
        u32 mask = DMA_ENABLE << 16;
        u32 value = *pScriptCursor;

        if ((s32)status < 0)
        {
            do
            {
                status = dmaRegs[2];
            } while (status & mask);
        }
        REG_DISPCNT &= ~DISPCNT_BG0_ON;
        gScriptVmFlags |= 0x100;
        *pScriptCursor = value + 1;
    }
    return 0;
}
// @ 0x080529B8
u8 Op_WaitFrames(u8 **pScriptCursor)
{
    u8 *pBytecode = *pScriptCursor;
    u8 ret = 0;
    u8 idx = pBytecode[1];
    pBytecode = 0;
    if ((gScriptVmFlags & 0x20) == 0)
    {
        gUnk_03000E74 = 0;
        gScriptVmFlags |= 0x20;
    }
    else if (gUnk_03000E74 < idx)
    {
        gUnk_03000E74++;
    }
    else
    {
        gUnk_03000E74 = 0;
        gScriptVmFlags &= ~0x20;
        *pScriptCursor += 2;
        ret = 1;
    }
    return ret;
}

// @ 0x08052A14
u32 Op_BgmPlay(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 param1;
    u16 param2;
    u8 new_var;
    pBytecode = (u8 *)(*pScriptCursor);
    new_var = pBytecode[3];
    param2 = pBytecode[2] | (new_var << 8);
    Bgm_Play(pBytecode[1], param2);
    *pScriptCursor += 4;
    return 0;
}

// @ 0x08052A38
u32 Op_BgmStop(u32 *pScriptCursor)
{
    Bgm_Stop();
    (*pScriptCursor)++;
    return 0;
}
// @ 0x08052A50
u32 Op_BgmVolume(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 new_var;

    pBytecode = (u8 *)*pScriptCursor;
    new_var = pBytecode[3];
    Bgm_SetVolume(pBytecode[2] | (new_var << 8));
    *pScriptCursor += 4;
    return 0;
}

// @ 0x08052A70
u32 Op_BgmFadeIn(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    Bgm_FadeIn(param);

    *pScriptCursor += 2;

    return 0;
}
// @ 0x08052A8C
u32 Op_BgmFadeOut(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    Bgm_FadeOut(param);

    *pScriptCursor += 2;

    return 0;
}

// @ 0x08052AA8
u32 Op_SfxPlay(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    Sfx_Play(pBytecode[1], pBytecode[2], pBytecode[3] != 0);
    *pScriptCursor += 4;
    return 0;
}

// @ 0x08052ACC
u32 Op_SfxStop(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    Sfx_StopTrack(param);

    *pScriptCursor += 2;

    return 0;
}
/* 从记录 rec 的 [1]..[2] 号段随机取一值查表 gUnk_02016000, 结果指针写入 *pScriptCursor。
 * 表基址/目标基址必须写成常量地址 (非数组符号): GCC2 对 SYMBOL_REF 会把基址留在 callee-saved
 * r7 不外提重取, 导致 val/max 寄存器分配错位 (差 15B); 常量地址才触发 rematerialize 命中目标。 */
// @ 0x08052AE8
u32 Op_RandomJump(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 diff;
    u16 val;
    u16 *jtbl;

    pBytecode = (u8 *)*pScriptCursor;
    jtbl = (u16 *)0x02016000;
    val = jtbl[pBytecode[1]];
    if (pBytecode[1] < pBytecode[2])
    {
        diff = pBytecode[2] - pBytecode[1];
        val = jtbl[(u8)(pBytecode[1] + Rng_LcgNext() % (diff + 1))];
    }
    *pScriptCursor = 0x02016200 + val;
    return 1;
}
// @ 0x08052B34
u32 Op_ScriptCallAlt(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u16 ofs;

    pBytecode = (u8 *)*pScriptCursor;
    if (gScriptCallStackDepth <= 7)
    {
        gScriptCallStack[gScriptCallStackDepth] = (u32)(pBytecode + 2);
        ofs = *(u16 *)((u32)gUnk_02016000 + pBytecode[1] * 2);
        gScriptCallStackDepth++;
        *pScriptCursor = ofs + (u32)gUnk_02016200;
    }
    else
    {
        *pScriptCursor = (u32)(pBytecode + 2);
    }
    return 1;
}
// @ 0x08052B80
u32 Op_WaitCharsStop(u32 *pScriptCursor)
{
    if (Chara_AnyMoving() == 0)
    {
        (*pScriptCursor)++;
        return 1;
    }

    return 0;
}
// @ 0x08052BA0
u32 Op_LoadCharaGfx(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    if (pBytecode[1] == 0xFF)
    {
        gMoveCmdSetId = pBytecode[2] + (pBytecode[3] << 8);
        BgScroll_LoadFromTable(gMoveCmdSetId);
    }
    else
    {
        SetSlotGfxId(pBytecode[1], pBytecode[2] | (pBytecode[3] << 8));
    }
    *pScriptCursor += 4;
    return 0;
}
// @ 0x08052BE0
u32 Op_LoadCharaPal(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    SetSlotPalId(pBytecode[1], pBytecode[2] | (pBytecode[3] << 8));
    *pScriptCursor += 4;
    return 0;
}
// @ 0x08052C04
u32 Op_WaitSpriteLoad(u32 *pScriptCursor)
{
    if (GetPendingSpriteLoad() == 0)
    {
        (*pScriptCursor)++;
        return 1;
    }

    return 0;
}

// @ 0x08052C24
u32 Op_SceneChangeFade(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    Palette_Backup();
    switch (pBytecode[1])
    {
        case 0:
            gSceneEntryFlag = 0xFF;
            ScreenFx_SetMode(4);
            break;
        case 1:
            gSceneEntryFlag = 0xFF;
            ScreenFx_SetMode(7);
            break;
        case 2:
            gSceneEntryFlag = 0xFF;
            Bgm_FadeOut(0x2E);
            ScreenFx_SetMode(4);
            break;
    }
    *pScriptCursor += 2;
    return 1;
}
// @ 0x08052C90
u32 Op_SceneChangePlain(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    switch (pBytecode[1])
    {
        case 0:
            Palette_Backup();
            /* fall through */
        case 1:
            ScreenFx_SetMode(3);
            break;
        case 2:
            Palette_Backup();
            ScreenFx_SetMode(7);
            break;
    }
    *pScriptCursor += 2;
    return 1;
}
// @ 0x08052CD0
u32 Op_WaitSceneIdle(u32 *pScriptCursor)
{
    if (gScreenTransitionState == 0)
    {
        (*pScriptCursor)++;
        return 1;
    }
    return 0;
}
// @ 0x08052CF0
u32 Op_LoadMap(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    gMapNpcSetId = pBytecode[1];
    gMoveCmdSetId = pBytecode[2] + (pBytecode[3] << 8);
    gSpawnTileX = pBytecode[4];
    gSpawnTileY = pBytecode[5];
    gSpawnFacingDir = pBytecode[6];
    gGameState = GAME_STATE_SCENE_LOAD;
    gVBlankPipelineMode = 1;
    *pScriptCursor = (u32)(pBytecode + 7);
    return 0;
}
// @ 0x08052D4C
u32 Op_IfEventFlagJump(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    if (EventFlags_Test(pBytecode[1] | (pBytecode[2] << 8)) != 0)
    {
        *pScriptCursor = *(u16 *)((u32)gUnk_02016000 + pBytecode[3] * 2) + (u32)gUnk_02016200;
    }
    else
    {
        *pScriptCursor += 4;
    }
    return 1;
}
// @ 0x08052D8C
u32 Op_SetEventFlag(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    EventFlags_Set(pBytecode[1] | (pBytecode[2] << 8));
    *pScriptCursor += 3;
    return 1;
}
// @ 0x08052DAC
u32 Op_ClearEventFlag(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    EventFlags_Reset(pBytecode[1] | (pBytecode[2] << 8));
    *pScriptCursor += 3;
    return 1;
}
// @ 0x08052DCC
u32 Op_IfSwitchJump(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    if (SwitchFlags_Test(pBytecode[1] | (pBytecode[2] << 8)) != 0)
    {
        *pScriptCursor = *(u16 *)((u32)gUnk_02016000 + pBytecode[3] * 2) + (u32)gUnk_02016200;
    }
    else
    {
        *pScriptCursor += 4;
    }
    return 1;
}
// @ 0x08052E0C
u32 Op_SetSwitch(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    SwitchFlags_Set(pBytecode[1] | (pBytecode[2] << 8));
    *pScriptCursor += 3;
    return 1;
}
// @ 0x08052E2C
u32 Op_ClearSwitch(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    SwitchFlags_Reset(pBytecode[1] | (pBytecode[2] << 8));
    *pScriptCursor += 3;
    return 1;
}
// @ 0x08052E4C
u32 Op_CameraSnap(u32 *pScriptCursor)
{

    gCameraSnapFlag = 1;
    gCameraPanDuration = 0;
    (*pScriptCursor)++;
    return 1;
}
// @ 0x08052E6C
s32 Op_CameraFollow(u32 *pScriptCursor)
{

    gCameraSnapFlag = 0;
    (*pScriptCursor)++;
    return 1;
}
// @ 0x08052E80
u32 Op_WaitCameraPan(u32 *pScriptCursor)
{
    if (gCameraPanDuration != 0)
    {
        return 0;
    }
    (*pScriptCursor)++;
    return 1;
}
// @ 0x08052E9C
u32 Op_LoadCutsceneAnim(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    CutsceneAnim_Load(pBytecode[1] | (pBytecode[2] << 8), pBytecode[3], pBytecode[4]);
    *pScriptCursor += 5;
    return 0;
}
// @ 0x08052EC0
u32 Op_RestartCharaAnim(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 v;

    pBytecode = (u8 *)*pScriptCursor;
    if (gActors[pBytecode[1]].sprNodeIdx != 0)
    {
        Sprite_FreeChain(&gSpriteNodePool[gActors[pBytecode[1]].sprNodeIdx]);
    }
    v = Sprite_AllocNode();
    if (v <= 0x6F)
    {
        gActors[pBytecode[1]].sprNodeIdx = v;
        Chara_StartScriptAnim(pBytecode[1], pBytecode[2]);
    }
    *pScriptCursor += 3;
    return 0;
}
// @ 0x08052F20
u32 Op_WaitCharaAnim(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    if (Chara_AnimWaitDone(pBytecode[1]) != 0)
    {
        *pScriptCursor += 2;
        return 1;
    }
    return 0;
}
// 脚本 opcode: 队伍成员计数条件跳转。
//   统计 gPartyMemberIds[0..4] 中等于 data[1] 的成员个数 count。
//   count == data[2] → 脚本指针跳到 gUnk_02016200 + gUnk_02016000[data[3]]
//   否则             → 跳过本指令(4 字节)
// 注: 同族条件跳转见 Op_IfEventFlagJump / Op_IfSwitchJump / Op_IfMoneyJump;
//     跳转表写法必须写成 *(u16 *)((u32)gUnk_02016000 + data[3] * 2) + gUnk_02016200,
//     与 Op_IfEventFlagJump 逐字节同形。
// 注: 循环计数 i 必须是 u16 (目标 lsls/lsrs #0x10), count 必须是 u8 (#0x18);
//     且 i 不能在声明处初始化 —— `for (i = 0; ...)` 自带初始化, 预先写
//     `u16 i = 0;` 会让 GCC2 把 data/count 的寄存器 home 在 r3/r4 互换(差 12B)。
// @ 0x08052F44
u32 Op_IfPartyMemberJump(u32 *pScriptCursor)
{
    u8 *pBytecode = (u8 *)*pScriptCursor;
    u8 count = 0;
    u16 i;

    for (i = 0; i <= 4; i++)
    {
        if (gPartyMemberIds[i] == pBytecode[1])
        {
            count++;
            break;
        }
    }
    if (count == pBytecode[2])
    {
        *pScriptCursor = *(u16 *)((u32)gUnk_02016000 + pBytecode[3] * 2) + (u32)gUnk_02016200;
    }
    else
    {
        *pScriptCursor += 4;
    }
    return 1;
}
// @ 0x08052FAC
u32 Op_LoadAnimSet(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param1 = pBytecode[1];
    u8 param2 = pBytecode[2];

    AnimSlot_LoadSet(param1, param2);

    *pScriptCursor += 3;

    return 1;
}

// @ 0x08052FC8
u32 Op_AnimSlotResume(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    AnimSlot_Resume(param);

    *pScriptCursor += 2;

    return 1;
}
// @ 0x08052FE4
u32 Op_AnimSlotPause(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    AnimSlot_Pause(param);

    *pScriptCursor += 2;

    return 1;
}

// @ 0x08053000
u32 Op_WaitAnimSlotIdle(u32 *pScriptCursor)
{
    u8 *pBytecode = (u8 *)*pScriptCursor;

    if (!AnimSlot_Active(pBytecode[1]))
    {
        *pScriptCursor += 2;
        return 1;
    }

    return 0;
}
// @ 0x08053024
u32 Op_MenuLoadAnims(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param1 = pBytecode[1];
    u8 param2 = pBytecode[2];

    MenuEnt_ParseRange(param1, param2);

    *pScriptCursor += 3;

    return 1;
}
// @ 0x08053040
u32 Op_MenuUnlock(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    MenuEnt_Unlock(param);

    *pScriptCursor += 2;

    return 1;
}

// @ 0x0805305C
u32 Op_MenuLock(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    MenuEnt_Lock(param);

    *pScriptCursor += 2;

    return 1;
}

// @ 0x08053078
u32 Op_WaitMenuReady(u32 *pScriptCursor)
{
    u8 *pBytecode = (u8 *)*pScriptCursor;

    if (!MenuEnt_GetState(pBytecode[1]))
    {
        *pScriptCursor += 2;
        return 1;
    }

    return 0;
}

// @ 0x0805309C
u32 Op_FullHealParty(u32 *pScriptCursor)
{
    FullHealParty();
    (*pScriptCursor)++;
    return 1;
}
// @ 0x080530B4
u32 Op_EquipItem(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    EquipItem(pBytecode[1], pBytecode[2], pBytecode[3]);
    *pScriptCursor += 4;
    return 1;
}
// @ 0x080530D4
u32 Op_GiveTakeItem(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    if (pBytecode[2] > 100)
    {
        sub_800AA84(pBytecode[1], pBytecode[2] - 100);
    }
    else
    {
        Inventory_AddItem(pBytecode[1], pBytecode[2]);
    }

    *pScriptCursor += 3;
    return 1;
}
// @ 0x08053104
u32 Op_SilverAddSub(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    if (pBytecode[1] != 0)
    {
        Silver_Add(pBytecode[2] + (pBytecode[3] << 8));
    }
    else
    {
        Silver_Sub(pBytecode[2] + (pBytecode[3] << 8));
    }
    *pScriptCursor += 4;
    return 1;
}

// @ 0x08053138
u32 Op_IfItemQtyJump(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 index;

    pBytecode = (u8 *)*pScriptCursor;
    index = pBytecode[1];
    if (gInventory[index] <= 0x62)
    {
        *pScriptCursor = (u32)(pBytecode + 3);
    }
    else
    {
        *pScriptCursor = *(u16 *)((u32)gUnk_02016000 + pBytecode[2] * 2) + (u32)gUnk_02016200;
    }
    return 1;
}
// @ 0x0805316C
u32 Op_ChestOpen(u32 *pScriptCursor)
{
    // 读取外部变量的值作为参数
    u8 param = gUnk_03004860;

    ChestObject_Open(param);

    // 递增指针指向的值
    (*pScriptCursor)++;

    return 0;
}
// @ 0x0805318C
u32 Op_SaveUiTrigger(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    SaveUi_Open(param);

    *pScriptCursor += 2;

    return 0;
}
// @ 0x080531A8
u32 Op_IfSaveLoadedJump(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u32 base;
    u32 value;
    u32 index;

    pBytecode = (u8 *)*pScriptCursor;
    if (gSaveBusyA != 0)
    {
        return 0;
    }
    if (gSaveBusyB == 0)
    {
        value = (u32)(pBytecode + 2);
    }
    else
    {
        index = pBytecode[1] * 2;
        base = (u32)gUnk_02016000;
        value = *(u16 *)(base + index);
        base = (u32)gUnk_02016200;
        value += base;
    }
    do
    {
        do
        {
            do
            {
                do
                {
                    do
                    {
                        *pScriptCursor = value;
                    } while (0);
                } while (0);
            } while (0);
        } while (0);
    } while (0);
    return 0;
}
// @ 0x080531E4
u32 Op_SaveTimerA(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    SaveTimer_Inc(param);

    *pScriptCursor += 2;

    return 1;
}
// @ 0x08053200
u32 Op_SaveTimerB(u32 *pScriptCursor)
{
    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    SaveTimer_Dec(param);

    *pScriptCursor += 2;

    return 1;
}
// @ 0x0805321C
u32 Op_IfSaveFlagJump(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    if (SaveTimer_Get(pBytecode[1]) != 0)
    {
        *pScriptCursor += 3;
    }
    else
    {
        *pScriptCursor = *(u16 *)((u32)gUnk_02016000 + pBytecode[2] * 2) + (u32)gUnk_02016200;
    }
    return 1;
}
// @ 0x08053254
u32 Op_SaveOp(u32 *pScriptCursor)
{

    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param = pBytecode[1];

    SaveFlag_Set(param);

    *pScriptCursor += 2;

    return 1;
}
// 脚本 opcode: 遍历脚本数据里的一段 u16 标志号列表, 逐个置位。
//   项数 = data[1] >> 1, 每项 = data[2+2k] | data[3+2k] << 8 (小端拼 u16)
//   号 <= 0x1FF → EventFlags_Set(号)        (置 0x03001C60 位图)
//   号 >  0x1FF → SwitchFlags_Set(号 - 0x200)(置 0x030018F0 位图)
// 最后把脚本指针推过整个列表。Op_ClearFlagsList 的 Set 姊妹。
// 注: 循环条件必须写成 `n > i`(界在左), 否则 GCC2 不会把 i=0 代入入口测试。
//     `off = t + 2;` 必须单独一句(规律30)。
// @ 0x08053270
u32 Op_SetFlagsList(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 t;
    u8 n;
    u32 off;
    u16 v;
    u16 i;

    pBytecode = (u8 *)*pScriptCursor;
    t = pBytecode[1];
    n = t >> 1;
    for (i = 0; n > i; i++)
    {
        v = pBytecode[i * 2 + 2] | (pBytecode[i * 2 + 3] << 8);
        if (v > 0x1FF)
            SwitchFlags_Set(v - 0x200);
        else
            EventFlags_Set(v);
    }
    off = t + 2;
    *pScriptCursor = *pScriptCursor + off;
    return 1;
}
// 脚本 opcode: 遍历脚本数据里的一段 u16 标志号列表, 逐个清除标志位。
//   项数 = data[1] >> 1, 每项 = data[2+2k] | data[3+2k] << 8 (小端拼 u16)
//   号 <= 0x1FF → EventFlags_Reset(号)       (清 0x03001C60 标志位图)
//   号 >  0x1FF → SwitchFlags_Reset(号 - 0x200) (清 0x030018F0 标志位图)
// 最后把脚本指针推过整个列表。
// 注: 循环条件必须写成 `n > i`(界在左), 否则 GCC2 不会把 i=0 代入入口测试,
//     得到 `cmp r4,r0; bcs` 而非目标的 `cmp r0,#0; bls`。
// 注: `off = t + 2;` 必须单独一句(规律30), 写成 `*ptr + t + 2` 会被重结合成
//     `ldr; adds #2; add r8`。
// @ 0x080532DC
u32 Op_ClearFlagsList(u32 *pScriptCursor)
{
    u8 *pBytecode;
    u8 t;
    u8 n;
    u32 off;
    u16 v;
    u16 i;

    pBytecode = (u8 *)*pScriptCursor;
    t = pBytecode[1];
    n = t >> 1;
    for (i = 0; n > i; i++)
    {
        v = pBytecode[i * 2 + 2] | (pBytecode[i * 2 + 3] << 8);
        if (v > 0x1FF)
            SwitchFlags_Reset(v - 0x200);
        else
            EventFlags_Reset(v);
    }
    off = t + 2;
    *pScriptCursor = *pScriptCursor + off;
    return 1;
}
// @ 0x08053348
u32 Op_ClearSwitchTail(u32 *pScriptCursor)
{

    SwitchFlags_ClearRange();
    (*pScriptCursor)++;
    return 1;
}
// 脚本 opcode: 金额条件跳转。
//   操作数: data[1] = 跳转表索引, data[2..3] = 小端 u16 金额阈值
//   银两 > 阈值 → 脚本指针跳到 gUnk_02016200 + gUnk_02016000[data[1]]
//   否则           → 跳过本指令(4 字节)
// 注: 参数不是 ScriptContext 结构体。本文件里所有 Op_* 都是 `u32 Op_xxx(u32 *pScriptCursor)`,
//     pScriptCursor 指向脚本指针本身(即 *pScriptCursor = scriptPtr), 与邻居 Op_IfEventFlagJump 同形。
// @ 0x08053360
u32 Op_IfMoneyJump(u32 *pScriptCursor)
{
    u8 *pBytecode;

    pBytecode = (u8 *)*pScriptCursor;
    if (gSilverAmount > pBytecode[2] + (pBytecode[3] << 8))
    {
        *pScriptCursor = *(u16 *)((u32)gUnk_02016000 + pBytecode[1] * 2) + (u32)gUnk_02016200;
    }
    else
    {
        *pScriptCursor += 4;
    }
    return 1;
}

// @ 0x080533A0
u32 Op_StartLogoFade(u32 *pScriptCursor)
{

    gLogoEffectState = 1;
    (*pScriptCursor)++;
    return 0;
}
// @ 0x080533B4
u32 Op_WaitLogoFade(u32 *pScriptCursor)
{
    if (gLogoEffectState == 0)
    {
        *pScriptCursor += 1;
        return 1;
    }
    return 0;
}
// @ 0x080533D4
u32 Op_SetCharacterLevel(u32 *pScriptCursor)
{
    u8 *pBytecode = (u8 *)*pScriptCursor;

    u8 param1 = pBytecode[1];
    u8 param2 = pBytecode[2];

    sub_800A3C8(param1, param2);

    *pScriptCursor += 3;

    return 1;
}
