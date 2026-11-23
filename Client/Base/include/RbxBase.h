// Base/include/RbxBase.h: October 2013 reconstruction (Project OGRE).
// The include every Base header starts from. Release builds must have checked iterators off:
// Poly::getNumSurfaces (PLAYER 0x00754610) matches only under _SECURE_SCL=0 (CLAUDE.md).
#pragma once

#include <stdint.h>

#if defined(_DEBUG)
// unreachable code is an error in debug builds
#pragma warning(error: 4702)
#elif _SECURE_SCL != 0
#error Release builds need _SECURE_SCL defined to 0
#endif
