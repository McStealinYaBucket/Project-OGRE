/* App/v8datamodel/legacy.cpp -- October 2013.
   EnumDesc<Legacy::SurfaceConstraint> ctor (PLAYER 0x009528f0). Its addPair call goes to the "Reflection/EnumConverter.h"
   copy (PLAYER 0x008b5df0), so this TU spells the include that way. No 2013 assert names the
   file; the path is the 2016 tree's, which holds the same ctor. */

#include "stdafx.h"

#include "Util/SurfaceType.h"


// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "Reflection/EnumConverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x009528f0
// FUNCTION: STUDIO 0x00981400
template<>
EnumDesc<Legacy::SurfaceConstraint>::EnumDesc()
	: EnumDescriptor("SurfaceConstraint", typeid(Legacy::SurfaceConstraint))
{
	addPair(Legacy::NO_CONSTRAINT, "None");
	addPair(Legacy::ROTATE_LEGACY, "Hinge");
	addPair(Legacy::ROTATE_P_LEGACY, "SteppingMotor");
	addPair(Legacy::ROTATE_V_LEGACY, "Motor");
}

template<>
const Type& Type::getSingleton<Legacy::SurfaceConstraint>()
{
	return EnumDesc<Legacy::SurfaceConstraint>::singleton();
}

template<> EnumRegistrar<Legacy::SurfaceConstraint> EnumRegistrar<Legacy::SurfaceConstraint>::registrar(0);
template<> TypeRegistrar<Legacy::SurfaceConstraint> TypeRegistrar<Legacy::SurfaceConstraint>::registrar(0);

} // namespace Reflection
} // namespace RBX
