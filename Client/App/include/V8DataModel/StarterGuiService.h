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
		enum CoreGuiType
		{
			COREGUI_PLAYERLISTGUI = 0,
			COREGUI_HEALTHGUI = 1,
			COREGUI_BACKPACKGUI = 2,
			COREGUI_CHATGUI = 3,
			COREGUI_ALL = 4,
		};

		StarterGuiService() {}
	};

} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x0070a2c0
// TEMPLATE: STUDIO 0x007969e0
// ??0?$Described@VBasePlayerGui@RBX@@$E?sBasePlayerGui@2@3QBDBV?$NonFactoryProduct@VInstance@RBX@@$E?sBasePlayerGui@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@XZ

