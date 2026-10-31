import os, re, sys, pathlib, collections, atexit

class _Tee:
    """출력을 여러 스트림(화면 + 파일)에 동시에 쓴다."""
    def __init__(self, *streams):
        self.streams = streams
    def write(self, s):
        for st in self.streams:
            st.write(s)
    def flush(self):
        for st in self.streams:
            st.flush()

_out_path = pathlib.Path(__file__).resolve().parent / "ModuleGraph.txt"
_out_file = open(_out_path, "w", encoding="utf-8", newline="\n")

def _close_tee():
    sys.stdout.flush()
    sys.stdout = sys.__stdout__   # 파이썬이 마지막에 flush할 대상을 원래 stdout으로 복구
    _out_file.close()

atexit.register(_close_tee)
sys.stdout = _Tee(sys.stdout, _out_file)

roots = [pathlib.Path(p) for p in sys.argv[1].split(os.pathsep)]   # Windows는 ';' 구분
files = [f for r in roots for f in r.rglob("*")]
trace = sys.argv[2:]   # 출처를 추적할 import 대상 (모듈 이름이 정확히 같은 것만)

exts = {".ixx", ".cppm", ".cpp"}
decl = re.compile(r'^\s*(export\s+)?module\s+([\w.]+)(?::(\w+))?\s*;')
imp  = re.compile(r'^\s*(export\s+)?import\s+(:?[\w.:]+)\s*;')

iface = collections.defaultdict(dict)   # 모듈 -> {대상: 재export 여부}
impl  = collections.defaultdict(set)    # 구현 유닛/구현 파티션의 import
sources = []

for f in files:
    if f.suffix not in exts:
        continue
    cur, kind = None, None
    text = f.read_text(encoding="utf-8", errors="ignore")
    for lineno, line in enumerate(text.splitlines(), 1):
        if line.lstrip().startswith("//"):
            continue
        m = decl.match(line)
        if m:
            is_export, cur, part = m.group(1), m.group(2), m.group(3)
            if is_export:
                kind = "iface"          # export module X; / export module X:P;
            elif part:
                kind = "part"           # module X:P;  (구현 파티션)
            else:
                kind = "impl"           # module X;    (구현 유닛)
            iface[cur]                  # 노드 등록
            continue
        m = imp.match(line)
        if m and cur:
            is_reexport, target = bool(m.group(1)), m.group(2)
            if target.startswith(":"):  # 같은 모듈의 파티션
                continue
            target = target.split(":")[0]
            if target == cur or target == "std":
                continue
            if kind in ("impl", "part"):    # 구현 유닛/구현 파티션은 전파되지 않음
                impl[cur].add(target)
            else:
                iface[cur][target] = iface[cur].get(target, False) or is_reexport
            if target in trace:
                sources.append((f, lineno, cur, kind, line.strip()))

# ---------------------------------------------------------------
# 재export 도달 집합: mod를 import하면 (export import 체인을 따라) 함께 보이는 모듈들
# ---------------------------------------------------------------
_reach_cache = {}
def reach(mod):
    if mod in _reach_cache:
        return _reach_cache[mod]
    out, stack = set(), [mod]
    while stack:
        n = stack.pop()
        for t, is_re in iface.get(n, {}).items():
            if is_re and t not in out:
                out.add(t)
                stack.append(t)
    out.discard(mod)
    _reach_cache[mod] = out
    return out

def iface_cover(mod, target, is_re):
    """인터페이스 간선 mod -> target이 다른 import의 재export로 이미 덮이면 그 모듈 이름을 반환."""
    # 재export 간선은 다른 '재export' 간선으로만 덮인 것으로 본다 (소비자 노출 의도 보존)
    for u, u_re in sorted(iface[mod].items()):
        if u == target or (is_re and not u_re):
            continue
        if target in reach(u):
            return u
    return None

def impl_cover(mod, target):
    """구현부 import가 인터페이스/다른 구현부 import의 재export로 이미 보이면 그 모듈을 반환."""
    for u in sorted((set(iface[mod]) | impl[mod]) - {target}):
        if target in reach(u):
            return u
    return None

# 중복 판정
redundant_iface = {}   # (mod, target) -> via
for mod, targets in iface.items():
    for t, is_re in targets.items():
        via = iface_cover(mod, t, is_re)
        if via:
            redundant_iface[(mod, t)] = via

redundant_impl = {}
for mod, targets in impl.items():
    for t in targets:
        if t in iface[mod]:                 # 인터페이스에 이미 있는 건 기존처럼 생략
            continue
        via = impl_cover(mod, t)
        if via:
            redundant_impl[(mod, t)] = via

def fmt(mod):
    items = []
    for t, re_ in iface[mod].items():
        if (mod, t) in redundant_iface:
            continue
        items.append(f"{t}(re)" if re_ else t)
    return ", ".join(sorted(items))

total_if = sum(len(v) for v in iface.values())
total_im = sum(len(v - set(iface[m])) for m, v in impl.items())

print("== interface graph (reduced: 재export로 이미 보이는 간선 제거) ==")
for mod in sorted(iface):
    print(f"{mod} -> {fmt(mod) or '-'}")

print("\n== impl-only imports (reduced; 구현부 전용, 소비자에게 전파되지 않음) ==")
for mod in sorted(impl):
    only = sorted(t for t in impl[mod] - set(iface[mod]) if (mod, t) not in redundant_impl)
    if only:
        print(f"{mod} => {', '.join(only)}")

print(f"\n== removed as redundant: interface {len(redundant_iface)}/{total_if}, impl-only {len(redundant_impl)}/{total_im} ==")
if redundant_iface:
    print("[interface]")
    for (mod, t), via in sorted(redundant_iface.items()):
        print(f"  {mod} -> {t}   (covered by {via})")
if redundant_impl:
    print("[impl-only]")
    for (mod, t), via in sorted(redundant_impl.items()):
        print(f"  {mod} => {t}   (covered by {via})")

# 순환 검출: 인터페이스 그래프(원본 전체)로 한다. 중복 제거는 도달 가능성을 바꾸지 않는다.
seen, stack, cycles = set(), [], []
def dfs(n):
    if n in stack:
        cycles.append(stack[stack.index(n):] + [n])
        return
    if n in seen:
        return
    seen.add(n)
    stack.append(n)
    for m in sorted(iface.get(n, {})):
        dfs(m)
    stack.pop()

for n in sorted(iface):
    dfs(n)

print("\n== cycles (interface graph) ==")
for c in cycles:
    print(" -> ".join(c))
if not cycles:
    print("No cycles found.")

if trace:
    print(f"\n== sources ({', '.join(trace)}) ==")
    for f, lineno, cur, kind, line in sources:
        print(f"{f}:{lineno}: [{cur} / {kind}] {line}")
    if not sources:
        print("No matching imports found.")