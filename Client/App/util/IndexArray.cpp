/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */

// IndexArray is a header-only template (Util/IndexArray.h).
// The October 2013 binary contains only out-of-line template
// instantiations of IndexArray::fastRemove (emitted from the header,
// see evidence EAs listed in IndexArray.h); there is no module-level
// implementation code for Util/IndexArray.cpp.
//
// Related non-template code that merely consumes IndexArray
// (RBX::SleepStage::doContacts 0x59D55C, RBX::SleepStage::stepContacts
// mac: 0x59DCE0, both from Client/App/v8world/SleepStage.cpp lines 300-345)
// belongs to the SleepStage / v8world module, not here.

#include "Util/IndexArray.h"

// Intentionally empty.
