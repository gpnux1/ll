# BATTLEOBJ-FIELDS — BattleObj 字段语义核查与命名 (2026-09-13, defcdgg-zcode)

## 结果

- 结构体含义**确认正确**: 0xC8 池对象 = 战斗单位 (玩家 ≤0xB / 敌方 ≤0x70 / 特效 0x71+),
  两个 ObjHead 图形头 + 战斗数值区 + 状态区。`make`+SHA1 绿 (802/1059), fncheck 16 函数 OK。
- 16 个字段获得证据命名 (E2/E3), 4 个语义未定保留 f_XX (f_B4/f_B6/f_BD/f_C3) + 补全消费者注释。
- 全部为等偏移 token 改名, 代码生成不变。

## 关键突破: sub_80200E8 = PlayerStats → BattleObj 装载器 (E3 级锚点)

asm/nonmatchings/sub_80200E8.s 逐字段搬运 &gPartyStats[n] (PlayerStats, iwram.h):
hp→hp, max_hp→maxHp, mp→mp, max_mp→maxMp,
(base_atc+equip_atc)→atc, (def/agl/men/res 同构)→def/agl/men/res,
equip_slot1..6→equipSlots[6] (+0x8D..0x92), skills[i]-1→skills[8] (+0x99..0xA0, 0xFF/0x26=空),
noa→noa(+0xA9), lv→lv(+0xAA), variantClass=0, +0x7E..0x87 连清 5×u16, +0xAC=arg2, f_C3=0x10, +0xC4=0x10。
PlayerStats 词表 (hp/mp/atc/def/agl/men/res/noa/luc/max_hp/equip_*) 与本结构交叉验证成立。

## 次级锚点

- **sub_804A368** (0x0804A368): 遍历 5 成员 (0xC8 步长), `statusAil & 1` (中毒) →
  `dmgAmount = maxHp/10 (sub_8048D64)` → `sub_801D568(obj)` 弹数字 → (后段) `hp -= dmgAmount`。
  ⇒ statusAil bit0=中毒, dmgAmount=待结算伤害, maxHp 语义三重验证。
- **sub_801B964** (动画流解释器, 出场装载): slot/posX/posY ← 阵型表 4 字节条目
  ([id*4+1]=slot, [+2]=X, [+3]=Y); memberIdx = sub_80487CC(memberId) = 队伍位次 0-5
  (查 0x03004A88[], 0xA1/0xA7→2); fxKind=0xFF; f_BD=0; posC1 清 0。
- **sub_801D568**: dmgAmount 弹 3 位数字 (≤999 夹断), 锚 (posX-16, posY-8) → 0x03000670 表。
- **sub_801FF40/8020AE4**: 0x03000690 队列逐帧 dmgAmount++, (n+1)*40 vs rand%101 且 n>4
  才可被选为副代表 (按 memberIdx 判重) — dmgAmount 的队列计数复用, 已在注释如实记录。
- **sub_802B608** (战斗事件 actor): posX/posY → 0x03000828/0x03000829 全局。
- **sub_801CF90** (动画状态机): 读 substate(+0xA2)/flashLevel(+0xA3), 以 +0x70 为基址访问。
- **sub_8048B5C**: equipSlots[4]/[5] (0x91/0x92)==0xB3 (空) → animPtr 低 16 位写 0x20, 否则 arg1。

## 命名总表 (offset: 新名 ← 证据)

0x6C hp / 0x6E maxHp / 0x70 mp / 0x72 maxMp / 0x74 atc / 0x76 def / 0x78 agl / 0x7A men /
0x7C res / 0x8D equipSlots[6] / 0x99 skills[8] / 0xA2 substate / 0xA3 flashLevel /
0xA9 noa / 0xAA lv / 0xAB variantClass / 0xB2 dmgAmount / 0xB8 statusAil / 0xBB memberIdx /
0xBC fxKind / 0xBF posX / 0xC0 posY / 0xC2 animSubIdx

保留编号: f_B4/f_B6 (动画表 +0xC/+0xE 装载, 战斗运算广泛读写, 语义未定)、
f_BD (出场 0, 常见 0-5, >4 改变 sub_8020DF0 扫描模式)、f_C3 (出场 0x10, animPtr[0x23]/[0x24]
源, 战斗对象逐帧大量读)。

## 补遗 (2026-09-13 晚): statMods 命名

- +0x7E..0x87 = **`u16 statMods[5]`** ([0..4]↔atc/def/agl/men/res 修正值): E2 证据 =
  sub_8046F0C (属性取值器) case5-9 将 +0x74..0x7C 与本区成对相加截断 u16;
  sub_8048D40 (我方战斗开始清零) 与 sub_80200E8 (装载) 连清 5×u16;
  sub_8048CEC 以 [0]/[1] 非零作状态标记 (结果 1-4)。sub_80200E8 已改用字段访问 (fncheck OK)。

## 未决

- +0x93..0x98、+0xA4..0xA8、+0xAC..0xAF (0xAC=装载 arg2, sub_8022710 变址读)、+0xBA、
  pad_C1/C4 的完整语义。
- animPtr 低 16 位复用 (slot≤6 计数器) 是否需要拆成 union 视图 — 待更多消费者后再动。
