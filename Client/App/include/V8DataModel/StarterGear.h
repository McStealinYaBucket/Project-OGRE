/* V8DataModel/StarterGear.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<StarterGear, Instance> 0x3EEF6C;
     sStarterGear = "StarterGear".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/StarterGear.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sStarterGear;

	class StarterGear
		: public DescribedCreatable<StarterGear, Instance, sStarterGear>
	{
	public:
		StarterGear();
	};

} // namespace RBX

// TEMPLATE: PLAYER 0x006ca3f0
// TEMPLATE: STUDIO 0x007b25e0
// ??0?$Described@VStarterGear@RBX@@$E?sStarterGear@2@3QBDBV?$FactoryProduct@VStarterGear@RBX@@VInstance@2@$E?sStarterGear@2@3QBDBV32@@2@$0BL@$0A@@Reflection@RBX@@QAE@XZ
