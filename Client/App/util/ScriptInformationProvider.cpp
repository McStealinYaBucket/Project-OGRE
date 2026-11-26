/* App/v8datamodel/ScriptInformationProvider.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/ScriptInformationProvider.h"

namespace RBX {
	const char* const sScriptInformationProvider = "ScriptInformationProvider";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ScriptInformationProvider);
}

#include "decomp.h"

namespace RBX {

	// Not written yet (assigns the string member at +0xac). STUB body so callers pair.
	// STUB: PLAYER 0x00652d70
	// STUB: STUDIO 0x007cb930
	void ScriptInformationProvider::setAssetUrl(std::string url)
	{
		STUB(0x00652d70);
	}

} // namespace RBX
