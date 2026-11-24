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

	// Layout from the PLAYER ctor 0x0070c350: BasePlayerGui (0x74), Service::isPublic at 0x74 (RTTI mdisp
	// 116), the RobloxGui ScreenGui at 0x78 (createRobloxScreenGui 0x00708d80), and a vector of
	// shared_ptrs at 0x80 that the ctor resizes to 3 (2016: onScreenMessages, MAX_ON_SCREEN_MESSAGES).
	// RTTI: DescribedNonCreatable<CoreGuiService, BasePlayerGui, sCoreGuiService, 1 = INTERNAL_LOCAL, 2 = RobloxPlace>.
	// VTABLE: PLAYER 0x00b485fc RBX::Debugable
	// VTABLE: STUDIO 0x00bfd8ec RBX::Debugable
	// SIZE 0x90
	class CoreGuiService
		: public DescribedNonCreatable<CoreGuiService, BasePlayerGui, sCoreGuiService, Reflection::ClassDescriptor::INTERNAL_LOCAL, Security::RobloxPlace>
		, public Service
	{
	public:
		CoreGuiService();

		// mac: 0x45E19E (member, no args).
		void createRobloxScreenGui();

		// mac: 0x45E2EA (44B): parents the child under the RobloxGui ScreenGui; NULL-child no-op.
		// FUNCTION: PLAYER 0x00708530
		// FUNCTION: STUDIO 0x00795310
		void addChild(Instance* child)
		{
			if (child != NULL)
				child->setParent(robloxGui.get());
		}

		/*override*/ void onDescendantAdded(Instance* instance);
		/*override*/ bool canClientCreate() { return false; }	// vtable+0x1c: the engine `xor al,al; ret` copy 0x00927cc0
		/*override*/ bool scriptShouldRun(BaseScript* script) { return false; }	// IScriptFilter slot: 0x0070c430

	private:
		shared_ptr<ScreenGui> robloxGui;					// 0x78
		std::vector<shared_ptr<Instance> > onScreenMessages;	// 0x80
	};

} // namespace RBX

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// One body for every Described/NonFactoryProduct dtor over BasePlayerGui (CoreGuiService's and
// StarterGuiService's fold); the nameref is the PUBLICS name our PDB keeps for it.
// TEMPLATE: PLAYER 0x00708b60
// TEMPLATE: STUDIO 0x007955e0
// ??1?$NonFactoryProduct@VBasePlayerGui@RBX@@$E?sStarterGuiService@2@3QBDB@RBX@@UAE@XZ
// clang-format on
