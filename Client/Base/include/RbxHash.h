// Base/include/RbxHash.h: October 2013 reconstruction (Project OGRE).
// Pulls the pre-C++11 hash_map into scope under one name on both toolchains: Dinkumware keeps it in
// stdext (VC9), libstdc++ in __gnu_cxx (the Mac and iOS builds). No file in this tree includes it.
#pragma once

#if defined(_WIN32)
#include <hash_map>
using stdext::hash_map;
#else
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#endif
