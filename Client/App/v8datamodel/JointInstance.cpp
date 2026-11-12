/* App/v8datamodel/JointInstance.cpp -- October 2013 (Joint* ctor only;
   the class is otherwise still a minimal shell -- see the header). */

#include "stdafx.h"

#include "V8DataModel/JointInstance.h"
#include "V8World/Joint.h"

namespace RBX {
	const char* const sJointInstance = "JointInstance";

	// mac: 0x40CE04.  Load-bearing prefix only (see header): store +132 and wire
	// the backref.  The part snapshots (+116/+124) are lifetime management
	// for DataModel-owned parts; harness parts are shared_ptr-held by the
	// test itself, so nothing dangles.
	// STUB: PLAYER 0x007fccd0
	// STUB: STUDIO 0x007a2e30
	JointInstance::JointInstance(Joint* joint)
		: joint(joint)
	{
		if (joint != NULL)
			joint->setJointOwner(this);
	}

	// mac: 0x40D3B2 / 0x40D3E8.
	shared_ptr<PartInstance> JointInstance::getPart0()
	{
		return weakPart0.lock();
	}
	shared_ptr<PartInstance> JointInstance::getPart1()
	{
		return weakPart1.lock();
	}

	// mac: 0x40C86C.
	shared_ptr<PartInstance> JointInstance::getPart0Dangerous() const
	{
		return weakPart0.lock();
	}

	// mac: 0x40CAE8 / 0x40CB42.
	const G3D::CoordinateFrame& JointInstance::getC0() const
	{
		return joint->getJointCoord0();
	}
	const G3D::CoordinateFrame& JointInstance::getC1() const
	{
		return joint->getJointCoord1();
	}
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::JointInstance);
}
