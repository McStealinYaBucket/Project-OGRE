/* App/v8datamodel/CharacterMesh.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/CharacterMesh.h"

namespace RBX {
	const char* const sCharacterMesh = "CharacterMesh";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::CharacterMesh);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x0076d660
// FUNCTION: STUDIO 0x008a23e0
template<>
EnumDesc<CharacterMesh::BodyPart>::EnumDesc()
	: EnumDescriptor("BodyPart", typeid(CharacterMesh::BodyPart))
{
	addPair(CharacterMesh::HEAD, "Head");
	addPair(CharacterMesh::TORSO, "Torso");
	addPair(CharacterMesh::LEFTARM, "LeftArm");
	addPair(CharacterMesh::RIGHTARM, "RightArm");
	addPair(CharacterMesh::LEFTLEG, "LeftLeg");
	addPair(CharacterMesh::RIGHTLEG, "RightLeg");
}

template<>
const Type& Type::getSingleton<CharacterMesh::BodyPart>()
{
	return EnumDesc<CharacterMesh::BodyPart>::singleton();
}

template<> EnumRegistrar<CharacterMesh::BodyPart> EnumRegistrar<CharacterMesh::BodyPart>::registrar(0);
template<> TypeRegistrar<CharacterMesh::BodyPart> TypeRegistrar<CharacterMesh::BodyPart>::registrar(0);

} // namespace Reflection
} // namespace RBX
