/* V8DataModel/ModelInstance.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ModelInstance, PVInstance, sModel> 0x437C70;
     sModelInstance = "Model" (0.125 metadata class list; C++ name differs from Lua).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ModelInstance.h */

#pragma once

#include "V8DataModel/PVInstance.h"

namespace RBX {

	extern const char* const sModel;

	class ModelInstance
		: public DescribedCreatable<ModelInstance, PVInstance, sModel>
	{
	public:
		ModelInstance() {}

		// PVInstance overrides (2013 vftable 0x00b3f6bc slots +0x78/+0x7c/+0x80). Not written yet.
	protected:
		/*override*/ virtual bool hitTestImpl(const RbxRay& worldRay, G3D::Vector3& worldHitPoint);
	public:
		/*override*/ virtual Extents computeExtentsWorld() const;
		/*override*/ virtual void legacyTraverseState(const G3D::CoordinateFrame& cf);
	};

} // namespace RBX
