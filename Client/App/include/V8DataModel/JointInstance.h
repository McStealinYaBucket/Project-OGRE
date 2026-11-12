/* V8DataModel/JointInstance.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<JointInstance, Instance> 0x371B10; abstract joint root;
     sJointInstance = "JointInstance".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/JointInstance.h */

#pragma once

#include "V8Tree/Instance.h"
#include "Util/Object.h"
#include "boost/weak_ptr.hpp"

namespace RBX {

	extern const char* const sJointInstance;

	class Joint;
	class PartInstance;

	class JointInstance
		: public DescribedNonCreatable<JointInstance, Instance, sJointInstance>
	{
	public:
		JointInstance() : joint(NULL) {}
		// mac: 0x40CE04 (564B).  Stores the physics joint at +132 (the link the
		// 5-arg destroyAutoJoints reads back), wires the owner's backref via
		// Joint::setJointOwner, and snapshots both parts (prim+16/+20) as
		// shared_ptrs (+116/+124, lifetime).  The vtable/reflection fixups
		// are the base's own work.  The +96..+112 dwords, FastLog lifetime
		// line, and part-snapshot members stay unmodeled (nothing reads
		// them in any ported body) -- so this ctor stores the joint and
		// calls setJointOwner, which is the entire load-bearing prefix.
		explicit JointInstance(Joint* joint);

		// The +132 backlink reader (destroyAutoJoints phase 2).
		Joint* getJoint() const { return joint; }

		// mac: 0x40D3B2 / 0x40D3E8 (54B each): lock the weak_ptr at +116/+124
		// into a shared_ptr (nothrow -- expired yields null, never throws).
		// Our equivalents (boost::weak_ptr::lock() has exactly those
		// semantics).  The weak members are written only by setPart
		// (0x40D650, needs Joint slot-4 -- later round), so until then these
		// read null -- asserted as such, not as behavior.
		shared_ptr<PartInstance> getPart0();
		shared_ptr<PartInstance> getPart1();

		// mac: 0x40C86C getPart0Dangerous (const): byte-identical body to
		// getPart0 (same weak lock at +116).  The name suggests it skips
		// some safety, but the disasm shows none -- ported as-is.
		shared_ptr<PartInstance> getPart0Dangerous() const;

		// mac: 0x40CAE8 / 0x40CB42 (const, 17B each): return joint+56 /
		// joint+104 -- the jointCoord0/1 members -- BY ADDRESS (the binary
		// returns the pointer).  No null guard in the binary either.
		const G3D::CoordinateFrame& getC0() const;
		const G3D::CoordinateFrame& getC1() const;

	private:
		// +116/+124: weak_ptr<PartInstance> (8B each: raw + weak count).
		// The C2/setPart stores decode exactly to weak assignment (raw to
		// .px, weak_count::operator= to .pn) -- NOT shared_ptr members.
		boost::weak_ptr<PartInstance> weakPart0;
		boost::weak_ptr<PartInstance> weakPart1;
		// +132: the physics Joint* (NULL until the Joint* ctor runs).
		Joint* joint;
	};

} // namespace RBX
