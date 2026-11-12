"""MSVC (VS2008, x86) RTTI and vftable walker, for IDA databases and for plain PE files.

A library: tools/ida/export_evidence.py and tools/ida/name_rtti.py import it (they put tools/ida on
sys.path). It reads bytes only and never changes a database.

  R = scan(from_ida())  or  scan(from_pe(path))  ->  Rtti with
    tds   {TypeDescriptor ea: ".?AV..." name}  every `.?A` descriptor: [pVFTable][spare=0][name]
    cols  {COL ea: Col}   signature 0, pTypeDescriptor -> a TD, pClassDescriptor -> a valid CHD whose
                          bases[0] is that TD
    chds  {CHD ea: Chd}   signature 0, attributes (1 multiple, 2 virtual, 4 ambiguous), base array
    bcds  {BCD ea: Bcd}   td, num_contained, PMD (mdisp, pdisp, vdisp), attributes, chd (if 0x40)
    vfts  {vftable ea: Vft}  every data word holding a COL address, plus the slots after it: words
          that point at code, ending at the first word that does not (the next COL pointer, data,
          a string), or, on IDA, at a word something references (another table starts there)
    classes {TD ea: [Vft...]}  each complete class's vftables, ordered by (COL offset, cdOffset)

vfptr_paths(R, td) says which bases name each vftable; vftable_name(R, vft) gives MSVC's decorated
name (??_7Class@@6B@ or ??_7Class@@6BBase@@@), col_name/td_name/chd_name/bca_name/bcd_name the
??_R4/_R0/_R3/_R2/_R1 names. The rule and the name mangling are checked against every vftable
OgreMain.dll and RenderSystem_Direct3D9.dll export (the same VS2008 toolchain, with real names):

  python3 -P tools/ida/msvc_rtti.py bin/player/OgreMain.dll bin/player/RenderSystem_Direct3D9.dll
"""
import bisect
import re
import struct
from collections import namedtuple

Col = namedtuple("Col", "ea offset cd_offset td chd")
Chd = namedtuple("Chd", "ea attributes bases")  # bases: [BCD ea], bases[0] = the class itself
Bcd = namedtuple("Bcd", "ea td num_contained mdisp pdisp vdisp attributes chd")
Vft = namedtuple("Vft", "ea col slots")  # slots: [target ea]

BCD_HASPCHD = 0x40


def s32(v):
    return v - (1 << 32) if v & 0x80000000 else v


class Rtti:
    """data: [(start, end, bytes)] of the initialised, non-code sections; code: [(start, end)].
    slot_ok(target): the target is an instruction start (default: inside a code range).
    referenced(ea): something references this data word (default: unknown, never)."""

    def __init__(self, data, code, slot_ok=None, referenced=None):
        self.data, self.code = sorted(data), sorted(code)
        self._dstarts = [d[0] for d in self.data]
        self._cstarts = [c[0] for c in self.code]
        self.slot_ok = slot_ok or self.is_code
        self.referenced = referenced or (lambda ea: False)
        self.tds, self.cols, self.chds, self.bcds, self.vfts, self.classes, self.paths = {}, {}, {}, {}, {}, {}, {}

    def _seg(self, ea):
        i = bisect.bisect_right(self._dstarts, ea) - 1
        if i >= 0 and ea < self.data[i][1]:
            return self.data[i]
        return None

    def u32(self, ea):
        d = self._seg(ea)
        if d is None or ea + 4 > d[1]:
            return None
        return struct.unpack_from("<I", d[2], ea - d[0])[0]

    def is_data(self, ea):
        return self._seg(ea) is not None

    def is_code(self, ea):
        i = bisect.bisect_right(self._cstarts, ea) - 1
        return i >= 0 and ea < self.code[i][1]

    def bcd(self, ea):
        if ea in self.bcds:
            return self.bcds[ea]
        w = [self.u32(ea + 4 * k) for k in range(6)]
        r = None
        if None not in w and w[0] in self.tds and w[1] < 4096 and w[5] < 0x100:
            chd = self.u32(ea + 24) if w[5] & BCD_HASPCHD else None
            r = Bcd(ea, w[0], w[1], s32(w[2]), s32(w[3]), s32(w[4]), w[5], chd)
        self.bcds[ea] = r
        return r

    def chd(self, ea):
        if ea in self.chds:
            return self.chds[ea]
        r = None
        w = [self.u32(ea + 4 * k) for k in range(4)]
        # attributes 5 = multiple + ambiguous (CSourceStream, PLAYER 0x00c70da0), so < 8, not < 4
        if None not in w and w[0] == 0 and w[1] < 8 and 1 <= w[2] <= 4096 and self.is_data(w[3]):
            bases = [self.u32(w[3] + 4 * k) for k in range(w[2])]
            if None not in bases and all(self.bcd(b) for b in bases):
                r = Chd(ea, w[1], bases)
        self.chds[ea] = r
        return r


def from_ida():
    import ida_bytes, ida_segment, idautils
    data, code = [], []
    for s in idautils.Segments():
        seg = ida_segment.getseg(s)
        if seg.type == ida_segment.SEG_CODE:
            code.append((seg.start_ea, seg.end_ea))
        elif seg.type not in (ida_segment.SEG_XTRN, ida_segment.SEG_BSS):
            data.append((seg.start_ea, seg.end_ea, ida_bytes.get_bytes(seg.start_ea, seg.end_ea - seg.start_ea)))
    return Rtti(data, code,
                slot_ok=lambda t: ida_bytes.is_code(ida_bytes.get_flags(t)),  # an instruction head, not "png\0"
                referenced=lambda ea: ida_bytes.has_xref(ida_bytes.get_flags(ea)))


def from_pe(path):
    import pefile
    pe = pefile.PE(path, fast_load=True)
    img, base = pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
    data, code = [], []
    for s in pe.sections:
        a, b = base + s.VirtualAddress, base + s.VirtualAddress + max(s.Misc_VirtualSize, s.SizeOfRawData)
        if s.Characteristics & 0x20000000:  # IMAGE_SCN_MEM_EXECUTE
            code.append((a, b))
        else:
            data.append((a, b, img[a - base:b - base]))
    return Rtti(data, code)


def scan(R):
    for start, end, buf in R.data:  # 1. type descriptors
        for m in re.finditer(rb"\.\?A[VUTW]", buf):
            td = start + m.start() - 8
            if td < start or td & 3 or R.u32(td + 4) != 0 or not R.u32(td):
                continue
            e = buf.find(b"\0", m.start())
            name = buf[m.start():e].decode("latin-1")
            # not required to end in "@": MSVC truncates very long names and appends an MD5
            if len(name) > 5 and all(32 < ord(c) < 127 for c in name):
                R.tds[td] = name
    for start, end, buf in R.data:  # 2. complete object locators
        n = len(buf) // 4
        w = struct.unpack_from("<%dI" % n, buf)
        for k in range(n - 4):
            if w[k] == 0 and w[k + 3] in R.tds and w[k + 1] < 0x100000 and w[k + 2] < 0x10000:
                chd = R.chd(w[k + 4])
                if chd and R.bcds[chd.bases[0]].td == w[k + 3]:
                    ea = start + 4 * k
                    R.cols[ea] = Col(ea, w[k + 1], w[k + 2], w[k + 3], w[k + 4])
    for start, end, buf in R.data:  # 3. vftables: [COL pointer][code pointers...]
        n = len(buf) // 4
        w = struct.unpack_from("<%dI" % n, buf)
        for k in range(n - 1):
            if w[k] in R.cols:
                ea, slots, j = start + 4 * (k + 1), [], k + 1
                while j < n and R.is_code(w[j]) and R.slot_ok(w[j]) and not (slots and R.referenced(start + 4 * j)):
                    slots.append(w[j])
                    j += 1
                if slots:
                    R.vfts[ea] = Vft(ea, w[k], slots)
    for v in sorted(R.vfts.values(), key=lambda v: (R.cols[v.col].offset, R.cols[v.col].cd_offset, v.ea)):
        R.classes.setdefault(R.cols[v.col].td, []).append(v)
    return R


def vfptr_paths(R, td):
    """{COL offset: [TD ea...]} for the class whose TypeDescriptor is td: the bases MSVC mangles into
    each vftable's name, deepest first ([] is the plain `6B@`). An offset missing from the dict could
    not be placed. Cached on R.
    This is the MSVC ABI rule as clang implements it (MicrosoftVTableContext::computeVPtrInfo and
    rebucketPaths), run over the RTTI base array: a class's paths are its direct bases' paths, each
    marked with that base as its next name to mangle, plus one of its own when it introduces a
    vfptr; while two paths have the same mangled path, each one that has a next name takes it.
    A subobject introduces a vfptr when a vftable exists at its offset (a COL offset of the complete
    class) that none of its bases accounts for, and it is polymorphic (see there). Proven on every vftable OgreMain.dll and
    RenderSystem_Direct3D9.dll export (see _selfcheck); the "deepest base at the offset" rule an
    earlier draft used got 23 of 442 wrong (Font's primary is `6BResource@1@@`, not StringInterface;
    Camera's Renderable one is plain `6B@`).
    Virtual bases: RTTI does not hold their offsets (the vbtable does). Each one's vfptrs keep the
    offsets its own standalone vftables have, and the bases are laid on the COL offsets that no
    non-virtual subobject takes, in base-array order. ponytail: no vbtable is read; read one from
    a constructor if a name in a class with virtual bases ever matters."""
    if td in R.paths:
        return R.paths[td]
    group = R.classes[td]
    S = {R.cols[v.col].offset for v in group}
    B = [R.bcds[b] for b in R.chds[R.cols[group[0].col].chd].bases]
    nv = {b.mdisp for b in B if b.pdisp == -1}
    left = sorted(o for o in S if o not in nv)
    vbo, k = {}, 0
    for b in B:
        key = (b.pdisp, b.vdisp)
        if b.pdisp != -1 and key not in vbo:
            rel = sorted({R.cols[v.col].offset for v in R.classes.get(b.td, ())}) or [0]
            vbo[key] = left[k] - rel[0] if k < len(left) else None
            k += len(rel)

    def absoff(i):
        b = B[i]
        if b.pdisp == -1:
            return b.mdisp
        base = vbo.get((b.pdisp, b.vdisp))
        return None if base is None else base + b.mdisp

    def related(i, j):  # one contains the other in the base array
        lo, hi = min(i, j), max(i, j)
        return hi <= lo + B[lo].num_contained

    def polymorphic(i):
        """Has a vftable of its own in this image, or (a base from another module, or a
        __declspec(novtable) interface) no unrelated subobject with one shares its offset: an empty
        AllocatedObject<> sits at FrameListener's offset in Ogre::FrameTimeControllerValue."""
        if B[i].td in R.classes:
            return True
        o = absoff(i)
        return not any(B[j].td in R.classes and absoff(j) == o and not related(i, j) for j in range(len(B)))

    def children(i):
        j, end = i + 1, i + B[i].num_contained
        while j <= end:
            yield j
            j += B[j].num_contained + 1

    def paths(i):  # [[offset, mangled path, next base to mangle]]
        res, seen = [], set()
        for c in children(i):
            for off, mp, nxt in paths(c):
                if off in seen:
                    continue  # a virtual base reached twice
                seen.add(off)
                res.append([off, list(mp), B[c].td if not mp or mp[-1] != B[c].td else nxt])
        o = absoff(i)
        if o is not None and o in S and o not in seen and polymorphic(i):
            res.append([o, [], None])
        changed = True
        while changed:
            changed, buckets = False, {}
            for p in res:
                buckets.setdefault(tuple(p[1]), []).append(p)
            for ps in buckets.values():
                if len(ps) > 1:
                    for p in ps:
                        if p[2] is not None:
                            p[1].append(p[2])
                            p[2], changed = None, True
        return res

    r = R.paths[td] = {off: mp for off, mp, _ in paths(0)}
    return r


# ------------------------------------------------------------------ MSVC name mangling (subset)

class _P:
    """Just enough of the MSVC decoration grammar to find where each name fragment ends, so the
    outer-level fragments of a qualified name can be back-referenced the way the compiler does."""

    def __init__(self, s):
        self.s, self.i = s, 0

    def c(self, n=1):
        return self.s[self.i:self.i + n]

    def take(self, n=1):
        r = self.s[self.i:self.i + n]
        if len(r) < n:
            raise ValueError("eof")
        self.i += n
        return r

    def ident(self):
        e = self.s.index("@", self.i)
        r = self.s[self.i:e + 1]
        self.i = e + 1
        return r

    def number(self):
        if self.c() == "?":
            self.i += 1
        if self.c().isdigit():
            self.i += 1
        else:
            self.ident()

    def fragment(self):
        st = self.i
        ch = self.c()
        if ch.isdigit():
            self.i += 1
        elif self.c(2) == "?$":
            self.i += 2
            if self.c() == "?":  # operator template name (?$?6...)
                self.i += 2
            else:
                self.ident()
            while self.c() != "@":
                self.targ()
            self.i += 1
        elif self.c(2) == "?A":  # anonymous namespace ?A0x...@
            self.ident()
        elif ch == "?":  # local scope ?N?<decorated function>@ (`?1??fn@C@@...Z@`)
            self.i += 1
            self.number()
            self.take()  # '?' before the enclosing function's own decorated name
            self.symbol()  # the '@' after it ends the whole qualified name
        else:
            self.ident()
        return self.s[st:self.i]

    def qname(self):
        frags = []
        while self.c() != "@":
            frags.append(self.fragment())
        self.i += 1
        return frags

    def symbol(self):  # ?name@scope@@ + encoding
        self.take()  # '?'
        if self.c() == "?":  # special name (?0 ctor, ?_7, ...)
            self.take(2 if self.c(2) != "?_" else 3)
        self.qname()
        k = self.take()
        if k in "0123":  # data: type + cv
            self.typ()
            self.take()
        elif k == "Y":
            self.func()
        elif k in "ABEFIJMNQRUV":  # non-static member function: this-cv, then the function
            self.take()
            self.func()
        elif k in "CDKLST":
            self.func()
        else:
            raise ValueError("encoding " + k)

    def func(self):  # callconv, return, args, throw
        if self.c() == "_":
            self.i += 1
        self.take()
        if self.c() == "@":
            self.i += 1
        else:
            self.typ(ret=True)
        self.args()
        self.take()  # 'Z' throw spec

    def args(self):
        if self.c() in ("X", "Z"):
            self.i += 1
            return
        while self.c() not in ("@", "Z"):
            self.typ()
        self.i += 1

    def targ(self):
        if self.c(3) in ("$$V", "$$Z") or self.c(2) == "$S":
            self.take(3 if self.c(2) == "$$" else 2)
        elif self.c(2) == "$0":
            self.i += 2
            self.number()
        elif self.c(2) in ("$1", "$E", "$H"):
            self.i += 2
            if self.c() == "@":
                self.i += 1
            else:
                self.symbol()
        else:
            self.typ()

    def typ(self, ret=False):
        ch = self.take()
        if ch.isdigit() or ch in "CDEFGHIJKMNOXZ":
            return
        if ch == "_":
            self.take()
            return
        if ch == "W":
            self.take()
            self.qname()
        elif ch in "TUVY":
            if ch == "Y":  # array: dims, sizes, element type
                n = self.c()
                self.number()
                for _ in range(int(n) + 1 if n.isdigit() else 1):
                    self.number()
                self.typ()
            else:
                self.qname()
        elif ch in "PQRSAB" or ch == "?":
            while self.c() in ("E", "I", "F"):
                self.i += 1
            if ch != "?" and self.c() == "6":
                self.i += 1
                self.func()
            elif ch != "?" and self.c() == "8":
                self.i += 1
                self.qname()
                self.take()
                self.func()
            else:
                if self.take() in "QRST":  # pointer to member: cv, then the class
                    self.qname()
                self.typ()
        elif ch == "$":
            k = self.take(2)
            if k in ("$Q", "$R", "$B"):
                self.typ()
            elif k == "$C":
                self.take()
                self.typ()
            elif k == "$A":
                self.take()  # '6'
                self.func()
            elif k != "$T":
                raise ValueError("$$" + k)
        else:
            raise ValueError("type " + ch)


def split_qname(s):
    """Outer fragments of a decorated qualified name ending in '@' (`Instance@RBX@@` -> ['Instance@',
    'RBX@']), or None when it does not parse (a truncated name)."""
    try:
        p = _P(s)
        f = p.qname()
        return f if p.i == len(s) else None
    except (ValueError, IndexError):
        return None


def _resolve(frags):
    """Replace outer back-references by the fragment they stand for."""
    out = []
    for f in frags:
        out.append(out[int(f)] if len(f) == 1 and f.isdigit() else f)
    return out


def _compress(table, frags):
    out = []
    for f in frags:
        if f in table and table.index(f) < 10:
            out.append(str(table.index(f)))
        else:
            out.append(f)
            if len(table) < 10 and not f.isdigit():
                table.append(f)
    return "".join(out)


def class_part(td_name):
    return td_name[4:]  # ".?AV" stripped: `Instance@RBX@@`


def suffix(R, vft):
    """`6B@` or `6B<base>@...@` with MSVC's back-references, or None when the vftable's path or a
    name in it cannot be worked out."""
    col = R.cols[vft.col]
    mp = vfptr_paths(R, col.td).get(col.offset)
    if mp is None:
        return None
    if not mp:
        return "6B@"
    parts = [split_qname(class_part(R.tds[t])) for t in [col.td] + mp]
    if None in parts:
        return None
    table = []
    _compress(table, _resolve(parts[0]))
    return "6B" + "".join(_compress(table, _resolve(p)) + "@" for p in parts[1:]) + "@"


def vftable_name(R, vft):
    s = suffix(R, vft)
    return s and "??_7" + class_part(R.tds[R.cols[vft.col].td]) + s


def col_name(R, vft):
    s = suffix(R, vft)
    return s and "??_R4" + class_part(R.tds[R.cols[vft.col].td]) + s


def td_name(R, td):
    return "??_R0" + R.tds[td][1:] + "@8"


def chd_name(R, chd):
    return "??_R3" + class_part(R.tds[R.bcds[R.chds[chd].bases[0]].td]) + "8"


def bca_name(R, chd):
    return "??_R2" + class_part(R.tds[R.bcds[R.chds[chd].bases[0]].td]) + "8"


def _num(v):
    if v < 0:
        return "?" + _num(-v)
    if 1 <= v <= 10:
        return str(v - 1)
    return "".join("ABCDEFGHIJKLMNOP"[int(h, 16)] for h in "%X" % v).lstrip("A") + "@" if v else "A@"


def bcd_name(R, bcd):
    b = R.bcds[bcd]
    return "??_R1" + _num(b.mdisp) + _num(b.pdisp) + _num(b.vdisp) + _num(b.attributes) + class_part(R.tds[b.td]) + "8"


def _selfcheck(paths):
    """Every vftable a DLL exports must be found, at the exported address, with the exported name."""
    import pefile
    assert _num(0) == "A@" and _num(-1) == "?0" and _num(12) == "M@" and _num(0x40) == "EA@" and _num(4) == "3"
    for path in paths:
        R = scan(from_pe(path))
        bad = [n for n in R.tds.values() if n.endswith("@") and split_qname(class_part(n)) is None]
        print("%s: %d TDs (%d do not parse%s), %d COLs, %d vftables, %d classes" % (
            path, len(R.tds), len(bad), (": " + bad[0][:80]) if bad else "", len(R.cols), len(R.vfts), len(R.classes)))
        pe = pefile.PE(path, fast_load=True)
        pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_EXPORT"]])
        base = pe.OPTIONAL_HEADER.ImageBase
        ok = wrong = missing = 0
        for e in pe.DIRECTORY_ENTRY_EXPORT.symbols:
            n = (e.name or b"").decode()
            if not n.startswith("??_7"):
                continue
            v = R.vfts.get(base + e.address)
            mine = v and vftable_name(R, v)
            if mine == n:
                ok += 1
            elif v is None:
                missing += 1
                print("  not found:", hex(base + e.address), n)
            else:
                wrong += 1
                print("  wrong:", hex(v.ea), n, "->", mine)
        print("  exported vftables: %d named right, %d wrong, %d not found" % (ok, wrong, missing))
        assert wrong == missing == 0


if __name__ == "__main__":
    import sys
    _selfcheck(sys.argv[1:])
