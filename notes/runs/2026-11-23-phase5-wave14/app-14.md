# Wave 14: App and Base follow-ups (app-14)

- **Owner:** Client/App and Client/Base. For item 2 only, I also edited the FFlag stand-in lines in Client/Network/{Server,CrashReporter,Player}.cpp.
- **Build dirs:** build-ap/ and build-ap-studio/. The private /D_MBCS build is in build-ap/mbcs-build/.
- **Scripts, runs and snapshots:** build-ap/w/ (`snap.sh`, `ab.sh`, `rounds.sh`, `score.sh`, `cmp.py`). The MBCS toolchain and launcher are in build-ap/w/mbcs/.
- **How I measured:** with a private root (build-ap/w/root) and CSVs regenerated from pooled runs of the current link. The shared reccmp/ was not written.
- **Baseline:** sweep20.
- **Caveat:** the shared tree also held the network and shell agents' uncommitted work (Server, Peer, GuidRegistryService, IdSerializer, RenderSettingsItem, GameVerbs, View, Win/placeholder), so their gains and losses are in these totals too.

## Final figures (effective 100%)

| | sweep20 | this wave | net | gained / lost vs sweep20 |
|---|---|---|---|---|
| PLAYER | 11,997 | **12,155** (W3z) | +158 | +170 / −12 |
| STUDIO | 11,378 | **11,520** (S2z) | +142 | +146 / −4 |

- **PLAYER rounds:**
  - W1: items 1 and 2, 12,029.
  - W2: items 3 and 4, the GlobalBasicSettings ctor chain, the converters, getInstance and openState, 12,137.
  - W3: setter operand order, the ~GlobalAdvancedSettings fix and the marker flips. One regen round gave 12,155.
- **STUDIO rounds:** S1 = everything up to W2 (11,499). S2 = W3's tree, 11,520 after one regen round.
- **Comparisons:** build-ap/w/runs/{W1r,W2z,W3z}-vs-s20.txt and S{1r,2z}-vs-s20.txt.

## Changes, with what each raised and lost

### 1. GameSettings and GameBasicSettings: registrars, accessors and full reflection

- **New file `v8datamodel/GameBasicSettings.cpp`; the class moved out of GameSettings.cpp.**
  - Evidence that it is its own object: GameSettings.obj's initializers end at 0x00a6fa50, and three boost category initializers (0x00a6fa90..0x00a6fab0) open a new TU.
- **Registrars:** ClassRegistrar<GameBasicSettings>, plus EnumRegistrar and TypeRegistrar for ControlMode and RenderQualitySetting. They are now defined, and the map shows all three in GameBasicSettings.cpp.obj.
- **2013 reflection, in initializer order** (0x00a6fac0..0x00a6fe40):
  - EnumProps: ControlMode, VideoUploadPromptBehavior, SavedQualityLevel.
  - BoundFuncDescs: GetTutorialState and SetTutorialState (RobloxScript).
  - PropDescriptors: CompletedTutorials (**STREAMING, no security** in 2013), AllTutorialsDisabled, Fullscreen.
  - BoundFuncDescs: InFullScreen and InStudioMode.
  - EventDescs: StudioModeChanged and FullscreenChanged.
- **Out-of-line accessors, from the ios 1029 symbols and the descriptors:**
  - setControlMode, setRenderQuality, setMouseLock, setUploadVideoSetting, setAllTutorialsDisabled, reset, getCompletedTutorials, getTutorialState, setTutorialState, setCompletedTutorials.
  - **`inFullScreen()` is 0x006f61f0, not getFullScreen.** The InFullScreen BoundFuncDesc stores it, and ios has the non-const inFullScreen().
  - getFullScreen() const and setFullScreen are inline. The Fullscreen descriptor stores the shell's copies, 0x004659c0 and 0x004659d0.
  - The getters ControlMode, RenderQuality, UploadVideo, AllTutorialsDisabled and inStudioMode are inline: the descriptors take folded or shell addresses for them.
- **`setFullScreen` now fires `fullscreenChangedSignal(value)` before it stores** (shell 0x004659d0).
- **GameSettings.cpp now has the 2013 reflection** (0x00a6f690..0x00a6fa50):
  - 11 BoundProps
  - EnumProps VideoQuality and ImageUploadPromptBehavior ("Screenshots")
  - EventDesc VideoRecordingChangeRequest (RobloxScript)
  - BoundProp VideoCaptureEnabled
  - setPostImageSetting and setVideoQualitySetting are out of line.
- **Idioms that decided the match:**
  - Enum and bool setters compare `member != value`, which gives `cmp [ecx+X],eax`.
  - getTutorialState stores find()'s result into a **const_iterator**, which gives `mov eax,[eax]; cmp eax,[esi+0x84]`.
  - getCompletedTutorials hoists `end`.
  - setCompletedTutorials names the substr: `std::string tutorialId = ...`.
- **FUNCTION in both targets (all at 100% or effective 100%):**
  - inFullScreen
  - the 5 setters and setMouseLock
  - reset, getCompletedTutorials, getTutorialState, setTutorialState, setCompletedTutorials
  - the 2 GameSettings setters
- **Side gains, PLAYER:**
  - 7 BoundProp/PropDescriptor GetSet vftables and 9 descriptor vftables
  - EnumDesc<UploadSetting>::convertToValue and its singleton
  - map operator[] 0x006f7290 and the _Tree helpers
  - BoundFuncDesc<void(string,bool)> 0x006f8800
  - EventDescImpl 0x006f8a30
- **Lost, STUDIO only:** `EnumDesc<UploadSetting>::convertToValue` 0x0073cfc0 (1.0 → 0.864). Our copy calls `EnumDesc<UploadSetting>::singleton()` out of line where 2013 inlines it. Probable cause: the COMDAT now comes from GameBasicSettings.obj, which globs ahead of GameSettings.obj, while in 2013 GameSettings.obj links first. PLAYER gained the same function.
- **GlobalBasicSettings ctor 0x00658640 is FUNCTION in both** (STUDIO 0x00740dd0):
  - `Settings(const std::string&)` is out of line: FUNCTION PLAYER 0x00657f60 / STUDIO 0x00740700.
  - The settings file is a static `basicSettingsFile()`: FUNCTION 0x00656390 / 0x0073eed0. It returns `getUserDirectory(true, DirAppData, NULL) + "\GlobalBasicSettings_13.xml"` when the directory is not empty.
  - The string reaches Settings through the existing forwarding template ctors, which pair as a side effect: DescribedNonCreatable 0x00658500, Described 0x006583b0 and NonFactoryProduct 0x006581f0. ~Settings 0x00657e00 and the scalar deleting destructor 0x00658710 also pair.
- **`~GlobalAdvancedSettings() {}` is user-declared.** The 2013 deleting dtor 0x00658430 stores the vptrs, which GlobalBasicSettings' 0x00658710 does not. Without the declaration the two thunks folded and cost GlobalAdvancedSettings' vftables 0x00b31458 and 0x00b31468 in both targets; W3 restores them.
  - GlobalSettings.h's asserts stay on lines 132 and 176: a header comment line was joined to make room.
- **Not done:**
  - **GlobalAdvancedSettings ctor 0x00658580.** It ends with `create<Selection>()` (0x00656f10), but our Selection stub is neither a Service nor creatable, so that does not compile. The ctor was reverted.
  - **Game ctor 0x00655e70.** It is 948 bytes with about 15 unwritten callees: LegacyLock, setupDataModel bound into 0x006538d0, 0x00653790 and others. It needs a Game.cpp of its own.

### 2. FFlag stand-ins moved to FastLogSettings.cpp

- **Moved:** DebugLocalRccServerConnection, CrispFilteringEnabled (.data byte 1), ImmediateCrashUploadEnabled and ImmediateHangUploadEnabled. GLOBAL markers in both targets, placed in initializer order between DataModelJobs and CloseDataModel.
- **Network side:** each definition was replaced by a comment plus `FASTFLAG(...)` extern lines, line for line.
  - Server.cpp's assert is still on line 96 (assert_lines shows no deltas).
  - CrashReporter.cpp and Player.cpp keep their line counts.
- **Result:** the map shows all four in FastLogSettings.cpp.obj. No loss.

### 3. Headers for the network and shell agents

- **FriendService.h:** `void playerAdded(int userId)` and `void playerRemoving(int userId)`.
  - They have STUB bodies in FriendService.cpp so callers pair: PLAYER 0x006b36a0 / 0x006afb40, STUDIO 0x008123a0 / 0x0080e860.
  - Both bodies start with `Players::backendProcessing(this, false)`.
- **DataModel.h: `static void TakeScreenshotTask(weak_ptr<DataModel>)`.**
  - It is defined at the end of DataModel.cpp, so no assert moves. It fires screenshotSignal (+0xacc) when lock() succeeds.
  - FUNCTION PLAYER 0x0063e240 / STUDIO 0x0075c0f0.
- **New header `Util/UserInputBase.h` and `util/UserInputBase.cpp` (was 0 bytes).**
  - The class is novtable: it has no COL, and its ctor stores no vptr. Size 0x38, because UserInput's members start at +0x38.
  - Members:
    - ContentId currentCursorId, +0x04
    - TextureProxyBaseRef currentCursor, +0x24 (typedef'd here as shared_ptr<TextureProxyBase>)
    - fallbackCursor, +0x2c
    - signal<void()> cursorIdChangedSignal, +0x34
  - Eight virtuals in UserInput's vftable order (0x00be4d9c): getCursorPosition, getGameCursor, removeJobs, centerCursor, keyDown, setKeyState, setCursorId, renderGameCursor.
  - FUNCTION: ctor 0x0072af80 / 0x0082a870 and setCursorId 0x0072af30 / 0x0082a820.
  - STUB: getGameCursor 0x0072ad90 and renderGameCursor 0x0072aaf0. Both need Adorn's texture interface.

### 4. Base

- **`rbx::signals::signal<Sig>::next` has its 2013 shape** (signal.h; CRLF kept):
  - It copies the item (keep-alive), takes `scoped_lock(mutex())` of the signature, does `item = item ? item->next : head;`, and returns whether item is non-null.
  - That puts a per-signature `call_once` static into next() (2013 0x00439fe0 for bool, 0x00459dd0 for `const PropertyDescriptor*`), so the operator() copies for different signatures stop folding.
  - After regeneration, Lighting's onPropChanged and friends stay at 100% (W2's first run showed them lost only through stale pairing).
  - New pairings include several `signal<...>` and `callable` shell entries, RunService::raiseHeartbeat 0x0072da30, World::destroyJoint and createAutoJoints, and SendPhysics onMovingAssemblyRoot{Added,Removing}.
- **`RBX::isFinite` is inline** in rbx/boost.hpp: `boost::math::isfinite(value)`, plus `return true` for int. MathUtil.cpp's out-of-line copies are gone.
  - Gained: `RunningAverage<double,double>::sample` 0x004612a0 (0.30 → 1.0).
- Losses attributable to item 4: none found.

### 5. /D_MBCS for App, Base and Network (private build, not committed)

- **Method:** a launcher adds `/D_MBCS` to every compile under Client/App, Client/Base and Client/Network, RakNet included (build-ap/w/mbcs/launch.sh and toolchain.cmake). The comparison is M1 against W3, one run each with identical CSVs.
- **Result: +1, −0.**
  - `ATL::CUrl::CUrl` 0x00401930: 0.947 → 1.0.
  - `Http::isMoneySite` 0x00623bb0: 0.851 → 0.858. The `_mbsnbcpy_s` call now matches; the 4-byte frame difference is still there.
  - `ATL::CUrl::Parse`: 0.701 → 0.704.
  - The only other change was Server's vftable, which is the network agent's edit landing between the two builds, not MBCS.
- **Proposed CMakeLists.txt change, for the integrator:**

```diff
-set(OGRE_APP_FLAGS /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /D_SECURE_SCL=0
-    /DNDEBUG /DWIN32 /D_LIB /D_RELEASE)
+# /D_MBCS: the 2016 App/Base/Network vcxprojs are CharacterSet=MultiByte; ATL::CUrl::CUrl 0x00401930
+#   reaches 100% and Http::isMoneySite 0x00623bb0 calls _mbsnbcpy_s only with it (wave 14, +1/-0).
+set(OGRE_APP_FLAGS /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /D_SECURE_SCL=0
+    /DNDEBUG /DWIN32 /D_LIB /D_RELEASE /D_MBCS)
@@
 set(OGRE_BASE_FLAGS /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /D_SECURE_SCL=0
-    /DWIN32 /DNDEBUG /D_LIB /D_WIN32_WINNT=0x0501 /DNTDDI_VERSION=0x05010100 /D_CRT_SECURE_NO_WARNINGS)
+    /DWIN32 /DNDEBUG /D_LIB /D_WIN32_WINNT=0x0501 /DNTDDI_VERSION=0x05010100 /D_CRT_SECURE_NO_WARNINGS /D_MBCS)
```

- **Caution:** OGRE_NETWORK_FLAGS inherits from OGRE_APP_FLAGS, which is what I measured. But G3D, RbxOgre, GfxAdapters and the stubs library also inherit OGRE_APP_FLAGS, and my measurement did not cover them.
  - To limit the define to App, Base and Network, add `/D_MBCS` to OGRE_NETWORK_FLAGS and pass it to the App library separately.
  - Otherwise, measure the inheriting libraries once on the integrated build.

### 6. Converters, getInstance and openState

- **`StringConverter<ProtectedString>::convertToValue` (PLAYER 0x0066f460 / STUDIO 0x00780050, in ProtectedString.cpp) and `<ContentId>` (0x008d3540 / 0x00846e30, in UDim.cpp ahead of the UDim converters):**
  - Both are written and linked; the map shows them in ProtectedString.cpp.obj and UDim.cpp.obj. Their markers stay STUB.
  - reccmp still names both addresses `ogre_stub_0066f460` / `ogre_stub_008d3540`, because the generated Client/_generated/*/stubs_00.cpp still defines those stubs.
  - So genericConvert<ProtectedString> 0x0066f7f0 and genericConvert<ContentId> 0x00701c40 (STUDIO 0x00780350 / 0x0070af80) stay at 0.939. The verbose diff is the call target plus the register-only differences they already had at 0.948 effective.
  - They should come back once `tools/gen_link_stubs.py` is rerun.
- **`ObjectBridge::getInstance(L, index)` is a static inline in LuaInstanceBridge.h.** 2013 calls it out of line.
  - FUNCTION 0x00877530 / 0x009dbc30, with a header marker.
  - newInstance and on_newindex (two sites) now call it.
  - **newInstance 0x0087af00 / 0x009df580: 99.2 → 100%, FUNCTION.**
- **ScriptContext::openState: added 2013's debug-API keep-alive block**, `if (g_unk0x00de8bd9) { lua_getstack ... lua_gethookcount }`.
  - The gate byte has one reader and no writer.
  - openState goes from 0.555 to 0.583, still STUB. Asserts are unchanged: two blank lines were absorbed.
- **DataModel's unnamed group:** not reached.

## Losses against sweep20 at the end, read with --verbose

- **PLAYER (12):**
  - **genericConvert<ProtectedString> and genericConvert<ContentId>:** the generated-stub naming above.
  - **TypedStatsItem<int>::update and TypedStatsItem<bool>::update** (0x0048e7a0, 0x0048e7d0): the call target pairs with a different folded `function0<R>::operator()` twin (`function0<void>` against our `function0<bool>`). Pairing only; W2 gained `function0<void>::operator()` 0x00434c20.
  - **EventDescImpl<Lighting,void(bool)>::connectGeneric 0x00707830 (0.99):** the original's connect is a folded twin named after GameBasicSettings. It now pairs with our separate GameBasicSettings copy. Pairing only; STUDIO has the same loss at 0x007d3620.
  - **Six weak_ptr<DataModel> bind/function entries** (0x0073da60, 0x0073db90, 0x008445f0, 0x008447e0, 0x00844de0, 0x00844f60):
    - Now that TakeScreenshotTask takes weak_ptr<DataModel>, the shell's GameVerbs.cpp (shell agent) binds it. That gives a same-named `/GL` COMDAT, which our link picks for the engine callers too: the map shows only GameVerbs' copy, with `/GS` cookies.
    - In 2013 both copies exist: shell 0x0043a480, engine 0x008447e0.
    - The signature is the 2013 one, so the remaining issue is shell/engine twin selection, which belongs to the shell agent or the integrator.
  - **AsyncHttpQueue::dispatchGenericCallback 0x0086d480 (0.989):** a no-.reloc constant. It was already in W2 at the first run, and there is no code change of mine in that TU.
- **STUDIO (4):**
  - the two genericConverts (generated stubs)
  - UploadSetting convertToValue (TU order, above)
  - Lighting connectGeneric (pairing)
- **Recovered during the wave, as measurement artifacts:**
  - W1's GlobalBasicSettings singleton and UDim2 "gone" entries came from editing marked files after the build.
  - The FriendService Creator twin (0x004200e0 / 0x00422ee0) was lost in W2 and is back in W3.

## Checks

- `check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1695 files, 0 badly padded.
- `assert_lines`:
  - ScriptContext.cpp, GlobalSettings.h, Server.cpp, CrashReporter.cpp, Player.cpp and LuaInstanceBridge.cpp are unchanged against 2013.
  - DataModel.cpp's misalignments were there at HEAD; I appended at the end of the file only.
- `reccmp-decomplint` on GameBasicSettings.cpp, GameSettings.cpp, GlobalBasicSettings.cpp, Settings.cpp, UserInputBase.cpp, FriendService.cpp, UDim.cpp, FastLogSettings.cpp and LuaInstanceBridge.h: clean. ProtectedString.cpp:53 was already a descent.
- **Line endings:** signal.h and boost.hpp were CRLF at HEAD and are CRLF again. A Python rewrite had flipped them; I caught it from the `git diff --stat` size.

## Unresolved

1. **Integrator:**
   - Rerun `tools/gen_link_stubs.py` (delete the exes first). Without it, `ogre_stub_0066f460`, `_008d3540`, `_00780050` and `_00846e30` shadow the new converters, and the two genericConverts stay at 0.939 in each target.
   - Regenerate the shared CSVs. Several new markers pair only after regeneration: the GameBasicSettings descriptor vftables, the forwarding template ctors, and getInstance.
   - Decide on /D_MBCS (diff above).
2. **Shell agent:**
   - **Item-1 header changes:**
     - `GameBasicSettings::inFullScreen()` is the out-of-line 0x006f61f0. getFullScreen() const is inline, and so is setFullScreen, which now signals before it stores.
     - New out-of-line accessors are declared, with inline getters.
   - **The weak_ptr<DataModel> bind twin** (six engine entries listed above): GameVerbs' bind of TakeScreenshotTask now supplies the only copy.
   - `Util/UserInputBase.h` is ready for UserInput.
3. **Network agent:** `FriendService::playerAdded(int)` and `playerRemoving(int)` exist, as STUB bodies so their calls pair. The four FFlags now live in FastLogSettings.cpp, with `FASTFLAG` externs in your three files.
4. **App, next:**
   - Selection as a real Service, then the GlobalAdvancedSettings ctor 0x00658580 (`create<Selection>()` 0x00656f10).
   - The Game ctor 0x00655e70 in a new Game.cpp.
   - FriendService::playerAdded and playerRemoving bodies.
   - UserInputBase getGameCursor and renderGameCursor (these need Adorn texture calls).
   - The rest of openState (0.583).
   - DataModel's 163 unnamed functions.
   - The STUDIO TU-order loss of UploadSetting convertToValue.
5. **IDA owner (names):**
   - 0x006f61f0 is `GameBasicSettings::inFullScreen()`, not getFullScreen (ios 1029). Same for STUDIO 0x00792fd0.
   - basicSettingsFile 0x00656390 / 0x0073eed0
   - advancedSettingsFile 0x00656320 / 0x0073ee60
   - `Settings::Settings(const std::string&)` 0x00657f60 / 0x00740700
   - UserInputBase ctor 0x0072af80 / 0x0082a870 and renderGameCursor 0x0072aaf0 / 0x0082a3e0
   - `ObjectBridge::getInstance` 0x00877530 / 0x009dbc30
   - `DataModel::TakeScreenshotTask` 0x0063e240 / 0x0075c0f0
   - FriendService::playerRemoving STUDIO 0x0080e860
   - the openState debug gate `g_unk0x00de8bd9`
6. **Types to import:** UserInputBase 0x38 (novtable), GameBasicSettings 0x90 (reflection now complete).
