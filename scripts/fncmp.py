#!/usr/bin/env python3
"""fncmp.py <fn> [cand.c] [--full]
未链接对象字节级比对 + 可读指令 diff。
依赖 permuter/<fn>/compile.sh 已用 .equ 固化 IWRAM 池常量 (否则池字为 0, 恒有假差异)。
"""
import re, subprocess, sys, os, difflib

ROOT = "/home/gpnux/decomp/ll"


def dump(o):
    p = subprocess.run(["arm-none-eabi-objdump", "-d", "--no-show-raw-insn", "-M",
                        "reg-names-std", o], capture_output=True, text=True, cwd=ROOT).stdout
    r = []
    for l in p.splitlines():
        m = re.match(r'\s*([0-9a-f]+):\t(.*)', l)
        if m:
            ins = m.group(2).strip()
            ins = re.sub(r'\s*<[^>]*>', '', ins)
            ins = re.sub(r'\s*@\s+\(.*', '', ins).strip()
            if ins.startswith('.word') or ins.startswith('.short'):
                continue
            r.append((int(m.group(1), 16), ins))
    return r


def raw(o, out):
    subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", "--only-section=.text", o, out],
                   check=True, cwd=ROOT)
    return open(out, "rb").read()


fn = sys.argv[1]
cand = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[2].startswith("--") else \
    f"{ROOT}/permuter/{fn}/base.c"
full = "--full" in sys.argv
out = f"{ROOT}/.scratch/fncmp/{fn}"
os.makedirs(out, exist_ok=True)

r = subprocess.run([f"{ROOT}/permuter/{fn}/compile.sh", cand, "x", f"{out}/mine.o"],
                   capture_output=True, text=True, cwd=ROOT)
if r.returncode:
    print("COMPILE-FAIL"); print(r.stdout[-2000:]); print(r.stderr[-2000:]); sys.exit(1)

a = raw(f"{out}/mine.o", f"{out}/mine.bin")
b = raw(f"{ROOT}/permuter/{fn}/target.o", f"{out}/tgt.bin")
diff = [i for i in range(min(len(a), len(b))) if a[i] != b[i]]
print(f"# size mine={len(a):#x} tgt={len(b):#x}   diffbytes={len(diff) + abs(len(a)-len(b))}")
if not diff and len(a) == len(b):
    print("*** BYTE-EXACT ***")
    sys.exit(0)
print("# first diff offsets:", " ".join("+0x%x" % d for d in diff[:24]))

ia, ib = dump(f"{out}/mine.o"), dump(f"{ROOT}/permuter/{fn}/target.o")
sa, sb = [x[1] for x in ia], [x[1] for x in ib]
sm = difflib.SequenceMatcher(None, sb, sa, autojunk=False)
n = 0
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == "equal" and not full:
        continue
    if tag == "equal":
        for k in range(i1, i2):
            print("    " + sb[k])
    else:
        for k in range(i1, i2):
            print("T   " + sb[k])
        for k in range(j1, j2):
            print("M   " + sa[k])
    n += 1
    if not full and n > 40:
        print("... (截断, 用 --full 看全部)")
        break
print(f"# ratio={sm.ratio():.4f}")
