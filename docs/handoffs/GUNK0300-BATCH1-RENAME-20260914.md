# HANDOFF: gUnk_030xxx 批量语义命名 第 1 批 (41 符号)

- 日期: 2026-09-14, agent: zcode (锚点认领 sub_804442C, 已释放)
- 前置: 同日《gUnk_030xxx 未命名变量分析报告》(对话内交付), 用户已批准命名方案。
- 结果: 41 个 IWRAM 符号重命名 + iwram.h 簇注释 + compat #define 保留;
  make + sha1 OK (ROM 零字节), ll.map 41/41 地址一致, 16 代表函数 fncheck OK。

## 1. 簇 A: BattleTask 物件演出引擎工作区 (25)

驱动: `BattleTask_Run` → `sub_804442C(step)` 复位整块 → 逐帧驱动动作链 (0x03000318)
上对象的 handler (event_actor ~50 / event_hub ~38 / cutscene_mgr 3, 各自
`switch (gUnk_03000820)`)。步骤值域 0x00–0x1F: 0=起手, 1/2=动画, 5-8=等待, 9=收尾,
0x12-0x1A=细分动画, 0x1C-0x1F=特殊。单线程顺序演出。

| 旧 | 新 | 语义 |
|---|---|---|
| gUnk_03000820 | gObjActStep | 步骤 PC |
| gUnk_03000822 | gObjActSavedF2A | 保存 headA.f_1E |
| gUnk_03000824 | gObjActSavedPal | 保存 headA.palSlot |
| gUnk_03000825 | gObjActStepTimer | 步骤帧计数 (插值 t) |
| gUnk_03000826 | gObjActResult | 演出结果 (sub_80446A4 读) |
| gUnk_03000828/29 | gObjActSavedX/Y | 保存 posX/posY |
| gUnk_0300083D | gObjActGroupCount | 多对象组登记数 |
| gUnk_03000840 | gObjActGroupSlots | 组登记表 (低4=池槽, 高4=flags) |
| gUnk_03000844/45/56 | gActWaitBusy0/1/2 | 等待窗互斥 (全 0 = sub_80444E8 放行) |
| gUnk_03000857/8C | gActWaitCnt0/1 | 等待窗子计数 |
| gUnk_03000858 | gActEventCount | 事件计数 (event_hub 2896-2925) |
| gUnk_0300085A | gActWaitFrames | 等待窗时长 (负 → 12) |
| gUnk_03000867/68 | gSceneFadeOut/In | 场景淡出/淡入量 (sub_801A2AC(0x710,·,·)) |
| gUnk_0300086A | gObjActBranch | 演出分支模式 (复位=1) |
| gUnk_0300086B | gObjActParam | 演出参数 (0 / 0xC) |
| gUnk_0300086E/6F | gObjActMoveFromX/Y | 移动插值起点 |
| gUnk_03000884 | gObjActSfxLatch | 一次性音效闩 (sub_8044394) |
| gUnk_03000889 | gSceneTransStep | 场景切换状态机 PC (scene_interact) |
| gUnk_03004F90 | gObjSlotFxCmd | 每 obj 槽演出控制码 [12] (sub_80445E8 写) |

## 2. 簇 B: 逐行 BG 波浪滚动引擎 (6) — sio_link.c 内, 与 SIO 无关

| 旧 | 新 | 语义 |
|---|---|---|
| gUnk_030004D0 | gWaveTablePtr | 运行时生成波形表指针 (常指 gUnk_02036EC0; ROM 常量表另有 gWaveSineTable@0x080576D0, 已有名, 勿混) |
| gUnk_030004D4 | gWaveAngleVel | 相位步进 ((angle+vel)%360) |
| gUnk_03000386 | gWaveAngle | 相位累加器 |
| gUnk_03000390 | gWaveRowOffset | 逐行偏移表 [0xA0] |
| gUnk_030004D8 | gWaveBgHofsTbl | BG HOFS 寄存器地址表 [4] |
| gUnk_030004E8 | gWaveBgVofsTbl | BG VOFS 寄存器地址表 [4] |

注: 消费者含已命名 BattleFx_UpdateTable(0x08019784); **sio_link.c 文件混真 SIO + 波浪引擎,
建议后续拆分**。

## 3. 簇 C: 独立小簇 (9) + 散户 (1)

| 旧 | 新 | 语义 |
|---|---|---|
| gUnk_03000010 | gMenuEntAnimFlags | 实体调色板动画 flags (bit1=锁定) |
| gUnk_03000014 | gMenuEntAnimThreshold | 帧阈值 |
| gUnk_03000018 | gMenuEntAnimShift | 移位量 |
| gUnk_03000020 | gMenuEntAnimCounter | 帧计数 (>>shift 查 gMenuEntPaletteFrames) |
| gUnk_0300068C | gDmgPopupSlot | 弹数字槽 (tile 基号=*4+0x158) |
| gUnk_0300068D | gDmgPopupPhase | 弹出相位 |
| gUnk_0300068E | gDmgPopupLevel | 活动级数 ((phase+1)%(10-level)) |
| gUnk_03000714 | gFxQueueWriteIdx | 效果队列写游标 (队列 gUnk_030006F8[7]) |
| gUnk_03000715 | gFxQueueReadIdx | 读游标 (分析修正: 报告原写 Count, 实为读游标) |
| gUnk_03000004 | gBlendFadeStep | 混合 blend 递增步 (sprite_engine, (x>>2)&0x1F) |

## 4. 实施与验证

- iwram.h: 41 extern 改名 + 41 行 `#define gUnk_old 新名` 兼容宏 (沿 gScript* 惯例,
  防并行 agent 在途引用) + 5 段簇注释。
- linker.ld: 41 个块内符号同步改名 (保持 `. = 0xXXXXXX; SYM = .;` 格式)。
- src/*.c: 10 个文件全量词边界替换 (event_actor/event_hub/obj_state/scene_interact/
  cutscene_mgr/obj_pool/sio_link/battle_obj_core/sprite_engine/player_stats)。
- 验证: make + sha1 OK (869/1059, ROM 零字节); ll.map 41/41 地址一致;
  16 代表函数 fncheck OK (sub_8044394/804442C/80444E8/8044514/8044574/80445E8/80446A4,
  sub_8018A58, BattleFx_UpdateTable, sub_801D568, sub_801E040, LogoBlendEffect_Update,
  PaletteEffects_Update, sub_8031580, sub_8032548, sub_8026D08)。

## 5. 剩余 (未命名 ~126)

- 中等: gUnk_03000048 (text/menu struct, 需解码), gUnk_030047AC (map_view 阈值),
  gUnk_03004D40/4D4C (存档邻域), gUnk_03000500 (sio_link struct)。
- 低热度 ~120: 建议按 TU 分批 (save/dialog/map/menu 各一轮), 同流程: 热度榜 → 读写点
  → 调用图 → 命名 → 批量替换 → map/sha1 验证。
