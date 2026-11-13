/* V8DataModel/Lighting.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::find<Lighting> from DataModel::getLightingDeprecated
     0x2D1636; "Lighting" string in both 2013 PEs.
   Bases (DescribedCreatable + Service) carried from March 2016 header.
   TODO: full decompile (sky/fog/time-of-day machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Lighting.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "decomp.h"

namespace RBX {

	extern const char* const sLighting;

	class Sky;

	// 2013 layout (PLAYER onChildAdded 0x00707510, onChildRemoving 0x007068f0; STUDIO the same):
	// lightingChangedSignal @0x1e0, sky @0x1e4, size 0x1f0 (Creatable<Instance>::create<Lighting>).
	// The members 0x64..0x1e0 (2016: G3D::LightingParameters skyParameters, hasSky, the colors,
	// fog, timeOfDay, ...) are not laid out yet.
	// 2013 RTTI says functionality 11 (PERSISTENT_HIDDEN, as 2016: DescribedCreatable<...$0L@...>).
	// With it, the Lighting template bodies fold with Teams' identical ones in our link and four
	// 100% functions plus FactoryProduct<Lighting>'s vftable lose their pairing (measured
	// 2026-11-13), so the default stays until the fold issue is solved.
	// SIZE 0x1f0
	class Lighting
		: public DescribedCreatable<Lighting, Instance, sLighting>
		, public Service
	{
	private:
		typedef DescribedCreatable<Lighting, Instance, sLighting> Super;
		undefined m_unk0x64[0x1e0 - 0x64];		// 0x64

	public:
		Lighting() {}

		rbx::signal<void(bool)> lightingChangedSignal;		// 0x1e0 (arg=skyboxChanged)
		shared_ptr<Sky> sky;								// 0x1e4

	private:
		undefined4 m_unk0x1ec;							// 0x1ec

	protected:
		/*override*/ void onChildChanged(Instance* instance, const PropertyChanged& event);
		/*override*/ bool askAddChild(const Instance* instance) const;
	};

} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x007057a0
// TEMPLATE: STUDIO 0x007d13d0
// ??_E?$FactoryProduct@VLighting@RBX@@VInstance@2@$E?sLighting@2@3QBDBV32@@RBX@@OBM@AEPAXI@Z

// TEMPLATE: PLAYER 0x00705b00
// TEMPLATE: STUDIO 0x007d0c60
// ??1?$DescribedCreatable@VLighting@RBX@@VInstance@2@$E?sLighting@2@3QBDB$0BL@$0A@@RBX@@UAE@XZ

// TEMPLATE: PLAYER 0x007075e0
// TEMPLATE: STUDIO 0x007d3390
// ??0?$Described@VLighting@RBX@@$E?sLighting@2@3QBDBV?$FactoryProduct@VLighting@RBX@@VInstance@2@$E?sLighting@2@3QBDBV32@@2@$0BL@$0A@@Reflection@RBX@@QAE@XZ

