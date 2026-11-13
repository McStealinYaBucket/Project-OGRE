/* v8datamodel/Surface.cpp -- October 2013, minimal (see V8DataModel/Surface.h).
   The ctor's PLAYER body 0x009b1a20 is shared through /OPT:ICF with RakNet's
   RangeNode<uint24_t> ctor, so it carries no marker of its own here. */

#include "stdafx.h"

#include "V8DataModel/Surface.h"

namespace RBX {

	Surface::Surface(PartInstance* partInstance, NormalId surfId)
		: partInstance(partInstance)
		, surfId(surfId)
	{
	}

} // namespace RBX
