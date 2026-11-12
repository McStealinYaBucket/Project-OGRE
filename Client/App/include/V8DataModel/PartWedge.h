/* V8DataModel/PartWedge.h — October 2013 STUB (PART::Wedge shell).
   Evidence: Described<PART::Wedge, PART::sWedge, FactoryProduct<PART::Wedge,
     PART::ParametricPartInstance, PART::sWedge, Instance>>::classDescriptor
     0x4414F4 (display literal "WedgePart"); PART::Wedge C1 0x44051A;
     FactoryProduct Creator 0x440B20/0x440BD8/0x440C68.
     Descriptor parent is FormFactorPart (binary passes its descriptor) — the
     C++ ParametricPartInstance intermediate is TODO (not in tree).
   TODO: full decompile (param geometry surface + ParametricPartInstance).
   Original file: UNKNOWN (RBX::PART namespace; 2013 path TODO). */

#pragma once

#include "V8DataModel/FormFactorPart.h"

namespace RBX {
namespace PART {

	extern const char* const sWedge;

	class Wedge
		: public DescribedCreatable<Wedge, ::RBX::FormFactorPart, sWedge>
	{
	public:
		// STUB: PLAYER 0x009302b0
		// STUB: STUDIO 0x00958710
		Wedge() {}
	};

} // namespace PART
} // namespace RBX
