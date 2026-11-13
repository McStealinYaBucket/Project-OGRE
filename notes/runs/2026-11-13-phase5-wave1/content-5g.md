# content-5g (workflow phase5-wave1, wf_322cba92-6a4)

Phase 5g: content providers and HTTP, measured on my own builds (build-cnt/, build-cnt-studio/) against the sweep6 baselines.

**Row 5g coverage (PLAYER, `tools/row_coverage_5g.py`, same file set and ranges both times)**
- Before: 67/475 = 14.1% on build/sweep6/P-final.json. The first run, with the markers the tree had then, read 51/651 = 7.8%.
- After: 104/475 = 21.9% on scratchpad/cnt/P3.json.
- With the CSV fixes listed below (tested in a private reccmp project): 108/475 = 22.7%.

**Whole-target scores (my build vs baseline)**
- PLAYER: effective 100% went from 3,646 to 3,722 (non-library 1,724 → 1,796). This includes other agents' concurrent edits.
- Inside the 5g address ranges: +40 functions at 100%, none lost.
- STUDIO: 3,527 → 3,589 (non-library 1,661 → 1,726). Inside the 5g ranges: +35, none lost.
- Losses outside my files: tss_data_node and Name-map templates, Instance::onAncestorChanged and onDescendantAdded, DataModel::onChildChanged. These are other agents' work or no-.reloc pairing churn, not my code.
- One unexplained loss: vector<shared_ptr<Instance>>::_Buy (PLAYER 0x008b3120, STUDIO 0x00987a10) is no longer paired. Its body is not mine and I could not attribute it.
- Markers in my files now: 24 PLAYER `// FUNCTION:` and 22 STUDIO `// FUNCTION:`, every one at effective 100% in P3.json / S1.json.

**HTTP library facts (from the import tables)**
- No libcurl: the strings `libcurl`, `curl_easy` and `CURLOPT` are not in the binary.
- Both Windows builds import WINHTTP.dll (12 functions) and WININET.dll (15 functions).
- PLAYER calls through the linker's IAT at 0x00aa2c18–0x00aa2c84. Only functions in 0x008239b0–0x00829210 call it:
  - WinHttp: 0x008239b0–0x00826990. The request driver is sub_826990 (2,938 B). `WinHttpRequest_source` RTTI is a boost::iostreams source.
  - WinInet: 0x00827540–0x00828250. The request driver is sub_828250 (3,979 B). The user agent is "Roblox/WinInet".
- These are the 2013 counterparts of 2016's App/util/WinHttp.cpp and WinInet.cpp. Neither file exists in our tree yet.
- urlmon's UrlMkSetSessionOption is only called by the client shell (0x00480424).
- Our Client/App/util/Http.cpp is still the Darwin/Mac reconstruction. The only 2013 Windows assert in it is at Http.cpp:100.

**New code**
- **LegacyContentTable**, rewritten. `normalizeUrl`, `FindEntry` (non-const, as the iOS symbol has it), `AddEntry`, and the 34,734-byte constructor. Its 298 AddEntry pairs were read from the 2013 bytes; PLAYER and STUDIO carry the same table. All four functions are byte-identical in a private compile. Layout: unordered_map @0x00, empty string @0x18, size 0x34.
  - normalizeUrl is at 100% in both targets.
  - This put ContentId::convertToLegacyContent at 100% in both (it was 97.56%).
- **ContentProviderJob**, new header and .cpp, layout taken from the 2013 constructor at 0x008efdc0. sleepTime is at 100% in both; the implicit destructor and two deque/_Construct template bodies are at 100% too.
- **ContentProvider**:
  - Added the 2013 `APPLOG` macro and its statics, `appLog` (0x00de8340) and `appLogLock` (0x00de8344).
  - New `blockingLoadContent` (0x00666ca0) and `isContentLoaded` (0x00666da0). These are 2016 names, confirmed by the 2013 log string "ContentProvider::requestContentString %d".
  - Rewritten from the 2013 bytes: getFile, hasContent, getContentString (its assert sits on PLAYER line 553), isValidRobloxAssetUrl, initAssetFolder, assetFolder, platformAssetFolder, getAssetFile and findLocalFile.
  - Renamed parseContent to parseXml (iOS name) and added the 2013 `catch (std::runtime_error)`.
  - loadContent now binds the id directly, as 2013 does.
  - Added GLOBAL markers (assetFolderPath, assetFolderOnce, PRIORITY_SCRIPT) and new markers on isUrlBad, registerFile, the CachedContent constructor and findHashFile.
- **AsyncHttpQueue**:
  - `checkContentUrl(std::string)` by value (iOS name), at 100%.
  - 0x0086f610 is HttpQueueStatsItem::create, not init. It now goes through Creatable<Instance>::create, and init is inlined into it.
  - Moved the destructor marker to the D1 at 0x00870ad0 and added a SYNTHETIC marker for the D0 at 0x00871000, which is now at 100%.
  - Rewrote syncRequest (75.4 → 84.1).
  - Added markers on the constructor, asyncRequest, setThreadPool, getRequestQueueSize, addAsyncRetryTask, isRequestQueueEmpty, processRequests and the FailedUrl constructor (FailedUrl is now at 100%).
  - Added a VTABLE marker at 0x00b7fbac, now at 100%.
- **Http**:
  - Windows 2013 layout: alternateUrl @0x00, instanceApi @0x1c, url @0x20, headers @0x3c, with inline constructors (ctor 0x00402b50).
  - urlEncode takes std::string by value and is at 100% in both targets.
- TEMPLATE markers for the template bodies that pair: 13 in ContentProvider.cpp, 10 in AsyncHttpQueue.cpp, 4 in ContentProviderJob.cpp.
- New tool: tools/row_coverage_5g.py. It registers a "5g" row (span 0x4000) in row_coverage.py's ROWS table.

**Near misses**
- **ContentId::convertToLegacyContent: fixed** by the source change above (100% in both).
- **AsyncHttpQueue::isUrlBad 0x0086e540: needs a CSV change, not code.** The libdata row names the original's 0x004366c0 (STUDIO 0x0041b2f0) `unique_lock<mutex>::lock`, but its body is the recursive_mutex lock (GetCurrentThreadId plus a recursion count). For the Integrate agent:
  - Change the symbol on both rows to `?lock@?$unique_lock@Vrecursive_mutex@boost@@@boost@@QAEXXZ`. In my private copy this put isUrlBad, onHeartbeat, getRequestQueueSize and isRequestQueueEmpty at 100% in both targets.
  - It loses unique_lock<mutex>::unique_lock at 0x004365f0 (STUDIO 0x0041b220), 88.89%. That row is also recursive_mutex and needs the same rename.
- **LibraryService::ContentReadyHelper 0x0088edf0: not fixable by CSV.** Our one weak_ptr::lock body is already paired with the original's shell copy at 0x00439eb0. The App caller uses the other original copy at 0x0049ccc0, so one recompiled body would have to pair with two originals.

**Unresolved**
See the list below.

## Files changed

- Client/App/util/LegacyContentTable.cpp
- Client/App/include/Util/LegacyContentTable.h
- Client/App/util/ContentProviderJob.cpp
- Client/App/include/Util/ContentProviderJob.h
- Client/App/util/ContentProvider.cpp
- Client/App/include/V8DataModel/ContentProvider.h
- Client/App/util/AsyncHttpQueue.cpp
- Client/App/include/Util/AsyncHttpQueue.h
- Client/App/util/Http.cpp
- Client/App/include/Util/Http.h
- tools/row_coverage_5g.py

## Unresolved

- Integrate (CSV): rename player-libdata 004366c0 and studio-libdata 0041b2f0 to ?lock@?$unique_lock@Vrecursive_mutex@boost@@@boost@@QAEXXZ, and the unique_lock<mutex> constructor rows at PLAYER 0x004365f0 / STUDIO 0x0041b220 likewise. Measured in a private copy: +4 functions per target (isUrlBad, onHeartbeat, getRequestQueueSize, isRequestQueueEmpty).
- Base (not mine), Stats layout: 2013 TaskScheduler::Job::Stats has timespanSinceLastStep at +0 (computeStandardError 0x009b98b0 and ContentProviderJob::error 0x008ef6f0 read [stats+0]); ours puts timeNow first. This blocks ContentProviderJob::error (91.67%).
- Base (not mine), spin_mutex: 2013 rbx::safe_queue's mutex is tbb::spin_mutex, a 1-byte flag. Its ctor is inlined as `mov byte [q+0x20],0` and lock is __TBB_LockByte with backoff at 0x00402240. Ours is rbx::atomic<int>, so the safe_queue ctor is not inlined: ContentProviderJob ctor 0x008efdc0 is 86.6%, and stepDataModelJob/addTask (97.8/98.8) have unpaired pop_if_present/push.
- Base (not mine), UTF-8 helpers: RBX::utf8_decode (PLAYER 0x009b8100, std::wstring via MultiByteToWideChar) and the pathToString helper 0x0065fcd0 are missing. They block ContentProvider::findAsset (assert line 1141), requestContentString, setAssetFolder, getContent(ContentId) and readContent.
- ContentProvider ctor 0x00666280: 2013 starts a cleanup boost::thread bound to sub 0x00664260 (884 B, arguments 600000 and &flag@0x84) and stores it in the shared_ptr at +0x74; not written. setAssetFolder 0x00661f00 has no system_complete in 2013 (path built straight from char*) and ends with pathToString.
- ContentProvider asserts: PLAYER and STUDIO carry different lines (getContentString 553 vs 548, requestContentString 559 vs 554, getContent(callback) 490 vs 485, requestContentFile 512 vs 507), so one source can match only PLAYER. getContentString is laid out for PLAYER and is 98.74% in STUDIO.
- ContentProvider::getContent(callback) 0x00666490 (76.65%): 2013 tail-merges the two dispatchCallback calls (push 2 / push 1 into one shared call sequence). A switch rewrite did not reproduce it.
- LegacyContentTable FindEntry/AddEntry (97.37/97.06%) and the ctor (99.97%): the unordered_map find_node 0x008f05c0 and operator[] 0x008f0630 bodies are byte-identical but our linker ICF-folds them, so TEMPLATE markers cannot pair them. The ctor's last diff is a no-.reloc immediate: prime_list end 0x00bdb080 reads as a string.
- TextXmlParser (v8xml, not mine): the 2013 parser object is 0x38 bytes larger, its ctor (0x00402c00) and dtor (0x00404e10) are out of line, and parse() returns auto_ptr by hidden pointer. This blocks parseXml 0x00665810 (59%).
- AsyncHttpQueue::processRequests 0x0086f9d0 (43%): the 2013 signature takes shared_ptr<RBX::mutex>, not spin_mutex (iOS mangle), and the body is about 900 B longer; needs a rewrite. syncRequest (84%) still has a register-allocation difference and one extra EH state around the registerContent arguments. setThreadPool/asyncRequest/ctor are 95-98% only because callees in ThreadPool and other classes are unpaired.
- HttpQueueStatsItem ctor 0x0086f270 (79.5%) is blocked by Instance/Object.h (the vtable store order); create 0x0086f610 (90%) only by StatsItem::createChildItem being unpaired.
- Not started: WinHttp.cpp/WinInet.cpp (about 180 functions at 0x008239b0-0x00829210), Http::get (11%), CacheableContentProvider/Texture/MeshContentProvider (our CacheableContentProvider.h is empty; 118 functions at 0x0074b270-0x007505c0), InsertService (128) and MarketplaceService (170). Our InsertService.cpp and MarketplaceService.cpp are empty stubs. Each needs its Instance-derived layout first.
