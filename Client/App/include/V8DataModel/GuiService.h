/* V8DataModel/GuiService.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<GuiService> from DataModel::initializeContents
     0x2D3336; "GuiService" string in both 2013 PEs.
   Bases (DescribedNonCreatable + Service) carried from March 2016 header.
   TODO: full decompile (selection/error-message machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sGuiService;

	class GuiService
		: public DescribedNonCreatable<GuiService, Instance, sGuiService, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	public:
		GuiService() {}
	};

} // namespace RBX
