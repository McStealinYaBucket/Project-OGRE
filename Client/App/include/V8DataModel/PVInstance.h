/* V8DataModel/PVInstance.h — October 2013 (position/velocity base shell).
   Evidence: PVInstance ctor 0x4687C8 (name + FWPVInstance* -> Instance ctor,
     Described<PVInstance, sPVInstance, Instance, Functionality 27>).
     sPVInstance present 34x in the 0.125 Studio PE string table.
     TRUE position in the chain: Instance -> PVInstance -> PartInstance
     (PartInstance isA-checks route through it; FW chain FWInstance ->
     FWPVInstance -> FWPartInstance mirrors it).
   TODO: full decompile (position/rotation/velocity/filtering surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PVInstance.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/OnDemand.h"
#include "RbxG3D/RbxRay.h"
#include "Util/Extents.h"
#include "Util/IHasLocation.h"

namespace RBX {

	extern const char* const sPVInstance;

	class PartInstance;

	// 2013 only (2016 keeps just the comment "// IHasPrimaryPart" in ModelInstance.h/PartInstance.h).
	// RTTI of PartInstance 0x00b4c784 and ModelInstance 0x00b3f6bc: IHasPrimaryPart at +96 with the
	// virtual IHasLocation reached through the vbptr at +100 (pdisp 100). One slot: getPrimaryPart
	// (PartInstance 0x00723500 `lea eax,[ecx-0x60]; ret`, ModelInstance 0x006d48b0).
	class RBXInterface IHasPrimaryPart : public virtual IHasLocation
	{
	public:
		virtual PartInstance* getPrimaryPart() = 0;  // vtable+0x00
	};

	class PVInstance
		: public DescribedNonCreatable<PVInstance, Instance, sPVInstance, Reflection::ClassDescriptor::PERSISTENT>
		// Functionality 27 per the 0x4687C8 classDescriptor reference
		// (Described<..., Instance, 27, Permissions 0>).
		, public IHasPrimaryPart  // +0x60 (vbptr +0x64), RTTI above
	{
	public:
		PVInstance();
		PVInstance(const char* name, FWPVInstance* fw);
		virtual OnDemandInstance* initOnDemand();  // 0x468DD0: news OnDemandPVInstance.
		// mac: 0x46D4E0 / 0x519920 (7B each: `xor eax,eax`).  PVInstance slots.
		// STUB: PLAYER 0x0096bde0
		virtual int topHashCode() const { return 0; }
		// STUB: PLAYER 0x0096bde0
		virtual int childHashCode() const { return 0; }

		// vtable+0x78/+0x7c/+0x80: the other virtuals PVInstance adds in 2013 (ModelInstance
		// vftable 0x00b3f6bc has 33 slots ending here; PartInstance 0x00b4c784 and iOS 10-29
		// put the same three after childHashCode). Pure in 2016's PVInstance.h (hitTestImpl,
		// computeExtentsWorld); legacyTraverseState is gone by 2016.
	protected:
		/*override*/ virtual void readProperty(const XmlElement* propertyElement, IReferenceBinder& binder);  // vtable+0x6c
		virtual bool hitTestImpl(const RbxRay& worldRay, G3D::Vector3& worldHitPoint) = 0;
	public:
		virtual Extents computeExtentsWorld() const = 0;
		virtual void legacyTraverseState(const G3D::CoordinateFrame& cf) = 0;
	};

} // namespace RBX
