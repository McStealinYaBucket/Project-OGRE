/* V8DataModel/CoreGuiService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<CoreGuiService> from DataModel::initializeContents
     0x2D3336 (stored at +2920; createRobloxScreenGui called on it in initializeContents.); "CoreGuiService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CoreGuiService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "V8DataModel/ScreenGui.h"
#include "V8DataModel/BasePlayerGui.h"

namespace RBX {

	extern const char* const sCoreGuiService;

class CoreGuiService
		: public DescribedNonCreatable<CoreGuiService, BasePlayerGui, sCoreGuiService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		// Restored TRUE parent (was Instance standing in; binary:
		// NonFactoryProduct<CoreGuiService, BasePlayerGui> 0x460834).
		, public Service
	{
	public:
		CoreGuiService() {}

		// mac: 0x45E19E (member, no args — was misdeclared static with arg).
		void createRobloxScreenGui();

		// mac: 0x45E2EA (44B): parents the child under the +116 roblox-Gui slot
		// (the shared_ptr's object), NULL-child no-op.  Exact edge: with a
		// NULL slot this unparents (setParent(NULL) semantics) -- carried
		// by calling setParent unconditionally like the binary.
		void addChild(Instance* child)
		{
			if (child != NULL)
				child->setParent(robloxGui.get());
		}

	private:
		// +116 (shared_ptr slot; TODO verify).
		shared_ptr<ScreenGui> robloxGui;
	};

} // namespace RBX
