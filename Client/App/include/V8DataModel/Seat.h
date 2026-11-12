/* V8DataModel/Seat.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED parent (true FactoryProduct<Seat, SeatImpl<BasicPartInstance>>); SeatImpl TODO;
     sSeat = "Seat" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Seat.h */

#pragma once

#include "V8DataModel/BasicPartInstance.h"

namespace RBX {

	extern const char* const sSeat;

	class Seat
		: public DescribedCreatable<Seat, BasicPartInstance, sSeat>
	{
	public:
		Seat() {}
	};

} // namespace RBX
