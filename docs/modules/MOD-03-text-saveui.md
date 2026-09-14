# MOD-03 文本引擎/存档 UI/SIO/窗口系统 (0x08010F10-0x0801A3C4)

> 分析人: plan (2026-08-31)。源文件 `src/code_8010F10.c` (1465 行, 99 个真 C 函数)。
> 本模块确认 `gWindowBgBuf`(0x02005800) 是**窗口/文本 tilemap 缓冲** (32×32 tile, 写入后由 MOD-01 DMA 到 0x0600F800)。

## 子系统 A: 文本引擎 (核心 = sub_8016368 字符写入)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x080163CC | ✅C | `TextBlocks_Render` | 描述符流渲染: (x,y,pal) 头 + 0xFF 结尾字符串, 0xFE=扩展码(下一字节<<8) |
| 0x08016BE0 | ✅C | `Text_WriteChars` | 字符串→tile 项 (pal<<12 + 0x200 + char) |
| 0x08016C10 | ✅C | `Text_FillHidden` | 写 0xF200 (隐藏字符) 到 0xFF 结尾 |
| 0x08016C2C | ✅C | `Text_TileAt` | 窗口缓冲坐标→u16* (0x02005800 + y*32+x) — 被存档菜单 F3b 调用 |
| 0x080166A4 | ✅C | `Text_DrawChar` | 从 gUnk_08095028 字库表绘制单字符 (8 字节/字符码) |
| 0x0801654C | ✅C | `Text_WriteOrClear` | arg2=0 清除区域否则写字符串 |
| 0x080164F8/8016508 | ✅C | `Msg_Show` / `Msg_ShowById` | 0x030001D0 消息指针表 + 0xFF 分隔, 调 BEE4 写入 |
| 0x0801667C | ✅C | `MenuUi_HideAll` | 实体 5-14 置 0x40 (隐藏) |
| 0x08016628 | ✅C | `MenuUi_SetExclusive` | 除 arg0 外全隐藏/恢复 (gBgmRequestId!=0 时 arg0=5) |
| 0x080165C8 | ✅C | `Text_ClearRect` | 窗口缓冲矩形清 0xB001 |
| 0x08016444/8016460 | ✅C/asm | `Msg_ShowEndMark` | 行尾写 0xC9 结束符 |
| 0x0801A324/330/33C | ✅C | `BgLoad_Reset/Finish/GetPos` | gUnk_030004F8 背景 LZ77 流式装载游标 (配合 801A2EC) |
| 0x08019E38/E4C/ECC | ✅C | `Disp_ObjOff/ObjOn/Bg1Off` | DISPCNT 位操作 |
| 0x0801A36C | ✅C | `BgScrolls_WriteAll` | gUnk_03000500 (8 个 BG 滚动值) → 寄存器 |
| 0x08019148 | ❌ | `Bg0_InitClear` | 清 0x02035AC0/0x06007000 0x400 项 + BG0CNT 配置 (挂起, 见 progress.md) |
| 0x08019E60 | ✅C | `BlankTilemap` (已有名) | 0x020352C0 全部指向空白 tile 0x2C0 |
| 0x0801A270 | ✅C | `WaveBuf_Fill100` | 0x020362C0 填 100 (HBlank 波形复位) |
| 0x08019ECC | ✅C | `Disp_Bg1Off` | |

## 子系统 B: 存档菜单 / 存档状态 (F3a 底层的消费者)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x08013F3C | ✅asm | `Save_LoadSlot0` | (附完整 C 注释) 重试循环 Save_Fsm + 0x02027000 签名校验 (gUnk_08098199 12B) + 清计时器 |
| 0x08013FE8 | ❌ | `Save_LoadContinue` | (被 5EA0/6BB0 调) |
| 0x08014084 | ✅C | `SaveTimer_CountUsed` | 统计 gUnk_03004D60 半字节非零数 → gUnk_03004DE4 |
| 0x080140D0 | ✅C | `SaveTimer_Inc` | 半字节递增 (上限 5) |
| 0x08014124 | ✅C | `SaveTimer_Dec` | 半字节递减 (0 保持 0) |
| 0x08015F50 | ✅C | `SaveTimer_Get` | 读半字节 |
| 0x08015E88 | ✅C | `Save_ResetReadState` | gUnk_03004D44=1, 槽=0 (引擎侧存档触发, 被 800C2F8 调) |
| 0x08015EA0 | ✅C | `Save_StartWrite` | 预处理+状态=3 (写档), 调 Save_LoadContinue + 音效 |
| 0x08016BB0 | ✅C | `SaveUi_OpenLoad` | 读档菜单入口 (槽 0xC, 03000221=0) |
| 0x08015FB4 | ✅C | `SaveUi_Open` | **存档 UI 入口**: gUnk_03004DD4=arg0, gUnk_03004D40=0x28, state=0xD→gMainGameState=0xD, 清窗口缓冲 (脚本 Op_TriggerSave 调用) |
| 0x08015F14 | ✅C | `SaveUi_DrawSlots` | 3 槽存档信息绘制 (Save_FillSlot3 + 0x08010F10) |
| 0x08015F74/8015F94 | ✅C | `SaveFlag_Set/Get` | gUnk_03004DC8 位图 |
| 0x08014554* | ❌ | `SaveUi_Main` | 0x08011454 = 存档菜单主控 (1855 行, F3b 主体) |
| 0x08012790 | ❌ asm | `SaveUi_LoadScreen` | 存档/场景选择 UI 主状态机；扫描 `gSaveMapUnlockFlags` 并用 `SaveFlag_Get` 判断场景解锁，确认后调用 `MapScene_Load` |
| 0x08013934/13B0C/13C00/13870/12530/1114C/11268/113CC | ❌ | 存档 UI 辅助 + 文本界面 (1114C=技能菜单绘制, 附 C 注释) |
| 0x08016A14 | ❌ | `Save_SyncShadow` | 0x02027000 影子同步 (Save_LoadSlot0 调用) |

## 子系统 C: 背包 UI / 物品游标

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x08016978 | ✅C | `Inv_FindFirstHeld` | 扫 gUnk_0839CFAA[16] 道具类别表, 返回首个持有项位置 |
| 0x080169AC | ✅C | `Inv_FindPrevHeld` | 从 gUnk_03000227 向上找 |
| 0x08016A6C | ✅C | `Inv_SeekFirst` | gUnk_03000199 = 首个非零背包项 |
| 0x08016AA0 | ✅C | `Inv_PrevNonZero` | 游标上移 |
| 0x08016AD4 | ✅C | `Inv_NextNonZero` | 跳过 arg0 个非零项 |
| 0x08015B90 | ✅C | `InvUi_DrawCursors` | 菜单游标 tile (0x826/0x26 族) |
| 0x08015C18 | ❌ | `InvUi_Main` | 背包界面 |
| 0x08015E1C | ❌ | (待匹配) | |

## 子系统 D: 队伍菜单数据 (gUnk_03000186-1B4 游标组)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x080167F8 | ✅C | `Party_SlotOfMember` | gBattleFormationIds→gPartyMemberIds 下标 |
| 0x0801682C | ✅C | `SkillMenu_SaveCursor` | 游标存入角色 field_unk[2/3] |
| 0x080168EC | ✅C | `SkillMenu_RestoreCursor` | 恢复 |
| 0x08016930 | ✅C | `SkillMenu_GetSkill` | 角色技能 (0x26=空, 越界 0xFF) |
| 0x080167D4 | ✅C | `MenuUi_MoveCursor` | 依 65B8 游标移动实体 |
| 0x080165B8 | ✅C | `Menu_GetFocus` | gUnk_03000188[0]-1 |
| 0x080168A8/6878 | ✅C | `ItemUse_SetCtx` / `ItemUse_Execute` | gUnk_030001B4 表 + sub_800FF10/sub_8010170 |

## 子系统 E: HP/MP/Lv 数字绘制 (战斗/菜单 HUD)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x0801624C | ✅C | `Num_Draw16` | sub_800BFF8 包装 (0xB000 色) |
| 0x08016260 | ✅C | `Hud_DrawLv` | "Lv" 图标 0xB257 + 等级数字 |
| 0x080162A8 | ✅C | `Hud_DrawHp` | 0xB258 + HP (满血 0xF000 高亮色) |
| 0x08016308 | ✅C | `Hud_DrawMp` | 0xB259 + MP |
| 0x08016424 | ✅C | `Math_DivLoop` | 减法循环除法 (商≤255) |

## 子系统 F: SIO 多机通信 (0x08016C88-0x080170D0)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x08016E30 | ✅C | `Sio_BuildPacket` | 打包: 头(0x1C 指针)+校验和(~sum-0x10)+CpuSet 24B, state[4]=1 |
| 0x08016F30 | ✅C | `Sio_VSyncPump` | VBlank 泵: 交换 0x24/0x28 双缓冲, SIOCNT 启动, TM3 0xC0; 非活动态走 IRQ 检查 |
| 0x08016FC0 | ✅C | `sub_8016FC0` | Multi-SIO 串行 IRQ: 收 0xFEFE 同步头→复位+换 0x24/0x28 与 0x1C/0x20 双缓冲; SIODATA8=((u16*)0x20)[0x14]; 0x24[2][16] 收矩阵; 主机拉 SIO Start+启 TM3; 尾置 [9]=1 (fncheck 252B OK) |
| 0x080170BC | ✅C | `Sio_SetReady` | state[6]=1 |
| 0x080170D0 | ✅C | `Sio_Shutdown` | 关 SIO/Timer3 IRQ, SIOCNT=0x2003 |
| 0x08016C88/6D24/6E80/7120/71E4/7588/75C0/7600/761C/7640/768C/7FA4/8070/82A8/84A8/869C/8744-8838/88BC | ❌ | SIO 状态机族 + gUnk_03004F20 会话结构; 8070=菜单主帧(VBlank 模式2 调用) |
| **0x080177AC** | ❌ | **`BattleTask_Run` (战斗主循环)** — **重归属 F7**: gMainTasks[1], 被 AgbMain 经 gMainTaskSlot=1 进入。调 46 函数: 战斗 UI 命令族(8017FA4/80184A8/801869C/80188BC/8018A58)+链表(8018800/801880C/8018818)+对象系统(80207B4/8020F08/8020F4C/802103C/8021064/802151C/8021700/802192C)+战斗逻辑(804442C/804448C/80457AC/8048DA4/8048FB8/8049C1C/8049DF8/804ADE0/804ADF8/804A148/804A368/804AD60/804B288/804DE20/804EEC4/801FF40) |
| 0x08017588 | ✅C | `Sio_IsHost` | gUnk_03004F20+0x18+idx*24 == 0x4E4C ('LN') 判主机 |
| 0x08017600 | ✅C | `Sio_SetXferCtx` | gUnk_03004F80 字段填充 |
| 0x0801761C | ✅C | `Sio_ClearSlot` | 清会话槽 + Sio_Shutdown |

## 子系统 G: 引擎杂项

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x08018844 | ✅C | `Rng_LcgNext` | LCG 线性同余: seed*0x41C64E6D+0x3039 → &0x7FFF (战斗随机) |
| 0x08018800/801880C/8018818 | ✅C | `ListNode_Init/InitKey/InsertSorted` | 通用有序链表 (prev/next/key) |
| 0x08018864/86C/874 | ✅C | `GetBuf_37028/GetCtx_0248/GetBuf_37410` | 固定 EWRAM/IWRAM 缓冲指针 getter |
| 0x080187xx 族 | ✅C | `Gstate_Get/Set` 族 | gUnk_03000312/314/324/32E/330/340/348 状态字段 getter/setter |
| 0x08018E34/8EA8/8FC0/8D9C | ❌ | (对话窗口系统 gUnk_03000348 消费者) | |
| 0x08019DF8/E04/E18/E24 | ✅C | `BattleUiFlag_Clear/Set/Get/Reset` | gUnk_03000510 位组 |
| 0x0801A13C/148/154 | ✅C | `FlashFlag_Clear/Get/Reset` | gUnk_03000384 |
| 0x0801A168/1DC/218 | ✅C | `BattleFx_Init/Stop/DispOff` | gUnk_030004D4-D7 参数组 + BLDY/BLDCNT |
| 0x08019784 | ✅C | `BattleFx_UpdateTable` | 淡出波表生成器: 按 gFlashFlags 高低位分派, 从 cos/sin 表 (0x0839C26C/0x0839BCCC float[360]) + gUnk_030004D5 生成 gUnk_03000390[256] 波表 (供 80199E0 读/BLDY) |
| 0x08019304 | ✅C | `DialogCtx_Clear3` | 清 gUnk_03000348[0..2] (附注释) |
| 0x08019748 | ✅C | `DialogCtx_SetPair` | 写表项 8 字节 |
| 0x08019EE0 | ✅C | `DialogCtx_SetHead` | field_8/9/A/C |
| 0x0801A05C | ✅C | `DialogCtx_GetField_C` | |
| 0x0801A0F0 | ✅C | `DialogCtx_Flush` | 有内容时 0x02035AC0→0x06007000 DMA |

## RAM 语义确认 (本模块)

- `gWindowBgBuf`(0x02005800) = 窗口/文本 tilemap ✓ (F4a 结论坐实)
- `gUnk_03004D60[]` = **存档页计时器半字节数组** (0x5A 项, 每项≤5)
- `gUnk_03004DC8[]` = 存档相关位图; `gUnk_03004DD4` = 存档 UI 参数; `gUnk_03004DC4/03004DD8` = 存档状态门
- `gUnk_030001D0[]` = 消息指针表 (3 组); `gUnk_03000199` = 背包游标
- `gUnk_03000186/187/188[]` = 菜单焦点游标组; `gUnk_030001B4[]` = 物品使用上下文表
- `gUnk_030001AE` = 物品治愈分支选择 (1=HP, else=MP); `gUnk_030001AF` = 治愈量 (MP 分支 0 → 999);
  `gUnk_030001B0` = 物品使用置位 (0x10); `gMenuResultCode` = 物品使用返回值 (u16, sub_800C2F8 消费)
- `gUnk_03000348[3]` = 对话窗口上下文 (0x14/项); `gUnk_03000328` = 战斗 RNG 种子 (LCG)
- `gUnk_03004DF0` = SIO 会话状态结构 (符号: `gSioState` = u8[](老函数下标视图) 与 `gUnk_03004DF0` = `Unk_03004DF0` 结构视图, 同址 0x03004DF0; 字段见 iwram.h; 0x00=isParent/mode, 0x04=包交换就绪, 0x06=ready, 0x14/0x18=收发列计数, 0x1C/0x20=收发双缓冲, 0x24/0x28=接收缓冲 A/B, 0x2C=收包缓冲)
- `gUnk_03004F20` = SIO 会话结构 (field_4D=槽 idx, +0x18 槽表 24B/项, 0x4E4C='LN' 主机标记); `gUnk_03004F80` = 传输上下文

## 调用图证据 (2026-08-31)

- ~~sub_80177AC=菜单窗口系统~~ **修正: 是战斗主循环** (用户确认 gMainTasks[1]), 已重归属 F7
- 本模块真实的战斗相关部分只剩 8017FA4/80184A8/801869C/80188xx 命令族 (被 177AC 调用)
- 8011454 (存档菜单主控) 无 MOD-02 依赖, 独立于精灵层
