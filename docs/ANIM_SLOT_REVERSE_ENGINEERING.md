# 精灵动画槽系统 (AnimSlot) 逆向工程与字段语义解析报告

> **权威与合规声明**:
> - 本文档由 Antigravity 独立创建与维护，严禁向项目历史共享文档（`docs/progress.md`, `docs/EXPERIENCE.md`, `docs/INCIDENTS.md` 等）追加或覆盖内容。
> - 终验标准: `make` 构建成功且 `sha1sum -c ll.sha1` 100% 逐字节完全匹配（匹配进度 746/1059, 70.4%）。
> - `python3 scripts/audit.py` 746/746 状态全量通过，符号漂移数为 0。
> - 绝对遵守铁律 8: 严禁自动执行 `git commit` / `git push`。

---

## 1. 模块概述与硬件背景

在《Lunar Legend》中，除常规硬件 OAM 精灵外，游戏拥有一个轻量级的软件动画槽系统（**AnimSlot**），用于在特定的 2D 图块缓冲（`0x02006000` / `0x0200E000`）中，对 NPC、动态特效、UI 动态图标进行基于帧时钟（Tick）的连续图块 DMA/CPU 拷贝刷新。

- **全局数据位置**: IWRAM `0x030046A0` - `0x030047A0` (256 字节)。
- **槽位容量**: 16 个动画槽位（`NUM_ANIM_SLOTS = 16`），每个槽位占 16 字节（`0x10`）。
- **目标显存/缓冲基址**:
  - Bank 0: `0x02006000`
  - Bank 1: `0x0200E000` (偏移 `+0x8000`，即 32KB)
- **图块缓冲行跨度（Stride）**: 0x80 个 16-bit 字（即 128 像素宽度 / 256 字节）。

---

## 2. AnimSlot (原 Unk_030046A0) 结构体字段详细分析

```c
typedef struct AnimSlot
{
    /* 0x00 */ u8 activeBank;   /* 槽位状态/模型库选择 (0=空闲, 1=Bank 0, 2=Bank 1) */
    /* 0x01 */ u8 numFrames;    /* 动画总帧数 */
    /* 0x02 */ u8 speedShift;   /* 帧率分频移位量 (当前帧 = tickCounter >> speedShift) */
    /* 0x03 */ u8 flags;        /* 控制标志位 (bit0: ONESHOT, bit1: PAUSED) */
    /* 0x04 */ u8 unk4;         /* 保留字段 / 填充对齐 */
    /* 0x05 */ u8 unk5;         /* 保留字段 / 填充对齐 */
    /* 0x06 */ u8 destTileX;    /* 目标图块列偏移 (以 16-bit 字为单位, 字节偏移 = destTileX * 2) */
    /* 0x07 */ u8 destTileY;    /* 目标图块行偏移 (每行 256 字节, 字节偏移 = destTileY * 256) */
    /* 0x08 */ u8 width;        /* 单帧宽度 (以 16-bit 字/图块为单位) */
    /* 0x09 */ u8 height;       /* 单帧高度 (行数) */
    /* 0x0A */ u16 tickCounter; /* 内部帧步进计时器 (每步自增 1) */
    /* 0x0C */ u8 *srcData;     /* 帧原始图块数据指针 */
} AnimSlot;
```

### 逐字段逆向证据与功能解释

| 偏移 | 类型 | 语义字段名 | 原字段名 | 核心逆向证据与数学模型 |
|:---:|:---:|---|---|---|
| `0x00` | `u8` | `activeBank` | `field_0` | **槽位状态与目标 Bank**。<br>`if (ptr->activeBank == 0) return;`<br>计算目标地址：`bankOff = (ptr->activeBank - 1) << 15;`。<br>当值为 1 时，偏移为 0；为 2 时，偏移为 `1 << 15 = 0x8000` (32KB)。为 0 时槽位处于未分配/休眠状态。 |
| `0x01` | `u8` | `numFrames` | `field_1` | **动画总帧数**。<br>`if (frame >= ptr->numFrames)`：当计算出的帧序号超出总帧数时，判定单轮播放完成，重置 `tickCounter = 0`。若带 `ONESHOT` 标志，则将 `activeBank = 0` 回收。 |
| `0x02` | `u8` | `speedShift` | `field_2` | **帧分频因子**。<br>`frame = ptr->tickCounter >> ptr->speedShift;`<br>每 `1 << speedShift` 个主循环拍（Tick）推进一个逻辑帧。如移位为 3，则每 8 拍推进一帧。 |
| `0x03` | `u8` | `flags` | `field_3` | **播放控制标志位**：<br>- `bit 0 (0x01)`: `ANIM_SLOT_FLAG_ONESHOT`，单次播放模式，循环末端回收；<br>- `bit 1 (0x02)`: `ANIM_SLOT_FLAG_PAUSED`，暂停标志，`AnimSlot_Pause` 写入，`AnimSlot_Step` 直接跳过更新。 |
| `0x04` | `u8` | `unk4` | `field_4` | 保留/未用对齐字节。 |
| `0x05` | `u8` | `unk5` | `field_5` | 保留/未用对齐字节。 |
| `0x06` | `u8` | `destTileX` | `field_6` | **目标图块 X 坐标偏移**。<br>`dest = (u8 *)(bankOff + rowOff + (ptr->destTileX << 1));`<br>以 16-bit 字为步长，即每次递增 2 字节。 |
| `0x07` | `u8` | `destTileY` | `field_7` | **目标图块 Y 坐标行号**。<br>`rowOff = (ptr->destTileY << 8) + 0x02006000;`<br>由于图块缓冲区每行跨度为 128 个 16-bit 字（256 字节），故 `destTileY << 8` 精确计算目标行首地址。 |
| `0x08` | `u8` | `width` | `field_8` | **单帧矩形宽度**。<br>单位为 16-bit 字（每字 2 字节）。拷贝内层循环使用 `while (width != 0) { *(u16 *)dest = *(u16 *)src; ... width--; }`。 |
| `0x09` | `u8` | `height` | `field_9` | **单帧矩形高度（行数）**。<br>外层拷贝行数循环。每拷贝完一行，目标指针跳进到下一行：`dest += (0x80 - width) * 2;`。 |
| `0x0A` | `u16` | `tickCounter` | `field_A` | **内部帧计时计数器**。<br>在 `AnimSlot_Step` 中每轮无条件自增 1：`ptr->tickCounter++;`。<br>在 `AnimSlot_ParseLoop` 中提前初始化为 `(numFrames << speedShift) - 2`，实现首帧闭环。 |
| `0x0C` | `u8 *` | `srcData` | `field_C` | **源动画原始像素图块数据指针**。<br>当前帧源地址计算：`src = srcData + (width * height * frame * 2);`，每帧占用固定字节数 `width * height * 2`。 |

---

## 3. 核心 API 与重命名对照

| 原符号名 | 语义重命名 | 功能说明 | 涉及文件 |
|---|---|---|---|
| `sub_8007A1C` | `AnimSlot_Step` | 推进单个槽位帧计数，按帧图块尺寸与步长搬运至 0x02006000 缓冲 | `src/anim_slot.c`<br>`include/anim_slot.h` |
| `sub_8008BA4` | `AnimSlot_LoadSet` | 从 0x087EA1A0 模型集解包一组连续动画模型并绑定到指定起始槽位 | `src/anim_slot.c`<br>`src/script_vm.c`<br>`include/anim_slot.h` |
| `Unk_030046A0` | `AnimSlot` | 动画槽结构体类型 | `include/anim_slot.h` |
| `gUnk_030046A0` | `gAnimSlots` | 16 项全局动画槽数组 | `linker.ld`<br>`include/anim_slot.h`<br>`src/anim_slot.c` |

---

## 4. 全量验证自证

```bash
$ timeout 900 make 2>&1 | tail -5 && sha1sum -c ll.sha1
arm-none-eabi-objcopy -O binary ll.elf ll.gba
/usr/bin/sha1sum -c ll.sha1
ll.gba: 成功
匹配进度: 746/1059 (70.4%)
ll.gba: 成功

$ python3 scripts/audit.py
=== functions.tsv 函数清单 ===
共 1059 函数 | 已匹配 746 (70%) | 未匹配 313
改名漂移: 0
note 覆盖: 挂起 55/313 | 完成 212/746

=== status=1 字节核验 ===
通过 746/746
```

---

## 5. 关联全局变量逆向分析与重命名 (原 gUnk_0300000A 族)

在 `src/anim_slot.c` 的开场/场景 LOGO 精灵渲染体系中，涉及一组位于 IWRAM `0x03000008` - `0x0300000E` 的全局动画控制变量：

| 原全局符号 | 物理地址 | 类型 | 重命名后语义符号 | 逆向分析与业务含义 |
|---|---|---|---|---|
| `gUnk_0300000A` | `0x0300000A` | `u8[2]` | `gLogoSpriteNodes` | **开场/菜单 LOGO 精灵节点索引表**。<br>`gLogoSpriteNodes[0]` 存储标题 LOGO 背景层精灵节点序号（由 `Sprite_AllocNode()` 分配）；`gLogoSpriteNodes[1]` 存储前景层精灵节点序号。在双态渲染模式中分别提交给 OAM。 |
| `gUnk_03000008` | `0x03000008` | `u8` | `gLogoAnimDirection` | **LOGO 光标呼吸动画递增/递减方向**。<br>`0` = 递增阶段（Timer 递增至 `0xC0` 后翻转为 1）；<br>`1` = 递减阶段（Timer 递减至 `0` 后翻转为 0）。 |
| `gUnk_0300000C` | `0x0300000C` | `u16` | `gLogoAnimTimer` | **LOGO 光标呼吸动画计时器**。<br>在 `0` 至 `0xC0` (192) 范围内往复振荡。在 `BattleIntro_Cursor` 中以 `(gLogoAnimTimer >> 6) + 0x10`（即 16..19）作为渲染排序优先级。 |
| `gUnk_03004604` | `0x03004604` | `u16` | `gWindowTransitionProgress` | **窗口转场动画进度计数器**。<br>记录屏幕水平窗口切割/开闭的像素宽度进度（0 至 `DISPLAY_WIDTH = 240`）。 |
