/* App/util/AsyncHttpQueue.cpp -- October 2013 (HTTP queue + stats item +
   file-local URL screening).
   Carried from March 2016 App/util/AsyncHttpQueue.cpp; every 2013 delta is
   binary-proven (batches 691-696):
     * 2013-only URL sanitizer (file-local static, sub_1E3B7A, 1028B) with
       exactly two call sites: processRequests net arm 0x1E29B8 and
       syncRequest 0x1E47F3. Both callers build a FRESH COPY of the url/id
       (copy ctor @0x1E29AD / @0x1E47EB, the copy destroyed immediately
       after the call) and RBX::Http still receives the ORIGINAL string
       (syncRequest builds Http from arg_4 @0x1E4814, not from the copy).
       Throws, in order: "invalid request" (statistics.ashx found),
       "invalid request 2" (/asset/ + isapproved), "invalid request 3"
       (//c), "invalid request 3" (//t without //test.roblox.com),
       "invalid request 5" (!Network::isTrustedContent). Messages at
       0xB34394/0xB343AF/0xB343C5/0xB343ED; needles "//c"@0xB343C1,
       "//t"@0xB343D7, "//test.roblox.com"@0xB343DB. The binary receives
       the string in ECX (thiscall-like ABI); ported as a plain file-local
       static -- both call sites are in this TU, so the private ABI is
       invisible. DELTA: helper name invented (no mangled symbol survives).
       First statement lowercases in place via safeToLower (the 2016 TU's
       vestigial #include "Util/SafeToLower.h" is this artifact).
     * processRequests: one print per catch -- only "Content failed for %s
       because %s" (0xB342F4) under the url.find("http") guard; the second
       MESSAGE_ERROR print and the shared_ptr<std::exception> threading are
       2016-only. Local-file arm passes filename.c_str() straight to
       ifstream (2016 utf8-decodes). Slow path (deltaT.seconds() >= 5.0,
       5.0@0xC26EB0) increments numSlowRequests only -- the two
       FASTLOG*F(FLog::SlowHttpRequest) lines at 2016:387-388 have no 2013
       counterpart. Net arm has no setExpectedAssetType/setCachePolicy.
       Callback fanout binds 4 args (no exception).
     * AsyncHttpQueue ctor 0x1E1D42 constructs both RunningAverages with an
       explicit 0.0 lerp (2016 uses 0.1; zero stores confirmed, no 0.1
       anywhere in the image). D0 0x1E21CA / D2 0x1E2204 (lock + clear +
       setThreadPool(0)).
     * No getFailedUrls/getRequestQueueUrls; const getRequestQueueSize
       0x1E2176 (unique_lock + size).
     * HttpQueueStatsItem: ctor 0x1E76F0 / init 0x1E7698 / update 0x1E7954 /
       create 0x1E4CEA. THREE children only -- no "Queue size" (children
       "Average time in queue"@0xB343FF, "Average process time"@0xB34415,
       "Num slow requests"@0xB3442A). update() calls Stats::Item::
       formatValue<int> (0x4A30CE) for numSlowRequests; this tree's int
       forwarder is formatValueIi (StatsItem.cpp:97). MI thunks ZThn32/
       ZThn36. create() used Creatable<Instance>::create in the binary;
       ported with the in-tree direct-new convention (DELTA inline).
     * resetStatsItem 0x1E209E: setParent(NULL) + reset(); then
       ServiceProvider::create<Stats::StatsService>(provider) (mangle
       __ZNK3RBX15ServiceProvider6createINS_5Stats12StatsServiceEEEPT_v --
       the static template inlines the const member create<T>()).
     * FailedUrl ctor 0x1E40B6 (expiration = now + 5*60 s; 300.0@0xC36400;
       #ifdef _DEBUG keeps 10*60), isUrlBad 0x1E4126 (verbatim 2016
       including the FASTLOGS + erase(iter, end) early-return),
       asyncRequest 0x1E422A, syncRequest 0x1E4788 (two-try; __cxa_rethrow
       @0x1E4B8A after pushing the failed url), callback static 0x1E3F7F
       ("ContentProvider callback exception: %s").
     * LOGVARIABLE(HttpQueue, 0): channel exists in the PE (FASTLOGS call
       sites) but its 2013 definition site is unknown; 2016 defines it in
       App/v8datamodel/FastLogSettings.cpp:128 (default 0). Defined here so
       FLog::HttpQueue links.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/AsyncHttpQueue.cpp */

#include "stdafx.h"

#include "Util/AsyncHttpQueue.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Stats.h"
#include "V8DataModel/StatsItem.h"
#include "V8DataModel/StatsService.h"
#include "V8DataModel/ContentProvider.h"
#include "Network/API.h"

#include "Util/SafeToLower.h"
#include "util/standardout.h"
#include "rbx/make_shared.h"
#include "Util/Object.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cctype>

LOGVARIABLE(HttpQueue, 0)

namespace RBX
{

class HttpQueueStatsItem : public StatsItem
{
	AsyncHttpQueue* httpQueue;
	StatsItem* avgTimeInQueue;
	StatsItem* avgRequestCompleteTime;
	StatsItem* numSlowRequests;

public:
	// STUB: PLAYER 0x0086f270
	// STUB: STUDIO 0x009bc430
	HttpQueueStatsItem(AsyncHttpQueue* _httpQueue, Instance* httpQueueOwner) : httpQueue(_httpQueue)
	{
		setName("HttpQueue_" + httpQueueOwner->getName());
	}

	static shared_ptr<HttpQueueStatsItem> create(AsyncHttpQueue* httpQueue, Instance* httpQueueOwner)
	{
		// DELTA: the binary went through Creatable<Instance>::create
		// (0x1E4CEA); in-tree convention for non-reflected helper items is
		// direct new + shared (see StatsItem.h note). Same observable.
		shared_ptr<HttpQueueStatsItem> result(new HttpQueueStatsItem(httpQueue, httpQueueOwner));
		result->init();
		return result;
	}

	// FUNCTION: PLAYER 0x0086f330
	// FUNCTION: STUDIO 0x009bc4e0
	virtual void update()
	{
		avgTimeInQueue->formatValue(httpQueue->getAvgTimeInQueue(), "%.4f msec", httpQueue->getAvgTimeInQueue());
		avgRequestCompleteTime->formatValue(httpQueue->getAvgRequestCompleteTime(), "%.4f msec", httpQueue->getAvgRequestCompleteTime());
		// DELTA: binary calls Stats::Item::formatValue<int>(int const&)
		// (0x4A30CE); formatValueIi (StatsItem.cpp:97) is this tree's int
		// forwarder -- same "%d" observable.
		numSlowRequests->formatValueIi(httpQueue->getNumSlowRequests());
	}

	// STUB: PLAYER 0x0086f610
	// STUB: STUDIO 0x009bc7a0
	void init()
	{
		avgTimeInQueue = createChildItem("Average time in queue");
		avgRequestCompleteTime = createChildItem("Average process time");
		numSlowRequests = createChildItem("Num slow requests");
	}
};

// 2013-only URL screening helper (see file-header note). Called with a
// fresh COPY at processRequests 0x1E29B8 / syncRequest 0x1E47F3.
static void sanitizeUrl(std::string& url)
{
	safeToLower(url);

	if (url.find("statistics.ashx") != std::string::npos)
		throw std::runtime_error("invalid request");
	if (url.find("/asset/") != std::string::npos && url.find("isapproved") != std::string::npos)
		throw std::runtime_error("invalid request 2");
	if (url.find("//c") != std::string::npos)
		throw std::runtime_error("invalid request 3");
	if (url.find("//t") != std::string::npos && url.find("//test.roblox.com") == std::string::npos)
		throw std::runtime_error("invalid request 3");
	if (!RBX::Network::isTrustedContent(url.c_str()))
		throw std::runtime_error("invalid request 5");
}

AsyncHttpQueue::AsyncHttpQueue(Instance* owner, boost::function<bool(const std::string& url, std::string* result)> getLocalFile, int threadCount)
	:owner(owner)
	,getLocalFile(getLocalFile)
	,avgRequestCompleteTime(0.0)
	,avgTimeInQueue(0.0)
	,currentWallTime(0)
	,numSlowRequests(0)
{
	setThreadPool(threadCount);
}

void AsyncHttpQueue::resetStatsItem(ServiceProvider* provider)
{
	if (statsItem)
	{
		statsItem->setParent(NULL);
		statsItem.reset();
	}

	// create the stats item
	Stats::StatsService* stats = ServiceProvider::create<Stats::StatsService>(provider);
	if (stats)
	{
		statsItem = HttpQueueStatsItem::create(this, owner);
		statsItem->setParent(stats);
	}
}

int AsyncHttpQueue::getRequestQueueSize() const
{
	boost::recursive_mutex::scoped_lock lock(requestSync);
	return requestQueue.size();
}

void AsyncHttpQueue::setThreadPool(int count)
{
	if(!threadPool || (threadPool->getThreadCount() != count)){
		threadPool.reset(new PriorityThreadPool(count, ThreadPool::NoAction));
	}
}

// STUB: PLAYER 0x00871000
// STUB: STUDIO 0x009be190
AsyncHttpQueue::~AsyncHttpQueue()
{
	{
		boost::recursive_mutex::scoped_lock lock(requestSync);

		requestQueue.clear();
	}

	//  We must join the thread since the work_function is a member function of this
	setThreadPool(0);
}

void AsyncHttpQueue::addAsyncRetryTask(RequestHandle request)
{
	boost::recursive_mutex::scoped_lock lock(asyncRetrySync);
	asyncRetryTasks.push(AsyncRetryTask(request, currentWallTime + 5));
}


void AsyncHttpQueue::onHeartbeat(const Heartbeat& heartbeatEvent)
{
	std::list<AsyncRetryTask> objectsToRequeue;
	{
		boost::recursive_mutex::scoped_lock lock(asyncRetrySync);
		currentWallTime += heartbeatEvent.wallStep;
		while(!asyncRetryTasks.empty() && asyncRetryTasks.front().retryTime < currentWallTime){
			objectsToRequeue.push_back(asyncRetryTasks.front());
			asyncRetryTasks.pop();
		}
	}

	if(!objectsToRequeue.empty()){
		boost::recursive_mutex::scoped_lock lock(requestSync);
		while(!objectsToRequeue.empty()){
			AsyncRetryTask& front = objectsToRequeue.front();
			threadPool->schedule(boost::bind(&AsyncHttpQueue::processRequests, weak_from(this), front.request, _1), front.request->priority);
			objectsToRequeue.pop_front();
		}
	}
}

static void InvokeAsyncCallback(AsyncHttpQueue::RequestCallback func, AsyncHttpQueue::RequestResult result, shared_ptr<const std::string> response)
{
	if(response){
		std::istringstream ss(*response);
		func(result, &ss, response);
	}
	else{
		func(result, NULL, response);
	}
}

void AsyncHttpQueue::dispatchGenericCallback(boost::function<void(DataModel*)> theCallback, Instance* instance, ResultJob jobType)
{
	switch(jobType)
	{
	case AsyncHttpQueue::AsyncInline:
		{
			theCallback(NULL);
			return;
		}
	case AsyncHttpQueue::AsyncNone:
		{
			if(instance){
				if(DataModel* dataModel = DataModel::get(instance)){
					dataModel->submitTask(theCallback, DataModelJob::None);
					return;
				}
			}
			break;;
		}
	case AsyncHttpQueue::AsyncRead:
		{
			if(instance){
				if(DataModel* dataModel = DataModel::get(instance)){
					dataModel->submitTask(theCallback, DataModelJob::Read);
					return;
				}
			}
			break;
		}
	case AsyncHttpQueue::AsyncWrite:
		{
			if(instance){
				if(DataModel* dataModel = DataModel::get(instance)){
					dataModel->submitTask(theCallback, DataModelJob::Write);
					return;
				}
			}
			break;
		}
	}
	throw std::runtime_error("Unable to dispatch task");
}


void AsyncHttpQueue::dispatchCallback(AsyncHttpQueue::RequestCallback theCallback, Instance* instance,
									  RequestResult result, boost::shared_ptr<const std::string> data, ResultJob jobType)
{
	dispatchGenericCallback(boost::bind(&InvokeAsyncCallback, theCallback, result, data), instance, jobType);
}

static void callback(AsyncHttpQueue::CallbackWrapper f, Instance* owner, AsyncHttpQueue::RequestResult result, shared_ptr<std::string> response)
{
	try
	{
		AsyncHttpQueue::dispatchCallback(f.callback, owner, result, response, f.jobType);
	}
	catch (RBX::base_exception& e)
	{
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_ERROR, "ContentProvider callback exception: %s", e.what());
	}
}

bool AsyncHttpQueue::isRequestQueueEmpty()
{
	boost::recursive_mutex::scoped_lock lock(requestSync);
	return requestQueue.empty();
}

void AsyncHttpQueue::processRequests(boost::weak_ptr<AsyncHttpQueue> weakHttpQueue, RequestHandle request, boost::shared_ptr<rbx::spin_mutex> criticalSection)
{
	shared_ptr<std::string> response(new std::string());
	RequestResult result;
	std::string url;
	std::string filename;
	
	if(boost::shared_ptr<AsyncHttpQueue> httpQueue = weakHttpQueue.lock())
	{
		boost::recursive_mutex::scoped_lock lock(httpQueue->requestSync);
		url = request->url;
	}
	else{
		return;
	}
	//Process the item
	try
	{
		try
		{
			try
			{

				if(boost::shared_ptr<AsyncHttpQueue> httpQueue = weakHttpQueue.lock())
				{
					
					bool localResult = httpQueue->getLocalFile(url, &filename);
					if(!localResult){
						result = Failed;
						return;
					}

					// sample average time spent in queue before actually fetching this request
					Time::Interval deltaTime = Time::now<Time::Fast>() - request->startTime;
					httpQueue->avgTimeInQueue.sample(deltaTime.msec());
				}

				if (!filename.empty()) // is a file on disk. do file load
				{
					rbx::spin_mutex::scoped_lock lock(*criticalSection);
					std::ifstream filestream(filename.c_str(), std::ios_base::in | std::ios_base::binary);
					std::ostringstream strbuffer;
					strbuffer << filestream.rdbuf();
					*response = strbuffer.str();
					result = Succeeded;
				}
				else
				{	
					//RBXASSERT(id.isHttp());
					// must be a net file.
					{
						std::string urlString(url);
						sanitizeUrl(urlString);
					}

					boost::shared_ptr<Http> httpRequest;
					httpRequest.reset(new RBX::Http(url));

					if(boost::shared_ptr<AsyncHttpQueue> httpQueue = weakHttpQueue.lock())					
					{
						// protect the usage of the shared pointer by resusing the requestSync lock.
						boost::recursive_mutex::scoped_lock lock(httpQueue->requestSync);

						request->http = httpRequest; // the sole purpose of saving the http object is to be able to cancel it.
					}

					httpRequest->get(*response);
					result = Succeeded;
				}
			}
			catch (RBX::http_status_error& e)
			{
				// This means the server has accepted  the request, but is taking longer to process, call again after some time and hopefully we will get a 200 OK once server is done processing
				if(e.statusCode == 202)
				{
					if(boost::shared_ptr<AsyncHttpQueue> httpQueue = weakHttpQueue.lock()) 
					{
						boost::recursive_mutex::scoped_lock lock(httpQueue->requestSync);
						//It accepted the request and we should try this request again later
						request->http.reset();
						httpQueue->addAsyncRetryTask(request);
					}
					return;
				}
				throw;
			}
		}
		catch (RBX::base_exception&)
		{
			result = Failed;
			if(boost::shared_ptr<AsyncHttpQueue> httpQueue = weakHttpQueue.lock()) {
				boost::recursive_mutex::scoped_lock lock(httpQueue->requestSync);
				FASTLOGS(FLog::HttpQueue, "Failed url on async fetch: %s", url);
				httpQueue->failedUrls.push_front(url.c_str());
			}
			
			throw;
		}
		
		if(boost::shared_ptr<AsyncHttpQueue> httpQueue = weakHttpQueue.lock()) {
			try
			{
				shared_ptr<std::string> filenamePtr;
				if(!filename.empty())
					filenamePtr.reset(new std::string(filename));
				httpQueue->registerContent(url, response, filenamePtr);
			}
			catch(RBX::base_exception&)
			{
				result = Failed;
				throw;
			}
		}
	}
	catch (std::logic_error& e)
	{
		if(url.find("http") != std::string::npos){
			RBX::StandardOut::singleton()->printf(RBX::MESSAGE_SENSITIVE, "Content failed for %s because %s", url.c_str(), e.what());
		}
		// the id has probably been put into failedUrls now
	}
	catch (RBX::base_exception& e)
	{
		if(url.find("http") != std::string::npos){
			RBX::StandardOut::singleton()->printf(RBX::MESSAGE_SENSITIVE, "Content failed for %s because %s", url.c_str(), e.what());
		}
		// the id has probably been put into failedUrls now
	}

	// keep a copy of the callbacks before we destroy them with the erase.
	
	if(boost::shared_ptr<AsyncHttpQueue> httpQueue = weakHttpQueue.lock())
	{
		Time::Interval deltaT = Time::now<Time::Fast>() - request->startTime;

		// increment slow request counter if we are over threshold
		if (deltaT.seconds() >= 5.0)
		{
			httpQueue->numSlowRequests++;
		}

		// sample average total process time between when request added to queue til callback
		httpQueue->avgRequestCompleteTime.sample(deltaT.msec());

		std::vector<CallbackWrapper> callbacks;
		{
			//Copy the callbacks and erase the request
			boost::recursive_mutex::scoped_lock lock(httpQueue->requestSync);
			callbacks = request->callbacks;
			httpQueue->requestQueue.erase(request);
		}

		{
			//Issue the callbacks
			rbx::spin_mutex::scoped_lock lock(*criticalSection);
			std::for_each(callbacks.begin(), callbacks.end(), boost::bind(&callback, _1, httpQueue->owner, result, response));
		}
	}
}

AsyncHttpQueue::FailedUrl::FailedUrl(const char* url)
	:url(url)
#ifdef _DEBUG
	,expiration(RBX::Time::now<Time::Fast>() + Time::Interval(10 * 60))
#else
	,expiration(RBX::Time::now<Time::Fast>() + Time::Interval(5 * 60))
#endif
{
}

bool AsyncHttpQueue::FailedUrl::expired() const
{
	return RBX::Time::now<Time::Fast>() >= expiration;
}


// STUB: PLAYER 0x0086e540
// STUB: STUDIO 0x009bb700
bool AsyncHttpQueue::isUrlBad(const std::string& id)
{
	if(id.empty())
		return true;

	boost::recursive_mutex::scoped_lock lock(requestSync);
	std::list< FailedUrl >::iterator end = failedUrls.end();
	for (std::list< FailedUrl >::iterator iter = failedUrls.begin(); iter!=end; ++iter)
	{
		if (iter->expired())	
		{
			FASTLOGS(FLog::HttpQueue, "Removing url from failed list, expired: %s", iter->url);
			failedUrls.erase(iter, end);
			return false;
		}
		if (iter->url==id)
			return true;
	}
	return false;
}

void AsyncHttpQueue::asyncRequest(const std::string& id, float priority, RequestCallback* callback, ResultJob jobType, bool ignoreBadRequests)
{
	if (ignoreBadRequests || !isUrlBad(id)) {
		boost::recursive_mutex::scoped_lock lock(requestSync);

		// See if it is already in the queue
		std::list<Request>::iterator iter = std::find(requestQueue.begin(), requestQueue.end(), id);						
		if (iter==requestQueue.end())
		{
			requestQueue.push_back(Request());
			RequestHandle r = requestQueue.end();
			r--; // get last element;
			r->url = id;
			r->priority = priority;
			r->startTime = RBX::Time::nowFast();
			if (callback)
				r->callbacks.push_back(CallbackWrapper(*callback, jobType));
			
			threadPool->schedule(boost::bind(&AsyncHttpQueue::processRequests, weak_from(this), r, _1), priority);
		}
		else {
			//It already exists
			if (callback)
			{
				iter->callbacks.push_back(CallbackWrapper(*callback, jobType));
			}
		}
	}
	else {
		if(callback)
			dispatchCallback(*callback, owner, Failed, shared_ptr<std::string>(), jobType);
	}
}
// STUB: PLAYER 0x0086eb40
// STUB: STUDIO 0x009bbd00
bool AsyncHttpQueue::syncRequest(const std::string& id)
{
	if (!isUrlBad(id)) {
		shared_ptr<std::string> response(new std::string());

		{
			std::string urlString(id);
			sanitizeUrl(urlString);
		}

		try
		{
			// TODO: Should we check for isBadUrl() here or only in the request queue?
				RBX::Http http(id);

                http.get(*response);
            }
		catch(RBX::base_exception&)
		{
			boost::recursive_mutex::scoped_lock lock(requestSync);
			FASTLOGS(FLog::HttpQueue, "Failed url on sync fetch: %s", id);
			failedUrls.push_front(id.c_str());
			throw;
		}
		try
		{
			registerContent(id, response, shared_ptr<const std::string>());
		}
		catch(RBX::base_exception&)
		{
			return false;
		}
		return true;
	}
	return false;
}

} // namespace RBX
