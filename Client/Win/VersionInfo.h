/* Win/VersionInfo.h -- October 2013 client shell. PJ Naughter's CVersionInfo (the version resource
   reader), refactored by Roblox to std::string, as far as the 2013 shell uses it: GetAppVersion
   (LogManager.cpp, PLAYER 0x0043eb80) builds one on the stack, loads the exe's resource and reads
   FileVersion. /OPT:REF dropped every other accessor. No 2013 __FILE__ string names the file; the
   path is the 2016 tree's.
   The ctor and dtor are inlined into GetAppVersion under LTCG; the dtor's out-of-line copy
   (PLAYER 0x00463f20, `this` in esi) is reached only from its unwind funclet. */

#pragma once

#include <windows.h>

#include <string>

class CVersionInfo
{
public:
	struct TRANSLATION
	{
		WORD m_wLangID;
		WORD m_wCodePage;
	};

	CVersionInfo();
	~CVersionInfo();

	BOOL Load(HMODULE module);
	BOOL Load(const std::string& fileName);
	std::string GetValue(const std::string& keyName);
	std::string GetFileVersionAsString();

protected:
	void Unload();

	WORD m_wLangID;                 // 0x00
	WORD m_wCharset;                // 0x02
	LPVOID m_pVerData;              // 0x04
	TRANSLATION* m_pTranslations;   // 0x08
	int m_nTranslations;            // 0x0c
	VS_FIXEDFILEINFO* m_pffi;       // 0x10
};
