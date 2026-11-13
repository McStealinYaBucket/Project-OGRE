/* App/v8world/MacroTypes.cpp -- October 2013.
   EnumDesc<SurfaceType> ctor (PLAYER 0x00952a20). Its addPair call goes to the "Reflection/EnumConverter.h"
   copy (PLAYER 0x008b5df0), so this TU spells the include that way. No 2013 assert names the
   file; the path is the 2016 tree's, which holds the same ctor. */

#include "stdafx.h"

#include "Util/SurfaceType.h"


// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "Reflection/EnumConverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x00952a20
// FUNCTION: STUDIO 0x00981be0
template<>
EnumDesc<SurfaceType>::EnumDesc()
	: EnumDescriptor("SurfaceType", typeid(SurfaceType))
{
	addPair(NO_SURFACE, "Smooth");
	addPair(GLUE, "Glue");
	addPair(WELD, "Weld");
	addPair(STUDS, "Studs");
	addPair(INLET, "Inlet");
	addPair(UNIVERSAL, "Universal");
	addLegacyName("Spawn", NO_SURFACE);
	addPair(ROTATE, "Hinge");
	addPair(ROTATE_V, "Motor");
	addPair(ROTATE_P, "SteppingMotor");
	addPair(NO_JOIN, "Unjoinable");
	addPair(NO_SURFACE_NO_OUTLINES, "SmoothNoOutlines");
	addLegacyName("Bumps", GLUE);
}

template<>
const Type& Type::getSingleton<SurfaceType>()
{
	return EnumDesc<SurfaceType>::singleton();
}

template<> EnumRegistrar<SurfaceType> EnumRegistrar<SurfaceType>::registrar(0);
template<> TypeRegistrar<SurfaceType> TypeRegistrar<SurfaceType>::registrar(0);

} // namespace Reflection
} // namespace RBX
