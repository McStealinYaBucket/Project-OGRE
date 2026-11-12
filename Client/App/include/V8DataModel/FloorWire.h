/* V8DataModel/FloorWire.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<FloorWire, GuiBase3d> 0x366C1E;
     sFloorWire = "FloorWire".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/FloorWire.h */

#pragma once

#include "V8DataModel/GuiBase3d.h"

namespace RBX {

	extern const char* const sFloorWire;

	class FloorWire
		: public DescribedCreatable<FloorWire, GuiBase3d, sFloorWire>
	{
	public:
		FloorWire() {}
	};

} // namespace RBX
