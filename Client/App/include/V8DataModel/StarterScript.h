/* V8DataModel/StarterScript.h -- October 2013 (starter core script).
   Recon evidence (batches 730-771, MAC dbq24):
   - Class shape: Described<StarterScript, sStarterScript, NonFactoryProduct<CoreScript,
     sStarterScript>, ClassDescriptor::Functionality 1 (= INTERNAL_LOCAL),
     Security::Permissions 0> (batch768:182-231, StarterScript ctor 0x16075A).
     StarterScript : CoreScript (the NonFactoryProduct parent is CoreScript).
   - StarterScript ctor 0x16075A: the Described chain forwards the initial ContentId
     into CoreScript::CoreScript(contentId) (so scriptId = the starter-script asset;
     inherits the CoreScript missing/reported/setName("CoreScript") block), then
     overrides the name: setName("StarterScript").
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/StarterScript.h */

#pragma once

#include "V8DataModel/CoreScript.h"

namespace RBX {

	extern const char* const sStarterScript;

	class StarterScript
		: public DescribedNonCreatable<StarterScript, CoreScript, sStarterScript, Reflection::ClassDescriptor::INTERNAL_LOCAL>
	{
	public:
		StarterScript(const ContentId& contentId);
		/*override*/ virtual ~StarterScript();
	};

} // namespace RBX
