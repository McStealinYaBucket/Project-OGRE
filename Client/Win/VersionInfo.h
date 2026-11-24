/* Win/VersionInfo.h -- October 2013 client shell. PJ Naughter's CVersionInfo (the version resource
   reader), as far as GetAppVersion (LogManager.cpp, PLAYER 0x0043eb80) shows it: a 0x14-byte object
   on the stack whose constructor sets the 1252 (0x4e4) character set and zeroes the rest, and whose
   destructor delete[]s the version blob at +0x4. Load (PLAYER 0x00463f50) and
   GetFileVersionAsString (0x00464370, `this` in edi) live in VersionInfo.cpp, not written yet. Both
   are inlined under LTCG wherever they are defined, so they are written inline here. */

#pragma once

#include <windows.h>

#include <string>

class CVersionInfo
{
public:
	CVersionInfo()
		: m_wLangID(0)
		, m_wCharset(1252)
		, m_pVerData(NULL)
		, m_pTranslations(NULL)
		, m_nTranslations(0)
		, m_pffi(NULL)
	{
	}
	~CVersionInfo()
	{
		delete[] static_cast<BYTE*>(m_pVerData);
	}

	BOOL Load(HMODULE module);
	std::string GetFileVersionAsString();

protected:
	WORD m_wLangID;               // 0x00
	WORD m_wCharset;              // 0x02
	LPVOID m_pVerData;            // 0x04
	void* m_pTranslations;        // 0x08
	int m_nTranslations;          // 0x0c
	VS_FIXEDFILEINFO* m_pffi;     // 0x10
};
