#!/usr/bin/env python3
"""从 functions.tsv + ll.cfg + code.s 生成 asm/{matchings,nonmatchings}/<name>.s。

取代 split_asm.py。与它的三个本质区别:
  1. 主键 = addr。当前名以 ll.cfg 为准; TSV 的 name 列只是缓存,
     不一致 = 检测到改名 -> 报告, --sync 回写缓存列 (改名不再产生孤儿/失联)。
  2. 增量写: 内容不变不 touch 文件 (split_asm 全删重建 -> 每次全量重编)。
  3. 只依赖 TSV/ll.cfg/code.s, 不读 functions.yaml。

用法: gen_asm.py [--dry-run] [--sync] [--verbose]
退出码: 0 成功; 1 有错误 (缺块/重复 addr 等)。
"""
import argparse
import csv
import re
import sys
from collections import Counter

TSV = "functions.tsv"
LLCFG = "ll.cfg"
CODES = "code.s"
ASM = "asm"
HEADER = "\t.syntax unified\n"
FOOTER = "\t.syntax divided\n"
FUNC_START_RE = re.compile(r"^[ \t]*((?:thumb|arm)_func_start) (\w+)$", re.M)


def load_llcfg(path):
    addr2name = {}
    for line in open(path, encoding="utf-8", errors="replace"):
        m = re.match(r'^(?:thumb|arm)_func (0x[0-9a-fA-F]+) (\w+)', line)
        if m:
            addr2name[int(m.group(1), 16)] = m.group(2)
    return addr2name


def load_tsv(path):
    rows = []
    for r in csv.DictReader(open(path, encoding="utf-8"), delimiter="\t"):
        rows.append((int(r["status"]), r["isa"], r["module"], int(r["addr"], 16), r["name"]))
    return rows


def split_code_s(path):
    """name -> 函数块文本 (从 *_func_start 行起到下一个 *_func_start 前)。
    同时返回 addr -> (块文本, code.s 旧标签) 供 ll.cfg 改名后按地址回退。"""
    content = open(path, encoding="utf-8", errors="replace").read()
    starts = [(m.start(1), m.group(2)) for m in FUNC_START_RE.finditer(content)]
    blocks = {}
    addr_blocks = {}
    for i, (pos, name) in enumerate(starts):
        if i + 1 < len(starts):
            end = starts[i + 1][0]
        else:
            end = len(content) - (1 if content.endswith("\n") else 0)
        blocks[name] = content[pos:end]
        # 地址注释在 func_start 的下一行: <label>: @ 0x08012790
        head = content[pos:pos + 400].splitlines()
        for line in head[1:3]:
            ma = re.search(r"@\s*(0x[0-9a-fA-F]+)", line)
            if ma:
                addr_blocks[int(ma.group(1), 16)] = (content[pos:end], name)
                break
    return blocks, addr_blocks


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--sync", action="store_true", help="把 ll.cfg 当前名回写进 TSV name 列")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    addr2name = load_llcfg(LLCFG)
    rows = load_tsv(TSV)
    blocks, addr_blocks = split_code_s(CODES)

    # code.s 旧标签 -> ll.cfg 当前名 (地址是主键; 引用统一改写, code.s 本体不动)
    old2new = {}
    for a, (_, label) in addr_blocks.items():
        if a in addr2name and label != addr2name[a]:
            old2new[label] = addr2name[a]
    ref_re = (
        re.compile(r"\b(" + "|".join(sorted(map(re.escape, old2new), key=len, reverse=True)) + r")\b")
        if old2new
        else None
    )

    def canon(text):
        return ref_re.sub(lambda m: old2new[m.group(1)], text) if ref_re else text

    errors = []
    drift = []
    renamed = []
    expected = {}
    for status, isa, module, addr, tsv_name in rows:
        name = addr2name.get(addr)
        if name is None:
            errors.append(f"addr 0x{addr:08x} 不在 ll.cfg (tsv 行: {tsv_name})")
            continue
        if name != tsv_name:
            drift.append((addr, tsv_name, name))
        folder = "matchings" if status == 1 else "nonmatchings"
        if name in expected:
            errors.append(f"重复 addr 0x{addr:08x}: {name}")
            continue
        blk = blocks.get(name)
        if blk is None and addr in addr_blocks:
            # code.s 标签未跟上 ll.cfg 语义名: 按地址取块并改写标签 (地址是主键)
            blk, old_label = addr_blocks[addr]
            renamed.append((addr, old_label, name))
        if blk is None:
            errors.append(f"code.s 无函数块: {name} (0x{addr:08x})")
            continue
        expected[name] = (folder, HEADER + canon(blk) + FOOTER)

    dup_addr = [hex(a) for a, c in Counter(r[3] for r in rows).items() if c > 1]
    errors += [f"TSV 重复 addr {a}" for a in dup_addr]

    # 安全阀: 任何错误 (尤其 code.s/TSV 解析异常导致 expected 大面积缺失) 时不落盘,
    # 防止把整个 asm/ 目录清空。
    if errors:
        print("错误 (未改动 asm/):")
        for e in errors:
            print("  " + e)
        sys.exit(1)

    written = unchanged = deleted = 0
    if not args.dry_run:
        keep = {f"{folder}/{name}.s" for name, (folder, _) in expected.items()}
        for folder in ("matchings", "nonmatchings"):
            import os
            os.makedirs(f"{ASM}/{folder}", exist_ok=True)
            for fn in os.listdir(f"{ASM}/{folder}"):
                if f"{folder}/{fn}" not in keep:
                    os.remove(f"{ASM}/{folder}/{fn}")
                    deleted += 1
                    if args.verbose:
                        print(f"del  {folder}/{fn}")
        for name, (folder, text) in expected.items():
            path = f"{ASM}/{folder}/{name}.s"
            try:
                if open(path, encoding="utf-8").read() == text:
                    unchanged += 1
                    continue
            except OSError:
                pass
            with open(path, "w", encoding="utf-8") as f:
                f.write(text)
            written += 1
            if args.verbose:
                print(f"wri {folder}/{name}.s")

    if args.sync and drift and not args.dry_run:
        fix = {a: n for a, _, n in drift}
        lines = open(TSV, encoding="utf-8").read().splitlines(True)
        out = [lines[0]]
        for line in lines[1:]:
            p = line.split("\t")
            if len(p) >= 6 and int(p[3], 16) in fix:
                p[5] = fix[int(p[3], 16)]
                line = "\t".join(p)
            out.append(line)
        open(TSV, "w", encoding="utf-8").writelines(out)
        print(f"已同步 {len(drift)} 个改名进 {TSV}")

    mode = "dry-run: " if args.dry_run else ""
    print(f"{mode}目标 {len(expected)} 个 .s | 写入 {written} | 未变 {unchanged} | 删除 {deleted}")
    for addr, old, new in renamed:
        print(f"renamed: 0x{addr:08x} code.s='{old}' -> '{new}'")
    for addr, old, new in drift:
        print(f"drift: 0x{addr:08x} tsv='{old}' ll.cfg='{new}'" + ("  [--sync 回写]" if not args.dry_run and not args.sync else ""))
    if errors:
        print("错误:")
        for e in errors:
            print("  " + e)
        sys.exit(1)


if __name__ == "__main__":
    main()
