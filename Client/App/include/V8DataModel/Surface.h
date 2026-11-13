/* V8DataModel/Surface.h -- October 2013, minimal: only what PartInstance::getSurface needs.
   Shape from the 2016 header (partInstance, surfId); 2013 evidence: PartInstance::getSurface
   0x0071c220 returns it through the hidden pointer and builds it with the out-of-line
   Surface(PartInstance*, NormalId) at 0x009b1a20 (folded by /OPT:ICF with RakNet's
   RangeNode<uint24_t> ctor: two dword stores, `ret 8`). No 2013 __FILE__ names Surface.h, so
   the path follows the 2016 tree. */

#pragma once

#include "Util/NormalId.h"

namespace RBX {

	class PartInstance;

	class Surface
	{
	private:
		PartInstance* partInstance;  // 0x00
		NormalId surfId;             // 0x04

	public:
		Surface(PartInstance* partInstance, NormalId surfId);
	};

} // namespace RBX
