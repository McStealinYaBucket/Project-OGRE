// Base/include/HardwareInfo.h: October 2013 reconstruction (Project OGRE).
// Evidence: PLAYER 0x009bceb0 is an Intel-style processor census that RbxTotalUsableCoreCount
// 0x009bd190 calls (falling back to GetSystemInfo when it returns status 8; see Base/cpucount.cpp).
#pragma once

namespace RBX {

// Fills the logical, core and package counts; returns a status code.
unsigned char CPUCount(unsigned int* TotAvailLogical, unsigned int* TotAvailCore, unsigned int* PhysicalNum);

}
