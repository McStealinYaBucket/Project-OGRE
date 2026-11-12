/* App/script/StarterScript.cpp -- October 2013 (full C1 port).
   Recon evidence (batches 730-771, MAC dbq24): StarterScript ctor 0x16075A
   (batch768:182-231) = the Described chain forwards the ContentId into
   CoreScript::CoreScript(contentId), then overrides the name setName("StarterScript").
   Inherits CoreScript's onServiceProvider / requestCode / extraErrorReporting. */

#include "stdafx.h"

#include "V8DataModel/StarterScript.h"

namespace RBX {
	const char* const sStarterScript = "StarterScript";
}

using namespace RBX;

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::StarterScript);
}

namespace RBX {

	// FUNCTION: PLAYER 0x008aa850
	// FUNCTION: STUDIO 0x00992f00
	StarterScript::StarterScript(const ContentId& contentId)
		: DescribedNonCreatable<StarterScript, CoreScript, sStarterScript, Reflection::ClassDescriptor::INTERNAL_LOCAL>(contentId)
	{
		setName("StarterScript");
	}

	StarterScript::~StarterScript()
	{
	}

} // namespace RBX
