"""Plan the reccmp-proven names (and the hand-checked Phase 4 facts) for a Windows database. Offline: no IDA.

    python3 -P tools/ida/match_names.py PLAYER --json build/sweep2/P-final3.json --exe build/RobloxPlayerBeta.exe
    python3 -P tools/ida/match_names.py STUDIO --json build/sweep2/S-final3.json --exe build-studio/RobloxStudioBeta.exe
    python3 -P tools/ida/match_names.py --selftest

--exe is the build the --json run scored (its .pdb and .map beside it; the PDB view is cached as
<exe>.pdbview.tsv by tools/ida/gen_reccmp_csv.pdb_view, which runs tools/lib/pdb_view.py when missing).
Inputs: ida/exports/<T>-functions.csv, -names.csv, -lib-resolved.csv, STUDIO-PLAYER-bodymatch.csv (for the
STUDIO addresses of the hand facts). Outputs (read by tools/ida/apply_db_updates.py):
  ida/exports/<T>-db-updates.csv     address,action,name,tag,line
      action rename: set `name`; comment: write `line` only; unname: give the address back its dummy name.
      `line` is the repeatable-comment line, "p4 <tag> ..." (no other apply_*.py script strips a "p4 " line).
  ida/exports/<T>-match-conflicts.csv  every reccmp 100% function whose existing oracle (ios:/mac:) or FLIRT
      name disagrees with our recompiled name, with the verdict and the evidence that decided it.
STUDIO also takes PLAYER's reccmp-proven renames (run PLAYER first) on bodies STUDIO-PLAYER-bodymatch.csv
pairs uniquely, where STUDIO still has a dummy name and its own --json run did not decide the address.
Vftable slots (phase4-core3 sync): a dummy original function that a type-5 entry's "both" rows pair with one
of our slot functions in every vftable listing it (no disagreeing "orig" row, body over 8 bytes, not folded in
our build) is renamed after it, tag rtti:. Entries of type 10 (vtordisp thunks) count like functions.
Rules (step 3 of the Phase 4 sync): an entry of the --json run of type function at effective 100% pairs
original address A with our function R. Our name at R is its PUBLICS (decorated) name, or the PDB proc name
for a static; every map symbol at R counts (our linker folds identical bodies). Addresses the library
matchers own (lib-resolved identical/located) are left to tools/ida/apply_lib_names.py. Then:
  IDA name is a dummy                          -> rename, tag match:
  IDA name is one of our names at R            -> comment
  same signature (gen_reccmp_csv.norm_sig)     -> comment (the oracle spelling is kept)
  prior:/ida: name, other signature            -> rename (the 100% body outranks a prior or auto name)
  ios:/mac:/flirt:, other signature            -> conflict, listed in <T>-match-conflicts.csv and resolved by
                                                  resolve(): the oracle name stays when the original folded
                                                  bodies (rtti-slot "folded" evidence, a body of 8 bytes or
                                                  less, or our build defines the oracle signature elsewhere)
                                                  or the oracle evidence is strong (string, rtti slot); our
                                                  name replaces it (old one kept as a "superseded" line)
                                                  when only callgraph/body-match evidence placed it, and
                                                  always over a FLIRT name except the entry point
"""
import argparse
import csv
import json
import os
import re
import sys
from collections import Counter, defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools", "ida"))
import gen_reccmp_csv as G  # noqa: E402  (norm_sig, undname, read_map, pdb_view)

EXP = os.path.join(ROOT, "ida", "exports")
IDA_BAD = re.compile(r"[^A-Za-z0-9_:?@$.]")
DUMMY = re.compile(r"^(?:j_)*(?:sub|nullsub|unknown_libname)_[0-9A-Fa-f]+$")
REPORT = "notes/runs/2026-11-12-phase4-pairing"

# Phase 4 facts (phase4-core.md "Database updates to make", g3d-2013.md and pairing.md unresolved items).
# (PLAYER address, STUDIO address or None = find through STUDIO-PLAYER-bodymatch.csv, action, name, tag, evidence)
# Facts at 100% in the --json run are left to the bulk rule unless they need more than a rename.
FACTS = [
    (0x004022d0, None, "rename", "??0?$atomic@H@rbx@@QAE@XZ", "str:",
     "rbx::atomic<int>::atomic(): stores 0 to v, then the rbx/atomic.h assert '(((long)(&(this->v))) & 0x3) == 0' "
     "(0x00bd7a60/0x00bd7aa8); out of line, our Client/Base/include/rbx/atomic.h, reccmp 72% (phase4-core.md)"),
    (0x0070cf60, None, "rename", "_ZN3RBX16DataModelArbiterC2Ev", "prior:",
     "RBX::DataModelArbiter::DataModelArbiter(); STUB in Client/App/v8datamodel/DataModelJob.cpp, reccmp 77%, "
     "size 0xa20 from the layout (phase4-core.md)"),
    (0x0063ee60, None, "rename", "_ZN3RBX9DataModel10GenericJobC2EN5boost10shared_ptrIS0_EEPKcNS_12DataModelJob8TaskTypeE", "prior:",
     "RBX::DataModel::GenericJob::GenericJob; STUB in Client/App/v8datamodel/DataModel.cpp, reccmp 50%, "
     "new 0x248 three times in doDataModelSetup (phase4-core.md)"),
    (0x009b8f00, None, "rename", "_ZN3RBX23RbxInterlockedDecrementEPVl", "ios:",
     "RBX::RbxInterlockedDecrement(long volatile*) | iOS 1029 symbol; body = push arg, call kernel32 "
     "InterlockedDecrement (IAT 0x00aa1124), ret; reccmp 40% (import thunk)"),
    (0x009b8f10, None, "rename", "_ZN3RBX23RbxInterlockedIncrementEPVl", "ios:",
     "RBX::RbxInterlockedIncrement(long volatile*) | iOS 1029 symbol (next to Decrement there too); body = push "
     "arg, call kernel32 InterlockedIncrement (IAT 0x00aa123c), ret; called by the Diagnostics::Countable ctor "
     "in Instance() 0x00620efc"),
    (0x0061bbd0, None, "comment", "", "guess:",
     "the Instance allocator (phase4-core.md): calls 0x0061b100, throws std::bad_alloc on null, subtracts the size "
     "from the counter at 0x00de31d4; no source names it"),
    (0x0066ee10, 0x00773760, "rename", "?raiseEventInvocation@EventSource@Reflection@RBX@@UAEXABVEventDescriptor@23@ABV?$vector@VVariant@Reflection@RBX@@V?$allocator@VVariant@Reflection@RBX@@@std@@@std@@PBVSystemAddress@3@@Z",
     "rtti:", "EventSource vftable slot 2 (raiseEventInvocation), phase4-core.md; the body is `ret 0Ch`, ICF-folded "
     "with RakNet::CCRakNetSlidingWindow::Update(uint64,bool) and other empty 12-byte-argument methods"),
    (0x009dab20, None, "rename", "?maxFinite@Vector3@G3D@@SAABV12@XZ", "match:",
     "G3D::Vector3::maxFinite() | G3D 8.00-roblox identical (tools/lib/g3d_match.py; lib-resolved has it as a "
     "conflict only because of the ios: name): three stores of FLT_MAX from 0x00b06308, 53 calls; not "
     "RBX::Extents::vv (g3d-2013.md, pairing.md)"),
    (0x009da9e0, None, "rename", "?unitY@Vector3@G3D@@SAABV12@XZ", "match:",
     "G3D::Vector3::unitY() | G3D 8.00-roblox identical, not CoordinateFrame() (g3d-2013.md)"),
    (0x009daa20, None, "rename", "?unitZ@Vector3@G3D@@SAABV12@XZ", "match:",
     "G3D::Vector3::unitZ() | G3D 8.00-roblox identical, not Vector3::ignore (g3d-2013.md)"),
    (0x009d7c30, None, "rename", "?gray@Color3@G3D@@SAABV12@XZ", "match:",
     "G3D::Color3::gray() | G3D 8.00-roblox identical: loads 0.7f, not Color3::one (g3d-2013.md)"),
    (0x007681b0, None, "unname", "", "lib:",
     "not G3D::Color4::one (that name was wrong): stores 0.6f to all four channels (g3d-2013.md); unplaced"),
    (0x009df4e0, None, "rename",
     "?collisionLocationForMovingPointFixedAABox@CollisionDetection@G3D@@SA_NABVVector3@2@0ABVAABox@2@AAV32@AA_N2@Z",
     "match:", "G3D::CollisionDetection::collisionLocationForMovingPointFixedAABox | G3D 8.00-roblox identical; "
     "not rayAABox (g3d-2013.md)"),
    (0x004a2e90, None, "rename", "?toObjectSpace@CoordinateFrame@G3D@@QBE?AV12@ABV12@@Z", "lib:",
     "G3D::CoordinateFrame::toObjectSpace(const CoordinateFrame&) | G3D 8.00-roblox located, not identical: "
     "inline copy in App code computing inverse() * c, calls CoordinateFrame::operator* 0x0048f920; not "
     "toObjectSpace(Box) (g3d-2013.md)"),
    (0x004a2dc0, 0x00532180, "rename", "?inverse@CoordinateFrame@G3D@@QBE?AV12@XZ", "lib:",
     "G3D::CoordinateFrame::inverse() | G3D 8.00-roblox located, not identical: inline copy in App code, "
     "93% against ours when paired (g3d-2013.md)"),
    # phase4-core2 (notes/runs/2026-11-12-phase4-core2/core-datamodel.md "IDA updates to make"); STUDIO addresses
    # from STUDIO-PLAYER-bodymatch.csv pairs and, for the bodies it leaves open, the same DataModel ctor/dtor
    # call order, the same vftable stores or the jobId offset 4 bytes lower (checked 2026-11-12 with capstone).
    (0x0064ec50, 0x0076b720, "rename", "_ZN3RBX9DataModelC2EPNS_4VerbEPS0_", "ios:",
     "RBX::DataModel::DataModel(RBX::Verb*, RBX::DataModel*) | iOS 1029/1021 symbol; operator new(0xe10) then this "
     "call (PLAYER 0x0064f760, STUDIO 0x0076c210); stores the DataModel vftables (PLAYER 0x00b2ffec, STUDIO 0x00bf607c)"),
    (0x0064d850, 0x0076a340, "rename", "_ZN3RBX9DataModelD2Ev", "ios:",
     "RBX::DataModel::~DataModel() | iOS 1029/1021 symbol; the only other function storing the DataModel vftables "
     "(PLAYER 0x00b2ffec, STUDIO 0x00bf607c), same size (1436 bytes) in both builds"),
    (0x00641bd0, 0x0075f3c0, "rename", "_ZN3RBX9DataModel10MouseStatsC2Ev", "ios:",
     "RBX::DataModel::MouseStats::MouseStats() | iOS/Mac symbol; the DataModel ctor calls it on this+0xaf0 (PLAYER "
     "0x0064ed9b, STUDIO 0x0076b86b): two RunningAverageTimeInterval<>(0.5), size 0x80 (core-datamodel.md)"),
    (0x00644f30, 0x00761ef0, "rename", "_ZN3RBX10GuiBuilderC2Ev", "prior:",
     "RBX::GuiBuilder::GuiBuilder() | no oracle symbol (inlined there); the DataModel ctor calls it on this+0xdec, "
     "the guiBuilder member (PLAYER 0x0064f047, STUDIO 0x0076bb11); no stack argument (plain ret): zeros +8/+0xc, "
     "map at +0x10; the dataModel pointer is set later by Initialize (core-datamodel.md)"),
    (0x0056b810, 0x005d0cc0, "rename", "_ZN3RBX10GuiBuilderD2Ev", "prior:",
     "RBX::GuiBuilder::~GuiBuilder() | no oracle symbol; ~DataModel calls it on this+0xdec (PLAYER 0x0064d8e9, "
     "STUDIO 0x0076a3d9); also called from Network::Players::createLocalPlayer 0x0056dd48 (a local GuiBuilder or "
     "an identical folded dtor, not checked)"),
    (0x0062d240, 0x0074d560, "rename", "_ZNK3RBX9DataModel19getIsPersonalServerEv", "ios:",
     "RBX::DataModel::getIsPersonalServer() const | iOS/Mac symbol; returns the byte at 0xc59 (isPersonalServer in "
     "the 2013 layout, core-datamodel.md); the setter follows at +0x10"),
    (0x0062d250, 0x0074d570, "rename", "_ZN3RBX9DataModel19setIsPersonalServerEb", "ios:",
     "RBX::DataModel::setIsPersonalServer(bool) | iOS/Mac symbol; stores its argument to 0xc59 (isPersonalServer), "
     "ret 4"),
    (0x0062d1e0, 0x0074d500, "rename", "_ZNK3RBX9DataModel8getJobIdEv", "ios:",
     "RBX::DataModel::getJobId() const | iOS/Mac symbol; returns a copy of the std::string at 0xc84 (PLAYER) / "
     "0xc80 (STUDIO, no networkStatsWindowsOn): jobId in the 2013 layout (core-datamodel.md)"),
    (0x0068b1a0, 0x007f0900, "rename", "_ZN3RBX13ScriptContextD2Ev", "ios:",
     "RBX::ScriptContext::~ScriptContext() | iOS symbol; the dtor body the 0x288 layout was read from (every member "
     "offset confirmed by a probe compile, core-datamodel.md); called by the deleting dtor 0x0068b950"),
    (0x006f2fb0, 0x00807fb0, "rename", "_ZN3RBX10Soundscape12SoundChannelC2Ev", "ios:",
     "RBX::Soundscape::SoundChannel::SoundChannel() | iOS symbol; Diagnostics::Countable<SoundChannel> increment "
     "through RbxInterlockedIncrement at 0x006f2ffc (core-datamodel.md)"),
    (0x0084d2f0, 0x009a4fe0, "rename", "_ZN3RBX19PhysicsInstructionsC2Ev", "ios:",
     "RBX::PhysicsInstructions::PhysicsInstructions() | iOS symbol; the DataModel ctor calls it on this+0xbe8, the "
     "PhysicsInstructions member (PLAYER 0x0064ee60, STUDIO 0x0076b930), its only caller"),
    (0x00619a70, 0x006ee740, "comment", "", "ios:",
     "FWValue<std::string>::set(const std::string&, FWRef*) confirmed by the 2013 FWValue<T> member set() "
     "(FWValue<bool>::set 0x0052c000 has the same shape; core-datamodel.md)"),
    (0x005c30c0, 0x008a8950, "comment", "", "match:",
     "RakNet::BitStream::SetWriteOffset(unsigned int) | ICF-folded with RBX::GuiBuilder::Initialize(RBX::DataModel*) (iOS _ZN3RBX10GuiBuilder10InitializeEPNS_9DataModelE): "
     "the 9-byte body `mov eax,[esp+4]; mov [ecx],eax; ret 4` is both; callers: the DataModel ctor (PLAYER 0x0064f1ce, "
     "STUDIO 0x0076bc98) as Initialize, RakPeer::ShiftIncomingTimestamp and two RakNet functions as SetWriteOffset; "
     "the address lies in the RakNet .obj block, so the RakNet name stays and Initialize is FOLDED"),
    (0x009bc7d0, 0x00a72bc0, "comment", "", "match:",
     "the 'register a watched value' call of setPlaceID (core-datamodel.md) is RbxDbgInfo::AddPlace(placeID)"),
    (0x009bc810, 0x00a72c00, "rename", "_ZN3RBX10RbxDbgInfo11RemovePlaceEl", "ios:",
     "RBX::RbxDbgInfo::RemovePlace(long) | iOS symbol; body = --PlaceCounter (0x00e1b9d4), find the ID in the 4 "
     "PlaceIDs (0x00e1b9c4), shift the rest down; next to AddPlace 0x009bc7d0 (reccmp 100%); ~DataModel calls it "
     "on placeID (0x0064d8d4), the 'unregister a watched value' of core-datamodel.md"),
    # phase4-core3 (notes/runs/2026-11-12-phase4-core3/core-bodies.md "For the next IDA sync", vtables-3.md);
    # STUDIO addresses from STUDIO-PLAYER-bodymatch.csv pairs or the STUDIO DataModel ctor 0x0076b720 operands,
    # each checked with capstone 2026-11-13. RbxInterlockedExchange/CompareExchange, getPrimaryPart and the
    # getLocation vtordisp thunks are reccmp 100% and come through the bulk rule.
    (0x009bb680, 0x00a724b0, "rename", "_ZN3RBX4Time3nowILNS0_12SampleMethodE0EEES0_v", "ios:",
     "RBX::Time::now<(SampleMethod)0>() = now<Fast> | iOS 1029 symbol; `cmp preciseOverride(0x00db4b44),0; jg` "
     "(fast clock, one-time init through RbxInterlockedExchange) else the precise clock 0x009bb400; nowFast "
     "0x009bb4b0 (reccmp 100%) calls it. Our tree's CSV row calls it now<2> (wrong 2013 shape, core-bodies.md)"),
    (0x009bb6e0, 0x00a72510, "rename", "_ZN3RBX4Time3nowILNS0_12SampleMethodE1EEES0_v", "ios:",
     "RBX::Time::now<(SampleMethod)1>() = now<Benchmark> | iOS 1029 symbol; `cmp preciseOverride,1; jg` -> "
     "now<0> 0x009bb680, else the precise clock 0x009bb400"),
    (0x009bb400, 0x00a72250, "rename", "_ZN3RBX4Time3nowILNS0_12SampleMethodE2EEES0_v", "ios:",
     "RBX::Time::now<(SampleMethod)2>() = now<Precise> | iOS 1029 symbol; QueryPerformanceCounter (IAT "
     "0x00aa1150) minus a once-initialised start, / QueryPerformanceFrequency (0x00aa1154); the precise "
     "branch of now<0> and now<1>"),
    (0x005ba6f0, None, "comment", "", "guess:",
     "a second rbx::atomic-style default ctor: stores 0, then the rbx/atomic.h alignment assert "
     "'(((long)(&(this->v))) & 0x3) == 0' at line 182 (0xb6; rbx::atomic<int>::atomic() 0x004022d0 asserts at "
     "line 224); used only for TaskScheduler +8 (core-bodies.md); which atomic.h class is not known"),
    (0x0061c800, None, "comment", "", "guess:",
     "FWInstance's +0x18 base or member ctor (pool registration: Allocator<FWInstance>-like, counter 0x00de31d4); "
     "the FWInstance ctor 0x0061d7e0 calls it on this+0x18 (core-bodies.md); not modelled in our tree"),
    (0x009b1a20, 0x00917010, "comment", "", "match:",
     "ICF-folded with RBX::Surface::Surface(RBX::PartInstance*, RBX::NormalId) (iOS "
     "_ZN3RBX7SurfaceC1EPNS_12PartInstanceENS_8NormalIdE): the body `mov [ecx],arg1; mov [ecx+4],arg2; ret 8` is "
     "both; PartInstance::getSurface constructs Surfaces through it (vtables-3.md); the RakNet/boost library "
     "name stays, Surface's ctor is FOLDED"),
    # data (action data1/data4 = name a global of that size)
    (0x00de4760, 0x00f83b31, "data1", "_ZN5FFlag11NewLightAPIE", "ios:",
     "FFlag::NewLightAPI (bool) | iOS 1029 symbol; the DataModel ctor tests it (PLAYER 0x0064f05b, STUDIO "
     "0x0076bb25) before call_once(registerNewLightAPI)"),
    (0x00de4761, 0x00f83b32, "data1", "_ZN5FFlag16ImageRectEnabledE", "ios:",
     "FFlag::ImageRectEnabled (bool) | iOS 1029 symbol (next to NewLightAPI there too); the DataModel ctor tests "
     "it (PLAYER 0x0064f090, STUDIO 0x0076bb5a) before call_once(registerNewImageAPI)"),
    (0x00de4770, 0x00f83b40, "data4", "RBX::DataModel::DataModel::newLightAPIFlag", "guess:",
     "boost::once_flag for call_once(registerNewLightAPI) in the DataModel ctor (push at PLAYER 0x0064f083, "
     "STUDIO 0x0076bb4d); a function-local static, no oracle symbol, name from our DataModel.cpp"),
    (0x00de4768, 0x00f83b38, "data4", "RBX::DataModel::DataModel::newImageAPIFlag", "guess:",
     "boost::once_flag for call_once(registerNewImageAPI) in the DataModel ctor (push at PLAYER 0x0064f0b8, "
     "STUDIO 0x0076bb82); a function-local static, no oracle symbol, name from our DataModel.cpp"),
    (0x00de4778, 0x00f83b48, "data4", "RBX::DataModel::DataModel::networkStreamingPropFlag", "guess:",
     "boost::once_flag for call_once(registerNetworkStreamingProp) in the DataModel ctor (push at PLAYER "
     "0x0064f0e5, STUDIO 0x0076bbaf); a function-local static, no oracle symbol, name from our DataModel.cpp"),
    (0x00de4c0c, 0x00f83fd8, "data4", "_ZN3RBX9DataModel5countE", "ios:",
     "RBX::DataModel::count (rbx::atomic<int>) | iOS 1029 symbol; incremented by the DataModel ctor (`mov ecx, "
     "0x00de4c0c` at PLAYER 0x0064f16f, STUDIO 0x0076bc39), decremented by ~DataModel"),
    # wave1 sync (notes/runs/2026-11-13-phase4-core4/core-close.md "IDA NAMES FOR THE NEXT SYNC"; the Allocator,
    # FWBase::init, FWFinal<FWInstance> dtor, Average, vector<double>, checkSpeedHack, isSpeedCheater/isDebugged
    # and startMMTimer rows are reccmp 100% in sweep7 and come through the bulk rule). Checked 2026-11-13 with
    # capstone; STUDIO addresses from STUDIO-PLAYER-bodymatch.csv pairs, Time.cpp's GLOBAL markers, or the
    # same instruction in the paired STUDIO body.
    # The three Stats::Item::formatValue<T> keep rows (0x0073b3f0/0x0073b420/0x0073b450) were dropped in the
    # wave8 sync: the tree now spells them ??$formatValue@N/M/H@Item@Stats@RBX@@ and the iOS names agree by signature.
    (0x009bb130, 0x00a71f80, "rename", "_ZN3RBX8getStartEv", "ios:",
     "RBX::getStart() | iOS 1029 has its function-local static _ZZN3RBX8getStartEvE5start (the function itself "
     "is inlined there); body: guard bit 0x00e1b648, QueryPerformanceCounter (IAT 0x00aa1150) into the static "
     "0x00e1b640, returns its address; called by directCallback/interpolatedCallback and now<2> (reccmp 54%)"),
    (0x009bb180, 0x00a71fd0, "rename", "?directCallback@RBX@@YGXIIKKK@Z", "src2016:",
     "RBX::directCallback (CALLBACK, timeSetEvent) | 2016 Base/rbx/Time.cpp name, Windows-only (no oracle); "
     "2013 body has the 2016 shape: recguard 0x00e1b638 swap through RbxInterlockedExchange, QPC - getStart, "
     "currentSeconds 0x00e1b620 max, checkSpeedHack 0x009bb050, isDebuggedValue 0x00e1b629 (reccmp 52%, "
     "core-close.md: object-identical)"),
    (0x009bb230, 0x00a72080, "rename", "?interpolatedCallback@RBX@@YGXIIKKK@Z", "src2016:",
     "RBX::interpolatedCallback (CALLBACK) | 2016 Base/rbx/Time.cpp name, Windows-only (no oracle); 2013 body: "
     "recguard swap, statics last/secsPerTick (guard 0x00e1b660), mmticks 0x00e1b64c % ticksPerCorrection "
     "0x00e1b608, millisecondsPerTick 0x00e1b610, currentSeconds (reccmp 72%, core-close.md: object-identical)"),
    (0x00845c00, 0x0098b5d0, "rename", "??0?$FWFinal@VFWPartInstance@RBX@@@RBX@@QAE@ABVFWPartInstance@1@@Z", "rtti:",
     "RBX::FWFinal<FWPartInstance>::FWFinal(const FWPartInstance&) | calls the FWPartInstance copy base 0x00845b70, "
     "stores ??_7FWPartInstance 0x00b4c968, copies +0x38..+0x50, then stores ??_7FWFinal<FWPartInstance> "
     "0x00b4cb8c (core-close.md; used by valueUpdate)"),
    (0x007bc3e0, 0x00884ef0, "rename", "_ZN3RBX8InstanceD0Ev", "ios:",
     "RBX::Instance D0 = MSVC ??_GInstance (scalar deleting destructor) | iOS 1029 _ZN3RBX8InstanceD0Ev; body: "
     "~Instance 0x0061d510 (STUDIO 0x006f1f40), operator delete if (flags & 1), ret 4; target of the adjustor "
     "thunks 0x007bc400 (-0x1c) and 0x00958d00 (-0x18) (STUDIO 0x00884f10, 0x00669590). The client-shell LTCG "
     "copy of the same body is PLAYER 0x0040b010 / STUDIO 0x00477e20 (core-close.md)"),
    (0x007bc400, 0x00884f10, "comment", "", "rtti:",
     "adjustor thunk `sub ecx,0x1c; jmp` to Instance D0 0x007bc3e0 (STUDIO 0x00884ef0): Instance's slot 0 in a "
     "vftable of a base at +0x1c (core-close.md)"),
    (0x00958d00, 0x00669590, "comment", "", "rtti:",
     "adjustor thunk `sub ecx,0x18; jmp` to Instance D0 0x007bc3e0 (STUDIO 0x00884ef0): Instance's slot 0 in a "
     "vftable of a base at +0x18 (core-close.md)"),
    (0x008f90b0, 0x00a0e170, "rename", "_ZN3RBX11IndexedMesh11isUpperRootEPKS0_", "ios:",
     "RBX::IndexedMesh::isUpperRoot(const IndexedMesh*) | iOS symbol (PLAYER name); STUDIO 0x00a0e170 has the same "
     "0x7a bytes (math-5a.md) and was a tail chunk of Clump::isClumpRootPrimitive 0x009d00f0 until "
     "tools/ida/fix_bounds.py made it a function"),
    (0x0040b010, 0x00477e20, "comment", "", "rtti:",
     "the client-shell (LTCG) copy of Instance D0 (same bytes as 0x007bc3e0 / STUDIO 0x00884ef0, which carry the "
     "name); its adjustor thunks are STUDIO 0x00481830 (-0x1c) and 0x0043fc00 (-0x18)"),
    (0x00402240, 0x00443510, "rename", "?__TBB_LockByte@@YAIAAE@Z", "lib:",
     "uintptr_t __TBB_LockByte(unsigned char&) (TBB 2.1 header, COMDAT; returns uintptr_t, not 4.x's unsigned char: "
     "the callers keep `mov ebx,eax ... mov [esi],bl`, Body::getPV_Spin_Lock 0x007554e0; reccmp 100% vs our "
     "?__TBB_LockByte@@YAIAAE@Z in sweep13, settings-serialization.md wave6) | loop: tbb.dll __TBB_machine_trylockbyte thunk "
     "0x009eeea0, pause backoff to 16, then SwitchToThread (IAT 0x00aa1240); the safe_queue lock of "
     "rbx::spin_mutex = tbb::spin_mutex (core-close.md)"),
    (0x00e0c9dc, 0x00f83380, "data4", "_ZN3RBX10Reflection14ClassRegistrarINS_8InstanceEE9registrarE",
     "ios:", "RBX::Reflection::ClassRegistrar<RBX::Instance>::registrar | iOS 1029 symbol; both Instance ctors "
     "`inc dword` it (PLAYER 0x00620e22 / 0x00620f52, STUDIO 0x006f5662) (core-close.md)"),
    (0x00e1b640, 0x00f9e828, "data8", "_ZZN3RBX8getStartEvE5start", "ios:",
     "RBX::getStart()::start (LARGE_INTEGER) | iOS 1029 symbol; written once by getStart 0x009bb130 (STUDIO "
     "0x00a71f80) behind the guard bit at +8"),
    # Time.cpp globals: 2016 names (file statics there, Windows-only so no iOS/Mac symbol), each confirmed by
    # its use in the 2013 bodies; the addresses are Time.cpp's GLOBAL markers (checkSpeedHack 0x009bb050 is 100%)
    (0x00e1b608, 0x00f9e7f0, "data4", "RBX::ticksPerCorrection", "src2016:",
     "ticksPerCorrection (long) | 2016 Time.cpp; interpolatedCallback's mmticks % divisor (0x009bb29f)"),
    (0x00e1b610, 0x00f9e7f8, "data8", "RBX::millisecondsPerTick", "src2016:",
     "millisecondsPerTick (double) | 2016 Time.cpp; interpolatedCallback's secsPerTick initialiser (0x009bb281)"),
    (0x00e1b618, 0x00f9e800, "data8", "RBX::prevSpeedHackCheckpointTime", "src2016:",
     "prevSpeedHackCheckpointTime (LARGE_INTEGER) | 2016 Time.cpp; checkSpeedHack 0x009bb050 (reccmp 100%)"),
    (0x00e1b620, 0x00f9e808, "data8", "RBX::currentSeconds", "src2016:",
     "currentSeconds (volatile double) | 2016 Time.cpp; max-updated by both timer callbacks (0x009bb1e2)"),
    (0x00e1b628, 0x00f9e810, "data1", "RBX::cheater", "src2016:",
     "cheater (volatile bool) | 2016 Time.cpp; set by checkSpeedHack 0x009bb050 (reccmp 100%), read by "
     "Time::isSpeedCheater 0x009bb490"),
    (0x00e1b629, 0x00f9e811, "data1", "RBX::isDebuggedValue", "src2016:",
     "isDebuggedValue (volatile bool) | 2016 Time.cpp; set by directCallback (0x009bb214), read by "
     "Time::isDebugged 0x009bb4a0"),
    (0x00e1b62c, 0x00f9e814, "data4", "RBX::lastSpeedCheckCounter", "src2016:",
     "lastSpeedCheckCounter (long) | 2016 Time.cpp; checkSpeedHack 0x009bb050 (reccmp 100%)"),
    (0x00e1b630, 0x00f9e818, "data4", "RBX::violationsCount", "src2016:",
     "violationsCount (long) | 2016 Time.cpp; checkSpeedHack 0x009bb050 (reccmp 100%)"),
    (0x00e1b634, 0x00f9e81c, "data4", "RBX::prevTickTime", "src2016:",
     "prevTickTime (long) | 2016 Time.cpp; checkSpeedHack 0x009bb050 (reccmp 100%)"),
    (0x00e1b638, 0x00f9e820, "data4", "RBX::recguard", "src2016:",
     "recguard (rbx::atomic<int>) | 2016 Time.cpp; swapped through RbxInterlockedExchange 0x009b8f20 on entry and "
     "exit of both timer callbacks (0x009bb185, 0x009bb238)"),
    # wave2 sync (notes/runs/2026-11-13-phase5-wave2 integrate: Arbiter::getNumPlayers removed, MATCH_FIX,
    # FLog::GuiTargetLifetime). Checked 2026-11-13 with capstone on both targets.
    (0x0063e220, 0x0075c0d0, "rename", "?getNumPlayers@DataModel@RBX@@UBEHXZ", "rtti:",
     "RBX::DataModel::getNumPlayers() const | slot 7 of DataModel's DataModelArbiter-base vftable 0x00b2ff80 "
     "(also 0x00b36d64, 0x00b3715c); body `add ecx,-0xa8` (DataModel from the +0xa8 base), find Players, ret 0 "
     "if none, else tail-jump to Players::getNumPlayers. The 2013 Arbiter has 6 slots (ExclusiveArbiter "
     "0x00bb8af0), so slot 6 is the deleting dtor and slot 7 this; the old rtti: names of the two were swapped"),
    (0x0064de30, 0x0076a900, "rename", "??_EDataModel@RBX@@WKI@AEPAXI@Z", "rtti:",
     "RBX::DataModel `vector deleting destructor' adjustor{168} | `sub ecx,0xa8; jmp` to DataModel's deleting "
     "dtor 0x0064f1f0 (STUDIO 0x0084e250); slot 6 of vftable 0x00b2ff80 (swapped with getNumPlayers before)"),
    (0x008b2ea0, 0x00961180, "rename", "??_EGuiMain@RBX@@WBM@AEPAXI@Z", "match:",
     "RBX::GuiMain `vector deleting destructor' adjustor{28} | reccmp 100% vs our 0x00596af0, where our linker "
     "folds three names (GuiMain, FactoryProduct<GuiMain>, DescribedCreatable<GuiMain>); `sub ecx,0x1c; jmp` to "
     "the next function and slot 0 of both GuiMain vftables, so GuiMain's own (gen_reccmp_csv MATCH_FIX); the "
     "old ScreenGui name came from a stale pairing"),
    (0x00de3f47, 0x00f84e1f, "data1", "_ZN4FLog17GuiTargetLifetimeE", "ios:",
     "FLog::GuiTargetLifetime (FLog channel byte) | iOS 1029 symbol; ~GuiLayerCollector loads it (`mov cl,[...]` "
     "at PLAYER 0x008b439b, STUDIO 0x00988b9b) before FastLog"),
    # wave3 sync (notes/runs/2026-11-14-phase5-wave3: integrate.md / shell-5k.md link stubs, datamodel-5b-3.md
    # getTimeStr). Checked 2026-11-22 with capstone on both targets. The two link-stub rows carry the exact
    # decorated name our tree's unresolved external has, so tools/gen_link_stubs.py's exact lookup finds them.
    (0x009c5ce0, 0x006c20b0, "rename", "??0CRenderSettings@RBX@@QAE@XZ", "rtti:",
     "RBX::CRenderSettings::CRenderSettings() | stores vftable 0x00bbb474 (STUDIO 0x00be712c), whose COL names "
     ".?AVCRenderSettings@RBX@@; GfxBase /Od body (ebp frame); called on this+0x64 by CRenderSettingsItem() "
     "0x0045680c (STUDIO 0x0043771c); iOS 1021/1029 have _ZN3RBX15CRenderSettingsC2Ev"),
    (0x009b59b0, 0x00a68be0, "rename", "?getVideoMemory@SystemUtil@RBX@@YAIXZ", "src2016:",
     "RBX::SystemUtil::getVideoMemory() | 2016 Base/rbx/Win/SystemUtil.cpp name (Windows-only, no oracle); 2013: "
     "a tail `jmp` into the DXVRAM probe 0x009c0c30 (STUDIO 0x00a77ac0, FastLog 'DXVRAM: Already initialized, "
     "returning %d'), called by CRenderSettingsItem() 0x004568bd (STUDIO 0x004377cd), which compares eax only "
     "with 16000000 (32-bit return, shell-5k.md)"),
    (0x00706150, 0x007d1cf0, "rename",
     "?to_simple_string@posix_time@boost@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@"
     "Vtime_duration@12@@Z", "lib:",
     "boost::posix_time::to_simple_string(time_duration) (boost 1.53 header) | free function, no this: copies the "
     "8-byte time_duration argument and calls to_simple_string_type<char> 0x00705df0 (STUDIO 0x007d1990); the "
     "old ios: Lighting::getTimeStr name (callgraph only) belongs to the next function (datamodel-5b-3.md)"),
    (0x00706190, 0x007d1d30, "rename", "_ZNK3RBX8Lighting10getTimeStrEv", "ios:",
     "RBX::Lighting::getTimeStr() const | iOS 1029 symbol; reads this+0x1a0/+0x1a4 (the time-of-day "
     "time_duration) and calls to_simple_string_type<char>; the name sat on to_simple_string 0x00706150 before"),
    # wave4 sync (notes/runs/2026-11-23-phase5-wave4/5c-serialization.md "Util/Name.cpp"). Checked 2026-11-23 with
    # capstone on both targets.
    (0x00613be0, 0x007034b0, "rename", "_ZN3RBX4Name6lookupERKPKc", "ios:",
     "RBX::Name::lookup(char const* const&) | iOS 1029 symbol; static (cdecl): loads the pointer through its "
     "reference argument; null -> call_once(initNullName 0x00613a70) and returns the null Name, else builds a "
     "std::string and calls the string lookup 0x00613ab0 (STUDIO 0x00703380); the last function before "
     "XmlElement.obj (5c-serialization.md)"),
    # wave5 sync (notes/runs/2026-11-23-phase5-wave5: base-blockers.md "IDA owner"; sweep12). Checked 2026-11-23
    # with capstone on both targets.
    # (wave7: the keep row for doDeclare<Stats::sStatsItem> 0x00480e90 / 0x0043f250 is gone: the tree now spells
    # RBX::Stats::sStatsItem (util-target.md "Stats::Item"), so reccmp's 100% name is the 2013 one.)
    (0x00482680, 0x0043f240, "rename", "??$callDoDeclare@$E?sStatsItem@Stats@RBX@@3QBDB@Name@RBX@@SAXXZ", "match:",
     "RBX::Name::callDoDeclare<RBX::Stats::sStatsItem>() | reccmp 100% vs our callDoDeclare<sStatsItem> (sweep12), "
     "a `jmp` to doDeclare 0x00480e90 (STUDIO 0x0043f250); namespace Stats from the 2013 RTTI, as above"),
    (0x0085e310, 0x00948930, "rename", "??_GLuaSettings@RBX@@UAEPAXI@Z", "match:",
     "RBX::LuaSettings::`scalar deleting destructor' | reccmp 100% (sweep12) vs our 0x004b8b20, where our linker folds "
     "it with GlobalAdvancedSettingsItem<LuaSettings>'s ??_G; clears the singleton slot 0x00de68a8 (STUDIO "
     "0x00f89334), reached through the adjustor{28} thunk 0x0085e300 (sub ecx,0x1c; jmp) that slot 0 of both "
     "the LuaSettings and the GlobalAdvancedSettingsItem<LuaSettings> vftables at +0x1c hold (0x00b7d008, "
     "0x00b7cf70); base-blockers.md names it LuaSettings"),
    # wave6 sync (notes/runs/2026-11-23-phase5-wave6/settings-serialization.md "IDA owner"; sweep13). Checked
    # 2026-11-23 with capstone on both targets.
    (0x004c4720, 0x004885a0, "rename", "??0ContentId@RBX@@QAE@PBD@Z", "ios:",
     "RBX::ContentId::ContentId(char const*) | iOS 1029 _ZN3RBX9ContentIdC2EPKc; the twin of ContentId(std::string "
     "const&) 0x004c4780 (match:) except for the import it calls: basic_string(const char*) [0x00aa130c] (STUDIO "
     "[0x00b4e520], shell /GS copy) against basic_string(const string&) [0x00aa12f4]; then Name lookup 0x00613a90 "
     "into +0x1c; ContentProvider::readContent builds it in the return slot (settings-serialization.md)"),
    # wave8 sync: with program_options built /GL, reccmp pairs our windows_file_codecvt deleting dtor with the
    # boost filesystem copy 0x009befb0 (slot 0 of ??_7windows_file_codecvt 0x00bbb00c); the shell copy that held
    # the name since an earlier sync is the program_options facet's (identical body apart from the operator
    # delete call target).
    (0x00474040, None, "rename", "??_Eutf8_codecvt_facet@detail@program_options@boost@@UAEPAXI@Z", "rtti:",
     "boost::program_options::detail::utf8_codecvt_facet::`vector deleting destructor' | slot 0 of the vftable "
     "0x00be64d0 whose COL 0x00c75d38 names .?AUutf8_codecvt_facet@detail@program_options@boost@@ (its slots 2 and "
     "6 are the program_options do_max_length/do_unshift); the windows_file_codecvt name moves to 0x009befb0"),
    # wave9 sync: sweep16 pairs vector<shared_ptr<Instance>>::push_back with 0x00564bc0 at 100%; the Thread.obj
    # copy that held the name since an earlier sweep has no row now (shell-base.md, wave9).
    (0x009b7020, None, "unname", "", "match:",
     "not vector<shared_ptr<Instance>>::push_back (that name is reccmp 100% at 0x00564bc0 in sweep16): the "
     "Thread.obj push_back chain next to 0x009b6af0 (vector<shared_ptr<boost::thread>>), element type unproven "
     "(shell-base.md, wave9)"),
    # wave10 sync (integrate.md, wave10): the boost::throw_exception chain at PLAYER 0x0044d400..0x0044db50 is the
    # rbx::bad_placement_any_cast instance. lib-resolved matched its bodies "identical" to boost-iostreams'
    # std::exception copies (the bodies differ only in vftable and throw-info addresses), so they carried
    # std::exception names. Checked 2026-11-24 from the exe bytes: the throw info and the RTTI COL of every
    # vftable each function stores. STUDIO's chain is 0x004108f0..0x00411090 (0x00411090 is already the iOS name).
    (0x0044d400, 0x00411090, "rename",
     "??$throw_exception@Vbad_placement_any_cast@rbx@@@boost@@YAXABVbad_placement_any_cast@rbx@@@Z", "rtti:",
     "boost::throw_exception<rbx::bad_placement_any_cast> | throws _TI 0x00cd6854 (STUDIO 0x00e295f0) whose "
     "catchable types start .?AV?$clone_impl@U?$error_info_injector@Vbad_placement_any_cast@rbx@@...; not "
     "throw_exception<std::exception> (lib-resolved boost-iostreams body match; wave10 integrate.md)"),
    (0x0044d950, 0x004109f0, "rename",
     "??$enable_error_info@Vbad_placement_any_cast@rbx@@@boost@@YA?AU?$error_info_injector@Vbad_placement_any_cast"
     "@rbx@@@exception_detail@0@ABVbad_placement_any_cast@rbx@@@Z", "rtti:",
     "boost::enable_error_info<rbx::bad_placement_any_cast> | stores the error_info_injector<bad_placement_any_cast> "
     "vftables 0x00b11c70 / 0x00b11c68 (STUDIO 0x00bc97e0), called from throw_exception 0x0044d400"),
    (0x0044d9a0, 0x004109c0, "rename",
     "??$enable_current_exception@U?$error_info_injector@Vbad_placement_any_cast@rbx@@@exception_detail@boost@@@"
     "boost@@YA?AV?$clone_impl@U?$error_info_injector@Vbad_placement_any_cast@rbx@@@exception_detail@boost@@@"
     "exception_detail@0@ABU?$error_info_injector@Vbad_placement_any_cast@rbx@@@20@@Z", "rtti:",
     "boost::enable_current_exception<error_info_injector<rbx::bad_placement_any_cast>> | called from "
     "throw_exception 0x0044d400, calls the clone_impl ctor 0x0044da20 (STUDIO 0x004108f0)"),
    (0x0044da20, 0x004108f0, "rename",
     "??0?$clone_impl@U?$error_info_injector@Vbad_placement_any_cast@rbx@@@exception_detail@boost@@@exception_"
     "detail@boost@@QAE@ABU?$error_info_injector@Vbad_placement_any_cast@rbx@@@12@@Z", "rtti:",
     "clone_impl<error_info_injector<rbx::bad_placement_any_cast>>::clone_impl(const error_info_injector&) | "
     "stores the clone_impl<...bad_placement_any_cast> vftables 0x00b11c94 / 0x00b11c8c / 0x00b11c7c (STUDIO "
     "0x00bc9804 / 0x00bc97f4)"),
    (0x0044db50, 0x00410f30, "rename",
     "??_E?$error_info_injector@Vbad_placement_any_cast@rbx@@@exception_detail@boost@@WM@AEPAXI@Z", "rtti:",
     "error_info_injector<rbx::bad_placement_any_cast>::`vector deleting destructor'`adjustor{12}' | slot 0 of "
     "the vftable 0x00b11c68 (STUDIO 0x00bc97e0, {for boost::exception}), `sub ecx,0xc; jmp` to the D0 0x0044d6c0"),
    # STUDIO only: the same lib-resolved mistake on the std::bad_alloc chain (throws _TI 0x00e296c4, catchable
    # types .?AV?$clone_impl@U?$error_info_injector@Vbad_alloc@std@@...; 0x0041f8d0 stores the
    # error_info_injector<bad_alloc> vftables 0x00bbd6cc / 0x00bbd6c4). PLAYER's bad_alloc chain is named.
    (None, 0x0041fdb0, "rename", "??$throw_exception@Vbad_alloc@std@@@boost@@YAXABVbad_alloc@std@@@Z", "rtti:",
     "boost::throw_exception<std::bad_alloc> | throws _TI 0x00e296c4 (clone_impl<error_info_injector<bad_alloc>>); "
     "not throw_exception<std::exception>"),
    (None, 0x0041f8d0, "rename",
     "??$enable_error_info@Vbad_alloc@std@@@boost@@YA?AU?$error_info_injector@Vbad_alloc@std@@@exception_detail@0@"
     "ABVbad_alloc@std@@@Z", "rtti:",
     "boost::enable_error_info<std::bad_alloc> | stores the error_info_injector<bad_alloc> vftables 0x00bbd6cc / "
     "0x00bbd6c4; called from throw_exception<bad_alloc> 0x0041fdb0"),
    (None, 0x0041f8a0, "rename",
     "??$enable_current_exception@U?$error_info_injector@Vbad_alloc@std@@@exception_detail@boost@@@boost@@YA?AV?$"
     "clone_impl@U?$error_info_injector@Vbad_alloc@std@@@exception_detail@boost@@@exception_detail@0@ABU?$error_"
     "info_injector@Vbad_alloc@std@@@20@@Z", "rtti:",
     "boost::enable_current_exception<error_info_injector<std::bad_alloc>> | called from throw_exception<bad_alloc> "
     "0x0041fdb0"),
    # wave11 sync: shell callees shell-5k-2.md calls with no definition yet (integrate.md wave 11); named with the
    # decorated names our tree declares so tools/gen_link_stubs.py binds them. Checked from the exe bytes;
    # STUDIO addresses from STUDIO-PLAYER-bodymatch.csv pairs or the bytes (hook setters).
    (0x00520070, None, "rename", "??0CrashReporter@@QAE@XZ", "rtti:",
     "CrashReporter::CrashReporter() | stores ??_7CrashReporter@@6B@ 0x00b119e0 (COL .?AVCrashReporter@@), called "
     "by the RobloxCrashReporter ctor 0x0043f570"),
    (0x005200f0, None, "rename", "??1CrashReporter@@QAE@XZ", "rtti:",
     "CrashReporter::~CrashReporter() | stores ??_7CrashReporter@@6B@ 0x00b119e0, FastLog 'Closing crashreporter "
     "thread on destroy', thread::join; non-virtual (slot 0 of the vftable is a folded `ret 4`)"),
    (0x0051fcb0, None, "rename", "?ProcessException@CrashReporter@@UAEJPAU_EXCEPTION_POINTERS@@_N@Z", "rtti:",
     "CrashReporter::ProcessException(_EXCEPTION_POINTERS*, bool) | slot 1 of ??_7CrashReporter@@6B@ 0x00b119e0 "
     "(RobloxCrashReporter overrides it with 0x0043f660); MiniDumpWriteDump, 'MiniDumpWriteDump() failed with hr'"),
    (0x005205a0, None, "rename", "?Start@CrashReporter@@QAEXXZ", "src2016:",
     "CrashReporter::Start() | SetUnhandledExceptionFilter, new boost::thread over bind(mf0<void,CrashReporter>) "
     "(thread_data vftable 0x00b1191c); called once, from the MainLogManager ctor path"),
    (0x0051fad0, None, "rename", "?GenerateDmpFileName@CrashReporter@@QAEJPADH_N1@Z", "src2016:",
     "CrashReporter::GenerateDmpFileName(char*, int, bool, bool) | its two callers are ProcessException 0x0051fcb0 "
     "(vftable slot 1) and CreateFakeCrashDump 0x0043f8c0"),
    (0x009d5d10, None, "rename",
     "?getCommonLogFilename@Log@G3D@@SA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ", "src2016:",
     "G3D::Log::getCommonLogFilename() (G3D 8.00 Log.cpp) | 36 bytes: calls Log::common 0x009d5c60 (ios:) and "
     "copy-constructs its filename string, right after common() in Log.obj"),
    (0x009d63b0, 0x006ab080, "rename",
     "?setAssertionHook@G3D@@YAXP6A_NPBDABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0H_N@Z@Z",
     "src2016:",
     "G3D::setAssertionHook (G3D 8.00 debugAssert.cpp) | stores its argument to 0x00db54b0 (STUDIO 0x00eaadd8), "
     "whose initial value is 0x009d6510 ('Assertion Failure': _handleDebugAssert_); MainLogManager ctor passes "
     "handleDebugAssert"),
    (0x009d63c0, 0x006ab090, "rename",
     "?setFailureHook@G3D@@YAXP6A_NPBDABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0H_N@Z@Z",
     "src2016:",
     "G3D::setFailureHook (G3D 8.00 debugAssert.cpp) | stores its argument to 0x00db54b4 (STUDIO 0x00eaaddc), "
     "whose initial value is _handleErrorCheck_ 0x009d6780 (ios:)"),
    (0x0046a2e0, None, "rename", "?IsVistaPlus@@YA_NXZ", "src2016:",
     "IsVistaPlus() | GetVersionExA, returns dwPlatformId == 2 && dwMajorVersion >= 6"),
    (0x00435f10, None, "rename", "?UploadCrashEventFile@DumpErrorUploader@@SAXPAU_EXCEPTION_POINTERS@@@Z", "str:",
     "DumpErrorUploader::UploadCrashEventFile(_EXCEPTION_POINTERS*) | 'Crash Event post', 'Crash Event posted, "
     "response: %s', 'Exception during upload crash event'; called by ProcessException 0x0043f660 after "
     "'Uploading .crashevent...'"),
    (0x00463f50, None, "rename", "?Load@CVersionInfo@@QAEHPAUHINSTANCE__@@@Z", "src2016:",
     "CVersionInfo::Load(HINSTANCE) | GetModuleFileNameW + GetShortPathNameW, then the version-resource reader "
     "0x00464050"),
    (0x00464370, None, "rename",
     "?GetFileVersionAsString@CVersionInfo@@QAE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ",
     "str:", "CVersionInfo::GetFileVersionAsString() | queries the 'FileVersion' value through 0x00464200"),
    (0x00467880, None, "rename", "?SetFullscreen@View@RBX@@QAEX_N@Z", "str:",
     "RBX::View::SetFullscreen(bool) | compares the flag at +0x45, then calls View::changeResolution 0x00467ae0 "
     "('Done View::changeResolution') after GetWindowLongA(GWL_STYLE), or View::restoreResolution 0x004676a0 "
     "('Start View::restoreResolution'); PLAYER only"),
    # wave12 sync: the unresolved callees gen_link_stubs still called "unknown" (integrate.md wave 12,
    # Client/_generated/<t>/unresolved.txt), named with the decorated names our tree declares; checked from the
    # exe bytes. STUDIO addresses from STUDIO-PLAYER-bodymatch.csv pairs or the same byte pattern.
    (0x009b8060, 0x00a72540, "rename",
     "?utf8_encode@RBX@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ABV?$basic_string@_WU?$"
     "char_traits@_W@std@@V?$allocator@_W@2@@3@@Z", "src2016:",
     "RBX::utf8_encode(const std::wstring&) (Base StringConv) | WideCharToMultiByte into a std::string (char "
     "assign/_Tidy imports); CVersionInfo::Load 0x00463f50 passes it the GetShortPathNameW result"),
    (0x009b8100, 0x00a725e0, "rename",
     "?utf8_decode@RBX@@YA?AV?$basic_string@_WU?$char_traits@_W@std@@V?$allocator@_W@2@@std@@ABV?$basic_string@DU?$"
     "char_traits@D@std@@V?$allocator@D@2@@3@@Z", "src2016:",
     "RBX::utf8_decode(const std::string&) (Base StringConv) | MultiByteToWideChar into a std::wstring (wchar_t "
     "assign/_Tidy imports), 16 callers; directly after utf8_encode"),
    (0x009d5da0, 0x006aaa70, "rename",
     "?println@Log@G3D@@QAEXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z", "str:",
     "G3D::Log::println(const std::string&) (iOS 1029 symbol) | if logFile: printHeader 0x009d5df0, fprintf(logFile, "
     "'%s\\n' 0x00bbd0b8, s.c_str()), fflush"),
    (0x009d5fa0, 0x006aac70, "rename",
     "?print@Log@G3D@@QAEXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z", "str:",
     "G3D::Log::print(const std::string&) (iOS 1029 symbol) | println's twin with format '%s' 0x00be1f20 (no newline)"),
    (0x009b0f40, 0x00a68030, "rename", "?setLogProvider@Log@RBX@@SAXPAVILogProvider@2@@Z", "src2016:",
     "RBX::Log::setLogProvider(ILogProvider*) | stores its argument to 0x00e1aa80 (STUDIO 0x00f9dc70), "
     "RBX::Log::provider; the MainLogManager ctor 0x0043fa50 calls it with `this` right after "
     "`mainLogManager = this` (0x0043fbb7)"),
    (0x009b19a0, 0x00a68a90, "rename", "?setAssertionHook@RBX@@YAXP6A_NPBD0H@Z@Z", "src2016:",
     "RBX::setAssertionHook (Base RbxAssert.cpp) | stores to _debugHook 0x00e1aac8 (STUDIO 0x00f9dcb8, our GLOBAL "
     "markers); the MainLogManager ctor passes handleDebugAssert 0x00440850"),
    (0x009b19b0, 0x00a68aa0, "rename", "?setFailureHook@RBX@@YAXP6A_NPBD0H@Z@Z", "src2016:",
     "RBX::setFailureHook (Base RbxAssert.cpp) | stores to _failureHook 0x00e1aacc (STUDIO 0x00f9dcbc); the "
     "MainLogManager ctor passes handleFailure 0x00440920"),
    (0x00e1aa80, 0x00f9dc70, "data4", "_ZN3RBX3Log8providerE", "ios:",
     "RBX::Log::provider (ILogProvider*) | iOS 1029 symbol; written only by setLogProvider 0x009b0f40 "
     "(STUDIO 0x00a68030)"),
    (0x0066ef10, 0x0077fae0, "rename", "?getSalt@ProtectedString@RBX@@QBEPBDXZ", "ios:",
     "RBX::ProtectedString::getSalt() const (iOS 1029 symbol) | `mov eax, '10:46:55'; ret` (STUDIO '19:11:34', "
     "a __TIME__ salt), before readUnprotectedChar 0x0066ef50; our Util/ProtectedString.h declares it static"),
    (0x00703070, 0x0070c5d0, "rename",
     "??$convert@VVector2int16@G3D@@@Variant@Reflection@RBX@@QAEAAVVector2int16@G3D@@XZ", "ios:",
     "RBX::Reflection::Variant::convert<G3D::Vector2int16>() (iOS 1029 symbol) | `jmp genericConvert<Vector2int16>` "
     "0x00702090, the shape of convert<UDim2> 0x00703080 (100%); called by Variant::get<Vector2int16> 0x008b2496"),
    (0x008d3540, 0x00846e30, "rename",
     "?convertToValue@?$StringConverter@VContentId@RBX@@@RBX@@SA_NABV?$basic_string@DU?$char_traits@D@std@@V?$"
     "allocator@D@2@@std@@AAVContentId@2@@Z", "ios:",
     "RBX::StringConverter<ContentId>::convertToValue (iOS 1029 symbol) | cdecl (text, value): value = "
     "ContentId(text) 0x004c4780, return true; called by TypedPropertyDescriptor<ContentId>::setStringValue "
     "0x00700d40 (vftable slot 10)"),
    (0x0066f460, 0x00780050, "rename",
     "?convertToValue@?$StringConverter@VProtectedString@RBX@@@RBX@@SA_NABV?$basic_string@DU?$char_traits@D@std@@V?$"
     "allocator@D@2@@std@@AAVProtectedString@2@@Z", "ios:",
     "RBX::StringConverter<ProtectedString>::convertToValue (iOS 1029 symbol) | cdecl (text, value): value = "
     "ProtectedString::fromTrustedSource(text) 0x0066f320, calculateHash 0x0066f060, return true; called by "
     "TypedPropertyDescriptor<ProtectedString>::setStringValue 0x0066f6c0 (vftable slot 10)"),
    # app-12 (app-followups.md): STUB in Util/Http.cpp, 85.1 / 94.1 in sweep19, paired by reccmp in both targets.
    (0x00623bb0, 0x006cca30, "rename", "?isMoneySite@Http@RBX@@SA_NPBD@Z", "str:",
     "RBX::Http::isMoneySite(const char*) | host-suffix tests against 'paypal.com' and 'rixty.com'; our STUB "
     "Util/Http.cpp pairs here (85.1, PLAYER and STUDIO)"),
    (0x00624620, 0x006cd530, "rename", "?trustCheckBrowser@Http@RBX@@SA_NPBD@Z", "src2016:",
     "RBX::Http::trustCheckBrowser(const char*) | ends `test al,al; setne al` on isMoneySite 0x00623bb0; our STUB "
     "Util/Http.cpp pairs here (94.1, PLAYER and STUDIO)"),
    # wave13 sync (app-13.md "IDA owner (names)"): the GameBasicSettings / GameSettings property accessors our
    # tree does not declare yet (so no decoration to copy); iOS 1029 symbols, each checked from the exe bytes
    # (PLAYER and STUDIO bodies identical, raise target PLAYER 0x0061c750 / STUDIO 0x006f1260).
    # wave14 sync (app-14.md "IDA owner (names)"): 0x006f61f0 is inFullScreen(), not getFullScreen (wave 13): the
    # "InFullScreen" BoundFuncDesc stores it; getFullScreen() const / setFullScreen are inline (the Fullscreen
    # descriptor stores the shell copies 0x004659c0 / 0x004659d0). reccmp 100% in sweep21 (P and S).
    (0x006f61f0, 0x00792fd0, "rename", "?inFullScreen@GameBasicSettings@RBX@@QAE_NXZ", "match:",
     "RBX::GameBasicSettings::inFullScreen() (iOS 1029 has the non-const inFullScreen()) | `mov al,[ecx+7Ch]; "
     "ret`, stored by the InFullScreen BoundFuncDesc; reccmp 100% vs our GameBasicSettings.cpp (sweep21)"),
    (0x006f6200, 0x00792fe0, "rename", "_ZN3RBX17GameBasicSettings14setControlModeENS0_11ControlModeE", "ios:",
     "RBX::GameBasicSettings::setControlMode(ControlMode) (iOS 1029) | if [+0x6c] != v: store, raisePropertyChanged "
     "desc 0x00df2628 ('ControlMode')"),
    (0x006f6220, 0x00793000, "rename",
     "_ZN3RBX17GameBasicSettings16setRenderQualityENS0_20RenderQualitySettingE", "ios:",
     "RBX::GameBasicSettings::setRenderQuality(RenderQualitySetting) (iOS 1029) | if [+0x70] != v: store, "
     "raisePropertyChanged desc 0x00df26fc ('SavedQualityLevel')"),
    (0x006f6240, 0x00793020, "rename", "_ZN3RBX17GameBasicSettings12setMouseLockEb", "ios:",
     "RBX::GameBasicSettings::setMouseLock(bool) (iOS 1029) | `mov [ecx+74h],al; ret 4`, no signal"),
    (0x006f6250, 0x00793030, "rename",
     "_ZN3RBX17GameBasicSettings21setUploadVideoSettingENS_12GameSettings13UploadSettingE", "ios:",
     "RBX::GameBasicSettings::setUploadVideoSetting(GameSettings::UploadSetting) (iOS 1029) | if [+0x78] != v: "
     "store, raisePropertyChanged desc 0x00df27e8 ('VideoUploadPromptBehavior')"),
    (0x006f6270, 0x00793050, "rename", "_ZN3RBX17GameBasicSettings23setAllTutorialsDisabledEb", "ios:",
     "RBX::GameBasicSettings::setAllTutorialsDisabled(bool) (iOS 1029) | if byte [+0x8c] != v: store, "
     "raisePropertyChanged desc 0x00df26a0 ('AllTutorialsDisabled')"),
    (0x006f4810, 0x00791df0, "rename",
     "_ZN3RBX12GameSettings19setPostImageSettingENS0_13UploadSettingE", "ios:",
     "RBX::GameSettings::setPostImageSetting(UploadSetting) (iOS 1029) | if [+0x8c] != v: store, "
     "raisePropertyChanged desc 0x00df1fec ('ImageUploadPromptBehavior')"),
    (0x006f4840, 0x00791e20, "rename",
     "_ZN3RBX12GameSettings22setVideoQualitySettingENS0_12VideoQualityE", "ios:",
     "RBX::GameSettings::setVideoQualitySetting(VideoQuality) (iOS 1029) | if [+0x90] != v: store, "
     "raisePropertyChanged desc 0x00df207c ('VideoQuality', category 'Video')"),
    (0x00656320, 0x0073ee60, "rename", "globalAdvancedSettingsFile", "src2016:",
     "static globalAdvancedSettingsFile() (2016 GlobalSettings.cpp) | getUserDirectory(true, DirAppData, NULL) "
     "0x006ea4c0, append '\\GlobalSettings_13.xml' 0x00b31094 when not empty; twin of the basic-settings file "
     "helper 0x00656390 (our basicSettingsFile, 2016 globalBasicSettingsFile); reccmp 100% vs our "
     "RBX::advancedSettingsFile (app-15, sweep22): the 2016 spelling is kept as the name"),
    (0x0072aaf0, 0x0082a3e0, "rename", "?renderGameCursor@UserInputBase@RBX@@UAEXPAVAdorn@2@@Z", "rtti:",
     "RBX::UserInputBase::renderGameCursor(Adorn*) (iOS 1029) | slot 7 of UserInput's vftable 0x00be4d9c (not "
     "overridden); our Util/UserInputBase.cpp STUB pairs here"),
    (0x006afb40, 0x0080e860, "rename", "?playerRemoving@FriendService@RBX@@QAEXH@Z", "ios:",
     "RBX::FriendService::playerRemoving(int) (iOS 1021/1029) | only caller Network::Players::onChildRemoving "
     "0x0056bd8e (STUDIO 0x005d123e): find<FriendService> 0x00548490, then playerRemoving(player->userId [+0xc8]); "
     "our FriendService.cpp STUB pairs here"),
    # shell-5k-5.md item 3: the engine bind chain next to DebrisService is weak_ptr<Instance>, not weak_ptr<DataModel>
    # (2013's weak_ptr<DataModel> bind is the shell copy 0x0043a480, called from 0x00526859 / 0x005a42ba / 0x0066e277 /
    # 0x006a9290). Our build folds both instantiations into one body (reccmp 100%, it picked the DataModel spelling);
    # the Instance spelling is the DebrisService.cpp.obj name at the same recomp address. The function0 /
    # basic_vtable0 rows 0x00844de0 / 0x00844f60 / 0x0073da60 / 0x0073db90 / 0x0074c040 took their Instance
    # spellings from the bulk rule in the wave15 sync (integrate wave 15's MATCH_SUB made reccmp pair them so).
    (0x008445f0, 0x0098a120, "rename",
     "??0?$list1@V?$value@V?$weak_ptr@VInstance@RBX@@@boost@@@_bi@boost@@@_bi@boost@@QAE@V?$value@V?$weak_ptr@"
     "VInstance@RBX@@@boost@@@12@@Z", "match:",
     "boost::_bi::list1<value<weak_ptr<Instance>>> ctor | only caller the bind 0x008447e0 (STUDIO 0x0098a310); "
     "reccmp 100% vs our DebrisService.cpp.obj copy (folded with Script.cpp's weak_ptr<DataModel> twin)"),
    (0x008447e0, 0x0098a310, "rename",
     "??$bind@XV?$weak_ptr@VInstance@RBX@@@boost@@V12@@boost@@YA?AV?$bind_t@XP6AXV?$weak_ptr@VInstance@RBX@@@boost@@"
     "@ZV?$list1@V?$value@V?$weak_ptr@VInstance@RBX@@@boost@@@_bi@boost@@@_bi@2@@_bi@0@P6AXV?$weak_ptr@VInstance@RBX@@"
     "@0@@Z0@Z", "match:",
     "boost::bind<void, weak_ptr<Instance>>(&cleanup, weak_ptr<Instance>) | only caller DebrisService::addItem "
     "0x00845087 (STUDIO 0x0098aa57); reccmp 100% vs our DebrisService.cpp.obj copy (folded with Script.cpp's "
     "weak_ptr<DataModel> twin)"),
    # wave14 sync: three of integrate.md's "gen_link_stubs can find no address" callees, located from the 2013 call
    # sites of our own shell functions (decorations as Client/_generated/player/unresolved.txt spells them).
    (0x0045f1b0, None, "rename", "?PostUserInputMessage@UserInput@RBX@@QAEXIIJ@Z", "src2016:",
     "RBX::UserInput::PostUserInputMessage(UINT, WPARAM, LPARAM) | only caller View::HandleWindowsMessage 0x00466510 "
     "at 0x004665e1: `mov edi,[edi+0B0h]` (View+0xb0 userInput), push lParam/wParam, push edi (/GL shell, this on "
     "the stack) (shell-5k-5)"),
    (0x009c5e10, None, "rename",
     "?getResolutionPreset@CRenderSettings@RBX@@QAEABURESOLUTIONENTRY@12@W4ResolutionPreset@12@@Z", "str:",
     "RBX::CRenderSettings::getResolutionPreset(ResolutionPreset) | assert 'preset < ResolutionMaxIndex' "
     "(Rendering\\GfxBase\\RenderSettings.cpp:66, /Od frame); only caller View::initializeSizes 0x004679d1 with "
     "ecx = CRenderSettingsItem::singleton()+0x64, the preference pushed"),
    (0x00464f30, 0x0048caa0, "rename", "?startRecording@VideoControl@RBX@@QAEXPAVSoundService@Soundscape@2@@Z", "src2016:",
     "RBX::VideoControl::startRecording(Soundscape::SoundService*) | only caller RecordToggleVerb::startAction "
     "0x00439402: ecx = ServiceProvider::create<SoundService> result, edi = videoControl [this+0x14] (/GL shell "
     "registers); right after VideoControl's slot functions (STUDIO 0x0048caa0: same position after ~VideoControl "
     "0x0048c940, same 625 bytes, 199 of 199 mnemonics equal)"),
    # wave15 sync (app-15.md "IDA owner (names)"): DebugSettings' two hardware queries (our tree declares them,
    # STUB bodies) and two log/flag globals (FastLogSettings.cpp GLOBAL markers, iOS 1029 symbols).
    (0x00659170, 0x00773bf0, "rename",
     "?gfxcard@DebugSettings@RBX@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ", "ios:",
     "RBX::DebugSettings::gfxcard() const (iOS 1029) | EnumDisplayDevicesA, SetupDi* class enumeration, "
     "'HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Class\\' + '\\DriverVersion', ' drv=', 'unknown'; "
     "returns std::string; only caller View::calcDefaultResolution 0x00467dd0 (Intel / '828' test, shell-5k-6)"),
    (0x00659910, 0x00774340, "rename", "?videoMemory@DebugSettings@RBX@@QBEHXZ", "ios:",
     "RBX::DebugSettings::videoMemory() const (iOS 1029) | 90 bytes, one call and a std::string dtor; only caller "
     "View::calcDefaultResolution 0x00467dd0 (the 32/64/128/256 MB table, shell-5k-6)"),
    (0x00d30edd, 0x00ec17c9, "data1", "_ZN4FLog13RobloxWndInitE", "ios:",
     "FLog::RobloxWndInit (iOS 1029 data) | log channel, initial 1, registered 'RobloxWndInit' by the initializer "
     "PLAYER 0x00a65120 / STUDIO 0x00b1c2f0; our FastLogSettings.cpp GLOBAL (app-15)"),
    (0x00de3f88, 0x00f84e62, "data1", "_ZN6DFFlag15UseClusterCacheE", "ios:",
     "DFFlag::UseClusterCache (iOS 1029, a DFFlag not an FFlag) | registered 'UseClusterCache' with kind 1 "
     "(dynamic) by the initializer PLAYER 0x00a661d0 / STUDIO 0x00b1d3e0; read by Network::Server::onServiceProvider; "
     "our FastLogSettings.cpp GLOBAL (app-15)"),
    # wave16 sync (app-16.md / integrate.md "IDA owner"): FriendService's {24}/{28} adjustor thunks (reccmp does not
    # score them), its two replication handlers (STUBs in our FriendService.cpp) and the two GfxBase
    # FrameRateManager toggles the shell calls (ResumeAutoAdjustment is a gen_link_stubs "unknown" callee).
    (0x006b2950, 0x00811650, "rename", "??_EFriendService@RBX@@WBI@AEPAXI@Z", "match:",
     "RBX::FriendService `vector deleting destructor'`adjustor{24}' | `sub ecx,18h; jmp` the D0 0x006b2b30 (STUDIO "
     "0x00811830); vftable slot 0x00b39754; 8 bytes equal our FriendService.cpp.obj thunk (jmp offset included)"),
    (0x006b2960, 0x00811660, "rename", "??_EFriendService@RBX@@WBM@AEPAXI@Z", "match:",
     "RBX::FriendService `vector deleting destructor'`adjustor{28}' | `sub ecx,1Ch; jmp` the D0 0x006b2b30 (STUDIO "
     "0x00811830); vftable slot 0x00b39744; 8 bytes equal our FriendService.cpp.obj thunk (jmp offset included)"),
    (0x006b14e0, 0x008101e0, "rename",
     "?friendEventReplicationChanged@FriendService@RBX@@AAEXHHW4FriendEventType@12@@Z", "ios:",
     "RBX::FriendService::friendEventReplicationChanged(int, int, FriendEventType) (iOS 1029) | bound in "
     "onServiceProvider 0x006b2b50 (push at 0x006b2c09); finds Players and jumps to its 0x00566790; our "
     "FriendService.cpp STUB pairs here (app-16)"),
    (0x006b2440, 0x00811140, "rename",
     "?friendStatusReplicationChanged@FriendService@RBX@@AAEXHHW4FriendStatus@12@@Z", "ios:",
     "RBX::FriendService::friendStatusReplicationChanged(int, int, FriendStatus) (iOS 1029) | bound in "
     "onServiceProvider 0x006b2b50 (push at 0x006b2ba4); calls Players 0x0055f070 with the status remapped 3<->4; "
     "our FriendService.cpp STUB pairs here (app-16)"),
    (0x009c7aa0, 0x006c3e40, "rename", "?PauseAutoAdjustment@FrameRateManager@RBX@@QAEXXZ", "src2016:",
     "RBX::FrameRateManager::PauseAutoAdjustment() (2016 FrameRateManager.cpp: mAdjustmentOn = false) | /Od GfxBase "
     "body `mov byte [this+0Ch],0`; only caller VideoControl::startRecording 0x00464f30 at 0x00465186 (STUDIO "
     "0x0048ccf6), as 2016 VideoControl.cpp; next to ResumeAutoAdjustment"),
    (0x009c7ac0, 0x006c3e60, "rename", "?ResumeAutoAdjustment@FrameRateManager@RBX@@QAEXXZ", "src2016:",
     "RBX::FrameRateManager::ResumeAutoAdjustment() (2016 FrameRateManager.cpp: mAdjustmentOn = true) | /Od GfxBase "
     "body `mov byte [this+0Ch],1`; only caller 0x00439477 (STUDIO 0x004693e4), the VideoControl stop path; "
     "decoration from Client/_generated/player/unresolved.txt (shell-5k-7, VideoControl.h)"),
]


def read_csv(name):
    p = os.path.join(EXP, name)
    return list(csv.DictReader(open(p))) if os.path.exists(p) else []


def studio_of(player_addr):
    """STUDIO address of a PLAYER function: a unique body match (pair or pair-callpos)."""
    hits = [int(r["a_address"], 16) for r in read_csv("STUDIO-PLAYER-bodymatch.csv")
            if r["status"] in ("pair", "pair-callpos") and int(r["b_address"], 16) == player_addr]
    return hits[0] if len(hits) == 1 else None


def split_top(q):
    """Split a qualified name at the '::' outside template brackets."""
    out, depth, k0 = [], 0, 0
    for k, ch in enumerate(q):
        depth += ch == "<"
        depth -= ch == ">"
        if depth == 0 and q[k:k + 2] == "::" and k >= k0:
            out.append(q[k0:k])
            k0 = k + 2
    return out + [q[k0:]]


def canon(sig):
    """norm_sig plus what still differs between the ABIs' demanglers: bool template arguments (Itanium
    prints true/false, llvm-undname 1/0) and template arguments repeated on a ctor/dtor name (undname
    prints ~EnumDesc<X>, c++filt ~EnumDesc)."""
    if not sig:
        return sig
    sig = re.sub(r"(?<=[<,])true(?=[,>])", "1", sig)
    sig = re.sub(r"(?<=[<,])false(?=[,>])", "0", sig)
    q = G.qual(sig)
    parts = split_top(q)
    if len(parts) >= 2:
        last, cls = parts[-1], parts[-2].split("<")[0]
        if last.lstrip("~").split("<")[0] == cls and "<" in last:
            parts[-1] = last.split("<")[0]
            sig = "::".join(parts) + sig[len(q):]
    return sig


def last_name(q):
    """Last component of a qualified name, without template arguments."""
    if q.startswith("?") and not q.startswith("??"):
        return q[1:].split("@")[0]      # an MSVC name reccmp could not undecorate: ?name@scope@...
    m = re.match(r"\?\?([01])(?:\?\$)?([^@]+)@", q)
    if m:                               # an undecoratable ctor/dtor: ??0?$Value@... -> Value, ??1X@ -> ~X
        return ("~" if m.group(1) == "1" else "") + m.group(2)
    return split_top(q)[-1].split("<")[0]


def resolve(v, cur, src, dem, want, nm, size, oev, elsewhere):
    """Decide a conflict (our 100% body vs an oracle or FLIRT name) by the evidence that placed the old name.
    oev: (evidence_class, evidence) of the oracle row in <T>-names-proposed.csv. Returns (verdict, note)."""
    cls, ev = oev
    if src == "flirt:":
        if cur in ("start",):
            return "keep-entry", "the PE entry point keeps IDA's name"
        return "body-wins", "superseded flirt: %s (vc32rtf signature, contradicted by the 100%% body)" % cur
    if ev.startswith("folded (") or size <= 8:
        return "folded-original", ("the original linker folded identical bodies (%s); the oracle name is one of "
                                   "them, ours another; oracle name kept" % (ev.split(";")[0] if ev else
                                                                             "%d-byte body" % size))
    if elsewhere:
        return "folded-original", ("our build defines the oracle signature at 0x%s too: the original linker folded "
                                   "the two; oracle name kept" % ",0x".join("%08x" % x for x in elsewhere))
    if nm and dem and last_name(G.qual(G.norm_sig(dem) or dem)) == last_name(nm):
        return "same-function", "same function name (ours %s); the class or parameters are spelled differently; " \
                                "oracle name kept" % nm
    if cls in ("callgraph", "body-match", "body-match-guess"):
        return "body-wins", "superseded %s %s (%s evidence only, contradicted by the 100%% body)" % (src, dem, cls)
    return "same-function", ("oracle evidence %s (%s) names this function; our name differs in spelling or source; "
                             "oracle name kept" % (cls or "-", ev[:80]))


def classify(cur, src, sig_cur, ours, our_sigs, elsewhere):
    """Verdict for one 100% pairing. cur: IDA name, src: its export source, sig_cur: its norm_sig,
    ours: our names at R, our_sigs: their norm_sigs, elsewhere: our build defines sig_cur at another body."""
    if src == "dummy" or DUMMY.match(cur):
        return "rename"
    if cur in ours or cur in {IDA_BAD.sub("_", x) for x in ours}:
        return "agree"      # (IDA stores a proc name such as resume_adapter<T> with its bad characters as '_')
    if sig_cur and sig_cur in our_sigs:
        return "agree-sig"
    if sig_cur and G.qual(sig_cur) in {G.qual(x) for x in our_sigs}:
        return "agree-qual"     # same function, parameters or cv spelled differently (our source, not 2013)
    if src in ("prior:", "ida:", "match:"):
        return "rename"     # match: here is an earlier reccmp promotion (library-owned addresses never get here)
    if src in ("ios:", "mac:", "flirt:"):
        return "conflict-folded" if elsewhere else "conflict"
    return "conflict"   # match:/lib: names the library pass owns; reported, not touched


def plan(target, json_path, exe):
    funcs = {int(r["address"], 16): r["name"] for r in read_csv(f"{target}-functions.csv")}
    names = {int(r["address"], 16): r for r in read_csv(f"{target}-names.csv") if r["kind"] == "func"}
    libown = {int(r["address"], 16) for r in read_csv(f"{target}-lib-resolved.csv")
              if r["status"] in ("identical", "located")}
    size = {int(r["address"], 16): int(r["size"]) for r in read_csv(f"{target}-functions.csv")}
    oev = {}
    for r in read_csv(f"{target}-names-proposed.csv"):
        oev.setdefault(int(r["address"], 16), (r["evidence_class"], r["evidence"]))
    view = G.pdb_view(exe)
    addr, _, _ = G.read_map(exe[:-4] + ".map")
    at = defaultdict(set)
    for d, a in addr.items():
        at[a].add(d)
    entries = [e for e in json.load(open(json_path))["data"]
               if e.get("type", 1) in (1, 10) and e.get("recomp") and (e["matching"] >= 1.0 or e.get("effective"))]
    # every decorated name we need a signature for: ours at each R, and the current MSVC names
    dec = set()
    for e in entries:
        r = int(e["recomp"], 16)
        dec |= {d for d in at[r] if d.startswith("?")}
        if view.get(r, (0, None))[1]:
            dec.add(view[r][1])
    dec |= {n for n in funcs.values() if n.startswith("?")}
    sig = G.undname(sorted(dec))
    oursig_all = defaultdict(set)   # signature -> our addresses defining it
    for d, a in addr.items():
        if sig.get(d):
            oursig_all[canon(sig[d])].add(a)

    updates, conflicts, stat = {}, [], Counter()
    jname = os.path.basename(json_path)
    for e in entries:
        a, r = int(e["address"], 16), int(e["recomp"], 16)
        if a not in funcs:
            stat["not an IDA function start"] += 1
            continue
        if a in libown:
            stat["library-owned (apply_lib_names.py)"] += 1
            continue
        t, d, nm = view.get(r, ("-", None, None))
        want = d or nm
        if not want:
            stat["no recompiled name"] += 1
            continue
        flat = lambda x: re.sub(r"\b(?:class|struct|enum|union) |\s", "", x or "")
        named = [x for x in sorted(at[r]) if x.startswith("?") and (x == e["name"] or     # reccmp reports a decorated name
                 flat(sig.get(x)).replace("operator#36", "operator=").startswith(flat(e["name"]) + "("))]
        if d and len(at[r]) > 1 and named and d not in named:
            want = d = named[0]     # our linker folded several names at R: take the one reccmp reports
        ours = ({d} if d else set()) | at[r] | ({nm} if nm else set())
        our_sigs = {canon(sig[x]) for x in ours if sig.get(x)} | ({canon(G.norm_sig(nm + "()"))} if nm and not d else set())
        cur = funcs[a]
        row = names.get(a)
        src = row["source"] if row else "dummy"
        dem = (row["demangled"] if row else "") or cur
        sig_cur = canon(sig.get(cur) if cur.startswith("?") else G.norm_sig(dem))
        elsewhere = bool(sig_cur and oursig_all.get(sig_cur, set()) - {r})
        v = classify(cur, src, sig_cur, ours, our_sigs, elsewhere)
        stat[f"{v} ({src})"] += 1
        pct = "effective 100%" if e["matching"] < 1.0 else "100%"
        # an agreeing name is one of our folded names at R: the line records that one (export_names.py reads it)
        line = f"p4 match: reccmp {pct} vs our {cur if v == 'agree' else want} @0x{r:08x} ({jname})"
        if v in ("agree-sig", "agree-qual") and src in ("prior:", "ida:"):
            v = "rename"    # the 100% body outranks a prior or auto name, also when the signature agrees
        if v == "rename":
            updates[a] = ("rename", want, "match:", line)
        elif v.startswith("agree"):
            updates[a] = ("comment", "", "match:", line)
        else:
            v, note = resolve(v, cur, src, dem, want, nm, size.get(a, 0), oev.get(a, ("", "")),
                              sorted(oursig_all.get(sig_cur, set()) - {r}))
            stat[f"  resolved: {v}"] += 1
            if v == "body-wins":
                updates[a] = ("rename-lib" if src == "flirt:" else "rename", want, "match:", line + "; " + note)
            else:
                updates[a] = ("comment", "", "match:", line + "; " + note)
            conflicts.append((f"0x{a:08x}", cur, src, dem, want, f"0x{r:08x}", e["name"], v, note))

    # vftable slots: a slot both vftables agree on ("both" row of a type-5 entry: the original slot target is the
    # function reccmp pairs with our slot target) names a dummy original function after our slot function, tag
    # rtti:, when that original address pairs with one recompiled function in every vftable that lists it.
    slot = defaultdict(set)
    slotwhere = {}
    for e in json.load(open(json_path))["data"]:
        if e.get("type") != 5:
            continue
        for _, blocks in e.get("diff") or []:
            for b in blocks:
                for row in b.get("both", []):
                    m = re.match(r"\((0x[0-9a-f]+) / (0x[0-9a-f]+)\)", row[1])
                    if m:
                        oa, ra = int(m.group(1), 16), int(m.group(2), 16)
                        slot[oa].add(ra)
                        slotwhere.setdefault(oa, f"slot {row[0][6:]} of {e['name']} {e['address']}")
                for row in b.get("orig", []):      # an original slot target our vftable disagrees on
                    m = re.match(r"\((0x[0-9a-f]+) /", row[1])
                    if m:
                        slot[int(m.group(1), 16)].add(None)
    for a, rs in sorted(slot.items()):
        if a in updates or a in libown or a not in funcs or not DUMMY.match(funcs[a]):
            continue
        if len(rs) != 1 or None in rs:
            stat["slot: several recompiled functions or a disagreeing slot"] += 1
            continue
        r = next(iter(rs))
        if size.get(a, 0) <= 8 or len(at[r]) > 1:
            # a tiny body the original linker shares between classes, or one our linker folded: which of the
            # folded names the slot means is not decided by the slot
            stat["slot: folded or tiny body"] += 1
            continue
        t, d, nm = view.get(r, ("-", None, None))
        if not (d or nm):
            continue
        updates[a] = ("rename", d or nm, "rtti:",
                      f"p4 rtti: {slotwhere[a]}: reccmp pairs this slot with our {d or nm} @0x{r:08x} ({jname})")
        stat["slot rename (rtti:)"] += 1

    for pa, sa, act, name, tag, ev in FACTS:
        if pa is None and target == "PLAYER":     # a STUDIO-only fact
            continue
        a = pa if target == "PLAYER" else (sa or studio_of(pa))
        if a is None:
            stat["fact: no STUDIO address"] += 1
            print(f"{target}: no STUDIO address for fact 0x{pa:08x} {name or ev[:40]}")
            continue
        if tag == "prior:" and a in updates and updates[a][3].startswith("p4 match: reccmp"):
            stat["fact prior: left to reccmp 100%"] += 1
            continue
        where = "" if target == "PLAYER" or pa is None else f" [STUDIO: body-identical to PLAYER 0x{pa:08x}]"
        if a in updates and updates[a][0] == "rename" and act == "rename" and updates[a][1] != name:
            print(f"{target}: fact 0x{a:08x} {name} overrides the reccmp name {updates[a][1]}")
        if act == "keep":       # the oracle name stays over our 100% body's name (our name is an invented one)
            line = updates[a][3] + "; " + ev if a in updates else f"p4 {tag} {ev}"  # "p4 ios:;" misparses
            updates[a] = ("comment", "", tag, line + where)
            conflicts[:] = [c[:7] + ("oracle-kept (fact)", ev) if c[0] == f"0x{a:08x}" else c for c in conflicts]
        elif act == "comment" and a in updates:     # keep the reccmp line (and its source) and add the fact
            updates[a] = updates[a][:3] + (updates[a][3] + "; " + ev + where,)
        else:
            if act == "rename" and (names.get(a) or {}).get("source") == "flirt:":
                act = "rename-lib"  # a FLIRT false positive: also clear FUNC_LIB (STUDIO 0x006c3e40, ConcRT name)
            updates[a] = (act, name, tag, f"p4 {tag} {ev}{where}")
        stat[f"fact {act} ({tag})"] += 1

    if target == "STUDIO":
        # PLAYER's reccmp-proven renames carried to the STUDIO body that is identical to it (unique pair in
        # STUDIO-PLAYER-bodymatch.csv), where STUDIO's name is still a dummy and nothing above decided it.
        pmap = {int(r["b_address"], 16): int(r["a_address"], 16) for r in read_csv("STUDIO-PLAYER-bodymatch.csv")
                if r["status"] in ("pair", "pair-callpos")}
        # a name STUDIO keeps at another address (not renamed above) cannot be set (IDA names are unique): STUDIO
        # pairs its engine weak_ptr<DataModel> bind copies (0x0098a7b0 ...) with our DataModel spelling, while
        # PLAYER's engine copies took DebrisService's Instance spelling and its shell copies the DataModel one
        taken = {n for x, n in funcs.items() if not (x in updates and updates[x][0].startswith("rename"))}
        for r in read_csv("PLAYER-db-updates.csv"):
            pa = int(r["address"], 16)
            sa = pmap.get(pa)
            if r["action"] != "rename" or r["tag"] != "match:" or "reccmp" not in r["line"] or sa is None:
                continue
            if sa in updates or sa in libown or sa not in funcs or not DUMMY.match(funcs[sa]):
                continue
            if r["name"] in taken:
                stat["PLAYER name kept at another STUDIO address"] += 1
                continue
            updates[sa] = ("rename", r["name"], "match:",
                           r["line"] + f" [STUDIO: body-identical to PLAYER 0x{pa:08x}, measured there]")
            stat["rename from PLAYER via body match"] += 1
    with open(os.path.join(EXP, f"{target}-db-updates.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["address", "action", "name", "tag", "line"])
        for a in sorted(updates):
            w.writerow([f"0x{a:08x}", *updates[a]])
    with open(os.path.join(EXP, f"{target}-match-conflicts.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["address", "ida_name", "source", "demangled", "recompiled", "recomp_address", "reccmp_name",
                    "verdict", "evidence"])
        w.writerows(conflicts)
    for k, v in sorted(stat.items()):
        print(f"{target}: {k:55s} {v}")
    print(f"{target}: entries at 100% {len(entries)}, updates {len(updates)}, conflicts {len(conflicts)}")


def selftest():
    s = G.norm_sig
    assert classify("sub_401000", "dummy", None, {"?f@@YAXXZ"}, {"C:f"}, False) == "rename"
    assert classify("?f@@YAXXZ", "match:", "f()", {"?f@@YAXXZ"}, {"f()"}, False) == "agree"
    x = s("RBX::Name::Name(char const* const&)")
    assert classify("_ZN3RBX4NameC2ERKPKc", "ios:", x, {"?y"}, {x}, False) == "agree-sig"
    assert classify("_ZN1A1fEv", "ios:", "A::f()", {"?g"}, {"A::g()"}, True) == "conflict-folded"
    assert classify("_ZN1A1fEv", "ios:", "A::f()", {"?g"}, {"A::g()"}, False) == "conflict"
    assert classify("_ZN1A1fEv", "prior:", "A::f()", {"?g"}, {"A::g()"}, False) == "rename"
    assert resolve("conflict", "_Z", "ios:", "A::f()", "?g", "A::g", 6, ("rtti-slot", "x"), [])[0] == "folded-original"
    assert resolve("conflict", "_Z", "ios:", "A::f()", "?g", "A::g", 76, ("callgraph", "x"), [])[0] == "body-wins"
    assert resolve("conflict", "_Z", "ios:", "W(X*)", "W", "W", 177, ("callgraph", "x"), [])[0] == "same-function"
    assert resolve("conflict", "_Z", "ios:", "A::f()", "?g", "A::g", 99, ("string-unique", "s"), [])[0] == "same-function"
    assert resolve("conflict", "_Z", "ios:", "D<X, s>::cd()", "?cd@", "D<X,&s>::cd", 99, ("callgraph", ""), [])[0] \
        == "same-function"
    assert resolve("conflict", "_Z", "ios:", "D<X, s>::cd()", "?cd@?$D", "?cd@?$D", 99, ("callgraph", ""), [])[0] \
        == "same-function"
    assert canon("A::Bridge<X,true>::on_index(lua_State*)") == "A::Bridge<X,1>::on_index(lua_State*)"
    assert canon("R::EnumDesc<R::Material>::~EnumDesc<R::Material>()") == "R::EnumDesc<R::Material>::~EnumDesc()"
    assert canon("rbx::trie<P*,50>::trie<P*,50>()") == "rbx::trie<P*,50>::trie()"
    assert classify("_ZN1A1fEv", "ios:", "A::f()const", {"?f"}, {"A::f()"}, False) == "agree-qual"
    print("match_names selftest ok")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("target", nargs="?")
    ap.add_argument("--json")
    ap.add_argument("--exe")
    ap.add_argument("--selftest", action="store_true")
    o = ap.parse_args()
    if o.selftest:
        selftest()
    else:
        plan(o.target, o.json, o.exe)
