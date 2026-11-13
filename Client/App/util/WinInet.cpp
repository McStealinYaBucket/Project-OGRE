// October 2013 util/WinInet.cpp: the WinInet transport of RBX::Http.
// Path: no 2013 __FILE__ string names this file (it has no asserts), so it follows the 2016 tree
// (App/util/WinInet.cpp). PLAYER 0x00827540-0x00828250 holds this object's template bodies and
// 0x00828250 the request driver (STUDIO 0x0091e4b0). Differences from 2016 seen in the 2013 bytes:
// no contentType or CSRF handling, no timeouts or external-request checks, no INTERNET_FLAG_RELOAD
// or NO_COOKIES, HttpSendRequestEx failure throws a plain std::runtime_error, the upload write is
// unconditional, and the body is read for every status code.

#include "stdafx.h"

#ifdef _WIN32

#define NOMINMAX

#include "winsock2.h"
#include "windows.h"
#include "wininet.h"

#include "Util/Http.h"
#include "Util/SafeToLower.h"
#include "Util/standardout.h"

#pragma comment(lib, "Wininet.lib")
#include "atlutil.h"
#include "RbxFormat.h"
#include "Util/MD5Hasher.h"

#include <sstream>

#define BOOST_IOSTREAMS_NO_LIB
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
namespace io = boost::iostreams;
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>

class WININETHINTERNET : boost::noncopyable
{
	HINTERNET handle;
public:
	WININETHINTERNET(HINTERNET handle):handle(handle) {}
	WININETHINTERNET():handle(0) {}
	operator bool() { return handle!=0; }
	operator HINTERNET() { return handle; }
	~WININETHINTERNET()
	{
		::InternetCloseHandle(handle);
	}
};

struct String_sink : public io::sink {
	std::string& s;
	String_sink(std::string& s):s(s){}
	std::streamsize write(const char* s, std::streamsize n)
	{
		this->s.append(s, n);
		return n;
	}
};

// RTTI .?AVWiniInetRequest_source@@ (sic).
class WiniInetRequest_source : public io::source {
	HINTERNET request;
	DWORD numBytesAvailable;
public:
	WiniInetRequest_source(HINTERNET request)
		: request(request)
		, numBytesAvailable(0)
	{}

	// FUNCTION: PLAYER 0x00827720
	// FUNCTION: STUDIO 0x0091daa0
	std::streamsize read(char* s, std::streamsize n)
	{
		if(numBytesAvailable == 0)
		{
			while (!::InternetQueryDataAvailable(request, &numBytesAvailable, 0, 0))
			{
				DWORD err = GetLastError();
				if(err == ERROR_IO_PENDING)
				{
					Sleep(100);
				}
				else
				{
					RBX::Http::ThrowLastError(err, "(unknown)", "InternetQueryDataAvailable");
				}
			}
		}

		if (numBytesAvailable==0)
			return -1; // EOF

		DWORD bytesRead;

		RBX::Http::ThrowIfFailure(TRUE == ::InternetReadFile(request, (LPVOID) s, std::min(numBytesAvailable, (DWORD)n), &bytesRead), "(unknown)", "InternetReadFile");
		numBytesAvailable -= bytesRead;
		return bytesRead;
	}
};

namespace RBX
{
	struct CallbackContext
	{
		Http* http;
	};

	// INTERNET_STATUS_CALLBACK; 2016 name.
	// FUNCTION: PLAYER 0x00827cc0
	// FUNCTION: STUDIO 0x0091df20
	static VOID CALLBACK callback(
	  __in  HINTERNET hInternet,
	  __in  DWORD_PTR dwContext,
	  __in  DWORD dwInternetStatus,
	  __in  LPVOID lpvStatusInformation,
	  __in  DWORD dwStatusInformationLength
	)
	{
		CallbackContext* context = reinterpret_cast<CallbackContext*>(dwContext);
		if(!context || !context->http)
			return;

		if (dwInternetStatus == INTERNET_STATUS_REDIRECT)
		{
			CString s = (LPCTSTR) lpvStatusInformation;
			context->http->onWinHttpRedirect(dwInternetStatus, (LPCTSTR) s);
		}
	}

	// FUNCTION: PLAYER 0x00828250
	// FUNCTION: STUDIO 0x0091e4b0
	void Http::httpGetPostWinInet(bool isPost, std::istream& data, bool compressData, const std::map<std::string, std::string>& additionalHeaders, bool externalRequest, std::string& response)
	{
		std::string theUrl = alternateUrl.empty() ? url : alternateUrl;

		if (theUrl.size()==0)
			throw std::runtime_error("empty url");

		if (!Http::trustCheck(theUrl.c_str(), externalRequest))
			throw RBX::runtime_error("trust check failed for %s", theUrl.c_str());

		CUrl u;
		u.CrackUrl(theUrl.c_str());

		const bool isSecure = u.GetScheme() == ATL_URL_SCHEME_HTTPS;

		if (u.GetHostNameLength()==0)
			throw RBX::runtime_error("'%s' is missing a hostName", theUrl.c_str());

		WININETHINTERNET session = InternetOpen("Roblox/WinInet", PRE_CONFIG_INTERNET_ACCESS, NULL, NULL, 0);
		if (!session)
			throw RBX::runtime_error("InternetOpen failed for %s", theUrl.c_str());

		WININETHINTERNET connection = ::InternetConnect(session, u.GetHostName(), u.GetPortNumber(), u.GetUserName(), u.GetPassword(), INTERNET_SERVICE_HTTP, 0, 0);
		if (!connection)
			throw RBX::runtime_error("InternetConnect failed for %s", theUrl.c_str());

		CString requestUrl = u.GetUrlPath();
		requestUrl += u.GetExtraInfo();

		DWORD flags;
		if (isSecure)
			flags = INTERNET_FLAG_SECURE;
		else
			flags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NEED_FILE;

		WININETHINTERNET request = ::HttpOpenRequest(
			connection, isPost ? "POST" : "GET", requestUrl, HTTP_VERSION, "", NULL,
			flags,
			0);
		if (!request)
			throw RBX::runtime_error("HttpOpenRequest failed for %s", theUrl.c_str());

		{
			std::string headers;
			for (std::map<std::string, std::string>::const_iterator iter = additionalHeaders.begin(); iter!=additionalHeaders.end(); ++iter)
				headers += iter->first + ": " + iter->second + "\r\n";

			if (headers.size()>0)
				ThrowIfFailure(TRUE==::HttpAddRequestHeaders(request, headers.c_str(), headers.size(), HTTP_ADDREQ_FLAG_ADD), "HttpAddRequestHeaders failed");
		}

		{
			const char* acceptEncoding = "Accept-Encoding: gzip\r\n";
			ThrowIfFailure(TRUE==::HttpAddRequestHeaders(request, acceptEncoding, strlen(acceptEncoding), HTTP_ADDREQ_FLAG_ADD), "HttpAddRequestHeaders failed");
		}

		const io::gzip_params gzipParams = io::gzip::default_compression;

		if (!isPost)
		{
			CallbackContext context;
			try
			{
				context.http = this;

				void* buff = &context;
				ThrowIfFailure(FALSE != InternetSetOption(request, INTERNET_OPTION_CONTEXT_VALUE, &buff, sizeof(void*)), "InternetSetOption");
				ThrowIfFailure(INTERNET_INVALID_STATUS_CALLBACK != InternetSetStatusCallback(request, &callback), "InternetSetStatusCallback");

				DWORD httpSendResult = ::HttpSendRequest(request, NULL, 0, 0, 0);

				if(httpSendResult != TRUE)
				{
					DWORD err = GetLastError();
					Http::ThrowLastError(err, url.c_str(), "HttpSendRequest");
				}

				InternetSetStatusCallback(request, NULL);
				context.http = 0;
			}
			catch(...)
			{
				InternetSetStatusCallback(request, NULL);
				context.http = 0;
				throw;
			}
		}
		else
		{
			std::string uploadData;
			try
			{
				io::filtering_ostream out;
				if (compressData)
				{
					std::string contentEncodingHeader = "Content-Encoding: gzip\r\n";
					ThrowIfFailure(TRUE==::HttpAddRequestHeaders(request, contentEncodingHeader.c_str(), contentEncodingHeader.size(), HTTP_ADDREQ_FLAG_ADD), "HttpAddRequestHeaders failed");
					out.push(io::gzip_compressor(gzipParams));
				}
				out.push(String_sink(uploadData));
				io::copy(data, out);
			}
			catch (io::gzip_error& e)
			{
				int e1 = e.error();
				int e2 = e.zlib_error_code();
				throw RBX::runtime_error("Upload GZip error %d, ZLib error %d, \"%s\"", e1, e2, theUrl.c_str());
			}
			const size_t uploadSize = uploadData.size();

			if (uploadSize>10000)
			{
				std::string contentSizeHeader = RBX::format("Roblox-Content-Size: %d\r\n", uploadSize);
				ThrowIfFailure(TRUE==::HttpAddRequestHeaders(request, contentSizeHeader.c_str(), contentSizeHeader.size(), HTTP_ADDREQ_FLAG_ADD), "HttpAddRequestHeaders failed");

				std::string contentFirstHeader = RBX::format("Roblox-Content-First: %d\r\n", uploadData.c_str()[0]);
				ThrowIfFailure(TRUE==::HttpAddRequestHeaders(request, contentFirstHeader.c_str(), contentFirstHeader.size(), HTTP_ADDREQ_FLAG_ADD), "HttpAddRequestHeaders failed");

				std::string contentLastHeader = RBX::format("Roblox-Content-Last: %d\r\n", uploadData.c_str()[uploadSize-1]);
				ThrowIfFailure(TRUE==::HttpAddRequestHeaders(request, contentLastHeader.c_str(), contentLastHeader.size(), HTTP_ADDREQ_FLAG_ADD), "HttpAddRequestHeaders failed");

				boost::scoped_ptr<MD5Hasher> hasher(MD5Hasher::create());
				hasher->addData(uploadData);
				std::string contentHashHeader = RBX::format("Roblox-Content-Hash: %s\r\n", hasher->c_str());
				ThrowIfFailure(TRUE==::HttpAddRequestHeaders(request, contentHashHeader.c_str(), contentHashHeader.size(), HTTP_ADDREQ_FLAG_ADD), "HttpAddRequestHeaders failed");
			}

			{
				INTERNET_BUFFERS buffer;
				memset(&buffer, 0, sizeof(buffer));
				buffer.dwStructSize = sizeof(buffer);
				buffer.dwBufferTotal = uploadSize;
				if (!HttpSendRequestEx(request, &buffer, NULL, 0, 0))
					throw std::runtime_error("HttpSendRequestEx failed");

				try
				{
					DWORD bytesWritten;
					ThrowIfFailure(TRUE==::InternetWriteFile(request, (const void*)uploadData.c_str(), uploadSize, &bytesWritten), "InternetWriteFile failed");

					if (bytesWritten!=uploadSize)
						throw std::runtime_error("Failed to upload content");
				}
				catch (RBX::base_exception&)
				{
					::HttpEndRequest(request, NULL, 0, 0);
					throw;
				}

				ThrowIfFailure(TRUE==::HttpEndRequest(request, NULL, 0, 0), "HttpEndRequest failed");
			}
		}

		DWORD statusCode;
		{
			DWORD dwLen = sizeof(DWORD);
			ThrowIfFailure(TRUE==::HttpQueryInfo(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &dwLen, NULL), "HttpQueryInfo failed");
		}

		try
		{
			io::filtering_istream in;
			{
				TCHAR buffer[256];
				DWORD length = 256;
				if (::HttpQueryInfo(request, HTTP_QUERY_CONTENT_ENCODING, (LPVOID) buffer, &length, 0))
					if (std::string(buffer)=="gzip")
						in.push(io::gzip_decompressor());
			}
			in.push(WiniInetRequest_source(request));
			io::copy(in, String_sink(response));
		}
		catch (io::gzip_error& e)
		{
			int e1 = e.error();
			int e2 = e.zlib_error_code();
			throw RBX::runtime_error("GZip error %d, ZLib error %d, \"%s\"", e1, e2, theUrl.c_str());
		}
		catch (io::zlib_error& e)
		{
			int e1 = e.error();
			throw RBX::runtime_error("ZLib error %d, \"%s\"", e1, theUrl.c_str());
		}

		if(statusCode == HTTP_STATUS_CREATED)
			return;

		if (statusCode != HTTP_STATUS_OK)
		{
			TCHAR buffer[512];
			DWORD length = 512;
			if(response != "" && externalRequest)
				RBX::StandardOut::singleton()->printf(MESSAGE_WARNING, "Error response: %s", response.c_str());

			if (::HttpQueryInfo(request, HTTP_QUERY_STATUS_TEXT, (LPVOID) buffer, &length, 0))
				throw RBX::http_status_error(statusCode,buffer);
			else
				throw RBX::http_status_error(statusCode);
		}

		if (isSecure)
		{
			INTERNET_CERTIFICATE_INFO certificateInfo;
			DWORD certInfoLength = sizeof(INTERNET_CERTIFICATE_INFO);
			if ( TRUE == InternetQueryOption( request,
											  INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT,
											  &certificateInfo,
											  &certInfoLength) )
			{
				if ( certificateInfo.lpszEncryptionAlgName )
					LocalFree( certificateInfo.lpszEncryptionAlgName);

				if ( certificateInfo.lpszIssuerInfo )
					LocalFree( certificateInfo.lpszIssuerInfo );

				if ( certificateInfo.lpszProtocolName )
					LocalFree( certificateInfo.lpszProtocolName );

				if ( certificateInfo.lpszSignatureAlgName )
					LocalFree( certificateInfo.lpszSignatureAlgName );

				if ( certificateInfo.lpszSubjectInfo )
				{
					bool foundIt = false;
					std::string s = certificateInfo.lpszSubjectInfo;
					LocalFree( certificateInfo.lpszSubjectInfo );

					int pos = s.find(0x0d);
					if (pos == std::string::npos)
						throw std::runtime_error("mc");

					std::string currString = s.substr(0,pos);

					if (currString.size() >= 11 &&
						(currString.find(".roblox.com") != std::string::npos || currString.find(".robloxlabs.com") != std::string::npos))
						foundIt = true;

					while(!foundIt && !externalRequest)
					{
						pos = s.find(0x0d,pos + 1);
						if (pos == std::string::npos)
							throw std::runtime_error("mc");

						int nextReturn = s.find(0x0d,pos + 1);
						if(nextReturn == std::string::npos)
							nextReturn = s.size();

						currString = s.substr(pos,nextReturn - pos);
						safeToLower(currString);

						if (currString.size() >= 11 &&
							(currString.find(".roblox.com") != std::string::npos || currString.find(".robloxlabs.com") != std::string::npos))
							foundIt = true;

						if(!foundIt && nextReturn == s.size())
						{
							std::string su = "u";
							su += "d";
							throw std::runtime_error(su);
						}
					}
				}
				else
					throw std::runtime_error("mi");
			}
			else
			{
				DWORD error = GetLastError();
			}
		}
	}
}

#endif // _WIN32
