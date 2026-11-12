/* V8DataModel/ForceField.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ForceField, Instance>;
     sForceField = "ForceField" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ForceField.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sForceField;

	class ForceField
		: public DescribedCreatable<ForceField, Instance, sForceField>
	{
	public:
		ForceField() {}
	};

} // namespace RBX
