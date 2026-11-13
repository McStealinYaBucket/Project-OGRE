/* V8DataModel/ContentProvider.h — October 2013 (P2-499 full surface).
   Reconstructed from the Mach-O 0.126.0.39503 (dbq24) image; the March 2016
   carry (0.235) was used as a diff aid only.  Bases confirmed by the ctor
   sub-vtables (0xEF16B8/0xEF1744/0xEF1750/0xEF1764/0xEF177C): Heartbeat-
   Instance occupies +0x60 (vptr) / +0x64 (scoped_connection), exactly as in
   the 2016 header (`, public HeartbeatInstance`).
   Fetch pipeline: privateLoadContent 0x60AD32 resolves asset/hash/http
   locally, then applies the FFlag::ValidateAssetUrlEnabled gate (0x60B053)
   and dispatches on RequestType into AsyncHttpCache (asyncRequest/
   syncRequest over AsyncHttpQueue).  RequestResult/ResultJob/RequestCallback
   are AsyncHttpQueue:: nested (proven by the privateLoadContent mangle).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ContentProvider.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/ContentId.h"
#include "Util/AsyncHttpCache.h"
#include "Util/HeartbeatInstance.h"

#include <istream>
#include <memory>
#include <string>
#include <boost/shared_ptr.hpp>

namespace boost { class thread; }

namespace RBX {

	class Log;
	class mutex;

	extern const char* const sContentProvider;

class ContentProvider
		: public DescribedNonCreatable<ContentProvider, Instance, sContentProvider, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
		, public HeartbeatInstance
{
public:
	// Request priorities (mac float constants; PRIORITY_MFC is BSS-zero —
	// no initializer in the image).
	static float PRIORITY_SCRIPT;		// 10.0f
	static float PRIORITY_MESH;			// 20.0f
	static float PRIORITY_INSERT;		// 30.0f
	static float PRIORITY_CHARACTER;	// 40.0f
	static float PRIORITY_ANIMATION;	// 50.0f
	static float PRIORITY_TEXTURE;		// 55.0f
	static float PRIORITY_DECAL;		// 60.0f
	static float PRIORITY_SOUND;		// 80.0f
	static float PRIORITY_GUI;			// 90.0f
	static float PRIORITY_SKY;			// 100.0f
	static float PRIORITY_MFC;			// 0xFA9570 (zero)

	// privateLoadContent dispatch values (0x60AD32 return table).
	enum RequestType
	{
		NoHttpRequest = 0,		// cache/asset/hash only; miss -> Failed
		AsyncHttpRequest = 1,	// http ids only; enqueue async
		SyncHttpRequest = 2,	// syncRequest + cache probe
		FullAsyncRequest = 3	// enqueue async for any scheme
	};

	// mac: 0x611C46 (out-of-line 2-arg; 0- and 1-arg inline).  Names from the
	// 2016 carry; roles proven by registerFile/requestContentString data
	// flow.  Layout: data@+0, filename@+8 (the binary's +8 test in
	// registerFile is the filename-null gate).
	class CachedContent
	{
	public:
		boost::shared_ptr<const std::string> data;
		boost::shared_ptr<const std::string> filename;
		CachedContent() {}
		CachedContent(boost::shared_ptr<const std::string> filename) : filename(filename) {}
		CachedContent(boost::shared_ptr<const std::string> data, boost::shared_ptr<const std::string> filename);
	};

	ContentProvider();
	virtual ~ContentProvider();

	/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);
	/*implement*/ void onHeartbeat(const Heartbeat& event);

	// ---- fetch surface ----
	std::auto_ptr<std::istream> getContent(ContentId id);		// 0x60DB4A (member; sret@0/this@4)
	std::string getFile(ContentId id);
	// mac: 0x60C356: shared_ptr return — the loadfile call site allocates an
	// 8-byte sret {px,pn} and runs sp_counted_base::release on pn
	// (batch640); 2016 header agrees (shared_ptr<const std::string>).
	boost::shared_ptr<const std::string> getContentString(ContentId id);
	boost::shared_ptr<const std::string> requestContentString(const ContentId& contentId, float priority);	// 0x60C670
	bool hasContent(const ContentId& contentId);				// 0x60A26E
	// 2016 names; 2013 bodies at PLAYER 0x00666ca0 / 0x00666da0 (no iOS/Mac symbol: inlined
	// there). blockingLoadContent logs "ContentProvider::requestContentString %d" as 2016 does.
	bool blockingLoadContent(ContentId id, CachedContent* result);
	bool isContentLoaded(ContentId id);
	bool isUrlBad(ContentId contentId);							// 0x60A342
	bool isRequestQueueEmpty();									// 0x60A68C
	// mac: 0x60A38A (member; the binary's `this` slot is never touched — the
	// by-value ContentId arrives in the post-this slot; behavior identical
	// either way).
	bool isValidRobloxAssetUrl(ContentId contentId);
	AsyncHttpQueue::RequestResult privateLoadContent(ContentId& contentId, RequestType requestType, float priority, CachedContent* result, AsyncHttpQueue::RequestCallback* callback, AsyncHttpQueue::ResultJob jobType);	// 0x60AD32

	// ---- P2-499b: async front-end + callback surface ----
	void loadContent(const ContentId& id, float priority, boost::function<void(AsyncHttpQueue::RequestResult, boost::shared_ptr<Instances>)> callback, AsyncHttpQueue::ResultJob jobType = AsyncHttpQueue::AsyncInline);	// 0x60B562
	void getContent(const ContentId& id, float priority, AsyncHttpQueue::RequestCallback callback, AsyncHttpQueue::ResultJob jobType);	// 0x60B760 (callback overload)
	void requestContentFile(const ContentId& id, float priority, AsyncHttpQueue::RequestResult& result, std::string& fileName);	// 0x60C0C4

	static bool findLocalFile(const std::string& fileName, std::string* result);	// 0x6089CC
	static std::string findAsset(ContentId contentId);								// 0x608C72
	static std::string findHashFile(ContentId contentId);							// 0x609182
	// Member (uses this->contentCache; the sole 2013 caller is getFile).
	bool registerFile(const ContentId& contentId, CachedContent* item);				// 0x60A6A8
	static ContentId registerContent(std::istream& stream, const Name& name);		// 0x60A9E6

	// ---- asset folder ----
	static void setAssetFolder(const char* sPath);			// 0x60D6E8
	static std::string assetFolder();						// 0x60A22C
	static std::string platformAssetFolder();				// 0x60CB52
	static std::string getAssetFile(const char* filePath);	// 0x60A14E

	// ---- base url / pool / cache ----
	void setBaseUrl(std::string url);						// 0x608710
	const std::string& getBaseUrl() const;					// 0x608768
	void setThreadPool(int count);
	void setCacheSize(int size);

	// ---- kept 2013 surface ----
	void readContent(const char* name, std::istream& stream, unsigned int size);	// 0x60D178
	// mac: 0x609AC4 (static: the loadfile call site pushes both args at [esp]/
	// [esp+4] as cdecl with no ecx this-setup, batch640).
	static const char* verifyScriptSignature(const char* script, bool required);
	// mac: 0x60971E (626B, batch698): request-time signature check for library
	// fetches. Rejections throw std::runtime_error("").
	static const char* verifyRequestedScriptSignature(const char* script,
		const std::string& required, bool requiredFlag);
	static bool isHttpUrl(const std::string& s);	// 0x60CAB6
	static bool isUrl(const std::string& s);		// 0x60CC30

	// GLOBAL: PLAYER 0x00de8340
	// GLOBAL: STUDIO 0x00f86778
	static Log* appLog;
	// GLOBAL: PLAYER 0x00de8344
	// GLOBAL: STUDIO 0x00f8677c
	static RBX::mutex* appLogLock;

private:
	static void initAssetFolder();					// 0x60CB20 (call_once target)
	static std::string assetFolderPath;				// 0xFA9574 (class static; mangled)

	boost::shared_ptr<boost::thread> legacyContentCleanupThread;	// +0x68 (ctor-zeroed; unused 2013)
	boost::shared_ptr<boost::thread> contentCleanupThread;			// +0x70 (ctor-zeroed; unused 2013)
	boost::shared_ptr<AsyncHttpCache<CachedContent> > contentCache;	// +0x78
	bool clearFinishFlag;											// +0x80 (not written by the 2013 ctor; unused)
	std::string baseUrl;											// +0x84
};

} // namespace RBX

// Template and implicit member bodies the .cpp emits (identical to the 2013 bytes in a private
// compile; markers pair them so their callers match).

// TEMPLATE: PLAYER 0x0065fc10
// TEMPLATE: STUDIO 0x0078a080
// ?system_complete@filesystem@boost@@YA?AVpath@12@ABV312@@Z

// TEMPLATE: PLAYER 0x0065ffa0
// TEMPLATE: STUDIO 0x0078a450
// ??0?$storage1@V?$value@VContentId@RBX@@@_bi@boost@@@_bi@boost@@QAE@V?$value@VContentId@RBX@@@12@@Z

// TEMPLATE: PLAYER 0x00660600
// ??$?0PBD@path@filesystem@boost@@QAE@ABQBDPAX@Z

// TEMPLATE: PLAYER 0x00660710
// TEMPLATE: STUDIO 0x0078ac10
// ??0?$storage2@V?$value@VContentId@RBX@@@_bi@boost@@U?$arg@$00@3@@_bi@boost@@QAE@V?$value@VContentId@RBX@@@12@U?$arg@$00@2@@Z

// TEMPLATE: PLAYER 0x00660780
// TEMPLATE: STUDIO 0x0078ac80
// ??$append@$$BY01$$CBD@path@filesystem@boost@@QAEAAV012@AAY01$$CBDABV?$codecvt@_WDH@std@@@Z

// TEMPLATE: PLAYER 0x00660a60
// TEMPLATE: STUDIO 0x0078af30
// ??0CachedContent@ContentProvider@RBX@@QAE@V?$shared_ptr@$$CBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@boost@@@Z

// TEMPLATE: PLAYER 0x006611e0
// TEMPLATE: STUDIO 0x0078b610
// ??$?_0PBD@path@filesystem@boost@@QAEAAV012@ABQBD@Z

// TEMPLATE: PLAYER 0x00661290
// TEMPLATE: STUDIO 0x0078b6c0
// ??0?$storage3@V?$value@VContentId@RBX@@@_bi@boost@@U?$arg@$00@3@U?$arg@$01@3@@_bi@boost@@QAE@V?$value@VContentId@RBX@@@12@U?$arg@$00@2@U?$arg@$01@2@@Z

// TEMPLATE: PLAYER 0x006633d0
// TEMPLATE: STUDIO 0x0078d990
// ?clear@?$LRUCache@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@VCachedContent@ContentProvider@RBX@@@RBX@@QAEXXZ

// TEMPLATE: PLAYER 0x00663820
// TEMPLATE: STUDIO 0x0078de30
// ?setCacheSize@?$AsyncHttpCache@VCachedContent@ContentProvider@RBX@@$0A@@RBX@@QAEXH@Z

// TEMPLATE: PLAYER 0x00663980
// TEMPLATE: STUDIO 0x0078df90
// ??0?$LRUCache@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@VCachedContent@ContentProvider@RBX@@@RBX@@QAE@XZ

// TEMPLATE: PLAYER 0x00663c80
// TEMPLATE: STUDIO 0x0078e290
// ??1?$LRUCache@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@VCachedContent@ContentProvider@RBX@@@RBX@@QAE@XZ

// TEMPLATE: PLAYER 0x00664c70
// TEMPLATE: STUDIO 0x0078f190
// ??1?$AsyncHttpCache@VCachedContent@ContentProvider@RBX@@$0A@@RBX@@UAE@XZ
