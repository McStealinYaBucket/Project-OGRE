# shell-5k (phase5-wave3, 2026-11-14)

Client shell. I kept the leftover CRenderSettingsItem code, deleted the stand-in, and row 5k rose from 333 to 460. Other agents' uncommitted edits were in every build, so the run-to-run numbers mix their work with mine. All "100%" figures below come from my own private regeneration of the reccmp CSVs, not from the shared ones.

**How I measured.** Built PLAYER in build-sh and STUDIO in build-sh-studio. Copied the exe/pdb/map into scratch. Regenerated the CSVs in a private project (scratchpad/proj: symlinks to the repo plus a private copy of reccmp/), then rescored.
- With the shared CSVs as they are, the new shell functions score only 0.48–0.75: the bodies are identical, but the globals they reference (prop_*, the EnumDesc singletons, the vftables) are unpaired. **Integrate has to regenerate the CSVs before the new FUNCTION markers show 100%.**
- Other agents' half-finished edits broke my builds twice before they fixed them (GuiObject.cpp, HumanoidStage.cpp).

**Leftovers.**
- Kept: Client/ClientBase/RenderSettingsItem.h and .cpp (CRenderSettingsItem, 7 CRenderSettings EnumDesc constructors, 16 setters/getters) and the shell entries in CMakeLists.txt. Both targets build, and neither has a 2016 copy.
- Markers: 22 of 25 are at effective 100% in both targets. The other 3 per target are now STUB:
  - EnumDesc<QualityLevel> PLAYER 0x00456050 / STUDIO 0x00436f40, at 0.824. The LTCG register allocation and frame size differ.
  - EnumDesc<ResolutionPreset> 0x00456260 / 0x00437150, at 0.854. The map operator[] copy and its register convention differ.
  - CRenderSettingsItem() 0x004567d0 / 0x004376e0, at 0.925. Two callees are unresolved externals that land on the same address: CRenderSettings::CRenderSettings (PLAYER 0x009c5ce0, GfxBase) and SystemUtil::getVideoMemory (0x009b59b0, Base). There is also one store-order difference.

**The stand-in is deleted** (Client/ClientBase/placeholder.cpp, its CMake entry and the CMake comment). CRenderSettingsItem is a real shell Instance subclass and makes the shell emit the same LTCG copies.
- PLAYER, A/B on one tree, both with private regen: with the stand-in 8,481 functions at effective 100%, without it 8,700.
- 3 functions were lost between the two builds: GuiObject::onPropertyChanged 0x007bfe00, GuiButton::onPropertyChanged 0x007bfe70 and EventReplicatorBase<GuiButton> 0x0092c1c0. They are a twin-pairing change from the GUI agent's GuiObject.cpp edit at 00:13, between my two builds, not from the shell.
- The shell copies 0x004027c0, 0x00402730, 0x00402740 and 0x0040b950 stay at 100%.

**Costs the stand-in had, now:**
- The thread_resource_error bodies are at 100%: PLAYER 0x0040ded0, 0x0040e5c0, 0x0040e610 (baseline 0.45 / 0.65 / 0.49) and STUDIO 0x0040d7a0, 0x0040da00, 0x0040db10.
- Exception vftables below 100% in PLAYER went from 36 to 15. All of logic_error, length_error, out_of_range, lock_error, the gregorian errors and error_info_injector<logic_error> 0xb26db4 / 0xb26dd8 are fixed. Left: the boost::program_options ones, sp_counted_base 0xaff920 (0.8) and error_info_injector<std::exception> 0xb11c70.
- Instance::raisePropertyChanged 0x0061c750: 0.982 → 100%.
- ~FunctionDescriptor 0x00456740: 0.667 → 100%.
- EnumPropertyDescriptor::getDataSize 0x00455830: unpaired → 100%.

**Totals.**
- PLAYER: baseline 7,784 → 8,700 functions at effective 100%; vftables 2,051/3,110 → 2,274/3,278.
- STUDIO (no stand-in, two regen rounds): 7,687 → 8,531 functions; vftables 2,094 → 2,200/3,207.
- Row 5k (tools/row_coverage.py --row 5k, block 0x00401000–0x004803b0): 333/3,493 (9.5%) → 460/3,493 (13.2%).

**What the shell contains (step 1).** 3,493 IDA functions, 2,861 of them unnamed.

| Group | Functions | At 100% |
|---|---|---|
| boost | 366 | 278 |
| RBX::Reflection | 62 | 54 |
| FactoryProduct<...>::Creator::create/getClassName/static_getCreator, App classes | 42 | 1 |
| Name::doDeclare<...> | 30 | 0 |
| std | 24 | 20 |
| rbx::signals | 17 | 8 |
| ATL | 14 | 13 |
| MergeBinder | 6 | 3 |

- Source files named by the shell's __FILE__ strings: WindowsClient\UserInput.cpp, RenderJob.cpp, View.cpp, WebBrowserAxDialog.cpp; Win\DSVideoCaptureEngine.cpp, VideoControl.cpp, LogManager.cpp, UserInputUtil.cpp; ClientBase\ReflectionMetadata.cpp.
- ReflectionMetadata has one Described<...>::classDescriptor static per class: Classes, Item, Class, Properties, Functions, YieldFunctions, Events, Callbacks, Member (0x0040a8b0–0x0040ad30).
- The many App template copies in the shell (Creator::create, doDeclare, ServiceProvider::find<T>, GlobalAdvancedSettingsItem<DebugSettings/GameSettings>::singleton) exist because the shell code calls them. They need the real WindowsClient sources (RobloxView, RenderJob, UserInput).

**The Stats classes are not shell code.** The Stats::Item, StatsService and FrameRateManagerStatsItem vftables (0x00b01920–0x00b01c44) sit after RbxOgre's ViewRbxGfx (0x00b015ac) and Ogre::SharedPtr (0x00b003e0), and their own slots (0x0048e4e0, 0x00485b10, 0x0048d600, 0x0048e530) are in RbxOgre's .text. They belong to the Rendering contributor, not to this task.

**Step 3 (rest of the shell) was not started**; the measurement cycles used up the 90 minutes.

## Files

- /home/thebigw/Documents/Project OGRE/Client/ClientBase/RenderSettingsItem.cpp
- /home/thebigw/Documents/Project OGRE/Client/ClientBase/RenderSettingsItem.h
- /home/thebigw/Documents/Project OGRE/Client/ClientBase/placeholder.cpp (deleted)
- /home/thebigw/Documents/Project OGRE/CMakeLists.txt
- /home/thebigw/Documents/Project OGRE/tools/row_coverage.py
- /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/B/P2.json
- /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/SB/S3.json
- /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/5k-B.csv
- /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/proj/reccmp

## Unresolved

- INTEGRATE: regenerate the CSVs (gen_reccmp_csv.py, pooling 2-3 runs) before trusting the 22 FUNCTION markers per target in RenderSettingsItem.cpp. With the current shared CSVs they score 0.48-0.75 because the data rows are unpaired (prop_* globals, EnumDesc singleton statics such as 0xe272e8, CRenderSettingsItem vftables, map _Buynode). After a private regen they are 100%. Scored copies: scratchpad/B (PLAYER) and scratchpad/SB (STUDIO).
- INTEGRATE / STUDIO: 16 PropDescriptor<T,bool|int|float|string> vftables (0xbed2f4, 0xbed330, 0xbed36c, 0xbf4860, 0xbf494c, 0xbf9d10, 0xbfbeac, 0xc08df0, 0xc08ea4, 0xc17228, 0xc172a0, 0xc348a8, 0xc349b0, 0xc349ec, 0xc4c50c, ...) are at 0.93. Slot 0x04 isReadOnly is a folded shell copy in the original (0x0040eee0); ours has a shell copy for int/bool/string (SB 0x00403f20, from RenderSettingsItem.cpp) and a native copy for the other types (0x0057a140). Re-pointing the studio-libdata row for 0x0040eee0 to the _N symbol did not recover them. The real fix is more shell code that instantiates the other TypedPropertyDescriptor types (ReflectionMetadata's BoundProp<double/int/string>). PLAYER does not show this.
- INTEGRATE: twin-pairing losses, not caused by the shell. STUDIO Settings/GlobalAdvancedSettings vftables 0xbf2068/0xbf2078/0xbf2100/0xbf2110/0xbf22c8/0xbf22d8: deleting-dtor thunk twin 0x00740920 vs 0x0073fce0. BoundFuncDesc D0 twin vftables in both targets: PLAYER 0xb2a760/0xb2f3a8/0xb39088/0xb785d8, STUDIO 0xbed520/0xbf5438/0xbf71c8/0xbf9a30.
- BASE (owner side, Client/Base/include/rbx/signal.h): the 2013 signal_with_args<1,...>::operator() (PLAYER shell copy 0x00456d20, now 0.556) logs FastLog 'Signal with 1 arg executed' when a log channel is set and walks slots through a helper call. Ours has neither.
- CRenderSettingsItem() 0x004567d0 / 0x004376e0 (0.925): CRenderSettings::CRenderSettings (PLAYER 0x009c5ce0, GfxBase, other contributor) and SystemUtil::getVideoMemory (PLAYER 0x009b59b0, Base) are unresolved and land on the same address. They need link stubs (tools/gen_link_stubs.py, relinked with the exe deleted first) or the real definitions. RBX::CRenderSettings belongs in Client/Rendering/GfxBase/include/GfxBase/RenderSettings.h (Rendering contributor); RenderSettingsItem.h holds a stand-in definition until then.
- App owner: RenderSettingsItem.h also declares stand-ins for RBX::GlobalAdvancedSettings::Item and GlobalAdvancedSettingsItem<>, because V8DataModel/GlobalAdvancedSettings.h lacks them. PartInstance::disableInterpolation (PLAYER 0x00df3b55) is private in App's PartInstance.h but public in 2013; the shell binds to it through /alternatename.
- RENDERING contributor: the Stats::Item / StatsService / FrameRateManagerStatsItem / TypedStatsItem<T> vftables (PLAYER 0x00b01920-0x00b01e24) and their own slots (0x0048e4e0, 0x00485b10, 0x0048d600, 0x0048e530) are RbxOgre's, not the shell's.
- Rest of the shell not started (step 3): ClientBase\ReflectionMetadata.cpp (9 classes, classDescriptor statics 0x0040a8b0-0x0040ad30); WindowsClient RenderJob.cpp (RobloxView::RenderJob ctor 0x0044e360), UserInput.cpp, View.cpp, WebBrowserAxDialog.cpp; Win DSVideoCaptureEngine.cpp, VideoControl.cpp, LogManager.cpp, UserInputUtil.cpp. These also hold the ~70 FactoryProduct<...>::Creator / Name::doDeclare / ServiceProvider::find<T> shell copies of App templates.
- boost program_options vftables (11 in PLAYER, 0xbda2c0 and 0xbe6348-0xbe6628) stay below 100% while boost_po is native. Compiling it /GL cost 154 entries last run (integrate.md).
