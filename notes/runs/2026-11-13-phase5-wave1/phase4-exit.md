# phase4-exit (workflow phase5-wave1, wf_322cba92-6a4)

CLOSE PHASE 4: Instance's vftables and the reflection templates. Not all exit criteria are met. Two of Instance's four vftables reach 100% in both targets, but only after the generator change below is used to regenerate the CSVs. The Debugable and GuidItem vftables are blocked by about 30 markers in other agents' files. Most reflection descriptor templates have no instantiation at 100%.

Baselines: PLAYER 4,348/40,468, 1,522 non-library functions at 100%, 200 vftables at 100%. STUDIO 4,222/42,330, 1,472 non-library, 187 vftables.

== Exit criterion: vftables, under CSVs regenerated with my generator, on my final builds (PLAYER b3, STUDIO b1S) ==
- {for Described} (PLAYER 0x00b2a6c4, STUDIO 0x00bed484): 0.667 -> 1.0 in both targets.
- {for AbstractFactoryProduct} (PLAYER 0x00b2a6d4, STUDIO 0x00bed494): 0.5 -> 1.0 in both targets.
- {for Debugable} (PLAYER 0x00b2a6ec, STUDIO 0x00bed4ac): 0.571 -> 0.714, 20 of 28 slots.
- {for GuidItem} (PLAYER 0x00b2a6e0, STUDIO 0x00bed4a0): still 0.
- Also 1.0: FWInstance 0x00b2a234, FWPVInstance 0x00b4bce8, FWFinal<FWInstance> 0x00b2a8b4, AbstractFactoryProduct<Instance> 0x00b2a2f0.
- With the repo CSVs as they are now, the four Instance vftables are unchanged from baseline (0.571 / 0.667 / 0.5 / 0).

Why the remaining slots stay red: the original keeps two copies of each trivial body (one in the client shell at 0x40xxxx, one in the engine). Our link folds them into one, and reccmp pairs one original per body of ours. Markers in other agents' files hold our single bodies at the engine copy:
- `ret 4` (needs 0x004027c0): FOLDED markers at 0x00575d00 in IAdornable.h:114-129, SpanningTree.h:70, IndexedTree.h:24-33, Item.h:76.
- `xor al,al; ret` (needs 0x00402730): STUB markers at 0x00927cc0 in IAdornable.h:77-83, Reflection.h:128/175, Item.h:62, Joint.h:117/120; ProfanityFilter.cpp:24 at 0x006690f0; Joint.cpp:127 at 0x00820fb0.
- `ret 8` (needs 0x00402740): AdornBillboarder.cpp:128 at 0x009d5660; IAdornable.h:132 and SpanningTree.h:74 at 0x008c8450.
- `ret` (needs 0x0040b950): Adorn.h:90-99 at 0x009d55f0; IndexedTree.h:21/36, Item.h:60, Edge.h:84 at 0x00528720; Dragger.cpp:15 at 0x008e1ac0.
- What-if, private, on the baseline build: with the 30 markers at these addresses neutralized, Debugable reaches 0.929 and 6 more vftables reach 100%. The rest is then held by the Joint.cpp/Dragger.cpp STUBs and an older 0x008c8450 CSV row.
- Twin slot counts in the original: `ret 4` 3,620 slots vs 890; `xor al,al; ret` 781 vs 1,788; `ret 8` 499 vs 233. Among Instance-derived vftables the client copy wins every group.
- Tested: the VC9 linker folds /GL bodies with non-/GL bodies in a small test, so I did not find why the original kept them apart.

== Changes, with evidence and scores ==

1. Util/Object.h: Object and AbstractFactoryProduct are no longer novtable.
   - Evidence: the Instance ctor stores 0x00b2a2f0 at 0x00620df9; ~Instance stores 0x00b29f08 at 0x0061d684.
   - Line count kept at 368, so the asserts in that file did not move.
   - Results with the repo CSVs: ~Instance 0.928 -> 0.985, Instance(FWInstance*) 0.830 -> 0.895, Instance(const char*, FWInstance*) 0.828 -> 0.912.
   - With regenerated CSVs plus change 4: ~Instance 1.0 in both targets; both ctors at effective 100% in both targets.
   - Markers flipped to FUNCTION in Instance.cpp: PLAYER 0x00620da0, 0x00620ed0, 0x0061d510; STUDIO 0x006f55e0, 0x006f5710, 0x006f1f40. These depend on Integrate regenerating the CSVs.

2. CMakeLists.txt: BOOST_THREAD_DYN_LINK and BOOST_SYSTEM_DYN_LINK for every engine library (ogre_prior_sources).
   - Evidence: the original calls all 18 boost.dll imports (IAT 0x00aa2d00-0x00aa2d44) as `call [imp]`, at 146 sites in client, Network, App and Base. It has no jmp thunk for any of them.
   - Security::Context::current 0x0053ac40: 0.897 -> 1.0 with regenerated CSVs (STUDIO 0x005e9cb0 also 1.0); 0.931 with the repo CSVs, where reset<> is still unpaired.
   - This removes our instantiation of std::map<void const*, tss_data_node>. About 10 originals whose match: names were that map's names lose their pairing until the names are re-exported (for example 0x0085ede0, 0x0085f680, 0x00493590). The bodies are still in our build under other names, so this is pairing churn, not lost code.

3. Util/FeatherweightDict.h: the SYNTHETIC marker 0x0061c320 / STUDIO 0x007be5f0 moved to FWPVInstance with nameref RBX::FWPVInstance::`scalar deleting destructor'.
   - Evidence: FWPVInstance's vftable slot 0 is the same body (PLAYER 0x00b4bce8 -> 0x0061c320, STUDIO 0x00c08110 -> 0x007be5f0), and both our links keep the FWPVInstance PUBLICS name.
   - FWInstance D0: unpaired -> 1.0 in both targets. FWInstance vftable 0x00b2a234: 0 -> 1.0.

4. Client/Base/rbx/Log.cpp: the 12 Base log channels are now defined, in 2013 registration order, with GLOBAL markers (PLAYER 0x00db4084, 0x00e1aa88-0x00e1aa92; STUDIO 0x00f2f9d4, 0x00f9dc78-0x00f9dc82).
   - Channels: Crash, HangDetection, ContentProviderCleanup, ISteppedLifetime, MutexLifetime, TaskScheduler, TaskSchedulerInit, TaskSchedulerRun, TaskSchedulerFindJob, Asserts, FWLifetime, FWUpdate.
   - Evidence: initializers at PLAYER 0x00a8b4d0-0x00a8b630. All 12 were in the unresolved-symbol list, FLog::Asserts included.
   - This gives ~Instance its last data reference (FWLifetime at 0x0061d598), which takes it from 0.992 to 1.0.

5. Client/Base/rbx/TaskScheduler.cpp: asserts moved onto their 2013 lines (339 -> 337 and 557 -> 535). I moved two NOTE comments and removed 20 blank lines; all 6 asserts now sit on their 2013 lines.
   - STUDIO results: remove 0.962 -> 0.995, ~TaskScheduler 0.962 -> 0.995, findJobToRun 0.557 -> 0.624, TaskScheduler ctor 0.955 -> 0.972.

6. tools/ida/gen_reccmp_csv.py, for Integrate to run. I did not write reccmp/.
   - (a) Vftables pair by the original's RTTI name: an rtti: `??_7` row whose exact symbol is in our PDB. This pairs 2,829 of 7,333 vftables in PLAYER.
   - (b) Adjustor thunks are now pairable as slot functions. This fixes Instance's Described and AbstractFactoryProduct slot 0 (0x007bc400 and 0x00958d00).
   - (c) A library row on an identical-twin body yields to the twin that more vftable slots name. Example: 0x00475080 yields to 0x0070c430 (Instance::askForbidParent, 4 slots). The yielded original is kept out of every data_rows pass.
   - Clean A/B: same frozen source, baseline build and JSON, old logic vs new.
     - PLAYER: 4,312 -> 4,363; functions +52 / -2; vftables at 100% 202 -> 517.
     - STUDIO: 4,184 -> 4,223; functions +40 / -2; vftables 189 -> 585.
     - The losses are the Point D0 0x00978890 / 0x00a420c0 and one yielded library twin per target. The Point D0 had been 100% only because the wrong vftable was paired by position; with the RTTI row, its real difference shows: our body stores KernelIndex's vftable where the original stores Point's.
   - Recommendation: run the generator twice, the second pass with a reccmp run made on the first pass's CSVs, so the newly paired vftables also vote on twins. In STUDIO, the std logic_error/length_error/out_of_range vftables (0x00bb681c-0x00bb6834) fell to 0.5 when the private regeneration paired our D0 with 0x00550920 instead of 0x00403be0. With the repo CSVs they stay at 100%.

7. New tool tools/template_vtables.py: reports, per class template, how many of its 2013 vftables are paired and how many are at 100%.

== Totals ==
- PLAYER, final build b3, includes other agents' concurrent edits:
  - Repo CSVs: 4,414 / 40,469; non-library 1,522 -> 1,591; 7 lost, all pairing churn or other agents' edits (onAncestorChanged/onDescendantAdded lost a callee: shared_from<Instance> now shares a body, and so a name, with shared_from<GuiItem>; that callee's CSV row looks up the old name).
  - Regenerated CSVs: 4,602 / 40,520; non-library 1,724; 529 vftables at 100%. All 20 losses read with --verbose: callee twin or name churn, the tss map names from change 2, and Point D0.
- STUDIO, build b1S:
  - Repo CSVs: 4,237 / 42,330; non-library 1,472 -> 1,532.
  - Regenerated CSVs: 4,408 / 42,369; non-library 1,655; 599 vftables at 100%.
  - Lost to another agent's EnumConverter.h edit (03:44): 6 EnumDesc<T>::convertToValue functions.

== Templates: instantiations at 100% (PLAYER, regenerated CSVs; STUDIO is the same unless noted) ==
- At 100%:
  - DescribedCreatable: 26 of 120.
  - DescribedNonCreatable: 15 of 49 (STUDIO 14).
  - FactoryProduct: 37 of 491.
  - NonFactoryProduct: 57 of 261 (STUDIO 53 of 257).
  - AbstractFactoryProduct: 1 of 1.
  - EnumDesc: 7 of 9.
  - TType: 36 of 36.
  - RefType: 2 of 2.
- Described and GuidItem: no vftable of their own exists in 2013. They appear only as Instance's {for X} tables (status above).
- None at 100%:
  - BoundFuncDesc 0/34, GetSetImpl 0/20, FuncDesc 0/5, BoundPropGetSet 0/12, BoundYieldFuncDesc 0/3 (best 0.0).
  - PropDescriptor 0/19, TypedPropertyDescriptor 0/13, BoundProp 0/9 (best 0.786).
  - EventDesc 0/18 (0.143), EnumPropDescriptor 0/4 (0.1), GetImpl 0/6 (0.25), RefPropDescriptor 0/6 (0.118), YieldFuncDesc 0/2 (0.5).
  - Never paired: EventDescImpl, EventDescBase, RemoteEventDesc(Impl), CallbackDesc(Impl), BoundCallbackDesc, SetImpl, MemberDescriptorContainer.

I did not bring the missing reflection templates to 100%: their headers are not in my ownership.

IDA names for the next sync (PLAYER):
- 0x0061c320 RBX::FWPVInstance::`scalar deleting destructor' (FWInstance's too), match:
- 0x007bc400 and 0x00958d00: Instance `vector deleting destructor' adjustor thunks {0x1c} and {0x18}
- 0x00a8b4d0-0x00a8b630: dynamic initializers of the FLog channels listed in change 4

## Files changed

- Client/App/include/Util/Object.h
- Client/App/include/Util/FeatherweightDict.h
- Client/App/v8tree/Instance.cpp
- Client/Base/rbx/Log.cpp
- Client/Base/rbx/TaskScheduler.cpp
- CMakeLists.txt
- tools/ida/gen_reccmp_csv.py
- tools/template_vtables.py

## Unresolved

- Integrate: regenerate reccmp/ with the updated tools/ida/gen_reccmp_csv.py, using pooled fresh runs of the current builds and ideally a second pass. The 6 flipped Instance markers (PLAYER 0x00620da0, 0x00620ed0, 0x0061d510; STUDIO 0x006f55e0, 0x006f5710, 0x006f1f40) were measured at (effective) 100% only with the regenerated CSVs. With today's repo CSVs they read 0.91 / 0.92 / 0.99.
- Instance {for Debugable} vftable (PLAYER 0x00b2a6ec, STUDIO 0x00bed4ac; 0.714) and {for GuidItem} (0x00b2a6e0 / 0x00bed4a0; 0) need the marker owners to release our folded trivial bodies: the FOLDED markers at 0x00575d00 (IAdornable.h, SpanningTree.h, IndexedTree.h, Item.h), the STUBs at 0x00927cc0, 0x006690f0 (ProfanityFilter.cpp:24), 0x00820fb0 (Joint.cpp:127), 0x009d5660 (AdornBillboarder.cpp:128), 0x009d55f0 (Adorn.h:90-99), 0x00528720, 0x008e1ac0 (Dragger.cpp:15) and 0x008c8450. Several of these STUBs sit at addresses whose originals are a different size (for example 0x009d55f0 is 11 bytes, 0x009d5660 is 40), so they are probably wrong. Once released, the client-shell twins 0x004027c0 / 0x00402730 / 0x00402740 / 0x0040b950 can pair.
- Object vftable 0x00b29f08 has only 1 slot in 2013: 0x00b29f0c holds 0, and a string starts at 0x00b29f10. So the pure getClassName() const belongs to Creatable/AbstractFactoryProduct, not Object; the Object.h comment that says otherwise is wrong. Moving it is a full rebuild of both targets and was not done; Object's vftable stays at 0.5.
- Reflection (not my files): the 2013 FunctionDescriptor vftable has 2 slots, the deleting destructor and execute (for example BoundFuncDesc<Instance,...> 0x00b2a760: next COL at +8). The executeCustom virtual in Reflection.h:2514+ is a 2016 addition and makes every BoundFuncDesc/FuncDesc vftable mismatch. EventDesc slot 0x0c is the `xor al,al; ret` twin (see the marker item above).
- Step 4 not done. GlobalAdvancedSettingsItem<T,Name> belongs in V8DataModel/GlobalSettings.h: its singleton assert is at GlobalSettings.h:132 (GlobalBasicSettingsItem at :176). Our GlobalSettings.h is an empty file. Its shape is the 2016 one:
- Base DescribedCreatable<T, GlobalAdvancedSettings::Item, Name>, ctor 0x0085e1b0, plus Service (isPublic byte at +0x60).
- Ctor 0x0085e220 calls Instance::setName directly and throws runtime_error("singleton %s already exists") through 0x009b5b20 and throw info 0x00cd66cc.
- sing for LuaSettings is 0x00de68a8.
- The singleton locks a boost::mutex at GlobalAdvancedSettings+0xac, which needs Settings = ServiceProvider(0x8c) + std::string settingsFile + bool. Settings.h is not mine and is a stub.
- LuaSettings currently derives GlobalAdvancedSettings directly, which is wrong.
PhysicsSettings (singleton 0x00654470) has no class in the tree.
- TaskScheduler ctor 0x009b55e0 (0.972): the atomic.h:182 class (ctor 0x005ba6f0) is still unnamed. Its assert text is identical to the line-224 one, which suggests two class definitions in 2013 atomic.h, but there is no name evidence. Also missing are two `mov byte [esp+0x4e/0x4f], bl` empty temporaries around the intrusive list inits at +0x34/+0x40; writing sleepingJobs()/waitingJobs() or value_traits() explicitly does not produce them. Job/Thread/Arbiter ctor/dtor not reached.
- In the original, map<void const*, tss_data_node> bodies are generic map helpers that ICF shared (callers: AdornRbxGfx, GuiService...). The match: names in ida/exports/PLAYER-names.csv still carry the old tss names; the names need re-exporting after this build, or about 10 rows stay unpaired.
- TaskScheduler.cpp and Instance.cpp have pre-existing function_out_of_order lint errors (Instance.cpp 447/844/1083; TaskScheduler.cpp 188/208/266/395).
