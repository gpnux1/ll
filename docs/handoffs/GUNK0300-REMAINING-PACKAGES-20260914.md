# 任务包: gUnk_030xxx 剩余未命名符号分类 (2026-09-14, zcode)

> 供多 agent 认领。每包自包含: 符号表 / 入口函数 / 已有证据 / 建议步骤 / 验证要求。
> 数量 (2026-09-14 晚间复核): **剩余 68** (58 有引用 + 14 零引用占位) / 分类时 115。
> 进度: A1 ✅13 (zcode) | C1 ✅7 (zcode) | B1 部分 ✅4 (zcode) | A2 ✅~16 (zcode-menu2-808:
> 0x03000808-0818、0x03000882/0886/0888 改名 gActHitDmgAmount/gActWaitSfxId/gActWaitSfxParam,
> 0x0300088B-08A5 就地 /// 注释保留地址名; 0x0782→gMenuListCursor)。
> gUnk_03000830→gTargetSlotList / gUnk_0300083C→gTargetSlotCount (当前动作候选目标槽位表/数,
> event_hub:634 敌侧 mode1 / sub_8032EA0 我侧 mode0 枚举, B1 包范畴) — ✅ 2026-09-14 zcode。
> 已完成批次见 GUNK0300-BATCH1-RENAME-20260914.md (41 符号) 与 STRUCTS/RENAME 两份。

## 全局规约 (每个包都必须遵守)

1. 认领: `export DECOMP_AGENT=<你的名字>; scripts/claim.sh <锚点函数>` (各包指定锚点)。
2. 命名落地三件套: iwram.h (extern 改名 + `#define gUnk_old 新名` 兼容宏 + 簇注释) /
   linker.ld (iwram/ewram 块内 `. = 0x0000XXXX; SYM = .;` 行同步改名, **禁止顶层绝对赋值**) /
   src 词边界替换。
3. 验证四件套: `make` → `sha1sum -c ll.sha1` → ll.map 前后符号地址比对 (必须 100% 一致) →
   `fncheck` 使用该符号的代表性函数。**纯改名不允许 ROM 有任何字节变化。**
4. 证据分级标注 (E0/E1/E2/E3/H), 命名依据不足的宁可保留地址名并写明卡点。
5. 并行 agent 在途: zcode-dlg-9xx (dialog), claude804AF60 (sub_804AF60), zcode-pal-ae0
   (palette) — 勿动其编辑区; 遇 shared 文件冲突先重读原文。

---

## PKG-0300-A1 战斗结算/属性计算 (13 符号) ✅ 已完成 (2026-09-14 zcode)
> 13 符号已按建议名落地 (iwram.h+compat 宏/linker.ld/battle_engine.c);
> make+sha1 OK, ll.map 13/13 地址一致, fncheck 7 函数 OK (8048B30/8048ACC/804AC60/
> 8046F0C/80448A8/80494F0/804EC04)。遗留: 0x8F1 计数器消费点、09C5 值域闭环待 asm 侧确认。
- 锚点认领: `scripts/claim.sh sub_80494F0`
- 文件: battle_engine.c; 入口: sub_80494F0 (结算总驱动, 未匹配 318 行),
  sub_804EC04 (结算状态机), sub_804AC60 (结算画面), sub_8048B30, sub_8048ACC。
- 已有证据 (zcode, E2):
  - gUnk_030009BE=s8 掉落记录数 (BattleDrops_Roll 返回值写入, 初由 09BF=-1 配套)
  - gUnk_030009BF=s8 结算掉落选择索引 (×4 遍历 drop 记录, 注释块 battle_engine:2060-2080)
  - gUnk_030009C0=u32* 掉落表指针 (=gBattleDrops, BattleDrops_Roll 写入)
  - gUnk_030009C4=u8 结算步骤完成标志 (sub_804EC04 返回值落点)
  - gUnk_030009C5=s8 结算视图类别 (sub_804AC60 以它索引状态块)
  - gUnk_030009C8=u32* 结算状态块指针 (sub_804EC04 的 out 槽)
  - gUnk_03000908=u16 技能 HP 恢复值 (battle_engine:66 注释已述)
  - gUnk_030008F0=u8 属性重算类别 (sub_8048ACC 递归重算 statMods, "case5-9=基础+修正")
  - gUnk_030008EC=u32 属性重算池基址 (=GetObjPool())
  - gUnk_030008F1/8F2/8F3/0906: sub_8048B30(kind, 0x1E, ObjAnimEntry索引) 成组写入,
    第三参 0x362/0x371/0x3AD/0x3CB = gUnk_08393B28 效果表索引 (E2×6 调用点) →
    能力变化演出请求组。
- 建议名 (可直接采用):
  gResultsDropCount / gResultsDropSelIdx / gResultsDropTablePtr / gResultsStepDone /
  gResultsViewKind / gResultsStatePtr / gSkillHealAmount / gStatRecalcKind / gStatRecalcPool /
  gFxReqTimer / gFxReqKind / gFxReqFrames / gFxReqAnimIdx
- 待办: 0x8F1 计数器消费点确认 (谁读它递增/比较 0x1E); 09C5 的 0/1/2 值域闭环。

## PKG-0300-A2 演出等待窗/obj_state 补遗 (15) ✅ 已完成 (zcode-menu2-808)
> 0x03000882→gActHitDmgAmount, 0x03000886→gActWaitSfxId, 0x03000888→gActWaitSfxParam;
> 0x03000808-0818 与 0x0300088B-08A5 已改名/就地注释 (见 iwram.h 当前状态)。
> 本包关闭。
- 锚点: `scripts/claim.sh sub_8044514`
- 文件: obj_state.c (0x865/0882/0886/0888), scene_obj_fx.c (0x03000808..0x03000818)。
- 已有证据: 0x886=sub_8044574 的等待目标 (复位 0x37), 0x888=模式 (arg2), 0x882=sub_8044420
  返回值, 0x865=sub_8044498 返回值 (谁置 1 待查); 0x03000808..0x03000818 全部在
  scene_obj_fx.c — 场景物件效果参数区, 需读其消费函数定语义。
- 建议名前缀: gActWaitTarget/gActWaitMode (0x886/888, E2); 其余取证后以 gSceneFx* 前缀。

## PKG-0300-B1 BattleTask 队列/任务区 (剩 20) ★ 最大包
> ✅ 部分完成 (2026-09-14 zcode): 待选池段 4 变量 + 类型已落地 —
> gUnk_03000690→gTaskPoolHead (链表头哨兵), gUnk_030006A0→gTaskPoolNodes
> (类型 Unk_030006A0→TaskPoolNode, 5 项×16B 按池槽索引), gUnk_030006F0→gTaskPoolCount,
> gUnk_030006F8→gFxQueueObjs (效果/步进队列, 消费者 sub_801E040)。
> make+sha1 OK, map 4/4 一致, fncheck 5 函数 OK (801DC20/801DD04/801E040/801FF40/8020AE4)。
> 语义: 等待被选中对象池 — 池内 dmgAmount 逐帧+1 蓄力 (sub_8020AE4), sub_801FF40 按权重挑选。
- 锚点: `scripts/claim.sh BattleTask_Run`
- 文件: battle_obj_core.c; 入口: BattleTask_Run, sub_801DC20/801DD04 (挂/摘链),
  sub_80187E8, sub_801EA70 (其注释已给 0x03000638[0x03000669] 线索)。
- 线索: 0x0618-0x0624 六个 u16 成组 (疑似任务字段), 0x062C=u32 (入场对象计数, 已在
  sub_8020C58/sub_804E9DC 作 [0]=count 消费), 0x0638=u8*[12] 行动对象指针表,
  0x0668/0669=当前行动索引对, 0x0730/073C/073D/0744/0748=任务状态组,
  0x0716/0718/071C=效果队列邻域。
- 注意: 本包与 A1/BattleDrops_Roll 有交集 (0x062C), 认领前对 .claims --list。

## PKG-0300-B2 scene_obj_fx 补遗 (剩 7)
- 锚点: `scripts/claim.sh sub_8020CC4`
- 文件: scene_obj_fx.c (0x0758/0763/0765/0768/076A/076C/076E/0770/0781/0782/0784)。
- 线索: 0x770/0x781 x5/x5 是热符号; 该 TU 已注释较密, 从现有注释顺延。

## PKG-0300-C1 sio_link.c 剩余 (9) ✅ 已完成 7 个, 剩 0x0512/0514 (2026-09-14 zcode)
> gKeysHeld(0x310, 本帧按键 ~REG_KEYINPUT)/gKeyIgnoreTimer(0x316, 输入屏蔽倒计时 10 帧)
> — **修正: 0x310/0x316 与 SIO 无关, 是按键输入子系统**; gWaveAmp(0x4D5, BattleFx_Init arg1)/
> gWaveRowStep(0x4D6, 逐扫描线角度步进)/gWaveMode(0x4D7, BattleFx_Init arg3, Stop 清零,
> 推断由 asm HBlank 消费)/gBgLoadSlot(0x4F8, 4×4KB LZ77 分帧解压 VRAM 0x06008000 进度,
> E3: BgLoad_* 函数名)/gBgScrollBackup(0x500, struct 升级 BgScrollBackup{bg0..bg3 Hofs/Vofs})。
> make+sha1 OK, map 7/7 一致, fncheck 9 函数 OK。剩余: 0x0512/0x0514 (只写不读, 证据不足
> 保留地址名, sub_801A348 复位)。
- 锚点: `scripts/claim.sh BattleFx_UpdateTable`
- 已有证据 (zcode, E2/E3): 0x04D5=波形振幅 (float cos/sin 生成式 877), 0x04D6/04D7=波形参数,
  0x04F8=x7 热符号, 0x0500=struct{field_0/2/6}, 0x0512/0514=单点; 0x0310/0316 (sio_link)
  可能是真 SIO 收发缓冲 — **与波浪引擎分开判定, 勿混**。
- 建议名前缀: gWave* (波浪), gSio* (真 SIO)。

## PKG-0300-D1 存档/菜单邻域 (6)
- 锚点: `scripts/claim.sh sub_8040EE8` (或 save TU 任一已匹配热函数)
- 符号: 0x04D40 (menu/menu_ui/text_engine ×11! 跨 TU — 注意其真实归属可能是菜单而非存档),
  0x04D48 (menu/script_vm), 0x04D4C (menu_ui/scene_mgr/engine_core ×15), 0x04DBC (menu),
  0x04DE4 (menu), 0x04860 (script_vm/player_stats)。
- ⚠ 地址邻接 gSaveFsmState(0x04D44) 不代表语义同簇 — 0x04D40 消费者全是菜单 TU,
  先证伪"存档"假设再命名。

## PKG-0300-D2 0x03004DF0 命名争议 (1) ✅ 已裁决 (2026-09-14 zcode)
> 结论: **无冲突** — gSioState 是 0x130 字节 SIO 工作区的原始 u8 视图 (CpuFill32 清零
> 0x4DF0..0x4F20), gUnk_03004DF0 是其多机通信协议结构化视图; text_engine.c 混住 SIO
> 协议层 (sub_8016E80/8016FC0), 与 sio_link.c 的混居模式一致。
> 落地: gUnk_03004DF0→gSioCommState (类型 SioCommState), 字段 recvAccumMap/recvDoneMap/
> swapPending/frameHasPacket/peerReady 落地; unk_8/unk_A/unk_18 及双缓冲方向待 SIO 族匹配。
> make+sha1 OK, map 地址一致, fncheck sub_8016FC0/8016E80 OK。
- iwram 块里 0x4DF0 同址有 `gSioState`(旧命名) 与 `gUnk_03004DF0`(struct, x30, 消费者
  **text_engine.c**)。两个互斥假设: 真 SIO 状态 vs 文本引擎状态。裁决后合并名字并修块行。
- 入口: text_engine.c 的 30 个引用点 + sio 接收路径是否写它。

## PKG-0300-E1 地图/杂项单点 (7)
- 0x047AC (map_view ×6, 绘制阈值方向), 0x04618 (sprite_engine), 0x04688 (map_view),
  0x02C40 (engine_core s16 ×4), 0x01EE0 (vram_transfer, Actor[] — 邻近 gActors 家族?),
  0x01950 (engine_core u32[14]), 0x07FF8 (text_engine/engine_core, IWRAM 尾槽)。
- 每符号独立取证, 工作量小但分散; 适合顺手包。

## PKG-0300-F1 文本/UI struct (3) ✅ 已完成 (2026-09-14 zcode)
> gUnk_03000048→gMenuCursorSprite (类型升级 MenuCursorSprite, 0x10B = UISpriteEntity 前
> 0x10 字节同布局; 10 个字段语义化 statusFlags/animTimer/lerpFrame/oamSlotId/x/y/
> moveEndX/Y/moveStartX/Y — 滑动光标: 目标取 gUnk_087EB1F4 族[组][选择])。
> gUnk_03000028→gMenuEntPalDest (OBJ 调色板目标地址 0x05000002+slot*0x20),
> gUnk_03000038→gMenuEntAnimFrameTbl (帧号表指针, 索引 gMenuEntPaletteFrames) —
> 与 gMenuEntAnim* 四件套凑齐调色板动画簇。make+sha1 OK, map 3/3, fncheck 2 函数 OK。
- 0x000048 (x22, struct{field_0/4/6}, menu/menu_ui/text_engine 三 TU 共享 — 解码布局后
  升级为命名 struct), 0x000028 (u32[4], player_stats), 0x000038 (u8*[4], player_stats)。

## PKG-0300-G1 全局系统/菜单 UI 区 (10)
- 0x000000/0x000002 (scene_mgr, IWRAM 最前两字节 — 常见为全局模式/帧相),
  0x000010 (data_805769C 注释: MenuEnt 状态位), 0x00184-0x001C8 (menu_ui ×7, 菜单游标区),
  0x00198 (menu/text_engine)。

## PKG-0300-H1 零引用死声明 (10) ★ 需要 asm/ROM 字面量取证 (C 侧无证据)
- 0x03000204/0208/0210/0229/022B/022C/0240/0248, 0x03000317/032C, 0x0300464C/04650/047C4/047EC
  (14 个) + 复核 gUnk_03004DC0 (纯 linker 别名, 建议直接删除或在文档标注)。
- 方法: ROM 字面量扫描 (0x0300xxxx 在 code.s 池中) → 定位访问指令 → 归属函数 → 语义。
- 另: 若确认代码从未引用, 也可只写"仅占位"注释保留。

## 建议分派顺序
1. ~~A1~~ ✅已完成 → 2. **A2** → 3. **B2** → 4. ~~C1~~ ✅已完成(7/9, 0x512/0x514 证据不足保留) →
5. **D1/G1** (~~F1~~ ✅) (菜单 UI 簇可合并给同一 agent) → 6. **B1** (最大, 建议留给熟悉
BattleTask 的 agent) → 7. **E1** → 8. ~~D2~~ ✅ → 9. **H1** (需要 ROM 扫描技能)。
