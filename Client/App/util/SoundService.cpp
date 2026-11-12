/* App/util/SoundService.cpp — October 2013 STUB (sName + empty reflection).
   NOTE: see the header note about the preserved 2016 original.
   TODO: full decompile. */

#include "stdafx.h"

#include "Util/SoundService.h"
#include "decomp.h"

namespace RBX {
namespace Soundscape {
	const char* const sSoundService = "SoundService";

	// STUB: PLAYER 0x006f4600
	// STUB: STUDIO 0x00809650
	void SoundService::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		STUB(0x006f4600);
	}
}
}

using namespace RBX;
using namespace RBX::Soundscape;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Soundscape::SoundService);
}
