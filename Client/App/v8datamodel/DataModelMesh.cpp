/* App/v8datamodel/DataModelMesh.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/DataModelMesh.h"

namespace RBX {
	const char* const sDataModelMesh = "DataModelMesh";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::DataModelMesh);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x0075c1c0
// FUNCTION: STUDIO 0x0089ac10
template<>
EnumDesc<DataModelMesh::LODType>::EnumDesc()
	: EnumDescriptor("LevelOfDetailSetting", typeid(DataModelMesh::LODType))
{
	addPair(DataModelMesh::HIGH_LOD, "High");
	addPair(DataModelMesh::MEDIUM_LOD, "Medium");
	addPair(DataModelMesh::LOW_LOD, "Low");
}

template<>
const Type& Type::getSingleton<DataModelMesh::LODType>()
{
	return EnumDesc<DataModelMesh::LODType>::singleton();
}

template<> EnumRegistrar<DataModelMesh::LODType> EnumRegistrar<DataModelMesh::LODType>::registrar(0);
template<> TypeRegistrar<DataModelMesh::LODType> TypeRegistrar<DataModelMesh::LODType>::registrar(0);

} // namespace Reflection
} // namespace RBX
