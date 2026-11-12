/* V8DataModel/DialogChoice.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<DialogChoice, Instance> 0x32F778 (Dialog itself has no symbols — deferred);
     sDialogChoice = "DialogChoice".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DialogChoice.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sDialogChoice;

	class DialogChoice
		: public DescribedCreatable<DialogChoice, Instance, sDialogChoice>
	{
	public:
		DialogChoice() {}
	};

} // namespace RBX
