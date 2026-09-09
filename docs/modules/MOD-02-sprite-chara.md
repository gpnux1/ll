# MOD-02 精灵动画/地图资源/存档底层 (0x08005020-0x08010F10)

> 分析人: plan (2026-08-31)。源文件 `src/code_8005020.c` (2304 行) + `save.c` + `agb_sram.c`。
> 148 函数: 102✅ / 46❌。

## 模块总职责

地图场景的资源与角色渲染地基: 精灵动画槽 (Unk_030046A0[16])、地图 tile 缓冲、
调色板淡入淡出、宝箱、菜单 UI 精灵 (0x03000058 界面实体)、PlayerStats 属性系统、
背包/金钱、遇敌后的 SRAM 存取。以及 **save.c** (SRAM 存档状态机) + **agb_sram.c** (官方 SRAM 库)。

## 关键数据结构

- `Unk_030046A0[16]` 精灵动画槽 (sub_8007964 填充): field_0=模型库(1/2), field_1=帧数, field_2=分频移位, field_3=标志(bit1=暂停,bit0=单次), field_6/7=目标图块偏移, field_8/9=单帧宽高(块), field_A=帧计数, field_C=帧数据指针
- `gUnk_03000010[?]` 菜单实体状态 (sub_8009B84 从 ROM 表装载), 0x03000058 = 菜单 UI 实体[15] (x/y/statusFlags/oamSlot)
- `PlayerStats gUnk_03004AC0[?]`: hp/max_hp/mp/max_mp, base_* + equip_* → atc/def/agl/men/res/noa/luc, equip_slot1-6, skills, lv
- SRAM 布局: 15 个存档槽 × 0x800B @ 0x0E000000+slot<<11, 镜像 0x02021000/02023000/02025000/02027000 (save.c)

## 子系统 A: 精灵动画槽

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x08007964 | ✅C | `AnimSlot_Parse` | 解析一条动画模型记录进槽 (帧数/分频/宽高/帧数据指针) |
| 0x080079BC | ✅C | `AnimSlot_ParseLoop` | 同上, field_A 初值=(1<<field_2)-2 (循环对齐) |
| 0x08007A1C | ✅C | `AnimSlot_Step` (UpdateSpriteAnim 已名) | 推进帧计数, 当前帧图块拷入 0x02006000 缓存 (源码含完整注释) |
| 0x08008A3C | ✅C | `AnimSlots_Release` | 16 槽 field_0=0 |
| 0x08008A60 | ✅C | `AnimSlots_StepAll` | 逐槽 AnimSlot_Step |
| 0x08008BA4 | ✅C | `LoadSpriteAnimSet` (已有名) | 解析 gUnk_087EA1A0[setId] 整组 (count 条) 从 startSlot 起 |
| 0x08008BE4 | ✅C | `AnimSlot_Pause` | field_3 \|= 2 |
| 0x08008BFC | ✅C | `AnimSlot_Resume` | field_3 &= 0xFD |
| 0x08008C14 | ✅C | `AnimSlot_Active` | 返回 field_0 |
| 0x08008DF8 | ✅C | `AnimSlot_PlayOnce` | count 条记录逐条 Parse+Step 后停 (一次性动画) |
| 0x08007350 | ❌ | (待匹配) | 整组装入动画组 (LoadSpriteAnimSet 的槽 0 起始版) |

## 子系统 B: 地图 tile/调色板/显示资源

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x0800526C | ✅C | `BlendRegs_Update` | VBlank 混合寄存器刷新 + 剧情淡入淡出推进 (gScriptLockFlags 位图) |
| 0x08005B2C | ✅C | `MapTile_At` | 屏幕坐标→地图 tile 指针 (0x02004000/0x02004800 双缓冲, 视口裁剪) |
| 0x08005BB4 | ✅C | `MapTile_CollisionBits` | 采样 3×2 tile 的碰撞位 (对比 gUnk_030047AC 阈值) |
| 0x080064AC | ✅C | `BgMap_FillPattern` | 0x02005000 填 0xA000 花纹/纯色 (两模式) |
| 0x08006520 | ✅asm | `MapBg_LoadFull` | (附 C 注释) 地图整背景装载: LZ77 分块→0x02020000→DMA 0x06000000 + 调色板 |
| 0x08007D5C | ✅asm | `MapBg_LoadInterior` | (附 C 注释) 室内地图全资源装载 (Huff/LZ77 多块+BG 寄存器+波形表) |
| 0x08007FB8 | ✅asm | `MapBg_LoadInteriorPart2` | 上一函数的延续 |
| 0x08008A80 | ✅C | `BgTiles_LoadUiSet` | LZ77 装载 UI 图块组 (0x06008000-0x0600B000 + 调色板) |
| 0x08008CC0 | ✅asm | `SpawnTables_Skip` | (附 C 注释) 跳过 gUnk_08087658 出生点表前 N 组, 读出本组出生坐标 |
| 0x08008788 | ✅asm | `IntroBg_Load` | (附 C 注释) 开场/过场整屏图装载 id: gIntroBgTiles[id][0] (3KB tile组, 可选 [1] 8-tile 动画组) →0x02020000→CBB0, gIntroBgPalettes[id] (0x40B, 双 16 色库) →PLTT, gBgPalBackdropWhite→PLTT[0], gIntroBgMaps[id] (32x20 tilemap) →SBB3; gIntroBgTransferStage/gIntroBgTileSetIndex 暂存态, pipeline mode 6 |
| 0x08008E94 | ✅C | `MapBg_FlushPending` | 按 gIntroBgTransferStage/gIntroBgTileSetIndex 把 0x02020000 暂存 DMA 进 VRAM (tiles→0x06000000/06000C00, tilemap→0x0600E000) |

### ScreenIdleIcons 画面收藏系统 (开场图回想)

进入地图 `MapGroup_Lookup` 后由地图装载 `MapScene_Load` 置位 `gScreenIdleEventFlags[mapId>>3]` 的对应 bit
(特例: 地图 0x78 的 bit13 由事件标志 0xFD 解锁, `ScreenIdleIcons_BuildList` 里合并)。
菜单场景初始化 `sub_800ACC8` 调 `ScreenIdleIcons_BuildList` 打包成列表;

| 符号/函数 | 地址/位置 | 语义 |
|---|---|---|
| `gScreenIdleIconPageMap` | 0x080870DC, 16B | bit→地图 ID 对照表: {1,4,8,0x10,0x17,0x18,0x1B,0x1D,0x20,0x3D,0x21,0x28,0x2C,0x78,0x2F}, [15]=0 结束符 |
| `gMenuEntityPaletteTable` | 0x0808A234, 3968B | 124 项 × 0x20B 的 OBJ 调色板记录; 每项首半字保留, VBlank 刷新从 +2 DMA 15 色 |
| `gScreenIdleEventFlags` | 0x03004624, 2B | 已看地图位图 (bit 0..14); 引擎启动清零 (code_80002A0.c) |
| `gScreenIdleIconIds` | 0x030001F0, 16B | 列表产物: 已看地点 ID (0 补满), 0 结尾 |
| `gScreenIdleIconCursor` | 0x03000200 | 菜单内游标 (sub_800C2F8 B/↓ 翻动, ≤9) |
| `ScreenIdleIcons_BuildList` 0x08010978 | ✅C | EventFlags_Test(0xFD)→bit13; 按 bit 收集 PageMap→Ids, 0 补满, 游标=0 |
| sub_800C2F8 (待匹配) | 0x0800C2F8 | 菜单总控: Ids[Cursor+5] 非 0 时画第 2 页图标 (sub_8016758); Enter 组合翻页 |
| sub_800F3AC (待匹配) | 0x0800F3AC | 逐项绘制: 遍历 Ids, 图标 8→0x10D 事件检查, 0x18→事件 0xFF 检查 |
| sub_8016758 | 0x08016758 ✅C | 图块绘制 (Id 作为页选择, 已名) |

图像数据: 每个地点 ID ↔ `gIntroBgMaps`/`gIntroBgPalettes`/`gIntroBgTiles` 的 19 张 256x160 整屏图 (见 IntroBg 节)。
| 0x08008F28 | ✅C | `ChestObjects_LoadForMap` | 按 mapId 扫描 0x08088400 的 256×8B 宝箱表，装载最多 16 个 `ChestObject` 到 gChestObjects |
| 0x08008E44 | ✅C | `BgMap_FillRow` | 0x020053A8 填 0xA200 行 (arg0 选值) + gViewportFlags[13]=1 |
| 0x08008D18 | ✅C | `BgTile_PatchFlush` | LZ77 gUnk_030047CC→0x0600D000+(n-1)/2*0x800 + 调色板 DMA |
| 0x08008D78 | ✅C | `Camera_GetDrawOffset` | 按 gCameraDrawMode 返回绘制偏移 |
| 0x080088B4 | ✅C | `FadeScript_Start` | gScriptLockFlags=arg0 + 淡入淡出参数 (047A8/047F0/04834) |
| 0x080088F4 | ✅C | `AnimSlot_BankReload` | 0x0600E000 段两银行重载 (gVramBufferPointers 切换) |
| 0x08008978 | ✅C | `HBlank_WaveDma` | VCOUNT 波形 → DMA0 到 0x04000040 (背景波浪) |
| 0x080089E0 | ✅C | `ScreenFx_SetMode` | 窗口/黑屏模式切换 (gSceneSubState=arg0, sub_8009428 派生) |
| 0x08008DD8 | ✅C | `BgPal_ResetFirst` | 调色板[0] 复位 |
| 0x08009428 | ✅asm | `PaletteFx_Apply` | (附 C 注释) 调色板特效 (白闪/黑闪/恢复, 0x0203E600 暂存) |
| 0x08009B44 | ✅C | `Palette_Backup` | PLTT→0x0203EA00 |
| 0x08009B64 | ✅C | `Palette_FillWhite` | 调色板全白 |
| 0x080091C4 | ❌ | (待匹配) | (VBlank 调用族) |
| 0x08005020 | ❌ | `VBlank_UpdateSpriteAndWindow` | VBlank 精灵/调色板传输后，推进 WIN0H 虹膜过渡并生成 81 行边界表 |
| 0x080051D0/52F8/53B4/55E8/5C70 | ❌ | 51D0=`ScreenTransition_UpdateBlend`, 55E8=`MovePlayer`(#define 别名已应用), 5C70=精灵帧辅助; 其余待匹配 | MovePlayer(&gCameraTargetX,&gCameraTargetY,dirCode,speed): 按 `gWalkDirVectors`(0x080871C6, dir 0..8→s16 (dx,dy) 单位向量, 1=上顺时针) 步进, MapTile_At/CollisionBits 碰撞 + 8 方向滑动 switch + Actor[2..19]/gChestObjects[16] 重叠检查; 命中区域时 MapZone_FindAt→MapZone_Trigger |
| 0x08007ADC/7BD0 | 7BD0 ✅C / 7ADC ⏸ | `MapZone_FindAt` / `MapZone_Trigger` | 地图区域触发系统: FindAt 算 (x,y) 16×16 足迹的 ≤4 个瓦片坐标 (gZoneCheckTileXs/Ys) 并在 `gMapZoneHeader`[0] 的 cells 表 {count, [4B]{xTile,yTile,type,entryIdx}} 查命中 → gMapZoneType/gMapZoneEntryIdx; Trigger 按 type 0..4 分发 header[1..5] 记录表: 0=换图(gMapNpcSetId/gSpawnTileX/Y/gSpawnFacingDir/gMoveCmdSetId, state=3, SwitchFlags_ClearRange) 1=图内传送(state=4) 2=state=8(gChoiceGroupIdx/gChoiceSubIdx) 3=首次进入跑脚本(2B 记录, SwitchFlags_Test 门) 4=A键+朝向触发脚本(4B 记录). 头表来源: MapScene_Load 装载 `0x087EBB20[mapIdx]`. **Trigger 已真C匹配** (fncheck 396B OK); FindAt 指令流全对仅剩 home (progress.md) |
| 0x0800661C | ❌ | `MapScene_Load` | 地图场景资源/状态装载入口: 查 `gMapSceneDescriptors[arg0]`，装载 BG/OBJ 资源、窗口/调色板及菜单实体状态 |
| 0x080071EC | ❌ | `MapScene_LoadNpcSlotIds` | 按场景描述符的 `npcSlotGroupId` 选择 NPC 槽组，写入槽 2..9 的图形/调色板 ID |
| 0x08007350 | ❌ | (待匹配) | 整组装入动画组 |
| 0x08009370 | ❌ | `MenuEnt_FlushPalettes` (建议名) | VBlank 调色板刷新: 遍历 4 个菜单实体, 按 0x20B 调色板索引从 `gMenuEntityPaletteTable + 2` DMA 到 OBJ PLTT |

### IntroBg 开场/过场整屏图数据 (与 IntroBg_Load 配套)

| 符号 | 地址/大小 | 结构 | 消费者 |
|---|---|---|---|
| `gIntroBgTiles` | 0x087E9828, 152B | 19×{主3KB tile组 LZ77, 可选8-tile动画组 LZ77} (全 19 图主块均 3072B) | IntroBg_Load → LZ77UnCompWram→0x02020000→0x06000000 / 0x06000C00 |
| `gIntroBgMaps` | 0x087E98C0, 76B | 19×tilemap LZ77 (均解出 1280B = 32x20 半字) | IntroBg_Load → 0x0600E000 (SBB3, REG_BG3CNT=0x3C03) |
| `gIntroBgPalettes` | 0x08086C1C, 0x4C0B | 19×0x40B = 每图 2 个 16 色库: 图 0-9 仅 bank0 色 0 单色底, 图 10-18 bank0 为 16 级渐变; bank1 均 15 级渐变 | IntroBg_Load → PLTT[0..31] |
| `gBgPalBackdropWhite` | 0x08087216 | u16 0x7FFF (白), 后 4B 对齐填充 | IntroBg_Load / BgPal_ResetFirst → PLTT[0] (Backdrop 色) |

调用方: MOD-08 脚本 VM opcode 0x4D 处理器 (Op_SysEffect 内部 switch): `IntroBg_Load(data[2])` 与 `IntroBg_Load(0)`。
图像渲染验证: 每张 = 256x160 屏上单个大物件 (25-31 色), 空屏区由 BgMap_FillRow 填充。

## 子系统 C: 菜单 UI 实体 (0x03000058)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x0800BF5C | ✅asm | `MenuUi_Reset` | (附 C 注释) 15 实体布局: 队伍 5 人 x=0x48+i*40, oam 槽 0x71+ |
| 0x0800E170 | ✅C | `MenuUi_SetEntityPos` | 三种布局+状态位 |
| 0x0800E668 | ✅asm | `MenuUi_SetFocus` | (附 C 注释) 焦点切换 (三张位置表按剧情/菜单选择) |
| 0x0800E71C | ✅asm | `MenuUi_SetTargetPos` | (附 C 注释) 目标位置 (idx/mode) |
| 0x0800E7BC | ✅asm | `MenuUi_Step` | 菜单实体移动插值 |
| 0x0800EB98 | ✅asm | `MenuUi_InitCursor` | 光标初始化 |
| 0x0800B2D0 | ✅C | `MenuState_Reset` | gUnk_03000048(菜单窗口状态)/03000186-188 复位 |
| 0x0800B314 | ✅C | `MenuHp_Update` | 战斗后 HP 窗口刷新 (gUnk_03000184/185 门) |
| 0x0800C0D8 | ✅C | `BattleIntro_Setup` | 战斗开场精灵布局 (0x03004380 实体 + CutsceneAnim_Load(0x4C)) |
| 0x080081C0 | ✅C | `BattleIntro_Cursor` | 战斗开场光标 (0x03000008/0C 三角形上下扫) |

## 子系统 D: PlayerStats / 背包 / 金钱

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x0800A79C | ✅C | `Stats_RecalcEquip` | base+equip→最终七维 |
| 0x0800ABBC | ✅C | `Stats_ClearEquipBonus` | 9 个装备加成清零 |
| 0x0800A664 | ✅C | `Stats_RebuildEquipBonuses` | 清零累计装备加成，重算六个装备槽，并应用四槽形态加成 |
| 0x0800A9C0 | ✅asm | `EquipItem` | (附 C 注释) 换装 (旧装备回背包) |
| 0x0800AA60 | ✅C | `AddInventoryItem` (已有名) | 背包+count (上限 99) |
| 0x0800AA84 | ✅C | `RemoveInventoryItem` (已有名) | 背包-count (下限 0) |
| 0x0800AAA4 | ✅C | `Silver_Add` | 金钱+ (上限 999999) |
| 0x0800AAC0 | ✅C | `Silver_Sub` | 金钱- (下限 0) |
| 0x0800A980 | ✅C | `FullHealParty` (源注释) | 全队 HP/MP 满 |
| 0x0800ACA4 | ✅C | `FullHealCharacter` (源注释) | 单人 HP/MP 满 |
| 0x0800A0E4 | ✅C | `Chara_GetFormGfx` | 按装备返回形态图形 id (0x31-0x3C, 0x38/0x39/0x3A 特殊装备变形) |
| 0x0800A924 | ✅C | `Party_InitStats` | 10 人 Stats 初始化 + 背包清空 |
| 0x0800A86C | ✅C | `ExpToLevel` | 经验→等级 (gUnk_08092248 经验表, 上限 9999999) |
| 0x0800A8A0 | ✅C | `LevelToExp` | 等级→累计经验 |
| 0x08009F70 | ✅C | `sub_8009F70` (待改名) | 属性成长查询 (职业/等级/属性序号→属性值); 表 gClassStatCurveTable@0x080921F0 (9×8) + gStatGrowthCurveTables@0x080923D8 (每曲线100B逐级增量); 调用方 sub_800A3C8 / sub_8048818 |
| 0x0800A8D0 | ✅C | `ItemFindSlot` | gUnk_08093418 道具表查找 (type+sub type) |
| 0x0800A958 | ✅C | `ItemGetValue` | 道具表 field_4 |
| 0x0800A970/978 | ✅ | `SaveCopyHpMp` ×2 | 存档结构内 HP/MP 备份 |
| 0x0800AB3C | ✅C | `Party_AnyEquip` | 队伍有人装武器→1 |
| 0x0800AB7C | ✅C | `Chara_ClearTempStatus` | 清临时状态 (field_unk[2/3]) |
| 0x0801026C | ✅C | `ItemGetUsePower` | 道具威力 (0xAF 减半/0x63/0x83/0x84 减 2) |
| 0x0801A0E4? | — | (MOD-04 区) | |
| 0x0800A3C8/AC08/A1B4/A534 | ❌ | 其余函数待匹配 | |

## 子系统 E: 宝箱 / 开关

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x08088400 | — | `gChestSpawnTable` | 256 项 × 8B：mapId/itemId/specialFlag/pad/tileX/tileY；场景切换时筛选当前地图 |
| 0x08088C00 | — | `gDigitFontObjPalettes` | 2 组 × 16 色 OBJ 调色板，供数字字体装载到槽 14/15 |
| 0x08088C40 | — | `gDigitFontObjTiles` | 10 个 4bpp 8x8 数字字形 tile，供数字字体装载到 OBJ 槽 150~159 |
| 0x08088D80 | — | `gMapSceneDescriptors` | 180 项 × 0x14 字节：场景资源/显示参数、NPC 槽组及 BG 数据索引 |
| 0x08008FD0 | ✅C | `ChestObject_BuildSprite` | 宝箱精灵 OAM 链 (开/关两形态, 调色板按 flags bit7) |
| 0x0800908C | ✅C | `ChestObject_Open` | 开宝箱: 音效(8/9)、gChestFlags 位翻转、重建精灵 |
| 0x08009114 | ✅C | `LoadDigitFontObjTiles` (已有名) | 数字字形+调色板装载 |
| 0x08009168 | ✅C | `ChestFlags_ClearAll` | gChestFlags[0x20] 清零 |
| 0x08009184 | ✅C | `ChestFlags_Toggle` | 位翻转 |
| 0x080091A4 | ✅C | `ChestFlags_Test` | 读位 |
| 0x08009F48 | ✅C | `StaticObjs_Reset` | gStaticMapObjects[3] field_0=0 |

## 子系统 F: 存档 (save.c + agb_sram.c)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x080109F8 | ✅C | `Save_Fsm` | 存档/读档状态机 (gUnk_03004D44): 1=ReadSram→2=校验→3=WriteSram→4=校验→F9-FC=四槽初始化→FD=槽失效(4 个 0x0202?000 首字节=0xFF)→FF=收尾(sub_8015F14); 槽 gUnk_03004DD0 (0x800B/槽, 0x0E000000+slot<<11) |
| 0x08010BEC/10CCC/10D80/10E58 | ✅asm | `Save_FillSlot` ×4 | 四个存档槽的序列化填充 (gUnk_030001AA 族) |
| agb_sram.c | ✅ | (官方名) | ReadSram/WriteSram/VerifySram/WriteSramEx + SRAM_V112 (代码拷入 IWRAM 执行) |

## 其余未匹配 (46 个中已判定 12 个)

8005020(MOD-01 帧辅助), 8051D0, 80052F8, 80053B4, 80055E8(MovePlayer), 8005C70,
800661C(MapScene_Load), 80071EC(MapScene_LoadNpcSlotIds), 8007350(动画组装载), 8007ADC, 8007BD0, 8008254(菜单按键),
8008620, 8008F28(NPC 描述符), 80091C4, 8009370, 80094FC, 8009600, 8009F70, 800A048,
800A1B4(Stats 重置), 800A3C8, 800A534, 800A664(角色初始化), 800AC08, 800ACC8(菜单主控),
800B374, 800BFF8, 800C194(对话框帧), 800C2F8(存档触发), 800E244, 800E8F8, 800EAE4,
800EC54, 800F128, 800F3AC, 800F70C, 800FA24, 800FB2C, 800FDEC, 800FF10, 8010170,
8010300, 80104F8, 8010624, 8010770。

## 批量实装记录 (2026-09-01, 21 个 matchings 中实装 18 个)

| 语义名 | 地址 | 大小 | 语义摘要 |
|---|---|---|---|
| `MapBg_LoadFull` (sub_8006520) | 0x08006520 | 252B | 场景整背景装载 (tile 最多5块→CBB, 调色板 0x082893EC 子表, tilemap→SBB) |
| `MapScene_Load` | 0x0800661C | 3024B | 地图场景入口: 清状态、查场景描述符, 按资源集装载 BG/OBJ/菜单资源并设置显示状态 |
| `MapScene_InitSprites` (sub_800729C) | 0x0800729C | 180B | 槽0=主角模型, 槽1=11号; NPC 槽2..9 重载 (gSlotGfxId/PalId) |
| `MapBg_LoadInterior` (sub_8007D5C) | 0x08007D5C | 604B | 选项场景整资源装载 (Huff/LZ77 + BG 寄存器 + 文本块 + 水波) |
| `BgScroll_LoadFromTable` (sub_8008B14) | 0x08008B14 | 72B | 场景 BG 滚动参数表 gUnk_08089BC4[arg]<<6 → 03004650/464C/47C4/47EC |
| `MenuEnt_ParseAll` / `MenuEnt_ParseRange` | 0x08009A7C / 0x08009AC4 | 72/64B | 菜单条目描述表 gUnk_087EA138 全量/区间解析 |
| `PaletteFx_Apply` (sub_8009428) | 0x08009428 | 212B | 调色板特效: 1/7 白闪+WIN0, 3 黑闪, 5 备份, 其他恢复 |
| `StaticObjGfx_LoadPair` (sub_8009BF0) | 0x08009BF0 | 148B | 静态物件 2 条图形/调色板槽记录装载 |
| `StaticObjs_Spawn` (sub_8009C84) | 0x08009C84 | 176B | 从 gUnk_087EA38C 描述块生成 gStaticMapObjects |
| `StaticObjs_StepAll` (sub_8009D34) | 0x08009D34 | 332B | 静态物件逐帧动画 + EnqueueRender 入队 (⚠ s32 返回别名 S32) |
| `StaticObj_BuildChain` (sub_8009E80) | 0x08009E80 | 200B | 帧数据→Sprite_InitChainNode 精灵段链重建 |
| `EquipItem` | 0x0800A9C0 | 160B | 装备槽替换 + 旧装备回背包 + Recalc |
| `SceneBg_Reload` (sub_800B14C) | 0x0800B14C | 388B | 场景 BG 恢复 (窗口 tilemap/4 层 LZ77/BG tilemap 双缓冲) |
| `MenuUi_SwitchGroup`... 见函数 | 0x0800E668 | 180B | 菜单组切换 + 光标实体滑动设定 (三张 Vec2 表) |
| `MenuUi_DrawItemList` (sub_800F4A8) | 0x0800F4A8 | 456B | 物品列表页绘制 (gInventory→gInvViewState 页表 + 选中高亮 0xD/0xB) |
| `WarpTable_Check` | 0x08010434 | 196B | 传送表 gUnk_080987C4 命中当前地图→写出生点+gWarpAnimState |

## ScreenFade 状态机 (2026-09-01 codex)

本节上方的旧条目 `BlendRegs_Update`、`FadeScript_Start` 和 `sub_80051D0` 均由下列命名取代。
它们控制场景淡入淡出，且与普通的 `gBlendControl` / `gBlendCoefficients` 寄存器刷新共用硬件寄存器。

| 地址 | 状态 | 名称 | 职责 |
|---|---|---|---|
| 0x080051D0 | ✅C | `ScreenFade_Apply` | 将进度与 `VCOUNT` 的 1/16 差值钳制后写入 `REG_BLDCNT`、`REG_BLDALPHA`、`REG_BLDY`。 |
| 0x0800526C | ✅C | `ScreenFade_Update` | 每帧推进 signed step；达到终点后清除或置位 flags 的 bit 7。 |
| 0x080088B4 | ✅C | `ScreenFade_Start` | 初始化 flags、step、进度及脚本传入参数。 |

| 地址 | 旧名 | 新名 | 已确认语义 |
|---|---|---|---|
| 0x0300465C | `gScriptLockFlags` | `gScreenFadeFlags` | 活跃状态及 bit 7 的完成锁存。 |
| 0x030047A8 | `gUnk_030047A8` | `gScreenFadeProgress` | 0..0x1B0 的扫描线进度。 |
| 0x030047F0 | `gUnk_030047F0` | `gScreenFadeStep` | 每帧的有符号进度增量。 |
| 0x03004834 | `gUnk_03004834` | `gScreenFadeParam` | Start 的第三参数；读取者尚未确认。 |

规范名称以 `ll.cfg`、`functions.tsv` 和 `iwram.h` 为准：上述早期子系统表中的
`BlendRegs_Update`、`FadeScript_Start`、`gScriptLockFlags` 及其余旧符号仅作为历史别名保留，
当前代码统一使用 `ScreenFade_Update`、`ScreenFade_Start` 和 `gScreenFadeFlags` 等名称。

**code_8005020.c 的 21 个 matchings 全部处理完毕: 19 个实装真 C, 2 个保留 asm(已语义命名 + 注释草稿完整)**:
- `Logo_LoadAssets` (sub_8007FB8, 364B): 选项场景 LOGO 精灵装载, 卡点=GCC2 字面池内联位置(需 permuter)。
- `MenuUi_SpawnAuxSprites` (sub_800EB98, 188B): 菜单辅助精灵生成, 卡点=零常量 sb/r9 与常量 ip 中转的分配选择。

**Logo_LoadAssets (sub_8007FB8, 364B)**: 双图块 (gfx/+0x144) + 双调色板 (gfx-0x180/-0x160) 双精灵 (0x2A00/0x3A10);
未解锁单图块+单精灵 (tileOffsetY=0x20)。代码生成要点: **gfx 指针 ±偏移形态** (r4 缓存) 命中字面池内联布局,
独立符号会多 4 个池使 ROM 布局漂移。

**代码生成要点 (新增沉淀, 详见 EXPERIENCE 经验 89-95)**:
- 静态物件动画的 EnqueueRender 返回值不能以 u8 原型读 (目标直接 cmp) — 用 s32 别名 `Sprite_EnqueueRender_S32` (linker.ld 同址别名)。
- `StaticMapObject.x/y/z` 与 `ChestObject.x/y` 实为 **u16** (目标 ldrh 零扩展), 改 s16 会引入 ldrsh 使 ROM 红掉 8 字节。
- `gUnk_08095028` 若在 C 文件 内定义为空数组会占 .data 8 字节导致 rom 溢出 — 必须 extern 引用。
- 跨 C 文件 原型统一: 本批为 `sub_8009AC4(u8,u8)` / `sub_8009E80(u8,u8*)` / `EquipItem(u8,u8,u8)` 等补全了 code_0.h 原型。

## 选项目的地数据分割 (2026-09-02)

| 地址 | 名称 | 结构与用途 |
|---|---|---|
| 0x08087648 | `gChoiceDestTable` | 5 组变长目的地坐标表；每组为 `[count][count * {x, y}]`，供 `ChoiceMenu_ResolveDest` 解析。 |
| 0x0808823A | `gChoiceGroupPairTable` | 84 项 × 2B 的 `{groupId, packedPair}` 表；`ChoiceMenu_HandleInput` 先特判第 0 项，再从第 1 项按表基址扫描，命中项索引对应 NPC 行为命令流。 |
| 0x080882E2 | `gChoiceMapSpawnRecordStream` | 5 组地图出生记录流；每条 8B 为 `mapId/tileX/tileY/facingDir/moveCmdSetId` 及保留字段，组由 `0xFF` 分隔，末尾为 `0x00`。 |
