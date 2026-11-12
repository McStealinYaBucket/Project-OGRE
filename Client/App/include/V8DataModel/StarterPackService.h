/* V8DataModel/StarterPackService.h — October 2013 STUB.
   Evidence: ServiceProvider::create<StarterPackService> stored at +2952 in
     DataModel::initializeContents 0x2D3336; canWriteChild 0x706B22 isA-check
     (Described<StarterPackService, NonFactoryProduct<Hopper>, Functionality
     27=PERSISTENT>); string in PEs.
   TRUE 2013 bases noted — stub uses Instance directly until Hopper is reconstructed.
   TODO: full decompile. */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "V8DataModel/Hopper.h"

namespace RBX {

	extern const char* const sStarterPackService;

	class StarterPackService
		: public DescribedNonCreatable<StarterPackService, Hopper, sStarterPackService, Reflection::ClassDescriptor::PERSISTENT>
		// Restored TRUE parent (was Instance standing in; binary:
		// NonFactoryProduct<StarterPackService, Hopper>).
		, public Service
	{
	public:
		StarterPackService() {}
	};

} // namespace RBX
