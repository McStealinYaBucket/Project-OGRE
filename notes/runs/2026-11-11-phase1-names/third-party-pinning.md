# third-party-pinning (workflow phase1-names, 2026-11-11)

Third-party pinning for the October 2013 builds, from raw files only (no canonical .i64 opened, nothing saved or patched). PLAYER = bin/player_1023/RobloxPlayerBeta.exe, image base 0x00400000.

## 1. RakNet: 4.036 lineage, protocol version 5
- **Protocol constant is 5 in both builds.** PLAYER 0x00600fb6 `cmp al,5` (ID_OPEN_CONNECTION_REQUEST_1), 0x00600fc7 `cmp byte ptr [ebp+11h],al`, then writes 0x19 (ID_INCOMPATIBLE_PROTOCOL_VERSION) at 0x00600ff1 and 5 at 0x0060100b, followed by OFFLINE_MESSAGE_DATA_ID at 0x00b27d40. iOS Oct 29, in `RakNet::ProcessOfflineNetworkPacket` (0x00a65130): 0x00a65d02 `ldrb r1,[r12,#0x11]`, 0x00a65d06 `cmp r1,#5`. Upstream 4.081 has protocol 6; the 2016 tree's 4.036 has 5.
- **Bounded to 4.036 ≤ version < 4.063** by the upstream revision log. Lower bound: `PluginInterface2::UsesReliabilityLayer` (added in 4.036, 2012-02-01) is an iOS symbol. Upper bound: iOS has `RakNetSocket` and `SocketLayer` and no `RakNetSocket2`/`RNS2_*` (introduced 4.063); `SetIncomingDatagramEventHandler` (4.067) and `SetInternalID` (4.07) are absent; PLAYER carries a `SocketLayer.cpp` assert path.
- **Symbol set fits the 2016 tree.** Of 305 iOS `RakNet::Class::method` symbols, 282 have the same arity as an out-of-line definition in the 2016 tree and none mismatch; the other 23 are header-inline there. There is no version string in any binary. 4.036 exactly is the best-supported reading but is not byte-proven.
- **Compiled inside the Network project, not a separate library.** RakNet `__FILE__` references interleave with Network TUs in PLAYER .text: FileOperations 0x00570f2e, StringTable 0x005ab391, BitStream 0x005c2bd8–0x005c3061, ReliabilityLayer 0x005c9776–0x005ce59b, RakPeer 0x005faf17–0x00604e8c.
- **Linked subset is small.** PLAYER RTTI has only `RakNet::RakPeer`, `RakPeerInterface`, `PluginInterface2`. iOS has 22 classes (RakPeer, ReliabilityLayer, CCRakNetSlidingWindow, BitStream, SocketLayer, RakNetSocket, RakString, StringCompressor, StringTable, HuffmanEncodingTree, SignaledEvent, SimpleMutex, RakThread and others). No CCRakNetUDT, plugins, TCP, NAT or RPC.
- **Roblox additions confirmed in the 2013 bytes:** FastLog strings "Raknet: AckTimeout", "RakNet: CloseConnection", "RakNet: Calling CloseConnectionInternal, BCS_CLOSE_CONNECTION", "Raknet: connection lost: ..."; `RBX::Network::ProfiledRakPeer::RunUpdateCycle` (iOS 0x00ad71dc), so `RakPeer::RunUpdateCycle` is virtual; `ConcurrentRakPeer.cpp`; the flag-name strings "MarkRakNetSocketOnDelete", "NewRaknetTimestamp", "RakNetDisconnect". The 2016 tree marks Roblox edits in 20 files, which is a hypothesis list for 2013.

## 2. zlib: two copies, both attributed
- **1.2.3 is G3D's bundled zlib.** Banner at 0x00bbf618, crc table at 0x00bbd618, crc32 at 0x009e0420, inflate around 0x009e22e0. It is bracketed by G3D code on both sides (BinaryInput/GImage TGA around 0x009df070, G3D prompt/registry from 0x009e55c0), followed by libzip (0x009e7645 "%s.XXXXXX", 0x009e7d85 "TORRENTZIPPED-") and IJG jpeg.
- **Callers of ZLIB_VERSION "1.2.3" (0x00b29614):** sub_613170 (two references, next to "zlib error", which is boost::iostreams zlib), and libzip at 0x009e7281, 0x009e7843, 0x009ed8cc.
- **1.1.4 is inside the statically linked D3DX9.** Banner at 0x00bd5438, crc table at 0x00bd4fa0. Its .rdata neighbours are D3DX effect-state names, "DisableD3DXPSGP", the PNG signature plus "1.0.5" twice (libpng 1.0.5 at 0x00bd235c), and "File created by Microsoft (R) D3DX library". Code references into that .rdata come from 0x009f9e3b–0x00a0a112, in the D3DX block at the tail of .text (about 0x009ef000–0x00a5b000). This would be the "13.10.4035 prebuilt objects" from the earlier Rich-header note, but I did not re-check the Rich header.
- **Cross-check:** bin/player/RenderSystem_Direct3D9.dll, which also links d3dx9.lib, carries the same zlib 1.1.4 + libpng + IJG 6b set and "D3DX9 Shader Assembler 5.04.00.3900". OgreMain.dll has its own zlib 1.2.3 inside FreeImage.
- Both banners are also present in the Oct 15 Player and in Studio.

## 3. Lua: 5.1.x compiled as C++, 5.1.4 by path only
- **Version evidence:** `_VERSION` string "Lua 5.1" at 0x00b862d0 and the assert path `client\app\lua-5.1.4\src\luaconf.h` (lines 852 and 862, so luaconf.h itself is modified). There is no `$Lua:` ident string in any target.
- **Compiled as C++.** iOS symbols are mangled (`__Z12lua_newstate...`), with a `lua_exception` class (also in PLAYER RTTI) replacing longjmp.
- **Name diff against stock lua-5.1.4** over the iOS Lua block 0x00823f58–0x00835094: 228 of 253 stock externals and 216 of 486 statics are present.
- **Added:** `lua_setreadonly` (PLAYER string "Attempt to modify a readonly table"), `luaL_loadprotected` with static `protReader`, and `RobloxExtraSpace` (ctor, dtor, `Shared`, an `RBX::Intrusive::Set` of them).
- **Removed:** `luaopen_io`, `luaopen_os`, `luaopen_package`, `luaopen_debug` and all their statics; `luaL_openlibs`, `luaL_loadbuffer`, `lua_cpcall`, `lua_equal`, `lua_getallocf`, `lua_setallocf`, `luaL_gsub` and the lua.c/luac.c front ends. PLAYER has library-name strings only for table, string, coroutine (plus math functions).

## 4. Bullet: not linked anywhere in October 2013
Zero `bt*` RTTI, zero "Bullet"/`btGjk`/`btConvex` strings in PLAYER, the Oct 15 Player and Studio; zero `bt*` symbols in the iOS Oct 21/29 and Mac tables. The `App.BulletPhysics` folder in the existing App tree does not correspond to anything in these builds; where it came from I did not determine.

## 5. boost 1.53.0
- **boost.dll exports 69 symbols:** Boost.Thread (`boost::thread`, `this_thread`, `detail::thread_data_base`, tss, `win32::handle_manager`, `notify_all_at_thread_exit`) and Boost.System (`system_category`, `generic_category`, `throws`). Nothing else. Its PDB path is `Client\boostlibs\bin\Release\boost.pdb`. PLAYER has 18 boost.dll import names in its strings; Studio's readable import table has 17.
- **Compiled into the exe:** filesystem (five `boost::filesystem::...` error strings, `windows_file_codecvt`), program_options (22 RTTI, `utf8_codecvt_facet`), iostreams including zlib and gzip (43 RTTI, `zlib_error`, `gzip_error`).
- **Header-only use:** property_tree JSON on spirit::classic, date_time gregorian, format, any, function/bind, intrusive, lexical_cast, exception, `thread_specific_ptr`, smart_ptr, algorithm, math.
- **Absent:** regex, signals (Roblox uses its own `rbx::signals`), asio, serialization, chrono, random, locale.

## 6. Third-party table
| Library | Version | Evidence | Linkage | Targets |
|---|---|---|---|---|
| RakNet | 4.036 lineage, protocol 5 | section 1 | static, in Network | all |
| Lua | 5.1.x (5.1.4 by path), modified, as C++ | section 3 | static, in App | all |
| boost | 1.53.0 | section 5 | thread+system in boost.dll, rest static | Win; fully static on iOS |
| G3D | 8.00 | `sprintf("G3D %d.%02d", 8, 0)` at 0x009d7060–0x009d706b; `G3D::FileSystem::Dir` RTTI | static | all |
| zlib | 1.2.3 | section 2 | static, G3D-bundled | Win exes |
| libzip | unknown, TORRENTZIP-era | "TORRENTZIPPED-", "Zip archive inconsistent", "%s.XXXXXX" at 0x009e7645 | static, after G3D | Win exes, iOS |
| IJG libjpeg | 6b (27-Mar-1998) | banner, one message table | static, after G3D | Win exes |
| D3DX9 static | version string not in exe; "5.04.00.3900" in RenderSystem_Direct3D9.dll | "D3DX: (WARN)...", effect states, "DisableD3DXPSGP" | static, tail of .text | PLAYER, Oct 15 Player, STUDIO |
| zlib | 1.1.4 | section 2 | inside D3DX9 | Win exes, RenderSystem_Direct3D9.dll |
| libpng | 1.0.5 | PNG signature + "1.0.5" twice at 0x00bd235c | inside D3DX9 | Win exes |
| DirectShow BaseClasses | n/a | RTTI `CBaseFilter`, `CSource`, `CSourceStream`, `CBasePin`, `CAMThread`; WMVCore.DLL import | static | PLAYER |
| ATL | VC9 | 33 RTTI (`CAxDialogImpl`, `CComObject`, ...) | static | PLAYER |
| Ogre | 1.6.4 per the plan | OgreMain.dll import; 83 `Ogre::` RTTI incl. Roblox subclasses | DLL | Win; static on iOS |
| FreeImage, zziplib, OpenEXR, libpng 1.2.41, zlib 1.2.3 | n/a | strings in OgreMain.dll | inside OgreMain.dll | Win; FreeImage/FreeType/zzip static on iOS |
| FMOD Ex | per the plan | fmodex.dll, "Incorrect FMODEX DLL version" | DLL | Win; static on iOS |
| Intel TBB | 2.x | tbb.dll, `__TBB_machine_trylockbyte` | DLL | Win |
| CRT/STL | VC9 | MSVCR90/MSVCP90 imports | DLL | Win |
| Crash reporting | n/a | `CrashReporter`, `RobloxCrashReporter` RTTI; `MiniDumpWriteDump` | own code + dbghelp.dll | Win |
| HTTP | n/a | `WinHttpRequest_source`, `WiniInetRequest_source` | OS DLLs | Win |
| DirectInput 8, WinTrust/Crypt32 | n/a | import names | OS DLLs | PLAYER |

Not present in PLAYER by string or RTTI: breakpad, tinyxml, rapidxml, rapidjson, expat, hunspell, libcurl, OpenSSL, sqlite, freetype, protobuf, Bullet. XML and JSON are Roblox's own (`XmlParser`, `TextXmlParser`, `SimpleJSON`) plus boost property_tree. iOS-only extras: FreeType (103 `FT_`), FreeImage (103), zziplib, TestFlight (`tf_`), protobuf-c strings.

## 7. PLAYER RTTI namespace census
5,843 unique class/struct/enum descriptors, 5,841 demangled (5,612 class, 129 struct, 100 enum).

| Top-level namespace | Count |
|---|---|
| RBX | 4068 |
| boost | 942 |
| rbx | 561 |
| (global) | 88 |
| Ogre | 83 |
| std | 36 |
| ATL | 33 |
| G3D | 21 |
| RakNet | 3 |
| HardwareID | 2 |
| ResourceGroupHelper | 2 |
| FLog | 1 |
| anonymous | 1 |

Second level under RBX (namespace, enclosing class or template): Reflection 2277, directly in RBX 1371, FactoryProduct<> 168, Network 68, HUMAN 20, Diagnostics 13, Stats 10, Instance 9, Scripting 8, CRenderSettings 7, Voxel 6, TaskScheduler 5, DS 5, DataModel 5, Soundscape 5, Tasks 4, TextService 4, GuiObject 4, Intrusive 3, Lua 3, Camera 3, Feature 3, then about 60 scopes with 1–2 each. The tool prints the full list.

- Inside Reflection: PropDescriptor 526, BoundFuncDesc 288, Described 273, EventDescImpl/EventDescBase/EventDesc 138 each, TypedPropertyDescriptor 115, BoundProp 102, EnumDesc 100.
- Inside boost: detail 466, _bi 256, exception_detail 67, iostreams 43, program_options 22, intrusive 20, spirit 14.
- Inside rbx: signals::signal 269, callable 188, icallable 81, trie 10.

## Tools written
All read-only, each with a docstring: `tools/rtti_census.py` (uses llvm-undname), `tools/pe_strxref.py`, `tools/c_symdiff.py`, `tools/cpp_symdiff.py`. Stock lua-5.1.4 was downloaded to the scratchpad only and never built or copied into the repo.

Upstream sources used: [RakNet revision log](https://raw.githubusercontent.com/facebookarchive/RakNet/master/Help/revisionlog.html), [RakNet 4.081 RakNetVersion.h](https://raw.githubusercontent.com/facebookarchive/RakNet/master/Source/RakNetVersion.h), [lua-5.1.4.tar.gz](https://www.lua.org/ftp/lua-5.1.4.tar.gz).

## Files

- `tools/rtti_census.py`
- `tools/pe_strxref.py`
- `tools/c_symdiff.py`
- `tools/cpp_symdiff.py`

## Unresolved

- RakNet is bounded to 4.036 <= version < 4.063 with protocol 5; exactly 4.036 is not byte-proven. The public repo only has 4.081, so a pristine 4.036 archive is needed to diff against the 2016 tree and body-match.
- Lua 5.1.4 versus 5.1.5 is not distinguished by bodies; 5.1.4 rests on the luaconf.h path string. Compiling stock Lua for a symbol or body comparison was blocked by the permission classifier, so only a source-regex name diff was done.
- Whether the zlib 1.1.4 and libpng 1.0.5 code inside the static D3DX9 is live in PLAYER: the 1.1.4 crc table (0x00bd4fa0) and the PNG signature (0x00bd235c) have no imm32 references from .text.
- The exact DirectX SDK release of the static d3dx9.lib: the string "5.04.00.3900" appears in RenderSystem_Direct3D9.dll but not in the exe. The link to the 13.10.4035 Rich-header objects is inferred, not re-checked.
- libzip version is unknown; it has no version string.
- PLAYER's import table is not parseable by llvm-readobj (VMProtect); the boost.dll import list for PLAYER comes from name strings (18), while Studio's readable table shows 17.
- The detailed RTTI census and link-order mapping were done for PLAYER only. For the Oct 15 Player and Studio only the banner strings were confirmed, and the RakNet protocol byte was not checked in Studio.
- Two of the 5,843 RTTI descriptor names did not demangle with llvm-undname and are missing from the census counts.
- tools/pe_strxref.py attributes references using ida/exports/PLAYER-functions.csv, which is known to be incomplete, so function names and starts for hits can be off even though the reference addresses are exact.
- Origin of the App.BulletPhysics folder in the existing App tree is undetermined; nothing in the October 2013 binaries corresponds to it.
