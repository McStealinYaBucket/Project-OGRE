/* V8DataModel/LocalScript.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<LocalScript, Script> 0x1873C8;
     sLocalScript = "LocalScript" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/LocalScript.h */

#pragma once

#include "V8DataModel/Script.h"

namespace RBX {

	extern const char* const sLocalScript;

	class LocalScript
		: public DescribedCreatable<LocalScript, Script, sLocalScript>
	{
	public:
		LocalScript() {}
	};

} // namespace RBX
