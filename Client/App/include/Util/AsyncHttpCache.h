/* Util/AsyncHttpCache.h -- October 2013 (content cache over AsyncHttpQueue).
   Carried from the March 2016 App/include/util/AsyncHttpCache.h, minus
   getRequestedUrls() -- the 2013 PE has no symbol for it (nor for
   AsyncHttpQueue::getFailedUrls/getRequestQueueUrls), so the
   Reflection::ValueArray bridge surface is 2016-only. Everything else
   (setCacheSize/findCacheItem/removeCacheItem/invalidateCacheItemOrFailure/
   insertCacheItem/renameCacheItem/clearCache/printContentNames + the
   registerContent override that hooks the cached-content insert through
   the AsyncHttpQueue vtable, slot0=D1Ev 0x1E21FA) is kept verbatim.
   Consumers: App/util/Http.cpp-style content providers (ContentProvider::
   CachedContent is the first instantiation).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/util/AsyncHttpCache.h */

#pragma once
#include "FastLog.h"
#include "Util/AsyncHttpQueue.h"
#include "rbx/make_shared.h"

namespace RBX
{
template<typename CachedContent, bool Log=false>
class AsyncHttpCache
	:public AsyncHttpQueue
{
public:
	AsyncHttpCache(Instance* owner, boost::function<bool(const std::string&, std::string*)> getLocalFile, int threadCount, int cacheSize)
		:AsyncHttpQueue(owner, getLocalFile, threadCount)
		,contentCache(cacheSize)
	{}

	void setCacheSize(int count)
	{
		boost::mutex::scoped_lock lock(contentCacheMutex);
		contentCache.resize(count);
	}

	bool findCacheItem(const std::string& id, CachedContent* result)
	{
		boost::mutex::scoped_lock lock(contentCacheMutex);
		return contentCache.fetch(id, result);
	}
    void removeCacheItem(const std::string& id)
	{
		boost::mutex::scoped_lock lock(contentCacheMutex);
		contentCache.remove(id);
	}
	void invalidateCacheItemOrFailure(const std::string& id)
	{
		{
			boost::mutex::scoped_lock lock(contentCacheMutex);
			contentCache.remove(id);
		}
		{
			boost::recursive_mutex::scoped_lock lock(requestSync);
			std::list<FailedUrl>::iterator found = failedUrls.end();
			for (std::list<FailedUrl>::iterator itr = failedUrls.begin();
				itr != failedUrls.end() && found == failedUrls.end(); ++itr)
			{
				if (itr->url == id)
				{
					found = itr;
				}
			}
			if (found != failedUrls.end())
				failedUrls.erase(found);
		}
	}
	void insertCacheItem(const std::string& id, const CachedContent& result)
	{
		boost::mutex::scoped_lock lock(contentCacheMutex);
		contentCache.insert(id, result);
	}
	void renameCacheItem(const std::string& id, const std::string& newId)
	{
		boost::mutex::scoped_lock lock(contentCacheMutex);
		CachedContent content;
		if (contentCache.fetch(id, &content))
		{
			//"rename" the entry.
			contentCache.remove(id);
			contentCache.insert(newId, content);
		}
	}
    
    void clearCache()
    {
		{
			boost::mutex::scoped_lock lock(contentCacheMutex);
			contentCache.clear();
		}
		{
			boost::recursive_mutex::scoped_lock lock(requestSync);
			failedUrls.clear();
		}
    }

	void printContentNames()
	{
		boost::mutex::scoped_lock lock(contentCacheMutex);
		contentCache.printContentNames();
	}

protected:
	/*override*/ void registerContent(const std::string& url, shared_ptr<const std::string> response, shared_ptr<const std::string> filename)
	{
		if(Log)	FASTLOGS(FLog::HttpQueue, "URL(%s)", url.c_str());
		boost::mutex::scoped_lock lock(contentCacheMutex);
		contentCache.insert(url, CachedContent(response, filename));
	}
	typedef SizeEnforcedLRUCache<std::string, CachedContent> ContentCache;

	boost::mutex contentCacheMutex;			//synchronizes the contentCache
	ContentCache contentCache;
};
}
