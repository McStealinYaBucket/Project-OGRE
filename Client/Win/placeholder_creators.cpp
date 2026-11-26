// Shell template stand-in -- NOT original code, carries no marker (PLAYER and STUDIO: both shells
// compile Client/Win).
//
// Stands in for the 2013 shell code that instantiated FactoryProduct<FriendService>::Creator. The shell
// is /GL and linked first, so its copy is the one every caller reaches:
//   - in PLAYER, create 0x0041eb70, getClassName 0x004200e0, ctor 0x00422ee0, callDoDeclare 0x004273f0, in a
//     shell TU (0x0041e950..0x00428000) whose first Object.h include is spelled "util/object.h"
//     (string 0x00bda690: 152 of its 187 PLAYER references sit in the shell);
//   - in STUDIO, 0x00474490 / 0x00479490, spelled "Util/Object.h" (string 0x00d0a468).
// Instantiating it here keeps Network/Players.cpp, which reaches Object.h as "Util/Object.h", from
// supplying it once it includes V8DataModel/FriendService.h. Delete it when the real shell code is written.
//
// The Object.h include below must stay this TU's first.

// clang-format off
#ifdef OGRE_TARGET_STUDIO
#include "Util/Object.h"
#else
#include "util/object.h"
#endif
// clang-format on
#include "V8DataModel/FriendService.h"

extern "C" void ogreShellCreatorsStandInUse()
{
	RBX::FriendService::className();
}

// Keep it through /OPT:REF so its instantiations stay in the LTCG set.
#pragma comment(linker, "/include:_ogreShellCreatorsStandInUse")
