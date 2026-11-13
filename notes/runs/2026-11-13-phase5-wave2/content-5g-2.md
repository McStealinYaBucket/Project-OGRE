# content-5g-2 (workflow phase5-wave2, wf_cba1073c-3f1)

Phase 5g, second pass (content, Http, WinHttp, WinInet). Everything was measured on my own builds, build-cnt/ for PLAYER and build-cnt-studio/ for STUDIO, against the sweep7 baselines. Final runs: scratchpad/cnt3/P-r4.json and S-r4.json, scored with the exe, pdb and map currently in those two build dirs. Nothing I changed lost a function that was at 100%, but adding WinHttp.cpp moved three functions and two vftables elsewhere off 100% (item 2 under "Whole target").

**Row 5g (PLAYER, tools/row_coverage_5g.py, same 648-function file set both times)**
- Before: 179/648 = 27.6% on build/sweep7/P-final.json.
- After: 231/648 = 35.6% on P-r4.json, so +52.
- By file: Http.cpp 1→10/30, WinHttp.cpp 0→34/111, WinInet.cpp 1→10/33. ContentId 5/6, LegacyContentTable 7/11, ContentProvider 67/162, ContentProviderJob 9/16, AsyncHttpQueue 78/107 and CacheableContentProvider 11/118 are unchanged. InsertService (0/6) and MarketplaceService (0/48) are not started.
- WinHttp.cpp and WinInet.cpp are now in the row, with fixed ranges 0x008239b0-0x00827510 and 0x00827540-0x00828250.

**Whole target vs baseline**
- PLAYER: effective 100% went from 5,972 to 6,145 (non-library 3,790 → 3,968). Implemented went from 6,133/40,527 to 6,479/40,531.
- STUDIO: 5,848 → 6,014 (non-library 3,744 → 3,908). Implemented 5,923/42,371 → 6,231/42,374.
- Inside the 5g ranges plus their library callees: +79 PLAYER and +77 STUDIO functions at 100%, none lost.
- These totals include other agents' concurrent edits. Every function lost elsewhere was traced; none is in my files.
  1. phase4-exit's Client/ClientBase/placeholder.cpp now emits std::runtime_error::what, which is no longer folded with logic_error::what. That puts the std::logic_error/length_error/out_of_range vftables at 50%, and the thread_resource_error bodies 0x0040ded0, 0x0040e5c0 and 0x0040e610 now come from placeholder.cpp.obj.
  2. WinHttp.cpp's legitimate boost::throw_exception<std::logic_error> instantiation now supplies the error_info_injector<logic_error> bodies, built without /GS, before program_options' convert.cpp /GS copy. This loses PLAYER 0x00474630, 0x004746e0 and 0x004748d0 plus vftables 0x00b26db4/0x00b26dd8, and STUDIO vftable 0x00bdeca8. The fix is link order: program_options belongs in the client group ahead of App.
  3. Other agents' work or no-.reloc churn: Lua symbexec/luaV_execute, Teams, GuiMain, Frame, Soundscape, Reflection PropDescriptor vftables.

**Step 1 (TEMPLATE markers): done.**
- All 52 namerefs sit at the end of AsyncHttpQueue.h, ContentProvider.h and ContentProviderJob.h.
- decomplint --warnfail no longer reports byname_function_in_cpp for those files.
- No assert moved off its 2013 line.
- The remaining function_out_of_order errors (AsyncHttpQueue.cpp 138/325/492, ContentProvider.cpp 210-772, Http.cpp STUDIO 445) are the same as at HEAD 3ed7431.
- tools/reccmp_addr_padding.py: 0 badly padded markers.

**Step 2 (WinHttp.cpp, WinInet.cpp): kept and measured.** These are the first attempt's files, which no agent had measured.
- WinHttpRequest_source::read (PLAYER 0x00823e60, STUDIO 0x00919fb0) and WiniInetRequest_source::read (0x00827720 / 0x0091daa0): 100% in both.
- redirectCallback (0x00824fc0) and the WinInet callback (0x00827cc0): 100% in PLAYER, 97.67% and 97.83% in STUDIO. The STUDIO redirectCallback miss is onWinHttpRedirect, which had no STUDIO marker; I added STUB STUDIO 0x006cb3b0 in this pass.
- The two request drivers are structurally identical to the 2013 bytes:
  - httpGetPostWinHttp 0x00826990: 95.44% → 98.75% (STUDIO 98.75%).
  - httpGetPostWinInet 0x00828250: 98.06% → 99.56% (STUDIO 99.56%).
  - What is left is data pairing only (see Unresolved).
- About 140 TEMPLATE markers added to Util/Http.h. Each one pairs a library body our build emits from WinHttp.cpp, WinInet.cpp or Http.cpp with the copy the 2013 linker kept; every pairing was checked by instruction-sequence equality. They cover:
  - ATL: CUrl ctor, CrackUrl, Parse, InitFields, CStringT/CSimpleStringT ctors, dtor, operator=, operator+=, Append, SetString, LoadString, AtlThrowImpl, AtlCrtErrorCheck, AtlUnescapeUrl, CComBSTR(const char*).
  - boost gzip and iostreams bodies whose 2013 copy sits in Network (0x005ecd10-0x005f5510).
  - The map<string,string> const_iterator::_Inc at 0x0041b650.
- Fixed two first-attempt markers. 0x00826600 is chain_base<output>, not input. STUDIO 0x0068b780 moved into the existing seek block.

**Http.cpp**
- New markers:
  - STUB on Http::trustCheck (PLAYER 0x00624210, STUDIO 0x006cd090).
  - GLOBAL Http::accessKey (0x00de3eb4 / 0x00f66e9c), robloxResponceLock (0x00de3e78 / 0x00f66e60) and cdnResponceLock (0x00de3e7c / 0x00f66e64).
  - STUB STUDIO 0x006cb3b0 on onWinHttpRedirect.
- Comment lines were cut so the RBXASSERT stays on 2013 line 100. A first try had moved it to 102 and dropped httpGetPostImpl to 92%; it is now back at 100%.
- Now at 100% in both targets:
  - MutexGuard ctor, getRobloxResponceLock and getCdnResponceLock (they had FUNCTION markers but were at 95.8%/50%).
  - httpGetPostImpl, post, ThrowIfFailure ×2, ThrowLastError, the http_status_error ctors and urlEncode.
- Every PLAYER and STUDIO FUNCTION marker in my 13 files is at effective 100% in P-r4/S-r4 (checked with scratchpad/cnt3/chkmarks.py), and no STUB is at 100%.

**Step 3 (near misses): blocked outside my files.**
- HttpQueueStatsItem ctor 0x0086f270 (87.2%): 2013 calls Instance::fw out of line and then adds 0x1c; our Instance.h inlines fw(), and the vtable-store order differs.
- ContentProviderJob ctor 0x008efdc0 (86.6%): needs rbx::spin_mutex = tbb::spin_mutex in Client/Base/include/rbx/threadsafe.h, which is not my file.
- The rest of row 5g is listed under Unresolved.

## Files changed

- Client/App/include/Util/Http.h
- Client/App/util/Http.cpp
- Client/App/util/WinHttp.cpp
- Client/App/util/WinInet.cpp
- Client/App/util/AsyncHttpQueue.cpp
- Client/App/include/Util/AsyncHttpQueue.h
- Client/App/util/ContentProvider.cpp
- Client/App/include/V8DataModel/ContentProvider.h
- Client/App/util/ContentProviderJob.cpp
- Client/App/include/Util/ContentProviderJob.h
- tools/row_coverage_5g.py

## Unresolved

- Integrate (libdata/CSV), driver data: with these rows both drivers should reach 100% (PLAYER 98.75%/99.56%, STUDIO the same). (1) boost::iostreams::zlib::default_compression 0x00b29588, deflated 0x00b2958c, default_strategy 0x00b29590. (2) __TI3?AVhttp_status_error@RBX@@ 0x00cbb1d8. (3) FFlag::WinHttpDisableCookies PLAYER 0x00e1184c, STUDIO 0x00f96478. (4) Five wide strings in httpGetPostWinHttp that reccmp reads as narrow on the original side: L"Roblox/WinHttp", L"POST", L"GET", L"Accept-Encoding: gzip\r\n", L"Content-Encoding: gzip\r\n".
- Integrate (vtable rows): the boost iostreams template vftables the WinHttp bodies store are unpaired, which keeps about 20 WinHttp/WinInet template bodies at 89-98%. Examples: chain_client<output> 0x00b73ce8, filtering_stream_base<output> 0x00b73eec, filtering_stream<output> 0x00b73d9c and 0x00b73d94.
- Integrate (CSV), still pending from the first pass: rename player-libdata 0x004366c0 and studio-libdata 0x0041b2f0, plus the unique_lock<mutex> ctor rows (PLAYER 0x004365f0, STUDIO 0x0041b220), to the recursive_mutex symbols. Measured earlier at +4 functions per target.
- CMakeLists owner (link order): boost program_options belongs in the PLAYER client group ahead of App. Otherwise WinHttp.cpp's non-/GS throw_exception<logic_error> copy supplies the error_info_injector<logic_error> bodies, which loses PLAYER 0x00474630, 0x004746e0 and 0x004748d0 plus vftables 0x00b26db4/0x00b26dd8, and STUDIO vftable 0x00bdeca8.
- phase4-exit (placeholder.cpp): runtime_error::what now comes from placeholder.cpp.obj and is no longer folded with logic_error::what, so the std::logic_error/length_error/out_of_range vftables sit at 50%. The thread_resource_error bodies (0x0040ded0, 0x0040e5c0, 0x0040e610) also moved to placeholder.cpp.obj and dropped from 100%.
- Base (threadsafe.h): rbx::spin_mutex must be tbb::spin_mutex. This blocks the ContentProviderJob ctor 0x008efdc0 (86.6%) and stepDataModelJob/addTask pop_if_present/push. The TaskScheduler::Job::Stats layout puts timespanSinceLastStep at +0, which blocks ContentProviderJob::error 0x008ef6f0 (91.7%). RBX::utf8_decode 0x009b8100 and the pathToString helper 0x0065fcd0 are missing, which blocks findAsset, requestContentString, setAssetFolder and readContent.
- Instance.h owner: 2013 calls Instance::fw out of line from the HttpQueueStatsItem ctor 0x0086f270 (87.2%), then adds 0x1c for the name. Ours inlines fw().
- ContentProvider ctor 0x00666280 (60%) needs the 884-B cleanup-thread function 0x00664260, which is not yet written: a FindFirstFile '*.*' loop that logs 'Deleting old file number %u', bound with (604800, &clearFinishFlag@+0x84). The ctor stores the thread in the shared_ptr at +0x74. The ctor also shows members at +0x6c/+0x74/+0x7c/+0x84/+0x88, while ContentProvider.h comments say +0x68..+0x84; the header offsets need checking.
- Twin bodies the original keeps separately and our link folds, so they pair only once: the device_wrapper_impl read/write/seek bodies and concept_adapter seek (0x00824910, 0x008279c0, 0x00827980, 0x00827aa0, 0x00827c90), the 30-59-B ??_G deleting dtors, and list<T*>::_Buynode/_Incsize shared with SimulateStage's list<Assembly*> (0x005e48f0, 0x005e7940). Also the near-100 AsyncHttpQueue templates (asyncRequest 99.6%, _Insert_n 99.7%, _Growmap 99.3%), which differ only by a folded shared_ptr/function dtor callee.
- Not started: CacheableContentProvider/Texture/MeshContentProvider (118 functions at 0x0074b270-0x007505c0, mostly LRU-cache templates), InsertService (6 in range) and MarketplaceService (48 in range); their .cpp files are empty stubs. Also not done: AsyncHttpQueue::processRequests 0x0086f9d0 (44%, needs the shared_ptr<RBX::mutex> signature and about 900 more bytes), syncRequest (86%), getContent(callback) tail merge (76.6%), and the TextXmlParser-blocked parseXml (60%).
- Pre-existing decomplint function_out_of_order errors (the same at HEAD): AsyncHttpQueue.cpp lines 138/325/492, ContentProvider.cpp 210/386/434/576/751/772, Http.cpp STUDIO 445.
