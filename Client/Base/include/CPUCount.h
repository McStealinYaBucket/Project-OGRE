// Base/include/CPUCount.h: October 2013 reconstruction (Project OGRE).
// Evidence: iOS 1029 symbol RbxTotalUsableCoreCount(unsigned int); PLAYER 0x009bd190, called with 1
// by TaskScheduler::setThreadCount 0x009b7ec0. Global namespace, as the mangled name shows.
#pragma once

// Usable core count, or defaultValue when the count cannot be read.
unsigned int RbxTotalUsableCoreCount(unsigned int defaultValue);
