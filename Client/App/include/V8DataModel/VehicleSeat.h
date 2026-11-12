/* V8DataModel/VehicleSeat.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED parent (true FactoryProduct<VehicleSeat, SeatImpl<PartInstance>>); SeatImpl TODO;
     sVehicleSeat = "VehicleSeat" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/VehicleSeat.h */

#pragma once

#include "V8DataModel/BasicPartInstance.h"

namespace RBX {

	extern const char* const sVehicleSeat;

	class VehicleSeat
		: public DescribedCreatable<VehicleSeat, BasicPartInstance, sVehicleSeat>
	{
	public:
		VehicleSeat() {}
	};

} // namespace RBX
