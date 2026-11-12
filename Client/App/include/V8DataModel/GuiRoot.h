/* V8DataModel/GuiRoot.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: Creatable<Instance>::create<GuiRoot> TWICE (+2968/+2976) in the DataModel
     ctor; GuiRoot instances parented via setAndLockParent in initializeContents 0x2D3336.
   Service base inferred (setAndLockParent'ed to the DataModel ServiceProvider).
   NOTE: TWO roots are created (roles TODO — possibly main + thumbnail/core roots).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiRoot.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sGuiRoot;

	class GuiRoot
		: public DescribedCreatable<GuiRoot, Instance, sGuiRoot>
		, public Service
	{
	public:
		GuiRoot() {}
	};

} // namespace RBX
