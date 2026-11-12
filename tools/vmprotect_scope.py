"""VMProtect scope of a wrapped Player build, measured from the PE files (no IDA needed, nothing is patched).

What it establishes, per Player:
  * every `jmp rel32` in .text that lands in .vmp0/.vmp1 on a real instruction boundary (linear disassembly
    from the function start), the function it sits in, and the extent of the hole behind it: the bytes from
    the replaced `call ds:VMProtectBegin*` up to the point where compiler output resumes. Holes are refilled
    by VMProtect with its own code fragments, so nothing in them can be matched.
  * marker calls VMProtect removed without protecting anything (`lea esp,[esp+4]; nop; nop`, 6-byte NOP runs,
    `ret` + 5 NOPs).
  * the twin of each protected function in the unprotected STUDIO build, where the SDK markers are still
    ordinary imports of VMProtectSDK32.dll (paired on the normalised instructions before the Begin call and
    after the End call), plus RTTI, callers and strings on both sides.
  * where the import table .text really uses lives (the linker's original one in .rdata, not the directory's).

Usage (from the repo root):
  python3 -P tools/vmprotect_scope.py                # both Players, writes notes/vmprotect-player*.csv
  python3 -P tools/vmprotect_scope.py --selftest     # asserts on the facts this analysis rests on
  python3 -P tools/vmprotect_scope.py --iat PLAYER   # slot,dll,name CSV of the IAT that .text really uses
Inputs: bin/player_1023, bin/player, bin/studio executables and ida/exports/<TARGET>-functions.csv.
"""
import bisect, csv, re, struct, sys
import capstone, pefile

TARGETS = {
    "PLAYER": ("bin/player_1023/RobloxPlayerBeta.exe", "ida/exports/PLAYER-functions.csv", "notes/vmprotect-player.csv"),
    "PLAYER_1015": ("bin/player/RobloxPlayerBeta.exe", "ida/exports/PLAYER_1015-functions.csv", "notes/vmprotect-player_1015.csv"),
}
STUDIO = ("bin/studio/RobloxStudioBeta.exe", "ida/exports/STUDIO-functions.csv")
JUNK = {"pushfd", "popfd", "pushal", "popal"}  # never emitted by the compiler; in every VMProtect fragment chain
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True

# Names for the protected functions, keyed by name_key() (STUDIO twin, else RTTI slot, else first string).
# Each is a hypothesis carrying its source in braces (CLAUDE.md name sources); "unnamed" entries describe
# what the unprotected STUDIO twin does. The generated evidence column carries what the binaries show.
NAMES = {
    "str:Exception occured in Document::executeScript: %s": "Document::executeScript {str; src2016 WindowsClient/Document.cpp}",
    "rtti:RBX::RenderJob:6": "RBX::RenderJob::stepDataModelJob {src2016 marker placement in WindowsClient/RenderJob.cpp; slot not proven}",
    "studio:00605220": "unnamed: GetThreadContext/Dr7 hardware-breakpoint check, returns bool",
    "studio:00607680": "isDebuggedDirectThreadFunc {src2016 Network/API.cpp: the thread loop that calls VMProtectIsDebuggerPresent}",
    "studio:006079c0": "RBX::Network::initWithoutSecurity {src2016 marker placement in Network/API.cpp; ios name exists}",
    "studio:0060c2d0": "unnamed static in Network/Client.cpp: Dr7 check + VMProtectIsValidImageCRC, called from Client::OnReceive",
    "studio:0060c820": "unnamed static in Network/Client.cpp, called from Client::OnReceive",
    "studio:0060c9a0": "unnamed static in Network/Client.cpp, called from Client::OnReceive",
    "studio:0060cbb0": "RBX::Network::Client::OnReceive(RakNet::Packet*) {ios; RakNet PluginInterface2 slot 4}",
    "studio:0063e0b0": "RBX::Network::Replicator::SendDataJob::stepDataModelJob {ios; src2016 marker placement}",
    "studio:00642040": "RBX::Network::Replicator::PingJob::stepDataModelJob {ios; src2016 marker placement}",
    "studio:0064ffb0": "RBX::Network::ClientReplicator::onServiceProvider {ios; src2016 marker placement; ret 8}",
    "studio:00650e00": "RBX::Network::ClientReplicator::MemoryCheckerJob::stepDataModelJob {src2016 marker placement}",
    "studio:00650ff0": "RBX::Network::ClientReplicator::MemoryCheckerCheckerJob::stepDataModelJob {src2016 marker placement}",
    "studio:00681ed0": "RBX::Network::Replicator::PingBackItem::write(RakNet::BitStream&) {ios}",
    "studio:00684ad0": "RBX::Network::Replicator::PingItem::write(RakNet::BitStream&) {ios}",
    "studio:00759480": "RBX::DataModel::allHackFlagsOredTogether() {ios; src2016 call pattern in PingItem/PingBackItem::write}",
    "studio:0075d440": "unnamed DataModel core-script loader {str; ios candidates RBX::DataModel::loadCoreScripts()/startCoreScripts(Adorn*, bool)}",
    "studio:007c69a0": "runScript(shared_ptr<DataModel>, std::string), static in App/v8datamodel/Stats.cpp {src2016; str}",
    "studio:007ee750": "RBX::ScriptContext::executeInNewThread(Identities, ProtectedString, const char*, function1, function2, Continuations, lua_State*) {ios; str asserts}",
    "studio:007efdc0": "RBX::ScriptContext::executeInNewThread(Identities, const ProtectedString&, const char*) {ios; callers}",
    "studio:007eff30": "unnamed ScriptContext.cpp wrapper of the 7-argument executeInNewThread, called from LuaLibrary.cpp",
    "studio:008ec870": "unnamed ProgramMemoryChecker helper: finds the .text section of the main module",
    "studio:008ec900": "unnamed ProgramMemoryCheckerImpl helper called first by step()",
    "studio:008ec950": "unnamed ProgramMemoryChecker helper",
    "studio:008ec9d0": "ProgramMemoryCheckerImpl::step() {mac}",
    "studio:008eca40": "ProgramMemoryCheckerImpl constructor {rtti: stores the ProgramMemoryCheckerImpl vftable}",
}


class Img:
    def __init__(self, path, funcs_csv):
        self.pe = pefile.PE(path, fast_load=True)
        self.base = self.pe.OPTIONAL_HEADER.ImageBase
        self.img = self.pe.get_memory_mapped_image()
        self.sec = [(s.Name.rstrip(b"\0").decode(), self.base + s.VirtualAddress, self.base + s.VirtualAddress + s.Misc_VirtualSize)
                    for s in self.pe.sections]
        self.text = next((lo, hi) for n, lo, hi in self.sec if n == ".text")
        self.rdata = next((lo, hi) for n, lo, hi in self.sec if n == ".rdata")
        self.data = next((lo, hi) for n, lo, hi in self.sec if n == ".data")
        self.vmp = [(lo, hi) for n, lo, hi in self.sec if n.startswith(".vmp")]
        with open(funcs_csv) as f:
            self.starts = sorted(int(r["address"], 16) for r in csv.DictReader(f) if self.text[0] <= int(r["address"], 16) < self.text[1])
        self.startset = set(self.starts)

    def b(self, a, n):
        return self.img[a - self.base:a - self.base + n]

    def u32(self, a):
        return struct.unpack_from("<I", self.img, a - self.base)[0]

    def in_vmp(self, a):
        return any(lo <= a < hi for lo, hi in self.vmp)

    def in_text(self, a):
        return self.text[0] <= a < self.text[1]

    def dis(self, a, end):
        return MD.disasm(self.b(a, end - a), a)

    def lands(self, s, ea):
        for i in self.dis(s, ea + 16):
            if i.address >= ea:
                return i.address == ea
        return False

    def fstart(self, ea):
        """Start of the function holding ea: IDA's nearest start, moved up past any int3 padding in between."""
        s = self.starts[bisect.bisect_right(self.starts, ea) - 1]
        for p in range(ea & ~15, s, -16):
            if self.b(p - 1, 1) == b"\xcc" and self.lands(p, ea):
                return p
        return s

    def fend(self, a, limit=0x4000):
        """End of the compiler-generated run starting at a: first int3 or next aligned known function start."""
        for i in self.dis(a, a + limit):
            if i.mnemonic == "int3" or (i.address != a and i.address % 16 == 0 and i.address in self.startset):
                return i.address
        return None

    def rel_branches(self, lo, hi, opcodes=(0xE8, 0xE9)):
        """Byte-scan [lo,hi) for rel32 branches: yields (address, opcode, target). Not instruction-aligned."""
        d = self.b(lo, hi - lo)
        for m in re.finditer(b"[" + bytes(opcodes).replace(b"\\", b"\\\\") + b"]", d, re.S):
            i = m.start()
            if i + 5 <= len(d):
                yield lo + i, d[i], (lo + i + 5 + struct.unpack_from("<i", d, i + 1)[0]) & 0xFFFFFFFF

    def cstr(self, a):
        if not (self.rdata[0] <= a < self.data[1]):
            return None
        s = self.b(a, 160).split(b"\0")[0]
        if len(s) >= 5 and all(32 <= c < 127 for c in s):
            return s.decode()
        return None

    def strings(self, ranges):
        out = []
        for lo, hi in ranges:
            for i in self.dis(lo, hi):
                for op in i.operands:
                    v = op.imm if op.type == capstone.x86.X86_OP_IMM else op.mem.disp if op.type == capstone.x86.X86_OP_MEM else None
                    s = self.cstr(v & 0xFFFFFFFF) if v is not None else None
                    if s and s not in out:
                        out.append(s)
        return out

    def rtti_slots(self, f):
        """vftable slots in .rdata holding f: [(class, slot index, vftable address)]."""
        out, lo, hi = [], self.rdata[0], self.rdata[1]
        d, pat, i = self.b(lo, hi - lo), struct.pack("<I", f), -1
        while True:
            i = d.find(pat, i + 1)
            if i < 0:
                return out
            if i % 4:
                continue
            for k in range(600):
                v = lo + i - 4 * k
                col = self.u32(v - 4)
                if lo <= col < hi - 16 and self.u32(col) == 0:
                    td = self.u32(col + 12)
                    if self.data[0] <= td < self.data[1] and self.b(td + 8, 3) == b".?A":
                        out.append((demangle_class(self.b(td + 8, 200).split(b"\0")[0].decode()), k, v))
                        break

    def callers(self, wanted):
        """{callee: [caller function starts]} for direct E8 calls from .text (byte scan, exact-target hits)."""
        out = {w: [] for w in wanted}
        for a, op, t in self.rel_branches(*self.text, opcodes=(0xE8,)):
            if t in out:
                out[t].append(self.fstart(a))
        return out

    def norm(self, lo, hi):
        """Instruction list with image addresses masked, for comparing code across builds."""
        out = []
        for i in self.dis(lo, hi):
            s = re.sub(r"0x[0-9a-f]{6,8}", "ADDR", i.op_str)
            if i.mnemonic in ("call", "jmp") or i.mnemonic.startswith("j") and len(i.bytes) >= 5:
                s = re.sub(r"^0x[0-9a-f]+$|ADDR", "TGT", s)
            out.append(i.mnemonic + " " + s)
        return out


def demangle_class(n):
    """.?AVPingJob@Replicator@Network@RBX@@ -> RBX::Network::Replicator::PingJob (plain nested names only)."""
    m = re.match(r"\.\?A[VU]([\w@]+)@@$", n)
    return "::".join(reversed(m.group(1).split("@"))) if m else n


def orig_imports(im):
    """The linker's own import descriptors in .rdata (still present after wrapping): {IAT slot: (dll, name)}."""
    lo, hi = im.rdata
    slots, descs = {}, []
    for off in range(lo, hi - 20, 4):
        oft, ts, fwd, name, ft = struct.unpack_from("<5I", im.img, off - im.base)
        r = lambda v: lo <= v + im.base < hi
        if ts == 0 and r(oft) and r(name) and r(ft) and ft + im.base < lo + 0x8000:
            n = im.b(name + im.base, 40).split(b"\0")[0]
            if n.lower().endswith(b".dll") and n.isascii():
                descs.append(off)
                k = 0
                while im.u32(im.base + oft + 4 * k):
                    v = im.u32(im.base + oft + 4 * k)
                    nm = "#%d" % (v & 0xFFFF) if v >> 31 else im.b(im.base + v + 2, 200).split(b"\0")[0].decode("latin1")
                    slots[im.base + ft + 4 * k] = (n.decode(), nm)
                    k += 1
    return slots, descs


def player_scope(im):
    """Protected regions and stripped markers of one wrapped Player."""
    back = sorted({t for lo, hi in im.vmp for a, op, t in im.rel_branches(lo, hi, (0xE9,)) if im.in_text(t)})
    raw = [(a, op, t) for a, op, t in im.rel_branches(*im.text) if im.in_vmp(t)]
    # region entries: a jmp into .vmp* reached by clean compiler code from an aligned function start.
    # (Fragments VMProtect parks inside holes also jump into .vmp*, but the path to them is not clean.)
    regions = []
    for a, op, t in sorted(raw):
        fs = im.fstart(a)
        if op == 0xE9 and fs % 16 == 0 and im.lands(fs, a) and clean_code(im, fs, a, []) is not None:
            regions.append({"site": a, "target": t, "fstart": fs})
    for _ in range(3):  # hole ends; repeated so that fragments jumping into other holes are recognised
        holes = [(r["site"], r["end"]) for r in regions if "end" in r]
        for n, r in enumerate(regions):
            bound = regions[n + 1]["fstart"] if n + 1 < len(regions) else r["site"] + 0x4000
            others = [h for h in holes if h[0] != r["site"]]
            r["end"] = next((T for T in back if r["site"] < T < bound and T not in im.startset and clean_tail(im, T, others, back) is not None), None)
            r["to_function_end"] = r["end"] is None
            if r["end"] is None:  # protected to the end of the function (End marker is a tail `jmp ds:VMProtectEnd`)
                p = (r["site"] + 16) & ~15
                while p < bound and not (p in im.startset and im.b(p - 1, 1) == b"\xcc"):
                    p += 16
                while im.b(p - 1, 1) == b"\xcc":
                    p -= 1
                r["end"] = r["fend"] = p
            else:
                r["fend"] = clean_tail(im, r["end"], others, back)
            r["marker"] = last_push(im, r["fstart"], r["site"])
    # marker calls removed without protection
    text = im.b(im.text[0], im.text[1] - im.text[0])
    patches = []
    for pat, kind in ((b"\x8d\x64\x24\x04\x90\x90", "begin-stripped"), (b"\xc3\x90\x90\x90\x90\x90", "end-jmp-to-ret"), (b"\x90" * 6, "end-call-nopped")):
        for m in re.finditer(re.escape(pat), text):
            a = im.text[0] + m.start()
            if kind == "end-call-nopped" and (text[m.start() - 1] in (0x90, 0xC3) or text[m.start() + 6] == 0x90):
                continue  # longer NOP run, or the ret+5 form already counted
            fs = next((r["fstart"] for r in regions if r["fstart"] <= a < r["fend"]), None) or im.fstart(a)
            if im.lands(fs, a) and not any(r["site"] <= a < r["end"] for r in regions):
                patches.append({"site": a, "kind": kind, "fstart": fs, "marker": last_push(im, fs, a) if kind == "begin-stripped" else None})
    return regions, sorted(patches, key=lambda p: p["site"]), raw, back


def last_push(im, lo, hi):
    """Operand of the last `push imm32` in [lo,hi): the marker-name argument of the Begin call at hi."""
    v = None
    for i in im.dis(lo, hi):
        if i.mnemonic == "push" and len(i.bytes) == 5:
            v = i.operands[0].imm & 0xFFFFFFFF
    return v


def clean_code(im, lo, hi, holes):
    """Instruction addresses of [lo,hi) if it disassembles to code free of VMProtect fragments, else None."""
    heads = set()
    for i in im.dis(lo, hi):
        heads.add(i.address)
        if i.mnemonic in JUNK or i.mnemonic == "int3":
            return None
        if len(i.bytes) >= 5 and i.operands and i.operands[0].type == capstone.x86.X86_OP_IMM and (i.mnemonic[0] == "j" or i.mnemonic == "call"):
            t = i.operands[0].imm & 0xFFFFFFFF
            if im.in_vmp(t) or any(s + 5 <= t < e for s, e in holes):
                return None
    return heads


def clean_tail(im, T, holes, back):
    """End address of the compiler code starting at T (first int3 or next aligned function), or None if
    VMProtect fragments follow T. Every later address VMProtect jumps back to must be an instruction of it:
    a fragment ending in `ret` plus filler bytes otherwise passes for code (PLAYER 0073eb07)."""
    end = im.fend(T)
    heads = clean_code(im, T, end, holes) if end is not None else None
    if heads is None or any(b not in heads for b in back[bisect.bisect_right(back, T):bisect.bisect_left(back, end)]):
        return None
    return end


def studio_markers(im):
    """Functions of the unprotected build that call VMProtectSDK32.dll: begin/end offsets and extents."""
    im.pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]])
    slots = {i.address: i.name.decode() for e in im.pe.DIRECTORY_ENTRY_IMPORT if e.dll.lower().startswith(b"vmprotectsdk") for i in e.imports}
    text = im.b(im.text[0], im.text[1] - im.text[0])
    funcs = {}
    for slot, name in slots.items():
        for m in re.finditer(b"\xff[\x15\x25]" + re.escape(struct.pack("<I", slot)), text):
            a = im.text[0] + m.start()
            fs = im.fstart(a)
            if im.lands(fs, a):
                funcs.setdefault(fs, []).append((a, name, "call" if text[m.start() + 1] == 0x15 else "jmp"))
    out = []
    for fs, calls in sorted(funcs.items()):
        calls.sort()
        begins = [c for c in calls if "Begin" in c[1]]
        ends = [c for c in calls if c[1] == "VMProtectEnd"]
        last = max(c[0] for c in calls) + 6
        fe = last if calls[-1][2] == "jmp" else im.fend(last)
        b0 = begins[0][0]
        e0 = min(e[0] for e in ends if e[0] > b0)
        out.append({"fstart": fs, "fend": fe, "calls": calls, "begin": b0, "end": e0 + 6, "rtti": im.rtti_slots(fs),
                    "marker": im.b(last_push(im, fs, b0), 8).split(b"\0")[0].decode("latin1")})
    return out, slots


def profile(im, fs, ranges):
    return {"rtti": im.rtti_slots(fs), "strings": im.strings(ranges)}


def describe(im, r, callers, st, twin):
    """identity, evidence strings for one protected region."""
    fs = r["fstart"]
    clean = [(fs, r["site"])] + ([(r["end"], r["fend"])] if r["fend"] and r["fend"] > r["end"] else [])
    p = profile(im, fs, clean)
    ev = ["func %08x+0x%x, body %08x-%08x, hole ends %s" % (fs, r["site"] - fs, fs, r["fend"] or 0, "at function end" if r.get("to_function_end") else "+0x%x" % (r["end"] - fs))]
    ident = None
    if p["rtti"]:
        ident = "%s vftable slot %d" % p["rtti"][0][:2]
        ev.append("rtti: " + "; ".join("%s slot %d (vftable %08x)" % x for x in p["rtti"]))
    cl = []
    for c in sorted(set(callers.get(fs, []))):
        rt = im.rtti_slots(c)
        ss = im.strings([(c, im.fend(c) or c + 1)])[:2] if not rt else []
        cl.append("%08x%s%s" % (c, " = %s slot %d" % rt[0][:2] if rt else "", " %r" % ss if ss else ""))
    if cl:
        ev.append("called by: " + "; ".join(cl))
    if p["strings"]:
        ev.append("str: " + " | ".join(p["strings"][:4]))
    if twin:
        t = twin
        ts = st.strings([(t["fstart"], t["fend"])])
        files = sorted({m.group(1) for s in ts for m in [re.search(r"\\Client\\(.+?\.(?:cpp|h))", s)] if m})
        ev.append("studio twin %08x: Begin+0x%x End+0x%x size %d%s; prologue %s, tail %s" % (
            t["fstart"], t["begin"] - t["fstart"], t["end"] - 6 - t["fstart"], t["fend"] - t["fstart"],
            " [%s]" % ",".join(sorted({c[1].replace("VMProtect", "") for c in t["calls"]} - {"BeginMutation", "End"})) if len({c[1] for c in t["calls"]}) > 2 else "",
            "identical" if r["pro_eq"] else "DIFFERS (paired by RTTI slot)", "identical" if r["tail_eq"] else "differs"))
        if files:
            ev.append("studio __FILE__: " + ",".join(files))
        extra = [s for s in ts if s not in p["strings"] and "BuildAgent" not in s][:4]
        if extra:
            ev.append("studio str: " + " | ".join(extra))
    else:
        ev.append("no studio twin (Player-only code)")
    mk = im.b(r["marker"], 8).split(b"\0")[0] if r.get("marker") else None
    if mk is not None:
        ev.append("marker name at %08x %s%s" % (r["marker"], "%r" % mk.decode() if all(32 <= c < 127 for c in mk) else "scrambled",
                                                " (studio %r)" % twin["marker"] if twin else ""))
    name = NAMES.get(name_key(im, r))
    if name:
        ident = name + (" [" + ident + "]" if ident else "")
    return ident or "unidentified", "; ".join(ev)


def pair(im, regions, patches, st, sfuncs):
    """Attach the STUDIO twin to each region/patch: same normalised code before Begin and after End; failing
    that (the function changed between the builds) the same RTTI class and vftable slot."""
    free = list(sfuncs)
    todo = regions + [p for p in patches if p["kind"] == "begin-stripped"]
    for r in todo:
        pro = im.norm(r["fstart"], r["site"])
        tail = im.norm(r["end"], r["fend"]) if r.get("fend", 0) > r.get("end", 0) else []
        cands = [t for t in free if st.norm(t["fstart"], t["begin"]) == pro]
        best = [t for t in cands if "end" not in r or st.norm(t["end"], t["fend"]) == tail] or cands
        r["twin"], r["pro_eq"] = (best[0] if best else None), bool(cands)
        r["tail_eq"] = bool(best and "end" in r and st.norm(best[0]["end"], best[0]["fend"]) == tail)
        if r["twin"]:
            free.remove(r["twin"])
    for r in todo:
        if not r["twin"]:
            rt = {x[:2] for x in im.rtti_slots(r["fstart"])}
            r["twin"] = next((t for t in free if rt & {x[:2] for x in t["rtti"]}), None)
            if r["twin"]:
                free.remove(r["twin"])
    return free


def name_key(im, r):
    """Key into NAMES that is the same in every build: STUDIO twin, else RTTI slot, else first string."""
    if r.get("twin"):
        return "studio:%08x" % r["twin"]["fstart"]
    rt = im.rtti_slots(r["fstart"])
    if rt:
        return "rtti:%s:%d" % rt[0][:2]
    ss = im.strings([(r["fstart"], r["site"])] + ([(r["end"], r["fend"])] if "end" in r else []))
    return "str:" + ss[0] if ss else None


def run(target, st, sfuncs, write=True):
    path, fcsv, out = TARGETS[target]
    im = Img(path, fcsv)
    regions, patches, raw, back = player_scope(im)
    unpaired = pair(im, regions, patches, st, list(sfuncs))
    callers = im.callers({r["fstart"] for r in regions + patches})
    rows = []
    for r in regions:
        ident, ev = describe(im, r, callers, st, r["twin"])
        kind = "function-redirect" if r["site"] == r["fstart"] else "marker-region"
        rows.append((r["site"], r["end"] - r["site"], kind, ident, ev))
    for p in patches:
        twin = p.get("twin")
        fs = p["fstart"]
        if p["kind"] == "begin-stripped":
            ident = NAMES.get(name_key(im, p), "unidentified")
            rt = im.rtti_slots(fs)
            cl = ["%08x%s" % (c, " = %s slot %d" % im.rtti_slots(c)[0][:2] if im.rtti_slots(c) else "") for c in sorted(set(callers.get(fs, [])))]
            ev = "func %08x+0x%x; Begin call replaced by lea esp,[esp+4]/nop/nop, body intact%s%s%s" % (
                fs, p["site"] - fs, "; rtti: %s slot %d" % rt[0][:2] if rt else "", "; called by: " + "; ".join(cl) if cl else "",
                "; studio twin %08x size %d; marker name %r is also used by studio %s (duplicate name)" % (
                    twin["fstart"], twin["fend"] - twin["fstart"], twin["marker"],
                    ",".join("%08x" % t["fstart"] for t in sfuncs if t["marker"] == twin["marker"] and t["fstart"] != twin["fstart"])) if twin else "")
        else:
            owner = next((r for r in regions + patches if r["fstart"] == fs and r is not p), None)
            ident = "same function as 0x%08x" % owner["site"] if owner else "unidentified"
            ev = "func %08x+0x%x; %s" % (fs, p["site"] - fs, "second VMProtectEnd call replaced by 6 NOPs" if p["kind"] == "end-call-nopped" else "jmp ds:VMProtectEnd replaced by ret + 5 NOPs")
        rows.append((p["site"], 6, "marker-" + p["kind"], ident, ev))
    rows.sort()
    if write:
        with open(out, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["address", "size", "kind", "identity", "evidence"])
            for a, n, k, i, e in rows:
                w.writerow(["0x%08x" % a, n, k, i, e])
    return im, regions, patches, raw, back, rows, unpaired


def report(target, im, regions, patches, raw, rows, unpaired):
    print("== %s  .text %08x-%08x (%d bytes)" % (target, im.text[0], im.text[1], im.text[1] - im.text[0]))
    inside = sum(1 for a, op, t in raw if any(r["site"] < a < r["end"] for r in regions))
    entries = sum(1 for a, op, t in raw if any(r["site"] == a for r in regions))
    print("   rel32 jmp/call byte patterns into .vmp*: %d = %d region entries + %d inside holes + %d operand bytes of real instructions"
          % (len(raw), entries, inside, len(raw) - entries - inside))
    for a, n, k, i, e in rows:
        print("   %08x %5d %-22s %s" % (a, n, k, i))
    holes = sum(n for a, n, k, i, e in rows if k in ("marker-region", "function-redirect"))
    other = sum(n for a, n, k, i, e in rows if k not in ("marker-region", "function-redirect"))
    print("   holes: %d regions, %d bytes; removed marker calls: %d sites, %d bytes; total %d bytes = %.4f%% of .text"
          % (len(regions), holes, len(patches), other, holes + other, 100.0 * (holes + other) / (im.text[1] - im.text[0])))
    ext = {r["fstart"]: r["fend"] - r["fstart"] for r in regions}
    ext.update({p["fstart"]: im.fend(p["fstart"]) - p["fstart"] for p in patches if p["fstart"] not in ext})
    print("   functions touched: %d, %d bytes in total (what a whole-function score loses)" % (len(ext), sum(ext.values())))
    print("   STUDIO marker functions with no counterpart here: %s" % (["%08x" % t["fstart"] for t in unpaired] or "none"))
    slots, descs = orig_imports(im)
    d = im.pe.OPTIONAL_HEADER.DATA_DIRECTORY
    ilo = im.base + d[12].VirtualAddress
    text = im.b(im.text[0], im.text[1] - im.text[0])
    cnt = {"orig": [0, 0, set()], "dir": [0, 0, set()]}
    for m in re.finditer(b"\xff[\x15\x25]", text):
        v = struct.unpack_from("<I", text, m.start() + 2)[0] if m.start() + 6 <= len(text) else 0
        key = "orig" if v in slots else "dir" if ilo <= v < ilo + d[12].Size else None
        if key:
            cnt[key][text[m.start() + 1] == 0x25] += 1
            cnt[key][2].add(v)
    print("   import directory %08x (IAT %08x+%x) is in %s; linker's original descriptors: %d at %08x, %d slots %08x-%08x in .rdata"
          % (im.base + d[1].VirtualAddress, ilo, d[12].Size, next(n for n, lo, hi in im.sec if lo <= ilo < hi), len(descs), descs[0], len(slots), min(slots), max(slots) + 4))
    print("   .text FF15/FF25 byte patterns: original IAT call=%d jmp=%d (%d distinct slots); directory IAT call=%d jmp=%d"
          % (cnt["orig"][0], cnt["orig"][1], len(cnt["orig"][2]), cnt["dir"][0], cnt["dir"][1]))
    orphan = [a for a in range(min(slots), max(slots) + 4, 4) if im.u32(a) and a not in slots]
    print("   original IAT slots owned by no descriptor (the removed VMProtectSDK32.dll group): %s"
          % ", ".join("%08x=%08x" % (a, im.u32(a)) for a in orphan))
    return holes + other


def main():
    st = Img(*STUDIO)
    sfuncs, sslots = studio_markers(st)
    print("STUDIO VMProtectSDK32 imports: %s" % ", ".join("%08x %s" % kv for kv in sorted(sslots.items())))
    print("STUDIO functions with markers: %d" % len(sfuncs))
    res = {}
    for target in TARGETS:
        im, regions, patches, raw, back, rows, unpaired = run(target, st, [dict(t) for t in sfuncs])
        report(target, im, regions, patches, raw, rows, unpaired)
        res[target] = (im, regions, patches, rows)
    # do the two Players protect the same functions? pair their regions by twin, else by normalised prologue
    a, b = res["PLAYER"], res["PLAYER_1015"]
    key = name_key
    ka = {key(a[0], r): r for r in a[1] + a[2] if "twin" in r}
    kb = {key(b[0], r): r for r in b[1] + b[2] if "twin" in r}
    print("== PLAYER vs PLAYER_1015: %d common, only PLAYER %s, only PLAYER_1015 %s" % (
        len(set(ka) & set(kb)), ["%08x" % ka[k]["site"] for k in set(ka) - set(kb)], ["%08x" % kb[k]["site"] for k in set(kb) - set(ka)]))
    for k in sorted(set(ka) & set(kb), key=lambda k: ka[k]["site"]):
        ra, rb = ka[k], kb[k]
        sa, sb = ra.get("end", ra["site"] + 6) - ra["site"], rb.get("end", rb["site"] + 6) - rb["site"]
        print("   %08x %5d  <->  %08x %5d%s" % (ra["site"], sa, rb["site"], sb, "" if sa == sb else "   SIZE DIFFERS"))
    return res


def selftest():
    st = Img(*STUDIO)
    sfuncs, sslots = studio_markers(st)
    assert sorted(sslots.values()) == ["VMProtectBeginMutation", "VMProtectEnd", "VMProtectIsDebuggerPresent", "VMProtectIsValidImageCRC"]
    for target in TARGETS:
        im, regions, patches, raw, back, rows, unpaired = run(target, st, [dict(t) for t in sfuncs], write=False)
        assert regions and not unpaired, (target, unpaired)
        for r in regions:
            assert r["site"] != r["fstart"], "whole-function redirect found"
            assert im.b(r["site"], 1) == b"\xe9" and im.in_vmp(r["target"])
            if r["twin"]:  # the hole is exactly the marker region of the unprotected twin
                t = r["twin"]
                if not r["pro_eq"]:
                    continue  # function changed between the builds, paired by RTTI only
                assert r["site"] - r["fstart"] == t["begin"] - t["fstart"], hex(r["site"])
                assert r["end"] - r["fstart"] == t["end"] - t["fstart"], hex(r["site"])
        # every rel32 call/jcc byte pattern into .vmp* is inside a hole or inside a real instruction
        for a, op, t in raw:
            if not any(r["site"] <= a < r["end"] for r in regions):
                assert not im.lands(im.fstart(a), a), hex(a)
        slots, descs = orig_imports(im)
        assert len(descs) == 30 and len(slots) == 1928
    print("selftest ok")


if __name__ == "__main__":
    if "--selftest" in sys.argv:
        selftest()
    elif "--iat" in sys.argv:
        im = Img(*TARGETS[sys.argv[sys.argv.index("--iat") + 1]][:2])
        w = csv.writer(sys.stdout)
        w.writerow(["slot", "dll", "name"])
        w.writerows(("0x%08x" % a, d, n) for a, (d, n) in sorted(orig_imports(im)[0].items()))
    else:
        main()
