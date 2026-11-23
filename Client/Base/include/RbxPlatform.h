// Base/include/RbxPlatform.h: October 2013 reconstruction (Project OGRE).
// The OS include for Roblox TUs. On Windows it brings in a lean <windows.h> and <intrin.h>; the
// latter pulls in xmmintrin.h, which changes VC9's register conventions for static functions in
// every TU that sees it (CLAUDE.md: Lua probe, pushstr 0x008a7a50, newlstr). Keep both includes.
#pragma once

#if defined(WIN32)

#define VC_EXTRALEAN
#if !defined(STRICT)
#define STRICT 1
#endif
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN 1
#endif
#if !defined(NOMINMAX)
#define NOMINMAX 1
#endif

#include <windows.h>

// Undo the lean switches for later headers, and let winsock2 come in after windows.h.
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef _G3D_INTERNAL_HIDE_WINSOCK_
#undef _WINSOCKAPI_

#include <intrin.h>

#endif

#include "RbxBase.h"
