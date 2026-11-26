/* WindowsClient/Teleporter.cpp -- October 2013 client shell (PLAYER only; path from the 2016 tree, see
   Teleporter.h). Only the vftable's out-of-line slot, still a STUB. */

#include "Teleporter.h"

#include "decomp.h"

DECOMP_SIZE_ASSERT(RBX::Teleporter, 0xc);

namespace RBX {

	// STUB: PLAYER 0x0045d450
	__declspec(noinline) void Teleporter::doTeleport(const std::string& url, const std::string& ticket,
		const std::string& script)
	{
		STUB(0x0045d450);
	}

} // namespace RBX
