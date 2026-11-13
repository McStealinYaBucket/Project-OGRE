/* V8DataModel/ModelInstance.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ModelInstance, PVInstance, sModel> 0x437C70;
     sModelInstance = "Model" (0.125 metadata class list; C++ name differs from Lua).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ModelInstance.h */

#pragma once

#include "V8DataModel/PVInstance.h"
#include "GfxBase/IAdornable.h"
#include "Util/CameraSubject.h"
#include "Util/Selectable.h"

namespace RBX {

	extern const char* const sModel;

	// VTABLE: PLAYER 0x00b3f6bc RBX::Debugable
	// VTABLE: STUDIO 0x00c14ffc RBX::Debugable
	// VTABLE: PLAYER 0x00b3f68c RBX::IHasPrimaryPart
	// VTABLE: STUDIO 0x00c14fcc RBX::IHasPrimaryPart
	// VTABLE: PLAYER 0x00b3f658 RBX::IAdornable
	// VTABLE: STUDIO 0x00c14f98 RBX::IAdornable
	// VTABLE: PLAYER 0x00b3f62c RBX::CameraSubject
	// VTABLE: STUDIO 0x00c14f6c RBX::CameraSubject
	// VTABLE: PLAYER 0x00b3f624 RBX::Selectable
	// VTABLE: STUDIO 0x00c14f64 RBX::Selectable
	// VTABLE: PLAYER 0x00b3f61c RBX::IHasLocation
	// VTABLE: STUDIO 0x00c14f5c RBX::IHasLocation
	class ModelInstance
		: public DescribedCreatable<ModelInstance, PVInstance, sModel>
		// 2013 RTTI 0x00b3f6bc: IHasPrimaryPart +0x60 (in PVInstance), IAdornable +0x68,
		// CameraSubject +0x80, Selectable +0x94, Diagnostics::Countable<ModelInstance> +0x98,
		// virtual IHasLocation +0x12c. Members not modelled yet.
		, public IAdornable
		, public CameraSubject
		, public Selectable
	{
	public:
		ModelInstance() {}

		// Primary-vftable overrides (0x00b3f6bc slots +0x14/+0x18/+0x48/+0x50..+0x5c/+0x64).
		/*override*/ virtual void setName(const std::string& value);
		/*override*/ virtual int getPersistentDataCost() const;
	protected:
		/*override*/ virtual bool askSetParent(const Instance* instance) const;
		/*override*/ virtual void onDescendantAdded(Instance* instance);
		/*override*/ virtual void onDescendantRemoving(const shared_ptr<Instance>& instance);
		/*override*/ virtual void onChildAdded(Instance* child);
		/*override*/ virtual void onChildRemoving(Instance* child);
		/*override*/ virtual void onChildChanged(Instance* instance, const PropertyChanged& event);
	public:

		// IHasPrimaryPart / IHasLocation / CameraSubject / IAdornable overrides (2013 secondary
		// vftables of 0x00b3f6bc). Bodies in ModelInstance.cpp.
		/*override*/ virtual PartInstance* getPrimaryPart();
		/*override*/ virtual G3D::CoordinateFrame getLocation();
		/*override*/ virtual G3D::CoordinateFrame getRenderLocation();
		/*override*/ virtual G3D::Vector3 getRenderSize();
		/*override*/ virtual void onCameraNear(float distance);
		/*override*/ virtual void getCameraIgnorePrimitives(std::vector<const Primitive*>& primitives);
	protected:
		/*override*/ virtual bool shouldRender3dAdorn() const;
	public:
		/*override*/ virtual void render3dAdorn(RBX::Adorn* adorn);
		/*override*/ virtual void render3dSelect(RBX::Adorn* adorn, SelectState selectState);

		// PVInstance overrides (2013 vftable 0x00b3f6bc slots +0x78/+0x7c/+0x80). Not written yet.
	protected:
		/*override*/ virtual bool hitTestImpl(const RbxRay& worldRay, G3D::Vector3& worldHitPoint);
	public:
		/*override*/ virtual Extents computeExtentsWorld() const;
		/*override*/ virtual void legacyTraverseState(const G3D::CoordinateFrame& cf);
	};

} // namespace RBX
