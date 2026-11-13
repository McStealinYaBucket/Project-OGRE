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
	// Instance vtable+0x50. Body not written yet.
	// STUB: PLAYER 0x006d1bb0
	// STUB: STUDIO 0x00817360
	void ModelInstance::onDescendantAdded(Instance* instance)
	{
		STUB(0x006d1bb0);
	}

	// CameraSubject vtable+0x18. Body not written yet.
	// STUB: PLAYER 0x006d1be0
	// STUB: STUDIO 0x008173b0
	void ModelInstance::getCameraIgnorePrimitives(std::vector<const Primitive*>& primitives)
	{
		STUB(0x006d1be0);
	}

	// Instance vtable+0x18. Body not written yet.
	// STUB: PLAYER 0x006d1c50
	// STUB: STUDIO 0x00787ba0
	int ModelInstance::getPersistentDataCost() const
	{
		STUB(0x006d1c50);
		return 0;
	}

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

	// IAdornable vtable+0x04. Body not written yet.
	// STUB: PLAYER 0x006d21c0
	// STUB: STUDIO 0x00817880
	bool ModelInstance::shouldRender3dAdorn() const
	{
		STUB(0x006d21c0);
		return false;
	}

	// IAdornable vtable+0x20. Body not written yet.
	// STUB: PLAYER 0x006d2210
	// STUB: STUDIO 0x008178d0
	void ModelInstance::render3dAdorn(RBX::Adorn* adorn)
	{
		STUB(0x006d2210);
	}

	// CameraSubject vtable+0x14. Body not written yet.
	// STUB: PLAYER 0x006d2260
	// STUB: STUDIO 0x00817920
	void ModelInstance::onCameraNear(float distance)
	{
		STUB(0x006d2260);
	}

	// Instance vtable+0x54. Body not written yet.
	// STUB: PLAYER 0x006d2490
	// STUB: STUDIO 0x00817af0
	void ModelInstance::onDescendantRemoving(const shared_ptr<Instance>& instance)
	{
		STUB(0x006d2490);
	}

	// IHasLocation vtable+0x00 (vtordisp thunk 0x006d2960 / STUDIO 0x00817ef0). Body not written yet.
	// STUB: PLAYER 0x006d2580
	// STUB: STUDIO 0x00817be0
	G3D::CoordinateFrame ModelInstance::getLocation()
	{
		STUB(0x006d2580);
		return G3D::CoordinateFrame();
	}

	// CameraSubject vtable+0x0c: a virtual call to getLocation through the vbase.
	// STUB: PLAYER 0x006d28d0
	// STUB: STUDIO 0x00787bb0
	G3D::CoordinateFrame ModelInstance::getRenderLocation()
	{
		return getLocation();
	}

	// Instance vtable+0x14. Body not written yet.
	// STUB: PLAYER 0x006d2c60
	// STUB: STUDIO 0x008181c0
	void ModelInstance::setName(const std::string& value)
	{
		STUB(0x006d2c60);
	}

	// Instance vtable+0x58. Body not written yet.
	// STUB: PLAYER 0x006d3640
	// STUB: STUDIO 0x008188c0
	void ModelInstance::onChildAdded(Instance* child)
	{
		STUB(0x006d3640);
	}

	// Instance vtable+0x5c. Body not written yet.
	// STUB: PLAYER 0x006d36d0
	// STUB: STUDIO 0x00818950
	void ModelInstance::onChildRemoving(Instance* child)
	{
		STUB(0x006d36d0);
	}

	// Instance vtable+0x64. Body not written yet.
	// STUB: PLAYER 0x006d3790
	// STUB: STUDIO 0x00818a10
	void ModelInstance::onChildChanged(Instance* instance, const PropertyChanged& event)
	{
		STUB(0x006d3790);
	}

	// STUB: PLAYER 0x006d3bc0
	// STUB: STUDIO 0x00818e40
	Extents ModelInstance::computeExtentsWorld() const
	{
		STUB(0x006d3bc0);
		return Extents();
	}

	// IAdornable vtable+0x28. Body not written yet.
	// STUB: PLAYER 0x006d44c0
	// STUB: STUDIO 0x00819740
	void ModelInstance::render3dSelect(RBX::Adorn* adorn, SelectState selectState)
	{
		STUB(0x006d44c0);
	}

	// CameraSubject vtable+0x10 (calls 0x006d3fd0 on the ModelInstance). Body not written yet.
	// STUB: PLAYER 0x006d4620
	// STUB: STUDIO 0x00787bd0
	G3D::Vector3 ModelInstance::getRenderSize()
	{
		STUB(0x006d4620);
		return G3D::Vector3();
	}

	// IHasPrimaryPart vtable+0x00. Body not written yet.
	// STUB: PLAYER 0x006d48b0
	// STUB: STUDIO 0x00819b40
	PartInstance* ModelInstance::getPrimaryPart()
	{
		STUB(0x006d48b0);
		return NULL;
	}

	// Instance vtable+0x48: the same body as PartInstance::askSetParent, which /OPT:ICF folds with
	// it at PLAYER 0x006de360 (STUDIO 0x007c08d0), so the marker lives on PartInstance's copy.
	bool ModelInstance::askSetParent(const Instance* parent) const
	{
		if (parent == NULL)
			return false;
		return Instance::fastDynamicCast<ModelInstance>(parent) != NULL;
	}

}

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ModelInstance);
}
