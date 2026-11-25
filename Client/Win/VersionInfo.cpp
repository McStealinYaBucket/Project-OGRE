/* Win/VersionInfo.cpp -- October 2013 client shell (PLAYER and STUDIO compile it; see VersionInfo.h).
   The shell is /GL: Load(HMODULE) keeps __thiscall, Load(const std::string&) takes `this` in esi and
   the name in ecx, GetValue and GetFileVersionAsString take `this` in edi. 2013 differs from 2016 in
   Load: the module path goes through utf8_encode to the std::string overload, which copies the name
   and utf8_decodes the copy once per Win32 call. */

#include "VersionInfo.h"

#pragma comment(lib, "version.lib")

// Declared by headers this tree does not have yet (App's format_string.h, Base's StringConv.h).
std::string format_string(const char* fmt, ...); // PLAYER 0x00629910
namespace RBX
{
	std::string utf8_encode(const std::wstring& path); // PLAYER 0x009b8060
	std::wstring utf8_decode(const std::string& path); // PLAYER 0x009b8100
}

CVersionInfo::CVersionInfo()
{
	m_pVerData = NULL;
	m_pffi = NULL;
	m_wLangID = 0;
	m_wCharset = 1252;
	m_pTranslations = NULL;
	m_nTranslations = 0;
}

CVersionInfo::~CVersionInfo()
{
	Unload();
}

void CVersionInfo::Unload()
{
	m_pffi = NULL;
	if (m_pVerData)
	{
		delete[] m_pVerData;
		m_pVerData = NULL;
	}
	m_wLangID = 0;
	m_wCharset = 1252;
	m_pTranslations = NULL;
	m_nTranslations = 0;
}

// FUNCTION: PLAYER 0x00463f50
// FUNCTION: STUDIO 0x0048bb90
BOOL CVersionInfo::Load(HMODULE module)
{
	WCHAR name[500];
	::GetModuleFileNameW(module, name, 500);
	WCHAR path[_MAX_PATH];
	::GetShortPathNameW(name, path, _MAX_PATH);
	return Load(RBX::utf8_encode(std::wstring(path)));
}

// FUNCTION: PLAYER 0x00464050
// FUNCTION: STUDIO 0x0048bc90
BOOL CVersionInfo::Load(const std::string& fileName)
{
	Unload();

	std::string name(fileName);
	BOOL success = FALSE;
	DWORD handle = 0;
	DWORD size = ::GetFileVersionInfoSizeW(RBX::utf8_decode(name).c_str(), &handle);
	if (size)
	{
		m_pVerData = new BYTE[size];
		if (::GetFileVersionInfoW(RBX::utf8_decode(name).c_str(), handle, size, m_pVerData))
		{
			UINT length = 0;
			if (::VerQueryValueA(m_pVerData, "\\", (LPVOID*)&m_pffi, &length))
			{
				if (::VerQueryValueA(m_pVerData, "\\VarFileInfo\\Translation", (LPVOID*)&m_pTranslations, &length)
					&& length >= sizeof(TRANSLATION))
				{
					m_nTranslations = length / sizeof(TRANSLATION);
					m_wLangID = m_pTranslations[0].m_wLangID;
					m_wCharset = m_pTranslations[0].m_wCodePage;
				}
				success = TRUE;
			}
		}
	}

	if (!success && m_pVerData)
	{
		delete[] m_pVerData;
		m_pVerData = NULL;
	}
	return success;
}

// FUNCTION: PLAYER 0x00464200
// FUNCTION: STUDIO 0x0048be40
std::string CVersionInfo::GetValue(const std::string& keyName)
{
	std::string value;
	std::string query = format_string("\\StringFileInfo\\%04x%04x\\%s", m_wLangID, m_wCharset, keyName.c_str());

	LPCSTR text = NULL;
	UINT length = 0;
	if (::VerQueryValueA(m_pVerData, query.c_str(), (LPVOID*)&text, &length))
		value = text;
	return value;
}

// FUNCTION: PLAYER 0x00464370
// FUNCTION: STUDIO 0x0048bf40
std::string CVersionInfo::GetFileVersionAsString()
{
	return GetValue("FileVersion");
}
