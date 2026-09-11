# MOD-01 引擎/系统区 功能分析 (0x080002A0-0x08005020)

> 分析人: plan (2026-08-31)。源文件 `src/code_80002A0.c` (3921 行, 全仓真 C 最多)。
> 114 函数: 110 已匹配 (106 真 C + 4 asm-match), **4 未匹配**。
> 本文给出每个函数的语义摘要与命名。**2026-08-31 起语义名已应用为正式名** (src/iwram.h/ewram.h/linker.ld/ll.cfg/code.s/functions.tsv/asm 全链真改名, 旧 sub_XXXXXX 见 progress.md 对照表; IDA 同步)。

## 模块总职责

游戏引擎层: 中断与显示流水线 (VBlank/HBlank)、系统冷启动与主循环、按键、事件/开关标志位图、
LZ77 增量解压、精灵池/OAM 渲染队列/VRAM+调色板传输队列、CharacterObject 管理与动画、
游戏状态 (gMainGameState) 任务函数、场景切换装载、New Game 初始化、遇敌检查、LOGO 演出。

**核心数据结构**:
- `CharacterObject gUnk_03002E80[24]` (0x28/个): 地图角色 — x/y(s16)、facingDir、field_1(状态位: bit0=脚本动画中,bit2/4/5/6=图形类别)、field_12(状态位: bit0=视口偏移,bit2=移动中,bit6=对话锁,bit7=移动锁)、field_24(移动命令流指针)、field_17(命令流游标)
- `SpriteNode gSpriteNodePool[128]`: OAM 链节点 (attr0/1/2 + next 链)
- `gSpriteRenderQueue[128]`: Y 排序渲染队列
- `gVramTransferQueue[32]` + `gUnk_03003360[32]`(槽计数): VBlank 延迟 VRAM 拷贝队列
- `gUnk_03003380[32]` (调色板传输: field_0=槽号, field_1=模式(1=16色/2=32色), field_4=src)
- `gLzContext`: LZ77 增量解压上下文 (见 sub_8000D5C)
- `gUnk_03001C60`(事件标志位图, 0x40B=512 位) / `gUnk_030018F0`(开关位图, 0x50B=640 位)

## 子系统 A: 中断与显示流水线

| 地址 | 状态 | 建议名 | 语义 |
|---|---|---|---|
| 0x080002A0 | ✅C | `VBlank_UpdateGameScreen` | VBlank 主显示更新(模式1): BG1 按 gUnk_03004698 选 7 种滚动模式(LOGO 滚动/固定/窗口), BG2/3 用 gBG2ScrollX/Y, OAM DMA, LOGO 混合效果, 03004800[13] 触发 0x100 字节调色板补传, blend 设置, HBlank 效果计数器递减 |
| 0x080004F8 | ✅C | `VBlank_UpdateScreenSimple` | VBlank 显示更新(模式3): 全 BG 滚动清零, OAM DMA, VRAM 缓冲 0x02005800→0x0600F800 |
| 0x080005A8 | ✅C | `HBlank_ApplyLineScroll` | HBlank 逐行滚动: 按 scanline 查 gUnk_03001B60/030019C0 波形表写 BG1HOFS/VOFS (4 种效果模式) |
| 0x0800065C | ✅ | `VBlankIntr` (已有名) | VBlank 中断入口: 按 gUnk_0300259C(0-6) 分发 6 条显示流水线; 每条都先 m4aSoundVSync; gGameTimer 自增(饱和 0x0CDFD7F0); gUnk_030025A4/030025A0 帧计数 |
| 0x0800124C | ✅C | `HBlankIntr` | HBlank 中断: 模式1/3 调 HBlank_ApplyLineScroll(VCOUNT), 模式2 调 sub_801887C(窗口?) |
| 0x080012B8 | ✅C | `VBlank_UpdateScreenMode5` | VBlank 显示更新(模式5): 与 0B58 同构但用 CpuFastSet 且先调 sub_8016F30(SIO 泵) |
| 0x080008CC | ✅C | `Display_ShutdownSequence` | 关显示序列: 音频驱动 4 帧 (8053838×4/805369C), 等 VBlank+VCOUNT, 80533F0 泵, 最后 FORCED_BLANK |
| 0x08000ED8 | ✅C | `Display_RestartAfterLoad` | 装载后显示重启: 等 VCOUNT>0xC8, 清 DISPCNT bit7, 重载音频表/精灵, 等 VBlank 结束 |
| 0x08000F54 | ✅C | `System_ResetToLogo` | 全局复位到 LOGO: gUnk_0300259C=0, gMainGameState=0xB, 音频表重载×2 |
| 0x08000F94 | ✅C | `VBlankWait_PumpSound` | VBlankIntrWait + 80533F0(音频泵) |
| 0x08000FA4 | ✅C | `VBlankWaitExit_PumpSound` | VBlankIntrWait + 等 VBlank 结束 + 泵 (帧同步用) |
| 0x08000E1C | ✅C | `Intr_SetMode` | 中断模式切换: 0=常规(IntrMain 拷贝, IE=VBlank/HBLANK/GamePak, DISPSTAT bit4); 1=替换为 sub_8000170 且清 HBlank 中断 |

## 子系统 B: 系统启动/主循环/杂项

| 地址 | 状态 | 建议名 | 语义 |
|---|---|---|---|
| 0x08001128 | ✅C | `System_Init` | 冷启动: RegisterRamReset(3), WAITCNT(预取+WS0), 中断表拷贝, IntrMain→gIntrMainBuf, IE/DISPSTAT/IME, DISPCNT 初值(BG0-2+OBJ+WIN0, FORCED_BLANK), 调 0B58(软复位)/805359C/8021130(场景对象) |
| 0x0800128C | ✅C | `AgbMain` | **主循环** (源码注释 AgbMain): `while(1){ gMainTasks[gUnk_03001AC0](); VBlankIntrWait(); 80533F0(); }` |
| 0x08003088 | ✅C | `Task_DispatchGameState` | 每帧任务头: ReadKeys + sub_8004BE0(清渲染队列) + `gUnk_087E83F8[gMainGameState]()` 状态函数表派发 |
| 0x08000B58 | ✅C | `System_SoftReset` | 软复位: VRAM/OAM/PLTT/0x02004000(0x2000) DMA 清零, 精灵池复位(4B8C/4BBC), gGameTimer=0, 16 个全局清零, OAM 缓冲清零, 事件/开关位图清零, 8 个状态全局清零 |
| 0x08000FF8 | ✅C | `Rand_TableNext` | 随机数: `gUnk_08057750[gUnk_030025A4++]` (256 项伪随机表循环) |
| 0x08000C98 | ✅C | `HBlankSinTable_Init` | 初始化 HBlank 波形表 (gUnk_03001B60/030019C0 各 255 项): 按模式 1-4 从 gUnk_080576D0 正弦表取样/除法/掩码 |
| 0x080011F0 | ✅C | `ReadKeysRaw` | `gNewKeysRaw = ~KEYINPUT & ~gHeldKeysRaw; gHeldKeysRaw = ~KEYINPUT` |
| 0x0800121C | ✅C | `ReadKeys` (已有名) | 同上 (ll.cfg 名 ReadKeys) |
| 0x08001248 | ✅ | `nullsub_5` | 空 |
| 0x08001284/88 | ✅ | `DummyIntr3/4/5` | 空中断占位 |

## 子系统 C: LZ77 增量解压 (SRAM 安全)

| 地址 | 状态 | 建议名 | 语义 |
|---|---|---|---|
| 0x08000FD0 | ✅C | `LZ_InitContext` | 初始化 gLzContext: dest/src/flags 指针(数据后跟标志字节), 剩余量=uncompressedSize, 本帧处理量=arg2 |
| 0x08000D5C | ✅C | `LZ_UncompressChunk` | 增量 LZ77 解压一轮: 逐 bit 读标志, 0=字面复制 1=2 字节 token(12 位偏移+4 位长度-3); 返回剩余量(0=完成)。分帧执行避免占用过长 — 用于向 SRAM/WRAM 流式解压 |

## 子系统 D: 事件/开关标志位图

| 地址 | 状态 | 建议名 | 语义 |
|---|---|---|---|
| 0x08001014 | ✅C | `EventFlags_ClearAll` | 清事件位图 gUnk_03001C60 (0x40 字节 = 512 事件) |
| 0x08001030 | ✅C | `EventFlags_Test` | 读事件位 (`arg0>>3] & 1<<(arg0&7)`) — 事件号 ≤0x1FF; 0x200+ 走 SwitchFlags |
| 0x08001050 | ✅C | `EventFlags_Set` | 置位 |
| 0x08001070 | ✅C | `EventFlags_Reset` | 清位 |
| 0x08001090 | ✅C | `SwitchFlags_ClearAll` | 清开关位图 gUnk_030018F0 (0x50 字节 = 640 开关) |
| 0x080010AC | ✅C | `SwitchFlags_Test` | 读位 (脚本 0x200+ 号走这里, 见 sub_80532DC) |
| 0x080010CC | ✅C | `SwitchFlags_Set` | 置位 |
| 0x080010EC | ✅C | `SwitchFlags_Reset` | 清位 |
| 0x0800110C | ✅C | `SwitchFlags_ClearRange` | 清位图 0x3D-0x4F 段 (临时开关区?) |

## 子系统 E: 游戏状态任务 (gMainGameState 派发目标)

gMainGameState 值: 0xB=LOGO/启动, 1=地图探索, 2=场景切换装载, 3=传送中, 5=战斗, 9=门内/室内?

| 地址 | 状态 | 建议名 | 语义 |
|---|---|---|---|
| 0x08001D08 | ✅C | `Task_MapExplore` | **地图探索主帧** (最大函数): 对话标志(gUnk_025D8)/菜单标志(gUnk_03004D4C=0xD)锁移动; ABXY 同按=软复位; A 键=CheckFacingEvent→脚本, B 键=sub_800ACC8(菜单); 十字键→moveFlags 查 gUnk_0805881C 方向速度表→sub_80055E8(MovePlayer, MOD-02); sub_8002D54 遇敌→state=5; gUnk_0300260C 传送动画状态机(1-10: 音效 0x78/0x79, 白闪 0x12 特效, state=3 传送中); 末尾 8050014(脚本泵)/8002DDC(LOGO 混合)/8002154(精灵帧) |
| 0x08002D54 | ✅C | `CheckEncounter` (源注释 UpdateEncounter) | 遇敌判定: gUnk_03004820=遇敌开启; 移动中且事件位 0xBB 未置时 gUnk_030025D4 计数递减, 归零后查随机表→触发(返回1); 地图 0x7B 计数减半; 事件 0xBB 置位时改为保底递增 |
| 0x08002DDC | ✅C | `LogoBlendEffect_Update` | LOGO 淡入/淡出状态机 (gUnk_03002604=1-10): 关 OBJ→BLDY/BLDALPHA 递变→等待→复位; case5 置 DISPCNT bit8 |
| 0x08002F6C | ✅C | `LogoAssets_Load` | LOGO 资源装载状态机: case1 调色板+BG 地图 LZ77→case2/3/4 三块 tileset LZ77→case10 复位混合参数 |
| 0x08001354 | ✅C | `SceneTransition_Load` (源注释: 切换场景加载) | 场景切换: 停显示→暂存 gUnk_0300259C→清渲染→相机(gUnk_025F8/02C3C=gUnk_0468C/04638×8)→精灵/调色板复位→地图精灵装载(33E8)→NPC 描述符(661C/729C/33E8)→移动命令(8B14)→9BF0/9C84→win0 设置→转场播放(8053628/80536C0)→state=1 |
| 0x08001538 | ✅C | `NewGame_Init` (源注释: New Game) | 新游戏初始化: 队伍 gPartyMemberIds[0]=主角其余 0xFF, gBattleFormationIds 同构, 11 个角色槽初始化(A924/A664/A79C), 背包 gInventory[0xDD]=2, 金钱 300, 计时清零, 初始位置 0x60/0x50, 脚本泵复位(8052580), win0, 8B 式开场(80525E8) |
| 0x08001708 | ✅C | `Scene_EnterMap` | 进入地图帧: 等待装载完成→玩家落位(相机/朝向), 同伴 gUnk_03002E80[1] 对齐主角, 80525B0, state=1 |
| 0x08001828 | ✅C | `Scene_ExitToMenu` | 离开地图: 停显示→state 切换→sub_8017FA4(存档写入?)→gUnk_03001AC0=1(主任务槽切到菜单)→mode=2 |
| 0x080018D4 | ✅C | `Scene_Reload` | 场景重载 (战后/菜单返回): OAM 清零→判断 gUnk_03002C48(战斗标记)与 gUnk_03002C34→分支恢复或全量重载(661C/ReloadAllSpriteSheets/8B14/9BF0)→转场 |
| 0x08001A7C | ✅C | `Scene_EnterDoor` | 进入门内场景: 全量重置+sub_8007D5C(gUnk_030047BC=门配置)+state=9, 主角定位(gUnk_03004824/047B8), 脚本 0x39 位决定转场类型 5/6 |
| 0x08001BD0 | ✅C | `Scene_RestoreAfterBattle` | 战斗后恢复: 从 0x0203F000/0x0203FE00 DMA 恢复精灵池+OAM 备份, 全量重载, 转场 |
| 0x080030B0 | ✅C | `SceneTransition_RequestMap` (源注释: 地图场景切换) | 置 state=2 + mode=1 请求装载 (下一帧由 1354 执行) |
| 0x08003168 | ✅C | `Scene_ReloadViaMenu` | 菜单发起的场景重载: sub_80043D4(跟随者同步)→state=2 |
| 0x08003114 | ✅C | `Task_DialogueFrame` | 对话帧任务: sub_800ACC8(对话框)+sub_800C194+OAM 收尾 |
| 0x080031E4 | ✅C | `Task_SaveMenuFrame` | 存档菜单帧任务: sub_8011454(存档菜单主控)→显示收尾 |
| 0x080031F8 | ✅C | `Task_TextFrame` | 文本引擎帧: sub_801417C→收尾 |
| 0x08003208 | ✅C | `Scene_ResetResources` | 资源复位: 3348(释放角色精灵)+4BE0/4B8C/4BBC+OAM 清零+4AC0/4B60+8A3C/9F48 |

## 子系统 F: 精灵/OAM/渲染队列

| 地址 | 状态 | 建议名 | 语义 |
|---|---|---|---|
| 0x08002154 | ✅C | `Sprites_UpdateFrame` | 精灵主帧: 图形集 gObjGraphicsSetId 有效时遍历 24 个 Actor — 0x80 集模式走 field_1&1 分支(脚本动画), 普通模式走移动/对话锁判断; 每角色: 3C54(步进)→3B08(命令)→271C(动画 attr)→243C(入渲染队列)→2380(箭头); 宝箱 gChestObjects[16] 入队; 9D34/32BC/91C4 收尾 |
| 0x0800243C | ✅asm | `Sprite_EnqueueRender` | (源码内附完整 C 注释) 精灵→渲染队列: 屏幕坐标换算(BG3 视口), 视口裁剪(flags bit7), Y 排序插入 gSpriteRenderQueue; 返回 1=不可见 |
| 0x0800271C | ✅asm | `Sprite_UpdateCharaAnim` | (附 C 注释) 角色精灵 attr 更新: 走路动画帧(gWalkAnimFrameMapping/gWalkDirectionMapping 查表), 左右翻转(attr1 bit12), VRAM 槽 72tile 步进, palette bits; field_1 位分支(0x20/0x10/4/8=特殊图形模式) |
| 0x080029D8 | ✅asm | `Anim_PlayCustom` | (附 C 注释) 自定义动画播放器: gUnk_030044C0[currAnimIdx] 帧数据表, animFrameTimer 计时/循环(gUnk_03002C60 bit7), 帧回调 2B54 构建 OAM |
| 0x08002B54 | ✅asm | `Anim_BuildOamChain` | (附 C 注释) 从帧数据建 OAM 链: oamDataPtr 6B/节点, VRAM 拷贝请求(4B2C), attr0/1/2 组装, 翻转(gUnk_03002C60 bit6), gSpriteWidth/Height |
| 0x080032BC | ✅C | `OAM_FlushFromQueue` | 渲染队列→OAM 缓冲: 遍历 gSpriteRenderQueue, 每链逐节点 4F64 写入(游标>0x7F 停), 剩余 OAM 清隐(0xA0) |
| 0x08003348 | ✅C | `Sprites_ReleaseAll` | 释放全部 24 角色的精灵链 (flags=0, next=0) |
| 0x080033E8 | ✅asm | `Sprites_LoadMapNPCs` | (附 C 注释) 装载地图 NPC 组: `gMapSceneDescriptors[arg0].npcSlotGroupId` 组号→gUnk_08091948 数量→gUnk_087EA394 描述符表逐个 345C |
| 0x0800345C | ✅asm | `Chara_InitFromDesc` | (附 C 注释) 从 16B 描述符初始化角色: tile 坐标<<3, palette/图形, field_1 状态位→attr 链形状(bit2/4/5/6 四种 OAM 形态), 初始化全字段 |
| 0x0800375C | ✅C | `Chara_InitDialogArrow` | 初始化对话框箭头角色 (gUnk_03003178[idx], palette 9, x=相机+idx*40+40) |
| 0x080037DC | ✅C | `Chara_InitEffect` | 初始化特效角色 (gUnk_03002E80[arg0], palette 5) |
| 0x0800384C | ✅C | `Chara_InitEffectAtPlayer` | 初始化玩家旁特效角色 (gUnk_03003150, palette 10, 玩家坐标+8/+12) |
| 0x08002380 | ✅C | `Sprite_SetupDialogArrow` | 对话箭头精灵节点设置 (attr 0x892 特殊字形, 偶数帧交替 tileOffset 0xFC/0xFA) |
| 0x080038CC | ✅asm | `PendingSpriteLoad_Flush` | (附 C 注释) 消费延迟装载请求: bit0→LZ77 图形(0x06011400+slot*0x900), bit1→DMA 调色板 |
| 0x08004BFC | ✅C | `Sprite_AllocNode` | 分配精灵节点槽 (2..0x6F, flags==0) |
| 0x08004C28 | ✅C | `Sprite_InitChainNode` | 初始化链节点 (flags/attr0/1/2, 需要时链一个新节点并返回) |
| 0x08004D38 | ✅C | `Chara_FreeSprite` | 释放角色精灵链 (sprNodeIdx=0) |
| 0x08004F3C | ✅C | `Sprite_FreeChain` | 通用精灵链释放 |
| 0x08004F64 | ✅asm | `Sprite_WriteOam` | 节点→OAM 缓冲 (详细注释见源码 3856-3888 行: 负 flags=跳过, 游标>0x7F 满) |
| 0x08004CE8 | ✅C | `Ui_LoadArrowTiles` | 装载 ◀▶/滚动条字形到 OBJ VRAM 0x06011240 (按参数符号选 2/4 块) |

## 子系统 G: VRAM/调色板传输队列 (VBlank 延迟拷贝)

| 地址 | 状态 | 建议名 | 语义 |
|---|---|---|---|
| 0x08004A44 | ✅C | `VramTransfer_AllocSlot` | 分配 VRAM 传输槽 (32, gUnk_03003360==0) |
| 0x08004B2C | ✅C | `VramTransfer_Enqueue` | 登记 (src/dest/槽计数, <32 校验) |
| 0x08004ADC | ✅C | `VramTransfer_Flush` | VBlank 执行: DmaCopy16(src→dest, 计数<<5) 后清槽 |
| 0x08004AC0 | ✅C | `VramTransfer_Clear` | 清队列 |
| 0x08004A6C | ✅C | `PalTransfer_AllocSlot` | 分配调色板传输槽 (32, field_1==0) |
| 0x08004A94 | ✅C | `PalTransfer_Enqueue` | 登记 (槽号/源/模式: arg3=2→32 色 0x40B, 否则 16 色 0x20B) |
| 0x08003264 | ✅C | `PalTransfer_Flush` | VBlank 执行: DmaCopy16 到 0x05000000+槽号<<5 |
| 0x08004B60 | ✅C | `PalTransfer_Clear` | 清队列 |
| 0x08004B8C | ✅C | `SpritePool_Clear` | 清 128 节点 flags/next |
| 0x08004BBC | ✅C | `Queue2_Clear` | 清 gUnk_030034C0[32] (8B/项, 用途见 MOD-02?) |
| 0x08004BE0 | ✅C | `RenderQueue_Clear` | 清 gSpriteRenderQueue |
| 0x08004C8C | ✅C | `SpriteSheet_LoadGfx` | LZ77 解压 gUnk_087E8430[gfxId] → 0x06011400+slot*0x900 (注释详述: dst 必须先算入变量) |
| 0x08004CB8 | ✅C | `SpriteSheet_LoadPal` | DMA gUnk_080B9DFC[palId] → 0x05000200+slot*32 |

## 子系统 H: 角色对象 (CharacterObject) 辅助

| 地址 | 状态 | 建议名 | 语义 |
|---|---|---|---|
| 0x08003B08 | ✅C | `Chara_ProcessCmdStream` | 处理移动命令流 (field_24+field_17): 0xFE=循环回卷, 0xFD=结束+置 0x20+清指针, 0xFF=结束, 0x03=仅朝向, 0x01=走一步(3B), 0x02=转身(4B, 置 0x10), default=移动(4B); 无命令流时随机游荡(rand 1/4/5/6=field_11=0x10, 2/3=转向) |
| 0x08003C54 | ❌399行 | `Chara_StepMove` | 移动步进+碰撞: 引用 25D8(对话锁)/25F8/2C3C(玩家坐标)/2E80/04890(宝箱!)/0xFFF00000/0xFFF80000(符号扩展掩码) — 判定目标 tile 可走性(含宝箱阻挡), 推进像素步 |
| 0x08003F40 | ✅asm | `CheckFacingEvent` | A 键交互: 检查面前 tile 的事件, 返回事件号 (0=无) |
| 0x080040E4 | ❌322行 | `Party_FollowAnim` | 跟随者更新: 引用 25B0(朝向)/25C0/25E0/2C58(8 帧历史)/25F8/2C3C/2E80/3AC0(精灵池)/0x3FF/0x8E8 — 跟随者沿历史轨迹移动+精灵对齐 |
| 0x08004358 | ✅C | `Followers_ResetHistory` | 8 帧历史数组全部填主角当前位 (gUnk_03002C44 bit0=0 时) |
| 0x080043D4 | ✅C | `Followers_SyncToTail` | 跟随者对齐历史尾帧 + gUnk_03002C44 &= 0x80 |
| 0x0800445C | ✅asm | `Party_FollowStep` | (附 C 注释) 玩家+跟随者逐帧推进: field_12 bit7(移动中)时历史入栈, 25B0/25F8/2C3C 从角色回写; 跟随者取历史尾 |
| 0x080046DC | ✅C | `CutsceneAnim_Load` | 装载过场动画配置: gCutsceneAnimConfigTable[arg0]→gUnk_030044C0(脚本表)/gUnk_03003250(VRAM 0x02020000+arg1*0x1000)/gUnk_03002C60(flags)/gUnk_03003490(槽)/gUnk_030032E0(调色板), LZ77 解压图形; arg2>99 表示 +0x40 标志 |
| 0x0800478C | ❌236行 | `CutsceneAnim_PlayFrame` | 过场动画帧步进: 引用 0x02020000/2C60/3250/32E0/3490/44C0(与 46DC 同一套表)+08393B28/083989FC/0839C80C/087EBE00 — sub_80029D8(角色动画播放器)的过场变体 |
| 0x08004980 | ✅C | `MapGroup_Lookup` | 查 gUnk_087E94FC[22] 表 (field_0==gUnk_030047B0 当前地图) → gUnk_03004618=组号+1 (0=无) |
| 0x080049C8 | ✅C | `Chara_SetTilePos` | 设置角色 tile 坐标: arg1=0→x=arg2<<3+arg3, 否则 y=(arg2+1)<<3+arg3 |
| 0x08004A00 | ✅asm | `Chara_MoveBy` | 相对移动 x/y ±arg3 |
| 0x08004D20 | ✅C | `Chara_SetGfxPal` | 设 field_2(图形)/paletteId |
| 0x08004D8C | ✅C | `Chara_SetCmdPtr` | 设 field_24 = arg1 (移动命令流指针, sub_8003958 的消费端) |
| 0x08004DA4 | ✅C | `Chara_StartMoving` | field_12\|=0x80(移动锁), &=0xDF, field_10=1, 游标清零 |
| 0x08004DD0 | ✅C | `Chara_AnyMoving` | 24 角色中任一 field_12&0x80 → 1 |
| 0x08004E04 | ✅C | `Party_SetFollowMode` | gUnk_03002C44 \|= 1 (跟随者沿历史模式) |
| 0x08004E88 | ✅C | `Chara_SetPosDir` | 设 tile 坐标(x=arg1*8, y=arg2*8)+朝向+field_E, field_11=0 |
| 0x08004EB8 | ✅C | `Chara_GetDrawX` | field_12 bit0 时 x 加 sub_8008D78() 偏移 (镜头补偿) |
| 0x08004EDC | ✅C | `Chara_GetDrawYShift` | field_12 bit0 时按 gUnk_0300460C(2/5) 两模式减镜头差 |
| 0x08004FA8 | ✅C | `Chara_StartScriptAnim` | field_1\|=1(脚本动画模式), field_14=0, animIdx=arg1 |
| 0x08004FD0 | ✅C | `Chara_AnimWaitDone` | arg0<0x64: 清动画循环位(gUnk_03002C60 bit7), field_14>0xFE→完成; ≥0x64: 强制 gUnk_03001EE0[arg0] 完成 |
| 0x08004E14 | ✅C | `SpriteRequest_Gfx` | 请求装载图形: gSlotGfxId[slot]=gfxId + 置 PENDING_SPRITE_GFX |
| 0x08004E48 | ✅C | `SpriteRequest_Pal` | 请求装载调色板 + PENDING_SPRITE_PAL |
| 0x08004E7C | ✅C | `SpriteRequest_Pending` | 返回 gPendingSpriteLoad 位图 |

## 未匹配函数 (4 个)

| 地址 | 行数 | 判定 | 备注 |
|---|---|---|---|
| 0x08003958 | 232 | `Chara_SetWalkPath` | 寻路指令写入器 — **分析中** (progress.md 挂起, 候选 10270 分, 卡 global-alloc 排列墙; 完整语义见 progress.md 该条) |
| 0x08003C54 | 399 | `Chara_StepMove` | 移动步进+碰撞 (引用宝箱表 04890/对话锁 25D8/符号扩展掩码) — 建议下一个攻 |
| 0x080040E4 | 322 | `Party_FollowAnim` | 跟随者动画+位置 (引用 8 帧历史 25C0/25E0/2C58+精灵池) |
| 0x0800478C | 236 | `CutsceneAnim_PlayFrame` | 过场动画帧步进 (与 46DC/29D8 同一套表 44C0/2C60/3490/32E0, VRAM 0x02020000) |

## 关键全局变量语义 (本模块新确认)

| 符号 | 语义 |
|---|---|
| gUnk_0300259C | VBlank 显示流水线模式 (0-6; VBlankIntr/HBlankIntr/1354/1828/18D4 读写) |
| gUnk_03002604 | LOGO 演出状态机 (0-10; 2DDC/2F6C/802A0 驱动) |
| gUnk_03001AC0 | 主任务槽 (AgbMain 的 gMainTasks 下标; 1828 切 1) |
| gMainGameState | 游戏状态: 0xB=LOGO, 1=地图, 2=装载, 3=传送, 5=战斗, 9=门内 (087E83F8 派发表下标) |
| gUnk_03002600 | 场景内阶段: 1=装载中(锁输入), 5=传送菜单? |
| gUnk_030025D8 | 对话进行中标志 (0/1, 锁移动) |
| gUnk_03004D4C | 菜单打开标志 (0xD=菜单, 锁移动) |
| gUnk_0300260C | 传送动画状态机 (1-10) |
| gUnk_030025B4/030025FC | 相机像素坐标 (X/Y); gUnk_030025F8/02C3C = 相机目标 tile×8 |
| gUnk_030025B0 | 玩家朝向 (移动方向表下标+1 传给 MovePlayer) |
| gUnk_030025C0/25E0/2C58[8] | 跟随者 8 帧位置/朝向历史 |
| gUnk_03002C44 | bit0=跟随历史模式 (4E04 置) |
| gUnk_03002C48 | 战斗标记 (战斗链计数, 18D4/1828 判断战后恢复) |
| gUnk_03002C34 | 战斗结果类型 (1/2, 18D4 分支) |
| gUnk_0300465C | bit7=剧情锁 (锁移动/传送) |
| gUnk_03004820 | 遇敌开关 |
| gUnk_030025D4 | 遇敌计数器 (0xE8+rand&7<<5, 地图 0x7B 减半) |
| gUnk_030047B0 | 当前地图 id (MapGroup_Lookup / 遇敌减半) — 即 gCurrentMapId |
| gSceneBlendMode | 0x03004628 场景混合特效模式 (MapScene_Load 从 gMapSceneDescriptors[].bgLoadMode 装入; PaletteEffects_Update 驱动) |
| gObjGraphicsSetId | 精灵图形集 (0xFF=无, bit7=过场模式, sub_8002154 分支) |
| gPendingSpriteLoad | 延迟装载位图 (bit0=图形, bit1=调色板) |
| gUnk_03003480 | 待切角色 id (0xFF=无; 1D08 里触发 80526A0) |

## 与其他模块的接口 (调出)

- MOD-02 (code_8005020.c): `sub_80055E8`(MovePlayer 注释)/`sub_8005020`(精灵动画头)/`sub_8008D18`/`sub_8008B14`(移动命令装载)/`sub_8008A3C`/`ReloadAllSpriteSheets`/`sub_8009BF0/9C84/9D34/91C4/9A5C/9B44/9F48/9168`(音频+精灵辅助)
- MOD-03 (code_8010F10.c): `sub_8011454`(存档菜单)/`sub_801417C`(文本)/`sub_8016F30`(SIO 泵)/`sub_80171E4/8017588/801761C`(SIO 状态机, 从 sub_800096C)
- MOD-04/05: `sub_8021130`(场景对象复位, System_Init 调)
- MOD-08: `sub_8050014`(主循环/脚本泵)/`sub_805008C`/`sub_8052580`(脚本复位)/`sub_80525E8/80526A0/8052574`(开场/转场)/`sub_80533F0/80535F4/8053628/8053688/805369C/80536C0/8053720/805374C/8053838`(音频泵/转场效果族)
- MOD-07: `sub_800ACC8`(菜单)/`sub_800C194`(对话框)
- 事件位图消费者: MOD-06 脚本处理器 (sub_8001030/10AC/10EC 被 MOD-07 的 804F0B8 区和 0532DC 调用)

## 分析结论

1. 本模块是**引擎地基**: 显示流水线全部集中于此, gMainGameState 状态机的任务函数也大半在此。
2. 4 个未匹配函数全部有明确语义判定, 建议匹配顺序: **sub_8003C54(碰撞, 与 sub_8003958 同消费 25F8/2C3C) → sub_80040E4 → sub_800478C → sub_8003958(继续)**。
3. 建议名已按子系统前缀归类 (VBlank_/HBlank_/System_/LZ_/EventFlags_/SwitchFlags_/Task_/Scene_/Sprite_/Chara_/Party_/VramTransfer_/PalTransfer_/CutsceneAnim_), 可直接用于 IDA 批量标注脚本与 progress.md 命名同步。

## 2026-09-11 claude-8018A58: 战斗背景加载器 (sub_8018A58, sio_link.c M10 聚类)

✅ 已匹配 (fncheck 416B)。调用图: `BattleTask_Run → sub_8018A58 → sub_8018E34 / LZ77UnCompVram / sub_804C548(3) / sub_8018BF8 / BgLoad_Finish`。

- **入参**: `u8 unused` (code_0.h 原型带 u8, 函数体不读 r0; caller `movs r0,#0` 传 0)。
- **gUnk_087ED394** = 战斗背景三元组表, 每项 12B (3 个 ROM 指针): `field_0`=tilemap/tile 图 (LZ77→0x06000000), `field_4`=副资源 (sub_804C548 参数 0,3), `field_8`=块图 (LZ77→0x06006000)。索引 = `sub_8018E34()` 返回的场景/对话图标 ID (u32 接收, 无 u8 扩展 —— 原型已改 u32, 见 progress.md 2026-09-11)。
- **gUnk_0861A4A4** (0x140B) DmaCopy32(3)→0x06005000 (BG 拼块调色/字符数据)。
- **bldcnt 位组合写入 0x0400000C (GREENSWAP 地址)**: 原始代码死写, 用 REG_BG2CNT 宏达成同址形状; 疑似原 SDK 头文件 BLDCNT 定位差异, 勿"纠正"为 REG_BLDCNT (0x04000050) —— 字节为准。
- **gUnk_02036EC0** (EWRAM, 0x168B) DmaFill16(3) 清零 = 战斗 BG 滚动工作区; 首址存 `gUnk_030004D0`; `gUnk_030004D8[4]` = BG0..3 HOFS 寄存器地址 (0x04000010/14/18/1C), `gUnk_030004E8[4]` = BG0..3 VOFS (0x04000012/16/1A/1E), 消费方 = `BgScrolls_WriteAll` (0x0801A36C, 经 gUnk_03000500 8 字段)。
- `gUnk_030004D7` = 0 (滚动禁用标志?)。
