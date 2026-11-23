// Base/include/rbx/RbxStrings.h: October 2013 reconstruction (Project OGRE).
// A portable case-insensitive strstr for toolchains without strcasestr. No 2013 binary carries a
// name for it (it is a static inline helper) and nothing in this tree includes the file; it is
// kept so later imports that expect it compile. Written for this tree, not taken from 2016.
#pragma once

#include <ctype.h>

#if !defined(Rbx_strcasestr)
namespace RBX {

// First occurrence of needle in haystack ignoring ASCII case, or 0. Empty or null inputs give 0.
static const char* Rbx_strcasestr(const char* haystack, const char* needle)
{
	if (!haystack || !needle || !*needle)
		return 0;
	for (; *haystack; ++haystack)
	{
		const char* h = haystack;
		const char* n = needle;
		while (*h && *n && toupper((unsigned char)*h) == toupper((unsigned char)*n))
		{
			++h;
			++n;
		}
		if (!*n)
			return haystack;
	}
	return 0;
}

static inline const char* Rbx_strcasestr(const char* haystack, char* needle)
{
	return Rbx_strcasestr(haystack, (const char*)needle);
}

static inline const char* Rbx_strcasestr(char* haystack, const char* needle)
{
	return Rbx_strcasestr((const char*)haystack, needle);
}

}
#endif
