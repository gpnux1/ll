# MOD-05 场景对象尾段 (0x08020D50-0x080264C0)

> 分析人: plan (2026-08-31)。源文件 `src/code_8020D50.c` (307 行, 22 个真 C 函数)。
> 对象 = 0xC8 字节结构体 (池 0x02037028, `Unk_8020F4C`), 由 0x0801BD0 区 (MOD-04) 创建。

## 角色事件命令执行 (对象 +0xB0/0xBD/0xBE/0xBF/0xC0 状态字段)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x08020D50 | ✅C | `ObjCmd_StartWalk` | 对象 ≤0xA(槽有效): 0xA3=sub_804BBDC(...)移动参数, 0xB0\|=0x80 (移动中) |
| 0x08020DA0 | ✅C | `ObjCmd_EndWalk` | 移动位清除: 0xB0 &= 0xFF7F, 调 sub_804BD54 (落地回调) |
| 0x08020DE4 | ✅C | `SpawnSlots_ClearCount` | gUnk_0300071C=0 |
| 0x08020DF0 | ✅C | `SpawnTable_Load` | 查出生点表 (arg0+0xBD 组号>4 反转 flag), gUnk_03000730_arr←sub_8046480, 计数 073D/073C |
| 0x08020E54/E5C/E68 | ✅C | `SpawnTable_GetBuf/GetCount/Get62C` | getter |
| 0x08020E74/E90/EAC | ✅C | `SpawnSlots_Clear/Mark/Test` | gUnk_03000748[11] 已生成标记 (按对象 0xBE 槽号) |
| 0x08020EC8/EEC/F08 | ✅C | `FxQueue_Clear/Push/Flush` | gUnk_03000758[11] 特效队列 → sub_804C2FC(gUnk_087ED6A8[id], i+1, 1) 生成特效对象 |
| 0x08020F4C | ✅C | `Obj_Init` | 对象初始化: field_BB=0/BC=0xFF/B0=0/BE=0xB/36=0 + 6 个全局清零 + sub_801FA10(obj,0x31) |
| 0x08020FB8 | ✅C | `Obj_StartSlide` | 对象滑动: 0xB0 位段 + gUnk_03000618-624 滑动参数组 (dx=arg1-field_37...) |
| 0x0802103C | ✅C | `Obj_SetWalkAnim` | 0xB0 位段=0x60\|arg2, 0xBD=arg1 |
| 0x080210C0 | ✅C | `Obj_StartJumpArc` | 0xB0\|=0x2000, 0x66=3, 调 sub_801B81C(10 参对象 setter, 跳跃抛物线表 gUnk_0839B2A4[0]) |
| 0x08021130 | ✅C | `TileAnim_Reset` | 清 gUnk_03000788[10][5] + 07BA (tile 动画状态) — System_Init 调用 |
| 0x08021184 | ✅C | `Obj_SyncSlotState` (匹配 2026-09-06) | 对象槽号同步: arg1+0xBE 槽号→idx(非零-1), switch((s8)arg0) case 0/3/6/7 更新 gUnk_0300076A/76C/770/781/782/808/809/80A (步长5表 gUnk_03000788[idx][k]) |
| 0x08021700 | ✅C | `ObjSpawn_StepAll` | 逐对象生成步进: gMenuObjLoadSlots[idx]*0xC8 取对象, 两阶段 (07DC 初始化→等 bit11) |
| 0x0802550C | ✅C | `MenuStyle_Set` | gMenuWindowPhase=value |
| 0x08025638 | ✅C | `PickSlots_Reset` | gMenuSelSlot0/815=-1 |
| 0x0802576C | ✅C | `PickList_Draw` | 选择列表 3 项绘制 (sub_801A074, 选中样式 0xD/0xF) |

## 未匹配 (24 个)

80212B4, 802151C, 8021788, 802181C, 802192C, 8022458, 8022550,
8022710(2072 行 大函数), 8022F2C, 80230BC, 8023320, 8023414, 8023820, 80244BC, 8024618,
80246E8, 8024820, 8024940(2984B 最大), 8025518, 80256E4, 80257D8, 8025994, 8025DA8, 80260BC。

## RAM 语义 (本模块)

- `gUnk_03000730_arr` = 出生点表; `gUnk_0300073C/073D` = 出生点计数 (0xF0 标记计数)
- `gUnk_03000748[11]` = 对象已生成标记; `gUnk_03000758[11]/0763` = 特效生成队列
- `gUnk_03000618-624` = 对象滑动参数组; `gUnk_03000670[7]` = tile 动画对 (Unk_8021064)
- `gUnk_0300068C/68D/68E` = 生成队列头/计数/标志; `gUnk_03000788[10][5]/07BA` = tile 动画
- `gMenuList2Count/2Top/2Cursor`(0x03000808-080A) = 第二选择列表窗口; `gMenuObjLoadSlots[5]/Count/Idx/Phase`(0x0300080C-0813) = 菜单成员精灵装载队列; `gMenuSelSlot0/1`(0x0814/0815, 恒 -1)/`gMenuWindowPhase`(0x0816)/`gMenuWindowFlags`(0x0818, bit0x1000) = 菜单窗口状态
- 对象池 0x02037028 (`sub_8018864`), 0xC8/对象, 关键字段: 0xB0(状态16), 0xBD(动画), 0xBE(槽), 0xBF/C0(朝向/参数), 0x24(bit11=移动完成), 0x35-38(坐标), 0x66(样式)

## 调用图证据 (2026-08-31)

- 尾部 80257D8/8025994/8025DA8 是**对象生成器** (各调 80444A4/8044514 状态服务 78-84 次)
- 与 MOD-04 同一对象 VM 系统; 与 MOD-06 无直接边界 (物理相邻但调用经 0x08044 区中转)
