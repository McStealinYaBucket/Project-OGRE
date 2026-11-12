# phase4-core (workflow phase4-pairing, wf_4baec7d9-68d)

## Phase 4, first pass: 2013 layouts applied to the core classes

Every class on the scout's list has its 2013 layout written in, except the DataModel members (0xac8–0xe10) and the ScriptContext member order. In both targets, App markers at 100% went from 154 FUNCTION + 23 FOLDED to **184 + 23 in PLAYER** and from 150 + 23 to **175 + 23 in STUDIO**.

**One function I own the cause of, but not the file, dropped from 100%: `PartInstance::resize` (PLAYER 0x00720810, STUDIO 0x007bc260, now 90.48%).** It only matched because two mistakes cancelled out:
- The old Instance vftable had 5 extra leading slots (the 2016 EventSource slots, the destructor and `getClassName`) plus `humanoidChanged`.
- PartInstance's vftable lacks `hitTestImpl` (+0x78) and `setLinearVelocity`/`setRotationalVelocity` (+0xc4/+0xc8), and has non-2013 virtuals `setPartSizeUi`/`setPartSizeUnjoined`.

With Instance corrected, resize's `destroyJoints` call lands at +0xdc instead of +0xe4. PartInstance.h is outside my ownership; `app_sweep apply` set the marker back to STUB.

### Final figures (baseline is the pairing run, build/sweep2/PF/SF.json)

| | PLAYER before | PLAYER after | STUDIO before | STUDIO after |
|---|---|---|---|---|
| reccmp with libraries | 3,548 / 40,468 | **3,828** (9.46%) | 3,454 / 42,329 | **3,691** (8.72%) |
| `--nolib` implemented | 1,501 / 38,363 | **1,751** / 38,331 | 1,519 / 40,225 | **1,723** / 40,191 |
| Entries at 100% (non-library) | 975 | 1,188 | 955 | 1,137 |
| Entries at 100% (library) | 1,831 | 1,848 | 1,757 | 1,780 |
| Entries that lost 100% / gained it | | 16 / 246 | | 19 / 224 |

The 16 PLAYER (and 19 STUDIO) losses fall into three groups:
- **Address coincidences (4 RakNet functions).** The original's immediate `0x7fffff` now equals one of our data addresses, `describedClassDescriptor+215`. With no `.reloc`, reccmp reads it as a data reference. Verified with `--verbose` on 0x607e50 and 0x5c8930.
- **Pairing churn from regenerating the CSVs.** A different identical-body twin gets picked each run: `exception_ptr` dtor, `_Allocate<shared_ptr<Instance>>`, `set<BaseScript*>` tree, `shared_from_this` callee, `getSingleton<ContentId>`/`<SoundId>`, `too_many_refs` sdd. Also the `HumanoidChangedSignalData` sdd at 0x004027d0, which sits in slot 0 of the `ICombinedSignalData` vftable, so both drop together.
- **`PartInstance::resize`** (above).

### Per class (PLAYER scores; STUDIO matches in every case checked)

**Instance and its bases.** Bases are now Limits::Countable, Diagnostics::Countable<Instance>, Debugable, GuidItem, AbstractFactoryProduct, Described, noncopyable. The new `DECOMP_SIZE_ASSERT`s pass for all of them: Instance 0x60, Debugable 4, GuidItem 0x14, Object/AFP 4, EventSource 4, DescribedBase 0x10, Limits::Countable 8.
- **Changes:**
  - Debugable gets `virtual dump`.
  - New `RBX::Object` / `Creatable : Object` / `AbstractFactoryProduct`, with the pure `getClassName` moved from DescribedBase to Object.
  - DescribedBase is now `enable_shared_from_this, EventSource`, with no `xmlId`.
  - EventSource has 3 slots.
  - GuidItem holds `shared_ptr<Registry>`; Registry derives `enable_shared_from_this`.
  - Limits::Countable is non-virtual, `{shared_ptr<Counter>}`, ctor 0x009b94d0, dtor 0x009b9160.
  - Members are parent 0x38, children 0x3c, isSettingParent 0x44, fwBase 0x48, onDemandPtr 0x50, combined/ancestry/propertyChanged signals 0x54/0x58/0x5c.
  - `archivable` and `robloxLocked` are gone (they are FW bytes now) and `humanoidChanged` is no longer virtual.
- **Scores:**
  - Now 100%: fw, onDemandRead, readProperty, onChildChanged, verifyAddDescendant, onAncestorChanged, onDescendantAdded, remove, signal helpers.
  - Ctors: 26.4% → 56.5% and 21.6% → 51.3%.
  - ~Instance: 61.0% → 83.2% (marker added).
  - Also improved: findChildIndex 60.9% → 93.1%, writeXml 46.2% → 81.4%, raisePropertyChanged 69.2% → 92.9%.
  - Base pieces: ~GuidItem 0 → 100%, DescribedBase ctor 0 → 100%, Limits::Countable dtor 0 → 100% (ctor 62.5%), Debugable::dump 91.7% (only diff is the unpaired `__type_info_root_node`).
- **Primary vftable:** 0x00b2a6ec now lines up slot for slot.
- **Still code, not layout:** in the ctor, the 2013 `Diagnostics::Countable` ctor increments `count` (0x00620efc calls 0x9b8f10). Our header deliberately does not, because SoundChannel, ThreadRef, ScriptEvent and ScriptContext increment by hand. The ctor also logs "New Instance %p, fw ref: %p" and creates the FW through helper 0x61fe70.

**ServiceProvider.** Annotated at 0x60–0x88, size 0x8c (assert passes). Its ctor at 0x00644290 (new marker) is at 100%, and so are onDescendantAdded and onDescendantRemoving (from about 93%) and onChildRemoving (from 81%).

**MemberDescriptorContainer / ClassDescriptor.** `descriptorLookup` is now `rbx::trie<T*, 50>`; 50 is the depth check in node ctor 0x00614f60 and the depth_exceeded_exception vftable name. Sizes 0x60 and 0x20c (asserts pass). ClassDescriptor() went 39.8% → 100%; it also needed the root flags `true` (0x0061841a). Other ctor 50.4% → 61.5%, dtor 32% → 60.2%. EventDescriptor and YieldFunctionDescriptor ctors are now 100%.

**DataModel.**
- **Bases:** declaration order is now DescribedNonCreatable, VerbContainer, IDataState, IMetric, DataModelArbiter, Countable. IMetric is new (`Client/App/include/Util/IMetric.h`), with getMetric/getMetricValue stubs at 0x00650660 and 0x00642af0.
- **Side finding fixed:** VerbContainer is 0x14 (one `verbs` map, addVerb/removeVerb). Verb derives from Debugable, which puts isChecked at +0xc and doIt at +0x18.
- **DataModelArbiter (0xa20, assert passes):** it has no stepActivity/stepCounter of its own. preStep and postStep use the base Arbiter's meter at +8. The member at 0x830 is a `shared_ptr` to a zeroed 4-byte object (kept as `m_unk0x830`); the lookup tables are at 0x838, and 0xa1c is alignment padding.
- **Scores:** close 95.5% → 100% and toggleToolsOff 92.1% → 100%, both targets. canSave 46.4% → 100%. VerbContainer ctor, dtor, getVerb and addVerb are 100% (addVerb's assert line 58 now matches). arbiter::areExclusive 50% → 100%. Also: initializeContents 43.8% → 70.9%, loadWorld 66.9% → 90.2%, loadGame 88.6% → 97.2%, setPlaceID 79.4% → 90.9%.
- **Not done:** DataModel's own members are still unmapped, so DataModel has no size assert.

**Job classes** (asserts pass): DataModelJob 0x218, GenericJob 0x248, GcJob 0x228. GcJob's data ends at 0x224 and Job's 8-byte alignment rounds it up. `timestamped_safe_queue` loses 2016's `headTimestamp`. DataModelJob ctor 89.5% → 100% (assert text `arbiter`, line 36). GcJob sleepTime and error 91.7% → 100%. GcJob ctor 93.0% → 99.1%: the remaining diff is ScriptContext+0xf8 against our +0x1c8. GenericJob::error 17% → 70.4%.

**LibraryService** (0x68, assert passes): `markedLibraries` is a `boost::unordered_set`. queueResumeThread 93.75% → 100%; registerDevelopmentLibrary also reached 100%.

**MovingAssemblyStage** (0x28, assert passes): `listHeadB` dropped. AssemblyStage ctor 97.4% → 100%. onEdgeRemoving stays at 96.55% because of the Edge vftable.

**Name** (0x20, assert passes): `sortIndex` is an inline `rbx::atomic<int>`, and the ctor takes `const char* const&`. rbx::atomic gets a separate parameterless ctor, emitted out of line as in 0x004022d0. Name ctor 80.85% → 100% in both targets. declare (50.5% → 27.4%) and getNullName (36.4% → 28.1%) dropped; their diffs are code (`call_once` and so on), not layout, and neither was at 100%. 2013's `declare` also takes `const char* const&`.

**ScriptContext:** only `sandboxThread` was changed, made static: 97.3% → 100%. The full member order is not done.

### Checks

| Check | PLAYER | STUDIO |
|---|---|---|
| decomplint | 338 out-of-order (was 334), 24 duplicate_offset, 3 unicode | 342 (was 340), 16, 3 |
| vtable | 329 found, 167 not matching: 162 match (was 155) | 267 / 121: 146 match (was 139) |
| datacmp | 639 variables, 23 issues (was 565 / 17) | 607 / 25 (was 560 / 19) |
| check_folded | 0 problems | 0 problems |
| addr_padding | 1,654 files, 0 bad markers | |

- The extra lint out-of-order entries come from the markers I added; the extra datacmp issues are boost `stored_vtable` diffs on newly paired variables.
- **PLAYER datacmp crashes** with the same reccmp KeyError 49565 on two `describedClassDescriptor` statics (ClassDescriptor type), which the regenerated libdata now pairs. The PLAYER numbers above come from a scratch wrapper that skips them. The real fix is adding those names to `DATACMP_CRASH` in `tools/ida/gen_reccmp_csv.py`, which is not my file.

### Pipeline
- Regenerated `reccmp/player-*.csv` (pooling P-b3, P-c1, P-c2, P-c3) and `studio-*.csv` (S-b3, S-c1, S-c2, S-c3), all in `build/sweep2`.
- A single-run regeneration lost 82 entries; pooling 4 runs settled it.
- Then ran `app_sweep apply` / `report` / `xrefs` with P-final2 and S-final2 and rescored (P/S-final3).

### Database updates to make (no .i64 opened)
- **Name these (PLAYER, at 100%):**
  - 0x0061c2c0 `RBX::Reflection::DescribedBase::DescribedBase()`
  - 0x00644290 `RBX::ServiceProvider::ServiceProvider()`
  - 0x006f9200 `RBX::VerbContainer::VerbContainer(VerbContainer*)`
  - 0x0061b060 `RBX::GuidItem<RBX::Instance>::~GuidItem()`
  - 0x009b9160 / STUDIO 0x00a6b090 `RBX::Limits::Countable::~Countable()`
- **Name these (PLAYER, not yet proven by a match):**
  - 0x004022d0 `rbx::atomic<int>::atomic()`
  - 0x0070cf60 `RBX::DataModelArbiter::DataModelArbiter()`
  - 0x0063ee60 `RBX::DataModel::GenericJob::GenericJob`
  - 0x00615850 `rbx::trie<T*,50>::trie`
  - 0x006154a0 `object_pool<trie::Node>::construct`
  - 0x00614f60 `trie::Node::Node(int)`
  - 0x009b8f00 / 0x009b8f10 `RbxInterlockedDecrement` / `RbxInterlockedIncrement` (wrappers)
  - 0x0061bbd0 the Instance allocator
- **Fix a wrong name:** 0x0066ee10 is EventSource slot 2 (`raiseEventInvocation`), not `RakNet::CCRakNetSlidingWindow::Update`.
- **Import these types** (proven by the 100% ctors, dtors and accessors above):

| Type | Size | Layout |
|---|---|---|
| Debugable | 4 | vfptr (dump) |
| Object | 4 | vfptr: dtor, getClassName |
| EventSource | 4 | vfptr, 3 slots |
| DescribedBase | 0x10 | EventSource @0, weak_ptr @4, descriptor @0xc |
| GuidItem<Instance> | 0x14 | vfptr, `shared_ptr<Registry>` @4, Guid @0xc |
| Registry | | enable_shared_from_this @0, map @8, mutex @0x14 |
| Limits::Countable | 8 | `shared_ptr<Counter>` |
| Instance | 0x60 | bases and members as listed above |
| ServiceProvider | 0x8c | |
| VerbContainer | 0x14 | |
| Verb | | Debugable, name @4, container @8 |
| ClassDescriptor | 0x20c | |
| MemberDescriptorContainer | 0x60 | trie @0x10 |
| Name | 0x20 | |
| DataModelJob | 0x218 | |
| LibraryService | 0x68 | |
| MovingAssemblyStage | 0x28 | |
| GenericJob | 0x248 | inferred from operator new |
| DataModelArbiter | 0xa20 | inferred from layout |

- **FW bytes:** FWInstance's fw+0x18/0x19/0x1a bytes are parent-lock, robloxLocked and archivable (FWInstance ctor 0x0061d7e0). Our FWBase puts b21..b23 one byte later; that is FW's owner's to fix.

### Files outside my list that I had to touch
Consequences of the base-class changes: `Client/App/include/V8DataModel/ChatService.h` (removed `useSubmitTaskForLuaListeners`), `Client/App/v8xml/XmlElement.cpp` (2013 "RBX%d" referent), `Client/App/v8xml/SerializerV2.cpp` (no setXmlId), `Client/Base/include/rbx/threadsafe.h`, `Client/Base/include/rbx/atomic.h`.

## Structured fields

```json
{
 "files": [
  "Client/Base/include/rbx/Debug.h",
  "Client/Base/util/Debug.cpp",
  "Client/Base/include/rbx/Countable.h",
  "Client/Base/include/rbx/atomic.h",
  "Client/Base/include/rbx/threadsafe.h",
  "Client/App/include/Util/Limits.h",
  "Client/App/util/Limits.cpp",
  "Client/App/include/Util/Guid.h",
  "Client/App/include/Util/Object.h",
  "Client/App/include/Util/IMetric.h",
  "Client/App/include/Util/Name.h",
  "Client/App/util/Name.cpp",
  "Client/App/include/reflection/object.h",
  "Client/App/include/reflection/Event.h",
  "Client/App/include/reflection/member.h",
  "Client/App/reflection/reflection_object.cpp",
  "Client/App/include/V8Tree/Instance.h",
  "Client/App/v8tree/Instance.cpp",
  "Client/App/include/V8Tree/Service.h",
  "Client/App/v8tree/Service.cpp",
  "Client/App/include/V8Tree/Verb.h",
  "Client/App/v8tree/Verb.cpp",
  "Client/App/include/V8DataModel/DataModel.h",
  "Client/App/v8datamodel/DataModel.cpp",
  "Client/App/include/V8DataModel/DataModelJob.h",
  "Client/App/v8datamodel/DataModelJob.cpp",
  "Client/App/include/V8DataModel/ScriptContext.h",
  "Client/App/include/V8DataModel/ChatService.h",
  "Client/App/include/script/LuaLibrary.h",
  "Client/App/script/LuaLibrary.cpp",
  "Client/App/include/V8World/MovingAssemblyStage.h",
  "Client/App/v8world/MovingAssemblyStage.cpp",
  "Client/App/v8xml/XmlElement.cpp",
  "Client/App/v8xml/SerializerV2.cpp",
  "reccmp/player-symbols.csv",
  "reccmp/player-libdata.csv",
  "reccmp/studio-symbols.csv",
  "reccmp/studio-libdata.csv",
  "notes/app-sweep.csv",
  "notes/app-sweep-xrefs.csv",
  "build/sweep2/P-final3.json",
  "build/sweep2/P-final3-nolib.json",
  "build/sweep2/S-final3.json",
  "build/sweep2/S-final3-nolib.json",
  "build/sweep2/phase4-lint-PLAYER.txt",
  "build/sweep2/phase4-lint-STUDIO.txt",
  "build/sweep2/phase4-vtable-PLAYER.txt",
  "build/sweep2/phase4-vtable-STUDIO.txt",
  "build/sweep2/phase4-datacmp-PLAYER.txt",
  "build/sweep2/phase4-datacmp-STUDIO.txt",
  "build/sweep2/phase4-folded-PLAYER.txt",
  "build/sweep2/phase4-folded-STUDIO.txt"
 ],
 "unresolved": [
  "PartInstance::resize (PLAYER 0x00720810, STUDIO 0x007bc260) dropped from 100% to 90.48% and is now a STUB. Its old match relied on the wrong Instance vftable cancelling PartInstance's own errors. Fixing it needs PartInstance.h, which is not mine: add hitTestImpl at +0x78 and setLinearVelocity/setRotationalVelocity at +0xc4/+0xc8, and remove the non-2013 virtuals setPartSizeUi/setPartSizeUnjoined.",
  "Other entries that lost 100% (PLAYER 16, STUDIO 19) have no layout cause. 4 RakNet functions per target: the immediate 0x7fffff now coincides with our data symbol describedClassDescriptor+215 (no .reloc). The rest is twin-body pairing churn from regenerating the CSVs (exception_ptr dtor, _Allocate, set<BaseScript*> tree, shared_from_this, getSingleton<ContentId>/<SoundId>, too_many_refs sdd). Also the HumanoidChangedSignalData sdd at 0x004027d0, which is slot 0 of the ICombinedSignalData vftable 0x00affc84, so both are lost together.",
  "reccmp-datacmp PLAYER crashes with KeyError 49565 on two describedClassDescriptor statics, which the regenerated libdata now pairs (ClassDescriptor type, the same bug as `root`). Fix: add a describedClassDescriptor___ prefix to DATACMP_CRASH in tools/ida/gen_reccmp_csv.py (not my file). Today's PLAYER datacmp figures come from a scratch wrapper that skips them.",
  "DataModel members 0xac8-0xe10 are still not laid out; the 0x0064ec50 ctor needs a line-by-line read. No DataModel size assert yet. STUDIO's member order at 0xc60-0xcac differs from PLAYER's.",
  "ScriptContext member order is not done; only sandboxThread was made static. The ctor 0x0068ad20 read so far: bool @0x64, WeakThreadRef @0x70, set<BaseScript*> @0x8c, Time @0x98, vectors @0xa0/@0xb0, scoped_ptr @0xc8, safe_queue @0xcc, bools @0xf0-0xf3 (robloxPlace 0xf1), gcFrequencyBase near 0xf8, timeout block 0xa4-0xcc, size 0x288. GcJob ctor (99.13%) waits only on gcFrequencyBase at 0xf8.",
  "Diagnostics::Countable<T>'s ctor increments count in 2013 (Instance ctor 0x00620efc calls RbxInterlockedIncrement 0x9b8f10). Changing the header needs SoundChannel, ThreadRef, ScriptEvent and ScriptContext to drop their hand-written increments, which spans files outside my list.",
  "DataModelArbiter+0x830 is a shared_ptr to a zeroed 4-byte object, probably the Limits::Counter that DataModelJob::step's Activator uses, but not proven; it is named m_unk0x830. OnScreenProfiler hooks in preStep/postStep (0x0070cbd0/0x0070cc10) are not in the tree, so those stay at 53.85%.",
  "Our FWBase byte fields b21-b23 sit one byte later than 2013's: 2013 has fw+0x18 = parent lock, +0x19 = robloxLocked, +0x1a = archivable, name at +0x1c (FWInstance ctor 0x0061d7e0). FW is not on my list; setRobloxLocked/setIsArchivable stay at about 9% until FWValue<bool>::set lands.",
  "Name::declare (2013 signature `const char* const&`) and getNullName dropped (50.5% to 27.4%, 36.4% to 28.1%). The diffs are code (call_once), not layout, and neither was ever at 100%.",
  "Instance::onDemandWrite (94.87%) differs only in its assert line, 1090 in 2013 against 1159 in ours; Instance.cpp is about 69 lines longer than 2013's. VerbContainer::removeVerb (92.59%) needs its assert on line 65 while addVerb's stays on 58, which the two-line marker blocks make impossible without breaking brace style.",
  "Edge/Joint vftables (getEdgeType +0x10, a Joint slot +0x38) are not on my list; MovingAssemblyStage::onEdgeRemoving stays at 96.55% because of them.",
  "CLAUDE.md needs these additions (I may not edit it): MSVC places polymorphic bases first, in declaration order, and empty non-polymorphic bases after them (Instance RTTI 0x00b2a6ec). App classes derive Debugable (dump at slot 0, no virtual dtor), and Verb does too. rbx::atomic<int>'s parameterless ctor is emitted out of line (0x004022d0)."
 ]
}
```

