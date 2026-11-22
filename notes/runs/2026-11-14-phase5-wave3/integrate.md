# integrate (workflow phase5-wave3), 2026-11-22

I integrated all five phase5-wave3 Work agents (shell-5k, network-5f, datamodel-5b-3, gui-5h-2, world-5d), regenerated the CSVs, ran the sweep and reordered markers. Both targets are stable at the end:
- The last two scorings of each target agree exactly.
- A final regeneration round gained nothing.

Every heavy command ran through tools/capped.sh. I ran one build at a time and nothing hit the memory cap.

## What I did

**Leftovers.** All of the wave's uncommitted work (about 190 paths) compiles in both targets from clean, so I kept all of it and reverted nothing. That includes the two agents that died at the usage limit; their final reports came in through the task text. The deleted BasePlayerGui.cpp and CoreGuiService.cpp leave the globbed source lists by themselves.

**Clean rebuilds.**
- PLAYER: cmake configure, `--target clean`, then a full build (679 steps, 0 errors).
- STUDIO: the same, in build-studio with `--target studio`.

**Link stubs.** I deleted each exe first, then ran gen_link_stubs for each target. The stubs went from 5 to 8 addresses in each target:
- Added: the four `TypedPropertyDescriptor<Vector2int16>` string/xml slots (PLAYER 0x006fffc0, 0x00700000, 0x00700050, 0x00704a00).
- Dropped: `ogre_stub_009b1890` (STUDIO 0x00a68980), because Log.cpp now defines `Log::writeEntry`. This was datamodel-5b-3's request. writeEntry(char) 0x009b1890 and writeEntry(wchar_t) 0x009b1920 are now at 100%.
- unresolved.txt now also lists:
  - FLog::GfxClustersFull, JointLifetime and Network
  - `CRenderSettings::aaSamples`
  - `CRenderSettings()` and `SystemUtil::getVideoMemory`. gen_link_stubs found no oracle address for these two, so CRenderSettingsItem() 0x004567d0 stays at 0.925.

**Regeneration rounds.** Each round pools the two latest runs of the current link. In every round the two runs agreed exactly.
- PLAYER: 8,040 → 8,729 → 8,915 → 8,975 → 8,996 → 9,004 → 9,005 → 9,005.
- STUDIO: 7,480 → 8,503 → 8,690 → 8,754 → 8,774 → 8,781 → 8,783.
- The rows network-5f asked for appeared without hand edits:
  - The `FactoryProduct<Client,Peer,…>` rows re-paired, and isNetworkClient 0x0056b8c0 / 0x005d0d70 is at 100%.
  - The RTTI `??_7` rows paired the Replicator item vftables: PingBackItem, PingItem and MarkerItem in both targets (PLAYER 0x00b2611c / 0x00b26b1c / 0x00b26b70) are at 1.0. So is the PingBackItem deleting dtor 0x005e9e70.
  - Shell-5k's 22 RenderSettingsItem.cpp FUNCTION markers per target pair. All 44 shell FUNCTION markers measure 100%.

**Sweep.**
- Ran `app_sweep set FUNCTION`, scored both targets, then ran `apply`. Result:
  - PLAYER: 991 FUNCTION + 43 FOLDED + 672 STUB (all swept dirs).
  - STUDIO: 958 + 43 + 668.
- **tools/app_sweep.py, two fixes:**
  1. `set` and `apply` no longer rewrite Client/Rendering/ markers (Division of Work). `report` still reads them.
  2. `set FUNCTION` used to strip FOLDED. Without it, a folded group's markers collide ("Dropped duplicate address") and none of them scores, so `apply` then turned the whole group into STUB. In this run that cost the engine `xor al,al; ret` group, PLAYER 0x00927cc0 / STUDIO 0x008db930: 6 markers each in Joint.h, Item.h, GuiBase2d.h and Reflection.h, about 130 PLAYER vftables. I restored those 12 markers to `FUNCTION: … FOLDED`, as in HEAD, and `set` now keeps FOLDED. After the restore PLAYER is back at 9,005 and STUDIO at 8,783.
- The sweep found 64 more PLAYER markers at 100%, but they add no entries: CSV rows had already paired every one of them.
- check_folded's ScreenGui 0x008b1d90 problem (world-5d) is gone; `apply` made both markers FOLDED.

**Reorder.**
- `reorder_markers apply` on Client/Network/Peer.cpp (PLAYER descents 1 → 0) and Client/App/v8tree/Service.cpp (3 → 1). Both targets were rebuilt (full rebuilds, because the sweep had touched header comments).
- **STUDIO** scored 8,783 again, with 0 lost.
- **PLAYER lost 6 Lua library functions to a no-.reloc coincidence:** symbexec, luaV_execute, constructor, patchtestreg, removevalues, luaK_setreturns, at 97.8–99.9%.
  - --verbose on patchtestreg 0x009a6800: our `and eax, 0x7fc000` now reads as `??_R4Creator@?$FactoryProduct@VSkateboardPlatform…` (gui-5h-2 saw the same).
  - The relink moved .rdata by one page, so that page-aligned Complete Object Locator now sits at 0x7fc000.
  - None of the six was at 100% in the sweep8 baseline, and the same six flip with every relink, so I kept the reorder.
  - PLAYER is 8,999 after the reorder. Two more regeneration rounds and a final scoring each gave 8,999.
- Not reordered:
  - Everything else is pinned by asserts, crosses runs, or has studio-only chunks.
  - type.cpp: the plan says 2 → 0, but it breaks the compile with C2908, as before.
  - LuaInstanceBridge.cpp: needs a private member.

## Final figures (sweep8 baseline in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| Entries at effective 100% | **8,999** (7,784) | **8,783** (7,687) |
| Non-library entries | 6,537 (5,374) | 6,363 (5,329) |
| **Functions at 100%** | **6,635** (5,664) | **6,451** (5,544) |
| **Non-library functions at 100%** | **4,240** (3,321) | **4,078** (3,233) |
| Functions at 100%, owner side (App + Network + Base + Log + shells) | 4,224 | 4,068 |
| Functions at 100%, Rendering (minus g3d) | 2 | 3 |
| Functions at 100%, third party (library rows, 3rdparty/, RakNet, g3d) | 2,409 | 2,380 |
| Vftables at 100%: owner / Rendering / third party | 2,283 / 1 / 11 | 2,275 / 1 / 7 |
| reccmp Implemented, with libraries | 8,367 / 40,548 (7,383 / 40,534) | 8,117 / 42,382 (7,200 / 42,377) |
| reccmp Implemented, --nolib | 5,707 / 37,848 (4,760 / 37,866) | 5,545 / 39,684 (4,673 / 39,719) |
| Vftables at 100% | 2,295 / 3,288 (2,051 / 3,110) | 2,283 / 3,217 (2,094 / 3,039) |
| App markers: FUNCTION + FOLDED + STUB | 852 + 39 + 624 (633 + 31 + 686) | 827 + 39 + 618 (612 + 31 + 684) |
| Base / Network markers: FUNCTION + FOLDED + STUB | 84 + 0 + 26 / 52 + 2 + 9 | 76 + 0 + 29 / 52 + 2 + 9 |
| Shell markers (ClientBase etc.), FUNCTION, all at 100% | 22 | 22 |
| Sweep scope: markers at 100% / short | 1,031 / 666 (703 / 713) | 999 / 662 (680 / 710) |

How the split was made: by the `.obj` the link map puts at each recompiled address (build/sweep9/split.py). Library-flagged rows count as third party. The Rendering side has only 3 FUNCTION markers outside g3d, so its own count is still tiny; G3D's 172 library rows count as third party.

**Rows** (PLAYER, current ranges; sweep8 on the same ranges in brackets):
- 5a: 137/154 (131), 89.0%
- 5b: 509/1,605 (447), 31.7%
- 5d: 251/1,504 (124), 16.7%
- 5e (Client/App/script/*.cpp): 255/848 (254), 30.1%
- 5f: 488/3,259 (422), 15.0%
- 5g (row_coverage_5g.py): 300/601 (299), 49.9%
- 5h: 465/1,339 (147), 34.7%
- 5k: 506/3,493 (333), 14.5%

Per-function CSVs are in build/sweep9/row-*.csv.

**Near misses** (app_sweep report, build/sweep9/app-sweep.csv):
- PLAYER, 666 short:
  - By shape: inlining-size 215, ebp-frame 187, other 161, unpaired 52, x87-vs-sse 22, unpaired-callee 22, data-ref 3, assert-file-line 2, stack-cookie 2.
  - 61 are at 90% or more; 25 are pairing-only.
- STUDIO, 662 short:
  - By shape: inlining-size 213, ebp-frame 186, other 167, unpaired 40, unpaired-callee 26, x87-vs-sse 22, stack-cookie 3, data-ref 3, assert-file-line 2.
  - 75 are at 90% or more; 29 are pairing-only.
- Closest: LegacyContentTable ctor 0x008f08a0 99.97, ~DataModel 0x0064d850 99.76, asyncRequest 0x00870430 99.59, ~TaskScheduler 0x009b4e00 99.52, TaskScheduler::remove 0x009b52b0 99.49, ~Job 0x009ba1f0 99.28, ServiceProvider::onChildAdded 0x006a9770 99.19, openBrowserWindow 0x0069a540 99.08.

## Every 100% lost against sweep8 (read with --verbose)

**PLAYER: 11 lost, 1,226 gained.**
- **Pairing only: our folded body pairs with the other original twin.**
  - Lighting::onChildChanged 0x00706920 (0.94): the FUNCTION marker became STUB. Its call to `signal_with_args<1,void(bool)>::operator()` is folded with the lua_State copy in our build and pairs with that original. This is the same mechanism as Instance::raisePropertyChanged; the original keeps a separate copy.
  - `_Tree<Name*,ICreator*>::erase` 0x004b4860 (0.97): `_Eqrange` pairs with the Verb map's twin.
  - ThreadPoolData::getNextTask 0x0082bb90 (0.5): the tail jmp to `safe_queue::pop_if_present` is unpaired.
  - BoundYieldFuncDesc<DataModel,…> deleting dtor 0x009137d0 (0.91): its dtor callee pairs with the BoundFuncDesc twin.
  - `enable_shared_from_this<YieldFunctionStateObject>::_internal_accept_owner` 0x005bd080 (0.98): `weak_ptr::operator=` pairs with the BillboardGui twin.
  - The four BoundFuncDesc deleting-dtor twin vftables 0xb2a760, 0xb2f3a8, 0xb39088, 0xb785d8 (0.5). Shell-5k reported these.
- **SimulateStage::putFirstMovingRootInSendPhysics 0x00998920 (0.985):** the FUNCTION marker became STUB. World::getSendPhysics reads +0x74 where 2013 reads +0xb0, so it folds with UnifiedWidget::getMenuStateForTest. world-5d reported this; it needs the World layout.
- **boost iostreams `chain_impl::~chain_impl` 0x005f3110 (0.977):** a `mov eax, imm` no-.reloc coincidence.

**STUDIO: 39 lost, 1,135 gained.**
- 16 `PropDescriptor<T,bool|int|float|string>` vftables (0.93). Slot 4 isReadOnly is a folded shell copy in the original; shell-5k reported this.
- 6 Settings / GlobalAdvancedSettings vftables 0xbf2068–0xbf22d8 (deleting-dtor thunk twin) and the 4 BoundFuncDesc D0 twin vftables (shell-5k).
- EnumDesc<TextService::XAlignment/YAlignment/FontSize/Font>::convertToValue 0x0073c110–0x0073c200 (0.86). gui-5h-2 traded these on purpose: they now call `singleton()` out of line, the 2013 shape PLAYER has.
- copy_on_write_ptr<vector<shared_ptr<Instance>>> 0x005cb770 / 0x005cb7e0, vector copy ctor 0x00987ae0, and `_Insert_n` 0x00988840: the vector copy ctor / ~vector callee pairs with another original twin.
- The same twins as in PLAYER: 0x007fa090 `_Tree::erase`, getNextTask 0x009223a0, BoundYieldFuncDesc D0 0x0093ea50, `_internal_accept_owner` 0x007515d0.
- No-.reloc coincidences: RakNet IsOlderOrderedPacket 0x0065f060 (an immediate equals a Team RTTI COL, network-5f) and iostreams `~chain_impl` 0x0068b950.

## Checks

- **decomplint:**
  - PLAYER: 276 function_out_of_order and 12 duplicate_offset (baseline 242 / 13).
  - STUDIO: 284 and 4 (baseline 253 / 5).
  - The rise comes from the wave's new files. Every remaining descent in them is pinned by asserts or crosses runs (reorder_markers plan).
- **reccmp-vtable:** no traceback. "Problem with adjuster thunk" lines: 11 PLAYER, 13 STUDIO (as in the baseline).
- **reccmp-datacmp:** no traceback. PLAYER 1,719 variables, 51 issues; STUDIO 1,667 variables, 53 issues.
- **check_folded:** PLAYER 2,302 annotated addresses, 0 problems; STUDIO 2,165, 0.
- **reccmp_addr_padding:** 1,669 files, 0 bad.
- **check_2016_copies:** silent.

## Saved

- Final JSONs: build/sweep9/P-final.json and build-studio/sweep9/S-final.json.
- Alongside them: P/S-nolib.json, -vtable.txt, -datacmp.txt, -folded.txt and -decomplint.txt.
- Scored exe, pdb, map and pdbview: build/sweep9/scored/ and build-studio/sweep9/scored/.
- Every round's JSONs and gen logs: build/sweep9/ and build-studio/sweep9/.
- Helper scripts, also in build/sweep9/: score.sh, gen.sh, round.sh, summ.py (lost/gained by original address), split.py, stubcheck.py, v.sh.

## Files changed

- tools/app_sweep.py
- reccmp/player-*.csv and reccmp/studio-*.csv (boost, crt, g3d, libdata, libzip, lua, raknet, symbols, zlib)
- Client/_generated/{player,studio}/stubs_00.cpp and unresolved.txt
- Client/Network/Peer.cpp and Client/App/v8tree/Service.cpp (reordered)
- Marker words in the swept App/Base/Network files, and the FOLDED restore in Joint.h, Item.h, GuiBase2d.h and Reflection.h

## Unresolved

- **Link stubs.** CRenderSettings::CRenderSettings (PLAYER 0x009c5ce0) and SystemUtil::getVideoMemory (0x009b59b0) have no oracle row that gen_link_stubs can match. They stay /FORCE:UNRESOLVED and land on one address, so CRenderSettingsItem() 0x004567d0 is at 0.925. Two ways to fix it:
  - A names.csv row, from the IDA owner.
  - The real definitions: CRenderSettings is the Rendering contributor's; getVideoMemory is Base.
- **Twin pairing that no CSV regeneration settles:**
  - Lighting::onChildChanged 0x00706920 and Instance::raisePropertyChanged need the shell's copies of `signal_with_args<1,…>::operator()` (Base signal.h owner, shell code).
  - BoundFuncDesc D0 vftables.
  - The STUDIO PropDescriptor isReadOnly vftables and Settings vftables need more shell code (ReflectionMetadata's BoundProp instantiations).
  - The vector<shared_ptr<Instance>> copy ctor / ~vector twins, `_Tree::erase`, getNextTask.
- **No-.reloc coincidences:** Lua's `0x7fc000` mask equals our SkateboardPlatform Creator COL in PLAYER, which drops 6 Lua functions to 97.8–99.9%. iostreams `~chain_impl` and RakNet IsOlderOrderedPacket (STUDIO) are in the same state. They flip with any relink.
- **World layout:** putFirstMovingRootInSendPhysics 0x00998920 (world-5d, next pass).
- **Reorder:** type.cpp (C2908) and LuaInstanceBridge.cpp were not reordered. decomplint keeps 276 / 284 out-of-order descents (pinned, cross-run or studio-only) and 12 / 4 duplicate_offset.
- **The Work agents' own Unresolved lists are still open** (shell-5k, network-5f, datamodel-5b-3 reports; gui-5h-2 and world-5d in the task text). Examples:
  - The IAdornableCollector definition for Rendering.
  - UIEvent.h ownership.
  - Instance::fw out of line.
  - The Joint step/move hook bases on SpanningEdge.h.
  - The Players / Player / Replicator layouts.
- **IDA sync has not run** for this wave. The new match: names and the regenerated pairings are not in ida/*.i64 yet.
