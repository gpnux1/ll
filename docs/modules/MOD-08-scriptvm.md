# MOD-08 脚本 VM / opcode 处理器 (0x0804F0B8-0x080533F0)

> 分析人: plan (2026-08-31)。源文件 `src/code_804F0B8.c` (1109 行)。
> 102 函数: 74✅ / 28❌。已应用语义名。

## 模块总职责

**脚本虚拟机**: 游戏事件/剧情由字节码驱动, 每条 opcode 一个 handler,
统一签名 `u32 handler(u32 *ptr)` — `*ptr` 是脚本指针 (指向 `gUnk_02016200` 脚本数据区),
handler 读 `data[1..]` 参数、执行动作、推进 `*ptr`、返回 1=继续执行 / 0=本帧等待 (等动画/转场完成)。

**核心机制**:
- 分发表 `gScriptOpcodeHandlers[]` (原 gUnk_0862D434, ROM, `u32 (*)(u32 *)`): 下标 = `*(u8*)gScriptCursor` (opcode 号)
- 脚本 PC 槽 `gScriptCursor` (原 gUnk_03000E6C, 0x03000E6C u32): 存当前 opcode 字节地址; 主泵每帧 `gScriptOpcodeHandlers[*(u8*)gScriptCursor](&gScriptCursor)`
- 主泵 `ScriptPump_Run` (每帧由 Task_MapExplore 末尾调用): 采样按键 → `while(分发表[opcode](&ptr) == 1)` 循环执行, 遇 0 让出
- `gUnk_03000E70` = VM 状态位图: bit0=运行中, bit4(0x10)=窗口BG拷贝请求 (Op_OpenWindow 置位, sub_805008C 消费: BG0 滚动清零+0x800B 拷贝), bit6(0x40)=tile 传输待刷 (FlushTileDma 完成后清), bit8(0x100)=关窗后 BG 色块重载请求 (Op_CloseWindow 置位, BgTiles_LoadSet(0) 消费; 旧记 "bit9=窗口关闭后?" 系 bit 编号偏差), bit9(0x200)=LZ 流式解压进行中 (主泵暂停), bit10(0x400)=LZ 完成后 PC 跳解压缓冲入口
- 调用跳转表 `gUnk_02016000[]`(u16 偏移表) + `gUnk_02016200`(脚本数据基址) 实现 jump/call

## VM 基础设施

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x08050014 | ✅C | `ScriptPump_Run` | 脚本主泵 (见上); 按键边沿存 gUnk_03000F2E/03000F2C 并传给 sub_80182A8 |
| 0x0805008C | ✅C | `sub_805008C` | 逐帧后台服务 (VBlank_UpdateGameScreen 末尾调用, ScriptPump_Run 姊妹): 按 E70 状态位依次消费 bit4 窗口拷贝 (opcode∈{0,0x17} 时即便无请求位也主动做一次) / bit6 FlushTileDma / bit8 BgTiles_LoadSet(0) / bit9 LZ_UncompressChunk, 解压完且 bit10 时 PC=gUnk_02016200+gUnk_02016000[gUnk_03000E69] (gUnk_03000E69=脚本槽索引, 新符号) |
| 0x08052580 | ✅C | `Script_ResetVM` | VM 复位: 指针=02016200, 状态=0, gUnk_03000ECB=1/03000ECC=0xC, gUnk_03000E78(调用栈深)=0, gUnk_03000E80[8](返回地址栈)=0, gUnk_03000ECA=0 |
| 0x08052574 | ✅C | `Script_GetFlags` | 返回 gUnk_03000E70 (调用方用 bit0 判断脚本忙) |
| 0x080526A0 | ✅C | `sub_80526A0` | 脚本 VM 启动/跳转: arg1==2 → 指针=gUnk_02016200+gUnk_02016000[arg0], arg1==3 不动, 其它复位到 gUnk_02016200; 然后清 8 个 u16 局部槽 gScriptLocalSlots[0..7]=0xFFFF + 置 E70 bit0 + 清 gUnk_03000E72 (gScriptLocalSlots = gUnk_03000ED8 的 u16[] 视图别名, 新符号) |
| 0x0804FA94 | ✅C | `sub_804FA94` | 脚本条件跳转 (任一 flag 置位版, 镜像 804F974): 遍历 data[1]/2 个 u16 flag id (≤0x1FF 查 EventFlags_Test 否则 SwitchFlags_Test(id-0x200)), 任一真 → 指针=gUnk_02016200+gUnk_02016000[data[2]], 全假 → 指针推进 data+count+3; 经 gUnk_0862D434 函数指针表调用 |
| 0x08052728 | ✅C | `Script_Abort` | arg0=1: 指针复位+停; 3: 仅停 |
| 0x08051230 | ✅C | `Op_ScriptStop` | opcode: 停 VM, 清 gUnk_03000EA0[]/03000EC0[](数量=03000ECA), 非 0x200 模式指针复位 |
| 0x0804F210 | ✅C | `SioBattle_ResetState` | 清 gUnk_03000DF0[0..4] + 03000E04/03000E05 (通信对战状态) |
| 0x0804F244 | ✅C | `SioBattle_GetState` | 返回 03000E04 |
| 0x0804F250 | ✅C | `SioBattle_ClearSlots` | 清 gUnk_03000E08[10] 每项前 2 字节 + 03000E30 |
| 0x0804F768 | ✅C | `Op_RemovePartyMember` | 从 gPartyMemberIds/gBattleFormationIds 移除 data[1] 并压缩 |
| 0x08052780 | ✅C | `TileDma_Reset` | 清 gUnk_03000EE8[0x1E] + 03000F24(待传块数) |
| 0x080527AC | ✅C | `FlushTileDma` (已有名) | 0x0203DE00→0x0600B800 DMA, 03000F24×64B |
| 0x080527F4 | ✅C | `TileDma_GetCtx` | *arg0=0x03000EE8; 返回 03000F24 |
| 0x08052758 | ✅C | `BgTiles_LoadSet` | LZ77 gUnk_087ED904[arg0]→0x0600B800 (含空转 if, 见源码注释) |
| 0x0805291C | ✅C | `Op_CloseWindow` | 关窗口: 清 BG0 显示, 状态\|=0x100 |
| 0x08051A1C | ✅C | `Op_OpenWindow` | 开窗口: WindowBgBuf 填 0xB000, BG0CNT=charbase2/screenbase31, 状态\|=0x10 |
| 0x08051AEC | ✅C | `sub_8051AEC` | 浮点插值助手 (同 sub_801768C 家族): base + step*amp*f(step/total,mode)/total; 无调用点 (疑似死代码); 结果复用参数 arg1 (经验 173) |
| 0x0804F0B8 | ✅C | `CheckObjectKindSlot` (已有名) | 死代码 (无调用点) |
| 0x0804F10C/17C/280/64C/7F8/F8D8/974/FA04/FB24/80501B8/805063C/8050720/80511A0/80512C4/80513A0/805144C/8051BE4/80525E8/80529B8/8052AE8/8052F44/8053270 | ❌ | (待匹配) | 804F280=大型角色控制 opcode (调 Chara_SetGfxPal/FreeSprite/StartScriptAnim/AnimWaitDone/Chara_SetWalkPath); 80525E8=BGM/脚本装载入口(场景加载/NewGame 调用; 80526A0 已单独 ✅ 见上); 8052F44=队伍成员条件跳转; 8053270=循环指令 |
| 0x08050434 | ⏸ | `sub_8050434` | TileDma 请求收集器 (对话字模装载第一步): 扫 entry 的 u16 tile 表 (v=tile&0xFF00), v>0x6E6 时 0x800=id 终止/0xF00 终止/其他收集去重进 gUnk_03000EE8, v∈[0x6E3,0x6E6] 跳过, v<0x6E3 收集; F24>0x1D 硬停; 尾部从 gUnk_087ED904[1] (Set141 字模 LZ) 双 32B DmaCopy16(3) 炸开到 0x0203DE00 (row/col 推导, src2=src1+0x400); 调用者 Op_LoadTileGfx(0x6F1E)/sub_804AC60(0x4F1E); 候选 456B/488B 差, 语义全解见 progress.md (2026-09-08) |

## opcode 处理器 (真 C, 按功能分组)

### 转场/音频 (callee 在 sound.c, 已有名)

| 地址 | 语义名 | 动作 |
|---|---|---|
| 0x08052A14 | `Op_BgmPlay` | `Bgm_Play(data[1], data[2]\|data[3]<<8)` |
| 0x08052A38 | `Op_BgmStop` | `Bgm_Stop()` |
| 0x08052A50 | `Op_BgmVolume` | `Bgm_SetVolume(data[2]\|data[3]<<8)` |
| 0x08052A70 | `Op_BgmFadeIn` | `Bgm_FadeIn(data[1])` |
| 0x08052A8C | `Op_BgmFadeOut` | `Bgm_FadeOut(data[1])` |
| 0x08052AA8 | `Op_SfxPlay` | `Sfx_Play(data[1], data[2], data[3]!=0)` |
| 0x08052ACC | `Op_SfxStop` | `Sfx_StopTrack(data[1])` |
| 0x08052CD0 | `Op_WaitSceneIdle` | 等 gSceneSubState==0 |
| 0x080533A0 | `Op_StartLogoFade` | gLogoEffectState=1 |
| 0x080533B4 | `Op_WaitLogoFade` | 等 gLogoEffectState==0 |

### 场景/角色

| 地址 | 语义名 | 动作 |
|---|---|---|
| 0x08052CF0 | `Op_LoadMap` | 设 gMapNpcSetId/gMoveCmdSetId/gSpawnTileX/Y/FacingDir + state=2 + VBlankPipelineMode=1, 指针+7 |
| 0x08052C24 | `Op_SceneChangeFade` | sub_8009B44(存调色板) + gSceneEntryFlag=0xFF + FadeIn(4/7) 三样式 |
| 0x08052C90 | `Op_SceneChangePlain` | 同类, case0 贯穿 case1 |
| 0x08052B80 | `Op_WaitCharsStop` | 等 Chara_AnyMoving()==0 |
| 0x08052BA0 | `Op_LoadCharaGfx` | data[1]==0xFF→gMoveCmdSetId+Bgm 加载; 否则 SetSlotGfxId |
| 0x08052BE0 | `Op_LoadCharaPal` | SetSlotPalId |
| 0x08052C04 | `Op_WaitSpriteLoad` | 等 GetPendingSpriteLoad()==0 |
| 0x08052E9C | `Op_LoadCutsceneAnim` | `CutsceneAnim_Load(data16, data[3], data[4])` |
| 0x08052EC0 | `Op_RestartCharaAnim` | 释放角色精灵链→重分配→Chara_StartScriptAnim |
| 0x08052F20 | `Op_WaitCharaAnim` | 等 Chara_AnimWaitDone |
| 0x08052FAC | `Op_LoadAnimSet` | `LoadSpriteAnimSet(data[1], data[2])` |
| 0x08052E4C/2E6C/2E80 | `Op_CameraSnap` / `Op_CameraFollow` / `Op_WaitCameraSnap` | 相机吸附开关 (gCameraSnapFlag + gUnk_030047B4) |

### 标志/跳转

| 地址 | 语义名 | 动作 |
|---|---|---|
| 0x08052D4C | `Op_IfEventFlagJump` | EventFlags_Test(data16)→跳 |
| 0x08052D8C | `Op_SetEventFlag` | +3 |
| 0x08052DAC | `Op_ClearEventFlag` | +3 |
| 0x08052DCC | `Op_IfSwitchJump` | SwitchFlags_Test(data16)→跳 |
| 0x08052E0C | `Op_SetSwitch` | +3 |
| 0x08052E2C | `Op_ClearSwitch` | +3 |
| 0x080532DC | `ScriptClearFlags` (已有名) | 批量清标志列表 |
| 0x08053348 | `Op_ClearSwitchTail` | SwitchFlags_ClearRange |
| 0x08053360 | ✅asm `Op_IfMoneyJump` | gSilverAmount > data16 → 跳 (源码附 C) |
| 0x08053138 | `Op_IfItemQtyJump` | gInventory[data[1]]>0x62 → 跳 |
| 0x080530D4 | `Op_GiveTakeItem` | data[2]>100→RemoveInventoryItem(-100) 否则 Add |
| 0x08053108* | `Op_ItemFlagA`/`Op_ItemFlagB` | sub_800AAA4/800AAC0 = gSilverAmount 增减(见 0x08053104) |
| 0x08053104 | `Op_SilverAddSub` | data[1]?+:- |
| 0x080531A8 | `Op_IfSaveLoadedJump` | 存档状态门 (gUnk_03004DC4/03004DD8) — **匹配卡壳见 progress.md** |
| 0x080531E4/8053200 | `Op_SaveTimerOp` ×2 | sub_80140D0 / sub_8014124 (存档计时器半字节) |
| 0x0805318C | `Op_TriggerSaveUi` | sub_8015FB4(data[1]) (MOD-03 存档 UI) |
| 0x0805321C | `Op_IfSaveFlagJump` | sub_8015F50(data[1])→跳 |
| 0x08053254 | `Op_SaveOp5254` | sub_8015F74(data[1]) |
| 0x0805316C | `Op_ChestOp` | sub_800908C(gUnk_03004860) (开宝箱音效+状态) |
| 0x080533D4 | `Op_CallA3C8` | sub_800A3C8(data[1], data[2]) |

### 精灵动画/杂项 (MOD-02 callees)

0x08052FC8→sub_8008BFC(AnimSlot_Resume), 0x08052FE4→sub_8008BE4(AnimSlot_Pause),
0x08053000→wait !sub_8008C14(AnimSlot_Active), 0x08053024→sub_8009AC4, 0x08053040→sub_8009B04,
0x0805305C→sub_8009B1C, 0x08053078→wait !sub_8009B34, 0x0805309C→FullHealParty, 0x080530B4→sub_800A9C0(EquipItem),
0x08052B34(✅asm)/0x08052AE8(❌)/0x08052858(`ScriptGotoEntry` 已名)/0x08052878(`Script_Call`)/0x080528C8(`Op_DialogSetup`)/0x0805291C 见上。

## 未匹配 21 个清单

804F280(大型角色控制), 804F64C, 804F7F8, 804F974, 804FA04,
805008C, 80501B8, 8050434, 805063C, 8050720, 80511A0, 80512C4, 80513A0, 805144C, 8051BE4,
80525E8(BGM/脚本装载), 80529B8, 8052AE8, 8052F44(队伍条件跳转), 8053270(循环)
+ 已匹配 asm: 8052B34, 8053360(Op_IfMoneyJump)。
+ 已匹配真C: 80526A0 —— 脚本 VM 启动/跳转: 前任 score 25 是 extern 池重定位假分 (经验 164),
  去 m2c no-op 后人工形态直接过; `= -1` 须 extern u16[] 视图才出 orr 展开 (经验 165),
  新别名 gScriptLocalSlots; fncheck OK 136B。
+ 已匹配真C: 804FA94 —— 任一 flag 置位条件跳转: 前任 score 10 = extern 池假分 + bl 伪影,
  指令流本已全对; 跳转路径须 tbl 提升变量+字面量基址在前防跨跳合并 (经验 166);
+ 已匹配真C: 0x0804FB24 `Op_SysEffect` —— 脚本 VM 万能系统/屏幕特效 op (opcode 0x4D, 3 字节
  `4D <subop> <arg>`): 按 insn[2] 分执行(arg!=0)/复位(arg==0)两族再按 insn[1] 分派 —
  抖动(gViewportFlags[VF_EFFECT_EN]/[VF_SHAKE_MASK])/白闪(EN|=4)/开 BG 层(mapId==0x63→BG2 否则 BG3)/
  相机缓动(gDrawCamEaseActive)/5 步调色板渐变序列(VF_FADE_PHASE/FRAME/LONG_CNT 计数,
  DMA gFlashFxPaletteTable[0]→OBJ bank4, 清 gCutsceneGfxBuf→BG PLTT)/存档菜单(Save_Fsm+SaveUi_OpenLoad)/
  OBJ 调色板 10 帧渐显(DMA gObjPalFadeInSteps[0..9]→0x05000002+.., 末帧 gObjPalFadeInFinal→0x05000140)/
  满血复活/BGM 恢复/整图装载(MapBg_LoadFull(0x81+arg), IntroBg_Load)/白化淡入淡出(BLD 0x1E41)/
  等 A 键软复位(subop 6 SYSFX_WAIT_A, gNewKeysRaw&1 → Script_Abort(1)+System_ResetToLogo, 未按 return 不推进 PC)。
  分发表 slot 0x4D; fncheck OK 1264B (2026-09-07 sensenova 匹配; 同日 claude-opus-4-6 语义重命名)。
  代码可读性 (2026-09-07): 子命令/渐变步/抖动档/存档UI步 4 个枚举 (SysFxSubOp/SysFxPalStep/
  SysFxShakeMask/SysFxSaveUiStep, 定义在 code_804F0B8.c), gViewportFlags 下标枚举 ViewportFlagIdx
  (iwram.h, VF_EFFECT_EN..VF_FADE_FRAME); 局部 ppScriptCursor/insn/fadeStep/palDst;
  gUnk_030047F4→gPaletteFxPhase (PaletteEffects_Update 配套相位计数)。
  ⚠ 坑: `palDst=(u16*)0x02020000` 不能换 gCutsceneGfxBuf 符号 — 地址常量会触发 GCC2
  循环不变量保存 r4 (差 4B), 同 sub_80526A0 的 0x02016200 字面量坑。
  原型 void() → u32(u32*) (无调用方零风险); fncheck OK 144B (2 bl 忽略)。
+ 已匹配真C: 804F10C —— 搜索函数: GetObjPool 空闲槽里找首个 sub_804E76C 命中者, 返内部下标或 -1
  (sub_80489E8 先筛 sub_8045F10==2 的槽); 需 int idx + s8 tmp 中间变量复现截断调度, fncheck OK 110B。
+ 已匹配真C: 804F17C —— 收集版姊妹: 全命中 sub_804E76C 的槽下标写入 out[] 并返数量, 死代码无调用点, fncheck OK 148B。
+ 已匹配真C: 804F8D8 —— gAfterBattleCounter 状态机: 0→初始化置主游戏状态5返0, 3→测 sub_80187B4()&0x40 或跳转表; goto 强制冷路径布局, fncheck OK 156B。
