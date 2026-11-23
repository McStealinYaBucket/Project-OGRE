// Base/include/RbxFormat.h: October 2013 reconstruction (Project OGRE).
// printf-style string helpers plus the portability spellings the engine leans on.
// Evidence: RBX::vformat at PLAYER 0x009b59c0 (a 100% match in Base/RbxFormat.cpp) and RBX::format
// at 0x009b5ba0; iOS 1029 symbols RBX::runtime_error(char const*, ...) and
// RBX::trim_trailing_slashes(std::string const&).
// Dropped: physics_receiver_exception and network_stream_exception. Neither has RTTI in PLAYER or
// STUDIO or a symbol on Mac/iOS, so 2013 never threw them, and nothing here uses them. The iOS-only
// base_exception variant is dropped with them; Windows uses std::exception.
#ifndef RBX_RBXFORMAT_H
#define RBX_RBXFORMAT_H

#include <string>
#include <stdio.h>
#include <cstdarg>
#include <stdexcept>

#include "RbxBase.h"

#if defined(__APPLE__)
#include <objc/objc.h>
#endif

#if defined(_WIN32)
// VC9 has only the underscored names.
#define snprintf _snprintf
#define strcasecmp stricmp
#else
// and the other platforms lack the _s names
#define sprintf_s snprintf
#define sscanf_s sscanf
#if !defined(DWORD)
#define DWORD uint32_t
#endif
#if !defined(ARRRAYSIZE)
#define ARRAYSIZE(x) (sizeof(x) / (sizeof(((x)[0]))))
#endif
#endif

// Lets gcc check format strings; nothing on MSVC.
#if defined(__GNUC__)
#define RBX_PRINTF_ATTR(string_index, first_to_check) __attribute__((format(printf, string_index, first_to_check)))
#else
#define RBX_PRINTF_ATTR(string_index, first_to_check)
#endif

namespace RBX {

// A std::runtime_error whose message is formatted like printf.
std::runtime_error runtime_error(const char* fmt, ...) RBX_PRINTF_ATTR(1, 2);

// What the engine catches as "any exception".
typedef std::exception base_exception;

// printf into a std::string of any length (a fixed stack buffer covers short results).
std::string format(const char* fmt...) RBX_PRINTF_ATTR(1, 2);
// The same, from a va_list.
std::string vformat(const char* fmt, va_list argPtr);

// path without the slashes it ends with
std::string trim_trailing_slashes(const std::string& path);

}

#endif
