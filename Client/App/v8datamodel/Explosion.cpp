/* App/v8datamodel/Explosion.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/Explosion.h"

namespace RBX {
	const char* const sExplosion = "Explosion";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Explosion);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x0091aa00
// FUNCTION: STUDIO 0x007f4170
template<>
EnumDesc<Explosion::ExplosionType>::EnumDesc()
	: EnumDescriptor("ExplosionType", typeid(Explosion::ExplosionType))
{
	addPair(Explosion::NO_EFFECT, "NoCraters");
	addPair(Explosion::NO_DEBRIS, "Craters");
	addPair(Explosion::WITH_DEBRIS, "CratersAndDebris");
}

template<>
const Type& Type::getSingleton<Explosion::ExplosionType>()
{
	return EnumDesc<Explosion::ExplosionType>::singleton();
}

template<> EnumRegistrar<Explosion::ExplosionType> EnumRegistrar<Explosion::ExplosionType>::registrar(0);
template<> TypeRegistrar<Explosion::ExplosionType> TypeRegistrar<Explosion::ExplosionType>::registrar(0);

} // namespace Reflection
} // namespace RBX
