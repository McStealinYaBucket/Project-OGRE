/* Base/util/RegistryUtil.cpp: October 2013 reconstruction (Project OGRE).
   Evidence: this path is a 2013 __FILE__. PLAYER has the key parser at 0x009bba70 and keyExists,
   read32bitNumber, readBinaryData, readString and writeBinaryData at 0x009bbb90-0x009bc350; their
   asserts pin the lines below (53, 78, 93, 114, 133, 155, 230, 243, 260, 308). write32bitNumber
   and writeString leave no assert in either 2013 exe (nothing calls them, so /OPT:REF drops them);
   they are kept for the header's API. The parser is defined last although the 2013 image puts it
   first: moving it ahead of its callers would let /O2 consider inlining it.
   Every accessor starts the same way: find the first backslash, map the text before it to a root
   key, assert that something follows. OGRE_REG_ROOT spells that out once; it expands on the line of
   its use, so the assert inside keeps that line. OGRE_REG_SPLIT cuts "<sub keys>\<value>". */
#if defined(_WIN32)
// Keep windows.h from pulling in winsock.h; winsock2.h arrives later through boost.hpp.
#define _WINSOCKAPI_
#include "Util/RegistryUtil.h"
#undef min
#undef max
#include "Rbx/Debug.h"

// Older SDKs lack these roots.
#if !defined(HKEY_PERFORMANCE_DATA)
#define HKEY_PERFORMANCE_DATA ((HKEY)((LONG)0x80000004))
#endif
#if !defined(HKEY_PERFORMANCE_TEXT)
#define HKEY_PERFORMANCE_TEXT ((HKEY)((LONG)0x80000050))
#endif
#if !defined(HKEY_PERFORMANCE_NLSTEXT)
#define HKEY_PERFORMANCE_NLSTEXT ((HKEY)((LONG)0x80000060))
#endif

#define OGRE_REG_ROOT() \
	size_t pos = key.find('\\', 0); if (pos == std::string::npos) return false; \
	HKEY root = getKeyFromString(key.c_str(), pos); if (root == NULL) return false; \
	RBXASSERT(key.size() > (pos + 1))

#define OGRE_REG_SPLIT() \
	std::string path = key.substr(pos + 1, last - pos - 1); \
	std::string name = key.substr(last + 1, key.size() - last)

namespace RBX {

// Maps the leading "HKEY_..." name (its first `length` characters) to a predefined root.
static HKEY getKeyFromString(const char* str, size_t length);

// (spacing: keeps the asserts below on their 2013 line numbers)





// FUNCTION: PLAYER 0x009bbb90
bool RegistryUtil::keyExists(const std::string& key)
{
	OGRE_REG_ROOT();
	HKEY handle;
	INT32 result = RegOpenKeyEx(root, key.c_str() + pos + 1, 0, KEY_READ, &handle);
	if (result == ERROR_SUCCESS)
	{
		RegCloseKey(handle);
		return true;
	}
	else
		return false;
}

// (spacing: keeps the asserts below on their 2013 line numbers)









// FUNCTION: PLAYER 0x009bbc70
bool RegistryUtil::read32bitNumber(const std::string& key, INT32& valueData)
{
	OGRE_REG_ROOT();
	size_t last = key.rfind('\\');
	if (last != std::string::npos)
	{
		OGRE_REG_SPLIT();
		HKEY handle;
		INT32 result = RegOpenKeyEx(root, path.c_str(), 0, KEY_READ, &handle);
		if (result == ERROR_SUCCESS)
		{
			UINT32 size = sizeof(INT32);
			result = RegQueryValueEx(handle, name.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&valueData), reinterpret_cast<LPDWORD>(&size));




			RBXASSERT(result == ERROR_SUCCESS && "Could not read registry key value.");
			RegCloseKey(handle);
			return result == ERROR_SUCCESS;
		}
	}
	return false;
}

// (spacing: keeps the asserts below on their 2013 line numbers)









// FUNCTION: PLAYER 0x009bbeb0
bool RegistryUtil::readBinaryData(const std::string& key, BYTE* valueData, UINT32& dataSize)
{
	OGRE_REG_ROOT();
	size_t last = key.rfind('\\');
	if (last != std::string::npos)
	{
		OGRE_REG_SPLIT();
		HKEY handle;
		INT32 result = RegOpenKeyEx(root, path.c_str(), 0, KEY_READ, &handle);
		if (result == ERROR_SUCCESS)
		{
			// a NULL buffer asks only for the size
			if (valueData == NULL)
				result = RegQueryValueEx(handle, name.c_str(), NULL, NULL, NULL, reinterpret_cast<LPDWORD>(&dataSize));
			else
				result = RegQueryValueEx(handle, name.c_str(), NULL, NULL, valueData, reinterpret_cast<LPDWORD>(&dataSize));

			// (spacing: keeps the asserts below on their 2013 line numbers)



			RBXASSERT(result == ERROR_SUCCESS && "Could not read registry key value.");
			RegCloseKey(handle);
			return result == ERROR_SUCCESS;
		}
	}
	return false;
}

// (spacing: keeps the asserts below on their 2013 line numbers)










// FUNCTION: PLAYER 0x009bc100
bool RegistryUtil::readString(const std::string& key, std::string& valueData)
{
	OGRE_REG_ROOT();
	size_t last = key.rfind('\\');
	if (last != std::string::npos)
	{
		OGRE_REG_SPLIT();
		HKEY handle;
		INT32 result = RegOpenKeyEx(root, path.c_str(), 0, KEY_READ, &handle);
		if (result == ERROR_SUCCESS)
		{
			// first call sizes the value, second reads it into a zeroed buffer
			UINT32 size = 0;
			result = RegQueryValueEx(handle, name.c_str(), NULL, NULL, NULL, reinterpret_cast<LPDWORD>(&size));
			if (result == ERROR_SUCCESS)
			{
				char* text = new char[size];
				memset(text, 0, size);
				result = RegQueryValueEx(handle, name.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(text), reinterpret_cast<LPDWORD>(&size));
				if (result == ERROR_SUCCESS)
					valueData = text;
				delete[] text;
			}
			// no assert here in 2013: a missing string value is not an error
			RegCloseKey(handle);
			return result == ERROR_SUCCESS;
		}
	}
	return false;
}

bool RegistryUtil::write32bitNumber(const std::string& key, INT32 valueData)
{
	OGRE_REG_ROOT();
	size_t last = key.rfind('\\');
	if (last != std::string::npos)
	{
		OGRE_REG_SPLIT();
		HKEY handle;
		INT32 result = RegOpenKeyEx(root, path.c_str(), 0, KEY_ALL_ACCESS, &handle);
		if (result == ERROR_SUCCESS)
		{
			result = RegSetValueEx(handle, name.c_str(), NULL, REG_DWORD, reinterpret_cast<const BYTE*>(&valueData), sizeof(INT32));
			RBXASSERT(result == ERROR_SUCCESS && "Could not write registry key value.");
			RegCloseKey(handle);
			return result == ERROR_SUCCESS;
		}
	}
	return false;
}

// (spacing: keeps the asserts below on their 2013 line numbers)






















// FUNCTION: PLAYER 0x009bc350
bool RegistryUtil::writeBinaryData(const std::string& key, const BYTE* valueData, UINT32 dataSize)
{
	RBXASSERT(valueData);

	// (spacing: keeps the asserts below on their 2013 line numbers)










	OGRE_REG_ROOT();
	size_t last = key.rfind('\\');
	if (last != std::string::npos)
	{
		OGRE_REG_SPLIT();
		HKEY handle;
		INT32 result = RegOpenKeyEx(root, path.c_str(), 0, KEY_ALL_ACCESS, &handle);
		if (result == ERROR_SUCCESS)
		{
			if (valueData)
				result = RegSetValueEx(handle, name.c_str(), NULL, REG_BINARY, reinterpret_cast<const BYTE*>(valueData), dataSize);

			// (spacing: keeps the asserts below on their 2013 line numbers)




			RBXASSERT(result == ERROR_SUCCESS && "Could not write registry key value.");
			RegCloseKey(handle);
			return result == ERROR_SUCCESS;
		}
	}
	return false;
}

bool RegistryUtil::writeString(const std::string& key, const std::string& valueData)
{
	OGRE_REG_ROOT();
	size_t last = key.rfind('\\');
	if (last != std::string::npos)
	{
		OGRE_REG_SPLIT();
		HKEY handle;
		INT32 result = RegOpenKeyEx(root, path.c_str(), 0, KEY_ALL_ACCESS, &handle);
		if (result == ERROR_SUCCESS)
		{
			result = RegSetValueEx(handle, name.c_str(), NULL, REG_SZ, reinterpret_cast<const BYTE*>(valueData.c_str()), valueData.size() + 1);
			RBXASSERT(result == ERROR_SUCCESS && "Could not write registry key value.");
			RegCloseKey(handle);
			return result == ERROR_SUCCESS;
		}
	}
	return false;
}

// (spacing: keeps the asserts below on their 2013 line numbers)















// FUNCTION: PLAYER 0x009bba70
// FUNCTION: STUDIO 0x00a76590
static HKEY getKeyFromString(const char* str, UINT32 length)
{
	RBXASSERT(str);
	if (str)
	{
		// 2013 tests HKEY_CLASSES_ROOT a second time at the end of the chain.
		if (strncmp(str, "HKEY_CLASSES_ROOT", length) == 0)
			return HKEY_CLASSES_ROOT;
		else if (strncmp(str, "HKEY_CURRENT_CONFIG", length) == 0)
			return HKEY_CURRENT_CONFIG;
		else if (strncmp(str, "HKEY_CURRENT_USER", length) == 0)
			return HKEY_CURRENT_USER;
		else if (strncmp(str, "HKEY_LOCAL_MACHINE", length) == 0)
			return HKEY_LOCAL_MACHINE;
		else if (strncmp(str, "HKEY_PERFORMANCE_DATA", length) == 0)
			return HKEY_PERFORMANCE_DATA;
		else if (strncmp(str, "HKEY_PERFORMANCE_NLSTEXT", length) == 0)
			return HKEY_PERFORMANCE_NLSTEXT;
		else if (strncmp(str, "HKEY_PERFORMANCE_TEXT", length) == 0)
			return HKEY_PERFORMANCE_TEXT;
		else if (strncmp(str, "HKEY_CLASSES_ROOT", length) == 0)
			return HKEY_CLASSES_ROOT;
		else
			return NULL;
	}
	else
		return NULL;
}

}
#endif
