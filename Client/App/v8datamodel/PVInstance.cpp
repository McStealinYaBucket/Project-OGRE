/* App/v8datamodel/PVInstance.cpp — October 2013 (ctor shell).
   Evidence: PVInstance ctor 0x4687C8 (FWBase::init<FWPVInstance> +
     Instance(name, fw) + classDescriptor/registrar wiring).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/PVInstance.h"

namespace RBX {
	const char* const sPVInstance = "PVInstance";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::PVInstance);
}

namespace RBX {

	// OnDemandPVInstance source (C2 0x48E8B8, pool 24).
	// STUB: PLAYER 0x006d32c0
	// STUB: STUDIO 0x00788d50
	OnDemandInstance* PVInstance::initOnDemand()
	{
		return new OnDemandPVInstance();
	}

	// mac: 0x4687C8 shape: fresh FWPVInstance through the Instance(name, fw) path.
	// (Direct Described base init — PVInstance is only an indirect base.)
	typedef DescribedNonCreatable<PVInstance, Instance, sPVInstance, Reflection::ClassDescriptor::PERSISTENT> PVSuper;

	PVInstance::PVInstance()
		: PVSuper((const char*)NULL, FWBase::init<FWPVInstance>((FWPVInstance*)NULL))
	{
	}

	PVInstance::PVInstance(const char* name, FWPVInstance* fw)
		: PVSuper(name, fw != NULL ? fw : FWBase::init<FWPVInstance>((FWPVInstance*)NULL))
	{
	}

} // namespace RBX
