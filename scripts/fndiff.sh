#!/usr/bin/env bash
# fndiff.sh -- 单函数匹配回环: 单独编出 sub_xxx 的 .o, 用 tools/asm-differ 与
#              asm/nonmatchings/sub_xxx.s 汇编出的参考 target.o 逐指令对比。
#
#   scripts/fndiff.sh <func> [候选.c]
#
#   scripts/fndiff.sh --promote <func> <胜出版本.c>   # 回写 permuter/<func>/base.c 并清掉中间变体
#
# 例:
#   scripts/fndiff.sh sub_8052C24                       # 用 permuter/sub_8052C24/base.c
#   scripts/fndiff.sh sub_8052C24 /tmp/try.c            # 用指定源文件
#
# 不需要整个 ROM 变绿, 也不碰别人的半成品 —— 多智能体并行时的首选验证手段。
# 依赖: permuter/<func>/compile.sh (没有则自动生成), asm/nonmatchings/<func>.s
#
# 注意: score 可能因"字面池未重定位"虚高 (EXPERIENCE 经验 29)。
#       要字节级定论请再用 scripts/fncheck.py <func>。

set -euo pipefail
cd "$(dirname "$0")/.."

if [ "${1:-}" = "--promote" ]; then
  fn="${2:?--promote <func> <winner.c>}"; win="${3:?缺少胜出版本文件}"
  [ -f "$win" ] || { echo "找不到 $win"; exit 2; }
  mkdir -p "permuter/$fn"
  [ "$win" = "permuter/$fn/base.c" ] || cp "$win" "permuter/$fn/base.c"
  find "permuter/$fn" -maxdepth 1 -name '*.c' ! -name base.c -delete
  echo "promoted: $win -> permuter/$fn/base.c (已清理中间变体)"
  exit 0
fi

fn="${1:?用法: fndiff.sh [--promote] <func> [候选.c]}"
src="${2:-permuter/$fn/base.c}"
dir="permuter/$fn"
out=".scratch/fndiff/$fn"

# 参考汇编: 未匹配用 nonmatchings/, 已匹配回退到 matchings/ (仍可用来对字节)
ref="asm/nonmatchings/$fn.s"
[ -f "$ref" ] || ref="asm/matchings/$fn.s"

[ -f "$src" ] || { echo "找不到候选源: $src"; exit 2; }
[ -f "$ref" ] || { echo "找不到参考汇编: asm/{non,}matchings/$fn.s"; exit 2; }
echo "   参考: $ref"

mkdir -p .scratch "$out"

# 1) 通用单函数编译脚本 (cpp -> preproc -> agbcc -> as), 与 Makefile 同一套 flag
if [ ! -x "$dir/compile.sh" ]; then
  mkdir -p "$dir"
  cat > "$dir/compile.sh" <<'EOS'
#!/bin/bash
cd "$(dirname "$0")/../.." || exit 1
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
arm-none-eabi-cpp -nostdinc -I tools/agbcc/include -iquote include "$1" -o "$T/f.i" || exit 1
tools/preproc/preproc "$T/f.i" | tools/agbcc/bin/agbcc -mthumb-interwork -Wimplicit -Wparentheses -O2 -fhex-asm -fprologue-bugfix -o "$T/f.s" || exit 1
printf ".text\n\t.align\t2, 0\n" >> "$T/f.s"
arm-none-eabi-as -mcpu=arm7tdmi -mthumb-interwork -I sound -o "$3" "$T/f.s" || exit 1
EOS
  chmod +x "$dir/compile.sh"
fi

# 2) 参考 target.o (从 gbadisasm 的 .s 汇编而来, 池里是硬码值)
if [ ! -f "$dir/target.o" ]; then
  { printf '\t.include "macros/function.inc"\n'; cat "$ref"; } > "$dir/target.s"
  arm-none-eabi-as -mcpu=arm7tdmi -mthumb-interwork -I asm -o "$dir/target.o" "$dir/target.s"
fi

# 3) 编候选
if ! "./$dir/compile.sh" "$src" x "$out/mine.o" 2> "$out/err.txt"; then
  echo "== 编译失败 =="; cat "$out/err.txt"; exit 1
fi

# 4) asm-differ 逐指令对比
#    候选对象里的符号名可能已经改掉 (改名后再跑 fndiff 验证是常态)。diff.py 按同一个
#    名字在两个 .o 里查找, 找不到就输出空 diff -> 分数 5000, 看起来像"完全不匹配",
#    其实是符号名对不上。这里把候选符号临时改回参考名再比。
sym="$fn"
if ! arm-none-eabi-nm "$out/mine.o" 2>/dev/null | grep -qE "[Tt] $fn$"; then
  alt=$(arm-none-eabi-nm "$out/mine.o" 2>/dev/null | awk '$2=="T"{print $3}' | head -1)
  if [ -n "$alt" ] && [ "$alt" != "$fn" ]; then
    if arm-none-eabi-objcopy --redefine-sym "$alt=$fn" "$out/mine.o" "$out/renamed.o" 2>/dev/null; then
      echo "   (候选对象里符号叫 $alt, 已临时改回 $fn 比对)"
      mv "$out/renamed.o" "$out/mine.o"
    fi
  fi
fi
echo "== $fn : $src =="
PYTHONPATH="${DECOMP_VENV_SITE:-$PWD/.venv/lib/python3.14/site-packages}" /usr/bin/python3 tools/asm-differ/diff.py -o -f "$out/mine.o" -F "$dir/target.o" "$sym" --width 120 || true
