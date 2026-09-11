# GBA 硬件寄存器与 DMA 操作代码规范 (RULES_HARDWARE_IO)

> **适用范围**: 全项目所有反编译源码 (`src/*.c`)、头文件 (`include/**`) 以及 Permuter 套件 (`permuter/**/base.c`)。  
> **唯一原则**: **杜绝裸地址与手工寄存器展开，一律使用官方标准宏**。  
> **关联依据**: `AGENTS.md` §3 (硬约束)、`docs/EXPERIENCE.md` 经验 55 (DMA 展开形态) 与 经验 96 (REG 宏规范)。

---

## 1. 核心准则 (Golden Rules)

在反编译和函数匹配过程中，任何涉及 Game Boy Advance 硬件底层 I/O 寄存器或 DMA 控制器的代码，**必须严格遵守以下两条铁律**：

### 铁律 1: 禁止裸写硬件 I/O 寄存器地址
- ❌ **禁止**: 裸写物理地址指针强转，例如：
  ```c
  *((vu16 *)0x04000000) &= 0xFDFF;
  *(u16 *)0x04000054 = 0x10;
  *((vu16 *)0x04000048) = 0x3F;
  ```
- ✅ **必须**: 使用 `include/gba/io.h` 定义的标准 `REG_*` 宏：
  ```c
  REG_DISPCNT &= 0xFDFF; // 或使用位常量宏: REG_DISPCNT &= ~DISPCNT_BG1_ON;
  REG_BLDY = 0x10;
  REG_WININ = 0x3F;
  ```

### 铁律 2: 禁止手动展开 DMA 控制寄存器块
- ❌ **禁止**: 手写局部指针指向 DMA 控制寄存器并逐个字段赋值，例如：
  ```c
  {
      vu32 *dmaRegs = (vu32 *)0x040000D4;
      dmaRegs[0] = (vu32)src;
      dmaRegs[1] = (vu32)dest;
      dmaRegs[2] = (vu32)((0x8000 << 16) | (size / 2));
      dmaRegs[2]; // 空读
  }
  ```
- ✅ **必须**: 使用 `include/gba/macro.h` 定义的标准 `DmaCopy*` / `DmaFill*` / `DmaSet` 宏：
  ```c
  DmaCopy16(3, src, dest, size);
  ```

---

## 2. 为什么必须使用标准宏？

### 2.1 零汇编代价 (C 预处理器等价展开)
很多初学者误以为使用宏会增加代码开销或导致汇编不匹配。**事实上完全相反**：
- `REG_DISPCNT` 在 `include/gba/io.h` 中的定义本质就是：
  ```c
  #define REG_BASE               0x4000000
  #define REG_OFFSET_DISPCNT     0x0
  #define REG_ADDR_DISPCNT       (REG_BASE + REG_OFFSET_DISPCNT)
  #define REG_DISPCNT            (*(vu16 *)REG_ADDR_DISPCNT)
  ```
  预处理器在词法阶段将其直接替换为 `(*(vu16 *)0x04000000)`。**生成的汇编指令与裸写 100% 逐比特一致**！
- `DmaCopy16(3, src, dest, size)` 在 `include/gba/macro.h` 中的宏定义：
  ```c
  #define DmaSetUnchecked(dmaNum, src, dest, control) { \
      vu32 *dmaRegs = (vu32 *)REG_ADDR_DMA##dmaNum; \
      dmaRegs[0] = (vu32)(src); \
      dmaRegs[1] = (vu32)(dest); \
      dmaRegs[2] = (vu32)(control); \
      dmaRegs[2]; \
  }
  ```
  `vu32 *dmaRegs = (vu32 *)0x040000D4` 正是当年官方 SDK 开发人员写的宏展开产物！反汇编中看到的展开结构原本就是这个宏的本体。

### 2.2 避免 volatile 缺失导致的灾难性优化 (经验 96)
- GBA 硬件寄存器必须是 `volatile`（`vu16` / `vu32` / `vu8`）。
- 如果开发者裸写为普通非 volatile 指针（例如 `*(u16 *)0x04000054 = v;`），GCC 2.95 会认为这是普通内存访问：
  - 会将多次写入合并为单次；
  - 会将寄存器读取与其他指令任意乱序重排；
  - 会将“未使用的空读”作为死代码直接删除；
  - 最终生成的机器码与 Target ROM 产生严重偏差，且难以排查。

### 2.3 保证代码可读性与长久维护性
反编译工程的最终目标不仅是 SHA1 校验通过，更是恢复一份**人类可读、具有工程语义的高质量 C 源码**。充满 `0x04000000`、`0x040000D4` 魔法数字的代码无法进行跨平台移植或引擎分析。

---

## 3. GBA 硬件寄存器 (`REG_*`) 映射与速查表

所有宏均定义于 `include/gba/io.h`。

### 3.1 屏幕与背景控制 (Display & Background)

| 物理地址 | 数据类型 | 标准宏名称 | 硬件功能描述 | 典型操作示例 |
|---|---|---|---|---|
| `0x04000000` | `vu16` | `REG_DISPCNT` | LCD 显示控制寄存器 | `REG_DISPCNT &= ~0x0200;` / `REG_DISPCNT \|= 0x0100;` |
| `0x04000004` | `vu16` | `REG_DISPSTAT` | LCD 状态与中断发生设置 | `if (REG_DISPSTAT & 1) ...` (检测 V-Blank) |
| `0x04000006` | `vu16` | `REG_VCOUNT` | 当前扫描线计数 (V-Count) | `while (REG_VCOUNT < 160);` |
| `0x04000008` | `vu16` | `REG_BG0CNT` | BG0 控制 (大小/图块基址/图层优先级) | `REG_BG0CNT = 0x1C00;` |
| `0x0400000A` | `vu16` | `REG_BG1CNT` | BG1 控制 | `REG_BG1CNT = 0x1E01;` |
| `0x0400000C` | `vu16` | `REG_BG2CNT` | BG2 控制 | `REG_BG2CNT = 0x1F02;` |
| `0x0400000E` | `vu16` | `REG_BG3CNT` | BG3 控制 | `REG_BG3CNT = 0x1D03;` |
| `0x04000010` | `vu16` | `REG_BG0HOFS` | BG0 水平滚动偏移 | `REG_BG0HOFS = x;` |
| `0x04000012` | `vu16` | `REG_BG0VOFS` | BG0 垂直滚动偏移 | `REG_BG0VOFS = y;` |
| `0x04000014` | `vu16` | `REG_BG1HOFS` | BG1 水平滚动偏移 | `REG_BG1HOFS = x;` |
| `0x04000016` | `vu16` | `REG_BG1VOFS` | BG1 垂直滚动偏移 | `REG_BG1VOFS = y;` |

### 3.2 窗口与混合特效 (Window & Color Blend)

| 物理地址 | 数据类型 | 标准宏名称 | 硬件功能描述 | 典型操作示例 |
|---|---|---|---|---|
| `0x04000040` | `vu16` | `REG_WIN0H` | Window 0 水平范围 (高8位左, 低8位右) | `REG_WIN0H = (left << 8) \| right;` |
| `0x04000042` | `vu16` | `REG_WIN1H` | Window 1 水平范围 | `REG_WIN1H = (left << 8) \| right;` |
| `0x04000044` | `vu16` | `REG_WIN0V` | Window 0 垂直范围 (高8位上, 低8位下) | `REG_WIN0V = (top << 8) \| bottom;` |
| `0x04000046` | `vu16` | `REG_WIN1V` | Window 1 垂直范围 | `REG_WIN1V = (top << 8) \| bottom;` |
| `0x04000048` | `vu16` | `REG_WININ` | 内部窗口 (WIN0/WIN1) 图层显示控制 | `REG_WININ = 0x3F;` |
| `0x0400004A` | `vu16` | `REG_WINOUT` | 外部窗口与 OBJ 窗口图层显示控制 | `REG_WINOUT = 0x1F;` |
| `0x0400004C` | `vu16` | `REG_MOSAIC` | 马赛克尺寸控制 (BG/OBJ 水平垂直步长) | `REG_MOSAIC = 0;` |
| `0x04000050` | `vu16` | `REG_BLDCNT` | 色彩特效与混合模式控制 | `REG_BLDCNT = 0x0140;` |
| `0x04000052` | `vu16` | `REG_BLDALPHA` | 半透明 Alpha 混合权重系数 (EVA/EVB) | `REG_BLDALPHA = (evb << 8) \| eva;` |
| `0x04000054` | `vu16` | `REG_BLDY` | 亮度变化权重 (淡入淡出系数 0..16) | `REG_BLDY = fadeLevel;` |

### 3.3 按键、中断与定时器 (Keypad, Interrupts, Timers)

| 物理地址 | 数据类型 | 标准宏名称 | 硬件功能描述 | 典型操作示例 |
|---|---|---|---|---|
| `0x04000130` | `vu16` | `REG_KEYINPUT` | 按键状态输入 (低电平有效 0=按下) | `keys = ~REG_KEYINPUT & 0x3FF;` |
| `0x04000132` | `vu16` | `REG_KEYCNT` | 按键中断控制 | `REG_KEYCNT = 0;` |
| `0x04000200` | `vu16` | `REG_IE` | 中断使能寄存器 (Interrupt Enable) | `REG_IE \|= INTR_FLAG_VBLANK;` |
| `0x04000202` | `vu16` | `REG_IF` | 中断请求标志 (写1确认清除) | `REG_IF = INTR_FLAG_VBLANK;` |
| `0x04000204` | `vu16` | `REG_WAITCNT` | 游戏卡带总线等待周期控制 | `REG_WAITCNT = 0x4317;` |
| `0x04000208` | `vu16` | `REG_IME` | 全局中断总开关 (0=禁用, 1=使能) | `REG_IME = 1;` |
| `0x04000100` | `vu16` | `REG_TM0CNT_L` | 定时器 0 计数器值 | `REG_TM0CNT_L = 0;` |
| `0x04000102` | `vu16` | `REG_TM0CNT_H` | 定时器 0 控制位 (使能/分频比/级联) | `REG_TM0CNT_H = 0x0080;` |

> ⚠️ **字节宽度与特殊访问规范**:
> - GBA 绝大多数 I/O 寄存器为 16 位半字访问 (`vu16`)。
> - 若 Target 反汇编中出现针对寄存器的字节指令（`ldrb` / `strb`），一律使用地址转换形态：
>   ```c
>   *(vu8 *)&REG_DISPCNT = 0x80;       // 正确: 访问 REG_DISPCNT 低字节
>   *((vu8 *)&REG_DISPCNT + 1) = 0x01; // 正确: 访问 REG_DISPCNT 高字节
>   ```
>   严禁直接写裸地址 `*(vu8 *)0x04000001 = 0x01;`。

---

## 4. DMA 控制与标准宏规范

所有宏均定义于 `include/gba/macro.h`。

### 4.1 DMA 寄存器物理通道地址

| DMA 通道 | 控制块基地址 (`vu32 *`) | 对应常量宏 | 32位源地址 (`SAD`) | 32位目的地址 (`DAD`) | 32位控制字 (`CNT`) |
|---|---|---|---|---|---|
| **DMA 0** | `0x040000B0` | `REG_ADDR_DMA0` | `REG_DMA0SAD` (`+0x0`) | `REG_DMA0DAD` (`+0x4`) | `REG_DMA0CNT` (`+0x8`) |
| **DMA 1** | `0x040000BC` | `REG_ADDR_DMA1` | `REG_DMA1SAD` (`+0x0`) | `REG_DMA1DAD` (`+0x4`) | `REG_DMA1CNT` (`+0x8`) |
| **DMA 2** | `0x040000C8` | `REG_ADDR_DMA2` | `REG_DMA2SAD` (`+0x0`) | `REG_DMA2DAD` (`+0x4`) | `REG_DMA2CNT` (`+0x8`) |
| **DMA 3** | `0x040000D4` | `REG_ADDR_DMA3` | `REG_DMA3SAD` (`+0x0`) | `REG_DMA3DAD` (`+0x4`) | `REG_DMA3CNT` (`+0x8`) |

> ⚠️ **高频陷阱**: 遇到 `0x040000D4` 时，**绝不能凭空当成普通指针**！它就是通用数据传输最常用的 **DMA 通道 3**！

### 4.2 常用标准宏速查

```c
// 1. 数据块拷贝 (自增源地址，自增目的地址，立即启动)
DmaCopy16(channel, src, dest, sizeBytes); // 16 位半字拷贝 (sizeBytes 必须为偶数)
DmaCopy32(channel, src, dest, sizeBytes); // 32 位全字拷贝 (sizeBytes 必须为 4 的倍数)

// 2. 数据填充 (固定源地址，自增目的地址，立即启动)
DmaFill16(channel, value, dest, sizeBytes); // 16 位常数填充
DmaFill32(channel, value, dest, sizeBytes); // 32 位常数填充

// 3. 内存清零
DmaClear16(channel, dest, sizeBytes); // 填 0 (16位传输)
DmaClear32(channel, dest, sizeBytes); // 填 0 (32位传输)

// 4. 等待 DMA 传输完成
DmaWait(channel); // 轮询 CNT_H 的 DMA_ENABLE 位直到变为 0

// 5. 自定义高级控制字设置
DmaSet(channel, src, dest, controlWord32);
```

### 4.3 为什么反汇编中有一条“看似无用的空读”？ (经验 55)

在 GCC 2.95 编译 `DmaCopy` 宏时，生成的 Thumb 汇编序列几乎千篇一律：
```arm
ldr  r2, =0x040000D4    @ 加载 DMA3 基地址
str  r0, [r2, #0]       @ 写入 Source Address (dmaRegs[0])
str  r1, [r2, #4]       @ 写入 Destination Address (dmaRegs[1])
str  r3, [r2, #8]       @ 写入 Control Word (dmaRegs[2]) -> 触发 DMA 开始！
ldr  r3, [r2, #8]       @ 核心特征: 对 dmaRegs[2] 执行一次空读！
```

**工程原因**:
- GBA 硬件架构中，ARM7TDMI 写入 DMA 控制寄存器的最高位（`DMA_ENABLE = 1`）后，DMA 控制器需要一个总线时钟周期锁定内部总线并接管 CPU。
- 任天堂官方 SDK 在设置完控制字后，硬性编写了一条 `dmaRegs[2];`（volatile 变量求值但未赋值给任何变量）。
- 这条看似无用的求值语句触发了一次显式的 `ldr`，强制刷新 ARM 流水线等待总线生效。
- **只要在反汇编中看到 `str ... #8` 紧跟着同基址的 `ldr ... #8`，100% 就是 `DmaSet` 或 `DmaCopy*` 宏展开！**

---

## 5. 典型重构范例 (Before vs After)

### 范例 1: 修改屏幕显示图层 (`sub_804FB24`)

- ❌ **违规写法 (绝对禁止)**:
  ```c
  case 2:
      if (gCurrentMapId == 0x63)
      {
          *((vu16 *)0x04000000) |= 0x0400;
      }
      else
      {
          *((vu16 *)0x04000000) |= 0x0800;
      }
      break;
  
  case 1:
      gViewportFlags[0] &= 0xFFFB;
      *((vu16 *)0x04000000) &= 0xFDFF;
      break;
  ```

- ✅ **标准写法 (100% 推荐，字节完全一致)**:
  ```c
  case 2:
      if (gCurrentMapId == 0x63)
      {
          REG_DISPCNT |= 0x0400;
      }
      else
      {
          REG_DISPCNT |= 0x0800;
      }
      break;
  
  case 1:
      gViewportFlags[0] &= 0xFFFB;
      REG_DISPCNT &= 0xFDFF;
      break;
  ```

---

### 范例 2: DMA3 数据拷贝 (`sub_804FB24`)

- ❌ **违规写法 (绝对禁止)**:
  ```c
  case 1:
      REG_DISPCNT &= 0xFDFF;
      gBlendControl = 0xC10;
      gBlendCoefficients = 0xF0F;
      {
          vu32 *dmaRegs = (vu32 *)0x040000D4;
          dmaRegs[0] = (vu32)((const void *)0x0808B1B4);
          dmaRegs[1] = (vu32)((void *)0x05000080);
          dmaRegs[2] = (vu32)((((((0x8000 | 0x0000) | 0x0000) | 0x0000) | 0x0000) << 16) | (0x20 / 2));
          dmaRegs[2];
      }
      break;
  ```

- ✅ **标准写法 (100% 推荐，字节完全一致)**:
  ```c
  case 1:
      REG_DISPCNT &= 0xFDFF;
      gBlendControl = 0xC10;
      gBlendCoefficients = 0xF0F;
      DmaCopy16(3, (const void *)0x0808B1B4, (void *)0x05000080, 0x20);
      break;
  ```

---

### 范例 3: 动态地址数组 DMA 传输 (`sub_804FB24`)

- ❌ **违规写法 (绝对禁止)**:
  ```c
  {
      vu32 *dmaRegs = (vu32 *)0x040000D4;
      dmaRegs[0] = (vu32)((const void *)(0x08289B6E + (gViewportFlags[15] * 0x20)));
      dmaRegs[1] = (vu32)((void *)(0x05000002 + (gViewportFlags[15] * 0x20)));
      dmaRegs[2] = (vu32)((((((0x8000 | 0x0000) | 0x0000) | 0x0000) | 0x0000) << 16) | (0x1E / 2));
      dmaRegs[2];
  }
  ```

- ✅ **标准写法 (100% 推荐，字节完全一致)**:
  ```c
  DmaCopy16(3, (const void *)(0x08289B6E + (gViewportFlags[15] * 0x20)), 
               (void *)(0x05000002 + (gViewportFlags[15] * 0x20)), 0x1E);
  ```

---

## 6. Permuter 套件编写规范 (`permuter/<fn>/base.c`)

在运行 `permuter.py` 探索代码时，由于 permuter 采用 `-nostdinc` 且不引入全工程头文件，经常有开发者在 `base.c` 中手写裸地址 `*((vu16 *)0x04000000)`，导致合入 `src/` 时又被原样复制，造成坏代码扩散。

**规范操作**: 在 `permuter/<fn>/base.c` 头部，将需要的宏定义**按官方规范内联定义**：

```c
/* ======= Permuter base.c 头部标准内联模板 ======= */
typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;

#define REG_BASE               0x4000000
#define REG_DISPCNT            (*(vu16 *)(REG_BASE + 0x0))
#define REG_BLDCNT             (*(vu16 *)(REG_BASE + 0x50))
#define REG_BLDY               (*(vu16 *)(REG_BASE + 0x54))
#define REG_KEYINPUT           (*(vu16 *)(REG_BASE + 0x130))

#define REG_ADDR_DMA3          0x040000D4
#define DmaSet(dmaNum, src, dest, control) { \
    vu32 *dmaRegs = (vu32 *)REG_ADDR_DMA##dmaNum; \
    dmaRegs[0] = (vu32)(src); \
    dmaRegs[1] = (vu32)(dest); \
    dmaRegs[2] = (vu32)(control); \
    dmaRegs[2]; \
}
#define DmaCopy16(dmaNum, src, dest, size) \
    DmaSet(dmaNum, src, dest, (0x80000000) | ((size) / 2))
#define DmaCopy32(dmaNum, src, dest, size) \
    DmaSet(dmaNum, src, dest, (0x84000000) | ((size) / 4))
```

在 `base.c` 内部全程使用 `REG_DISPCNT` 和 `DmaCopy16` 调分。一旦达到分数 0，即可**原封不动直接合入 `src/*.c`**，实现代码风格的一步到位。

---

## 7. 代码合规自查与自动化审计

在提交任何匹配函数或进行全量验收前，请运行以下检查命令确保零违例：

```bash
# 1. 检查源码中是否残留任何 GBA 硬件裸指针强转 (应完全无结果或仅限底层宏定义)
grep -rnE '\(\s*(vu?16|u16|vu?32|u32)\s*\*\s*\)\s*0x04' src/

# 2. 检查源码中是否残留手写 dmaRegs 声明
grep -rn "dmaRegs = " src/

# 3. 运行函数字节定性验证
python3 scripts/fncheck.py <你的函数名>

# 4. 全量编译验证
timeout 900 make 2>&1 | tail -5 && sha1sum -c ll.sha1
```

---
*文档建立于: 2026-09-07*  
*维护者: Antigravity Decomp Team*
