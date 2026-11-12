/* App/v8datamodel/FWService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/FWService.h"

namespace RBX {
	const char* const sFWService = "FWService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::FWService);
}

namespace RBX {

	// mac: 0x6D2BCE/C2: Instance(nullptr) base; +92 = 1; dict/validator default.
	// The embedded dict self-provisions through the service (aliasing
	// getUniqueSharedPtr) — it is not itself shared-owned.
	FWService::FWService()
		: unknown92(true)
	{
		partDictMember.selfProvider = boost::bind(&FWService::getUniqueSharedPtr, this);
	}

	// mac: 0x6D2E24: aliasing shared_ptr onto the embedded dict (service-owned).
	shared_ptr<FWDictionnaryBase> FWService::getUniqueSharedPtr()
	{
		shared_ptr<FWService> self = shared_from<FWService>(this);
		return shared_ptr<FWDictionnaryBase>(self, &partDictMember);
	}

	// mac: 0x6D2E8E: holder == this+96.
	bool FWService::isOwnedHolder(IFWHolder* holder)
	{
		return holder == &partDictMember;
	}

} // namespace RBX
