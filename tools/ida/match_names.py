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
    (0x0073b3f0, 0x007c3a90, "keep", "", "ios:",
     "the iOS name RBX::Stats::Item::formatValue<double> stays: our StatsItem::formatValueId (100%) is the prior "
     "tree's invented spelling of the same template instance (CLAUDE.md: its invented names lose to oracle names)"),
    (0x0073b420, 0x007c3ac0, "keep", "", "ios:",
     "the iOS name RBX::Stats::Item::formatValue<float> stays: our StatsItem::formatValueIf (100%) is the prior "
     "tree's invented spelling of the same template instance"),
    (0x0073b450, 0x007c3af0, "keep", "", "ios:",
     "the iOS name RBX::Stats::Item::formatValue<int> stays: our StatsItem::formatValueIi (100%) is the prior "
     "tree's invented spelling of the same template instance"),
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
    (0x00402240, 0x00443510, "rename", "?__TBB_LockByte@@YAEAAE@Z", "lib:",
     "__TBB_LockByte(unsigned char&) (TBB 2.1 header, COMDAT) | loop: tbb.dll __TBB_machine_trylockbyte thunk "
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
    if cur in ours:
        return "agree"
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
        named = [x for x in sorted(at[r]) if x.startswith("?") and
                 flat(sig.get(x)).replace("operator#36", "operator=").startswith(flat(e["name"]) + "(")]
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
        a = pa if target == "PLAYER" else (sa or studio_of(pa))
        if a is None:
            stat["fact: no STUDIO address"] += 1
            print(f"{target}: no STUDIO address for fact 0x{pa:08x} {name or ev[:40]}")
            continue
        if tag == "prior:" and a in updates and updates[a][3].startswith("p4 match: reccmp"):
            stat["fact prior: left to reccmp 100%"] += 1
            continue
        where = "" if target == "PLAYER" else f" [STUDIO: body-identical to PLAYER 0x{pa:08x}]"
        if a in updates and updates[a][0] == "rename" and act == "rename" and updates[a][1] != name:
            print(f"{target}: fact 0x{a:08x} {name} overrides the reccmp name {updates[a][1]}")
        if act == "keep":       # the oracle name stays over our 100% body's name (our name is an invented one)
            base = updates[a][3] if a in updates else "p4 %s" % tag
            updates[a] = ("comment", "", tag, base + "; " + ev + where)
            conflicts[:] = [c[:7] + ("oracle-kept (fact)", ev) if c[0] == f"0x{a:08x}" else c for c in conflicts]
        elif act == "comment" and a in updates:     # keep the reccmp line (and its source) and add the fact
            updates[a] = updates[a][:3] + (updates[a][3] + "; " + ev + where,)
        else:
            updates[a] = (act, name, tag, f"p4 {tag} {ev}{where}")
        stat[f"fact {act} ({tag})"] += 1

    if target == "STUDIO":
        # PLAYER's reccmp-proven renames carried to the STUDIO body that is identical to it (unique pair in
        # STUDIO-PLAYER-bodymatch.csv), where STUDIO's name is still a dummy and nothing above decided it.
        pmap = {int(r["b_address"], 16): int(r["a_address"], 16) for r in read_csv("STUDIO-PLAYER-bodymatch.csv")
                if r["status"] in ("pair", "pair-callpos")}
        for r in read_csv("PLAYER-db-updates.csv"):
            pa = int(r["address"], 16)
            sa = pmap.get(pa)
            if r["action"] != "rename" or r["tag"] != "match:" or "reccmp" not in r["line"] or sa is None:
                continue
            if sa in updates or sa in libown or sa not in funcs or not DUMMY.match(funcs[sa]):
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
