// Base/include/util/RegistryUtil.h: October 2013 reconstruction (Project OGRE).
// Evidence: Base\util\RegistryUtil.cpp is a 2013 __FILE__; its asserts ("key.size() > (pos + 1)",
// "Could not read registry key value.", "Could not write registry key value.", "valueData", "str")
// and five bodies sit at PLAYER 0x009bba70-0x009bc350. Keys are written as one string,
// "<root>\<sub keys>\<value>", where <root> is one of the HKEY_* names (HKEY_CLASSES_ROOT,
// HKEY_CURRENT_CONFIG, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_PERFORMANCE_DATA,
// HKEY_PERFORMANCE_NLSTEXT, HKEY_PERFORMANCE_TEXT, HKEY_USERS). Every accessor returns false when
// the key cannot be opened, read or written.
#pragma once

#if defined(_WIN32)
#include "pdh.h"

#include <string>

namespace RBX {

class RegistryUtil
{
public:
	static bool keyExists(const std::string& key);

	static bool read32bitNumber(const std::string& key, INT32& valueData);
	// With valueData NULL, only stores the value's size in dataSize.
	static bool readBinaryData(const std::string& key, BYTE* valueData, UINT32& dataSize);
	static bool readString(const std::string& key, std::string& valueData);

	static bool write32bitNumber(const std::string& key, INT32 valueData);
	static bool writeBinaryData(const std::string& key, const BYTE* valueData, UINT32 dataSize);
	static bool writeString(const std::string& key, const std::string& valueData);
};

}

#endif
