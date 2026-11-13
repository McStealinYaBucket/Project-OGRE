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

#include "rbx/RunningAverage.h"

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
		// Inline in 2013: the WinHttp/WinInet drivers (PLAYER 0x00826990, 0x00828250) build it in place
		// (std::exception ctor, string copy, vftable store, statusCode store).
		// FUNCTION: PLAYER 0x00823990
		// FUNCTION: STUDIO 0x00919c00
		http_status_error(int statusCode, const std::string& message)
			: std::runtime_error(message), statusCode(statusCode) {}
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
	// get() statistics, PLAYER 0x00de3e58-0x00de3e74 (2016 names; 2013 calls RbxInterlockedIncrement).
	static long cdnSuccessCount;			// 0x00de3e58 (non-empty alternateUrl)
	static long cdnFailureCount;			// 0x00de3e5c
	static long alternateCdnSuccessCount;	// 0x00de3e60
	static long alternateCdnFailureCount;	// 0x00de3e64
	static double lastCdnFailureTimeSpan;	// 0x00de3e68
	static long robloxSuccessCount;			// 0x00de3e70 (empty alternateUrl)
	static long robloxFailureCount;			// 0x00de3e74
	// Response-time windows sampled by get() under the matching lock (2016 names and types).
	static WindowAverage<double, double> robloxResponce;
	static WindowAverage<double, double> cdnResponce;

	// Instance layout (from ctors 0x253430 / 0x71335C and dispose 0x1E6086):
	//   +0  API instanceApi (ctor copies Http::defaultApi)
	//   +8  std::string url
	//   +12 std::map<std::string, std::string> additionalHeaders
	//   +36 std::string (scratch/response buffer used by httpGetPostImpl)
	// Windows 2013 layout (inline ctor PLAYER 0x00402b50 / dtor 0x00403cd0, both COMDAT copies
	// from the client shell): alternateUrl @0x00 (default-constructed), instanceApi @0x1c
	// (= defaultApi), url @0x20, additionalHeaders @0x3c; 0x48 bytes. The Mac order differs.
	std::string alternateUrl;	// 0x00
	API instanceApi;			// 0x1c

public:
	std::string url;			// 0x20
	std::map<std::string, std::string> additionalHeaders;	// 0x3c

	Http(const char* url):instanceApi(defaultApi),url(url) {}
	Http(const std::string& url):instanceApi(defaultApi),url(url) {}

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
	static std::string urlEncode(std::string s); // 0x250BF2

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

	// Windows transport (2013 util/WinHttp.cpp), PLAYER 0x00826990; same arguments as httpGetPostImpl.
	void httpGetPostWinHttp(bool isPost, std::istream& dataStream, bool compressData,
		const std::map<std::string, std::string>& additionalHeaders,
		bool allowExternal, std::string& response);

	// PLAYER 0x00623040: ThrowIfFailure(success, url.c_str(), message), inlined.
	void ThrowIfFailure(bool success, const char* message);

public:
	// PLAYER 0x00622cb0 (ret 0x20): called from the WinHttp redirect callback.
	void onWinHttpRedirect(unsigned long dwInternetStatus, std::string redirectUrl);

	static void ThrowIfFailure(bool success, const char* url, const char* message); // 0x1FA24C
	// PLAYER 0x00622b20: FormatMessageA, then "%s: %s, err=0x%X" or "%s: %s, %s".
	static void ThrowLastError(int err, const char* url, const char* message);
};

} // namespace RBX

// Template bodies util/WinHttp.cpp emits (identical to the 2013 bytes after relocation masking in a
// private compile, tools-style objscan of PLAYER 0x008239b0-0x00826990 and STUDIO 0x00919c00-0x0091cd80).
// Their classes (WinHttpRequest_source, String_sink) are local to WinHttp.cpp.

// SYNTHETIC: PLAYER 0x00827510
// SYNTHETIC: STUDIO 0x0091d900
// ??0http_status_error@RBX@@QAE@ABV01@@Z

// TEMPLATE: PLAYER 0x00823b00
// TEMPLATE: STUDIO 0x00919d80
// ?close@?$chain_base@V?$chain@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@CAXPAV?$linked_streambuf@DU?$char_traits@D@std@@@234@H@Z

// TEMPLATE: PLAYER 0x00823b50
// TEMPLATE: STUDIO 0x00919c30
// ??_G?$chain_client@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@@detail@iostreams@boost@@UAEPAXI@Z

// TEMPLATE: PLAYER 0x00823b70
// ?reset@chain_impl@?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@boost@@QAEXXZ

// TEMPLATE: PLAYER 0x00823c00
// TEMPLATE: STUDIO 0x00919de0
// ?open@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@QAEXABUString_sink@@HH@Z

// TEMPLATE: PLAYER 0x00823c60
// TEMPLATE: STUDIO 0x00919e40
// ?open@?$indirect_streambuf@VWinHttpRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@QAEXABVWinHttpRequest_source@@HH@Z

// TEMPLATE: PLAYER 0x00823ce0
// TEMPLATE: STUDIO 0x00919ec0
// ?open@?$indirect_streambuf@V?$basic_null_device@DUoutput@iostreams@boost@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@QAEXABV?$basic_null_device@DUoutput@iostreams@boost@@@34@HH@Z

// TEMPLATE: PLAYER 0x00823d40
// TEMPLATE: STUDIO 0x00919f20
// ?write@?$non_blocking_adapter@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@iostreams@boost@@QAEHPBDH@Z

// TEMPLATE: PLAYER 0x00823d90
// ??1?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@UAE@XZ

// TEMPLATE: PLAYER 0x00823e10
// ?init_put_area@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@EAEXXZ

// TEMPLATE: PLAYER 0x008241a0
// TEMPLATE: STUDIO 0x0091a2a0
// ??1?$filtering_stream_base@V?$chain@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@Upublic_@23@@detail@iostreams@boost@@UAE@XZ

// TEMPLATE: PLAYER 0x00824220
// TEMPLATE: STUDIO 0x0091a320
// ??1?$filtering_stream_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@Upublic_@23@@detail@iostreams@boost@@UAE@XZ

// TEMPLATE: PLAYER 0x008242a0
// TEMPLATE: STUDIO 0x0091a420
// ?underflow@?$indirect_streambuf@VWinHttpRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@MAEHXZ

// TEMPLATE: PLAYER 0x00824370
// TEMPLATE: STUDIO 0x0091a3a0
// ?imbue@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@MAEXABVlocale@std@@@Z

// TEMPLATE: PLAYER 0x008243a0
// TEMPLATE: STUDIO 0x0091a4f0
// ?sync_impl@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@AAEXXZ

// TEMPLATE: PLAYER 0x00824460
// TEMPLATE: STUDIO 0x0091a5b0
// ??$copy_impl@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@UString_sink@@@detail@iostreams@boost@@YAHAAV?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@AAUString_sink@@HU?$bool_@$0A@@mpl@2@2@Z

// TEMPLATE: PLAYER 0x008245d0
// TEMPLATE: STUDIO 0x0091a720
// ?notify@?$filtering_stream_base@V?$chain@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@Upublic_@23@@detail@iostreams@boost@@EAEXXZ

// TEMPLATE: PLAYER 0x008246a0
// TEMPLATE: STUDIO 0x0091a7f0
// ?notify@?$filtering_stream_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@Upublic_@23@@detail@iostreams@boost@@EAEXXZ

// TEMPLATE: PLAYER 0x00824780
// TEMPLATE: STUDIO 0x0091a8d0
// ?overflow@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@MAEHH@Z

// TEMPLATE: PLAYER 0x008248c0
// TEMPLATE: STUDIO 0x0091a3d0
// ?close_impl@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@MAEXH@Z

// TEMPLATE: PLAYER 0x00824950
// TEMPLATE: STUDIO 0x0091aad0
// ??$copy_impl@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@@detail@iostreams@boost@@YAHAAV?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@2@AAV?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@HU?$bool_@$0A@@mpl@2@2@Z

// TEMPLATE: PLAYER 0x00824a10
// TEMPLATE: STUDIO 0x0091ab90
// ??_D?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@QAEXXZ

// TEMPLATE: PLAYER 0x00824a40
// TEMPLATE: STUDIO 0x0091abc0
// ??_D?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@QAEXXZ

// TEMPLATE: PLAYER 0x00824af0
// TEMPLATE: STUDIO 0x0091ac70
// ?open_impl@?$stream_buffer@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@iostreams@boost@@AAEXABUString_sink@@HH@Z

// TEMPLATE: PLAYER 0x00824c40
// TEMPLATE: STUDIO 0x0091adc0
// ?open_impl@?$stream_buffer@V?$basic_null_device@DUinput@iostreams@boost@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@iostreams@boost@@AAEXABV?$basic_null_device@DUinput@iostreams@boost@@@23@HH@Z

// TEMPLATE: PLAYER 0x00824d00
// ?imbue@?$indirect_streambuf@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@MAEXABVlocale@std@@@Z

// TEMPLATE: PLAYER 0x00824d30
// TEMPLATE: STUDIO 0x0091ae70
// ?open@?$indirect_streambuf@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@QAEXABV?$basic_gzip_compressor@V?$allocator@D@std@@@34@HH@Z

// TEMPLATE: PLAYER 0x00824e70
// TEMPLATE: STUDIO 0x0091afb0
// ?overflow@?$indirect_streambuf@V?$basic_null_device@DUoutput@iostreams@boost@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@MAEHH@Z

// TEMPLATE: PLAYER 0x00825330
// TEMPLATE: STUDIO 0x0091b4d0
// ?open_impl@?$stream_buffer@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@iostreams@boost@@AAEXABV?$basic_gzip_compressor@V?$allocator@D@std@@@23@HH@Z

// TEMPLATE: PLAYER 0x008253d0
// TEMPLATE: STUDIO 0x0091b570
// ?underflow@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@MAEHXZ

// TEMPLATE: PLAYER 0x00825480
// TEMPLATE: STUDIO 0x0091b620
// ?underflow@?$indirect_streambuf@V?$basic_null_device@DUoutput@iostreams@boost@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@MAEHXZ

// TEMPLATE: PLAYER 0x008258b0
// TEMPLATE: STUDIO 0x0091ba50
// ??0?$stream_buffer@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@iostreams@boost@@QAE@ABV?$basic_gzip_compressor@V?$allocator@D@std@@@12@HH@Z

// TEMPLATE: PLAYER 0x00825ac0
// TEMPLATE: STUDIO 0x0091be90
// ?seek_impl@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@AAE?AV?$fpos@H@std@@_JHH@Z

// TEMPLATE: PLAYER 0x00825cf0
// TEMPLATE: STUDIO 0x0091c0c0
// ??$push_impl@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@@?$chain_base@V?$chain@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@AAEXABV?$basic_gzip_compressor@V?$allocator@D@std@@@23@HH@Z

// TEMPLATE: PLAYER 0x00826050
// TEMPLATE: STUDIO 0x0091c4c0
// ?seekpos@?$indirect_streambuf@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@detail@iostreams@boost@@MAE?AV?$fpos@H@std@@V56@H@Z

// TEMPLATE: PLAYER 0x008260d0
// ?seekpos@?$indirect_streambuf@V?$basic_null_device@DUinput@iostreams@boost@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@boost@@MAE?AV?$fpos@H@std@@V56@H@Z

// TEMPLATE: PLAYER 0x00826150
// TEMPLATE: STUDIO 0x0091c540
// ?sync_impl@?$indirect_streambuf@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@AAEXXZ

// TEMPLATE: PLAYER 0x008262f0
// TEMPLATE: STUDIO 0x0091c6e0
// ??$copy@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@UString_sink@@@iostreams@boost@@YAHAAV?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@01@ABUString_sink@@HPAX2@Z

// TEMPLATE: PLAYER 0x00826330
// TEMPLATE: STUDIO 0x0091c720
// ??$checked_delete@Uimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@boost@@@boost@@YAXPAUimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@0@@Z

// TEMPLATE: PLAYER 0x00826350
// TEMPLATE: STUDIO 0x0091c740
// ?close_impl@?$indirect_streambuf@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@U?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@MAEXH@Z

// TEMPLATE: PLAYER 0x008263c0
// TEMPLATE: STUDIO 0x0091c7b0
// ??$close@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@iostreams@boost@@YAXAAV?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@1@H@Z

// TEMPLATE: PLAYER 0x00826400
// TEMPLATE: STUDIO 0x0091c7f0
// ??$close@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@iostreams@boost@@YAXAAV?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@1@H@Z

// TEMPLATE: PLAYER 0x00826440
// TEMPLATE: STUDIO 0x0091c830
// ??$copy@V?$basic_istream@DU?$char_traits@D@std@@@std@@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@iostreams@boost@@YAHAAV?$basic_istream@DU?$char_traits@D@std@@@std@@AAV?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@01@HPAX2@Z

// TEMPLATE: PLAYER 0x00826510
// TEMPLATE: STUDIO 0x0091c900
// ?dispose@?$sp_counted_impl_p@Uimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@boost@@@detail@boost@@UAEXXZ

// TEMPLATE: PLAYER 0x00826530
// ??$sp_pointer_construct@Uimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@boost@@U1234@@detail@boost@@YAXPAV?$shared_ptr@Uimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@boost@@@1@PAUimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@1@AAVshared_count@01@@Z

// TEMPLATE: PLAYER 0x008265a0
// ??$?0Uimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@boost@@@?$shared_ptr@Uimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@boost@@@boost@@QAE@PAUimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@1@@Z

// TEMPLATE: PLAYER 0x00826600
// TEMPLATE: STUDIO 0x0091c9f0
// ??0?$chain_base@V?$chain@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@IAE@XZ

// TEMPLATE: PLAYER 0x00826690
// TEMPLATE: STUDIO 0x0091ca80
// ??0?$filtering_stream_base@V?$chain@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@Upublic_@23@@detail@iostreams@boost@@IAE@XZ

// TEMPLATE: PLAYER 0x00826790
// TEMPLATE: STUDIO 0x0091cb80
// ??0?$filtering_stream_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@Upublic_@23@@detail@iostreams@boost@@IAE@XZ

// TEMPLATE: PLAYER 0x00826890
// TEMPLATE: STUDIO 0x0091cc80
// ??0?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x00826910
// TEMPLATE: STUDIO 0x0091cd00
// ??0?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x00827940
// TEMPLATE: STUDIO 0x0091aa10
// ?close_impl@?$indirect_streambuf@VWinHttpRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@MAEXH@Z

// TEMPLATE: PLAYER 0x00827aa0
// ??$seek@UString_sink@@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@?$device_wrapper_impl@Uany_tag@iostreams@boost@@@detail@iostreams@boost@@SA?AV?$fpos@H@std@@AAUString_sink@@PAV?$linked_streambuf@DU?$char_traits@D@std@@@123@_JHH@Z

// TEMPLATE: PLAYER 0x00827c90
// TEMPLATE: STUDIO 0x0068b780
// ??$seek@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@?$concept_adapter@UString_sink@@@detail@iostreams@boost@@QAE?AV?$fpos@H@std@@_JHHPAV?$linked_streambuf@DU?$char_traits@D@std@@@123@@Z

// Template and inline bodies util/WinInet.cpp emits (PLAYER 0x00827540-0x00828250, STUDIO
// 0x0091d930-0x0091e4b0); the bodies both transports share are listed once, above.

// TEMPLATE: PLAYER 0x00827550
// ?safeToLower@RBX@@YAXAAV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z

// TEMPLATE: PLAYER 0x008275d0
// TEMPLATE: STUDIO 0x0091d950
// ?open@?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@QAEXABVWiniInetRequest_source@@HH@Z

// TEMPLATE: PLAYER 0x00827660
// TEMPLATE: STUDIO 0x0091d9e0
// ??1?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@UAE@XZ

// TEMPLATE: PLAYER 0x008276f0
// TEMPLATE: STUDIO 0x0091da70
// ?set_auto_close@?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@UAEX_N@Z

// TEMPLATE: PLAYER 0x008277f0
// TEMPLATE: STUDIO 0x0091db70
// ?init_put_area@?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@EAEXXZ

// TEMPLATE: PLAYER 0x00827840
// TEMPLATE: STUDIO 0x0091dbc0
// ?imbue@?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@MAEXABVlocale@std@@@Z

// TEMPLATE: PLAYER 0x00827870
// TEMPLATE: STUDIO 0x0091dbf0
// ?underflow@?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@MAEHXZ

// TEMPLATE: PLAYER 0x00827a00
// TEMPLATE: STUDIO 0x0091dcc0
// ?open_impl@?$stream_buffer@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@iostreams@boost@@AAEXABVWiniInetRequest_source@@HH@Z

// TEMPLATE: PLAYER 0x00827ad0
// TEMPLATE: STUDIO 0x0091dd60
// ??0?$stream_buffer@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@iostreams@boost@@QAE@ABVWiniInetRequest_source@@HH@Z

// TEMPLATE: PLAYER 0x00827d60
// TEMPLATE: STUDIO 0x0091dfc0
// ??$push_impl@VWiniInetRequest_source@@@?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@boost@@AAEXABVWiniInetRequest_source@@HH@Z

// TEMPLATE: PLAYER 0x00827f60
// TEMPLATE: STUDIO 0x0091e1c0
// ?overflow@?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@MAEHH@Z

// TEMPLATE: PLAYER 0x008280c0
// TEMPLATE: STUDIO 0x0091e320
// ?seek_impl@?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@AAE?AV?$fpos@H@std@@_JHH@Z

// TEMPLATE: PLAYER 0x008281d0
// TEMPLATE: STUDIO 0x0091e430
// ?seekpos@?$indirect_streambuf@VWiniInetRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@detail@iostreams@boost@@MAE?AV?$fpos@H@std@@V56@H@Z

// Library and template bodies the WinHttp/WinInet drivers call whose 2013 copy sits in another
// object (ATL in the client shell, boost gzip in Network): our only copy is emitted by WinHttp.cpp,
// WinInet.cpp or Http.cpp, so these markers pair them with the copy the 2013 linker kept
// (instruction sequence identical; the two CStringT constructors differ by inlining).

// TEMPLATE: PLAYER 0x00401930
// TEMPLATE: STUDIO 0x006cb7f0
// ??0CUrl@ATL@@QAE@XZ

// TEMPLATE: PLAYER 0x00401980
// TEMPLATE: STUDIO 0x006cc4e0
// ?CrackUrl@CUrl@ATL@@QAEHPBDK@Z

// TEMPLATE: PLAYER 0x00406b30
// TEMPLATE: STUDIO 0x0041b900
// ??0?$CStringT@DV?$StrTraitATL@DV?$ChTraitsCRT@D@ATL@@@ATL@@@ATL@@QAE@PBD@Z

// TEMPLATE: PLAYER 0x0041b650
// TEMPLATE: STUDIO 0x004040f0
// ?_Inc@const_iterator@?$_Tree@V?$_Tmap_traits@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V12@U?$less@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@V?$allocator@U?$pair@$$CBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V12@@std@@@2@$0A@@std@@@std@@QAEXXZ

// TEMPLATE: PLAYER 0x00436af0
// TEMPLATE: STUDIO 0x0041b750
// ??1?$CStringT@DV?$StrTraitATL@DV?$ChTraitsCRT@D@ATL@@@ATL@@@ATL@@QAE@XZ

// TEMPLATE: PLAYER 0x00440d20
// TEMPLATE: STUDIO 0x0041e640
// ??0?$CStringT@DV?$StrTraitATL@DV?$ChTraitsCRT@D@ATL@@@ATL@@@ATL@@QAE@PB_W@Z

// TEMPLATE: PLAYER 0x00441c80
// TEMPLATE: STUDIO 0x0041fef0
// ??0CComBSTR@ATL@@QAE@PBD@Z

// TEMPLATE: PLAYER 0x005ecd10
// TEMPLATE: STUDIO 0x00684d40
// ??0gzip_params@iostreams@boost@@QAE@HHHHHV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0_J@Z

// TEMPLATE: PLAYER 0x005eceb0
// TEMPLATE: STUDIO 0x00684ee0
// ??1gzip_params@iostreams@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x005ed720
// TEMPLATE: STUDIO 0x00685740
// ?resize@?$basic_buffer@DV?$allocator@D@std@@@detail@iostreams@boost@@QAEXH@Z

// TEMPLATE: PLAYER 0x005eef80
// TEMPLATE: STUDIO 0x00687600
// ??1?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x005ef010
// TEMPLATE: STUDIO 0x00687690
// ??1?$basic_gzip_decompressor@V?$allocator@D@std@@@iostreams@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x005ef310
// TEMPLATE: STUDIO 0x00687990
// ??0?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@QAE@ABV012@@Z

// TEMPLATE: PLAYER 0x005ef7c0
// TEMPLATE: STUDIO 0x00687f00
// ??$throw_exception@Vfailure@ios_base@std@@@boost@@YAXABVfailure@ios_base@std@@@Z

// TEMPLATE: PLAYER 0x005efa40
// TEMPLATE: STUDIO 0x006881e0
// ??$wrap@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@@detail@iostreams@boost@@YA?AV?$basic_gzip_compressor@V?$allocator@D@std@@@12@ABV312@PAX@Z

// TEMPLATE: PLAYER 0x005efb20
// TEMPLATE: STUDIO 0x006882c0
// ?reset@?$optional@V?$concept_adapter@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@@detail@iostreams@boost@@@detail@iostreams@boost@@QAEXABV?$concept_adapter@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@@234@@Z

// TEMPLATE: PLAYER 0x005f0a40
// TEMPLATE: STUDIO 0x00689190
// ??$write@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@QAEHAAV?$linked_streambuf@DU?$char_traits@D@std@@@detail@12@PBDH@Z

// TEMPLATE: PLAYER 0x005f1fd0
// TEMPLATE: STUDIO 0x0068a7e0
// ??$close@V?$non_blocking_adapter@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@iostreams@boost@@@?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@QAEXAAV?$non_blocking_adapter@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@12@H@Z

// TEMPLATE: PLAYER 0x005f3cf0
// TEMPLATE: STUDIO 0x0068c320
// ??$close_all@V?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@23@@detail@iostreams@boost@@YAXAAV?$basic_gzip_compressor@V?$allocator@D@std@@@12@AAV?$linked_streambuf@DU?$char_traits@D@std@@@012@@Z

// TEMPLATE: PLAYER 0x005f41a0
// TEMPLATE: STUDIO 0x0068c700
// ??0?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@QAE@ABUgzip_params@12@H@Z

// TEMPLATE: PLAYER 0x005f43d0
// TEMPLATE: STUDIO 0x0068ca00
// ??0?$basic_gzip_decompressor@V?$allocator@D@std@@@iostreams@boost@@QAE@HH@Z

// TEMPLATE: PLAYER 0x005f4660
// TEMPLATE: STUDIO 0x0068cc90
// ??$push_impl@V?$basic_gzip_decompressor@V?$allocator@D@std@@@iostreams@boost@@@?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@boost@@AAEXABV?$basic_gzip_decompressor@V?$allocator@D@std@@@23@HH@Z

// TEMPLATE: PLAYER 0x005f5510
// TEMPLATE: STUDIO 0x0068db20
// ??0?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@boost@@IAE@XZ

// TEMPLATE: PLAYER 0x006ea480
// TEMPLATE: STUDIO 0x007727c0
// ??Y?$CSimpleStringT@D$0A@@ATL@@QAEAAV01@PBD@Z

// TEMPLATE: PLAYER 0x00739c80
// ??R?$function2@XPAV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PAVexception@2@@boost@@QBEXPAV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PAVexception@3@@Z

// TEMPLATE: PLAYER 0x008239c0
// TEMPLATE: STUDIO 0x00919c50
// ??$close_all@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@detail@iostreams@boost@@YAXAAV?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@@Z

// TEMPLATE: PLAYER 0x00823a60
// TEMPLATE: STUDIO 0x00919ce0
// ??$close_all@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@detail@iostreams@boost@@YAXAAV?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@@Z

// TEMPLATE: PLAYER 0x00824540
// TEMPLATE: STUDIO 0x0091a690
// ??1?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@UAE@XZ

// TEMPLATE: PLAYER 0x00824610
// TEMPLATE: STUDIO 0x0091a760
// ??1?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@UAE@XZ

// TEMPLATE: PLAYER 0x00824910
// TEMPLATE: STUDIO 0x0091aa90
// ??$read@UString_sink@@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@?$device_wrapper_impl@Uoutput@iostreams@boost@@@detail@iostreams@boost@@SAHAAUString_sink@@PAV?$linked_streambuf@DU?$char_traits@D@std@@@123@PADH@Z

// TEMPLATE: PLAYER 0x00825530
// TEMPLATE: STUDIO 0x0091b6d0
// ??$push_impl@UString_sink@@@?$chain_base@V?$chain@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uoutput@23@@detail@iostreams@boost@@AAEXABUString_sink@@HH@Z

// TEMPLATE: PLAYER 0x008256a0
// TEMPLATE: STUDIO 0x0091b840
// ??$push_impl@VWinHttpRequest_source@@@?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@boost@@AAEXABVWinHttpRequest_source@@HH@Z

// TEMPLATE: PLAYER 0x00825c50
// TEMPLATE: STUDIO 0x0091c020
// ??$execute_all@V?$copy_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@UString_sink@@@detail@iostreams@boost@@V?$device_close_all_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@234@V?$device_close_all_operation@UString_sink@@@234@@detail@iostreams@boost@@YAHV?$copy_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@UString_sink@@@012@V?$device_close_all_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@012@V?$device_close_all_operation@UString_sink@@@012@@Z

// TEMPLATE: PLAYER 0x008261e0
// TEMPLATE: STUDIO 0x0091c5d0
// ??$execute_all@V?$copy_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@@detail@iostreams@boost@@V?$device_close_all_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@@234@V?$device_close_all_operation@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@234@@detail@iostreams@boost@@YAHV?$copy_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@@012@V?$device_close_all_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@@012@V?$device_close_all_operation@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@012@@Z

// TEMPLATE: PLAYER 0x00826480
// ??$?0Uimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@boost@@@shared_count@detail@boost@@QAE@PAUimpl@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostreams@boost@@V?$allocator@D@std@@@iostreams@2@@Z

// TEMPLATE: PLAYER 0x00827980
// ??$write@VWiniInetRequest_source@@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@?$device_wrapper_impl@Uinput@iostreams@boost@@@detail@iostreams@boost@@SAHAAVWiniInetRequest_source@@PAV?$linked_streambuf@DU?$char_traits@D@std@@@123@PBDH@Z

// TEMPLATE: PLAYER 0x00474560
// TEMPLATE: STUDIO 0x00688110
// ??$throw_exception@Vlogic_error@std@@@boost@@YAXABVlogic_error@std@@@Z

// TEMPLATE: STUDIO 0x0068a7b0
// ??$seek@V?$basic_null_device@DUinput@iostreams@boost@@@iostreams@boost@@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@23@@?$device_wrapper_impl@Uany_tag@iostreams@boost@@@detail@iostreams@boost@@SA?AV?$fpos@H@std@@AAV?$basic_null_device@DUinput@iostreams@boost@@@23@PAV?$linked_streambuf@DU?$char_traits@D@std@@@123@_JHH@Z

// TEMPLATE: STUDIO 0x006d13d0
// ?_Buynode@?$list@PAV?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@V?$allocator@PAV?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@std@@@std@@IAEPAU_Node@?$_List_nod@PAV?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@V?$allocator@PAV?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@std@@@2@XZ

// TEMPLATE: STUDIO 0x0091aa50
// ??$write@VWinHttpRequest_source@@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@?$device_wrapper_impl@Uinput@iostreams@boost@@@detail@iostreams@boost@@SAHAAVWinHttpRequest_source@@PAV?$linked_streambuf@DU?$char_traits@D@std@@@123@PBDH@Z

// TEMPLATE: STUDIO 0x0091c990
// ??$?0Uchain_impl@?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@boost@@@?$shared_ptr@Uchain_impl@?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@boost@@@boost@@QAE@PAUchain_impl@?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Uinput@23@@detail@iostreams@1@@Z

// TEMPLATE: PLAYER 0x00825050
// ??0?$stream_buffer@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@iostreams@boost@@QAE@ABUString_sink@@HH@Z

// TEMPLATE: PLAYER 0x00825180
// ??0?$stream_buffer@VWinHttpRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@iostreams@boost@@QAE@ABVWinHttpRequest_source@@HH@Z

// TEMPLATE: PLAYER 0x00825810
// ??$execute_all@V?$copy_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@UString_sink@@@detail@iostreams@boost@@V?$device_close_all_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@234@@detail@iostreams@boost@@YAHV?$copy_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@UString_sink@@@012@V?$device_close_all_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@boost@@@012@@Z

// TEMPLATE: PLAYER 0x00825e40
// ??$execute_all@V?$copy_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@@detail@iostreams@boost@@V?$device_close_all_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@@234@@detail@iostreams@boost@@YAHV?$copy_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@V?$reference_wrapper@V?$filtering_stream@Uoutput@iostreams@boost@@DU?$char_traits@D@std@@V?$allocator@D@5@Upublic_@23@@iostreams@boost@@@2@@012@V?$device_close_all_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@@012@@Z

// TEMPLATE: PLAYER 0x004017e0
// TEMPLATE: STUDIO 0x006cadd0
// ?AtlUnescapeUrl@ATL@@YAHPBDPADPAKK@Z

// TEMPLATE: PLAYER 0x00401d70
// TEMPLATE: STUDIO 0x006cbb10
// ?Parse@CUrl@ATL@@AAEHPBD@Z

// TEMPLATE: PLAYER 0x004021e0
// TEMPLATE: STUDIO 0x006cb120
// ?InitFields@CUrl@ATL@@AAEXXZ

// TEMPLATE: PLAYER 0x0040b030
// TEMPLATE: STUDIO 0x0041ba00
// ?SetString@?$CSimpleStringT@D$0A@@ATL@@QAEXPBDH@Z

// TEMPLATE: PLAYER 0x0040b720
// TEMPLATE: STUDIO 0x0041bb70
// ?LoadStringA@?$CStringT@DV?$StrTraitATL@DV?$ChTraitsCRT@D@ATL@@@ATL@@@ATL@@QAEHPAUHINSTANCE__@@I@Z

// TEMPLATE: PLAYER 0x0041a940
// TEMPLATE: STUDIO 0x0041bc90
// ?AtlFindStringResourceInstance@ATL@@YAPAUHINSTANCE__@@IG@Z

// TEMPLATE: PLAYER 0x0041b990
// TEMPLATE: STUDIO 0x00420030
// ?AtlCrtErrorCheck@ATL@@YAHH@Z

// TEMPLATE: PLAYER 0x0041ba30
// TEMPLATE: STUDIO 0x0041bd80
// ?AtlThrowImpl@ATL@@YGXJ@Z

// TEMPLATE: PLAYER 0x0042eef0
// TEMPLATE: STUDIO 0x00420000
// ??1?$CAtlSafeAllocBufferManager@VCCRTAllocator@ATL@@@_ATL_SAFE_ALLOCA_IMPL@ATL@@QAE@XZ

// TEMPLATE: PLAYER 0x00441190
// TEMPLATE: STUDIO 0x0041eb00
// ??4?$CStringT@DV?$StrTraitATL@DV?$ChTraitsCRT@D@ATL@@@ATL@@@ATL@@QAEAAV01@PB_W@Z

// TEMPLATE: PLAYER 0x005e7920
// ?_Buynode@?$list@PAV?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@V?$allocator@PAV?$linked_s

// TEMPLATE: PLAYER 0x005ecdd0
// ??0gzip_header@detail@iostreams@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x005ecf40
// ??0gzip_params@iostreams@boost@@QAE@ABU012@@Z

// TEMPLATE: PLAYER 0x005ecfd0
// ?normalize_params@?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@CA?AUgzip_params@23@U423@@Z

// TEMPLATE: PLAYER 0x005ed050
// ?make_params@?$basic_gzip_decompressor@V?$allocator@D@std@@@iostreams@boost@@CA?AUgzip_params@23@H@Z

// TEMPLATE: PLAYER 0x005ef820
// ?open_impl@?$stream_buffer@VWinHttpRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@

// TEMPLATE: PLAYER 0x005f0640
// ?close_impl@?$basic_gzip_compressor@V?$allocator@D@std@@@iostreams@boost@@AAEXXZ

// TEMPLATE: PLAYER 0x005f0810
// ??$write@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@?$symmetric_filter@V?$zlib_compress

// TEMPLATE: PLAYER 0x005f0b70
// ??$close@V?$non_blocking_adapter@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@iostreams@b

// TEMPLATE: PLAYER 0x005f0d00
// ??$write_long@V?$non_blocking_adapter@V?$linked_streambuf@DU?$char_traits@D@std@@@detail@iostreams@boost@@@iostre

// TEMPLATE: PLAYER 0x005f2900
// ??$?0Uzlib_params@iostreams@boost@@@?$symmetric_filter@V?$zlib_compressor_impl@V?$allocator@D@std@@@detail@iostre

// TEMPLATE: PLAYER 0x005f2970
// ??$?0Uzlib_params@iostreams@boost@@@?$symmetric_filter@V?$zlib_decompressor_impl@V?$allocator@D@std@@@detail@iost

// TEMPLATE: PLAYER 0x005f3390
// ??0?$stream_buffer@V?$basic_gzip_decompressor@V?$allocator@D@std@@@iostreams@boost@@U?$char_traits@D@std@@V?$allo

// TEMPLATE: PLAYER 0x006ea1a0
// TEMPLATE: STUDIO 0x007724e0
// ?Append@?$CSimpleStringT@D$0A@@ATL@@QAEXPBDH@Z

// TEMPLATE: STUDIO 0x00686ec0
// ?cant_write@detail@iostreams@boost@@YA?AVfailure@ios_base@std@@XZ

// TEMPLATE: STUDIO 0x0091b170
// ??0?$stream_buffer@UString_sink@@U?$char_traits@D@std@@V?$allocator@D@3@Uoutput@iostreams@boost@@@iostreams@boost

// TEMPLATE: STUDIO 0x0091b320
// ??0?$stream_buffer@VWinHttpRequest_source@@U?$char_traits@D@std@@V?$allocator@D@3@Uinput@iostreams@boost@@@iostre

// TEMPLATE: STUDIO 0x0091b9b0
// ??$execute_all@V?$copy_operation@V?$reference_wrapper@V?$filtering_stream@Uinput@iostreams@boost@@DU?$char_traits

// TEMPLATE: STUDIO 0x0091c210
// ??$execute_all@V?$copy_operation@V?$reference_wrapper@V?$basic_istream@DU?$char_traits@D@std@@@std@@@boost@@V?$re

// TEMPLATE: STUDIO 0x0091c920
// ??$sp_pointer_construct@Uchain_impl@?$chain_base@V?$chain@Uinput@iostreams@boost@@DU?$char_traits@D@std@@V?$alloc
