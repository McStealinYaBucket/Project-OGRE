/* App/v8datamodel/ModelInstance.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/ModelInstance.h"
#include "decomp.h"

namespace RBX {
	const char* const sModel = "Model";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

namespace RBX {
	// STUB: PLAYER 0x006d2060
	// STUB: STUDIO 0x00817720
	void ModelInstance::legacyTraverseState(const G3D::CoordinateFrame& cf)
	{
		STUB(0x006d2060);
	}

	// STUB: PLAYER 0x006d2120
	// STUB: STUDIO 0x008177e0
	bool ModelInstance::hitTestImpl(const RbxRay& worldRay, G3D::Vector3& worldHitPoint)
	{
		STUB(0x006d2120);
		return false;
	}

	// STUB: PLAYER 0x006d3bc0
	// STUB: STUDIO 0x00818e40
	Extents ModelInstance::computeExtentsWorld() const
	{
		STUB(0x006d3bc0);
		return Extents();
	}
}

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ModelInstance);
}
