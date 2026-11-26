/* V8DataModel/Selection.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<Selection, Instance> 0x368E7E; abstract (no creator by design);
     sSelection = "Selection".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Selection.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "decomp.h"

namespace RBX {

	extern const char* const sSelection;

	// A Service in 2013: the ctor stores Service::isPublic (+0x60) = 1, and GlobalAdvancedSettings
	// creates it through ServiceProvider::create<Selection> (PLAYER 0x00656f10). Size 0x90
	// (Creatable::create<Selection> 0x00656d40).
	// SIZE 0x90
	class Selection
		: public DescribedNonCreatable<Selection, Instance, sSelection>
		, public Service
	{
	public:
		Selection();
		~Selection();

		void removeFromSelection(Instance* instance);

		// vtable+0x70 (Debugable group): Selection's first own virtual.
		virtual void onAncestryChanged(Instance* instance);

	private:
		undefined m_unk0x64[0x90 - 0x64];	// 0x64  ctor 0x0086b070: containers, a signal, a connection
	};

} // namespace RBX
