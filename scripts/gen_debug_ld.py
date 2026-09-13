#!/usr/bin/env python3
"""生成 linker_debug.ld — DEBUG 构建链接脚本。

原理: 调试构建 (-O0/-O1) 的 .text 比正式构建膨胀, 但本 ROM 的数据区地址被
① asm 字面池硬编码 (约 1386 处 0x08xxxxxx) 与 ② 数据 blob 内的函数指针 (379 处)
双重钉死, 数据区 [0x0805769C, 0x08000000+8M) 一个字节都不能漂。

因此 DEBUG 布局:
  - .text0  : crt0 + 全部代码文件 (剔除 GAP 名单), 原序铺在 [0xC0, 0x5769C);
              尾部 `. = ORIGIN(rom) + 0x5769C` forward 锚把 GAP 挪走留下的洞补齐,
              保证 data_805769C 起点仍为 0x0805769C;
  - .rodata : 与正式版完全一致 (含锚 A/B), 数据全部原位;
  - .text_gap : GAP 名单文件整体搬到 ORIGIN(rom)+0x800000 (16M region 的空区,
              距主代码 <8.5MB, thumb bl 超范围由 ld 自动生成 veneer);
  - IWRAM/EWRAM: 与正式版一致 (不依赖优化级别)。

数据 blob 内指向 GAP 文件函数的指针由 scripts/fix_debug_rom.py 链接后修补。
用法: python3 scripts/gen_debug_ld.py [--gap "event_hub event_actor ..."]
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DATA_TEXT_END = 0x5769C     # 数据区起点 (data_805769C), .text0 的硬上限
GAP_BASE = 0x900000         # .text_gap 相对 ORIGIN(rom) 的偏移 (0x08900000)

# .text 段输入文件原序 (与 linker.ld 一致, link 级别: C 文件 + asm + libgcc/libc)
TEXT_ORDER = [
    "asm/crt0.o",
    "src/engine_core.o", "src/scene_mgr.o", "src/sprite_engine.o",
    "src/vram_transfer.o", "src/map_view.o", "src/anim_slot.o",
    "src/player_stats.o", "src/menu_ui.o", "src/save.o", "src/save_menu.o",
    "src/text_engine.o", "src/sio_link.o", "src/battle_gfx_load.o",
    "src/battle_obj_core.o", "src/scene_obj_fx.o", "src/event_actor.o",
    "src/scene_interact.o", "src/event_hub.o", "src/cutscene_mgr.o",
    "src/obj_state.o", "src/battle_engine.o", "src/battle_anim.o",
    "src/battle_rewards.o", "src/obj_pool.o", "src/sio_battle.o",
    "src/script_vm.o", "src/sound.o",
    "asm/m4a_asm.o", "src/m4a.o", "asm/libagbsyscall.o", "src/agb_sram.o",
    "libgcc", "libc",
]

LIBGCC_OBJS = [
    "_call_via_rX.o", "_divsi3.o", "_dvmd_tls.o", "_fixunsdfsi.o",
    "_fixunssfsi.o", "_modsi3.o", "_udivsi3.o", "_umodsi3.o",
    "dp-bit.o", "fp-bit.o", "_lshrdi3.o", "_muldi3.o", "_negdi2.o",
]
LIBC_OBJS = ["memcpy.o"]


def build_gap_lines(gap: set[str]) -> list[str]:
    out = []
    for name in sorted(gap):
        out.append("        src/%s.o(.text);" % name)
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--gap", default="", help="空格分隔的 GAP 文件名单 (.text 段出现形式)")
    args = ap.parse_args()
    gap = set(args.gap.split())

    src = (ROOT / "linker.ld").read_text()

    # ① rom region 8M -> 16M
    src, n = re.subn(r"(rom \(rx\) : ORIGIN = 0x08000000, LENGTH = )8M",
                     r"\g<1>16M", src)
    assert n == 1, "rom region LENGTH 未找到"

    # ② .text 段: 剔除 GAP 文件 + 尾部补洞锚
    #    linker.ld 的 .text 段体格式为 "        <obj>(.text);" / "*libgcc.a:...(text);"
    def rewrite_text(m: re.Match) -> str:
        head, body, tail = m.group(1), m.group(2), m.group(3)
        kept = []
        for line in body.splitlines():
            s = line.strip()
            om = re.match(r"^(\S+)\(\.text\);$", s)
            libm = re.match(r"^\*(libgcc|libc)\.a:(\S+)\(\.text\);$", s)
            drop = False
            if om:
                # 归一成短名 (event_hub) 与 gap 名单比较
                short = re.sub(r"^(?:src|asm)/", "", om.group(1))
                short = re.sub(r"\.o$", "", short)
                drop = short in gap
            if not drop:
                kept.append(line)
        new_body = "\n".join(kept)
        # 不在段内补洞锚: ld 的跨段 veneer (stub) 会追加在输出段末尾, 若段尾被
        # 锚顶到 0x5769C, stub 就会越过数据起点。改由 .rodata 显式定位 0x5769C,
        # stub 落在 .text0 真实内容尾 (GAP 挪走留下的洞内)
        return head + new_body + tail

    src, n = re.subn(
        r"(\n    \.text : ALIGN\(4\) \{\n)(.*?)(\n    \}\n)",
        rewrite_text, src, flags=re.S)
    assert n == 1, ".text 段未找到"

    # ②b .rodata 显式定位数据区起点 (stub 落在 .text0 尾部洞内, 不越界)
    src, n = re.subn(r"    \.rodata ALIGN\(4\) : SUBALIGN\(4\) \{",
                     "    .rodata ORIGIN(rom) + 0x%X : SUBALIGN(4) {" % DATA_TEXT_END, src)
    assert n == 1, ".rodata 段头未找到"

    # ③ 追加 .text_gap 段 (放在 SECTIONS 的最后一个 `} >rom =0xFF` 之后)
    gap_body = "\n".join(build_gap_lines(gap))
    gap_sec = (
        "\n    /* DEBUG: GAP 文件整体搬到 8M 空区 (数据地址零漂移) */\n"
        "    .text_gap ORIGIN(rom) + 0x%X : ALIGN(4) SUBALIGN(4) {\n" % GAP_BASE
        + gap_body + "\n    } >rom =0xFF\n"
    )
    src, n = re.subn(r"(\n    \} >rom =0xFF\n)",
                     r"\g<1>" + gap_sec, src, count=1)
    assert n == 1, "rodata 收尾未找到"

    out = ROOT / "linker_debug.ld"
    out.write_text(src)
    print("GEN linker_debug.ld (gap=%d 文件: %s)" % (len(gap), " ".join(sorted(gap))))
    return 0


if __name__ == "__main__":
    sys.exit(main())
