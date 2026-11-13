/* App/util/SoundService.cpp — October 2013 STUB (sName + empty reflection).
   NOTE: see the header note about the preserved 2016 original.
   TODO: full decompile. */

#include "stdafx.h"

#include "Util/SoundService.h"
#include "decomp.h"

namespace RBX {
namespace Soundscape {
	const char* const sSoundService = "SoundService";

	// STUB: PLAYER 0x006f4600
	// STUB: STUDIO 0x00809650
	void SoundService::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		STUB(0x006f4600);
	}
}
}

using namespace RBX;
using namespace RBX::Soundscape;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Soundscape::SoundService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x006f0d20
// FUNCTION: STUDIO 0x00805e70
template<>
EnumDesc<Soundscape::ReverbType>::EnumDesc()
	: EnumDescriptor("ReverbType", typeid(Soundscape::ReverbType))
{
	addPair(Soundscape::NoReverb, "NoReverb");
	addPair(Soundscape::GenericReverb, "GenericReverb");
	addPair(Soundscape::PaddedCell, "PaddedCell");
	addPair(Soundscape::Room, "Room");
	addPair(Soundscape::Bathroom, "Bathroom");
	addPair(Soundscape::LivingRoom, "LivingRoom");
	addPair(Soundscape::StoneRoom, "StoneRoom");
	addPair(Soundscape::Auditorium, "Auditorium");
	addPair(Soundscape::ConcertHall, "ConcertHall");
	addPair(Soundscape::Cave, "Cave");
	addPair(Soundscape::Arena, "Arena");
	addPair(Soundscape::Hangar, "Hangar");
	addPair(Soundscape::CarpettedHallway, "CarpettedHallway");
	addPair(Soundscape::Hallway, "Hallway");
	addPair(Soundscape::StoneCorridor, "StoneCorridor");
	addPair(Soundscape::Alley, "Alley");
	addPair(Soundscape::Forest, "Forest");
	addPair(Soundscape::City, "City");
	addPair(Soundscape::Mountains, "Mountains");
	addPair(Soundscape::Quarry, "Quarry");
	addPair(Soundscape::Plain, "Plain");
	addPair(Soundscape::ParkingLot, "ParkingLot");
	addPair(Soundscape::SewerPipe, "SewerPipe");
	addPair(Soundscape::UnderWater, "UnderWater");
}

template<>
const Type& Type::getSingleton<Soundscape::ReverbType>()
{
	return EnumDesc<Soundscape::ReverbType>::singleton();
}

template<> EnumRegistrar<Soundscape::ReverbType> EnumRegistrar<Soundscape::ReverbType>::registrar(0);
template<> TypeRegistrar<Soundscape::ReverbType> TypeRegistrar<Soundscape::ReverbType>::registrar(0);

} // namespace Reflection
} // namespace RBX
