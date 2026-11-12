"""Scout the 2013 layout of a class from the original binary and compare it with our recompiled PDB.

For each class it prints (and with --csv appends rows for):
  * the RTTI base chain with each base's offset (mdisp), from ida/exports/<T>-evidence/vtables.jsonl
  * every vftable group: address, slot count, each slot's address and name (names.csv), and the iOS
    10-29 / 10-21 slot names beside it (Itanium D1/D0 collapsed onto MSVC's one destructor slot)
  * every function that stores one of the class's vftables (ctors and dtors) with each access through
    `this` (offset, width, read/write, stored value or callee), traced linearly with capstone
  * operator-new sizes: `push N; call alloc` right before a call to a ctor, in every caller
  * our layout from the recompiled PDB (cvdump -t), flattened through base classes, and for each
    2013 access the member of ours that covers that offset
  * the app-sweep score of every evidence function (100% = the fact is proven by a matching function)

Run with the reccmp venv python (it uses reccmp's cvdump types parser; cvdump runs under wine once and
the dump is cached in build-class-layout/):
    toolchain/venv/bin/python -I tools/class_layout.py --target PLAYER "RBX::Name" "RBX::Instance"
    toolchain/venv/bin/python -I tools/class_layout.py --target PLAYER --csv notes/class-layouts-2013.csv --all
  notes/class-layouts-2013.csv was made with (delete it first; rows are appended):
    toolchain/venv/bin/python -I tools/class_layout.py --target PLAYER --all --findings --csv notes/class-layouts-2013.csv \
        --fn 0x006136c0:RBX::Name --fn 0x00613530:RBX::Name --fn 0x0066fd40:RBX::ScriptContext \
        --fn 0x0066fd10:RBX::ScriptContext --fn 0x0064a350:RBX::DataModel --fn 0x0070cf60:RBX::DataModelArbiter \
        --fn 0x009babf0:RBX::TaskScheduler::Job --fn 0x008eb4d0:RBX::MovingAssemblyStage RBX::DataModelArbiter
    toolchain/venv/bin/python -I tools/class_layout.py --target STUDIO --all --csv notes/class-layouts-2013.csv \
        --fn 0x00702f90:RBX::Name
  --dispmap JSON PREFIX  pairs original vs our displacements from a reccmp --json (e.g. build/sweep/PLAYER-final.json)
  --fn 0xADDR[:Class]  also trace an extra function (e.g. a method that reads a member), owned by Class.
  --pdb PATH           recompiled PDB (default build/RobloxPlayerBeta.pdb or build/RobloxStudioBeta.pdb).
  --selftest           runs the self-check on the Instance ctor's known stores.
"""
import argparse
import csv
import json
import os
import re
import struct
import subprocess
import sys
from collections import defaultdict

import capstone
from capstone import x86

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXP = os.path.join(ROOT, "ida", "exports")
BIN = {"PLAYER": "bin/player_1023/RobloxPlayerBeta.exe", "STUDIO": "bin/studio/RobloxStudioBeta.exe"}
PDB = {"PLAYER": "build/RobloxPlayerBeta.pdb", "STUDIO": "build/RobloxStudioBeta.pdb"}
IOS = {"PLAYER": "IOS_1029", "STUDIO": "IOS_1021"}
CACHE = os.path.join(ROOT, "build-class-layout")

# Phase 4 classes (DECOMP_PLAN order) plus the ones the app sweep flagged. RTTI spelling.
CLASSES = [
    "RBX::Debugable", "RBX::Name", "RBX::GuidItem<class RBX::Instance>",
    "rbx::signals::connection::islot",
    "rbx::signals::signal<void __cdecl (bool)>::callable_slot<class boost::function<void __cdecl (bool)>>",
    "RBX::Reflection::Descriptor", "RBX::Reflection::EnumDescriptor", "RBX::Reflection::ClassDescriptor",
    "RBX::Reflection::Type", "RBX::Reflection::EventSource", "RBX::Object", "RBX::Instance",
    "RBX::ServiceProvider", "RBX::DataModel", "RBX::TaskScheduler::Job", "RBX::DataModelJob",
    "RBX::DataModel::GenericJob", "RBX::ScriptContext", "RBX::GcJob", "RBX::LibraryService",
    "RBX::AssemblyStage", "RBX::MovingAssemblyStage",
]
MAX_OTHER = 4
# Allocators IDA left unnamed. 0x0061bbd0: the Instance allocator (Creatable<Instance>::create paths;
# Instance::initOnDemand 0x0061f990 does `push 0x14; call 0x61bbd0`).
EXTRA_ALLOC = {0x0061BBD0}
ALLOC_RE = re.compile(r"operator new|\?\?2@|malloc|allocate|rbx_allocate|Allocator", re.I)


class Image:
    def __init__(self, path):
        self.data = open(path, "rb").read()
        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        nsec, = struct.unpack_from("<H", self.data, pe + 6)
        optsz, = struct.unpack_from("<H", self.data, pe + 20)
        self.base, = struct.unpack_from("<I", self.data, pe + 24 + 28)
        self.secs = []
        o = pe + 24 + optsz
        for i in range(nsec):
            name = self.data[o:o + 8].rstrip(b"\0").decode()
            vsize, va, rsize, raw = struct.unpack_from("<IIII", self.data, o + 8)
            self.secs.append((name, self.base + va, max(vsize, rsize), raw, rsize))
            o += 40

    def read(self, va, n):
        for _, sva, size, raw, rsize in self.secs:
            if sva <= va < sva + size:
                off = va - sva
                return self.data[raw + off: raw + min(off + n, rsize)]
        return b""

    def dword(self, va):
        b = self.read(va, 4)
        return struct.unpack("<I", b)[0] if len(b) == 4 else None


def load_target(t):
    names = {}
    for r in csv.DictReader(open(os.path.join(EXP, f"{t}-names.csv"), encoding="utf-8")):
        a = int(r["address"], 16)
        names[a] = ((r["demangled"] or r["name"]), r["source"])
    funcs, callers = {}, defaultdict(set)
    for l in open(os.path.join(EXP, f"{t}-evidence", "functions.jsonl")):
        d = json.loads(l)
        funcs[d["address"]] = d
        for c in d["callees"]:
            callers[c].add(d["address"])
    vt = {}
    for l in open(os.path.join(EXP, f"{t}-evidence", "vtables.jsonl")):
        d = json.loads(l)
        vt.setdefault(d["class"], d)
    strings = {}
    for l in open(os.path.join(EXP, f"{t}-evidence", "strings.jsonl")):
        d = json.loads(l)
        strings[d["address"]] = d["text"]
    return names, funcs, callers, vt, strings


def load_ios(t):
    out = {}
    for l in open(os.path.join(EXP, f"{IOS[t]}-evidence", "vtables.jsonl")):
        d = json.loads(l)
        out.setdefault(d["class"], d)
    return out


def ios_msvc_slots(group, groups=(), msvc_count=None):
    """Itanium slot names with the D1/D0 pair collapsed to one slot, so index i lines up with MSVC.
    For the primary group, Itanium also lists overriders of secondary-base virtuals (they have a
    non-virtual thunk `__ZThn<N>_` in a later group); MSVC gives those no primary slot, so they are
    dropped, and so is the destructor when that still leaves one slot too many (the primary base
    chain then has no virtual destructor, e.g. Instance over Debugable)."""
    thunked = set()
    for g in groups[1:]:
        for s in g["slots"]:
            m = re.match(r"_*ZThn\d+_(.*)", s["name"] or "")
            if m:
                thunked.add("__Z" + m.group(1))
    out, prev_dtor = [], False
    for s in group["slots"]:
        if thunked and (s["name"] or "") in thunked and not re.search(r"D[01]Ev$", s["name"] or ""):
            continue
        n = s["name"] or ""
        dtor = bool(re.search(r"D[01]Ev$", n))
        if dtor and prev_dtor:
            out[-1] = out[-1] + " / " + n
            prev_dtor = False
            continue
        out.append(n)
        prev_dtor = dtor
    if msvc_count is not None and len(out) == msvc_count + 1 and thunked:
        out = [n for n in out if not re.search(r"D[01]Ev( |$)", n)]
    return out


# Hand-read conclusions (2026-11-12), written to the CSV with --findings. Each row: class, target, offset,
# width, what 2013 has, evidence (PLAYER addresses; STUDIO checked with --target STUDIO, same unless noted),
# header_now (ours, from build/RobloxPlayerBeta.pdb), verdict. No fact below is proven by a 100% function:
# the only core-class FUNCTIONs at 100% are ClassDescriptor::operator== 0x00614c30 and two getStageType
# bodies, none of which read a member. "observed" = read from the original bytes.
FINDINGS = [
    ("RBX::Debugable", "both", "0x0", 4, "vfptr; slot 0 = virtual void dump(std::ostream&) (0x009b1a70); no virtual dtor",
     "slot 0 of every Instance-family vftable e.g. 0x00b2a6ec; iOS __ZN3RBX9Debugable4dumpERSo", "empty class, size 1", "WRONG: add virtual dump(std::ostream&); size 4"),
    ("RBX::Name", "both", "0x0", 4, "rbx::atomic<int> sortIndex (out-of-line rbx::atomic<int>::atomic() 0x004022d0, assert Base/include/rbx/atomic.h line 224 `(((long)(&(this->v))) & 0x3) == 0`)",
     "0x006136dd in Name::Name 0x006136c0; lock xadd [Name*] at 0x0061362c and mov [this],esi at 0x0061368d in setOrderIndex 0x00613530", "int* sortIndex @0x0", "WRONG: inline rbx::atomic<int>"),
    ("RBX::Name", "both", "0x4", 0x1c, "std::string const str (basic_string(const char*) import call at 0x006136ec)", "0x006136c0", "str @0x4", "OK"),
    ("RBX::Name", "both", "size", "", "0x20 (operator new 0x20 in Name::declare 0x006138c0)", "0x006138c0", "0x20", "OK"),
    ("RBX::Name", "both", "ctor", "", "Name(const char* const&): ret 4, dereferences the argument (iOS _ZN3RBX4NameC2ERKPKc)", "0x006136c0 / STUDIO 0x00702f90", "explicit Name(const char*)", "WRONG signature"),
    ("RBX::Guid", "both", "0x0", 8, "Data{Scope(const Name*) @0, int index @4}; ctor stores [0]=scope name, [4]=counter", "Guid::Guid 0x006f9740", "Data{scope,index} 8 bytes", "OK"),
    ("RBX::GuidItem<Instance>", "both", "0x4", 8, "boost::shared_ptr<Registry> registry (dtor 0x0061b060: registry->unregister if [+4]!=0, then lock-xadd release of the sp_counted_base at [+8]; unregister 0x0058dd90 resets item+4 via 0x00652fe0)",
     "0x0061b060, 0x0058dd90", "boost::intrusive_ptr<Registry> registry @0x4 (4 bytes)", "WRONG: shared_ptr (8)"),
    ("RBX::GuidItem<Instance>", "both", "0xc", 8, "Guid guid (Instance ctor calls Guid() 0x006f9740 on this+0x10 = GuidItem+0xc; unregister reads item+0xc/+0x10)", "0x00620f24", "guid @0x8", "WRONG (follows registry)"),
    ("RBX::GuidItem<Instance>", "both", "size", "", "0x14 (GuidItem @Instance+4, AbstractFactoryProduct @+0x18 by RTTI)", "RTTI COL of 0x00b2a6ec", "0x10", "WRONG"),
    ("rbx::signals", "both", "", "", "2013 has rbx::signals::{connection::islot, signal<Sig>::slot, signal<Sig>::callable_slot<F>}, islot : noncopyable, intrusive_ptr_target<islot,int,0,0> (empty, @5); no Notifier/Listener class in PLAYER or STUDIO RTTI",
     "vftable 0x00affa04 (islot, 3 slots), 0x00b2eb78 (callable_slot<function<void(bool)>>)", "rbx/signal.h #else branch: intrusive_ptr_target<islot> (= <islot,int,0,0>)", "OK"),
    ("RBX::Reflection::Descriptor", "both", "0x0", 0x10, "vfptr(1 slot: dtor) @0, Attributes byte @4, preferred @8, const Name* name @0xc", "ctor 0x00455710", "same", "OK"),
    ("RBX::Reflection::Type", "both", "0x10", 0xa, "typeInfo @0x10, tag @0x14, isFloat @0x18, isNumber @0x19", "ctor 0x006f97e0", "same", "OK"),
    ("RBX::Reflection::EnumDescriptor", "both", "0x1c", 0x18, "allItems vector @0x1c (zeroes 0x20/0x24/0x28), enumCount @0x2c, enumCountMSB @0x30", "ctor 0x006fa1a0", "same, size 0x34", "OK"),
    ("RBX::Reflection::MemberDescriptorContainer<T>", "both", "0x10", 0x3c,
     "descriptorLookup = rbx::trie<T*,N>: object_pool<Node> @0 (requested_size 0x1a4 = sizeof(trie::Node): string leaf + 96 Node* + value + depth), object_pool<T*> @0x1c (requested_size 4), Node* root @0x38 = pool.construct(1) via 0x006154a0",
     "lookup ctor 0x00615850 called on ClassDescriptor+0x20 at 0x00618335", "DenseHashMap<const char*,T*,...> 0x1c", "WRONG: rbx::trie (rbx/trie.h), not DenseHashMap"),
    ("RBX::Reflection::MemberDescriptorContainer<T>", "both", "size", "", "0x60: descriptors vector @0, lookup @0x10, derivedContainers vector @0x4c, base @0x5c",
     "ClassDescriptor RTTI bases at 16,112,208,304,400; ctor 0x006182e0 zero stores", "0x40", "WRONG (follows lookup)"),
    ("RBX::Reflection::ClassDescriptor", "both", "0x1f0", 0x1c, "security @0x1f0, derivedClasses vector @0x1f4, base @0x204, 4-bit flags @0x208; size 0x20c",
     "ctor 0x00618440 stores at 0x00618514..0x00618561; dtor 0x00402540", "security @0x150, derivedClasses @0x154, base @0x164, bits @0x168, size 0x16c", "WRONG only through MemberDescriptorContainer size"),
    ("RBX::Reflection::DescribedBase", "both", "0x0", 0x10,
     "bases: enable_shared_from_this<DescribedBase> (@+4, 8 bytes) and EventSource (@+0, vftable 0x00b2a060: dtor, processRemoteEvent 0x0066edf0, raiseEventInvocation); descriptor @+0xc; size 0x10; no xmlId",
     "ctor 0x0061c2c0 (zeroes +4/+8, stores EventSource vftable, descriptor at +0xc); ~Instance 0x0061d510 restores EventSource vftable at Instance+0x1c", "own vfptr, no EventSource base, xmlId scoped_ptr @+0x10, size 0x14", "WRONG"),
    ("RBX::Limits::Countable", "both", "0x0", 8, "{count, holder}, not polymorphic (ctor writes [+0]=0,[+4]=0, no vftable; placed after all polymorphic bases)",
     "ctor 0x009b94d0, dtor 0x009b9160", "virtual ~Countable() (adds a vfptr)", "WRONG: drop virtual"),
    ("RBX::Instance", "both", "bases", "", "declaration order (RTTI BCA): Limits::Countable, Diagnostics::Countable<Instance>, Debugable, GuidItem<Instance>, AbstractFactoryProduct<Instance>, Reflection::Described<Instance,sInstance,DescribedBase,27,0>, boost::noncopyable; layout Debugable@0, GuidItem@4, AFP/Object@0x18, Described@0x1c, Limits::Countable@0x2c, Diagnostics::Countable@0x34, noncopyable@0x35",
     "COL of 0x00b2a6ec; vfptr stores at 0x00620f58..0x00620f6b; Countable ctor on this+0x2c at 0x00620ef5", "Described@0, GuidItem@0x14, Countable@0x24; no Debugable vfptr, no Limits::Countable", "WRONG"),
    ("RBX::Instance", "both", "0x38", 4, "Instance* parent", "setParentInternal 0x00620440 reads +0x38 first", "parent @0x38", "OK (coincidence)"),
    ("RBX::Instance", "both", "0x3c", 8, "copy_on_write_ptr<Instances> children", "findChildIndex 0x0061b1d0, removeAllChildren 0x006219b0", "children @0x30", "WRONG offset"),
    ("RBX::Instance", "both", "0x44", 1, "bool isSettingParent (archivable/robloxLocked are NOT Instance members: FWValue<bool> in the FWInstance, setIsArchivable 0x0061d7b0 reads fw+0x1a, setRobloxLocked 0x0061d780 calls FWValue<bool>::set 0x0052c000)",
     "setParentInternal 0x00620646 cmp byte [this+0x44]", "archivable @0x25, robloxLocked @0x26, isSettingParent @0x27", "WRONG: remove the two bools"),
    ("RBX::Instance", "both", "0x48", 8, "shared_ptr<FWBase> fwBase (ctor: fw arg or new FWFinal<FWInstance> 0x38 bytes via 0x0061fe70, then shared_ptr ctor 0x0061a9d0)", "fw() 0x006beaa0 = mov eax,[ecx+0x48]", "fwBase @0x28", "WRONG offset"),
    ("RBX::Instance", "both", "0x50", 4, "scoped_ptr<OnDemandInstance> onDemandPtr", "onDemandRead 0x006be9e0, onDemandWrite 0x00619a00", "onDemandPtr @0x3c", "WRONG offset"),
    ("RBX::Instance", "both", "0x54", 4, "combinedSignal (signal_with_args<2,void(CombinedSignalType,ICombinedSignalData const*)>)", "raisePropertyChanged 0x0061c7c1", "combinedSignal @0x48", "WRONG order"),
    ("RBX::Instance", "both", "0x58", 4, "ancestryChangedSignal (inferred: the remaining signal; ctor 0x0061a340, dtor 0x0061c210)", "0x00620fb3", "ancestryChangedSignal @0x40", "WRONG order (inferred)"),
    ("RBX::Instance", "both", "0x5c", 4, "propertyChangedSignal (signal_with_args<1,void(PropertyDescriptor const*)>)", "raisePropertyChanged 0x0061c7ca", "propertyChangedSignal @0x44", "WRONG order"),
    ("RBX::Instance", "both", "size", "", "0x60 (ServiceProvider members start at 0x60; ScriptContext's IScriptFilter vfptr at 0x60)", "ServiceProvider ctor 0x00644290", "0x4c", "WRONG"),
    ("RBX::Instance", "both", "vftable", "", "Debugable group 28 slots = iOS 30 minus D1/D0 and raiseEventInvocation (both live in secondary groups); dtor is in the AbstractFactoryProduct group (+0x18)", "0x00b2a6ec, 0x00b2a6d4", "", "observed"),
    ("RBX::ServiceProvider", "both", "0x60", 0x2c, "serviceArray vector @0x60, serviceMap @0x70, closingSignal @0x7c, closingLateSignal @0x80, serviceAddedSignal @0x84, serviceRemovingSignal @0x88; size 0x8c",
     "ctor 0x00644290; clearServices 0x006a9540", "same order at 0x4c..0x78", "OK once Instance is 0x60"),
    ("RBX::DataModel", "both", "bases", "", "declaration order: DescribedNonCreatable<DataModel,ServiceProvider,sDataModel> @0, VerbContainer @0x8c, IDataState @0xa0, IMetric @0xa4 (getMetric/getMetricValue), DataModelArbiter @0xa8, Diagnostics::Countable<DataModel> @0xac8",
     "COL of 0x00b2ffec; ctor 0x0064ec50 vfptr stores 0x0064ecce..0x0064ed03", "IDataState @0, VerbContainer @4, DataModelArbiter @0x28, DescribedNonCreatable @0x1260; no IMetric base", "WRONG"),
    ("RBX::DataModel", "both", "size", "", "0xe10", "operator new 0xe10 in 0x0064f760 (STUDIO 0x0076c210)", "0x1510", "WRONG"),
    ("RBX::DataModel", "both", "0xac8", "", "first member offset (ctor 0x0064ed0a); then signals/containers 0xacc..0xaf0", "0x0064ec50", "first member genericJobsLock @0x12d8", "WRONG"),
    ("RBX::DataModel", "both", "0xbe5", 1, "bool isInitialized", "httpGetAsync 0x0064a37b (assert isInitialized, DataModel.cpp line 1292)", "@0x1395", "WRONG offset"),
    ("RBX::DataModel", "both", "0xcdd", 1, "bool suppressNavKeys (by pairing)", "doDataModelSetup 0x0064fb60", "@0x14d8", "WRONG offset"),
    ("RBX::DataModel", "both", "0xcf8", 0x58, "shared_ptr<GenericJob> genericJobs[11] ([0] @0xcf8, [1] @0xd00, [10] @0xd48)", "doDataModelSetup 0x0064f8dc/0x0064f9ce/0x0064fac0", "@0x12f0", "WRONG offset"),
    ("RBX::DataModel", "both", "0xdcc", 4, "placeID (by pairing)", "savePlaceAsync 0x0063f980", "@0x1450", "WRONG offset"),
    ("RBX::DataModel", "both", "0xdec", "", "member ours calls guiBuilder (removeCustomStat 0x0062d85a add ecx,0xdec)", "0x0062d840, 0x006400f0", "@0x149c", "WRONG offset"),
    ("RBX::DataModel", "STUDIO", "0xc60", "", "STUDIO member order differs from PLAYER in 0xc60..0xcac and stores 0xe08 instead of 0xe0c (same size 0xe10)", "STUDIO ctor 0x0076.. (tool --target STUDIO)", "", "observed"),
    ("RBX::DataModelArbiter", "both", "0x830", 8, "shared_ptr to a zeroed 4-byte object (new 4; shared_ptr ctor 0x0070cc50) - NOT ActivityMeter<2> stepActivity", "ctor 0x0070cf60 at 0x0070cfc7", "stepActivity ActivityMeter<2> 0x820 @0x830", "WRONG"),
    ("RBX::DataModelArbiter", "both", "0x838", 0x1e4, "lookup bool tables (memset rows of 0x79, filled to 0x9ad)", "0x0070cfe9", "lookup @0x107c", "WRONG offset"),
    ("RBX::DataModelArbiter", "both", "size", "", "0xa20 (DataModel bases: arbiter @0xa8, next @0xac8); 4 unidentified bytes at 0xa1c", "COL of 0x00b2ffec", "0x1238", "WRONG"),
    ("RBX::TaskScheduler::Job", "both", "0x8", 0x208, "members through dutyCycleWindow @0x1c0 match ours; size 0x210", "ctor 0x009babf0", "same, 0x210", "OK"),
    ("RBX::DataModelJob", "both", "0x210", 8, "taskType @0x210, bool isPerPlayer @0x214; size 0x218 (no stepsAccumulated, no profilingToken)", "ctor 0x0070ca20 at 0x0070ca96/0x0070ca9c", "taskType @0x210, stepsAccumulated @0x218, isPerPlayer @0x220, profilingToken @0x228, size 0x230", "WRONG"),
    ("RBX::DataModel::GenericJob", "both", "0x218", 0x30, "weak_ptr<DataModel> dataModel @0x218, task queue @0x220 (0x28 bytes); size 0x248", "ctor 0x0063ee60; new 0x248 x3 in doDataModelSetup 0x0064f7f0 (STUDIO 0x0076c2a0)", "dataModel @0x230, tasks (0x30) @0x238, size 0x268", "WRONG"),
    ("RBX::GcJob", "both", "0x218", 0xc, "weak_ptr<ScriptContext> @0x218, frequency @0x220 (from fild [ScriptContext+0xf8])", "ctor 0x00679c00 at 0x00679ce1..0x00679d1c", "@0x230/@0x238, size 0x240", "WRONG"),
    ("RBX::ScriptContext", "both", "0x60", 8, "IScriptFilter vfptr @0x60 (vftable 0x00b369cc), Service (empty) @0x64, first member @0x64", "ctor 0x0068ad20", "IScriptFilter @0x4c, Service @0x50", "WRONG (Instance size)"),
    ("RBX::ScriptContext", "both", "size", "", "0x288", "operator new 0x288 in 0x00421700 (STUDIO 0x0045f8a0)", "0x268", "WRONG"),
    ("RBX::ScriptContext", "both", "0xf1", 1, "bool robloxPlace (bools at 0xf0..0xf3)", "setRobloxPlace 0x0066fd14", "@0x260", "WRONG order"),
    ("RBX::ScriptContext", "both", "partial", "", "by reccmp pairing (orig <- ours): globalStates 0x68<-0x9c, scripts 0x8c<-0x54, nextPendingScripts 0x98<-0x60, pendingScripts 0xa0<-0x68, loadingScripts 0xb0<-0x78, yieldEvent 0xc8<-0xd8, waitingThreads 0xcc<-0xdc, gcFrequencyBase 0xf8<-0x1b0, reentrancy 0x120<-0x8c, timeout block 0x128..0x140<-0xa8..0xc0, libraryService 0x258<-0x1a8, luaAllocator 0x260<-0x1a0; WeakThreadRef (vftable 0x00b348b0) @0x70",
     "tools/class_layout.py --dispmap build/sweep/PLAYER-final.json RBX::ScriptContext::", "2016 order", "WRONG order"),
    ("RBX::ScriptContext", "both", "sandboxThread", "", "static: ends `ret` (cdecl), ours `ret 4`", "0x0066fdab", "member function", "WRONG declaration"),
    ("RBX::LibraryService", "both", "0x44", 0x18, "markedLibraries = boost::unordered_set<std::string> (iOS ctor/dtor callees: boost::unordered::detail::table<set<..., std::string, boost::hash<std::string>, ...>>); resumeCallbacks list @0x5c; size 0x68",
     "queueResumeThread 0x0088dd71 [esi+0x60], 0x0088dd77 add esi,0x5c", "std::set markedLibraries (0xc) @0x44, resumeCallbacks @0x50, size 0x5c", "WRONG type"),
    ("RBX::MovingAssemblyStage", "both", "0x10", 0xc, "boost::intrusive::list<Joint, base_hook<list_base_hook<tag<MovingAssemblyStage>>>> (size @0x10, root node @0x14/0x18; Joint hook @Joint+0x28 by RTTI); uiStepJoints std::set @0x1c; size 0x28",
     "onEdgeAdded 0x008eb27a..0x008eb28d; new 0x28 in AssemblyStage::AssemblyStage 0x008ea638", "numJoints/listHead/listTail @0x10, set @0x1c, listHeadB @0x28, size 0x2c", "WRONG: drop listHeadB"),
    ("RBX::Edge", "both", "vtable+0x10", 4, "getEdgeType is vtable slot +0x10 (ours +0xc); Joint slot +0x38 (ours +0x24): the 'onEdgeRemoving +0x10' near miss is this, not a stage member", "MovingAssemblyStage::onEdgeRemoving 0x008eb4f1, removeJoint 0x008eb3f7", "getEdgeType @vtable+0xc", "WRONG vftable"),
    ("RBX::AssemblyStage", "both", "size", "", "0x1c", "new 0x1c in MechToAssemblyStage ctor 0x009971d0", "0x1c", "OK"),
    ("RBX::Verb", "both", "", "", "fields read at +0xc and +0x18 where ours reads +0x8/+0x14 (one extra word before them)", "DataModel::close 0x0062d7b0, toggleToolsOff 0x0062da7c/0x0062da89", "", "WRONG (side finding)"),
]


def demangle(n):
    if not n.startswith("__Z"):
        return n
    try:
        return subprocess.run(["c++filt", "-_", n], capture_output=True, text=True).stdout.strip() or n
    except OSError:
        return n


# ---------------------------------------------------------------- tracing accesses through `this`

def trace(img, fn, md, this_reg="ecx"):
    """Linear walk of a function's chunks. Returns [(insn_ea, kind, offset, width, detail)] where kind is
    'w' (store), 'r' (load), 'call' (thiscall on this+offset), 'arg' (this+offset pushed before a call).
    Register offsets from `this` and simple constants are tracked; calls clobber eax/ecx/edx."""
    ranges = fn.get("ranges") or [[fn["address"], fn["address"] + fn["size"]]]
    off = {this_reg: 0}
    const = {}
    pushed = []
    out = []
    for lo, hi in ranges:
        for ins in md.disasm(img.read(lo, hi - lo), lo):
            ops = ins.operands
            m = ins.mnemonic
            # memory accesses through a this-derived register
            for i, op in enumerate(ops):
                if op.type == x86.X86_OP_MEM and op.mem.base and op.mem.index == 0:
                    b = ins.reg_name(op.mem.base)
                    if b in off and m != "lea":
                        o = off[b] + op.mem.disp
                        write = i == 0 and m not in ("cmp", "test", "push") and len(ops) >= 1
                        if m in ("fld", "fild", "push", "movss", "movaps", "movups", "movq", "movd") and i == 0 and len(ops) == 1:
                            write = False
                        if m in ("movss", "movaps", "movups", "movq", "movd", "movlps", "movhps") and i == 1:
                            write = False
                        val = ""
                        if write and len(ops) == 2:
                            s = ops[1]
                            if s.type == x86.X86_OP_IMM:
                                val = s.imm & 0xFFFFFFFF
                            elif s.type == x86.X86_OP_REG:
                                rn = ins.reg_name(s.reg)
                                val = const.get(rn, rn) if rn not in off else f"this+0x{off[rn]:x}"
                        out.append((ins.address, "w" if write else "r", o, op.size, val, m))
            # register bookkeeping
            if m == "call":
                tgt = ops[0].imm if ops and ops[0].type == x86.X86_OP_IMM else None
                if "ecx" in off:
                    out.append((ins.address, "call", off["ecx"], 0, tgt, m))
                for p in pushed[-4:]:
                    out.append((ins.address, "arg", p, 0, tgt, m))
                pushed = []
                for r in ("eax", "ecx", "edx"):
                    off.pop(r, None)
                    const.pop(r, None)
                continue
            if m == "push" and ops and ops[0].type == x86.X86_OP_REG and ins.reg_name(ops[0].reg) in off:
                pushed.append(off[ins.reg_name(ops[0].reg)])
            if m in ("ret", "retn"):
                continue
            if not ops or ops[0].type != x86.X86_OP_REG:
                continue
            d = ins.reg_name(ops[0].reg)
            if m == "mov" and len(ops) == 2 and ops[1].type == x86.X86_OP_REG and ins.reg_name(ops[1].reg) in off:
                off[d] = off[ins.reg_name(ops[1].reg)]
                const.pop(d, None)
            elif m == "lea" and ops[1].mem.base and ins.reg_name(ops[1].mem.base) in off and ops[1].mem.index == 0:
                off[d] = off[ins.reg_name(ops[1].mem.base)] + ops[1].mem.disp
                const.pop(d, None)
            elif m == "xor" and len(ops) == 2 and ops[1].type == x86.X86_OP_REG and ops[1].reg == ops[0].reg:
                off.pop(d, None)
                const[d] = 0
            elif m == "mov" and len(ops) == 2 and ops[1].type == x86.X86_OP_IMM:
                off.pop(d, None)
                const[d] = ops[1].imm & 0xFFFFFFFF
            elif m == "or" and len(ops) == 2 and ops[1].type == x86.X86_OP_IMM and ops[1].imm in (-1, 0xFFFFFFFF):
                off.pop(d, None)
                const[d] = 0xFFFFFFFF
            elif m == "add" and d in off and len(ops) == 2 and ops[1].type == x86.X86_OP_IMM:
                off[d] += ops[1].imm
            elif m == "sub" and d in off and len(ops) == 2 and ops[1].type == x86.X86_OP_IMM:
                off[d] -= ops[1].imm
            elif m not in ("cmp", "test", "push"):
                (_, w) = ins.regs_access()
                for r in w:
                    rn = ins.reg_name(r)
                    off.pop(rn, None)
                    const.pop(rn, None)
    return out


def alloc_sizes(img, md, funcs, callers, names, ctor):
    """[(caller, size, alloc_name)] for `push N ... call alloc ... call ctor` within 24 insns."""
    res = []
    for c in sorted(callers.get(ctor, ())):
        fn = funcs[c]
        ranges = fn.get("ranges") or [[c, c + fn["size"]]]
        insns = []
        for lo, hi in ranges:
            insns += list(md.disasm(img.read(lo, hi - lo), lo))
        for i, ins in enumerate(insns):
            if ins.mnemonic == "call" and ins.operands[0].type == x86.X86_OP_IMM and ins.operands[0].imm == ctor:
                for j in range(i - 1, max(0, i - 24), -1):
                    p = insns[j]
                    if p.mnemonic != "call":
                        continue
                    tgt = p.operands[0]
                    tname = ""
                    if tgt.type == x86.X86_OP_IMM:
                        tname = names.get(tgt.imm, ("sub_%x" % tgt.imm, ""))[0]
                    elif tgt.type == x86.X86_OP_MEM and tgt.mem.base == 0:
                        tname = names.get(tgt.mem.disp & 0xFFFFFFFF, ("[%x]" % (tgt.mem.disp & 0xFFFFFFFF), ""))[0]
                    # the size push sits up to 3 insns before the alloc call
                    for q in insns[max(0, j - 3):j][::-1]:
                        if q.mnemonic == "push" and q.operands[0].type == x86.X86_OP_IMM and (ALLOC_RE.search(tname) or tgt.type == x86.X86_OP_IMM and tgt.imm in EXTRA_ALLOC):
                            res.append((c, q.operands[0].imm, tname))
                            break
                    break
    return res


# ---------------------------------------------------------------- our layout from the PDB

def pdb_types(target, pdb):
    from reccmp.cvdump.parser import CvdumpParser
    from reccmp.cvdump.runner import iter_cvdump_sections
    os.makedirs(CACHE, exist_ok=True)
    dump = os.path.join(CACHE, f"{target}-types.txt")
    if not os.path.exists(dump) or os.path.getmtime(dump) < os.path.getmtime(pdb):
        import reccmp.bin
        cv = reccmp.bin.lib_path_join("cvdump.exe")
        win = "Z:" + os.path.abspath(pdb).replace("/", "\\")
        with open(dump, "w") as f:
            subprocess.run(["wine", cv, "-t", win], stdout=f, stderr=subprocess.DEVNULL, check=True)
    p = CvdumpParser()
    for n, s in iter_cvdump_sections(open(dump, encoding="utf-8", errors="ignore")):
        p.read_section(n, s)
    t = p.types
    byname = {}
    for k, (leaf, lt) in t._raw.items():
        if lt in ("LF_CLASS", "LF_STRUCTURE") and "Size = 0," not in leaf:
            m = re.search(r"class name = (.*?), (?:unique name|UDT)", leaf)
            if m:
                byname.setdefault(m.group(1), k)
    return t, byname


def msvc_name(rtti):
    """RTTI demangled spelling -> the cvdump spelling (no 'class '/'struct ' inside templates)."""
    return re.sub(r"\b(class|struct|enum) ", "", rtti).replace(" >", ">")


BYNAME = {}


def tsize(t, k):
    """Size of a type, following a forward reference to the defined class of the same name."""
    i = t.get(k)
    if not i.size and i.name in BYNAME:
        return t.get(BYNAME[i.name]).size or 0, i.name
    return i.size or 0, i.name or ""


def flatten(t, key, base=0, path=""):
    out = []
    info = t.get(key)
    try:
        ci = t.class_info(key)
        bases = t.base_classes(key)
    except Exception:
        return out
    has_primary_vf = any(o == 0 and t.class_info(b).has_vftable for b, o in bases.items() if _is_class(t, b))
    if ci.has_vftable and not has_primary_vf:
        out.append((base, 4, f"{path}{info.name}::vfptr"))
    for b, o in sorted(bases.items(), key=lambda x: x[1]):
        bsz, bn = tsize(t, b)
        if not t.get(b).size:
            b = BYNAME.get(bn, b)
        out.append((base + o, bsz, f"{path}[base {bn}]"))
        out += flatten(t, b, base + o, path)
    for m in t.members(key):
        try:
            sz, tn = tsize(t, m.type)
        except Exception:
            sz, tn = 0, ""
        out.append((base + m.offset, sz, f"{path}{info.name.split('::')[-1]}.{m.name}" + (f" ({tn})" if tn else "")))
    return out


def _is_class(t, k):
    try:
        t.class_info(k)
        return True
    except Exception:
        return False


def covering(layout, o):
    """The innermost non-base member of ours that covers offset o, or the vfptr at o."""
    best = None
    for mo, sz, n in layout:
        if "[base" in n:
            continue
        if mo == o and (sz == 0 or "vfptr" in n):
            return f"{n}@0x{mo:x}"
        if mo <= o < mo + max(sz, 1):
            if best is None or mo >= best[0]:
                best = (mo, sz, n)
    if best is None:
        return ""
    mo, sz, n = best
    return f"{n}@0x{mo:x}" + (f"+0x{o - mo:x}" if o != mo else "") + f" size 0x{sz:x}"


# ---------------------------------------------------------------- sweep scores

def sweep_scores(target):
    s = {}
    p = os.path.join(ROOT, "notes", "app-sweep.csv")
    for r in csv.DictReader(open(p)):
        if r["target"] == target:
            s[int(r["address"], 16)] = (r["marker"], float(r["score"] or 0), r["function"])
    return s


def fmt(v, names, strings):
    if isinstance(v, int):
        n = names.get(v)
        if n:
            return f"0x{v:x} {n[0][:90]}"
        if v in strings:
            return f"0x{v:x} \"{strings[v][:40]}\""
        return f"0x{v:x}"
    return str(v)


def analyse(cls, target, ctx, extra, rows):
    img, md, names, funcs, callers, vt, strings, ios, types, byname, scores = ctx
    print(f"\n==================== {cls}  [{target}]")
    d = vt.get(cls)
    vtaddrs = {}
    if d:
        print("RTTI bases (offset):", ", ".join(f"{b['class']}@{b['mdisp']}" for b in d["hierarchy"]["bases"][1:]))
        ig = ios.get(cls.replace("class ", "").replace("struct ", ""))
        igroups = ig["groups"] if ig else []
        for gi, g in enumerate(d["groups"]):
            vtaddrs[g["address_point"]] = g["col_offset"]
            print(f"vftable 0x{g['address_point']:08x} for {g['for'] or [cls]} at this+{g['col_offset']}: {len(g['slots'])} slots"
                  + (f"; iOS group {gi} has {len(ios_msvc_slots(igroups[gi]))} after D1/D0 collapse" if gi < len(igroups) else ""))
            islots = ios_msvc_slots(igroups[gi], igroups if gi == 0 else (), len(g["slots"])) if gi < len(igroups) else []
            if gi < len(igroups) and len(islots) != len(g["slots"]):
                print(f"   ! slot count differs after alignment: MSVC {len(g['slots'])} vs iOS {len(islots)}")
            for i, s in enumerate(g["slots"]):
                n = names.get(s["ea"], (s["name"], ""))
                io = demangle(islots[i]) if i < len(islots) else ""
                print(f"   +0x{i * 4:02x} 0x{s['ea']:08x} {n[0][:70]:70} | {io[:80]}")
            rows.append([cls, target, f"vft@{g['col_offset']}", 4, f"vftable 0x{g['address_point']:08x}, {len(g['slots'])} slots", f"0x{g['address_point']:08x}", "", ""])
        for b in d["hierarchy"]["bases"][1:]:
            rows.append([cls, target, f"0x{b['mdisp']:x}", "", f"base {b['class']}", "rtti", "", ""])
    else:
        print("no RTTI vftable")
    # ctor/dtor candidates: functions whose globals hold one of the vftable address points
    fns = set(extra)
    for a, fn in funcs.items():
        if any(g[0] in vtaddrs for g in fn.get("globals", [])):
            fns.add(a)
    key = byname.get(msvc_name(cls))
    layout = flatten(types, key) if key is not None else []
    if key is not None:
        print(f"ours: size 0x{types.get(key).size:x}")
        rows.append([cls, target, "size", "", "our sizeof", "pdb", f"0x{types.get(key).size:x}", ""])
    short = re.sub(r"<.*", "", cls).split("::")[-1]
    own = re.compile(rf"(^|::){re.escape(short)}(<[^()]*>)?::~?{re.escape(short)}\(")
    named = {a for a in fns if a in extra or own.search(names.get(a, ("", ""))[0])}
    others = sorted(fns - named)
    if len(others) > MAX_OTHER:
        print(f"({len(others)} other functions store these vftables; showing {MAX_OTHER}: inlined ctors/dtors of derived classes)")
    for a in sorted(named) + others[:MAX_OTHER]:
        fn = funcs.get(a)
        if fn is None:
            continue
        nm = names.get(a, (fn["name"], ""))
        sc = scores.get(a)
        print(f"-- 0x{a:08x} {nm[0][:100]} [{nm[1]}] size {fn['size']}" + (f"  sweep {sc[0]} {sc[1]}%" if sc else ""))
        acc = trace(img, fn, md)
        called = {ea for ea, kind, *_ in acc if kind == "call"}
        for ea, kind, o, w, v, m in acc:
            if kind == "arg" and ea in called:
                continue
            if kind in ("call", "arg"):
                what = f"{kind} {fmt(v, names, strings)}" if v is not None else kind
            else:
                if kind == "r" and a not in extra:
                    continue
                if isinstance(v, int) and v in vtaddrs:
                    what = f"vfptr <- {fmt(v, names, strings)}"
                else:
                    what = f"{kind}{w * 8 if w else ''} {m} {fmt(v, names, strings)}".rstrip()
            cov = covering(layout, o) if layout else ""
            print(f"   0x{ea:08x} this+0x{o:x}: {what[:120]}   ours: {cov[:90]}")
            proven = "proven(100%)" if sc and sc[1] >= 100 else (f"observed; fn at {sc[1]}%" if sc else "observed")
            rows.append([cls, target, f"0x{o:x}", w, what[:160], f"0x{ea:08x} in 0x{a:08x} {nm[0][:60]}", cov[:120], proven])
        for c, size, an in alloc_sizes(img, md, funcs, callers, names, a):
            print(f"   new 0x{size:x} via {an[:40]} in 0x{c:08x} {names.get(c, ('', ''))[0][:60]}")
            rows.append([cls, target, "size", "", f"operator new 0x{size:x} ({an[:40]})", f"0x{c:08x} -> 0x{a:08x}", f"0x{types.get(key).size:x}" if key is not None else "", ""])


def dispmap(json_path, prefix):
    """From a reccmp --json, pair each original displacement with ours where an orig/recomp line differs
    only in a [reg+disp] (reg not esp/ebp) or add/sub reg,imm. Returns {(orig, ours): set(func addrs)}."""
    pat = re.compile(r"\[(\w+) \+ (0x[0-9a-f]+)\]|(?:add|sub) (\w+), (0x[0-9a-f]+)")
    out = defaultdict(set)
    for e in json.load(open(json_path))["data"]:
        if not e["name"].startswith(prefix):
            continue
        for _, blocks in e.get("diff", []):
            for b in blocks:
                o, r = b.get("orig", []), b.get("recomp", [])
                if "both" in b or len(o) != len(r):
                    continue
                for lo, lr in zip(o, r):
                    if pat.sub("X", lo[1]) != pat.sub("X", lr[1]):
                        continue
                    for a, c in zip(pat.findall(lo[1]), pat.findall(lr[1])):
                        reg, vo, vr = a[0] or a[2], a[1] or a[3], c[1] or c[3]
                        if reg not in ("esp", "ebp") and vo != vr:
                            out[(int(vo, 16), int(vr, 16))].add(e["address"])
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("classes", nargs="*")
    ap.add_argument("--target", default="PLAYER")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--fn", action="append", default=[])
    ap.add_argument("--csv")
    ap.add_argument("--pdb")
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--findings", action="store_true", help="append the hand-read FINDINGS rows to --csv")
    ap.add_argument("--dispmap", nargs=2, metavar=("JSON", "PREFIX"),
                    help="print orig->ours displacement pairs from a reccmp --json for functions named PREFIX*")
    a = ap.parse_args()
    if a.dispmap:
        for (vo, vr), fs in sorted(dispmap(*a.dispmap).items()):
            print(f"orig 0x{vo:x} <- ours 0x{vr:x}  x{len(fs)}  {' '.join(sorted(fs)[:5])}")
        return
    t = a.target
    img = Image(os.path.join(ROOT, BIN[t]))
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    names, funcs, callers, vt, strings = load_target(t)
    ios = load_ios(t)
    types, byname = pdb_types(t, a.pdb or os.path.join(ROOT, PDB[t]))
    BYNAME.update(byname)
    scores = sweep_scores(t)
    ctx = (img, md, names, funcs, callers, vt, strings, ios, types, byname, scores)
    if a.selftest:
        acc = trace(img, funcs[0x620ED0], md)
        assert any(k == "w" and o == 0 for _, k, o, *_ in acc), "Instance ctor should store its vfptr at +0"
        print("selftest ok")
        return
    extra = defaultdict(set)
    for f in a.fn:
        addr, _, owner = f.partition(":")
        extra[owner].add(int(addr, 16))
    rows = []
    for c in (CLASSES if a.all else []) + a.classes:
        analyse(c, t, ctx, extra.get(c, set()) | extra.get("", set()), rows)
    if a.csv:
        new = not os.path.exists(a.csv)
        with open(a.csv, "a", newline="") as f:
            w = csv.writer(f)
            if new:
                w.writerow(["class", "target", "offset", "width", "what", "evidence", "header_now", "verdict"])
            w.writerows(rows)
            if a.findings:
                w.writerows([list(r) for r in FINDINGS])


if __name__ == "__main__":
    main()
