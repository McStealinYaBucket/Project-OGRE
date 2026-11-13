// October 2013 util/Http.cpp (Windows). The one 2013 assert, RBXASSERT(false) in httpGetPostImpl,
// is at Http.cpp:100 in both PLAYER and STUDIO; the functions down to get() are in PLAYER address
// order (0x00622980-0x00623310). Below them, the Mac-derived remainder (init, async get/post, trust
// checks) still awaits the 2013 Windows bytes; its "mac:" addresses are MACSTUDIO ones.

#include "Util/Http.h"
#include "Util/standardout.h"
#include <sstream>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include "rbx/Debug.h"
#include "rbx/rbxtime.h"
#include "util/ThreadPool.h"

#ifdef _WIN32
#include "RbxPlatform.h"
#endif

namespace RBX
{

// GLOBAL: PLAYER 0x00de3eb4
// GLOBAL: STUDIO 0x00f66e9c
std::string Http::accessKey;
std::string Http::gameSessionID;
std::string Http::gameID;
std::string Http::placeID;
std::string Http::requester;

int Http::playerCount = 0;
Http::API Http::defaultApi = Http::Uninitialized;
// GLOBAL: PLAYER 0x00de3e78
// GLOBAL: STUDIO 0x00f66e60
RBX::mutex *Http::robloxResponceLock = NULL;
// GLOBAL: PLAYER 0x00de3e7c
// GLOBAL: STUDIO 0x00f66e64
RBX::mutex *Http::cdnResponceLock = NULL;
RBX::ThreadPool *Http::httpThreadPool = NULL; // dword_FAE0A0 / copied to dword_FAE0B0
long Http::cdnSuccessCount = 0;
Http::MutexGuard Http::lockGuard;

// mac: 0x1FF782 - news two RBX::mutex objects (sizeof == 0x2C) into the static pointers (static init).
// FUNCTION: PLAYER 0x00622980
// FUNCTION: STUDIO 0x006cac90
Http::MutexGuard::MutexGuard()
{
	robloxResponceLock = new mutex();
	cdnResponceLock = new mutex();
}

// FUNCTION: PLAYER 0x00622a60
// FUNCTION: STUDIO 0x006cb180
RBX::mutex *Http::getRobloxResponceLock()
{
	return robloxResponceLock;
}

// FUNCTION: PLAYER 0x00622a70
// FUNCTION: STUDIO 0x006cb190
RBX::mutex *Http::getCdnResponceLock()
{
	return cdnResponceLock;
}

// FUNCTION: PLAYER 0x00622a80
// FUNCTION: STUDIO 0x006cb1a0
http_status_error::http_status_error(int statusCode)
	: std::runtime_error(format("statusCode = %d", statusCode))
	, statusCode(statusCode)
{
}

#ifdef _WIN32
// FUNCTION: PLAYER 0x00622b20
// FUNCTION: STUDIO 0x006cb220
void Http::ThrowLastError(int err, const char* url, const char* message)
{
	char buffer[256];
	if (::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
			buffer, 256, NULL) == 0)
		throw RBX::runtime_error("%s: %s, err=0x%X", url, message, err);
	else
		throw RBX::runtime_error("%s: %s, %s", url, message, buffer);
}
#endif

// FUNCTION: PLAYER 0x00622bc0
// FUNCTION: STUDIO 0x006cb2c0
void Http::httpGetPostImpl(bool isPost, std::istream& dataStream, bool compressData,
	const std::map<std::string, std::string>& additionalHeaders,
	bool allowExternal, std::string& response)
{
	// if/else chain (test eax / cmp eax,1, two tail jumps); the default RBXASSERT is 2013 line 100.
	if (instanceApi == WinInet)
		httpGetPostWinInet(isPost, dataStream, compressData, additionalHeaders, allowExternal, response);
	else if (instanceApi == WinHttp)
		httpGetPostWinHttp(isPost, dataStream, compressData, additionalHeaders, allowExternal, response);
	else
		RBXASSERT(false);


}

// FUNCTION: PLAYER 0x00622c20
// FUNCTION: STUDIO 0x006cb320
void Http::post(std::istream& input, bool compress, std::string& response, bool externalRequest)
{
	httpGetPostImpl(true, input, compress, additionalHeaders, externalRequest, response);
}

// Statics of get() (defined below the line-100 block so the assert keeps its 2013 line).
long Http::cdnFailureCount = 0;
long Http::alternateCdnSuccessCount = 0;
long Http::alternateCdnFailureCount = 0;
double Http::lastCdnFailureTimeSpan = 0;
long Http::robloxSuccessCount = 0;
long Http::robloxFailureCount = 0;
WindowAverage<double, double> Http::robloxResponce(256);
WindowAverage<double, double> Http::cdnResponce(256);

// STUB: PLAYER 0x00622cb0
// STUB: STUDIO 0x006cb3b0
void Http::onWinHttpRedirect(unsigned long dwInternetStatus, std::string redirectUrl)
{
	size_t pos = redirectUrl.find("bg.roblox");
	if (pos != std::string::npos)
	{
		alternateUrl = redirectUrl.substr(0, pos);
		alternateUrl += redirectUrl.substr(pos + 2);
	}
	else
	{
		pos = redirectUrl.find("-cf.roblox");
		if (pos != std::string::npos)
		{
			alternateUrl = redirectUrl.substr(0, pos);
			alternateUrl += redirectUrl.substr(pos + 3);
		}
	}
}

// mac: 0x1FF710 (complete dtor) / 0x1FB9D0 (thunk)
Http::MutexGuard::~MutexGuard()
{
	if (robloxResponceLock)
	{
		delete robloxResponceLock;
		robloxResponceLock = NULL;
	}
	if (cdnResponceLock)
	{
		delete cdnResponceLock;
		cdnResponceLock = NULL;
	}
}

// FUNCTION: PLAYER 0x00622ec0
// FUNCTION: STUDIO 0x006cb6a0
void Http::ThrowIfFailure(bool success, const char* url, const char* message)
{
	if (!success)
	{
#ifdef _WIN32
		ThrowLastError(GetLastError(), url, message);
#else
		throw RBX::runtime_error("%s: %s", url, message);
#endif
	}
}

// FUNCTION: PLAYER 0x00623040
// FUNCTION: STUDIO 0x006cb840
void Http::ThrowIfFailure(bool success, const char* message)
{
	ThrowIfFailure(success, url.c_str(), message);
}


} // namespace RBX

// sub_1FB9B3 - initializes the trusted roblox domain list (call_once target)
namespace RBX
{
// pooled workers (defined below)
static void asyncGetWorker(Http* h, boost::function<void(std::string*, std::exception*)> fn, bool ext);
static void asyncPostBodyWorker(Http* h, std::string body, bool compress_, boost::function<void(std::string*, std::exception*)> fn, bool ext);
static void asyncPostStreamWorker(Http* h, boost::shared_ptr<std::istream> stream, bool compress_, boost::function<void(std::string*, std::exception*)> fn, bool ext);

static void initRobloxDomains()
{
	// TODO(decompile): exact domain table from 0x1FB9B3; populated set drives rbx_isRobloxSite
}

// mac: 0x5FE6F7 - free function in the binary
static int rbx_isRobloxSite(const char* url)
{
	// TODO(decompile): domain-list lookup (initialized by initRobloxDomains)
	return 0;
}

// mac: 0x5FE861 - free function in the binary
static int rbx_isMoneySite(const char* url)
{
	// TODO(decompile): domain-list lookup
	return 0;
}

// used by Http::get (0x1FB406)

static bool isCdnUrl(const std::string& url)
{
	// TODO(decompile): CDN host check from the binary
	return false;
}

// OGRE vc9: every nullptr in this file (11, all pointer contexts) is now NULL; nullptr is C++11.
// pooled workers for the async get/post entry points (0x1FA3AA / 0x1FA892 / 0x1FAD96)
static void asyncGetWorker(Http* h, boost::function<void(std::string*, std::exception*)> fn, bool ext)
{
	std::string response;
	try {
		h->get(response, ext);
		fn(&response, NULL);
	}
	catch (const std::exception&) {
		// TODO(decompile): exact exception propagation from the binary
		fn(&response, NULL);
	}
	delete h;
}

static void asyncPostBodyWorker(Http* h, std::string body, bool compress_, boost::function<void(std::string*, std::exception*)> fn, bool ext)
{
	std::istringstream stream(body);
	std::string response;
	try {
		h->post(stream, compress_, response, ext);
		fn(&response, NULL);
	}
	catch (const std::exception&) {
		fn(&response, NULL);
	}
	delete h;
}

static void asyncPostStreamWorker(Http* h, boost::shared_ptr<std::istream> stream, bool compress_, boost::function<void(std::string*, std::exception*)> fn, bool ext)
{
	std::string response;
	try {
		h->post(*stream, compress_, response, ext);
		fn(&response, NULL);
	}
	catch (const std::exception&) {
		fn(&response, NULL);
	}
	delete h;
}

// - rbx_isRobloxSite (0x5FE6F7) and rbx_isMoneySite (0x5FE861) are free

// mac: 0x1FA134 - Http::init(RBX::Http::API)
// Assert string verbatim from binary:
//   "Http::defaultApi==Http::Uninitialized file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/Darwin/Http.cpp line: 67"
void Http::init(API api)
{
	ReleaseAssert(defaultApi == Uninitialized,
		"Http::defaultApi==Http::Uninitialized file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/Darwin/Http.cpp line: 67");

	// Function-local static thread pool: RBX::ThreadPool(8, 3), held by a
	// boost::scoped_ptr (__cxa_atexit registered in the binary).
	static boost::scoped_ptr<ThreadPool> pool(new ThreadPool(8, BaseThreadPool::NoAction));
	httpThreadPool = pool.get();

	defaultApi = api;
}

// Http::httpGetPostWinInet is in util/WinInet.cpp, httpGetPostWinHttp in util/WinHttp.cpp.

// mac: 0x1FB406 - Http::get(std::string&, bool)
// STUB: PLAYER 0x00623310
// STUB: STUDIO 0x006cbf50
void Http::get(std::string& response, bool allowExternal)
{
	Time startTime = Time::now<Time::Fast>();
	std::istringstream dummy;
	bool shouldRetry = true;
	try
	{
		try
		{
			httpGetPostImpl(false, dummy, false, additionalHeaders, allowExternal, response);
		}
		catch (RBX::http_status_error& e)
		{
			if (e.statusCode == 202)
				shouldRetry = false;
			throw;
		}
		const double elapsed = (Time::now<Time::Fast>() - startTime).seconds();
		if (!alternateUrl.empty())
		{
			RbxInterlockedIncrement(&cdnSuccessCount);
			if (cdnResponceLock)
			{
				RBX::mutex::scoped_lock lock(*cdnResponceLock);
				cdnResponce.sample(elapsed);
			}
		}
		else
		{
			RbxInterlockedIncrement(&robloxSuccessCount);
			if (robloxResponceLock)
			{
				RBX::mutex::scoped_lock lock(*robloxResponceLock);
				robloxResponce.sample(elapsed);
			}
		}
	}
	catch (RBX::base_exception& e)
	{
		if (!shouldRetry)
			throw;

		response.clear();
		const double elapsed = (Time::now<Time::Fast>() - startTime).seconds();
		if (!alternateUrl.empty())
		{
			RbxInterlockedIncrement(&cdnFailureCount);
			lastCdnFailureTimeSpan = elapsed;
			RBX::StandardOut::singleton()->printf(RBX::MESSAGE_WARNING,
				"httpGet %s failed. Trying alternate %s. Error: %s.  Elapsed time: %g",
				url.c_str(), alternateUrl.c_str(), e.what(), elapsed);
			try
			{
				httpGetPostImpl(false, dummy, false, additionalHeaders, allowExternal, response);
				RbxInterlockedIncrement(&alternateCdnSuccessCount);
			}
			catch (RBX::base_exception&)
			{
				RbxInterlockedIncrement(&alternateCdnFailureCount);
				throw;
			}
		}
		else
		{
			RbxInterlockedIncrement(&robloxFailureCount);
			RBX::StandardOut::singleton()->printf(RBX::MESSAGE_WARNING,
				"httpGet %s failed. Trying again. Error: %s.  Elapsed time: %g",
				url.c_str(), e.what(), elapsed);
			httpGetPostImpl(false, dummy, false, additionalHeaders, allowExternal, response);
		}
	}
}

// mac: 0x1FA3AA - Http::get(boost::function<void(std::string*, std::exception*)>, bool)
// Binds (url copy, allowExternal copy, handler copy) and schedules on the
// shared thread pool (dword_FAE0B0).
void Http::get(boost::function<void(std::string*, std::exception*)> handler, bool allowExternal)
{
	std::string urlCopy = url;
	// runs the sync get() on a pooled worker and invokes handler(response, exc)
	httpThreadPool->schedule(boost::bind(
		&asyncGetWorker, new Http(urlCopy), handler, allowExternal));
}

// mac: 0x1FA892 - Http::post(const std::string&, bool, handler, bool)
void Http::post(const std::string& input, bool compress, boost::function<void(std::string*, std::exception*)> handler, bool externalRequest)
{
	std::string urlCopy = url;
	std::string inputCopy = input;
	httpThreadPool->schedule(boost::bind(
		&asyncPostBodyWorker, new Http(urlCopy), inputCopy, compress, handler, externalRequest));
}

// mac: 0x1FAD96 - Http::post(boost::shared_ptr<std::istream>, bool, handler, bool)
void Http::post(boost::shared_ptr<std::istream> input, bool compress, boost::function<void(std::string*, std::exception*)> handler, bool externalRequest)
{
	std::string urlCopy = url;
	httpThreadPool->schedule(boost::bind(
		&asyncPostStreamWorker, new Http(urlCopy), input, compress, handler, externalRequest));
}

// mac: 0x1FB77E - Http::isScript(const char*)
bool Http::isScript(const char* url)
{
	std::string s(url);
	// Hex-Rays shows substr(0, N) then compare against "javascript:" and
	// "jscript:" - i.e. prefix compares (substr lengths elided by Hex-Rays).
	return s.compare(0, 11, "javascript:") == 0
		|| s.compare(0, 8, "jscript:") == 0;
}

// mac: 0x1FB93E - Http::isRobloxSite(const char*)
bool Http::isRobloxSite(const char* url)
{
	return rbx_isRobloxSite(url) != 0;
}

// Http::isMoneySite -- declared in Http.h ("implemented via rbx_isMoneySite
// (0x5FE861) elsewhere"); the elsewhere never landed, so define it here,
// mirroring isRobloxSite.
bool Http::isMoneySite(const char* url)
{
	return rbx_isMoneySite(url) != 0;
}

// mac: 0x1FB95C - Http::trustCheck(const char*, bool)
// STUB: PLAYER 0x00624210
// STUB: STUDIO 0x006cd090
bool Http::trustCheck(const char* url, bool allowExternal)
{
	static boost::once_flag domainsInitialized = BOOST_ONCE_INIT;
	boost::call_once(initRobloxDomains, domainsInitialized); // sub_1FB9B3 target

	bool isRoblox = rbx_isRobloxSite(url) != 0;
	if (allowExternal)
	{
		return !isRoblox;
	}
	if (!isRoblox)
	{
		return isScript(url);
	}
	return true;
}

// mac: 0x1FA0DA - Http::trustCheckBrowser(const char*)
bool Http::trustCheckBrowser(const char* url)
{
	if (!trustCheck(url, false))
	{
		return rbx_isMoneySite(url) != 0;
	}
	return true;
}

// mac: 0x250BF2 - Http::urlEncode(std::string)
// Hex-Rays condition: keeps chars where (c >= '0') && ((c - ':') >= 7u) is
// false... reconstructed as the standard unreserved set the arithmetic
// encodes: alnum plus a small punctuation range; everything else is emitted
// as "%%%2X" (the format string in the binary is literally "%%%2X").
// FUNCTION: PLAYER 0x00629c80
// FUNCTION: STUDIO 0x006ca1b0
std::string Http::urlEncode(std::string s)
{
	std::string result;
	size_t len = s.size();
	for (size_t i = 0; i < len; ++i)
	{
		char c = s[i];
		if (c > '/' && !(c >= ':' && c <= '@') && !(c >= '[' && c <= '`') && c < '{')
			result += c;
		else
			result += format("%%%2X", c);
	}
	return result;
}

} // namespace RBX

// TODO(ownership):
//   functions shared with other Util code; implemented in their own module.
// - RBX::Cocoa::httpGetPostCocoa (0x5FE9D1) belongs to HttpCocoa.mm.
// - 0x1E505C/0x1E5F3E/0x1E6076..0x1E6108 are boost::shared_ptr<RBX::Http>
//   template instantiations (boost header code, not to be written out).
// - The 2013 async get/post bind shapes are approximate: Hex-Rays elided the
//   bind argument lists; member-call targets and value copies are confirmed,
//   exact boost::_bi::list wiring is not.
// - Statics observed in the binary that could not be attributed to a public
//   member: WindowAverage-style response time tracking implied by
//   robloxResponce/cdnResponce locks (2016 names kept in header comment).
