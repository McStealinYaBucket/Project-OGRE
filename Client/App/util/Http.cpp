// Reconstructed October 2013 implementation for util/Darwin/Http.cpp
// Evidence: October 23 2013 symbolled Studio (Mach-O i386).
// Original path (assert string in Http::init, 0x1FA134):
//   /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/Darwin/Http.cpp line 67

#include "Util/Http.h"

#include <sstream>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include "rbx/Debug.h"
#include "util/ThreadPool.h"

#ifdef _WIN32
#include "RbxPlatform.h"
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
// OGRE vc9: was <zlib/zlib.h> from a 2016 Contribs zlib 1.2.8. PLAYER links one zlib, G3D's bundled
// 1.2.3, built stdcall (deflateInit2_ 0x009e1e80 ends ret 0x20), so callers need ZLIB_WINAPI.
#ifndef ZLIB_WINAPI
#define ZLIB_WINAPI
#endif
#include <zlib.h>
// (static-lib compile-check stage does not resolve externals).
#endif

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




// statics (addresses are .bss slots referenced by the decompiled code)
std::string Http::accessKey;
std::string Http::gameSessionID;
std::string Http::gameID;
std::string Http::placeID;
std::string Http::requester;

int Http::playerCount = 0;

Http::API Http::defaultApi = Http::Uninitialized;
RBX::mutex *Http::robloxResponceLock = NULL; // dword_FAE0A4-ish slot
RBX::mutex *Http::cdnResponceLock = NULL;
RBX::ThreadPool *Http::httpThreadPool = NULL; // dword_FAE0A0 / copied to dword_FAE0B0
long Http::cdnSuccessCount = 0;

Http::MutexGuard Http::lockGuard;

// mac: 0x1FF782 - news two RBX::mutex objects (sizeof == 0x2C) and stores them in
// the static pointers. Runs during static initialization.
Http::MutexGuard::MutexGuard()
{
	robloxResponceLock = new mutex();
	cdnResponceLock = new mutex();
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

// mac: 0x1FA110
RBX::mutex *Http::getRobloxResponceLock()
{
	return robloxResponceLock;
}

// mac: 0x1FA122
RBX::mutex *Http::getCdnResponceLock()
{
	return cdnResponceLock;
}

// mac: 0x1FA24C - Http::ThrowIfFailure(bool, const char*, const char*)
// Binary throws std::runtime_error("%s: %s") when success is false.
void Http::ThrowIfFailure(bool success, const char* url, const char* message)
{
	if (!success)
	{
		throw std::runtime_error(std::string(url) + ": " + message);
	}
}

http_status_error::http_status_error(int statusCode)
	: std::runtime_error(format("HTTP status %d", statusCode))
	, statusCode(statusCode)
{
}

http_status_error::http_status_error(int statusCode, const std::string& message)
	: std::runtime_error(message)
	, statusCode(statusCode)
{
}

#ifdef _WIN32
namespace {
	// RAII wrapper for the three WinInet handles (binary closes each explicitly
	// via InternetCloseHandle on every path; behaviorally identical).
	struct WinInetHandles
	{
		HINTERNET hOpen;
		HINTERNET hConnect;
		HINTERNET hRequest;
		WinInetHandles() : hOpen(NULL), hConnect(NULL), hRequest(NULL) {}
		~WinInetHandles()
		{
			if (hRequest) InternetCloseHandle(hRequest);
			if (hConnect) InternetCloseHandle(hConnect);
			if (hOpen) InternetCloseHandle(hOpen);
		}
	};

	// gzip inflate for Content-Encoding: gzip responses (sub_68CA00/68CC90 path).
	// Binary inits its helper with windowBits 15 (observed at sub_91E4B0); standard
	// gzip decoding needs 15+16 — used here with a plain-15 fallback.
	// TODO(decompile): confirm exact wrapper/args from sub_68CA00.
	static bool inflateGzip(const std::string& in, std::string& out)
	{
		z_stream strm;
		memset(&strm, 0, sizeof(strm));
		strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(in.data()));
		strm.avail_in = static_cast<uInt>(in.size());

		for (int windowBits = 15 + 16; ; windowBits = 15)
		{
			if (inflateInit2(&strm, windowBits) != Z_OK)
				return false;
			std::string result;
			char buf[4096];
			int ret;
			do {
				strm.next_out = reinterpret_cast<Bytef*>(buf);
				strm.avail_out = sizeof(buf);
				ret = inflate(&strm, Z_NO_FLUSH);
				if (ret != Z_OK && ret != Z_STREAM_END)
				{
					inflateEnd(&strm);
					if (windowBits > 15)
						break;  // retry as plain zlib wrapper
					return false;
				}
				result.append(buf, sizeof(buf) - strm.avail_out);
			} while (ret != Z_STREAM_END);
			inflateEnd(&strm);
			if (ret == Z_STREAM_END)
			{
				out.swap(result);
				return true;
			}
		}
	}

	// gzip deflate for compressData POST bodies (sub_68C700 path).
	// TODO(decompile): confirm exact wrapper/level from sub_68C700; Roblox-Content-*
	// hash headers (sub_9B66D0, bodies > 0x2710) still pending.
	static bool deflateGzip(const std::string& in, std::string& out)
	{
		z_stream strm;
		memset(&strm, 0, sizeof(strm));
		if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8,
			Z_DEFAULT_STRATEGY) != Z_OK)
			return false;
		strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(in.data()));
		strm.avail_in = static_cast<uInt>(in.size());
		std::string result;
		char buf[4096];
		int ret;
		do {
			strm.next_out = reinterpret_cast<Bytef*>(buf);
			strm.avail_out = sizeof(buf);
			ret = deflate(&strm, Z_FINISH);
			if (ret != Z_OK && ret != Z_STREAM_END)
			{
				deflateEnd(&strm);
				return false;
			}
			result.append(buf, sizeof(buf) - strm.avail_out);
		} while (ret != Z_STREAM_END);
		deflateEnd(&strm);
		out.swap(result);
		return true;
	}
}
#endif

// mac: 0x1FA2CA - Http::httpGetPostImpl(bool, std::istream&, bool,
//     const std::map<std::string,std::string>&, bool, std::string&)
// Mac build: validates trust for non-external requests (call_once guarded
// domain-list init at unk_FAE0B4 / sub_1FB9B3), then forwards to
// RBX::Cocoa::httpGetPostCocoa (0x5FE9D1).
void Http::httpGetPostImpl(bool isPost, std::istream& dataStream, bool compressData,
	const std::map<std::string, std::string>& additionalHeaders,
	bool allowExternal, std::string& response)
{
	if (!allowExternal)
	{
		static boost::once_flag domainsInitialized = BOOST_ONCE_INIT;
		boost::call_once(initRobloxDomains, domainsInitialized); // sub_1FB9B3 target
		ThrowIfFailure(isRobloxSite(url.c_str()), url.c_str(), "Unauthorized to connect to non-Roblox website");
	}
#ifdef RBX_PLATFORM_DARWIN
	Cocoa::httpGetPostCocoa(url, response, compressData, dataStream, isPost, additionalHeaders, response);
#else
	// Windows path — reconstructed from the 0.125 Studio PE (sub_91E4B0).
	// A parallel WinHTTP implementation (sub_91CD80) also exists in the binary;
	// instanceApi selects (WinHTTP port TODO).
	if (instanceApi == WinHttp)
		throw std::runtime_error("Http: WinHTTP transport not yet reconstructed (sub_91CD80)");
	httpGetPostWinInet(isPost, dataStream, compressData, additionalHeaders, allowExternal, response);
#endif
}

// mac: 0x1FA366 - Http::post(std::istream&, bool, std::string&, bool)
void Http::post(std::istream& input, bool compress, std::string& response, bool externalRequest)
{
	httpGetPostImpl(true, input, compress, additionalHeaders, externalRequest, response);
}

#ifdef _WIN32
// Windows transport — reconstructed from the 0.125 Studio PE (sub_91E4B0, WinInet/ANSI).
// Verified constants: UA "Roblox/WinInet" (0xC0EA78), HTTP/1.0, error strings
// "empty url" / "trust check failed for %s" / "'%s' is missing a hostName" /
// "InternetOpen failed for %s" / "InternetConnect failed for %s" /
// "HttpOpenRequest failed for %s" / "HttpAddRequestHeaders failed" /
// "HttpQueryInfo failed"; additional-header "Key: Value\r\n" loop; status query
// HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER (0x20000013); http_status_error
// on non-200/201 (reason via sub_919C00/sub_6CB1A0 in the binary).
// TODO(decompile, sub_91E4B0 details still open):
//   - URL parse uses a custom cracker (sub_6CB7F0/sub_6CC4E0, 2049-byte buffers);
//     InternetCrackUrlA used here (equivalent for standard URLs).
//   - szReferrer global passed to HttpOpenRequestA (content unknown).
//   - Non-secure flags raw 0x400040 from the binary (decode pending).
//   - gzip round-trip implemented via vendored zlib 1.2.8 (inflateGzip/deflateGzip);
//     exact binary wrapper/level (sub_68CA00 windowBits 15 observed, sub_68C700) TODO;
//     inflate-failure path (sub_68CC90) TODO.
//   - POST bodies: Roblox-Content-Size/First/Last/Hash via HttpSendRequestExA/
//     InternetWriteFile/HttpEndRequestA (sub_9B66D0, bodies > 0x2710) still pending;
//     simple HttpSendRequestA(body) used until then.
//   - Secure-connection cert pinning (.roblox.com/.robloxlabs.com check loop).
//   - GET status-callback (sub_91DF20) + InternetSetOptionA(0x2D) progress path.
void Http::httpGetPostWinInet(bool isPost, std::istream& dataStream, bool compressData,
	const std::map<std::string, std::string>& additionalHeaders,
	bool allowExternal, std::string& response)
{
	if (url.empty())
		throw std::runtime_error("empty url");

	if (!trustCheck(url.c_str(), allowExternal))
		throw std::runtime_error(format("trust check failed for %s", url.c_str()));

	// Crack the URL (binary: custom sub_6CC4E0; equivalent standard parse here)
	char hostName[256] = {0};
	char userName[128] = {0};
	char password[128] = {0};
	char urlPath[2048] = {0};
	URL_COMPONENTSA parts;
	memset(&parts, 0, sizeof(parts));
	parts.dwStructSize = sizeof(parts);
	parts.lpszHostName = hostName;
	parts.dwHostNameLength = ARRAYSIZE(hostName);
	parts.lpszUserName = userName;
	parts.dwUserNameLength = ARRAYSIZE(userName);
	parts.lpszPassword = password;
	parts.dwPasswordLength = ARRAYSIZE(password);
	parts.lpszUrlPath = urlPath;
	parts.dwUrlPathLength = ARRAYSIZE(urlPath);
	if (!InternetCrackUrlA(url.c_str(), 0, 0, &parts) || hostName[0] == '\0')
		throw std::runtime_error(format("'%s' is missing a hostName", url.c_str()));

	bool secure = (parts.nScheme == INTERNET_SCHEME_HTTPS);

	WinInetHandles h;
	h.hOpen = InternetOpenA("Roblox/WinInet", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!h.hOpen)
		throw std::runtime_error(format("InternetOpen failed for %s", url.c_str()));

	h.hConnect = InternetConnectA(h.hOpen, hostName, parts.nPort,
		userName[0] ? userName : NULL, password[0] ? password : NULL,
		INTERNET_SERVICE_HTTP, 0, 0);
	if (!h.hConnect)
		throw std::runtime_error(format("InternetConnect failed for %s", url.c_str()));

	// Binary: secure ? INTERNET_FLAG_SECURE (0x800000) : raw 0x400040.
	DWORD flags = secure ? INTERNET_FLAG_SECURE : 0x400040;
	h.hRequest = HttpOpenRequestA(h.hConnect, isPost ? "POST" : "GET", urlPath,
		"HTTP/1.0", NULL /* TODO: szReferrer global */, NULL, flags, 0);
	if (!h.hRequest)
		throw std::runtime_error(format("HttpOpenRequest failed for %s", url.c_str()));

	for (std::map<std::string, std::string>::const_iterator it = additionalHeaders.begin();
		it != additionalHeaders.end(); ++it)
	{
		std::string header = it->first + ": " + it->second + "\r\n";
		ThrowIfFailure(HttpAddRequestHeadersA(h.hRequest, header.c_str(),
			static_cast<DWORD>(header.size()), HTTP_ADDREQ_FLAG_ADD) != FALSE,
			url.c_str(), "HttpAddRequestHeaders failed");
	}

	// Binary always advertises gzip (sub_91E4B0 sends "Accept-Encoding: gzip").
	ThrowIfFailure(HttpAddRequestHeadersA(h.hRequest, "Accept-Encoding: gzip\r\n",
		static_cast<DWORD>(strlen("Accept-Encoding: gzip\r\n")), HTTP_ADDREQ_FLAG_ADD) != FALSE,
		url.c_str(), "HttpAddRequestHeaders failed");

	// Body for POST. Binary gzips when compressData (sub_68C700) via the
	// Ex/WriteFile/EndRequest path with Roblox-Content-* hash headers for large
	// bodies (sub_9B66D0 — TODO); simple deflate + Content-Encoding used here.
	std::string body;
	std::string contentEncoding;
	if (isPost)
	{
		body.assign(std::istreambuf_iterator<char>(dataStream), std::istreambuf_iterator<char>());
		if (compressData && !body.empty())
		{
			std::string gzipped;
			if (deflateGzip(body, gzipped))
			{
				body.swap(gzipped);
				contentEncoding = "Content-Encoding: gzip\r\n";
			}
		}
		if (!contentEncoding.empty())
			ThrowIfFailure(HttpAddRequestHeadersA(h.hRequest, contentEncoding.c_str(),
				static_cast<DWORD>(contentEncoding.size()), HTTP_ADDREQ_FLAG_ADD) != FALSE,
				url.c_str(), "HttpAddRequestHeaders failed");
	}

	ThrowIfFailure(HttpSendRequestA(h.hRequest, NULL, 0,
		isPost ? const_cast<char*>(body.data()) : NULL,
		isPost ? static_cast<DWORD>(body.size()) : 0) != FALSE,
		url.c_str(), format("HttpSendRequest failed: %u", GetLastError()).c_str());

	DWORD statusCode = 0;
	DWORD statusLen = sizeof(statusCode);
	ThrowIfFailure(HttpQueryInfoA(h.hRequest,
		HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
		&statusCode, &statusLen, NULL) != FALSE,
		url.c_str(), "HttpQueryInfo failed");

	// Read the response (binary read helper sub_91DAA0: QueryDataAvailable +
	// ERROR_IO_PENDING retry + InternetReadFile; straight read loop here).
	response.clear();
	char buf[4096];
	DWORD read = 0;
	while (InternetReadFile(h.hRequest, buf, sizeof(buf), &read) && read != 0)
		response.append(buf, buf + read);

	// Inflate gzip responses (binary checks Content-Encoding == "gzip", sub_68CA00).
	{
		char contentEncoding[64] = {0};
		DWORD encodingLen = sizeof(contentEncoding);
		if (HttpQueryInfoA(h.hRequest, HTTP_QUERY_CONTENT_ENCODING,
			contentEncoding, &encodingLen, NULL)
			&& _stricmp(contentEncoding, "gzip") == 0)
		{
			std::string inflated;
			if (inflateGzip(response, inflated))
				response.swap(inflated);
			// TODO(decompile): binary behavior on inflate failure (sub_68CC90 path).
		}
	}

	if (statusCode != 200 && statusCode != 201)
	{
		char statusText[512] = {0};
		DWORD textLen = sizeof(statusText);
		HttpQueryInfoA(h.hRequest, HTTP_QUERY_STATUS_TEXT, statusText, &textLen, NULL);
		throw http_status_error(static_cast<int>(statusCode), statusText[0] ? statusText : "HTTP error");
	}
}
#endif

// mac: 0x1FB406 - Http::get(std::string&, bool)
// STUB: PLAYER 0x00623310
// STUB: STUDIO 0x006cbf50
void Http::get(std::string& response, bool allowExternal)
{
	std::istringstream dataStream;
	httpGetPostImpl(false, dataStream, false, additionalHeaders, allowExternal, response);
	if (isCdnUrl(url))
	{
		InterlockedIncrement(&cdnSuccessCount);
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
