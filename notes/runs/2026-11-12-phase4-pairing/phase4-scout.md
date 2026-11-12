# phase4-scout (workflow phase4-pairing, wf_4baec7d9-68d)

I checked the 2013 layouts of the Phase 4 classes against the original bytes. **Instance, its bases, DataModel, ScriptContext, the reflection containers, the job classes, LibraryService and MovingAssemblyStage are laid out wrongly in our headers.** Descriptor, Type, EnumDescriptor, TaskScheduler::Job, AssemblyStage and the rbx::signals classes already match.

**No fact below is proven by a 100% function.** The only core-class FUNCTIONs at 100% are ClassDescriptor::operator== 0x00614c30 and two getStageType bodies, and none of them reads a member. Everything is read from the original bytes: ctor stores, operator new sizes, RTTI base offsets, and how reccmp lines our offsets up against the originals. STUDIO matches PLAYER for every class here, except that DataModel orders a few members in 0xc60–0xcac differently (same size, 0xe10).

There are two outputs. `tools/class_layout.py` prints, per class: the RTTI bases with offsets, each vftable with the iOS name beside every slot, every access through `this` in the ctors and dtors, the operator new sizes, and which member of ours covers each offset (from our PDB). It also has a `--dispmap` mode that pairs original and our offsets from a reccmp `--json`; its selftest passes. `notes/class-layouts-2013.csv` has 1,833 rows: the raw facts for both targets plus 58 hand-checked conclusions, 43 of them marked WRONG.

The fixes below are ordered by how many PLAYER sweep functions each one touches. Those counts come from reccmp diffs where our offset and the original differ, so they are lower bounds.

**1. RBX::Instance and its bases** (22+ functions, all below 90%, plus every member access in derived classes):
- **Base order.** Declare the bases as `Limits::Countable, Diagnostics::Countable<Instance>, Debugable, GuidItem<Instance>, AbstractFactoryProduct<Instance>, Reflection::Described<Instance,sInstance,DescribedBase,27,0>, boost::noncopyable`. The compiler then places them at Debugable 0, GuidItem 4, AbstractFactoryProduct 0x18, Described 0x1c, Limits::Countable 0x2c, Diagnostics::Countable 0x34. Evidence: the RTTI of 0x00b2a6ec and the ctor 0x00620ed0.
- **Debugable** gets `virtual void dump(std::ostream&)` (slot 0, 0x009b1a70) and no virtual destructor, so it is 4 bytes. Ours is empty.
- **Limits::Countable** must not be virtual. It is 8 bytes {count, holder}; its ctor 0x009b94d0 stores only zeros. Ours declares `virtual ~Countable()`.
- **GuidItem** holds `boost::shared_ptr<Registry> registry` (8 bytes) at +4 and the Guid at +0xc, for a size of 0x14. Our intrusive_ptr makes it 0x10. Evidence: the dtor 0x0061b060 and Registry::unregister 0x0058dd90.
- **DescribedBase** derives `enable_shared_from_this<DescribedBase>` (+4) and `EventSource` (+0, vftable 0x00b2a060), keeps `descriptor` at +0xc, and is 0x10 bytes with no xmlId. Evidence: its ctor 0x0061c2c0 stores the EventSource vftable, and ~Instance 0x0061d510 restores it. This contradicts the existing header comment that says there is no EventSource base.
- **Members:** parent 0x38, children 0x3c (findChildIndex 0x0061b1d0), isSettingParent 0x44, fwBase 0x48 (`fw()` 0x006beaa0 reads it), onDemandPtr 0x50, combinedSignal 0x54, ancestryChangedSignal 0x58 (by elimination), propertyChangedSignal 0x5c. Size 0x60, against ours 0x4c.
- **Drop the `archivable` and `robloxLocked` bools.** In 2013 they live in the FWInstance object: setIsArchivable 0x0061d7b0 reads it through fw, and setRobloxLocked 0x0061d780 calls `FWValue<bool>::set`.
- **ServiceProvider** then needs nothing else. Its members are already in the right order and land at 0x60–0x8c (size 0x8c).

**2. Edge/Joint vftables** (22 functions, 1 near miss; not on your list). `getEdgeType` is slot +0x10 in 2013 and +0xc in ours, and a Joint slot is +0x38 against ours +0x24. This is the real cause of the MovingAssemblyStage::onEdgeRemoving near miss (0x008eb4f1, 96.55%), not a stage member.

**3. MemberDescriptorContainer<T>** (13 functions). `descriptorLookup` is `rbx::trie<T*,N>`, which `Client/Base/include/rbx/trie.h` already provides, not DenseHashMap. The proof is the lookup's ctor at 0x00615850: two object pools (one with chunk size 0x1a4, exactly sizeof(trie::Node); one with chunk size 4) and the root at +0x38 built by `construct(1)`. That makes the container 0x60 instead of 0x40 and ClassDescriptor 0x20c instead of 0x16c. ClassDescriptor's tail (security, derivedClasses, base, flags at 0x1f0–0x208) is already in the right order.

**4. DataModel** (12 functions; near misses 0x0062d840, 0x0064a350, 0x0064a870, 0x0064f7f0; also close and toggleToolsOff).
- **Bases**, in this order: DescribedNonCreatable<…ServiceProvider…> at 0, VerbContainer 0x8c, IDataState 0xa0, **IMetric 0xa4 (ours lacks it)**, DataModelArbiter 0xa8, Diagnostics::Countable<DataModel> 0xac8.
- **Size and members:** 0xe10, from operator new in 0x0064f760, against ours 0x1510. Members start at 0xac8. Mapped so far: isInitialized 0xbe5, suppressNavKeys 0xcdd, genericJobs[11] 0xcf8, placeID 0xdcc, and the field we call guiBuilder at 0xdec.
- **DataModelArbiter** is 0xa20, not 0x1238. At 0x830 it holds an 8-byte shared_ptr where ours has `ActivityMeter<2> stepActivity`; the lookup tables follow at 0x838 (ctor 0x0070cf60).

**5. ScriptContext** (12 functions; near miss 0x0066fd10).
- The IScriptFilter vfptr is at 0x60 and members start at 0x64. Size 0x288 (new in 0x00421700), against ours 0x268.
- robloxPlace is at 0xf1, against ours 0x260. Partial map from the reccmp diffs: libraryService 0x258, luaAllocator 0x260, waitingThreads 0xcc, and the GC frequency field at 0xf8.
- `sandboxThread` is static: the original ends in `ret`, ours in `ret 4`. That fixes 0x0066fd40 (97.3%).

**6. Job classes** (0x0064f7f0, 0x00679c00).
- DataModelJob is 0x218: taskType at 0x210 and isPerPlayer at 0x214, with no stepsAccumulated and no profilingToken.
- GenericJob is 0x248, not 0x268 (`new 0x248` three times in doDataModelSetup). Its task queue is 0x28 bytes, against ours 0x30.
- GcJob keeps the ScriptContext weak_ptr at 0x218 and frequency at 0x220.
- TaskScheduler::Job is correct.

**7. LibraryService** (0x0088dd00, 93.75%). `markedLibraries` is `boost::unordered_set<std::string>` (0x18), not `std::set`; the iOS ctor and dtor confirm the type. resumeCallbacks moves to 0x5c and the size to 0x68.

**8. MovingAssemblyStage** (0x008ea620, 92.11%). Its member is a `boost::intrusive::list<Joint>` with the size at 0x10 and the list head at 0x14/0x18. Drop `listHeadB`; size 0x28. AssemblyStage (0x1c) is already correct.

**9. Name** (0x006136c0 at 80.85%, 0x00613530). `sortIndex` is an inline `rbx::atomic<int>`, constructed by an out-of-line ctor at 0x004022d0, not `int*`. The ctor signature is `Name(const char* const&)`.

**Also:** the 2013 tree has `rbx::signals` and no Notifier/Listener classes, so our signal headers are the right family. One side finding outside your list: in DataModel::close and toggleToolsOff the original reads Verb fields at +0xc and +0x18 where ours reads +8 and +0x14, so Verb has one extra word before them.

**Suggested database changes** (I opened no `.i64`):
- **Name these functions:** 0x0061c2c0 DescribedBase ctor, 0x00615850 trie ctor, 0x006154a0 trie node `construct`, 0x004022d0 `rbx::atomic<int>` ctor, 0x0061bbd0 Instance allocator, 0x00644290 ServiceProvider ctor, 0x0063ee60 GenericJob ctor, 0x0070cf60 DataModelArbiter ctor.
- **Wrong slot names in the names export:** 0x0066ee10 (EventSource slot 2) is named `RakNet::CCRakNetSlidingWindow::Update`. Shared one-instruction bodies such as 0x004027c0 carry one arbitrary name.

**Housekeeping:** the tool caches its type dumps in `build-class-layout/`, about 48 MB per target. I read the PDBs from scratch copies of `build/` taken at 12:44.

## Structured fields

```json
{
 "files": [
  "tools/class_layout.py",
  "notes/class-layouts-2013.csv"
 ],
 "unresolved": [
  "Most DataModel members are unmapped (0xac8\u20130xe10). Only isInitialized, suppressNavKeys, genericJobs, placeID and the field at 0xdec are placed. STUDIO also orders the members at 0xc60\u20130xcac differently from PLAYER.",
  "Only part of ScriptContext's 2013 member order is mapped, from reccmp pairings. Writing the full header needs the whole ctor at 0x0068ad20 read line by line.",
  "DataModelArbiter: I could not identify the type that +0x830's shared_ptr points to, or the last 4 bytes at 0xa1c.",
  "GenericJob's task queue is 0x28 bytes in 2013 and 0x30 in ours; I did not find which 8 bytes are missing.",
  "GcJob's size of 0x224 is inferred from its ctor stores; I found no operator new for it.",
  "Instance's ancestryChangedSignal at 0x58 is assigned by elimination, not by a function that uses it.",
  "The trie's maxDepth template argument is not determined.",
  "I did not analyze the TaskScheduler class itself, Security::Context (it has no vftable) or the EnumDesc<T> template layouts. The tool looks our classes up by name and misses template classes, because the type dump spells them differently.",
  "Fixing Edge/Joint vftables (getEdgeType at +0x10, a Joint slot at +0x38) is outside this task's class list but unblocks 22 functions.",
  "2013's atomic.h assert sits on line 224 with a different expression than ours, so matching assert line numbers needs the 2013 layout of that header.",
  "Database changes listed in the summary are suggestions only; no .i64 was touched."
 ]
}
```

