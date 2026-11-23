// Base/include/rbx/SystemUtil.h: October 2013 reconstruction (Project OGRE).
// The 2013 set is the twelve queries Mac Studio and iOS 1029 implement as RBX::MacSystemUtil::*
// (getCPUMake, getCPUSpeed, getCPULogicalCount, getCPUCoreCount, getCPUPhysicalCount, isCPU64Bit,
// getMBSysRAM, getMBSysAvailableRAM, getVideoMemory, osVer, getGPUMake, getMaxRes). 2016's
// osPlatform, osPlatformId and deviceName have no 2013 symbol and are dropped.
// getVideoMemory: PLAYER 0x009b59b0; its caller (0x004568c2) compares eax only, so the Windows
// return type is 32 bits. ClientBase/RenderSettingsItem.cpp declares it that way itself; the other
// return types are unproven and stay 64-bit.
#pragma once

#include <string>

#if defined(_WIN32)
typedef unsigned __int64 uint64_t;
#endif

namespace RBX {
namespace SystemUtil {

	// processor
	std::string getCPUMake();
	uint64_t getCPUSpeed();
	uint64_t getCPULogicalCount();
	uint64_t getCPUCoreCount();
	uint64_t getCPUPhysicalCount();
	bool isCPU64Bit();

	// memory
	uint64_t getMBSysRAM();
	uint64_t getMBSysAvailableRAM();
	size_t getVideoMemory();

	// system, graphics card, display
	std::string osVer();
	std::string getGPUMake();
	std::string getMaxRes();

}
}
