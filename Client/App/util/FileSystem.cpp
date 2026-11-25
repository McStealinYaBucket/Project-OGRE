/* App/util/FileSystem.cpp -- October 2013 (Windows). PLAYER FileSystem.obj: VistaAPIs' dtor
   0x006ea0e0, its ATL COMDATs (CStringT::Delete 0x006ea0f0, Append 0x006ea1a0, operator+= 0x006ea480),
   getBaseCacheDirectory 0x006ea260, getUserDirectory 0x006ea4c0, getCacheDirectory 0x006eab90; asserts
   on 2013 lines 105 and 151. The other ATL helpers called here are the client shell's copies
   (0x00406b30, 0x00436af0, 0x00436b10, 0x00440f00, 0x0041b960); these native calls keep their /GL
   copies on standard conventions. filepathExists, clearCacheDirectory and the one-argument
   getUserDirectory are the earlier Mac carry. */
#include "stdafx.h"
#define _WINSOCKAPI_
#include <windows.h>
#include <atlstr.h>
#include <atlpath.h>
#include <shlobj.h>
#include "Util/FileSystem.h"
#include "RbxFormat.h"
#include "rbx/Debug.h"
#include "rbx/atomic.h"
#include "Util/standardout.h"
#include <io.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

// 2016's Win/VistaTools.h class (no 2013 copy in this tree). getKnownFolderPath +0, gShell32DLLInst +4.
class VistaAPIs
{
	typedef HRESULT (WINAPI* GetKnownFolderPathPtr)(const GUID& rfid, DWORD dwFlags, HANDLE hToken, PWSTR* ppszPath);
	GetKnownFolderPathPtr getKnownFolderPath;
	HMODULE gShell32DLLInst;
public:
	VistaAPIs() : getKnownFolderPath(NULL) { gShell32DLLInst = LoadLibraryA("Shell32.dll"); if (gShell32DLLInst) getKnownFolderPath = (GetKnownFolderPathPtr)GetProcAddress(gShell32DLLInst, "SHGetKnownFolderPath"); }
	// FUNCTION: PLAYER 0x006ea0e0
	// FUNCTION: STUDIO 0x007723c0
	~VistaAPIs() { if (gShell32DLLInst) ::FreeLibrary(gShell32DLLInst); }
	HRESULT SHGetKnownFolderPath(const GUID& rfid, DWORD dwFlags, HANDLE hToken, PWSTR* ppszPath) { if (getKnownFolderPath == NULL) return E_NOTIMPL; return getKnownFolderPath(rfid, dwFlags, hToken, ppszPath); }
	bool isVistaOrBetter() { OSVERSIONINFOA osvi = {0}; osvi.dwOSVersionInfoSize = sizeof(osvi); GetVersionExA(&osvi); if (osvi.dwMajorVersion < 6) return false; return true; }
};

namespace RBX {
	namespace FileSystem {

		// FUNCTION: PLAYER 0x006ea260
		// FUNCTION: STUDIO 0x007725a0
		std::string getBaseCacheDirectory(bool create)
		{
			static volatile long initialized = 0;
			static std::string cached;
			if (!cached.empty())
				return cached;
			CPath path;
			::GetTempPathA(MAX_PATH, path.m_strPath.GetBuffer(MAX_PATH));
			path.m_strPath.ReleaseBuffer();
			path.Append("Roblox");
			path.AddBackslash();
			if (create)
				if (!::CreateDirectoryA(path, NULL) && ::GetLastError() != ERROR_ALREADY_EXISTS)
					throw RBX::runtime_error("Unable to create %s", (const char*)path);
			if (!path.FileExists())
				return "";
			if (RbxInterlockedCompareExchange(&initialized, 1, 0) == 0)
				cached = std::string(path);
			return std::string(path);
		}
		// FUNCTION: PLAYER 0x006ea4c0
		// FUNCTION: STUDIO 0x00772800
		std::string getUserDirectory(bool create, FileSystemDir dir, const char* subDirectory)
		{
			if (dir == DirExe)
			{
				WCHAR pathBuffer[MAX_PATH];
				::GetModuleFileNameW(NULL, pathBuffer, MAX_PATH);
				CString path(pathBuffer);
				int start = path.ReverseFind('\\') + 1;
				path.Delete(start, path.GetLength() - start);
				if (create)
					if (!::CreateDirectoryA(path, NULL) && ::GetLastError() != ERROR_ALREADY_EXISTS)
						return "";
				if (subDirectory)
				{
					path += subDirectory;
					if (create)
						if (!::CreateDirectoryA(path, NULL) && ::GetLastError() != ERROR_ALREADY_EXISTS)
							return "";
				}
				CPath result(path);
				result.AddBackslash();
				return std::string(result);
			}

			VistaAPIs apis;
			if (apis.isVistaOrBetter())
			{
				GUID folderID;
				switch (dir)
				{
				case DirAppData:
					CLSIDFromString(L"{F1B32785-6FBA-4FCF-9D55-7B8E7F157091}", &folderID);
					break;
				case DirPicture:
					CLSIDFromString(L"{33E28130-4E1E-4676-835A-98395C3BC3BB}", &folderID);
					break;
				case DirVideo:
					CLSIDFromString(L"{18989B1D-99B5-455B-841C-AB7C74E4DDFC}", &folderID);
					break;
				default:
					RBXASSERT(false);
					break;
				}

				PWSTR knownPath = NULL;
				if (apis.SHGetKnownFolderPath(folderID, create ? KF_FLAG_CREATE : 0, NULL, &knownPath) != E_NOTIMPL)
				{
					CString path(knownPath);
					CoTaskMemFree(knownPath);

					path += "\\Roblox\\";
					if (create)
						if (!::CreateDirectoryA(path, NULL) && ::GetLastError() != ERROR_ALREADY_EXISTS)
							return "";
					if (subDirectory)
					{
						path += subDirectory;
						if (create)
							if (!::CreateDirectoryA(path, NULL) && ::GetLastError() != ERROR_ALREADY_EXISTS)
								return "";
					}
					CPath result(path);
					result.AddBackslash();
					return std::string(result);
				}
			}

			DWORD flags = create ? CSIDL_FLAG_CREATE : 0;
			std::string robloxDir = "Roblox\\";
			if (subDirectory)
				robloxDir += subDirectory;

			CPath path;
			HRESULT hr;
			switch (dir)
			{
			case DirAppData:
				hr = SHGetFolderPathAndSubDirA(NULL, CSIDL_LOCAL_APPDATA | flags, NULL, SHGFP_TYPE_CURRENT, robloxDir.c_str(), path.m_strPath.GetBuffer(MAX_PATH));
				break;
			case DirPicture:
				hr = SHGetFolderPathAndSubDirA(NULL, CSIDL_MYPICTURES | flags, NULL, SHGFP_TYPE_CURRENT, robloxDir.c_str(), path.m_strPath.GetBuffer(MAX_PATH));
				break;
			case DirVideo:
				hr = SHGetFolderPathAndSubDirA(NULL, CSIDL_MYVIDEO | flags, NULL, SHGFP_TYPE_CURRENT, robloxDir.c_str(), path.m_strPath.GetBuffer(MAX_PATH));
				break;
			default:
				RBXASSERT(false);
				hr = S_FALSE;
				break;
			}

			if (hr != S_OK && dir == DirAppData)
				hr = SHGetFolderPathAndSubDirA(NULL, CSIDL_COMMON_APPDATA | flags, NULL, SHGFP_TYPE_CURRENT, robloxDir.c_str(), path.m_strPath.GetBuffer(MAX_PATH));

			if (hr == S_OK)
			{
				path.m_strPath.ReleaseBuffer();
				path.AddBackslash();
				return std::string(path);
			}
			else
				return "";
		}

		// FUNCTION: PLAYER 0x006eab90
		// FUNCTION: STUDIO 0x00772ed0
		std::string getCacheDirectory(bool create, const char* subDirectory)
		{
			if (!subDirectory)
				return getBaseCacheDirectory(create);

			std::string dir = getBaseCacheDirectory(create);
			if (!CPath(dir.c_str()).FileExists())
				return "";

			dir += subDirectory;
			dir += "\\";
			if (create)
				if (!::CreateDirectoryA(dir.c_str(), NULL))
				{
					int error = ::GetLastError() != ERROR_ALREADY_EXISTS;
					if (error)
					{
						StandardOut::singleton()->printf(MESSAGE_ERROR, "Unable to create %s, %d", dir.c_str(), error);
						throw RBX::runtime_error("Unable to create %s, %d", dir.c_str(), error);
					}
				}

			CPath path(dir.c_str());
			if (!path.FileExists())
				return "";
			path.AddBackslash();
			return std::string(path);
		}

		void getUserDirectory(std::string& result)
		{
			// TODO(decompile): 2013 user-directory resolution.
			result.clear();
		}

		bool filepathExists(const boost::filesystem::path& p)
		{
			return boost::filesystem::exists(p);
		}


		// mac: 0x1F969E: see header.  Body from the 2016 carry (the 2013 mode int
		// is unused by the only call site).
		void clearCacheDirectory(const char* subDirectory, int mode)
		{
			(void)mode;
			std::string cachePath = getCacheDirectory(false, subDirectory);
			namespace fs = boost::filesystem;
			boost::system::error_code ec;
			if (!cachePath.empty() && fs::exists(cachePath, ec) && !ec)
			{
				fs::directory_iterator end_iter;
				for (fs::directory_iterator iter(cachePath); end_iter != iter; ++iter)
				{
					if (!fs::is_directory(iter->status()))
					{
						boost::system::error_code ec;
						boost::filesystem::remove(*iter, ec);
					}
				}
			}
		}

	} // namespace FileSystem
} // namespace RBX
