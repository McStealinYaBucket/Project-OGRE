/* App/v8datamodel/ContentProvider.cpp — October 2013 (P2-499 full pipeline).
   Reconstructed from the Mach-O 0.126.0.39503 (dbq24) image; the March 2016
   carry (0.235) is used as a structural diff aid only, 2013 deltas marked.
   EA map: ctor 0x6092C6, dtor 0x6095BC, setBaseUrl 0x608710, findLocalFile
   0x6089CC, findAsset 0x608C72 (assert line 1141), findHashFile 0x609182,
   onHeartbeat 0x60A10E, getAssetFile 0x60A14E, assetFolder 0x60A22C,
   hasContent 0x60A26E, isUrlBad 0x60A342, isValidRobloxAssetUrl 0x60A38A,
   isRequestQueueEmpty 0x60A68C, registerFile 0x60A6A8, registerContent
   0x60A9E6, privateLoadContent 0x60AD32 (gate 0x60B053), loadContent 0x60B562,
   getContent-cb 0x60B760, parseContent 0x60BBBF + InvokeFileCallback 0x60BF6D
   (file-local), requestContentFile 0x60C0C4, getContentString
   0x60C356 (assert line 553), requestContentString 0x60C670, readContent
   0x60D178 (stub), getLocalCachePath 0x60D454, setAssetFolder 0x60D6E8,
   appendSlash 0x60DA71, initAssetFolder 0x60CB20, platformAssetFolder
   0x60CB52, getContent 0x60DB4A, getFile 0x60CF86, CachedContent 2-arg
   0x611C46.  Original file:
   /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ContentProvider.cpp */

#include "stdafx.h"

#include "V8DataModel/ContentProvider.h"
#include "V8Xml/SerializerV2.h"
#include "V8Xml/TextXmlParser.h"
#include "Util/FileSystem.h"
#include "Util/FastFlags.h"
#include "Util/MD5Hasher.h"
#include "Util/standardout.h"
#include "rbx/Crypt.h"
#include "rbx/Log.h"
#include "RbxFormat.h"
#include "FastLog.h"

#include <fstream>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cctype>

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/once.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/bind.hpp>

LOGVARIABLE(ContentProviderRequests, 0);

#define APPLOG(message) do { if (appLog != NULL) {RBX::mutex::scoped_lock lock(*appLogLock); appLog->writeEntry(RBX::Log::Information, message); } } while(0)

namespace RBX {
	const char* const sContentProvider = "ContentProvider";
}

using namespace RBX;

REFLECTION_BEGIN();
static Reflection::PropDescriptor<ContentProvider, std::string> prop_BaseUrl("BaseUrl", category_Data, &ContentProvider::getBaseUrl, &ContentProvider::setBaseUrl);
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ContentProvider);
}

namespace RBX {

	Log* ContentProvider::appLog = NULL;
	RBX::mutex* ContentProvider::appLogLock = NULL;

	// GLOBAL: PLAYER 0x00d38fdc
	// GLOBAL: STUDIO 0x00ec56b8
	float ContentProvider::PRIORITY_SCRIPT = 10.0f;
	float ContentProvider::PRIORITY_MESH = 20.0f;
	float ContentProvider::PRIORITY_INSERT = 30.0f;
	float ContentProvider::PRIORITY_CHARACTER = 40.0f;
	float ContentProvider::PRIORITY_ANIMATION = 50.0f;
	float ContentProvider::PRIORITY_TEXTURE = 55.0f;
	float ContentProvider::PRIORITY_DECAL = 60.0f;
	float ContentProvider::PRIORITY_SOUND = 80.0f;
	float ContentProvider::PRIORITY_GUI = 90.0f;
	float ContentProvider::PRIORITY_SKY = 100.0f;
	// 0xFA9570: BSS in the image (no initializer — zero).
	float ContentProvider::PRIORITY_MFC;
	// 0xFA9574: "Content/" by default (initAssetFolder); overridable once via
	// setAssetFolder.
	// GLOBAL: PLAYER 0x00de84ac
	// GLOBAL: STUDIO 0x00f868e4
	std::string ContentProvider::assetFolderPath;

} // namespace RBX

// 0xFB9384: guard for the default asset folder (assetFolder()/getAssetFile/
// findAsset).
// GLOBAL: PLAYER 0x00de8518
// GLOBAL: STUDIO 0x00f86950
static boost::once_flag assetFolderOnce;

namespace {

	// mac: 0x60DA71 (file-local): ensure a trailing separator.
	void appendSlashIfRequired(boost::filesystem::path& path)
	{
		std::string s = path.string();
		if (s[s.size() - 1] != '/' && s[s.size() - 1] != '\\')
			path /= "/";
	}

	// mac: 0x60D454: the on-disk content-hash cache folder, lazily created and
	// latched after the first successful lookup.
	std::string getLocalCachePath()
	{
		static std::string cachedResult;											// 0xFB9390
		static bool isCached = false;												// 0xFB938C

		if (isCached)
			return cachedResult;

		static std::string cacheDirectory = FileSystem::getCacheDirectory(true, "ContentProvider");	// 0xFB93A0

		boost::filesystem::path path(cacheDirectory);
		if (!FileSystem::filepathExists(path))
			return "";

		appendSlashIfRequired(path);

		// 0xFB9388: the image debounces the store with
		// RbxInterlockedCompareExchange(&flag, 1, 0) == 0.  That helper is
		// not present in this tree (rbx/atomic.h has only Increment/
		// Decrement) — DELTA: plain long guard; single-threaded semantics
		// equal for bring-up.
		static long caching = 0;
		if (caching == 0)
		{
			caching = 1;
			cachedResult = path.string();
			isCached = true;
		}

		return path.string();
	}

} // anonymous namespace

namespace RBX {

	// ---------------------------------------------------------------- asset
	// folder

	// FUNCTION: PLAYER 0x0065fed0
	// FUNCTION: STUDIO 0x0078a380
	void ContentProvider::initAssetFolder()
	{
		// mac: 0x60CB20 (call_once target): the shipped default.
		assetFolderPath = "Content/";
	}

	// FUNCTION: PLAYER 0x00660d10
	// FUNCTION: STUDIO 0x0078b1e0
	std::string ContentProvider::assetFolder()
	{
		boost::call_once(assetFolderOnce, &initAssetFolder);
		return assetFolderPath;
	}

	// FUNCTION: PLAYER 0x00660d50
	// FUNCTION: STUDIO 0x0078b220
	std::string ContentProvider::platformAssetFolder()
	{
		return assetFolder() + "../PlatformContent/pc/";
	}

	// FUNCTION: PLAYER 0x006616b0
	// FUNCTION: STUDIO 0x0078baa0
	std::string ContentProvider::getAssetFile(const char* filePath)
	{
		return std::string(assetFolder() + filePath);
	}

	// STUB: PLAYER 0x00661f00
	// STUB: STUDIO 0x0078c2f0
	void ContentProvider::setAssetFolder(const char* sPath)
	{
		// mac: 0x60D6F7: the image materializes assetFolder() first (call_once + a
		// discarded copy of assetFolderPath, 0x60D70F-0x60D730) so the
		// default is initialized before the override.  The discarded copy
		// has no visible effect (COW refcount bounce).
		assetFolder();

		if (RBX::Log::current())
		{
			RBX::Log::current()->writeEntry(RBX::Log::Information, RBX::format("setAssetFolder %s", sPath).c_str());
		}

		boost::filesystem::path path = boost::filesystem::system_complete(boost::filesystem::path(sPath));

		if (!boost::filesystem::exists(path))
			throw RBX::runtime_error("The path '%s' does not exist", path.string().c_str());
		if (!boost::filesystem::is_directory(path))
			throw RBX::runtime_error("'%s' is not a directory", path.string().c_str());

		appendSlashIfRequired(path);

		assetFolderPath = path.string();
	}

	// mac: 0x608C72 (static): resolve an rbxasset:// id against the platform
	// folder, then the plain asset folder.
	// STUB: PLAYER 0x00660ec0
	// STUB: STUDIO 0x0078b390
	std::string ContentProvider::findAsset(ContentId contentId)
	{
		RBXASSERT(contentId.isAsset());		// line 1141

		std::string result;
		{
			std::string path(platformAssetFolder());
			path += contentId.c_str() + 11;	// strip "rbxasset://"
			// DELTA(mac): the image wraps this in RBX::utf8_decode (0x723949),
			// which does not exist in this tree; ASCII-identical here.
			result = path;
		}

		if (!FileSystem::filepathExists(result))
		{
			std::string path(assetFolder());
			path += contentId.c_str() + 11;
			result = path;					// utf8_decode DELTA as above
			if (!FileSystem::filepathExists(result))
				return "";
		}
		return result;
	}

	// mac: 0x609182 (static): the local cache path for a content-hash id.
	// STUB: PLAYER 0x00661df0
	// STUB: STUDIO 0x0078c1e0
	std::string ContentProvider::findHashFile(ContentId contentId)
	{
		boost::filesystem::path filePath(getLocalCachePath());
		filePath /= contentId.c_str();
		if (FileSystem::filepathExists(filePath))
			return filePath.string();
		return "";
	}

	// mac: 0x6089CC (static): map a URL/path to a local file, if one exists.
	// FUNCTION: PLAYER 0x006623a0
	// FUNCTION: STUDIO 0x0078c710
	bool ContentProvider::findLocalFile(const std::string& fileName, std::string* result)
	{
		ContentId contentId(fileName);
		if (contentId.isAsset())
		{
			*result = findAsset(contentId);
			if (result->empty())
				return false;
		}
		else if (!contentId.isHttp())
		{
			*result = findHashFile(contentId);
			if (result->empty())
				return false;
		}
		return true;
	}

	// ---------------------------------------------------------------- fetch
	// pipeline

	// STUB: PLAYER 0x00665a40
	// STUB: STUDIO 0x0078fee0
	AsyncHttpQueue::RequestResult ContentProvider::privateLoadContent(ContentId& contentId, RequestType requestType, float priority, CachedContent* result, AsyncHttpQueue::RequestCallback* callback, AsyncHttpQueue::ResultJob jobType)
	{
		// mac: 0x60AD44: null-this guard kept in production (bare RBXCRASH — same
		// idiom as Reflection::Descriptor::checkLockedDown).
		if (!this)
			RBXCRASH();

		if (contentId.isNull())
			return AsyncHttpQueue::Failed;

		FASTLOGS(FLog::ContentProviderRequests, "Content requested: %s", contentId.c_str());

		contentId.convertAssetId(baseUrl);
		contentId.convertToLegacyContent(baseUrl);

		if (contentCache->findCacheItem(contentId.toString(), result))
			return AsyncHttpQueue::Succeeded;

		if (requestType != FullAsyncRequest)
		{
			if (contentId.isAsset())
			{
				boost::shared_ptr<const std::string> assetFile(new std::string(findAsset(contentId)));
				if (assetFile->empty())
					return AsyncHttpQueue::Failed;

				contentCache->insertCacheItem(contentId.toString(), CachedContent(assetFile));

				return contentCache->findCacheItem(contentId.toString(), result) ? AsyncHttpQueue::Succeeded : AsyncHttpQueue::Waiting;
			}
			else if (!contentId.isHttp())
			{
				// This will only work if the contentId is actually a hash.
				boost::shared_ptr<const std::string> hashFile(new std::string(findHashFile(contentId)));
				if (!hashFile->empty())
				{
					contentCache->insertCacheItem(contentId.toString(), CachedContent(hashFile));

					return contentCache->findCacheItem(contentId.toString(), result) ? AsyncHttpQueue::Succeeded : AsyncHttpQueue::Waiting;
				}
			}
		}

		// mac: 0x60B053: the 2013 url-validation gate (2016 dropped it).
		if (FFlag::ValidateAssetUrlEnabled && !isValidRobloxAssetUrl(contentId))
			return AsyncHttpQueue::Failed;

		// If we get all the way here then try to request it for later use.
		switch (requestType)
		{
		case NoHttpRequest:
			break;

		case AsyncHttpRequest:
			if (!contentId.isHttp())
			{
				break;
			}
			// Fall through:
		case FullAsyncRequest:
			{
				FASTLOGS(FLog::ContentProviderRequests, "Content requested: %s", contentId.c_str());
				contentCache->asyncRequest(contentId.c_str(), priority, callback, jobType, false);
				return AsyncHttpQueue::Waiting;
			}

		case SyncHttpRequest:
			{
				if (contentCache->syncRequest(contentId.c_str()))
					return contentCache->findCacheItem(contentId.c_str(), result) ? AsyncHttpQueue::Succeeded : AsyncHttpQueue::Failed;
				return AsyncHttpQueue::Waiting;
			}
		}

		return AsyncHttpQueue::Failed;
	}

	// mac: 0x60C670: fast path for scripts — serve locally available content or
	// kick an async request.
	// STUB: PLAYER 0x00666940
	// STUB: STUDIO 0x00790e80
	boost::shared_ptr<const std::string> ContentProvider::requestContentString(const ContentId& contentId, float priority)
	{
		RBXASSERT(this!=NULL);		// line 603

		ContentId activeId(contentId);
		CachedContent item;

		if (privateLoadContent(activeId, AsyncHttpRequest, priority, &item, NULL, AsyncHttpQueue::AsyncInline) != AsyncHttpQueue::Succeeded)
			return boost::shared_ptr<const std::string>();

		if (!item.data)
		{
			// DELTA(mac): the image reads through utf8_decode(*item.filename)
			// (0x723949), which does not exist in this tree; ASCII-identical.
			std::ifstream file(item.filename->c_str(), std::ios_base::in | std::ios_base::binary);
			std::ostringstream data;
			boost::iostreams::copy(file, data);		// 0x60C7F1 (buffer 4096)
			item.data.reset(new std::string(data.str()));
			// The image inserts unconditionally (no 2016-only
			// FFlag::NoCacheForLocalContent gate).
			contentCache->insertCacheItem(activeId.toString(), item);
		}

		return item.data;
	}

	// ------------------------------------------------------------- P2-499b

	// mac: 0x60BBBF (file-local): loadContent's parse thunk.  A fresh Instances
	// vector is always delivered; only Succeeded parses (2013 has no catch --
	// malformed XML propagates; the 2016 exception param arrives later).
	// STUB: PLAYER 0x00665810
	// STUB: STUDIO 0x0078fcb0
	static void parseXml(const ContentId& id, AsyncHttpQueue::RequestResult result, std::istream* stream, boost::function<void(AsyncHttpQueue::RequestResult, boost::shared_ptr<Instances>)> callback)
	{
		boost::shared_ptr<Instances> instances(new Instances());

		if (result == AsyncHttpQueue::Succeeded)
		{
			try
			{
				FASTLOGS(FLog::ContentProviderRequests, "Parsing xml for content: %s", id.c_str());

				TextXmlParser parser(stream->rdbuf());
				std::auto_ptr<XmlElement> root(parser.parse());

				SerializerV2 serializer;
				serializer.loadInstancesFromText(root.get(), *instances);
			}
			catch (std::runtime_error e)
			{
				StandardOut::singleton()->printf(MESSAGE_ERROR, "Content failed to parse for %s because %s", id.c_str(), e.what());
				result = AsyncHttpQueue::Failed;
			}
		}

		callback(result, instances);
	}

	// mac: 0x60BF6D (file-local): re-open a filename-only cache item and answer
	// Succeeded inline.  utf8_decode DELTA as in requestContentString.
	static void InvokeFileCallback(AsyncHttpQueue::RequestCallback callback, boost::shared_ptr<const std::string> filename)
	{
		std::ifstream fileStream(filename->c_str(), std::ios_base::in | std::ios_base::binary);
		callback(AsyncHttpQueue::Succeeded, &fileStream, boost::shared_ptr<const std::string>());
	}

	// mac: 0x60B562: async front-end.  The bind drops RequestCallback's trailing
	// response argument (parseContent wants id/result/stream/callback only).
	// FUNCTION: PLAYER 0x00667140
	// FUNCTION: STUDIO 0x00791680
	void ContentProvider::loadContent(const ContentId& id, float priority, boost::function<void(AsyncHttpQueue::RequestResult, boost::shared_ptr<Instances>)> callback, AsyncHttpQueue::ResultJob jobType)
	{
		getContent(id, priority, boost::bind(&parseXml, id, _1, _2, callback), jobType);
	}

	// mac: 0x60B760: callback getContent.  Data items dispatch with the payload;
	// filename-only items refetch through the generic dispatch; Failed
	// answers with the empty string; Waiting stays silent.
	// STUB: PLAYER 0x00666490
	// STUB: STUDIO 0x007909d0
	void ContentProvider::getContent(const ContentId& id, float priority, AsyncHttpQueue::RequestCallback callback, AsyncHttpQueue::ResultJob jobType)
	{
		ContentId activeId(id);
		RBXASSERT(this!=NULL);		// line 490

		CachedContent item;
		AsyncHttpQueue::RequestResult requestResult = privateLoadContent(activeId, AsyncHttpRequest, priority, &item, &callback, jobType);

		if (requestResult == AsyncHttpQueue::Succeeded)
		{
			if (item.data)
			{
				AsyncHttpQueue::dispatchCallback(callback, this, AsyncHttpQueue::Succeeded, item.data, jobType);
			}
			else
			{
				AsyncHttpQueue::dispatchGenericCallback(boost::bind(&InvokeFileCallback, callback, item.filename), this, jobType);
			}
		}
		else if (requestResult == AsyncHttpQueue::Failed)
		{
			AsyncHttpQueue::dispatchCallback(callback, this, AsyncHttpQueue::Failed, boost::shared_ptr<const std::string>(new std::string("")), jobType);
		}
	}

	// mac: 0x60C0C4: full-async fetch that also reports the local file.  A miss
	// downgrades to Failed only for banned urls; otherwise Waiting.
	// STUB: PLAYER 0x006667c0
	// STUB: STUDIO 0x00790d00
	void ContentProvider::requestContentFile(const ContentId& id, float priority, AsyncHttpQueue::RequestResult& result, std::string& fileName)
	{
		RBXASSERT(this!=NULL);		// line 512

		ContentId activeId(id);
		CachedContent item;

		result = privateLoadContent(activeId, FullAsyncRequest, priority, &item, NULL, AsyncHttpQueue::AsyncInline);

		if (result == AsyncHttpQueue::Succeeded)
		{
			if (registerFile(activeId, &item))
			{
				fileName = *item.filename;
				result = AsyncHttpQueue::Succeeded;
			}
			else
			{
				result = AsyncHttpQueue::Failed;
			}
		}
		else if (contentCache->isUrlBad(activeId.c_str()))
		{
			result = AsyncHttpQueue::Failed;
		}
		else
		{
			result = AsyncHttpQueue::Waiting;
		}
	}








































	// mac: 0x60C356: blocking string load: request, then sync-load once on miss.
	// Shared_ptr return proven by the loadfile sret ABI (batch640; 2016 agrees).
	// FUNCTION: PLAYER 0x006672c0
	// STUB: STUDIO 0x00791800
	boost::shared_ptr<const std::string> ContentProvider::getContentString(ContentId id)
	{
		APPLOG("ContentProvider::getContentString");

		boost::shared_ptr<const std::string> result = requestContentString(id, PRIORITY_SCRIPT);
		if (!result)
		{
			CachedContent item;
			if (!blockingLoadContent(id, &item))
				throw RBX::runtime_error("Unable to load %s", id.c_str());

			result = requestContentString(id, PRIORITY_SCRIPT);
			if (!result)
				throw RBX::runtime_error("Unable to retrieve cache of %s", id.c_str());
		}

		RBXASSERT(result);		// line 553
		return result;
	}

	// mac: 0x60DB4A: id -> stream, from memory or a cached file.
	std::auto_ptr<std::istream> ContentProvider::getContent(ContentId id)
	{
		CachedContent item;
		ContentId contentId(id);

		if (privateLoadContent(contentId, SyncHttpRequest, -1.0f, &item, NULL, AsyncHttpQueue::AsyncInline) != AsyncHttpQueue::Succeeded)
			throw RBX::runtime_error("Unable to load %s", id.c_str());

		if (item.data)
			return std::auto_ptr<std::istream>(new std::istringstream(*item.data));
		// DELTA(mac): utf8_decode(*item.filename) (0x723949) not in this tree;
		// ASCII-identical on Windows paths.
		return std::auto_ptr<std::istream>(new std::ifstream(item.filename->c_str(), std::ios_base::in | std::ios_base::binary));
	}

	// mac: 0x60A342: url validity via the queue's failed-url list.
	// FUNCTION: PLAYER 0x00660470
	// FUNCTION: STUDIO 0x0078a920
	bool ContentProvider::isUrlBad(ContentId contentId)
	{
		contentId.convertAssetId(baseUrl);
		contentId.convertToLegacyContent(baseUrl);
		return contentCache->isUrlBad(contentId.toString());
	}

	// mac: 0x60A38A: blocks asset-url spoofing (host must match before the .com).
	// FUNCTION: PLAYER 0x00661750
	// FUNCTION: STUDIO 0x0078bb40
	bool ContentProvider::isValidRobloxAssetUrl(ContentId contentId)
	{
		// 2013 shares one 'return true' epilogue for the http, /asset/ and allowed-host exits.
		if (contentId.isHttp())
		{
			std::string lower(contentId.toString());
			std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

			if (lower.find("/asset/") != std::string::npos)
			{
				size_t posCom = lower.find(".com");

				if (lower.find(".roblox.com/asset/") < posCom
					|| lower.find(".roblox.com//asset/") < posCom
					|| lower.find("http://roblox.com/asset/") < posCom
					|| lower.find("http://roblox.com//asset/") < posCom
					|| lower.find(".robloxlabs.com/asset/") < posCom
					|| lower.find(".robloxlabs.com//asset/") < posCom)
				{
					if (lower.find("asset/..") == std::string::npos)
						return true;
				}

				StandardOut::singleton()->printf(RBX::MESSAGE_WARNING, "blocking %s", lower.c_str());
				return false;
			}
		}
		return true;
	}

	// mac: 0x60A6A8: register in-memory data as a loadable hash file.
	// FUNCTION: PLAYER 0x00664090
	// FUNCTION: STUDIO 0x0078e6a0
	bool ContentProvider::registerFile(const ContentId& contentId, CachedContent* item)
	{
		if (!item->filename)
		{
			std::istringstream stream(*item->data);
			ContentId hash = registerContent(stream, Name::getNullName());
			std::string filePath = findHashFile(hash);
			if (filePath.empty())
				return false;

			item->filename.reset(new std::string(filePath));
			contentCache->insertCacheItem(contentId.c_str(), *item);
		}
		return true;
	}

	// FUNCTION: PLAYER 0x00666ca0
	// FUNCTION: STUDIO 0x007911e0
	bool ContentProvider::blockingLoadContent(ContentId id, CachedContent* result)
	{
		APPLOG(RBX::format("ContentProvider::requestContentString %d", id.c_str()).c_str());
		return privateLoadContent(id, SyncHttpRequest, -1, result, NULL, AsyncHttpQueue::AsyncInline) == AsyncHttpQueue::Succeeded;
	}

	// FUNCTION: PLAYER 0x00666da0
	// FUNCTION: STUDIO 0x007912e0
	bool ContentProvider::isContentLoaded(ContentId id)
	{
		return privateLoadContent(id, NoHttpRequest, -1, NULL, NULL, AsyncHttpQueue::AsyncInline) == AsyncHttpQueue::Succeeded;
	}

	// mac: 0x60CF86: id -> a local file path (hash-file registration on demand).
	// FUNCTION: PLAYER 0x00666e10
	// FUNCTION: STUDIO 0x00791350
	std::string ContentProvider::getFile(ContentId id)
	{
		CachedContent item;
		if (!blockingLoadContent(id, &item) || !registerFile(id, &item))
			throw RBX::runtime_error("Unable to load %s", id.c_str());

		return *item.filename;
	}

	// FUNCTION: PLAYER 0x00667110
	// FUNCTION: STUDIO 0x00791650
	bool ContentProvider::hasContent(const ContentId& id)
	{
		return isContentLoaded(id);
	}

	// mac: 0x60A9E6: hash a stream, store it under the hash in the local cache.
	ContentId ContentProvider::registerContent(std::istream& stream, const Name& name)
	{
		ContentId result;
		{
			// Get the hash
			boost::scoped_ptr<MD5Hasher> hasher(MD5Hasher::create());
			hasher->addData(stream);
			result = ContentId(hasher->toString(), name);
		}

		// Get/Create the local cache folder
		boost::filesystem::path filePath(getLocalCachePath());
		filePath /= result.c_str();

		if (!FileSystem::filepathExists(filePath))
		{
			// Save it to the local cache
			std::ofstream outStream(filePath.string().c_str(), std::ios_base::out | std::ios_base::binary);

			stream.clear();
			stream.seekg(0, std::ios_base::beg);
			do
			{
				char buffer[1024];
				stream.read(buffer, 1024);
				outStream.write(buffer, stream.gcount());
			}
			while (stream.gcount() > 0);
		}

		return result;
	}

	// ---------------------------------------------------------------- service
	// plumbing

	void ContentProvider::setBaseUrl(std::string url)
	{
		if (url != baseUrl)
		{
			baseUrl = url;
			raisePropertyChanged(prop_BaseUrl);
		}
	}

	const std::string& ContentProvider::getBaseUrl() const
	{
		return baseUrl;
	}

	void ContentProvider::setThreadPool(int count)
	{
		contentCache->setThreadPool(count);
	}

	void ContentProvider::setCacheSize(int size)
	{
		contentCache->setCacheSize(size);
	}

	bool ContentProvider::isRequestQueueEmpty()
	{
		return contentCache->isRequestQueueEmpty();
	}

	void ContentProvider::onHeartbeat(const Heartbeat& event)
	{
		contentCache->onHeartbeat(event);
	}

	void ContentProvider::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		// 0x60A???: no ServiceProvider::onServiceProvider super call in the
		// image (DELTA vs the usual pattern); stats item + heartbeat forward
		// only.
		contentCache->resetStatsItem(newProvider);
		onServiceProviderHeartbeatInstance(oldProvider, newProvider);
	}

	// STUB: PLAYER 0x00666280
	// STUB: STUDIO 0x007907c0
	ContentProvider::ContentProvider()
	{
		// mac: 0x6092C6: thread count / cache size are literals in the image (the
		// 2016 DYNAMIC_FASTINTVARIABLEs do not exist here).
		setName("ContentProvider");
		contentCache.reset(new AsyncHttpCache<CachedContent>(this, &ContentProvider::findLocalFile, 16, 1000));
	}

	ContentProvider::~ContentProvider()
	{
		// mac: 0x6095BC: on-disk cleanup first, then the implicit member/base
		// destructors (threads, contentCache, baseUrl).
		FileSystem::clearCacheDirectory("ContentProvider", 0);
	}

} // namespace RBX

namespace RBX {

// STUB: PLAYER 0x00660220
// STUB: STUDIO 0x0078a6d0
const char* ContentProvider::verifyScriptSignature(const char* script, bool required)
{
	// mac: 0x609AC4 (1096B, batches 679/681/684): old %-signed form first,
	// then --rbxsig%, then plain (required -> throw). Verification goes
	// through Crypt (WinCrypt carry; Mac binary used OpenSSL).
	if (script[0] == '%')
	{
		const char* sigStart = script + 1;
		const char* sigEnd = strchr(sigStart, '%');
		if (sigEnd == NULL)
			throw std::runtime_error("");
		std::string signature(sigStart, sigEnd - sigStart);
		const char* body = sigEnd + 1;
		Crypt().verifySignatureBase64(body, signature);
		return body;
	}
	if (strncmp(script, "--rbxsig%", 9) == 0)
	{
		const char* sigStart = script + 9;
		const char* sigEnd = strchr(sigStart, '%');
		if (sigEnd == NULL)
			throw std::runtime_error("");
		std::string signature(sigStart, sigEnd - sigStart);
		Crypt().verifySignatureBase64(sigEnd + 1, signature);
		return script;
	}
	if (required)
		throw RBX::runtime_error("No sig");
	return script;
}

// STUB: PLAYER 0x00660b00
// STUB: STUDIO 0x0078afd0
const char* ContentProvider::verifyRequestedScriptSignature(const char* script,
	const std::string& required, bool requiredFlag)
{
	// mac: 0x60971E (626B, batch698): verifyScriptSignature first; then the
	// "--rbxassetid%<required>%" marker (the verified body is returned
	// as-is) or the leading %token% restriction. Every rejection throws
	// std::runtime_error("") -- contentReady maps it to "Library failed
	// signature check".
	const char* body = verifyScriptSignature(script, requiredFlag);
	const char* marker = strstr(body, "--rbxassetid%");
	if (marker != NULL)
	{
		if (strncmp(marker + 13, required.c_str(), required.size()) == 0
			&& marker[13 + required.size()] == '%')
			return body;
		throw std::runtime_error("");
	}
	if (body[0] == '%')
	{
		size_t end = 1;
		while (end <= 20 && body[end] != 0 && body[end] != '%')
			++end;
		if (end <= 20 && body[end] == '%')
		{
			std::string token(body + 1, end - 1);
			if (required.compare(token) == 0)
				return body + end + 1;
			throw std::runtime_error("");
		}
	}
	if (requiredFlag)
		throw std::runtime_error("");
	return body;
}

// FUNCTION: PLAYER 0x006649e0
ContentProvider::CachedContent::CachedContent(
	boost::shared_ptr<const std::string> data,
	boost::shared_ptr<const std::string> filename)
	: data(data)
	, filename(filename)
{
}

bool ContentProvider::isHttpUrl(const std::string& s)
{
	return s.find("http://") == 0 || s.find("https://") == 0;
}

bool ContentProvider::isUrl(const std::string& s)
{
	return isHttpUrl(s) || s.find("rbxasset://") == 0 || s.find("rbxassetid://") == 0;
}

// STUB: PLAYER 0x006676a0
// STUB: STUDIO 0x00791be0
void ContentProvider::readContent(const char* name, std::istream& stream, unsigned int size)
{
	// mac: 0x60D178 minimal (original TU: App/util/ContentProvider.cpp:1093 —
	// consolidated here to avoid a duplicate ContentProvider.obj in the
	// bring-up build): stream-position effect only; cache TODO (see header).
	(void)name;
	stream.ignore(size);
}

} // namespace RBX
