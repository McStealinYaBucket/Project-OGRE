#pragma once

// Reconstructed October 2013 header for Util/Http.h
// Primary evidence: symbolled October 23 2013 Studio (Mach-O i386).
// Original source path per ReleaseAssert string in Http::init():
//   /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/Darwin/Http.cpp (line 67)
// Structure template: 2016 Util/Http.h, trimmed to what the 2013 binary shows.

#include <string>
#include <map>
#include <istream>
#include <stdexcept>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

// TODO(dependency): rbx/mutex.h and util/ThreadPool.h are owned by their own
// modules; the 2013 binary uses RBX::mutex and RBX::ThreadPool (constructed
// with (8, 3) inside Http::init, 0x1FA134).
namespace RBX { class mutex; class ThreadPool; }

namespace RBX
{

	class http_status_error:
		public std::runtime_error
	{
	public:
		int statusCode;
		http_status_error(int statusCode);
		http_status_error(int statusCode, const std::string& message);
	};

class Http
{
public:
	// mac: 0x1FA134 Http::init(API) treats -1 as the uninitialized sentinel.
	// WinHttp/WinInet names per 2016 source; only Uninitialized is directly
	// evidenced by the assert string in this build.
	typedef enum { Uninitialized = -1, WinInet = 0, WinHttp = 1 } API;

	static std::string accessKey;
	static std::string gameSessionID;
	static std::string gameID;
	static std::string placeID;
	static std::string requester;

	static int playerCount;

private:
	static API defaultApi;

	// mac: 0x1FF782 MutexGuard::MutexGuard() news two RBX::mutex (0x2C bytes each)
	// and stores them here; 0x1FF710 dtor destroys them. Both appear as raw
	// pointers, matching the 2016 header.
	static RBX::mutex *robloxResponceLock; // 0x1FA110 getRobloxResponceLock()
	static RBX::mutex *cdnResponceLock;    // 0x1FA122 getCdnResponceLock()

	class MutexGuard
	{
	public:
		MutexGuard();  // 0x1FF782
		~MutexGuard(); // 0x1FF710 / 0x1FB9D0
	};

	static MutexGuard lockGuard;

	// Function-local static created in Http::init (0x1FA134): guarded
	// construction of RBX::ThreadPool(8, 3), kept alive via boost::scoped_ptr
	// (__cxa_atexit registered).
	static RBX::ThreadPool *httpThreadPool;

	// mac: 0x1FB406 Http::get(std::string&, bool) increments this on success via
	// RBX::RbxInterlockedIncrement (0x1E6086-era static in .bss).
	static long cdnSuccessCount;

	// Instance layout (from ctors 0x253430 / 0x71335C and dispose 0x1E6086):
	//   +0  API instanceApi (ctor copies Http::defaultApi)
	//   +8  std::string url
	//   +12 std::map<std::string, std::string> additionalHeaders
	//   +36 std::string (scratch/response buffer used by httpGetPostImpl)
	API instanceApi;
	void init(); // instance part of 0x1FA134

public:
	std::string url;
	std::map<std::string, std::string> additionalHeaders;	// +12 in the binary layout

	Http(const char* url);        // 0x71335C
	Http(const std::string& url); // 0x253430

	// Async
	// mac: 0x1FA3AA - schedules a bind of Http::get(std::string&, bool) on the
	// shared thread pool (dword_FAE0B0).
	void get(boost::function<void(std::string*, std::exception*)> handler, bool allowExternal = false);
	// mac: 0x1FA892 - schedules Http::post(std::istream&, bool, std::string&, bool)
	// with an istringstream over `input`.
	void post(const std::string& input, bool compress, boost::function<void(std::string*, std::exception*)> handler, bool externalRequest = false);
	// mac: 0x1FAD96 - schedules Http::post with the shared istream.
	void post(boost::shared_ptr<std::istream> input, bool compress, boost::function<void(std::string*, std::exception*)> handler, bool externalRequest = false);

	// Sync
	// mac: 0x1FA366
	void post(std::istream& input, bool compress, std::string& response, bool externalRequest = false);
	// mac: 0x1FB406
	void get(std::string& response, bool allowExternal = false);

	static void init(API api); // 0x1FA134

	static RBX::mutex *getRobloxResponceLock(); // 0x1FA110
	static RBX::mutex *getCdnResponceLock();    // 0x1FA122

	static bool isScript(const char* url);     // 0x1FB77E
	static bool isRobloxSite(const char* url); // 0x1FB93E
	static bool trustCheck(const char* url, bool allowExternal = false); // 0x1FB95C
	static bool trustCheckBrowser(const char* url); // 0x1FA0DA
	static bool isMoneySite(const char* url); // implemented via rbx_isMoneySite (0x5FE861) elsewhere

	// Utility
	static std::string urlEncode(const std::string& s); // 0x250BF2

private:
	// mac: 0x1FA2CA - Mac/Cocoa entry point; forwards to
	// RBX::Cocoa::httpGetPostCocoa (0x5FE9D1).
	void httpGetPostImpl(bool isPost, std::istream& dataStream, bool compressData,
		const std::map<std::string, std::string>& additionalHeaders,
		bool allowExternal, std::string& response);

	// Windows transport — reconstructed from the 0.125 Studio PE (no VMProtect).
	// Main path sub_91E4B0 (WinInet ANSI, UA "Roblox/WinInet" at 0xC0EA78);
	// read helper sub_91DAA0; trust check sub_6CD090; isScript sub_6CC910.
	// Export: F:\2013Decompile\exports\decomp\Http_WinInet_0125.cpp.txt
	void httpGetPostWinInet(bool isPost, std::istream& dataStream, bool compressData,
		const std::map<std::string, std::string>& additionalHeaders,
		bool allowExternal, std::string& response);

	static void ThrowIfFailure(bool success, const char* url, const char* message); // 0x1FA24C
};

} // namespace RBX
