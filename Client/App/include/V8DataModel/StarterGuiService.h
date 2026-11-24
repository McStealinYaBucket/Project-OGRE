/* V8DataModel/StarterGuiService.h -- October 2013.
   Layout from the PLAYER ctor 0x0070c150 (`new 0x98` at 0x0058ea3f): Described<StarterGuiService,
   sStarterGuiService, NonFactoryProduct<BasePlayerGui, sStarterGuiService>, 27> (ctor 0x0070bf50, RTTI),
   Service at 0x74 (ctor stores 1), the boost::unordered_map at 0x78 (inline ctor: prime-list
   lower_bound(11), max_load_factor 1.0), the signal at 0x90 (ctor 0x00708a90) and two bools at
   0x94/0x95 that the ctor sets to true (iOS setShowGui / setResetPlayerGui). Member names are 2016's.
   getCoreGuiEnabled 0x007095e0 and setCoreGuiEnabled 0x00709c60 index the map at 0x78 and fire the
   signal at 0x90. In 2013 the class sits in PlayerGui.obj (0x007083d0-0x0070c450), so its code is
   in v8datamodel/PlayerGui.cpp. */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "V8DataModel/BasePlayerGui.h"
#include "rbx/signal.h"
#include <boost/unordered_map.hpp>

namespace RBX {

	extern const char* const sStarterGuiService;

	// SIZE 0x98
	class StarterGuiService
		: public DescribedNonCreatable<StarterGuiService, BasePlayerGui, sStarterGuiService, Reflection::ClassDescriptor::PERSISTENT>
		, public Service	// 0x74
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

		StarterGuiService();

		void setCoreGuiEnabled(CoreGuiType type, bool enabled);
		bool getCoreGuiEnabled(CoreGuiType type);

	private:
		boost::unordered_map<CoreGuiType, bool> coreGuiEnabledState;	// 0x78

	public:
		rbx::signal<void(CoreGuiType, bool)> coreGuiChangedSignal;		// 0x90

	private:
		bool showGui;			// 0x94
		bool resetPlayerGui;	// 0x95
	};

} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x0070a2c0
// TEMPLATE: STUDIO 0x007969e0
// ??0?$Described@VBasePlayerGui@RBX@@$E?sBasePlayerGui@2@3QBDBV?$NonFactoryProduct@VInstance@RBX@@$E?sBasePlayerGui@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@XZ
