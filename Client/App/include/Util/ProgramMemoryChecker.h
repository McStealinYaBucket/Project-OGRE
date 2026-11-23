/* Util/ProgramMemoryChecker.h -- October 2013 reconstruction.

   The 2013 memory checker is an abstract interface with one hidden
   implementation, not the 2016 concrete class:
   - PLAYER RTTI has RBX::ProgramMemoryChecker and a global-namespace
     ProgramMemoryCheckerImpl (??_7ProgramMemoryChecker@RBX@@6B@ 0x00b71238,
     ??_7ProgramMemoryCheckerImpl@@6B@ 0x00b71254);
   - the interface's vftable is { deleting dtor, _purecall, _purecall } and
     the implementation's is { 0x0081d960 deleting dtor, 0x0081d890 step,
     0x0066ef40 getLastCompletedHash }; iOS 10-29 and Mac name those
     ProgramMemoryCheckerImpl::step() and getLastCompletedHash() const;
   - iOS 10-29 also has the static
     RBX::ProgramMemoryChecker::areMemoryPagePermissionsSetupForHacking().

   Dropped as 2016-only (no 2013 symbol, string or vftable slot): the Hasher
   section enums and masks, ScanRegion/ScanRegionTest, NtApiCaller and its
   HeapValue members, the concrete checker's data and its hsce/lua-lock/
   golden-hash members, maskAddr/goldHash/protectVmpSections and the
   Security::rbx* section constants.

   Kept without 2013 evidence: PmcHashContainer and pmcHash, which
   script/LuaVM.cpp defines for the Lua fork's unlock hook. */
#pragma once

#include <vector>

namespace RBX {

// Last reported hashes and the nonce they were taken under.
struct PmcHashContainer
{
	typedef std::vector<unsigned int> HashVector;

	unsigned int nonce;
	HashVector hash;

	PmcHashContainer() : nonce(0) {}
	PmcHashContainer(const PmcHashContainer& other);
};

extern PmcHashContainer pmcHash;

class ProgramMemoryChecker
{
public:
	virtual ~ProgramMemoryChecker() {}                       // vtable+0x00
	virtual unsigned int step() = 0;                         // vtable+0x04
	virtual unsigned int getLastCompletedHash() const = 0;   // vtable+0x08

	// Detects pages made non-executable so a fault handler can serve
	// patched code without disturbing the hash.
	static bool areMemoryPagePermissionsSetupForHacking();
};

} // namespace RBX
