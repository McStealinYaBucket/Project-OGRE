/* V8DataModel/StarterGuiService.h — October 2013 STUB.
   Evidence: canWriteChild 0x706B22 isA-check (Described<StarterGuiService,
     NonFactoryProduct<BasePlayerGui>, Functionality 27=PERSISTENT>); string in PEs.
   TRUE 2013 bases (NonFactoryProduct<BasePlayerGui>) noted — stub uses Instance
   directly until BasePlayerGui/Hopper are reconstructed (isA-via-cast works for
   exact types in canWriteChild).
   TODO: full decompile. */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sStarterGuiService;

	class StarterGuiService
		: public DescribedNonCreatable<StarterGuiService, Instance, sStarterGuiService, Reflection::ClassDescriptor::PERSISTENT>
		, public Service
	{
	public:
		StarterGuiService() {}
	};

} // namespace RBX
