import re, os, glob

def defs_in(path):
    out = set()
    txt = open(path, errors='ignore').read()
    for m in re.finditer(r'INCLUDE_ASM\s*\([^,]+,\s*([A-Za-z_]\w*)\s*\)', txt):
        out.add(m.group(1))
    # function bodies: name(...) {  at col 0, allow leading type tokens
    for m in re.finditer(r'(?m)^[A-Za-z_][\w \t\*]*\b([A-Za-z_]\w*)\s*\([^;]*\)\s*\{', txt):
        out.add(m.group(1))
    return out

# gather defs across repo C
alldefs = {}
for root, _, files in os.walk('src'):
    for f in files:
        if f.endswith('.c'):
            p = os.path.join(root, f)
            for d in defs_in(p):
                alldefs.setdefault(d, []).append(p)

rows = []
for line in open('functions.tsv', errors='ignore'):
    parts = line.rstrip('\n').split('\t')
    if len(parts) < 6 or parts[0] == 'status':
        continue
    rows.append(parts)

print("tsv rows:", len(rows))
miss = [(r[5], r[2]) for r in rows if r[5] not in alldefs]
print("tsv funcs with NO C definition:", len(miss))
for n, f in miss:
    print("  MISS", n, f)

tsvnames = set(r[5] for r in rows)
extra = {d: p for d, p in alldefs.items() if d not in tsvnames}
print("\nC defs not in tsv:", len(extra))
for d, p in sorted(extra.items())[:80]:
    print("  EXTRA", d, p[0])
