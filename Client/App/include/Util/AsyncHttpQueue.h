/* Util/AsyncHttpQueue.h -- October 2013 (async HTTP fetch queue).
   Carried from the March 2016 App/include/util/AsyncHttpQueue.h; the member
   layout is batch695/batch696-verified against the binary:
     vptr@0, enable_shared_from_this weak@4 (px@4/pn@8), statsItem@0xC,
     avgTimeInQueue RunningAverage@0x14, avgRequestCompleteTime RA@0x40,
     numSlowRequests@0x6C, requestSync@0x70, requestQueue@0x9C,
     failedUrls@0xA4, threadPool@0xAC, asyncRetrySync@0xB0,
     asyncRetryTasks@0xDC, currentWallTime@0x104, owner@0x10C,
     getLocalFile@0x110..0x120 -- base size 0x120 (AQ ctor 0x1E1D42,
     D2 0x1E2204, getRequestQueueSize 0x1E2176, isUrlBad 0x1E4126,
     asyncRequest 0x1E422A, syncRequest 0x1E4788).
   DELTAS vs 2016 (each binary-confirmed):
     * RequestCallback has 3 parameters (no shared_ptr<std::exception>);
       dispatchCallback likewise (5 params, no exception).
     * asyncRequest has no expectedType parameter; syncRequest neither;
       Request has no expectedType member.
     * No setCachePolicy/cachePolicy (no HttpCache::Policy in 2013).
     * No getFailedUrls()/getRequestQueueUrls() (absent from the PE).
     * Heartbeat comes from Util/RunTransition.h in this tree (2016 pulled
       it in via Util/HeartbeatInstance.h, which is not ported yet).
   LOGGROUP(HttpQueue): the channel exists in the 2013 binary (FASTLOGS
   call sites at isUrlBad/syncRequest/processRequests); its definition site
   is unknown -- LOGVARIABLE(HttpQueue, 0) lives in AsyncHttpQueue.cpp.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/util/AsyncHttpQueue.h */

#pragma once

#include <stdio.h>
#include <string>
#include <istream>
#include <memory>
#include <vector>
#include <list>
#include <queue>

#include "Util/Name.h"
#include "rbx/boost.hpp"
#include "rbx/rbxTime.h"
#include "rbx/RunningAverage.h"
#include "FastLog.h"
#include "Util/ContentId.h"
#include "Util/RunTransition.h"
#include "Util/LRUCache.h"
#include "Util/ThreadPool.h"
#include "Util/Http.h"
#include "V8Tree/Service.h"

LOGGROUP(HttpQueue)

namespace RBX {
	class DataModel;
	class HttpQueueStatsItem;

class AsyncHttpQueue
	 : public boost::enable_shared_from_this<AsyncHttpQueue>
	 , public boost::noncopyable
{
	shared_ptr<HttpQueueStatsItem> statsItem;

public:
	typedef enum { Waiting, Succeeded, Failed } RequestResult;
	typedef enum { AsyncInline, AsyncNone, AsyncRead, AsyncWrite} ResultJob;

	typedef boost::function<void(RequestResult, std::istream*, shared_ptr<const std::string> response)> RequestCallback;

	struct CallbackWrapper
	{
		RequestCallback callback;
		ResultJob jobType;
		CallbackWrapper(RequestCallback callback, ResultJob jobType)
			:callback(callback)
			,jobType(jobType)
		{}
	};

	void setThreadPool(int count);
	
	bool isRequestQueueEmpty();
	bool isUrlBad(const std::string& id);

	void asyncRequest(const std::string& id, float priority, RequestCallback* callback, ResultJob jobType, bool ignoreBadRequests=false);
	bool syncRequest(const std::string& id);


	static void dispatchGenericCallback(boost::function<void(DataModel*)> theCallback, Instance* instance, ResultJob jobType);
	static void dispatchCallback(RequestCallback theCallback, Instance* instance, 
								 RequestResult result, boost::shared_ptr<const std::string> data, ResultJob jobType);

	AsyncHttpQueue(Instance* owner,boost::function<bool(const std::string& url, std::string* result)> getLocalFile, int threadCount);
	virtual ~AsyncHttpQueue();
	void onHeartbeat(const Heartbeat& heartbeatEvent);

	int getRequestQueueSize() const;

	void resetStatsItem(ServiceProvider* provider);
	double getAvgTimeInQueue() { return avgTimeInQueue.value(); }
	double getAvgRequestCompleteTime() { return avgRequestCompleteTime.value(); }
	int getNumSlowRequests() {return numSlowRequests;}

protected:
	virtual void registerContent(const std::string& url, shared_ptr<const std::string> response, shared_ptr<const std::string> filename)
	{}
	struct Request
	{
		std::string url;
		std::vector<CallbackWrapper> callbacks;
		float priority;
		boost::shared_ptr<Http> http; // keep a handle so we can cancel.
		RBX::Time startTime; // the time when this request was issued
		bool operator==(const std::string& url) const { return this->url==url; }
	};

	RunningAverage<double> avgTimeInQueue;				// in msec
	RunningAverage<double> avgRequestCompleteTime;		// in msec
	int numSlowRequests;

	struct FailedUrl
  	{
		std::string url;
		RBX::Time expiration;
		FailedUrl(const char* url);
		bool expired() const;
	};

	typedef std::list< Request > RequestList;
	typedef RequestList::iterator RequestHandle;

	struct AsyncRetryTask
	{
		double retryTime;
		RequestHandle request;
		AsyncRetryTask(RequestHandle request , double retryTime)
			:request(request), retryTime(retryTime)
		{}
	};

	mutable boost::recursive_mutex requestSync;		// synchronizes the requestQueue and failedUrls queue, and threadPool
			  									    // also used to protect modification of the http shared pointer.
	
	
	RequestList requestQueue;				// queue of requested URLs
	std::list< FailedUrl > failedUrls;		// List of bad URLs

	boost::scoped_ptr<PriorityThreadPool> threadPool;

	boost::recursive_mutex asyncRetrySync;
	std::queue<AsyncRetryTask> asyncRetryTasks;

	double currentWallTime;

	static void processRequests(boost::weak_ptr<AsyncHttpQueue> httpQueue, RequestHandle request, boost::shared_ptr<rbx::spin_mutex> lock);
	void addAsyncRetryTask(RequestHandle request);

	Instance* owner;
	boost::function<bool(const std::string& url, std::string* result)> getLocalFile;
};
}
